/*
 * display_engine.cpp
 *
 *  Display engine implementation
 */

#include "display_engine.hpp"
#include "stm32h7xx_nucleo.h"

// External UART handle from BSP
extern UART_HandleTypeDef hcom_uart[];

namespace Display {
// ========================================================================
// Display State (Definition)
// ========================================================================
std::array<uint8_t, VramSize> vram{};

// ========================================================================
// Font Data
// ========================================================================
static const uint8_t font5x7[][5] = {
    {0x3F, 0x48, 0x48, 0x48, 0x3F}, // A
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // B
    {0x7F, 0x40, 0x40, 0x40, 0x7F}, // U
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}  // 2
};

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
      drawPixel(x + i, y - j);
    }
  }
}

void drawChar(int x, int y, char c) {
  const uint8_t *bitmap = nullptr;

  if (c == 'U')
    bitmap = font5x7[2];
  else if (c == 'S')
    bitmap = font5x7[3];
  else if (c == 'M')
    bitmap = font5x7[4];
  else if (c == 'F')
    bitmap = font5x7[5];
  else if (c == 'R')
    bitmap = font5x7[6];
  else if (c == 'I')
    bitmap = font5x7[7];
  else if (c == 'N')
    bitmap = font5x7[8];
  else if (c == '1')
    bitmap = font5x7[9];
  else if (c == '2')
    bitmap = font5x7[10];

  if (!bitmap)
    return;
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
