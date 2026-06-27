#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

class CPU {
public:
    CPU();

    void reset();

    [[nodiscard]] std::uint32_t pc() const;
    void setPc(std::uint32_t value);

    [[nodiscard]] std::uint32_t readRegister(std::size_t index) const;
    void writeRegister(std::size_t index, std::uint32_t value);
    void dumpState(std::ostream& output) const;

private:
    std::uint32_t pc_{0};
    std::array<std::uint32_t, 32> registers_{};
};