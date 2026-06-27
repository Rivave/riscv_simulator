#include "simulator.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

constexpr std::size_t kRegisterA0 = 10;
constexpr std::size_t kRegisterA1 = 11;
constexpr std::size_t kRegisterA7 = 17;

std::uint32_t alignDown(std::uint32_t value) {
    return value & ~static_cast<std::uint32_t>(1);
}

}

void Simulator::loadProgramFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("No se pudo abrir el archivo: " + path);
    }

    const std::istreambuf_iterator<char> begin(file);
    const std::istreambuf_iterator<char> end;
    const std::vector<char> raw(begin, end);

    std::vector<std::uint8_t> program(raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        program[i] = static_cast<std::uint8_t>(raw[i]);
    }

    memory_.clear();
    memory_.loadProgram(0x00000000u, program);

    cpu_.reset();
    halted_ = false;
    exitCode_ = 0;
}

void Simulator::reset() {
    cpu_.reset();
    halted_ = false;
    exitCode_ = 0;
}

void Simulator::halt(int exitCode) {
    halted_ = true;
    exitCode_ = exitCode;
}

bool Simulator::halted() const {
    return halted_;
}

int Simulator::exitCode() const {
    return exitCode_;
}

CPU& Simulator::cpu() {
    return cpu_;
}

const CPU& Simulator::cpu() const {
    return cpu_;
}

Memory& Simulator::memory() {
    return memory_;
}

const Memory& Simulator::memory() const {
    return memory_;
}

std::uint64_t Simulator::stateFingerprint() const {
    std::uint64_t hash = cpu_.fingerprint();
    hash ^= memory_.fingerprint() + 0x9E3779B97F4A7C15ull + (hash << 6) + (hash >> 2);
    return hash;
}

StepStatus Simulator::step() {
    if (halted_) {
        return StepStatus::Halted;
    }

    const std::uint32_t pc = cpu_.pc();
    const std::uint32_t raw = memory_.read32(pc);
    const Instruction instruction = decode(raw);

    if (instruction.mnemonic == Mnemonic::Invalid) {
        std::ostringstream message;
        message << "Instrucción inválida en 0x" << std::hex << std::uppercase
                << pc << " (0x" << raw << ")";
        throw std::runtime_error(message.str());
    }

    execute(instruction, pc);

    return halted_ ? StepStatus::Halted : StepStatus::Running;
}

void Simulator::execute(const Instruction& instruction, std::uint32_t pc) {
    const std::uint32_t rs1 = cpu_.readRegister(instruction.rs1);
    const std::uint32_t rs2 = cpu_.readRegister(instruction.rs2);
    const std::int32_t srs1 = static_cast<std::int32_t>(rs1);
    const std::int32_t srs2 = static_cast<std::int32_t>(rs2);
    const std::uint32_t uimm = static_cast<std::uint32_t>(instruction.imm);
    const std::uint32_t shamt = static_cast<std::uint32_t>(instruction.imm) & 0x1Fu;
    const std::uint32_t address = rs1 + uimm;

    std::uint32_t nextPc = pc + 4;

    switch (instruction.mnemonic) {
        case Mnemonic::Lb:
            cpu_.writeRegister(
                instruction.rd,
                static_cast<std::uint32_t>(
                    static_cast<std::int32_t>(
                        static_cast<std::int8_t>(memory_.read8(address))
                    )
                )
            );
            break;
        case Mnemonic::Lh:
            cpu_.writeRegister(
                instruction.rd,
                static_cast<std::uint32_t>(
                    static_cast<std::int32_t>(
                        static_cast<std::int16_t>(memory_.read16(address))
                    )
                )
            );
            break;
        case Mnemonic::Lw:
            cpu_.writeRegister(instruction.rd, memory_.read32(address));
            break;
        case Mnemonic::Lbu:
            cpu_.writeRegister(instruction.rd, memory_.read8(address));
            break;
        case Mnemonic::Lhu:
            cpu_.writeRegister(instruction.rd, memory_.read16(address));
            break;

        case Mnemonic::Addi:
            cpu_.writeRegister(instruction.rd, rs1 + uimm);
            break;
        case Mnemonic::Slti:
            cpu_.writeRegister(instruction.rd, srs1 < instruction.imm ? 1u : 0u);
            break;
        case Mnemonic::Sltiu:
            cpu_.writeRegister(instruction.rd, rs1 < uimm ? 1u : 0u);
            break;
        case Mnemonic::Xori:
            cpu_.writeRegister(instruction.rd, rs1 ^ uimm);
            break;
        case Mnemonic::Ori:
            cpu_.writeRegister(instruction.rd, rs1 | uimm);
            break;
        case Mnemonic::Andi:
            cpu_.writeRegister(instruction.rd, rs1 & uimm);
            break;
        case Mnemonic::Slli:
            cpu_.writeRegister(instruction.rd, rs1 << shamt);
            break;
        case Mnemonic::Srli:
            cpu_.writeRegister(instruction.rd, rs1 >> shamt);
            break;
        case Mnemonic::Srai:
            cpu_.writeRegister(
                instruction.rd,
                static_cast<std::uint32_t>(srs1 >> shamt)
            );
            break;

        case Mnemonic::Auipc:
            cpu_.writeRegister(instruction.rd, pc + uimm);
            break;
        case Mnemonic::Lui:
            cpu_.writeRegister(instruction.rd, uimm);
            break;

        case Mnemonic::Sb:
            memory_.write8(
                address,
                static_cast<std::uint8_t>(rs2 & 0xFFu)
            );
            break;
        case Mnemonic::Sh:
            memory_.write16(
                address,
                static_cast<std::uint16_t>(rs2 & 0xFFFFu)
            );
            break;
        case Mnemonic::Sw:
            memory_.write32(address, rs2);
            break;

        case Mnemonic::Add:
            cpu_.writeRegister(instruction.rd, rs1 + rs2);
            break;
        case Mnemonic::Sub:
            cpu_.writeRegister(instruction.rd, rs1 - rs2);
            break;
        case Mnemonic::Sll:
            cpu_.writeRegister(instruction.rd, rs1 << (rs2 & 0x1Fu));
            break;
        case Mnemonic::Slt:
            cpu_.writeRegister(instruction.rd, srs1 < srs2 ? 1u : 0u);
            break;
        case Mnemonic::Sltu:
            cpu_.writeRegister(instruction.rd, rs1 < rs2 ? 1u : 0u);
            break;
        case Mnemonic::Xor:
            cpu_.writeRegister(instruction.rd, rs1 ^ rs2);
            break;
        case Mnemonic::Srl:
            cpu_.writeRegister(instruction.rd, rs1 >> (rs2 & 0x1Fu));
            break;
        case Mnemonic::Sra:
            cpu_.writeRegister(
                instruction.rd,
                static_cast<std::uint32_t>(srs1 >> (rs2 & 0x1Fu))
            );
            break;
        case Mnemonic::Or:
            cpu_.writeRegister(instruction.rd, rs1 | rs2);
            break;
        case Mnemonic::And:
            cpu_.writeRegister(instruction.rd, rs1 & rs2);
            break;

        case Mnemonic::Beq:
            if (rs1 == rs2) nextPc = pc + uimm;
            break;
        case Mnemonic::Bne:
            if (rs1 != rs2) nextPc = pc + uimm;
            break;
        case Mnemonic::Blt:
            if (srs1 < srs2) nextPc = pc + uimm;
            break;
        case Mnemonic::Bge:
            if (srs1 >= srs2) nextPc = pc + uimm;
            break;
        case Mnemonic::Bltu:
            if (rs1 < rs2) nextPc = pc + uimm;
            break;
        case Mnemonic::Bgeu:
            if (rs1 >= rs2) nextPc = pc + uimm;
            break;

        case Mnemonic::Jal:
            cpu_.writeRegister(instruction.rd, pc + 4);
            nextPc = pc + uimm;
            break;
        case Mnemonic::Jalr:
            cpu_.writeRegister(instruction.rd, pc + 4);
            nextPc = alignDown(rs1 + uimm);
            break;

        case Mnemonic::Ecall:
            handleEcall();
            break;
        case Mnemonic::Ebreak:
            halted_ = true;
            break;

        case Mnemonic::Invalid:
            break;
    }

    cpu_.setPc(nextPc);
}

void Simulator::handleEcall() {
    const std::uint32_t which = cpu_.readRegister(kRegisterA7);

    switch (which) {
        case 1:
            std::cout << static_cast<std::int32_t>(
                cpu_.readRegister(kRegisterA0)
            );
            std::cout.flush();
            break;
        case 4: {
            std::uint32_t address = cpu_.readRegister(kRegisterA0);
            for (;;) {
                const std::uint8_t character = memory_.read8(address);
                if (character == 0) {
                    break;
                }
                std::cout << static_cast<char>(character);
                ++address;
            }
            std::cout.flush();
            break;
        }
        case 5: {
            std::int32_t value = 0;
            std::cin >> value;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cpu_.writeRegister(
                kRegisterA0,
                static_cast<std::uint32_t>(value)
            );
            break;
        }
        case 8: {
            const std::uint32_t address = cpu_.readRegister(kRegisterA0);
            const std::int32_t maxLength =
                static_cast<std::int32_t>(cpu_.readRegister(kRegisterA1));

            std::string line;
            std::getline(std::cin, line);

            std::int32_t index = 0;
            for (; index < maxLength - 1 &&
                   index < static_cast<std::int32_t>(line.size());
                 ++index) {
                memory_.write8(
                    address + static_cast<std::uint32_t>(index),
                    static_cast<std::uint8_t>(line[index])
                );
            }
            if (maxLength > 0) {
                memory_.write8(
                    address + static_cast<std::uint32_t>(index),
                    0
                );
            }
            break;
        }
        case 11:
            std::cout << static_cast<char>(
                cpu_.readRegister(kRegisterA0) & 0xFFu
            );
            std::cout.flush();
            break;
        case 12: {
            const int character = std::cin.get();
            cpu_.writeRegister(
                kRegisterA0,
                static_cast<std::uint32_t>(character)
            );
            break;
        }
        case 10:
            halt(0);
            break;
        case 17:
            halt(static_cast<std::int32_t>(
                cpu_.readRegister(kRegisterA0)
            ));
            break;
        default:
            throw std::runtime_error(
                "Número de syscall no soportado: " + std::to_string(which)
            );
    }
}
