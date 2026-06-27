#include "cpu.hpp"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <stdexcept>

CPU::CPU() {
    reset();
}

void CPU::reset() {
    pc_ = 0;
    registers_.fill(0);
}

std::uint32_t CPU::pc() const {
    return pc_;
}

void CPU::setPc(std::uint32_t value) {
    pc_ = value;
}

std::uint32_t CPU::readRegister(std::size_t index) const {
    if (index >= registers_.size()) {
        throw std::out_of_range("Índice de registro RISC-V inválido");
    }

    return registers_[index];
}

void CPU::writeRegister(std::size_t index, std::uint32_t value) {
    if (index >= registers_.size()) {
        throw std::out_of_range("Índice de registro RISC-V inválido");
    }

    if (index == 0) {
        return;
    }

    registers_[index] = value;
}

std::uint64_t CPU::fingerprint() const {
    std::uint64_t hash = 1469598103934665603ull;

    auto mix = [&hash](std::uint32_t value) {
        for (int shift = 0; shift < 32; shift += 8) {
            hash ^= static_cast<std::uint8_t>(value >> shift);
            hash *= 1099511628211ull;
        }
    };

    mix(pc_);
    for (std::uint32_t value : registers_) {
        mix(value);
    }

    return hash;
}

void CPU::dumpState(std::ostream& output) const {
    const auto previousFlags = output.flags();
    const auto previousFill = output.fill();

    output << "PC = 0x"
           << std::hex
           << std::uppercase
           << std::setw(8)
           << std::setfill('0')
           << pc_
           << '\n';

    for (std::size_t i = 0; i < registers_.size(); ++i) {
        output << "x"
               << std::dec
               << std::setw(2)
               << std::setfill(' ')
               << i
               << " = 0x"
               << std::hex
               << std::uppercase
               << std::setw(8)
               << std::setfill('0')
               << registers_[i];

        if ((i + 1) % 4 == 0) {
            output << '\n';
        } else {
            output << "    ";
        }
    }

    output.flags(previousFlags);
    output.fill(previousFill);
}
