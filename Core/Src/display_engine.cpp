/*
 * display_engine.cpp
 *
 *  Display engine implementation
 */

#include "display_engine.hpp"
#include "font5x7_ascii.hpp"
#include "stm32h7xx_nucleo.h"

// External UART handle from BSP
extern UART_HandleTypeDef hcom_uart[];

namespace Display {
// ========================================================================
// Display State (Definition)
// ========================================================================
std::array<uint8_t, VramSize> vram{};

// ========================================================================
// Graphics Implementation
// ========================================================================
void clear() { vram.fill(0); }

void drawPixel(int x, int y) {
  if (x < 0 || x >= DisplayWidth || y < 0 || y >= DisplayHeight)
    return;
  vram[(y / 8) * DisplayWidth + x] |= (1 << (y % 8));
}

void drawBar(int x, int y, int width, int height) {
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      const int yPos{y - j};

      if ((yPos & 1) == 0) {
        drawPixel(x + i, yPos);
      }
    }
  }
}

void drawChar(int x, int y, char c) {
  const uint8_t uc{static_cast<uint8_t>(c)};
  if (uc < Font5x7::FirstChar || uc > Font5x7::LastChar) {
    return;
  }

  const auto &bitmap{Font5x7::Glyphs[uc - Font5x7::FirstChar]};

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 7; j++) {
      if (bitmap[i] & (1 << j)) {
        drawPixel(x + i, y + j);
      }
    }
  }
}

void drawString(int x, int y, const char *str) {
  while (*str) {
    drawChar(x, y, *str++);
    x += 6;
  }
}

void updateDisplay() {
  uint8_t header[2] = {0xFF, 0xAA};
  HAL_UART_Transmit(&hcom_uart[COM1], header, 2, 10);
  HAL_UART_Transmit(&hcom_uart[COM1], vram.data(), vram.size(), 1000);
}
} // namespace Display
