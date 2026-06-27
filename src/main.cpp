#include "instruction.hpp"
#include "simulator.hpp"

#include <array>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

const std::array<const char*, 32> kAbiNames = {
    "zero", "ra", "sp", "gp", "t0", "t1", "t2", "s0",
    "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6",
    "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8",
    "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

constexpr std::uint64_t kMaxRunSteps = 100000000ull;

std::string toLower(std::string text) {
    for (char& character : text) {
        character = static_cast<char>(std::tolower(
            static_cast<unsigned char>(character)
        ));
    }
    return text;
}

std::optional<std::uint32_t> parseNumber(const std::string& token) {
    try {
        std::size_t consumed = 0;
        const unsigned long value = std::stoul(token, &consumed, 0);
        if (consumed != token.size()) {
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(value);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::size_t> parseRegister(const std::string& token) {
    const std::string name = toLower(token);

    if (name.size() >= 2 && name[0] == 'x') {
        const auto index = parseNumber(name.substr(1));
        if (index && *index < 32) {
            return static_cast<std::size_t>(*index);
        }
        return std::nullopt;
    }

    for (std::size_t i = 0; i < kAbiNames.size(); ++i) {
        if (name == kAbiNames[i]) {
            return i;
        }
    }

    if (name == "pc") {
        return std::nullopt;
    }

    return std::nullopt;
}

std::string formatWord(std::uint32_t value) {
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << std::setw(8)
           << std::setfill('0') << value;
    return stream.str();
}

void printRegister(std::size_t index, std::uint32_t value) {
    std::cout << "x" << index << " = " << formatWord(value) << '\n';
}

void commandStep(Simulator& simulator, std::uint64_t count) {
    for (std::uint64_t i = 0; i < count; ++i) {
        if (simulator.halted()) {
            std::cout << "El programa ya termino.\n";
            return;
        }

        const std::uint32_t pc = simulator.cpu().pc();
        const Instruction instruction = decode(simulator.memory().read32(pc));

        std::cout << formatWord(pc) << ": "
                  << disassemble(instruction, pc) << '\n';

        simulator.step();

        if (simulator.halted()) {
            std::cout << "Programa finalizado (codigo "
                      << simulator.exitCode() << ").\n";
            return;
        }
    }
}

void commandRun(Simulator& simulator) {
    std::uint64_t executed = 0;

    while (!simulator.halted() && executed < kMaxRunSteps) {
        const std::uint32_t pc = simulator.cpu().pc();
        simulator.step();
        ++executed;

        if (simulator.cpu().pc() == pc && !simulator.halted()) {
            std::cout << "Loop infinito detectado en " << formatWord(pc)
                      << " (programa finalizado).\n";
            return;
        }
    }

    if (simulator.halted()) {
        std::cout << "Programa finalizado (codigo "
                  << simulator.exitCode() << ").\n";
    } else {
        std::cout << "Limite de instrucciones alcanzado.\n";
    }
}

void commandRegs(Simulator& simulator, const std::vector<std::string>& args) {
    if (args.empty()) {
        simulator.cpu().dumpState(std::cout);
        std::cout << '\n';
        return;
    }

    for (const std::string& token : args) {
        const std::string lowered = toLower(token);

        if (lowered == "pc") {
            std::cout << "pc = " << formatWord(simulator.cpu().pc()) << '\n';
            continue;
        }

        const auto index = parseRegister(token);
        if (!index) {
            std::cout << "Registro invalido: " << token << '\n';
            continue;
        }

        printRegister(*index, simulator.cpu().readRegister(*index));
    }
}

void commandMem(Simulator& simulator, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cout << "Uso: mem <inicio> <fin>\n";
        return;
    }

    const auto start = parseNumber(args[0]);
    const auto end = parseNumber(args[1]);

    if (!start || !end) {
        std::cout << "Direcciones invalidas.\n";
        return;
    }

    if (*end < *start) {
        std::cout << "El fin debe ser mayor o igual al inicio.\n";
        return;
    }

    std::cout << "Memoria (" << formatWord(*start) << "-" << formatWord(*end)
              << "):";

    for (std::uint32_t address = *start; address <= *end; ++address) {
        std::cout << ' ' << std::hex << std::uppercase << std::setw(2)
                  << std::setfill('0')
                  << static_cast<unsigned int>(
                         simulator.memory().read8(address)
                     );
        if (address == *end) {
            break;
        }
    }

    std::cout << std::dec << '\n';
}

void printHelp() {
    std::cout
        << "Comandos disponibles:\n"
        << "  load <archivo>      Carga un binario crudo en 0x00000000\n"
        << "  step [n]            Ejecuta n instrucciones (por defecto 1)\n"
        << "  run                 Ejecuta hasta terminar o detectar un loop\n"
        << "  pc                  Muestra el contador de programa\n"
        << "  regs [r1 r2 ...]    Muestra registros (todos si no se indican)\n"
        << "  mem <inicio> <fin>  Muestra memoria en el rango dado\n"
        << "  reset               Reinicia PC y registros\n"
        << "  help                Muestra esta ayuda\n"
        << "  exit                Sale del simulador\n";
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

}

int main(int argc, char** argv) {
    Simulator simulator;

    if (argc >= 2) {
        try {
            simulator.loadProgramFromFile(argv[1]);
            std::cout << "\"" << argv[1] << "\" cargado a memoria.\n";
        } catch (const std::exception& error) {
            std::cerr << "Error: " << error.what() << '\n';
            return 1;
        }
    } else {
        std::cout << "RISC-V RV32I Simulator\n";
        std::cout << "Use 'load <archivo>' para cargar un programa. "
                  << "'help' para ayuda.\n";
    }

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        const std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string command = toLower(tokens[0]);
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        try {
            if (command == "exit" || command == "quit") {
                break;
            } else if (command == "help") {
                printHelp();
            } else if (command == "load") {
                if (args.empty()) {
                    std::cout << "Uso: load <archivo>\n";
                } else {
                    simulator.loadProgramFromFile(args[0]);
                    std::cout << "\"" << args[0] << "\" cargado a memoria.\n";
                }
            } else if (command == "pc") {
                std::cout << "pc = " << formatWord(simulator.cpu().pc())
                          << '\n';
            } else if (command == "step" || command == "s") {
                std::uint64_t count = 1;
                if (!args.empty()) {
                    const auto parsed = parseNumber(args[0]);
                    if (parsed) {
                        count = *parsed;
                    }
                }
                commandStep(simulator, count);
            } else if (command == "run" || command == "r") {
                commandRun(simulator);
            } else if (command == "regs" || command == "reg") {
                commandRegs(simulator, args);
            } else if (command == "mem") {
                commandMem(simulator, args);
            } else if (command == "reset") {
                simulator.reset();
                std::cout << "Estado reiniciado.\n";
            } else {
                std::cout << "Comando desconocido: " << command
                          << " (use 'help').\n";
            }
        } catch (const std::exception& error) {
            std::cout << "Error: " << error.what() << '\n';
        }
    }

    std::cout << "See you next time...\n";
    std::cout << "Program exited with code " << simulator.exitCode() << ".\n";

    return 0;
}
