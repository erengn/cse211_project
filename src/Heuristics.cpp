#include "Heuristics.h"
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <climits>

DispatchHeuristics::DispatchHeuristics(const ProblemInstance& instance)
    : instance_(instance) {
}

std::vector<OpKey> DispatchHeuristics::getReadyOperations(
    const std::unordered_map<std::string, int>& scheduledOpCount,
    const std::string& machineId) const {
    
    std::vector<OpKey> ready;
    
    // Tüm işleri kontrol et
    for (const auto& [jobId, jobPtr] : instance_.jobs) {
        const Job* job = jobPtr.get();
        if (!job) continue;
        
        const std::vector<Operation>& ops = job->operations();
        
        // Bu işin kaç işlemi çizelgelenmiş?
        int scheduledCount = 0;
        auto it = scheduledOpCount.find(jobId);
        if (it != scheduledOpCount.end()) {
            scheduledCount = it->second;
        }
        
        // Eğer bir sonraki işlem hazırsa ve bu makineye aitse
        if (scheduledCount < static_cast<int>(ops.size())) {
            const Operation& nextOp = ops[scheduledCount];
            if (nextOp.machineId() == machineId) {
                ready.push_back(OpKey{jobId, scheduledCount});
            }
        }
    }
    
    return ready;
}

int DispatchHeuristics::getJobTotalTime(const std::string& jobId) const {
    const Job* job = instance_.getJob(jobId);
    if (!job) return 0;
    return job->totalProcessingTime();
}

Schedule DispatchHeuristics::buildScheduleFromMachineSequences(
    const std::unordered_map<std::string, std::vector<OpKey>>& machineSequences) const {
    
    Schedule schedule;
    schedule.machineOrder = machineSequences;
    
    // ScheduleDecoder kullanarak zamanları hesapla
    ScheduleDecoder::decode(schedule, instance_);
    
    return schedule;
}

Schedule DispatchHeuristics::buildSPTSchedule() {
    // Makine sıralarını tut
    std::unordered_map<std::string, std::vector<OpKey>> machineSequences;
    
    // Her makine için boş sıra oluştur
    for (const auto& [machineId, _] : instance_.machines) {
        machineSequences[machineId] = std::vector<OpKey>();
    }
    
    // Her işin kaç işleminin çizelgelendiğini takip et
    std::unordered_map<std::string, int> scheduledOpCount;
    for (const auto& [jobId, _] : instance_.jobs) {
        scheduledOpCount[jobId] = 0;
    }
    
    // Toplam işlem sayısını hesapla
    int totalOps = 0;
    for (const auto& [jobId, jobPtr] : instance_.jobs) {
        totalOps += static_cast<int>(jobPtr->operations().size());
    }
    
    // Tüm işlemler çizelgelenene kadar devam et
    for (int scheduled = 0; scheduled < totalOps; ++scheduled) {
        OpKey bestOp;
        std::string bestMachine;
        int bestDuration = INT_MAX;
        
        // Her makine için hazır işlemleri kontrol et
        for (const auto& [machineId, _] : instance_.machines) {
            std::vector<OpKey> ready = getReadyOperations(scheduledOpCount, machineId);
            
            // En kısa süreye sahip işlemi bul
            for (const OpKey& opKey : ready) {
                const Job* job = instance_.getJob(opKey.jobId);
                if (!job) continue;
                
                const std::vector<Operation>& ops = job->operations();
                if (opKey.opIndex < 0 || opKey.opIndex >= static_cast<int>(ops.size())) {
                    continue;
                }
                
                int duration = ops[opKey.opIndex].duration();
                if (duration < bestDuration) {
                    bestDuration = duration;
                    bestOp = opKey;
                    bestMachine = machineId;
                }
            }
        }
        
        // En iyi işlemi çizelgele
        if (bestDuration != INT_MAX) {
            machineSequences[bestMachine].push_back(bestOp);
            scheduledOpCount[bestOp.jobId]++;
        } else {
            // Hiç hazır işlem yoksa hata
            break;
        }
    }
    
    return buildScheduleFromMachineSequences(machineSequences);
}

Schedule DispatchHeuristics::buildLJFSchedule() {
    // Makine sıralarını tut
    std::unordered_map<std::string, std::vector<OpKey>> machineSequences;
    
    // Her makine için boş sıra oluştur
    for (const auto& [machineId, _] : instance_.machines) {
        machineSequences[machineId] = std::vector<OpKey>();
    }
    
    // Her işin kaç işleminin çizelgelendiğini takip et
    std::unordered_map<std::string, int> scheduledOpCount;
    for (const auto& [jobId, _] : instance_.jobs) {
        scheduledOpCount[jobId] = 0;
    }
    
    // Toplam işlem sayısını hesapla
    int totalOps = 0;
    for (const auto& [jobId, jobPtr] : instance_.jobs) {
        totalOps += static_cast<int>(jobPtr->operations().size());
    }
    
    // Tüm işlemler çizelgelenene kadar devam et
    for (int scheduled = 0; scheduled < totalOps; ++scheduled) {
        OpKey bestOp;
        std::string bestMachine;
        int bestJobTotalTime = -1;
        
        // Her makine için hazır işlemleri kontrol et
        for (const auto& [machineId, _] : instance_.machines) {
            std::vector<OpKey> ready = getReadyOperations(scheduledOpCount, machineId);
            
            // En uzun toplam işlem süresine sahip işin işlemini bul
            for (const OpKey& opKey : ready) {
                int jobTotalTime = getJobTotalTime(opKey.jobId);
                if (jobTotalTime > bestJobTotalTime) {
                    bestJobTotalTime = jobTotalTime;
                    bestOp = opKey;
                    bestMachine = machineId;
                }
            }
        }
        
        // En iyi işlemi çizelgele
        if (bestJobTotalTime != -1) {
            machineSequences[bestMachine].push_back(bestOp);
            scheduledOpCount[bestOp.jobId]++;
        } else {
            // Hiç hazır işlem yoksa hata
            break;
        }
    }
    
    return buildScheduleFromMachineSequences(machineSequences);
}

Schedule DispatchHeuristics::buildCriticalPathSchedule() {
    // Önce bir SPT çizelgesi oluştur
    Schedule sptSchedule = buildSPTSchedule();
    
    // Çizelgeyi çöz
    ScheduleDecoder::decode(sptSchedule, instance_);
    
    // Makespan'ı hesapla
    int makespan = MakespanCalculator::calculate(sptSchedule);
    
    // Kritik yoldaki işlemleri belirle
    // Kritik yol: makespan'a kadar uzanan en uzun yol
    // Basit yaklaşım: makespan'da biten işlemleri kritik olarak işaretle
    std::unordered_set<std::string> criticalJobs;
    
    for (const auto& [jobId, opTimes] : sptSchedule.opTimes) {
        for (const auto& [opIndex, timeWindow] : opTimes) {
            if (timeWindow.end == makespan) {
                criticalJobs.insert(jobId);
            }
        }
    }
    
    // Şimdi kritik işleri önceliklendirerek yeni bir çizelge oluştur
    std::unordered_map<std::string, std::vector<OpKey>> machineSequences;
    
    // Her makine için boş sıra oluştur
    for (const auto& [machineId, _] : instance_.machines) {
        machineSequences[machineId] = std::vector<OpKey>();
    }
    
    // Her işin kaç işleminin çizelgelendiğini takip et
    std::unordered_map<std::string, int> scheduledOpCount;
    for (const auto& [jobId, _] : instance_.jobs) {
        scheduledOpCount[jobId] = 0;
    }
    
    // Toplam işlem sayısını hesapla
    int totalOps = 0;
    for (const auto& [jobId, jobPtr] : instance_.jobs) {
        totalOps += static_cast<int>(jobPtr->operations().size());
    }
    
    // Tüm işlemler çizelgelenene kadar devam et
    for (int scheduled = 0; scheduled < totalOps; ++scheduled) {
        OpKey bestOp;
        std::string bestMachine;
        bool bestIsCritical = false;
        int bestDuration = INT_MAX;
        
        // Her makine için hazır işlemleri kontrol et
        for (const auto& [machineId, _] : instance_.machines) {
            std::vector<OpKey> ready = getReadyOperations(scheduledOpCount, machineId);
            
            // Önce kritik işleri kontrol et, sonra diğerlerini
            for (const OpKey& opKey : ready) {
                bool isCritical = (criticalJobs.find(opKey.jobId) != criticalJobs.end());
                const Job* job = instance_.getJob(opKey.jobId);
                if (!job) continue;
                
                const std::vector<Operation>& ops = job->operations();
                if (opKey.opIndex < 0 || opKey.opIndex >= static_cast<int>(ops.size())) {
                    continue;
                }
                
                int duration = ops[opKey.opIndex].duration();
                
                // Kritik işler öncelikli, aynı öncelikteyse en kısa süre
                if (isCritical && !bestIsCritical) {
                    bestIsCritical = true;
                    bestDuration = duration;
                    bestOp = opKey;
                    bestMachine = machineId;
                } else if (isCritical == bestIsCritical) {
                    if (duration < bestDuration) {
                        bestDuration = duration;
                        bestOp = opKey;
                        bestMachine = machineId;
                    }
                }
            }
        }
        
        // En iyi işlemi çizelgele
        if (bestDuration != INT_MAX) {
            machineSequences[bestMachine].push_back(bestOp);
            scheduledOpCount[bestOp.jobId]++;
        } else {
            // Hiç hazır işlem yoksa hata
            break;
        }
    }
    
    return buildScheduleFromMachineSequences(machineSequences);
}

