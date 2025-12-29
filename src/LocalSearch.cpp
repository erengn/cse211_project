#include "LocalSearch.h"
#include <algorithm>
#include <climits>

LocalSearch::LocalSearch(const ProblemInstance& instance)
    : instance_(instance) {
}

Schedule LocalSearch::swapAdjacentOperations(
    const Schedule& schedule,
    const std::string& machineId,
    size_t pos1,
    size_t pos2) const {
    
    Schedule newSchedule = schedule;
    
    // Makine sırasını kontrol et
    auto it = newSchedule.machineOrder.find(machineId);
    if (it == newSchedule.machineOrder.end()) {
        return newSchedule; // Makine bulunamadı
    }
    
    std::vector<OpKey>& sequence = it->second;
    
    // Pozisyonları kontrol et
    if (pos2 != pos1 + 1 || pos2 >= sequence.size()) {
        return newSchedule; // Geçersiz pozisyonlar
    }
    
    // İki işlem farklı işlerden olmalı
    if (sequence[pos1].jobId == sequence[pos2].jobId) {
        return newSchedule; // Aynı işten, swap yapılamaz
    }
    
    // Swap yap
    std::swap(sequence[pos1], sequence[pos2]);
    
    // opTimes'ı temizle, yeniden çözülecek
    newSchedule.opTimes.clear();
    
    return newSchedule;
}

std::pair<int, Schedule> LocalSearch::findBestSwap(
    const Schedule& currentSchedule,
    int currentMakespan) const {
    
    int bestMakespan = currentMakespan;
    Schedule bestSchedule = currentSchedule;
    bool improved = false;
    
    // Her makine için bitişik işlemleri kontrol et
    for (const auto& [machineId, sequence] : currentSchedule.machineOrder) {
        if (sequence.size() < 2) {
            continue; // En az 2 işlem gerekli
        }
        
        // Her bitişik çifti dene
        for (size_t i = 0; i < sequence.size() - 1; ++i) {
            // Farklı işlerden olmalı
            if (sequence[i].jobId == sequence[i + 1].jobId) {
                continue;
            }
            
            // Swap yap
            Schedule candidate = swapAdjacentOperations(currentSchedule, machineId, i, i + 1);
            
            // Çizelgeyi çöz
            if (!ScheduleDecoder::decode(candidate, instance_)) {
                continue; // Çözülemedi
            }
            
            // Uygulanabilirliği kontrol et
            if (!FeasibilityChecker::isValid(candidate, instance_)) {
                continue; // Geçersiz
            }
            
            // Makespan'ı hesapla
            int candidateMakespan = MakespanCalculator::calculate(candidate);
            
            // Daha iyi mi?
            if (candidateMakespan < bestMakespan) {
                bestMakespan = candidateMakespan;
                bestSchedule = candidate;
                improved = true;
            }
        }
    }
    
    return {bestMakespan, bestSchedule};
}

std::pair<Schedule, int> LocalSearch::improveSchedule(
    const Schedule& initialSchedule,
    int maxIterations) const {
    
    Schedule currentSchedule = initialSchedule;
    
    // İlk çizelgeyi çöz
    if (!ScheduleDecoder::decode(currentSchedule, instance_)) {
        return {currentSchedule, -1}; // Çözülemedi
    }
    
    // İlk makespan'ı hesapla
    int currentMakespan = MakespanCalculator::calculate(currentSchedule);
    
    // Yerel arama döngüsü
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // En iyi swap'ı bul
        auto [newMakespan, newSchedule] = findBestSwap(currentSchedule, currentMakespan);
        
        // İyileştirme var mı?
        if (newMakespan >= currentMakespan) {
            break; // Daha iyi çözüm bulunamadı
        }
        
        // Yeni çözümü kabul et
        currentSchedule = newSchedule;
        currentMakespan = newMakespan;
    }
    
    return {currentSchedule, currentMakespan};
}

