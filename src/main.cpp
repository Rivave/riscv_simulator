#include "cpu.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        CPU cpu;

        std::cout << "RISC-V RV32I Simulator\n";
        std::cout << "Initial architectural state:\n\n";

        // Prueba de escritura en registros.
        cpu.writeRegister(0, 999);
        cpu.writeRegister(1, 10);
        cpu.writeRegister(2, 20);
        cpu.writeRegister(10, 100);

        cpu.dumpState(std::cout);

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}