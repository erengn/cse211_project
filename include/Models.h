#pragma once

#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <memory>

// --------------------
// Operation key (stable identifier)
// --------------------
struct OpKey {
    std::string jobId;
    int opIndex = 0;
};

// --------------------
// Operation
// --------------------
class Operation {
private:
    std::string jobId_;
    int index_ = 0;
    std::string machineId_;
    int duration_ = 0;

public:
    Operation(std::string jobId, int index, std::string machineId, int duration)
        : jobId_(std::move(jobId)),
          index_(index),
          machineId_(std::move(machineId)),
          duration_(duration) {}

    const std::string& jobId() const { return jobId_; }
    int index() const { return index_; }
    const std::string& machineId() const { return machineId_; }
    int duration() const { return duration_; }

    // Precedence is job order: (jobId, index-1)
    std::optional<OpKey> predecessor() const {
        if (index_ == 0) return std::nullopt;
        return OpKey{jobId_, index_ - 1};
    }

    std::string toString() const {
        std::ostringstream os;
        os << "Op(" << jobId_ << "#" << index_
           << ", M=" << machineId_
           << ", dur=" << duration_ << ")";
        return os.str();
    }
};

// --------------------
// Job
// --------------------
class Job {
private:
    std::string id_;
    std::vector<Operation> ops_;

public:
    Job(std::string id, std::vector<Operation> ops)
        : id_(std::move(id)), ops_(std::move(ops)) {}

    const std::string& id() const { return id_; }
    const std::vector<Operation>& operations() const { return ops_; }

    int totalProcessingTime() const {
        int sum = 0;
        for (const auto& op : ops_) sum += op.duration();
        return sum;
    }

    std::string toString() const {
        std::ostringstream os;
        os << "Job(" << id_ << ", total=" << totalProcessingTime() << ", ops=[";
        for (size_t i = 0; i < ops_.size(); ++i) {
            if (i) os << ", ";
            os << ops_[i].toString();
        }
        os << "])";
        return os.str();
    }
};

// --------------------
// Machine
// --------------------
class Machine {
private:
    std::string id_;
    std::deque<OpKey> waiting_;

public:
    explicit Machine(std::string id) : id_(std::move(id)) {}

    const std::string& id() const { return id_; }

    std::deque<OpKey>& waitingQueue() { return waiting_; }
    const std::deque<OpKey>& waitingQueue() const { return waiting_; }

    std::string toString() const {
        std::ostringstream os;
        os << "Machine(" << id_ << ", waiting=" << waiting_.size() << ")";
        return os.str();
    }
};

// --------------------
// Schedule representation
// - machineOrder: solution representation (per-machine sequence)
// - opTimes: decode result (start/end times), can be filled later
// --------------------
struct TimeWindow {
    int start = 0;
    int end = 0;
};

class Schedule {
public:
    // machineId -> sequence of operations in that machine
    std::unordered_map<std::string, std::vector<OpKey>> machineOrder;

    // opTimes[jobId][opIndex] = (start,end)
    std::unordered_map<std::string, std::unordered_map<int, TimeWindow>> opTimes;

    std::string toString() const {
        std::ostringstream os;
        os << "Schedule:\n";
        for (const auto& [mid, seq] : machineOrder) {
            os << "  " << mid << ": ";
            for (size_t i = 0; i < seq.size(); ++i) {
                if (i) os << " -> ";
                os << seq[i].jobId << "#" << seq[i].opIndex;
            }
            os << "\n";
        }
        return os.str();
    }
};

// --------------------
// ProblemInstance: owns all data (safe for team edits)
// --------------------
struct ProblemInstance {
    // We store as pointers to avoid default-constructor/operator[] pitfalls.
    std::unordered_map<std::string, std::unique_ptr<Machine>> machines;
    std::unordered_map<std::string, std::unique_ptr<Job>> jobs;

    // Safe getters (return nullptr if not found)
    const Machine* getMachine(const std::string& id) const {
        auto it = machines.find(id);
        return (it == machines.end()) ? nullptr : it->second.get();
    }

    const Job* getJob(const std::string& id) const {
        auto it = jobs.find(id);
        return (it == jobs.end()) ? nullptr : it->second.get();
    }

    std::string toString() const {
        std::ostringstream os;
        os << "Machines:\n";
        for (const auto& [id, m] : machines) os << "  " << m->toString() << "\n";
        os << "Jobs:\n";
        for (const auto& [id, j] : jobs) os << "  " << j->toString() << "\n";
        return os.str();
    }
};
