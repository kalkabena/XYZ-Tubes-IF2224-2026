#pragma once

#include <string>
#include <vector>

namespace interpreter {

struct VulnerabilityTestResult {
    std::string name;
    bool passed;
    std::string message;
};

std::vector<VulnerabilityTestResult> runVulnerabilitySelfTests();
void writeVulnerabilityReport(const std::string& outputPath);

} // namespace interpreter
