#ifndef NESTURBIA_JOYPAD_HPP_INCLUDED
#define NESTURBIA_JOYPAD_HPP_INCLUDED

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Joypad {
  // Types
  struct input_t {
    bool a = false;
    bool b = false;
    bool select = false;
    bool start = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
  };

  // Data
  uint8 currentInput;
  uint8 shiftRegister;

  // TODO when to reset?
  bool strobeLatch;

  // Public functions
  void SetInput(const input_t &input);
  uint8 Read();
  void Strobe(bool strobe);
};

} // namespace nesturbia

#endif // NESTURBIA_JOYPAD_HPP_INCLUDED
