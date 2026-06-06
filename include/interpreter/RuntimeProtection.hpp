#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <limits>

class RuntimeProtectionError : public std::runtime_error {
public:
    explicit RuntimeProtectionError(const std::string& message)
        : std::runtime_error(message) {}
};

class RuntimeProtection {
public:
    static constexpr int MAX_STACK_SIZE = 1000;
    static constexpr int INT_MIN_LIMIT = std::numeric_limits<int>::min();
    static constexpr int INT_MAX_LIMIT = std::numeric_limits<int>::max();

    static void checkStackOverflow(size_t currentSize, size_t pushCount = 1) {
        if (currentSize + pushCount > MAX_STACK_SIZE) {
            throw RuntimeProtectionError(
                "StackOverflowError: ukuran stack melebihi batas maksimum " +
                std::to_string(MAX_STACK_SIZE)
            );
        }
    }

    static void checkStackUnderflow(size_t currentSize, size_t required = 1) {
        if (currentSize < required) {
            throw RuntimeProtectionError(
                "StackUnderflowError: operasi membutuhkan " +
                std::to_string(required) + " nilai, tetapi stack hanya berisi " +
                std::to_string(currentSize)
            );
        }
    }

    static void checkMemoryAddress(int address, size_t memorySize, const std::string& operation) {
        if (address < 0 || address >= static_cast<int>(memorySize)) {
            throw RuntimeProtectionError(
                "MemoryAccessError: instruksi " + operation +
                " mencoba mengakses address " + std::to_string(address) +
                ", tetapi ukuran memori hanya " + std::to_string(memorySize)
            );
        }
    }

    static void checkJumpTarget(int target, size_t codeSize) {
        if (target < 0 || target >= static_cast<int>(codeSize)) {
            throw RuntimeProtectionError(
                "InvalidJumpTargetError: target jump " +
                std::to_string(target) + " tidak ada dalam intermediate code"
            );
        }
    }

    static void checkArrayIndex(int index, int low, int high, const std::string& arrayName) {
        if (index < low || index > high) {
            throw RuntimeProtectionError(
                "IndexOutOfBoundsException: indeks " + std::to_string(index) +
                " di luar batas array " + arrayName +
                " [" + std::to_string(low) + ".." + std::to_string(high) + "]"
            );
        }
    }

    static int safeAdd(int a, int b) {
        long long result = static_cast<long long>(a) + b;
        checkIntegerRange(result, "OverflowError/UnderflowError pada operasi ADD");
        return static_cast<int>(result);
    }

    static int safeSub(int a, int b) {
        long long result = static_cast<long long>(a) - b;
        checkIntegerRange(result, "OverflowError/UnderflowError pada operasi SUB");
        return static_cast<int>(result);
    }

    static int safeMul(int a, int b) {
        long long result = static_cast<long long>(a) * b;
        checkIntegerRange(result, "OverflowError/UnderflowError pada operasi MUL");
        return static_cast<int>(result);
    }

    static int safeDiv(int a, int b) {
        if (b == 0) {
            throw RuntimeProtectionError("DivisionByZeroError: pembagian dengan nol tidak diperbolehkan");
        }
        if (a == INT_MIN_LIMIT && b == -1) {
            throw RuntimeProtectionError("OverflowError: hasil pembagian melewati batas integer");
        }
        return a / b;
    }

    static int safeMod(int a, int b) {
        if (b == 0) {
            throw RuntimeProtectionError("ModuloByZeroError: modulus dengan nol tidak diperbolehkan");
        }
        return a % b;
    }

private:
    static void checkIntegerRange(long long value, const std::string& message) {
        if (value > INT_MAX_LIMIT || value < INT_MIN_LIMIT) {
            throw RuntimeProtectionError(message);
        }
    }
};
