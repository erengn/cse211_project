#include <iostream>
#include <cassert>
#include "ScheduleDecoder.h"
#include "MakespanCalculator.h"
#include "FeasibilityChecker.h"
#include "Models.h"

// Basit bir test örneği oluşturan yardımcı fonksiyon
ProblemInstance createTestInstance() {
    ProblemInstance instance;

    // Makineleri oluştur
    instance.machines["M1"] = std::make_unique<Machine>("M1");
    instance.machines["M2"] = std::make_unique<Machine>("M2");

    // Job1 oluştur: M1'de op0 (süre 5), M2'de op1 (süre 3)
    std::vector<Operation> job1Ops;
    job1Ops.emplace_back("J1", 0, "M1", 5);
    job1Ops.emplace_back("J1", 1, "M2", 3);
    instance.jobs["J1"] = std::make_unique<Job>("J1", std::move(job1Ops));

    // Job2 oluştur: M2'de op0 (süre 2), M1'de op1 (süre 4)
    std::vector<Operation> job2Ops;
    job2Ops.emplace_back("J2", 0, "M2", 2);
    job2Ops.emplace_back("J2", 1, "M1", 4);
    instance.jobs["J2"] = std::make_unique<Job>("J2", std::move(job2Ops));

    return instance;
}

void testValidSchedule() {
    std::cout << "Test 1: Valid Schedule\n";
    ProblemInstance instance = createTestInstance();
    Schedule schedule;

    // Makine M1: J1.op0, J2.op1
    schedule.machineOrder["M1"] = {OpKey{"J1", 0}, OpKey{"J2", 1}};
    
    // Makine M2: J2.op0, J1.op1
    schedule.machineOrder["M2"] = {OpKey{"J2", 0}, OpKey{"J1", 1}};

    // Çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");

    // Uygulanabilirliği kontrol et
    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(feasible && "Schedule should be feasible");

    // Makespan'ı kontrol et
    int makespan = MakespanCalculator::calculate(schedule);
    std::cout << "  Makespan: " << makespan << "\n";
    assert(makespan > 0 && "Makespan should be positive");

    // İş önceliğini doğrula: J1.op1, J1.op0 bittikten sonra başlamalı
    int j1_op0_end = schedule.opTimes.at("J1").at(0).end;
    int j1_op1_start = schedule.opTimes.at("J1").at(1).start;
    assert(j1_op1_start >= j1_op0_end && "J1.op1 must start after J1.op0 finishes");

    // Makine kısıtını doğrula: M1 işlemleri sıralı olmalı
    int j1_op0_end_m1 = schedule.opTimes.at("J1").at(0).end;
    int j2_op1_start_m1 = schedule.opTimes.at("J2").at(1).start;
    assert(j2_op1_start_m1 >= j1_op0_end_m1 && "M1 operations must be sequential");

    std::cout << "  ✓ Passed\n\n";
}

void testPrecedenceViolation() {
    std::cout << "Test 2: Precedence Violation Detection\n";
    ProblemInstance instance = createTestInstance();
    Schedule schedule;

    // J1.op1'in J1.op0 bitmeden başladığı bir çizelge oluştur
    // Bu, geçersiz zamanları manuel olarak ayarlayarak yapılır
    schedule.machineOrder["M1"] = {OpKey{"J1", 0}};
    schedule.machineOrder["M2"] = {OpKey{"J1", 1}};

    // Önce çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");

    // Şimdi manuel olarak bir öncelik ihlali oluştur
    schedule.opTimes["J1"][0] = TimeWindow{0, 5};
    schedule.opTimes["J1"][1] = TimeWindow{3, 6}; // 3'te başlıyor, ama op0 5'te bitiyor - İHLAL

    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(!feasible && "Schedule with precedence violation should be invalid");

    std::cout << "  ✓ Passed (correctly detected violation)\n\n";
}

void testMachineOverlap() {
    std::cout << "Test 3: Machine Overlap Detection\n";
    ProblemInstance instance = createTestInstance();
    Schedule schedule;

    // Aynı makinede çakışan işlemlerle bir çizelge oluştur
    schedule.machineOrder["M1"] = {OpKey{"J1", 0}, OpKey{"J2", 1}};
    schedule.machineOrder["M2"] = {OpKey{"J2", 0}, OpKey{"J1", 1}};

    // Önce çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");

    // Şimdi M1'de manuel olarak bir çakışma oluştur
    schedule.opTimes["J1"][0] = TimeWindow{0, 5};
    schedule.opTimes["J2"][1] = TimeWindow{3, 7}; // J1.op0 ile çakışıyor - İHLAL

    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(!feasible && "Schedule with machine overlap should be invalid");

    std::cout << "  ✓ Passed (correctly detected overlap)\n\n";
}

void testMakespanCalculation() {
    std::cout << "Test 4: Makespan Calculation\n";
    ProblemInstance instance = createTestInstance();
    Schedule schedule;

    schedule.machineOrder["M1"] = {OpKey{"J1", 0}, OpKey{"J2", 1}};
    schedule.machineOrder["M2"] = {OpKey{"J2", 0}, OpKey{"J1", 1}};

    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");

    int makespan = MakespanCalculator::calculate(schedule);
    
    // Makespan'ın maksimum bitiş zamanı olduğunu doğrula
    int maxEnd = -1;
    for (const auto& [jobId, opTimes] : schedule.opTimes) {
        for (const auto& [opIndex, timeWindow] : opTimes) {
            if (timeWindow.end > maxEnd) {
                maxEnd = timeWindow.end;
            }
        }
    }
    
    assert(makespan == maxEnd && "Makespan should equal maximum end time");
    std::cout << "  Makespan: " << makespan << " (max end time: " << maxEnd << ")\n";
    std::cout << "  ✓ Passed\n\n";
}

void testComplexSchedule() {
    std::cout << "Test 5: Complex Schedule\n";
    
    ProblemInstance instance;
    instance.machines["M1"] = std::make_unique<Machine>("M1");
    instance.machines["M2"] = std::make_unique<Machine>("M2");
    instance.machines["M3"] = std::make_unique<Machine>("M3");

    // Job1: M1(10) -> M2(5) -> M3(8)
    std::vector<Operation> job1Ops;
    job1Ops.emplace_back("J1", 0, "M1", 10);
    job1Ops.emplace_back("J1", 1, "M2", 5);
    job1Ops.emplace_back("J1", 2, "M3", 8);
    instance.jobs["J1"] = std::make_unique<Job>("J1", std::move(job1Ops));

    // Job2: M2(3) -> M1(7) -> M3(4)
    std::vector<Operation> job2Ops;
    job2Ops.emplace_back("J2", 0, "M2", 3);
    job2Ops.emplace_back("J2", 1, "M1", 7);
    job2Ops.emplace_back("J2", 2, "M3", 4);
    instance.jobs["J2"] = std::make_unique<Job>("J2", std::move(job2Ops));

    Schedule schedule;
    schedule.machineOrder["M1"] = {OpKey{"J1", 0}, OpKey{"J2", 1}};
    schedule.machineOrder["M2"] = {OpKey{"J2", 0}, OpKey{"J1", 1}};
    schedule.machineOrder["M3"] = {OpKey{"J1", 2}, OpKey{"J2", 2}};

    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");

    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(feasible && "Complex schedule should be feasible");

    int makespan = MakespanCalculator::calculate(schedule);
    std::cout << "  Makespan: " << makespan << "\n";
    assert(makespan > 0 && "Makespan should be positive");

    // Tüm işlemlerin çizelgelendiğini doğrula
    assert(schedule.opTimes["J1"].size() == 3 && "J1 should have 3 operations scheduled");
    assert(schedule.opTimes["J2"].size() == 3 && "J2 should have 3 operations scheduled");

    std::cout << "  ✓ Passed\n\n";
}

int main() {
    std::cout << "=== Schedule Decoding, Makespan, and Feasibility Tests ===\n\n";

    try {
        testValidSchedule();
        testPrecedenceViolation();
        testMachineOverlap();
        testMakespanCalculation();
        testComplexSchedule();

        std::cout << "=== All tests passed! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
}

