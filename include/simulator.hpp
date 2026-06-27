#pragma once

#include <cstdint>
#include <string>

#include "cpu.hpp"
#include "instruction.hpp"
#include "memory.hpp"

enum class StepStatus {
    Running,
    Halted
};

class Simulator {
public:
    void loadProgramFromFile(const std::string& path);
    void reset();

    StepStatus step();

    [[nodiscard]] bool halted() const;
    [[nodiscard]] int exitCode() const;

    [[nodiscard]] CPU& cpu();
    [[nodiscard]] const CPU& cpu() const;
    [[nodiscard]] Memory& memory();
    [[nodiscard]] const Memory& memory() const;

private:
    void execute(const Instruction& instruction, std::uint32_t pc);
    void handleEcall();

    CPU cpu_;
    Memory memory_;
    bool halted_{false};
    int exitCode_{0};
};
