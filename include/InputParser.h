
#pragma once
#include <string>
#include "Models.h"

class InputParser {
public:
    // Throws std::runtime_error on invalid input.
    static ProblemInstance parseFromJsonFile(const std::string& filePath);
};
