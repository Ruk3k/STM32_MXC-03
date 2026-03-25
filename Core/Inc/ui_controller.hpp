/*
 * ui_controller.hpp
 *
 *  UI state machine and control interface
 *      Author: Ruk3k
 */

#pragma once

#include "arm_math.h"
#include <cstdint>


namespace UI {
// ========================================================================
// UI State Definitions
// ========================================================================
enum class Mode {
  HomeAdjustMasterVol, // State 1: Adjust master volume
  SelectChannel,       // State 2: Select input channel
  AdjustChannelVol,    // State 3: Adjust selected channel volume
};

struct State {
  Mode currentMode{Mode::HomeAdjustMasterVol};
  int32_t selectedIdx{0}; // 0:IN1_L, 1:IN2_R, 2:USB_M, 3:USB_F, 4:USB_R
  uint32_t pressStartTime{0};
  bool isPressed{false};
};

// ========================================================================
// UI Controller State
// ========================================================================
extern State state;
extern volatile bool isRenderNeeded;

// ========================================================================
// UI Control Interface
// ========================================================================
/**
 * @brief Initialize UI state
 */
void initialize();

/**
 * @brief Render the UI to display
 */
void render();

/**
 * @brief Process a command from user input
 * @param cmd Command character ('D', 'A', 'E', 'R', etc.)
 */
void processCommand(char cmd);

/**
 * @brief Request UI re-render
 */
inline void requestRender() { isRenderNeeded = true; }
} // namespace UI
