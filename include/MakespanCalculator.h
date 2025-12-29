#pragma once

#include "Models.h"

/**
 * MakespanCalculator: Bir çizelgenin makespan'ını (maksimum bitiş zamanı) hesaplar.
 */
class MakespanCalculator {
public:
    /**
     * Çözülmüş bir çizelgeden makespan'ı hesaplar.
     * 
     * @param schedule opTimes'ı doldurulmuş çizelge
     * @return Makespan (maksimum bitiş zamanı), çizelge boş/geçersizse -1
     */
    static int calculate(const Schedule& schedule);
};

