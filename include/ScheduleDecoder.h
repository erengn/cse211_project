#pragma once

#include "Models.h"

/**
 * ScheduleDecoder: Makine başına işlem sıralarını zamana dayalı bir çizelgeye dönüştürür.
 * 
 * Kurallar:
 * - Bir işlem, aynı işin önceki işlemi bitmeden başlayamaz
 * - Bir makine aynı anda sadece 1 işlem çalıştırabilir
 * - başlangıç = max(iş_hazır_zamanı, makine_müsait_zamanı)
 * - bitiş = başlangıç + süre
 */
class ScheduleDecoder {
public:
    /**
     * machineOrder'a göre opTimes'ı doldurarak bir çizelgeyi çözer.
     * 
     * @param schedule machineOrder'ı doldurulmuş çizelge (opTimes doldurulacak)
     * @param instance İş ve makine tanımlarını içeren problem örneği
     * @return çözme başarılıysa true, aksi halde false
     */
    static bool decode(Schedule& schedule, const ProblemInstance& instance);
};

