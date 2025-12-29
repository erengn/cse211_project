#include "ScheduleDecoder.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

bool ScheduleDecoder::decode(Schedule& schedule, const ProblemInstance& instance) {
    // Mevcut opTimes'ı temizle
    schedule.opTimes.clear();

    // Her işin bir sonraki işleminin ne zaman başlayabileceğini takip et (iş hazır zamanı)
    std::unordered_map<std::string, int> jobReadyTime;
    
    // Her makinenin ne zaman müsait olduğunu ve sıradaki konumunu takip et
    std::unordered_map<std::string, int> machineAvailableTime;
    std::unordered_map<std::string, size_t> machinePosition;

    // Makine müsait zamanlarını ve konumlarını başlat
    for (const auto& [machineId, _] : schedule.machineOrder) {
        machineAvailableTime[machineId] = 0;
        machinePosition[machineId] = 0;
    }

    // Hangi işlemlerin çizelgelendiğini takip et
    std::unordered_map<std::string, std::unordered_set<int>> scheduledOps;

    // Tüm işlemler çizelgelenene veya takılı kalana kadar çizelgeleme yap
    bool progress = true;
    int maxIterations = 10000; // Güvenlik sınırı
    int iterations = 0;

    while (progress && iterations < maxIterations) {
        progress = false;
        iterations++;

        // Her makinede işlemleri çizelgelemeyi dene
        for (auto& [machineId, opSequence] : schedule.machineOrder) {
            size_t& pos = machinePosition[machineId];
            if (pos >= opSequence.size()) {
                continue; // Bu makinenin sırası bitti
            }

            const OpKey& opKey = opSequence[pos];

            // Bu işlemin çizelgelenip çizelgenemeyeceğini kontrol et (aynı işin önceki işlemi bitmiş olmalı)
            bool canSchedule = true;
            int jobReady = 0;

            if (opKey.opIndex > 0) {
                // Aynı işin önceki işleminin çizelgelenip çizelgenmediğini kontrol et
                auto scheduledIt = scheduledOps.find(opKey.jobId);
                if (scheduledIt == scheduledOps.end() || 
                    scheduledIt->second.find(opKey.opIndex - 1) == scheduledIt->second.end()) {
                    canSchedule = false; // Önceki işlem henüz çizelgelenmedi
                } else {
                    // Önceki işlemin bitiş zamanını al
                    auto jobIt = schedule.opTimes.find(opKey.jobId);
                    if (jobIt != schedule.opTimes.end()) {
                        auto prevIt = jobIt->second.find(opKey.opIndex - 1);
                        if (prevIt != jobIt->second.end()) {
                            jobReady = prevIt->second.end;
                        }
                    }
                }
            }

            if (!canSchedule) {
                continue; // Bu işlemi şimdilik atla
            }

            // İşi ve işlemi al
            const Job* job = instance.getJob(opKey.jobId);
            if (!job) {
                return false; // Geçersiz iş referansı
            }

            const std::vector<Operation>& ops = job->operations();
            if (opKey.opIndex < 0 || opKey.opIndex >= static_cast<int>(ops.size())) {
                return false; // Geçersiz işlem indeksi
            }

            const Operation& op = ops[opKey.opIndex];
            
            // Bu işlemin bu makineye ait olduğunu doğrula
            if (op.machineId() != machineId) {
                return false; // İşlem yanlış makineye atanmış
            }

            // Başlangıç zamanı = max(iş_hazır_zamanı, makine_müsait_zamanı)
            int startTime = std::max(jobReady, machineAvailableTime[machineId]);
            int endTime = startTime + op.duration();

            // Zaman penceresini kaydet
            schedule.opTimes[opKey.jobId][opKey.opIndex] = TimeWindow{startTime, endTime};
            scheduledOps[opKey.jobId].insert(opKey.opIndex);

            // Makine müsait zamanını güncelle
            machineAvailableTime[machineId] = endTime;

            // Bu makinedeki bir sonraki işleme geç
            pos++;
            progress = true;
        }
    }

    // Tüm işlemlerin çizelgelenip çizelgenmediğini kontrol et
    for (const auto& [machineId, opSequence] : schedule.machineOrder) {
        if (machinePosition[machineId] < opSequence.size()) {
            return false; // Tüm işlemler çizelgelenmedi (muhtemelen geçersiz öncelik nedeniyle)
        }
    }

    return true;
}

