/**
 * @file display_engine.hpp
 * @brief OLED display engine and drawing primitives.
 */

#pragma once

#include <array>
#include <cstdint>

namespace Display {
// ========================================================================
// Display Configuration
// ========================================================================
/** @brief Display width in pixels. */
constexpr int DisplayWidth{128};
/** @brief Display height in pixels. */
constexpr int DisplayHeight{64};
/** @brief VRAM size in bytes for 1bpp page layout. */
constexpr int VramSize{(DisplayHeight / 8) * DisplayWidth};

// ========================================================================
// Display State
// ========================================================================
/** @brief Display VRAM buffer. */
extern std::array<uint8_t, VramSize> vram;

// ========================================================================
// Graphics API
// ========================================================================
/**
 * @brief Clear the display
 */
void clear();

/**
 * @brief Draw a pixel at (x, y)
 * @param x X position in pixels.
 * @param y Y position in pixels.
 */
void drawPixel(int x, int y);

/**
 * @brief Draw a filled bar/rectangle
 * @param x X position
 * @param y Y position (top-left)
 * @param width Width in pixels
 * @param height Height in pixels
 */
void drawBar(int x, int y, int width, int height);

/**
 * @brief Draw a single character
 * @param x X position
 * @param y Y position
 * @param c Character to draw
 */
void drawChar(int x, int y, char c);

/**
 * @brief Draw a string
 * @param x X position
 * @param y Y position
 * @param str String (null-terminated)
 */
void drawString(int x, int y, const char *str);

/**
 * @brief Update the physical display from VRAM
 */
void updateDisplay();
} // namespace Display
