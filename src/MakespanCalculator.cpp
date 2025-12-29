#include "MakespanCalculator.h"

int MakespanCalculator::calculate(const Schedule& schedule) {
    int makespan = -1;

    // Tüm işlemler arasında maksimum bitiş zamanını bul
    for (const auto& [jobId, opTimes] : schedule.opTimes) {
        for (const auto& [opIndex, timeWindow] : opTimes) {
            if (timeWindow.end > makespan) {
                makespan = timeWindow.end;
            }
        }
    }

    return makespan;
}

