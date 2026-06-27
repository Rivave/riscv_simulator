#include "instruction.hpp"

#include <array>
#include <sstream>

namespace {

std::int32_t signExtend(std::uint32_t value, int bits) {
    const std::uint32_t mask = 1u << (bits - 1);
    return static_cast<std::int32_t>((value ^ mask) - mask);
}

std::int32_t immI(std::uint32_t raw) {
    return signExtend((raw >> 20) & 0xFFFu, 12);
}

std::int32_t immS(std::uint32_t raw) {
    const std::uint32_t value =
        (((raw >> 25) & 0x7Fu) << 5) | ((raw >> 7) & 0x1Fu);
    return signExtend(value, 12);
}

std::int32_t immB(std::uint32_t raw) {
    const std::uint32_t value =
        (((raw >> 31) & 0x1u) << 12) |
        (((raw >> 7) & 0x1u) << 11) |
        (((raw >> 25) & 0x3Fu) << 5) |
        (((raw >> 8) & 0xFu) << 1);
    return signExtend(value, 13);
}

std::int32_t immU(std::uint32_t raw) {
    return static_cast<std::int32_t>(raw & 0xFFFFF000u);
}

std::int32_t immJ(std::uint32_t raw) {
    const std::uint32_t value =
        (((raw >> 31) & 0x1u) << 20) |
        (((raw >> 12) & 0xFFu) << 12) |
        (((raw >> 20) & 0x1u) << 11) |
        (((raw >> 21) & 0x3FFu) << 1);
    return signExtend(value, 21);
}

const char* mnemonicName(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::Lb: return "lb";
        case Mnemonic::Lh: return "lh";
        case Mnemonic::Lw: return "lw";
        case Mnemonic::Lbu: return "lbu";
        case Mnemonic::Lhu: return "lhu";
        case Mnemonic::Addi: return "addi";
        case Mnemonic::Slli: return "slli";
        case Mnemonic::Slti: return "slti";
        case Mnemonic::Sltiu: return "sltiu";
        case Mnemonic::Xori: return "xori";
        case Mnemonic::Srli: return "srli";
        case Mnemonic::Srai: return "srai";
        case Mnemonic::Ori: return "ori";
        case Mnemonic::Andi: return "andi";
        case Mnemonic::Auipc: return "auipc";
        case Mnemonic::Sb: return "sb";
        case Mnemonic::Sh: return "sh";
        case Mnemonic::Sw: return "sw";
        case Mnemonic::Add: return "add";
        case Mnemonic::Sub: return "sub";
        case Mnemonic::Sll: return "sll";
        case Mnemonic::Slt: return "slt";
        case Mnemonic::Sltu: return "sltu";
        case Mnemonic::Xor: return "xor";
        case Mnemonic::Srl: return "srl";
        case Mnemonic::Sra: return "sra";
        case Mnemonic::Or: return "or";
        case Mnemonic::And: return "and";
        case Mnemonic::Lui: return "lui";
        case Mnemonic::Beq: return "beq";
        case Mnemonic::Bne: return "bne";
        case Mnemonic::Blt: return "blt";
        case Mnemonic::Bge: return "bge";
        case Mnemonic::Bltu: return "bltu";
        case Mnemonic::Bgeu: return "bgeu";
        case Mnemonic::Jalr: return "jalr";
        case Mnemonic::Jal: return "jal";
        case Mnemonic::Ecall: return "ecall";
        case Mnemonic::Ebreak: return "ebreak";
        case Mnemonic::Invalid: return "invalid";
    }
    return "invalid";
}

std::string reg(std::uint8_t index) {
    return "x" + std::to_string(static_cast<int>(index));
}

std::string hex(std::uint32_t value) {
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << value;
    return stream.str();
}

bool isLoad(Mnemonic m) {
    return m == Mnemonic::Lb || m == Mnemonic::Lh || m == Mnemonic::Lw ||
           m == Mnemonic::Lbu || m == Mnemonic::Lhu;
}

bool isStore(Mnemonic m) {
    return m == Mnemonic::Sb || m == Mnemonic::Sh || m == Mnemonic::Sw;
}

bool isBranch(Mnemonic m) {
    return m == Mnemonic::Beq || m == Mnemonic::Bne || m == Mnemonic::Blt ||
           m == Mnemonic::Bge || m == Mnemonic::Bltu || m == Mnemonic::Bgeu;
}

bool isImmAlu(Mnemonic m) {
    return m == Mnemonic::Addi || m == Mnemonic::Slti ||
           m == Mnemonic::Sltiu || m == Mnemonic::Xori ||
           m == Mnemonic::Ori || m == Mnemonic::Andi;
}

bool isShiftImm(Mnemonic m) {
    return m == Mnemonic::Slli || m == Mnemonic::Srli || m == Mnemonic::Srai;
}

bool isRType(Mnemonic m) {
    return m == Mnemonic::Add || m == Mnemonic::Sub || m == Mnemonic::Sll ||
           m == Mnemonic::Slt || m == Mnemonic::Sltu || m == Mnemonic::Xor ||
           m == Mnemonic::Srl || m == Mnemonic::Sra || m == Mnemonic::Or ||
           m == Mnemonic::And;
}

}

Instruction decode(std::uint32_t raw) {
    Instruction instruction;
    instruction.raw = raw;

    const std::uint32_t opcode = raw & 0x7Fu;
    const std::uint8_t rd = static_cast<std::uint8_t>((raw >> 7) & 0x1Fu);
    const std::uint32_t funct3 = (raw >> 12) & 0x7u;
    const std::uint8_t rs1 = static_cast<std::uint8_t>((raw >> 15) & 0x1Fu);
    const std::uint8_t rs2 = static_cast<std::uint8_t>((raw >> 20) & 0x1Fu);
    const std::uint32_t funct7 = (raw >> 25) & 0x7Fu;

    instruction.rd = rd;
    instruction.rs1 = rs1;
    instruction.rs2 = rs2;

    switch (opcode) {
        case 0x03u:
            instruction.imm = immI(raw);
            switch (funct3) {
                case 0: instruction.mnemonic = Mnemonic::Lb; break;
                case 1: instruction.mnemonic = Mnemonic::Lh; break;
                case 2: instruction.mnemonic = Mnemonic::Lw; break;
                case 4: instruction.mnemonic = Mnemonic::Lbu; break;
                case 5: instruction.mnemonic = Mnemonic::Lhu; break;
                default: break;
            }
            break;

        case 0x13u:
            switch (funct3) {
                case 0: instruction.mnemonic = Mnemonic::Addi; instruction.imm = immI(raw); break;
                case 2: instruction.mnemonic = Mnemonic::Slti; instruction.imm = immI(raw); break;
                case 3: instruction.mnemonic = Mnemonic::Sltiu; instruction.imm = immI(raw); break;
                case 4: instruction.mnemonic = Mnemonic::Xori; instruction.imm = immI(raw); break;
                case 6: instruction.mnemonic = Mnemonic::Ori; instruction.imm = immI(raw); break;
                case 7: instruction.mnemonic = Mnemonic::Andi; instruction.imm = immI(raw); break;
                case 1:
                    if (funct7 == 0x00u) {
                        instruction.mnemonic = Mnemonic::Slli;
                        instruction.imm = static_cast<std::int32_t>(rs2);
                    }
                    break;
                case 5:
                    instruction.imm = static_cast<std::int32_t>(rs2);
                    if (funct7 == 0x00u) {
                        instruction.mnemonic = Mnemonic::Srli;
                    } else if (funct7 == 0x20u) {
                        instruction.mnemonic = Mnemonic::Srai;
                    }
                    break;
                default: break;
            }
            break;

        case 0x17u:
            instruction.mnemonic = Mnemonic::Auipc;
            instruction.imm = immU(raw);
            break;

        case 0x23u:
            instruction.imm = immS(raw);
            switch (funct3) {
                case 0: instruction.mnemonic = Mnemonic::Sb; break;
                case 1: instruction.mnemonic = Mnemonic::Sh; break;
                case 2: instruction.mnemonic = Mnemonic::Sw; break;
                default: break;
            }
            break;

        case 0x33u:
            switch (funct3) {
                case 0:
                    if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Add;
                    else if (funct7 == 0x20u) instruction.mnemonic = Mnemonic::Sub;
                    break;
                case 1: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Sll; break;
                case 2: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Slt; break;
                case 3: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Sltu; break;
                case 4: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Xor; break;
                case 5:
                    if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Srl;
                    else if (funct7 == 0x20u) instruction.mnemonic = Mnemonic::Sra;
                    break;
                case 6: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::Or; break;
                case 7: if (funct7 == 0x00u) instruction.mnemonic = Mnemonic::And; break;
                default: break;
            }
            break;

        case 0x37u:
            instruction.mnemonic = Mnemonic::Lui;
            instruction.imm = immU(raw);
            break;

        case 0x63u:
            instruction.imm = immB(raw);
            switch (funct3) {
                case 0: instruction.mnemonic = Mnemonic::Beq; break;
                case 1: instruction.mnemonic = Mnemonic::Bne; break;
                case 4: instruction.mnemonic = Mnemonic::Blt; break;
                case 5: instruction.mnemonic = Mnemonic::Bge; break;
                case 6: instruction.mnemonic = Mnemonic::Bltu; break;
                case 7: instruction.mnemonic = Mnemonic::Bgeu; break;
                default: break;
            }
            break;

        case 0x67u:
            if (funct3 == 0) {
                instruction.mnemonic = Mnemonic::Jalr;
                instruction.imm = immI(raw);
            }
            break;

        case 0x6Fu:
            instruction.mnemonic = Mnemonic::Jal;
            instruction.imm = immJ(raw);
            break;

        case 0x73u:
            if (funct3 == 0) {
                const std::uint32_t value = (raw >> 20) & 0xFFFu;
                if (value == 0) instruction.mnemonic = Mnemonic::Ecall;
                else if (value == 1) instruction.mnemonic = Mnemonic::Ebreak;
            }
            break;

        default:
            break;
    }

    return instruction;
}

std::string disassemble(const Instruction& instruction, std::uint32_t pc) {
    const Mnemonic m = instruction.mnemonic;

    if (m == Mnemonic::Invalid) {
        return "invalid (" + hex(instruction.raw) + ")";
    }

    std::ostringstream stream;
    stream << mnemonicName(m) << ' ';

    if (m == Mnemonic::Ecall || m == Mnemonic::Ebreak) {
        return mnemonicName(m);
    }

    if (isLoad(m)) {
        stream << reg(instruction.rd) << ", " << instruction.imm
               << '(' << reg(instruction.rs1) << ')';
    } else if (isStore(m)) {
        stream << reg(instruction.rs2) << ", " << instruction.imm
               << '(' << reg(instruction.rs1) << ')';
    } else if (isImmAlu(m)) {
        stream << reg(instruction.rd) << ", " << reg(instruction.rs1)
               << ", " << instruction.imm;
    } else if (isShiftImm(m)) {
        stream << reg(instruction.rd) << ", " << reg(instruction.rs1)
               << ", " << instruction.imm;
    } else if (isRType(m)) {
        stream << reg(instruction.rd) << ", " << reg(instruction.rs1)
               << ", " << reg(instruction.rs2);
    } else if (m == Mnemonic::Lui || m == Mnemonic::Auipc) {
        stream << reg(instruction.rd) << ", "
               << hex(static_cast<std::uint32_t>(instruction.imm) >> 12);
    } else if (isBranch(m)) {
        const std::uint32_t target =
            pc + static_cast<std::uint32_t>(instruction.imm);
        stream << reg(instruction.rs1) << ", " << reg(instruction.rs2)
               << ", " << hex(target);
    } else if (m == Mnemonic::Jal) {
        const std::uint32_t target =
            pc + static_cast<std::uint32_t>(instruction.imm);
        stream << reg(instruction.rd) << ", " << hex(target);
    } else if (m == Mnemonic::Jalr) {
        stream << reg(instruction.rd) << ", " << instruction.imm
               << '(' << reg(instruction.rs1) << ')';
    }

    return stream.str();
}
