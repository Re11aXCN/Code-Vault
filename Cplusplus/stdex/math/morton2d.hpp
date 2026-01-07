#pragma once

#include <cstdint>
#include <tuple>

namespace stdex { namespace morton {
// Morton encoding (Z-order curve) for 2D coordinates
// Interleaves bits of x and y coordinates to create a single value
// Useful for spatial indexing and improving cache locality
namespace detail {
// Spread bits by inserting zeros between each bit
constexpr uint64_t spread_bits_2d(uint64_t x)
{
  x = (x | (x << 16)) & 0x00'00'FF'FF'00'00'FF'FF;
  x = (x | (x << 8)) & 0x00'FF'00'FF'00'FF'00'FF;
  x = (x | (x << 4)) & 0x0F'0F'0F'0F'0F'0F'0F'0F;
  x = (x | (x << 2)) & 0x33'33'33'33'33'33'33'33;
  x = (x | (x << 1)) & 0x55'55'55'55'55'55'55'55;
  return x;
}

// Compact bits by removing every other bit
constexpr uint64_t compact_bits_2d(uint64_t x)
{
  x = x & 0x55'55'55'55'55'55'55'55;
  x = (x | (x >> 1)) & 0x33'33'33'33'33'33'33'33;
  x = (x | (x >> 2)) & 0x0F'0F'0F'0F'0F'0F'0F'0F;
  x = (x | (x >> 4)) & 0x00'FF'00'FF'00'FF'00'FF;
  x = (x | (x >> 8)) & 0x00'00'FF'FF'00'00'FF'FF;
  x = (x | (x >> 16)) & 0xFF'FF'FF'FF'FF'FF'FF'FF;
  return x;
}

// Spread bits for 3D coordinates (insert two zeros between each bit)
constexpr uint64_t spread_bits_3d(uint64_t x)
{
  x = (x | (x << 32)) & 0x7F'FF'00'00'00'00'FF'FF;
  x = (x | (x << 16)) & 0x00'FF'00'00'FF'00'00'FF;
  x = (x | (x << 8)) & 0x70'0F'00'F0'0F'00'F0'0F;
  x = (x | (x << 4)) & 0x30'C3'0C'30'C3'0C'30'C3;
  x = (x | (x << 2)) & 0x12'49'24'92'49'24'92'49;
  return x;
}

// Compact bits for 3D coordinates (remove two of every three bits)
constexpr uint64_t compact_bits_3d(uint64_t x)
{
  x = x & 0x12'49'24'92'49'24'92'49;
  x = (x | (x >> 2)) & 0x30'C3'0C'30'C3'0C'30'C3;
  x = (x | (x >> 4)) & 0x70'0F'00'F0'0F'00'F0'0F;
  x = (x | (x >> 8)) & 0x00'FF'00'00'FF'00'00'FF;
  x = (x | (x >> 16)) & 0x7F'FF'00'00'00'00'FF'FF;
  x = (x | (x >> 32)) & 0xFF'FF'FF'FF'FF'FF'FF'FF;
  return x;
}
}  // namespace detail

// 2D Morton encoding
constexpr uint64_t encode_2d(uint64_t x, uint64_t y)
{
  return detail::spread_bits_2d(x) | detail::spread_bits_2d(y << 1);
}

// 2D Morton decoding
constexpr std::tuple<uint64_t, uint64_t> decode_2d(uint64_t code)
{
  return { detail::compact_bits_2d(code), detail::compact_bits_2d(code >> 1) };
}

// 3D Morton encoding
constexpr uint64_t encode_3d(uint64_t x, uint64_t y, uint64_t z)
{
  return detail::spread_bits_3d(x) | detail::spread_bits_3d(y << 1) |
      detail::spread_bits_3d(z << 2);
}

// 3D Morton decoding
constexpr std::tuple<uint64_t, uint64_t, uint64_t> decode_3d(uint64_t code)
{
  return { detail::compact_bits_3d(code), detail::compact_bits_3d(code >> 1),
           detail::compact_bits_3d(code >> 2) };
}
}}  // namespace stdex::morton

/*
// Usage Examples:

#include <iostream>
#include <vector>

int main() {
    using namespace stdex::morton;

    // 2D encoding/decoding
    uint64_t x = 5, y = 3;
    uint64_t code_2d = encode_2d(x, y);
    std::cout << "2D: (" << x << ", " << y << ") -> " << code_2d << std::endl;

    auto [decoded_x, decoded_y] = decode_2d(code_2d);
    std::cout << "Decoded: (" << decoded_x << ", " << decoded_y << ")" <<
std::endl;

    // 3D encoding/decoding
    uint64_t x3 = 2, y3 = 4, z3 = 1;
    uint64_t code_3d = encode_3d(x3, y3, z3);
    std::cout << "3D: (" << x3 << ", " << y3 << ", " << z3 << ") -> " << code_3d
<< std::endl;

    auto [decoded_x3, decoded_y3, decoded_z3] = decode_3d(code_3d);
    std::cout << "Decoded: (" << decoded_x3 << ", " << decoded_y3 << ", " <<
decoded_z3 << ")" << std::endl;

    // Practical use: Z-order curve traversal
    std::cout << "\nZ-order curve (4x4 grid):" << std::endl;
    for (uint64_t i = 0; i < 4; ++i) {
        for (uint64_t j = 0; j < 4; ++j) {
            std::cout << encode_2d(i, j) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
*/
