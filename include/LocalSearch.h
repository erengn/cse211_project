#pragma once

#include "Models.h"
#include "ScheduleDecoder.h"
#include "MakespanCalculator.h"
#include "FeasibilityChecker.h"

/**
 * LocalSearch: Mevcut çizelgeleri iyileştirmek için yerel arama yapar.
 * 
 * Strateji:
 * - Makinelerdeki bitişik işlemleri değiştirir (sadece farklı işlerden olanlar)
 * - Sadece uygulanabilir ve makespan'ı iyileştiren çizelgeleri kabul eder
 */
class LocalSearch {
private:
    const ProblemInstance& instance_;

    /**
     * Bir makinede iki bitişik işlemi değiştirir (swap).
     * 
     * @param schedule Değiştirilecek çizelge
     * @param machineId Makine ID'si
     * @param pos1 İlk işlemin pozisyonu
     * @param pos2 İkinci işlemin pozisyonu (pos1 + 1 olmalı)
     * @return Değiştirilmiş çizelge (kopya)
     */
    Schedule swapAdjacentOperations(
        const Schedule& schedule,
        const std::string& machineId,
        size_t pos1,
        size_t pos2) const;

    /**
     * Bir çizelgede tüm geçerli swap'ları bulur ve en iyisini döndürür.
     * 
     * @param currentSchedule Mevcut çizelge
     * @param currentMakespan Mevcut makespan
     * @return En iyi swap sonucu (makespan, yeni çizelge) veya (currentMakespan, currentSchedule) eğer iyileştirme yoksa
     */
    std::pair<int, Schedule> findBestSwap(
        const Schedule& currentSchedule,
        int currentMakespan) const;

public:
    /**
     * ProblemInstance referansı ile başlatır.
     * 
     * @param instance Problem örneği
     */
    explicit LocalSearch(const ProblemInstance& instance);

    /**
     * Bir çizelgeyi yerel arama ile iyileştirir.
     * Bitişik işlemleri değiştirerek daha iyi makespan bulmaya çalışır.
     * 
     * @param initialSchedule Başlangıç çizelgesi
     * @param maxIterations Maksimum iterasyon sayısı (varsayılan: 100)
     * @return İyileştirilmiş çizelge ve makespan'ı
     */
    std::pair<Schedule, int> improveSchedule(
        const Schedule& initialSchedule,
        int maxIterations = 100) const;
};

