#include "FeasibilityChecker.h"
#include <algorithm>
#include <vector>

bool FeasibilityChecker::isValid(const Schedule& schedule, const ProblemInstance& instance) {
    // Kontrol 1: İş öncelik kısıtı
    // Her iş için, işlemler sırayla olmalı: Op[j][k+1], Op[j][k] bitmeden başlayamaz
    for (const auto& [jobId, opTimes] : schedule.opTimes) {
        const Job* job = instance.getJob(jobId);
        if (!job) {
            return false; // Geçersiz iş referansı
        }

        const std::vector<Operation>& ops = job->operations();
        
        // Her ardışık işlem çiftini kontrol et
        for (int i = 0; i < static_cast<int>(ops.size()) - 1; ++i) {
            auto it1 = opTimes.find(i);
            auto it2 = opTimes.find(i + 1);

            if (it1 == opTimes.end() || it2 == opTimes.end()) {
                return false; // Eksik işlem zamanları
            }

            // İşlem i+1, işlem i bittikten sonra başlamalı
            if (it2->second.start < it1->second.end) {
                return false; // Öncelik ihlali
            }
        }
    }

    // Kontrol 2: Makine kısıtı
    // Her makinede, işlemler çakışmamalı ve sıralı olmalı
    for (const auto& [machineId, opSequence] : schedule.machineOrder) {
        if (opSequence.empty()) {
            continue;
        }

        // Bu makinedeki tüm işlemleri zaman pencereleriyle birlikte topla
        std::vector<std::pair<OpKey, TimeWindow>> machineOps;
        for (const OpKey& opKey : opSequence) {
            auto jobIt = schedule.opTimes.find(opKey.jobId);
            if (jobIt == schedule.opTimes.end()) {
                return false; // Eksik iş zamanları
            }

            auto opIt = jobIt->second.find(opKey.opIndex);
            if (opIt == jobIt->second.end()) {
                return false; // Eksik işlem zamanları
            }

            // İşlemin doğru makinede olduğunu doğrula
            const Job* job = instance.getJob(opKey.jobId);
            if (!job) {
                return false;
            }
            const std::vector<Operation>& ops = job->operations();
            if (opKey.opIndex < 0 || opKey.opIndex >= static_cast<int>(ops.size())) {
                return false;
            }
            if (ops[opKey.opIndex].machineId() != machineId) {
                return false; // İşlem yanlış makineye atanmış
            }

            machineOps.push_back({opKey, opIt->second});
        }

        // Çakışmaları kontrol et: işlemler sıralı olmalı (sonraki başlangıç >= önceki bitiş)
        for (size_t i = 0; i < machineOps.size() - 1; ++i) {
            if (machineOps[i + 1].second.start < machineOps[i].second.end) {
                return false; // Aynı makinede çakışan işlemler
            }
        }
    }

    return true;
}

