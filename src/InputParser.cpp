#include "InputParser.h"

#include <fstream>
#include <stdexcept>
#include <unordered_set>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

static void require(bool cond, const std::string& msg) {
    if (!cond) throw std::runtime_error("Input error: " + msg);
}

static bool whitespaceOnly(const std::string& s) {
    for (char c : s) {
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r')) return false;
    }
    return true;
}

ProblemInstance InputParser::parseFromJsonFile(const std::string& filePath) {
    std::ifstream in(filePath);
    require(in.good(), "Cannot open file: " + filePath);

    json j;
    in >> j;

    require(j.is_object(), "root must be a JSON object");
    require(j.contains("machines") && j["machines"].is_array(), "`machines` must be an array");
    require(j.contains("jobs") && j["jobs"].is_array(), "`jobs` must be an array");

    ProblemInstance inst;

    // --------------------
    // Machines
    // --------------------
    std::unordered_set<std::string> machineSet;

    for (const auto& m : j["machines"]) {
        require(m.is_string(), "each machine id must be a string");
        std::string mid = m.get<std::string>();

        require(!mid.empty() && !whitespaceOnly(mid), "machine id cannot be empty/whitespace");
        require(!machineSet.count(mid), "duplicate machine id: " + mid);

        machineSet.insert(mid);
        inst.machines.emplace(mid, std::make_unique<Machine>(mid));
    }

    require(!inst.machines.empty(), "machines list cannot be empty");

    // --------------------
    // Jobs
    // --------------------
    std::unordered_set<std::string> jobSet;

    for (const auto& job : j["jobs"]) {
        require(job.is_object(), "each job must be an object");
        require(job.contains("id") && job["id"].is_string(), "job missing string `id`");

        std::string jid = job["id"].get<std::string>();
        require(!jid.empty() && !whitespaceOnly(jid), "job id cannot be empty/whitespace");
        require(!jobSet.count(jid), "duplicate job id: " + jid);

        jobSet.insert(jid);

        require(job.contains("operations") && job["operations"].is_array(),
                "job " + jid + " missing `operations` array");

        const auto& opsJson = job["operations"];
        require(!opsJson.empty(), "job " + jid + " operations cannot be empty");

        std::vector<Operation> ops;
        ops.reserve(opsJson.size());

        for (size_t idx = 0; idx < opsJson.size(); ++idx) {
            const auto& op = opsJson[idx];
            require(op.is_object(), "job " + jid + " op " + std::to_string(idx) + " must be object");

            require(op.contains("machine") && op["machine"].is_string(),
                    "job " + jid + " op " + std::to_string(idx) + " missing string `machine`");
            require(op.contains("duration") && op["duration"].is_number_integer(),
                    "job " + jid + " op " + std::to_string(idx) + " missing int `duration`");

            std::string mid = op["machine"].get<std::string>();
            int dur = op["duration"].get<int>();

            require(!mid.empty() && !whitespaceOnly(mid),
                    "job " + jid + " op " + std::to_string(idx) + " machine cannot be empty/whitespace");
            require(machineSet.count(mid),
                    "unknown machine " + mid + " in job " + jid + " op " + std::to_string(idx));
            require(dur > 0,
                    "duration must be > 0 in job " + jid + " op " + std::to_string(idx));

            ops.emplace_back(jid, static_cast<int>(idx), mid, dur);
        }

        inst.jobs.emplace(jid, std::make_unique<Job>(jid, std::move(ops)));
    }

    require(!inst.jobs.empty(), "jobs list cannot be empty");
    return inst;
}
