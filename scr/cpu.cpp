#include "cpu.hpp"

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

std::uint32_t CPU::readRegister(std::size_t index) const {
    if (index >= registers_.size()) {
        throw std::out_of_range("Invalid RISC-V register index");
    }

    return registers_[index];
}

void CPU::writeRegister(std::size_t index, std::uint32_t value) {
    if (index >= registers_.size()) {
        throw std::out_of_range("Invalid RISC-V register index");
    }

    // El registro x0 de RISC-V siempre debe valer cero.
    if (index == 0) {
        return;
    }

    registers_[index] = value;
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