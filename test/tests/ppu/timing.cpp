#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cartridge.hpp"
#include "nesturbia/ppu.hpp"
using namespace nesturbia;

TEST_CASE("Ppu_TimingVBlank", "[ppu]") {
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  //
  // Test that a VBLANK occurs at the appropriate time
  //

  // PPUSTATUS bit 7 can be anything at power up
  // Turn of the bit so that we can see when it first comes on
  ppu.status.vblankStarted = false;

  uint16 vblankScanline = 0;
  uint16 vblankDot = 0;

  // Run for 1 million cycles in case the VBLANK bit isn't set
  for (int i = 0; i < 1000000; i++) {
    static_cast<void>(ppu.Tick());

    if (ppu.status.vblankStarted) {
      // Break once the bit is set so we can check when it was set
      break;
    }

    vblankScanline = ppu.scanline;
    vblankDot = ppu.dot;
  }

  CHECK(vblankScanline == 241);
  CHECK(vblankDot == 1);

  //
  // Test that PPUSTATUS flags are cleared at the appropriate time
  //

  // First, set the two flags other than the VBLANK flag
  ppu.status.spriteOverflow = true;
  ppu.status.sprite0Hit = true;

  // Keep track of the current PPUSTATUS
  const auto statusCopy = ppu.status;

  uint16 statusClearedScanline = 0;
  uint16 statusClearedDot = 0;

  // Run for 1 million cycles in case the status doesn't behave as expected
  for (int i = 0; i < 1000000; i++) {
    static_cast<void>(ppu.Tick());

    // Break once the status changes
    if (statusCopy != ppu.status) {
      break;
    }

    statusClearedScanline = ppu.scanline;
    statusClearedDot = ppu.dot;
  }

  CHECK(statusClearedScanline == 261);
  CHECK(statusClearedDot == 1);
}

TEST_CASE("Ppu_TimingOddFrame_RenderingEnabled", "[ppu]") {
  // Test that "odd frames" skip the first tick when rendering is enabled
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Enable rendering (only one of these is necessary)
  ppu.mask.showBackground = true;
  ppu.mask.showSprites = true;

  // Run the even frame
  for (int i = 0; i < 262 * 341; i++) {
    static_cast<void>(ppu.Tick());
  }

  CHECK(ppu.scanline == 0);
  CHECK(ppu.dot == 0);

  // Run the odd frame
  for (int i = 0; i < 262 * 341; i++) {
    static_cast<void>(ppu.Tick());
  }

  CHECK(ppu.scanline == 0);
  CHECK(ppu.dot == 1);
}

TEST_CASE("Ppu_TimingOddFrame_RenderingDisabled", "[ppu]") {
  // Test that "odd frames" _don't_ skip the first tick when rendering is disabled
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Disable rendering (these should be false on "power up" anyway)
  ppu.mask.showBackground = false;
  ppu.mask.showSprites = false;

  // Run the even frame
  for (int i = 0; i < 262 * 341; i++) {
    static_cast<void>(ppu.Tick());
  }

  CHECK(ppu.scanline == 0);
  CHECK(ppu.dot == 0);

  // Run the odd frame
  for (int i = 0; i < 262 * 341; i++) {
    static_cast<void>(ppu.Tick());
  }

  CHECK(ppu.scanline == 0);
  CHECK(ppu.dot == 0);
}
