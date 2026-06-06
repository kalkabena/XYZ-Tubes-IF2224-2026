#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <limits>
#include <cstddef>

namespace interpreter {

class RuntimeProtectionError : public std::runtime_error {
public:
    explicit RuntimeProtectionError(const std::string& message);
};

class RuntimeProtection {
public:
    static constexpr int MAX_STACK_SIZE = 1000;
    static constexpr int INT_MIN_LIMIT = std::numeric_limits<int>::min();
    static constexpr int INT_MAX_LIMIT = std::numeric_limits<int>::max();

    static void checkStackOverflow(std::size_t currentSize, std::size_t pushCount = 1, std::size_t maxStackSize = MAX_STACK_SIZE);
    static void checkStackUnderflow(std::size_t currentSize, std::size_t required = 1);
    static void checkStackBalanced(std::size_t expectedSize, std::size_t actualSize, const std::string& context);
    
    static void checkMemoryAddress(int address, std::size_t memorySize, const std::string& operation);
    
    static void checkJumpTarget(int target, std::size_t codeSize);
    static void checkInstructionPointer(std::size_t ip, std::size_t codeSize);
    
    static void checkArrayIndex(int index, int low, int high, const std::string& arrayName);
    
    static void checkCallDepth(std::size_t currentDepth, std::size_t maxDepth = 100);

    static int safeNeg(int value);
    static int safeAdd(int left, int right);
    static int safeSub(int left, int right);
    static int safeMul(int left, int right);
    static int safeDiv(int left, int right);
    static int safeMod(int left, int right);

private:
    static int ensureIntegerRange(long long value, const std::string& operationName);
};

} // namespace interpreter
