#include "nesturbia/joypad.hpp"

namespace nesturbia {

void Joypad::SetInput(const input_t &input) {
  currentInput = input.a | input.b << 1 | input.select << 2 | input.start << 3 | input.up << 4 |
                 input.down << 5 | input.left << 6 | input.right << 7;
}

uint8 Joypad::Read() {
  // Initialize the return value
  // TODO I'm not yet sure why this bit is set
  uint8 returnByte = 0x40;

  // If the strobe signal is high, return the status of the A button (bit 0)
  if (strobeLatch) {
    returnByte |= currentInput.bit(0);
  } else {
    // Get bit 0 from the shift register
    returnByte |= shiftRegister.bit(0);

    // Shift the shift register by one
    shiftRegister >>= 1;

    // The shift register is filled with 1's in the highest bit after shifting
    shiftRegister |= 0x80;
  }

  return returnByte;
}

void Joypad::Strobe(bool strobe) {
  if (strobeLatch && !strobe) {
    // Falling edge (1 to 0) transition of the strobe signal
    // This latches the current joypad state into the shift register
    shiftRegister = currentInput;
  }

  // Keep track of the current strobe level to detect falling edges
  strobeLatch = strobe;
}

} // namespace nesturbia
