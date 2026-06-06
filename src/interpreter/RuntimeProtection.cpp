#include "interpreter/RuntimeProtection.hpp"

namespace interpreter {

RuntimeProtectionError::RuntimeProtectionError(const std::string& message)
    : std::runtime_error(message) {}

void RuntimeProtection::checkStackOverflow(
    std::size_t currentSize,
    std::size_t pushCount,
    std::size_t maxStackSize
) {
    if (pushCount > maxStackSize || currentSize > maxStackSize - pushCount) {
        throw RuntimeProtectionError(
            "StackOverflowError: operasi push akan membuat ukuran stack melebihi batas maksimum " +
            std::to_string(maxStackSize)
        );
    }
}

void RuntimeProtection::checkStackUnderflow(std::size_t currentSize, std::size_t required) {
    if (currentSize < required) {
        throw RuntimeProtectionError(
            "StackUnderflowError: operasi membutuhkan " + std::to_string(required) +
            " nilai, tetapi stack hanya berisi " + std::to_string(currentSize) + " nilai"
        );
    }
}

void RuntimeProtection::checkStackBalanced(
    std::size_t expectedSize,
    std::size_t actualSize,
    const std::string& context
) {
    if (expectedSize != actualSize) {
        throw RuntimeProtectionError(
            "StackCorruptionError: ukuran stack tidak seimbang pada " + context +
            ". Expected=" + std::to_string(expectedSize) +
            ", actual=" + std::to_string(actualSize)
        );
    }
}

void RuntimeProtection::checkMemoryAddress(
    int address,
    std::size_t memorySize,
    const std::string& operation
) {
    if (address < 0 || address >= static_cast<int>(memorySize)) {
        throw RuntimeProtectionError(
            "MemoryAccessError: instruksi " + operation +
            " mencoba mengakses address " + std::to_string(address) +
            ", tetapi ukuran memory hanya " + std::to_string(memorySize)
        );
    }
}

void RuntimeProtection::checkJumpTarget(int target, std::size_t codeSize) {
    if (target < 0 || target >= static_cast<int>(codeSize)) {
        throw RuntimeProtectionError(
            "InvalidJumpTargetError: target jump " + std::to_string(target) +
            " tidak ada pada intermediate code berukuran " + std::to_string(codeSize)
        );
    }
}

void RuntimeProtection::checkInstructionPointer(std::size_t ip, std::size_t codeSize) {
    if (ip >= codeSize) {
        throw RuntimeProtectionError(
            "InvalidInstructionPointerError: IP " + std::to_string(ip) +
            " berada di luar intermediate code berukuran " + std::to_string(codeSize)
        );
    }
}

void RuntimeProtection::checkArrayIndex(
    int index,
    int low,
    int high,
    const std::string& arrayName
) {
    if (index < low || index > high) {
        throw RuntimeProtectionError(
            "IndexOutOfBoundsException: indeks " + std::to_string(index) +
            " di luar batas array " + arrayName +
            " [" + std::to_string(low) + ".." + std::to_string(high) + "]"
        );
    }
}

void RuntimeProtection::checkCallDepth(std::size_t currentDepth, std::size_t maxDepth) {
    if (currentDepth >= maxDepth) {
        throw RuntimeProtectionError(
            "StackOverflowError: kedalaman call stack melebihi batas maksimum " +
            std::to_string(maxDepth)
        );
    }
}

int RuntimeProtection::safeNeg(int value) {
    if (value == INT_MIN_LIMIT) {
        throw RuntimeProtectionError("OverflowError: operasi NEG melewati batas integer");
    }
    return -value;
}

int RuntimeProtection::safeAdd(int left, int right) {
    return ensureIntegerRange(
        static_cast<long long>(left) + static_cast<long long>(right),
        "ADD"
    );
}

int RuntimeProtection::safeSub(int left, int right) {
    return ensureIntegerRange(
        static_cast<long long>(left) - static_cast<long long>(right),
        "SUB"
    );
}

int RuntimeProtection::safeMul(int left, int right) {
    return ensureIntegerRange(
        static_cast<long long>(left) * static_cast<long long>(right),
        "MUL"
    );
}

int RuntimeProtection::safeDiv(int left, int right) {
    if (right == 0) {
        throw RuntimeProtectionError("DivisionByZeroError: pembagian dengan nol tidak diperbolehkan");
    }
    if (left == INT_MIN_LIMIT && right == -1) {
        throw RuntimeProtectionError("OverflowError: operasi DIV melewati batas integer");
    }
    return left / right;
}

int RuntimeProtection::safeMod(int left, int right) {
    if (right == 0) {
        throw RuntimeProtectionError("ModuloByZeroError: modulus dengan nol tidak diperbolehkan");
    }
    return left % right;
}

int RuntimeProtection::ensureIntegerRange(long long value, const std::string& operationName) {
    if (value > INT_MAX_LIMIT) {
        throw RuntimeProtectionError("OverflowError: operasi " + operationName + " melewati batas maksimum integer");
    }
    if (value < INT_MIN_LIMIT) {
        throw RuntimeProtectionError("UnderflowError: operasi " + operationName + " melewati batas minimum integer");
    }
    return static_cast<int>(value);
}

} // namespace interpreter
