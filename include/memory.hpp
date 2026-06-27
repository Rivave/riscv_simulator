#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <unordered_map>
#include <vector>

class Memory {
public:
    Memory() = default;

    void clear();

    void loadProgram(
        std::uint32_t address,
        const std::vector<std::uint8_t>& program
    );

    [[nodiscard]] std::uint8_t read8(std::uint32_t address) const;
    [[nodiscard]] std::uint16_t read16(std::uint32_t address) const;
    [[nodiscard]] std::uint32_t read32(std::uint32_t address) const;

    void write8(std::uint32_t address, std::uint8_t value);
    void write16(std::uint32_t address, std::uint16_t value);
    void write32(std::uint32_t address, std::uint32_t value);

    [[nodiscard]] std::uint64_t fingerprint() const;

    void dump(
        std::ostream& output,
        std::uint32_t startAddress,
        std::size_t byteCount
    ) const;

private:
    static void validateRange(
        std::uint32_t address,
        std::size_t byteCount
    );

    std::unordered_map<std::uint32_t, std::uint8_t> bytes_;
};
