#include "memory.hpp"

#include <iomanip>
#include <ostream>
#include <stdexcept>

void Memory::clear() {
    bytes_.clear();
}

void Memory::validateRange(
    std::uint32_t address,
    std::size_t byteCount
) {
    if (byteCount == 0) {
        return;
    }

    const std::uint64_t lastAddress =
        static_cast<std::uint64_t>(address)
        + static_cast<std::uint64_t>(byteCount)
        - 1;

    if (lastAddress > 0xFFFFFFFFULL) {
        throw std::out_of_range(
            "Memory access exceeds the 32-bit address space"
        );
    }
}

std::uint8_t Memory::read8(std::uint32_t address) const {
    const auto iterator = bytes_.find(address);

    if (iterator == bytes_.end()) {
        return 0;
    }

    return iterator->second;
}

std::uint16_t Memory::read16(std::uint32_t address) const {
    validateRange(address, 2);

    const std::uint16_t byte0 =
        static_cast<std::uint16_t>(read8(address));

    const std::uint16_t byte1 =
        static_cast<std::uint16_t>(read8(address + 1));

    return static_cast<std::uint16_t>(
        byte0 |
        static_cast<std::uint16_t>(byte1 << 8)
    );
}

std::uint32_t Memory::read32(std::uint32_t address) const {
    validateRange(address, 4);

    const std::uint32_t byte0 =
        static_cast<std::uint32_t>(read8(address));

    const std::uint32_t byte1 =
        static_cast<std::uint32_t>(read8(address + 1));

    const std::uint32_t byte2 =
        static_cast<std::uint32_t>(read8(address + 2));

    const std::uint32_t byte3 =
        static_cast<std::uint32_t>(read8(address + 3));

    return byte0
        | (byte1 << 8)
        | (byte2 << 16)
        | (byte3 << 24);
}

void Memory::write8(
    std::uint32_t address,
    std::uint8_t value
) {
    bytes_[address] = value;
}

void Memory::write16(
    std::uint32_t address,
    std::uint16_t value
) {
    validateRange(address, 2);

    write8(
        address,
        static_cast<std::uint8_t>(value & 0xFFU)
    );

    write8(
        address + 1,
        static_cast<std::uint8_t>((value >> 8) & 0xFFU)
    );
}

void Memory::write32(
    std::uint32_t address,
    std::uint32_t value
) {
    validateRange(address, 4);

    write8(
        address,
        static_cast<std::uint8_t>(value & 0xFFU)
    );

    write8(
        address + 1,
        static_cast<std::uint8_t>((value >> 8) & 0xFFU)
    );

    write8(
        address + 2,
        static_cast<std::uint8_t>((value >> 16) & 0xFFU)
    );

    write8(
        address + 3,
        static_cast<std::uint8_t>((value >> 24) & 0xFFU)
    );
}

void Memory::dump(
    std::ostream& output,
    std::uint32_t startAddress,
    std::size_t byteCount
) const {
    validateRange(startAddress, byteCount);

    const auto previousFlags = output.flags();
    const auto previousFill = output.fill();

    for (std::size_t offset = 0; offset < byteCount; ++offset) {
        const std::uint32_t currentAddress =
            startAddress + static_cast<std::uint32_t>(offset);

        if (offset % 16 == 0) {
            if (offset != 0) {
                output << '\n';
            }

            output << "0x"
                   << std::hex
                   << std::uppercase
                   << std::setw(8)
                   << std::setfill('0')
                   << currentAddress
                   << ": ";
        }

        output << std::hex
               << std::uppercase
               << std::setw(2)
               << std::setfill('0')
               << static_cast<unsigned int>(
                      read8(currentAddress)
                  )
               << ' ';
    }

    if (byteCount != 0) {
        output << '\n';
    }

    output.flags(previousFlags);
    output.fill(previousFill);
}