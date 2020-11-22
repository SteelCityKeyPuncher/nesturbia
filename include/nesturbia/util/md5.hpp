#ifndef UTIL_MD5_HPP_INCLUDED
#define UTIL_MD5_HPP_INCLUDED

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

static inline std::string md5(const void *message, size_t length) {
  auto round = [](uint32_t *X, const uint32_t *M) {
    static constexpr std::array<uint32_t, 16> S = {7, 12, 17, 22, 5, 9,  14, 20,
                                                   4, 11, 16, 23, 6, 10, 15, 21};

    static constexpr std::array<uint32_t, 64> K = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613,
        0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193,
        0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d,
        0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
        0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
        0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244,
        0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb,
        0xeb86d391};

    uint32_t A = X[0];
    uint32_t B = X[1];
    uint32_t C = X[2];
    uint32_t D = X[3];

    for (uint32_t i = 0; i < 64; ++i) {
      uint32_t F = 0;
      if (i < 16) {
        F = ((B & C) | ((~B) & D)) + A + K[i] + M[i];
      } else if (i < 32) {
        F = ((D & B) | ((~D) & C)) + A + K[i] + M[(5 * i + 1) & 0xf];
      } else if (i < 48) {
        F = (B ^ C ^ D) + A + K[i] + M[(3 * i + 5) & 0xf];
      } else {
        F = (C ^ (B | (~D))) + A + K[i] + M[(7 * i) & 0xf];
      }

      A = D;
      D = C;
      C = B;
      const auto j = ((i >> 2) & 0xc) | (i & 0x3);
      B += (F << S[j]) | (F >> (32 - S[j]));
    }

    X[0] += A;
    X[1] += B;
    X[2] += C;
    X[3] += D;
  };

  auto messageBytes = reinterpret_cast<const uint8_t *>(message);
  const auto lengthOriginalBits = static_cast<uint64_t>(length << 3);

  std::array<uint32_t, 4> digest = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};

  while (length >= 64) {
    round(digest.data(), reinterpret_cast<const uint32_t *>(messageBytes));
    messageBytes += 64;
    length -= 64;
  }

  std::array<uint8_t, 64> buf;
  memset(buf.data(), 0, buf.size());
  memcpy(buf.data(), messageBytes, length);
  buf[length] = 0x80;

  if (length > 56) {
    round(digest.data(), reinterpret_cast<const uint32_t *>(buf.data()));
    memset(buf.data(), 0, buf.size());
  }

  memcpy(&buf[56], &lengthOriginalBits, 8);
  round(digest.data(), reinterpret_cast<const uint32_t *>(buf.data()));

  std::string hashStr;
  auto digestBytes = reinterpret_cast<const uint8_t *>(digest.data());
  for (size_t i = 0; i < 16; i++) {
    hashStr += "0123456789abcdef"[digestBytes[i] >> 4];
    hashStr += "0123456789abcdef"[digestBytes[i] & 0xf];
  }

  return hashStr;
}

#endif // UTIL_MD5_HPP_INCLUDED
