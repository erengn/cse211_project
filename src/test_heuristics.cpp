#include <iostream>
#include <cassert>
#include "Heuristics.h"
#include "LocalSearch.h"
#include "InputParser.h"
#include "ScheduleDecoder.h"
#include "MakespanCalculator.h"
#include "FeasibilityChecker.h"

// Basit bir test örneği oluştur
ProblemInstance createTestInstance() {
    ProblemInstance instance;

    // Makineleri oluştur
    instance.machines["M1"] = std::make_unique<Machine>("M1");
    instance.machines["M2"] = std::make_unique<Machine>("M2");
    instance.machines["M3"] = std::make_unique<Machine>("M3");

    // Job1: M1(10) -> M2(5) -> M3(8) - toplam 23
    std::vector<Operation> job1Ops;
    job1Ops.emplace_back("J1", 0, "M1", 10);
    job1Ops.emplace_back("J1", 1, "M2", 5);
    job1Ops.emplace_back("J1", 2, "M3", 8);
    instance.jobs["J1"] = std::make_unique<Job>("J1", std::move(job1Ops));

    // Job2: M2(3) -> M1(7) -> M3(4) - toplam 14
    std::vector<Operation> job2Ops;
    job2Ops.emplace_back("J2", 0, "M2", 3);
    job2Ops.emplace_back("J2", 1, "M1", 7);
    job2Ops.emplace_back("J2", 2, "M3", 4);
    instance.jobs["J2"] = std::make_unique<Job>("J2", std::move(job2Ops));

    // Job3: M3(2) -> M2(6) -> M1(9) - toplam 17
    std::vector<Operation> job3Ops;
    job3Ops.emplace_back("J3", 0, "M3", 2);
    job3Ops.emplace_back("J3", 1, "M2", 6);
    job3Ops.emplace_back("J3", 2, "M1", 9);
    instance.jobs["J3"] = std::make_unique<Job>("J3", std::move(job3Ops));

    return instance;
}

void testSPT() {
    std::cout << "Test 1: SPT (Shortest Processing Time) Heuristic\n";
    ProblemInstance instance = createTestInstance();
    
    DispatchHeuristics heuristics(instance);
    Schedule schedule = heuristics.buildSPTSchedule();
    
    // Çizelgeyi çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");
    
    // Uygulanabilirliği kontrol et
    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(feasible && "SPT schedule should be feasible");
    
    // Makespan'ı hesapla
    int makespan = MakespanCalculator::calculate(schedule);
    std::cout << "  SPT Makespan: " << makespan << "\n";
    assert(makespan > 0 && "Makespan should be positive");
    
    std::cout << "  ✓ Passed\n\n";
}

void testLJF() {
    std::cout << "Test 2: LJF (Longest Job First) Heuristic\n";
    ProblemInstance instance = createTestInstance();
    
    DispatchHeuristics heuristics(instance);
    Schedule schedule = heuristics.buildLJFSchedule();
    
    // Çizelgeyi çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");
    
    // Uygulanabilirliği kontrol et
    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(feasible && "LJF schedule should be feasible");
    
    // Makespan'ı hesapla
    int makespan = MakespanCalculator::calculate(schedule);
    std::cout << "  LJF Makespan: " << makespan << "\n";
    assert(makespan > 0 && "Makespan should be positive");
    
    std::cout << "  ✓ Passed\n\n";
}

void testCriticalPath() {
    std::cout << "Test 3: Critical Path Priority Heuristic\n";
    ProblemInstance instance = createTestInstance();
    
    DispatchHeuristics heuristics(instance);
    Schedule schedule = heuristics.buildCriticalPathSchedule();
    
    // Çizelgeyi çöz
    bool decoded = ScheduleDecoder::decode(schedule, instance);
    assert(decoded && "Decoding should succeed");
    
    // Uygulanabilirliği kontrol et
    bool feasible = FeasibilityChecker::isValid(schedule, instance);
    assert(feasible && "Critical Path schedule should be feasible");
    
    // Makespan'ı hesapla
    int makespan = MakespanCalculator::calculate(schedule);
    std::cout << "  Critical Path Makespan: " << makespan << "\n";
    assert(makespan > 0 && "Makespan should be positive");
    
    std::cout << "  ✓ Passed\n\n";
}

void testLocalSearch() {
    std::cout << "Test 4: Local Search Improvement\n";
    ProblemInstance instance = createTestInstance();
    
    // Önce bir SPT çizelgesi oluştur
    DispatchHeuristics heuristics(instance);
    Schedule initialSchedule = heuristics.buildSPTSchedule();
    
    bool decoded = ScheduleDecoder::decode(initialSchedule, instance);
    assert(decoded && "Initial schedule decoding should succeed");
    
    int initialMakespan = MakespanCalculator::calculate(initialSchedule);
    std::cout << "  Initial Makespan: " << initialMakespan << "\n";
    
    // Yerel arama ile iyileştir
    LocalSearch localSearch(instance);
    auto [improvedSchedule, improvedMakespan] = localSearch.improveSchedule(initialSchedule, 50);
    
    std::cout << "  Improved Makespan: " << improvedMakespan << "\n";
    
    // İyileştirilmiş çizelge uygulanabilir olmalı
    bool feasible = FeasibilityChecker::isValid(improvedSchedule, instance);
    assert(feasible && "Improved schedule should be feasible");
    
    // Makespan iyileşmiş veya aynı olmalı (daha kötü olmamalı)
    assert(improvedMakespan <= initialMakespan && "Makespan should not worsen");
    
    std::cout << "  ✓ Passed\n\n";
}

void testAllHeuristicsComparison() {
    std::cout << "Test 5: Compare All Heuristics\n";
    ProblemInstance instance = createTestInstance();
    
    DispatchHeuristics heuristics(instance);
    
    // SPT
    Schedule sptSchedule = heuristics.buildSPTSchedule();
    ScheduleDecoder::decode(sptSchedule, instance);
    int sptMakespan = MakespanCalculator::calculate(sptSchedule);
    
    // LJF
    Schedule ljfSchedule = heuristics.buildLJFSchedule();
    ScheduleDecoder::decode(ljfSchedule, instance);
    int ljfMakespan = MakespanCalculator::calculate(ljfSchedule);
    
    // Critical Path
    Schedule cpSchedule = heuristics.buildCriticalPathSchedule();
    ScheduleDecoder::decode(cpSchedule, instance);
    int cpMakespan = MakespanCalculator::calculate(cpSchedule);
    
    std::cout << "  SPT Makespan: " << sptMakespan << "\n";
    std::cout << "  LJF Makespan: " << ljfMakespan << "\n";
    std::cout << "  Critical Path Makespan: " << cpMakespan << "\n";
    
    // Tümü pozitif olmalı
    assert(sptMakespan > 0 && ljfMakespan > 0 && cpMakespan > 0);
    
    std::cout << "  ✓ Passed\n\n";
}

int main() {
    std::cout << "=== Heuristics and Local Search Tests ===\n\n";

    try {
        testSPT();
        testLJF();
        testCriticalPath();
        testLocalSearch();
        testAllHeuristicsComparison();

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

