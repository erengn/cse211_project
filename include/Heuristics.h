#pragma once

#include "Models.h"
#include "ScheduleDecoder.h"
#include "MakespanCalculator.h"
#include "FeasibilityChecker.h"
#include <vector>
#include <unordered_map>

/**
 * DispatchHeuristics: Farklı dağıtım sezgileri kullanarak başlangıç çizelgeleri oluşturur.
 * 
 * Desteklenen sezgiler:
 * - SPT (Shortest Processing Time): En kısa işlem süresine sahip işlemleri önceliklendirir
 * - LJF (Longest Job First): En uzun toplam işlem süresine sahip işleri önceliklendirir
 * - Critical Path Priority: Kritik yoldaki işlemleri önceliklendirir
 */
class DispatchHeuristics {
private:
    const ProblemInstance& instance_;
    
    // Yardımcı fonksiyonlar
    std::vector<OpKey> getReadyOperations(
        const std::unordered_map<std::string, int>& scheduledOpCount,
        const std::string& machineId) const;
    
    int getJobTotalTime(const std::string& jobId) const;
    
    Schedule buildScheduleFromMachineSequences(
        const std::unordered_map<std::string, std::vector<OpKey>>& machineSequences) const;

public:
    /**
     * ProblemInstance referansı ile başlatır.
     * 
     * @param instance Problem örneği (işler ve makineler)
     */
    explicit DispatchHeuristics(const ProblemInstance& instance);

    /**
     * SPT (Shortest Processing Time) sezgisi ile çizelge oluşturur.
     * Her makinede, hazır işlemler arasından en kısa süreye sahip olanı seçer.
     * 
     * @return Oluşturulan çizelge
     */
    Schedule buildSPTSchedule();

    /**
     * LJF (Longest Job First) sezgisi ile çizelge oluşturur.
     * Her makinede, hazır işlemler arasından en uzun toplam işlem süresine sahip işin işlemini seçer.
     * 
     * @return Oluşturulan çizelge
     */
    Schedule buildLJFSchedule();

    /**
     * Critical Path Priority sezgisi ile çizelge oluşturur.
     * Önce bir SPT çizelgesi oluşturur, sonra kritik yoldaki işlemleri belirler
     * ve bunları önceliklendirerek yeni bir çizelge oluşturur.
     * 
     * @return Oluşturulan çizelge
     */
    Schedule buildCriticalPathSchedule();
};

