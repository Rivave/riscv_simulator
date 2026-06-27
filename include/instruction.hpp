#pragma once

#include <cstdint>
#include <string>

enum class Mnemonic {
    Lb, Lh, Lw, Lbu, Lhu,
    Addi, Slli, Slti, Sltiu, Xori, Srli, Srai, Ori, Andi,
    Auipc,
    Sb, Sh, Sw,
    Add, Sub, Sll, Slt, Sltu, Xor, Srl, Sra, Or, And,
    Lui,
    Beq, Bne, Blt, Bge, Bltu, Bgeu,
    Jalr, Jal,
    Ecall, Ebreak,
    Invalid
};

struct Instruction {
    std::uint32_t raw{0};
    Mnemonic mnemonic{Mnemonic::Invalid};
    std::uint8_t rd{0};
    std::uint8_t rs1{0};
    std::uint8_t rs2{0};
    std::int32_t imm{0};
};

[[nodiscard]] Instruction decode(std::uint32_t raw);

[[nodiscard]] std::string disassemble(
    const Instruction& instruction,
    std::uint32_t pc
);
