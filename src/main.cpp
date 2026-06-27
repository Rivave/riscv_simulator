#include "cpu.hpp"
#include "memory.hpp"

#include <exception>
#include <iomanip>
#include <iostream>

int main() {
    try {
        CPU cpu;
        Memory memory;

        std::cout << "RISC-V RV32I Simulator\n";
        std::cout << "Initial architectural state:\n\n";

        // Prueba temporal del banco de registros.
        cpu.writeRegister(0, 999);
        cpu.writeRegister(1, 10);
        cpu.writeRegister(2, 20);
        cpu.writeRegister(10, 100);

        cpu.dumpState(std::cout);

        std::cout << "\nLittle-endian memory test:\n\n";

        // Escribir diferentes tamaños de datos.
        memory.write32(0x00001000, 0x12345678U);
        memory.write16(0x00001004, 0xABCDU);
        memory.write8(0x00001006, 0xEFU);

        // Mostrar 8 bytes desde 0x1000.
        memory.dump(std::cout, 0x00001000, 8);

        std::cout << '\n';

        std::cout << "read32(0x00001000) = 0x"
                  << std::hex
                  << std::uppercase
                  << std::setw(8)
                  << std::setfill('0')
                  << memory.read32(0x00001000)
                  << '\n';

        std::cout << "read16(0x00001004) = 0x"
                  << std::setw(4)
                  << static_cast<std::uint32_t>(
                         memory.read16(0x00001004)
                     )
                  << '\n';

        std::cout << "read8 (0x00001006) = 0x"
                  << std::setw(2)
                  << static_cast<std::uint32_t>(
                         memory.read8(0x00001006)
                     )
                  << '\n';

        std::cout << "read8 (0x00001007) = 0x"
                  << std::setw(2)
                  << static_cast<std::uint32_t>(
                         memory.read8(0x00001007)
                     )
                  << '\n';

        std::cout << std::dec;

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: "
                  << error.what()
                  << '\n';

        return 1;
    }
}