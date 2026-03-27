/*
 * ui_controller.cpp
 *
 *  UI controller implementation
 */

#include "ui_controller.hpp"
#include "audio_system.hpp"
#include "display_engine.hpp"

namespace UI {
namespace {
constexpr int GainSteps{20};
constexpr int PixelsPerStep{2};

int gainToSteps(float32_t gain) {
  const float32_t clamped{Mixer::clampGain(gain)};
  return static_cast<int>(clamped * static_cast<float32_t>(GainSteps) + 0.5f);
}

float32_t stepsToGain(int steps) {
  if (steps < 0) {
    steps = 0;
  } else if (steps > GainSteps) {
    steps = GainSteps;
  }
  return static_cast<float32_t>(steps) / static_cast<float32_t>(GainSteps);
}

void nudgeGain(float32_t &gain, int deltaSteps) {
  int steps{gainToSteps(gain) + deltaSteps};
  if (steps < 0) {
    steps = 0;
  } else if (steps > GainSteps) {
    steps = GainSteps;
  }
  gain = stepsToGain(steps);
}

int gainToBarHeight(float32_t gain) {
  return gainToSteps(gain) * PixelsPerStep;
}
} // namespace

// ========================================================================
// UI State (Definition)
// ========================================================================
State state;
volatile bool isRenderNeeded{true};

// ========================================================================
// UI Implementation
// ========================================================================
void initialize() {
  state.currentMode = Mode::HomeAdjustMasterVol;
  state.selectedIdx = 0;
  state.pressStartTime = 0;
  state.isPressed = false;
  isRenderNeeded = true;
}

void render() {
  Display::clear();

  int barWidth{9};
  int spacing{11};

  // Calculate bar heights from mixer gains
  int in1Height{gainToBarHeight(Mixer::param.gainADCLeft)};
  int in2Height{gainToBarHeight(Mixer::param.gainADCRight)};
  int mainUSBHeight{gainToBarHeight(Mixer::param.gainMainUSB)};
  int frontUSBHeight{gainToBarHeight(Mixer::param.gainFrontUSB)};
  int rearUSBHeight{gainToBarHeight(Mixer::param.gainRearUSB)};
  int masterHeight{gainToBarHeight(Mixer::param.gainMaster)};

  // Draw channel labels
  Display::drawString(1 * spacing - 1, 5, "I1");
  Display::drawString(2 * spacing + 1 * barWidth - 1, 5, "I2");
  Display::drawString(3 * spacing + 2 * barWidth - 1, 5, "UM");
  Display::drawString(4 * spacing + 3 * barWidth - 1, 5, "UF");
  Display::drawString(5 * spacing + 4 * barWidth - 1, 5, "UR");
  Display::drawString(6 * spacing + 5 * barWidth - 1, 5, "MN");

  // Draw bars based on current mode
  if (state.currentMode == UI::Mode::HomeAdjustMasterVol) {
    Display::drawBar(1 * spacing, 55, barWidth, in1Height);
    Display::drawBar(2 * spacing + 1 * barWidth, 55, barWidth, in2Height);
    Display::drawBar(3 * spacing + 2 * barWidth, 55, barWidth, mainUSBHeight);
    Display::drawBar(4 * spacing + 3 * barWidth, 55, barWidth, frontUSBHeight);
    Display::drawBar(5 * spacing + 4 * barWidth, 55, barWidth, rearUSBHeight);
    Display::drawBar(6 * spacing + 5 * barWidth, 55, barWidth, masterHeight);
  }

  else if (state.currentMode == UI::Mode::SelectChannel) {
    Display::drawBar(1 * spacing, 55, barWidth, in1Height);
    Display::drawBar(2 * spacing + 1 * barWidth, 55, barWidth, in2Height);
    Display::drawBar(3 * spacing + 2 * barWidth, 55, barWidth, mainUSBHeight);
    Display::drawBar(4 * spacing + 3 * barWidth, 55, barWidth, frontUSBHeight);
    Display::drawBar(5 * spacing + 4 * barWidth, 55, barWidth, rearUSBHeight);
    Display::drawBar(6 * spacing + 5 * barWidth, 55, barWidth, masterHeight);

    if (state.selectedIdx == 0)
      Display::drawBar(1 * spacing, 60, barWidth, 1);
    if (state.selectedIdx == 1)
      Display::drawBar(2 * spacing + 1 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 2)
      Display::drawBar(3 * spacing + 2 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 3)
      Display::drawBar(4 * spacing + 3 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 4)
      Display::drawBar(5 * spacing + 4 * barWidth, 60, barWidth, 1);
  }

  else if (state.currentMode == UI::Mode::AdjustChannelVol) {
    Display::drawBar(1 * spacing, 55, barWidth, in1Height);
    Display::drawBar(2 * spacing + 1 * barWidth, 55, barWidth, in2Height);
    Display::drawBar(3 * spacing + 2 * barWidth, 55, barWidth, mainUSBHeight);
    Display::drawBar(4 * spacing + 3 * barWidth, 55, barWidth, frontUSBHeight);
    Display::drawBar(5 * spacing + 4 * barWidth, 55, barWidth, rearUSBHeight);
    Display::drawBar(6 * spacing + 5 * barWidth, 55, barWidth, masterHeight);

    if (state.selectedIdx == 0)
      Display::drawBar(1 * spacing, 60, barWidth, 1);
    if (state.selectedIdx == 1)
      Display::drawBar(2 * spacing + 1 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 2)
      Display::drawBar(3 * spacing + 2 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 3)
      Display::drawBar(4 * spacing + 3 * barWidth, 60, barWidth, 1);
    if (state.selectedIdx == 4)
      Display::drawBar(5 * spacing + 4 * barWidth, 60, barWidth, 1);
  }

  Display::updateDisplay();
}

void processCommand(char cmd) {
  // Reset command
  if (cmd == 'R') {
    state.currentMode = UI::Mode::HomeAdjustMasterVol;
    state.selectedIdx = 0;
  }

  if (state.currentMode == UI::Mode::HomeAdjustMasterVol) {
    if (cmd == 'D')
      nudgeGain(Mixer::param.gainMaster, +1);
    if (cmd == 'A')
      nudgeGain(Mixer::param.gainMaster, -1);
    if (cmd == 'E')
      state.currentMode = UI::Mode::SelectChannel;
  }

  else if (state.currentMode == UI::Mode::SelectChannel) {
    if (cmd == 'D')
      state.selectedIdx = (state.selectedIdx + 1) % 5;
    if (cmd == 'A')
      state.selectedIdx = (state.selectedIdx - 1 + 5) % 5;
    if (cmd == 'E')
      state.currentMode = UI::Mode::AdjustChannelVol;
  }

  else if (state.currentMode == UI::Mode::AdjustChannelVol) {
    if (state.selectedIdx == 0) {
      if (cmd == 'D')
        nudgeGain(Mixer::param.gainADCLeft, +1);
      if (cmd == 'A')
        nudgeGain(Mixer::param.gainADCLeft, -1);
    } else if (state.selectedIdx == 1) {
      if (cmd == 'D')
        nudgeGain(Mixer::param.gainADCRight, +1);
      if (cmd == 'A')
        nudgeGain(Mixer::param.gainADCRight, -1);
    } else if (state.selectedIdx == 2) {
      if (cmd == 'D')
        nudgeGain(Mixer::param.gainMainUSB, +1);
      if (cmd == 'A')
        nudgeGain(Mixer::param.gainMainUSB, -1);
    } else if (state.selectedIdx == 3) {
      if (cmd == 'D')
        nudgeGain(Mixer::param.gainFrontUSB, +1);
      if (cmd == 'A')
        nudgeGain(Mixer::param.gainFrontUSB, -1);
    } else if (state.selectedIdx == 4) {
      if (cmd == 'D')
        nudgeGain(Mixer::param.gainRearUSB, +1);
      if (cmd == 'A')
        nudgeGain(Mixer::param.gainRearUSB, -1);
    }

    if (cmd == 'E')
      state.currentMode = UI::Mode::SelectChannel;
  }
  isRenderNeeded = true;
}
} // namespace UI
