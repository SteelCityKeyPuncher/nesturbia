#ifndef NESTURBIA_COMMON_HPP_INCLUDED
#define NESTURBIA_COMMON_HPP_INCLUDED

#include <cstdint>
#include <type_traits>

namespace nesturbia {

template <unsigned Precision> struct uint {
  using type_t = std::conditional_t<Precision == 8, uint8_t,
                                    std::conditional_t<Precision == 16, uint16_t, void>>;

  uint() = default;
  template <typename T> uint(const T &value) : data(static_cast<type_t>(value)) {}

  operator type_t() const { return data; }

  [[nodiscard]] bool bit(int bit) const {
    if (bit < 0 || bit >= static_cast<int>((sizeof(type_t) * 8))) {
      return false;
    }

    return (data & (1U << bit)) != 0;
  }

  uint operator++(int) {
    auto value = *this;
    data = static_cast<type_t>(data + 1);
    return value;
  }

  uint operator--(int) {
    auto value = *this;
    data = static_cast<type_t>(data - 1);
    return value;
  }

  uint &operator++() {
    data = static_cast<type_t>(data + 1);
    return *this;
  }

  uint &operator--() {
    data = static_cast<type_t>(data - 1);
    return *this;
  }

  template <typename T> uint &operator=(const T &value) {
    data = static_cast<type_t>(value);
    return *this;
  }

  template <typename T> auto &operator*=(const T &value) {
    data = static_cast<type_t>(data * value);
    return *this;
  }

  template <typename T> auto &operator/=(const T &value) {
    data = static_cast<type_t>(data / value);
    return *this;
  }

  template <typename T> auto &operator%=(const T &value) {
    data = static_cast<type_t>(data % value);
    return *this;
  }

  template <typename T> auto &operator+=(const T &value) {
    data = static_cast<type_t>(data + value);
    return *this;
  }

  template <typename T> auto &operator-=(const T &value) {
    data = static_cast<type_t>(data - value);
    return *this;
  }

  template <typename T> auto &operator<<=(const T &value) {
    data = static_cast<type_t>(data << value);
    return *this;
  }

  template <typename T> auto &operator>>=(const T &value) {
    data = static_cast<type_t>(data >> value);
    return *this;
  }

  template <typename T> auto &operator&=(const T &value) {
    data = static_cast<type_t>(data & value);
    return *this;
  }

  template <typename T> auto &operator^=(const T &value) {
    data = static_cast<type_t>(data ^ value);
    return *this;
  }

  template <typename T> auto &operator|=(const T &value) {
    data = static_cast<type_t>(data | value);
    return *this;
  }

private:
  type_t data = 0;
};

using uint8 = uint<8>;
using uint16 = uint<16>;

} // namespace nesturbia

#endif // NESTURBIA_COMMON_HPP_INCLUDED
