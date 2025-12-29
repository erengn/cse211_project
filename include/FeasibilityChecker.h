#pragma once

#include "Models.h"

/**
 * FeasibilityChecker: Bir çizelgenin uygulanabilir olup olmadığını doğrular.
 * 
 * Kontroller:
 * - İş önceliği: Aynı işin işlemleri sıraya uymalı
 * - Makine kısıtı: Aynı makinede çakışan aralıklar olmamalı
 */
class FeasibilityChecker {
public:
    /**
     * Bir çizelgenin uygulanabilir olup olmadığını kontrol eder.
     * 
     * @param schedule Doğrulanacak çizelge (opTimes doldurulmuş olmalı)
     * @param instance Doğrulama için problem örneği
     * @return çizelge geçerliyse true, aksi halde false
     */
    static bool isValid(const Schedule& schedule, const ProblemInstance& instance);
};

