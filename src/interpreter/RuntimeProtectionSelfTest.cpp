#include "interpreter/RuntimeProtectionSelfTest.hpp"
#include "interpreter/SafeStackMachine.hpp"
#include "interpreter/RuntimeProtection.hpp"

#include <fstream>
#include <limits>
#include <sstream>

namespace interpreter {

static VulnerabilityTestResult expectRuntimeError(
    const std::string& name,
    const std::vector<Instruction>& program,
    const std::string& expectedKeyword
) {
    try {
        std::ostringstream ignoredOutput;
        SafeStackMachine machine(program);
        machine.setOutputStream(ignoredOutput);
        machine.run();
        return {name, false, "Expected error containing '" + expectedKeyword + "', tetapi program selesai tanpa error"};
    } catch (const RuntimeProtectionError& error) {
        std::string message = error.what();
        bool matched = message.find(expectedKeyword) != std::string::npos;
        return {name, matched, message};
    } catch (const std::exception& error) {
        return {name, false, std::string("Error bukan RuntimeProtectionError: ") + error.what()};
    }
}

static VulnerabilityTestResult expectOutput(
    const std::string& name,
    const std::vector<Instruction>& program,
    const std::string& expectedOutput
) {
    try {
        std::ostringstream output;
        SafeStackMachine machine(program);
        machine.setOutputStream(output);
        machine.run();
        bool matched = output.str() == expectedOutput;
        return {name, matched, "Output='" + output.str() + "', expected='" + expectedOutput + "'"};
    } catch (const std::exception& error) {
        return {name, false, std::string("Program valid menghasilkan error: ") + error.what()};
    }
}

std::vector<VulnerabilityTestResult> runVulnerabilitySelfTests() {
    using I = Instruction;
    std::vector<VulnerabilityTestResult> results;

    results.push_back(expectRuntimeError(
        "TC-01 Stack underflow pada OPR ADD",
        { I(OpCode::OPR, 0, OPR_ADD) },
        "StackUnderflowError"
    ));

    results.push_back(expectRuntimeError(
        "TC-02 Invalid jump target",
        { I(OpCode::JMP, 0, 99) },
        "InvalidJumpTargetError"
    ));

    results.push_back(expectRuntimeError(
        "TC-03 Memory out-of-bounds pada LOD",
        { I(OpCode::INT, 0, 5), I(OpCode::LOD, 0, 10), I(OpCode::RET, 0, 0) },
        "MemoryAccessError"
    ));

    results.push_back(expectRuntimeError(
        "TC-04 Numerical overflow pada ADD",
        {
            I(OpCode::INT, 0, 0),
            I(OpCode::LIT, 0, std::numeric_limits<int>::max()),
            I(OpCode::LIT, 0, 1),
            I(OpCode::OPR, 0, OPR_ADD),
            I(OpCode::RET, 0, 0)
        },
        "OverflowError"
    ));

    results.push_back(expectRuntimeError(
        "TC-05 Division by zero",
        {
            I(OpCode::LIT, 0, 10),
            I(OpCode::LIT, 0, 0),
            I(OpCode::OPR, 0, OPR_DIV),
            I(OpCode::RET, 0, 0)
        },
        "DivisionByZeroError"
    ));

    results.push_back(expectRuntimeError(
        "TC-06 Array index out-of-bounds helper",
        {},
        "__manual__"
    ));

    try {
        RuntimeProtection::checkArrayIndex(10, 1, 5, "numbers");
        results.back() = {"TC-06 Array index out-of-bounds helper", false, "Expected IndexOutOfBoundsException, tetapi tidak ada error"};
    } catch (const RuntimeProtectionError& error) {
        std::string message = error.what();
        results.back() = {"TC-06 Array index out-of-bounds helper", message.find("IndexOutOfBoundsException") != std::string::npos, message};
    }

    results.push_back(expectOutput(
        "TC-07 Program valid: y := 10 + 5; writeln(y)",
        {
            I(OpCode::INT, 0, 5),
            I(OpCode::LIT, 0, 10),
            I(OpCode::LIT, 0, 5),
            I(OpCode::OPR, 0, OPR_ADD),
            I(OpCode::STO, 0, 3),
            I(OpCode::LOD, 0, 3),
            I(OpCode::OPR, 0, OPR_WRTLN),
            I(OpCode::RET, 0, 0)
        },
        "15\n"
    ));

    return results;
}

void writeVulnerabilityReport(const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        throw RuntimeProtectionError("FileError: gagal membuka file output vulnerability test: " + outputPath);
    }

    auto results = runVulnerabilitySelfTests();
    int passed = 0;

    out << "--- Milestone 4 Vulnerability Protection Test ---\n";
    for (const auto& result : results) {
        if (result.passed) ++passed;
        out << (result.passed ? "[PASS] " : "[FAIL] ")
            << result.name << " -> " << result.message << "\n";
    }

    out << "\nSummary: " << passed << "/" << results.size() << " test passed\n";
}

} // namespace interpreter
