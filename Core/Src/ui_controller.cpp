/*
 * ui_controller.cpp
 *
 *  UI controller implementation
 */

#include "ui_controller.hpp"
#include "audio_system.hpp"
#include "display_engine.hpp"

namespace UI {
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

	int barWidth{11};
	int spacing{9};

	// Calculate bar heights from mixer gains
	int in1Height{static_cast<int>(Mixer::param.gainADCLeft * 40.0f)};
	int in2Height{static_cast<int>(Mixer::param.gainADCRight * 40.0f)};
	int mainUSBHeight{static_cast<int>(Mixer::param.gainMainUSB * 40.0f)};
	int frontUSBHeight{static_cast<int>(Mixer::param.gainFrontUSB * 40.0f)};
	int rearUSBHeight{static_cast<int>(Mixer::param.gainRearUSB * 40.0f)};
	int masterHeight{static_cast<int>(Mixer::param.gainMaster * 40.0f)};

	// Draw channel labels
	Display::drawString(1 * spacing, 5, "I1");
	Display::drawString(2 * spacing + 1 * barWidth, 5, "I2");
	Display::drawString(3 * spacing + 2 * barWidth, 5, "UM");
	Display::drawString(4 * spacing + 3 * barWidth, 5, "UF");
	Display::drawString(5 * spacing + 4 * barWidth, 5, "UR");
	Display::drawString(6 * spacing + 5 * barWidth, 5, "MN");

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
	float32_t step{0.10f};

	// Reset command
	if (cmd == 'R') {
		state.currentMode = UI::Mode::HomeAdjustMasterVol;
		state.selectedIdx = 0;
	}

	if (state.currentMode == UI::Mode::HomeAdjustMasterVol) {
		if (cmd == 'D')
			Mixer::param.gainMaster =
					Mixer::clampGain(Mixer::param.gainMaster + step);
		if (cmd == 'A')
			Mixer::param.gainMaster =
					Mixer::clampGain(Mixer::param.gainMaster - step);
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
				Mixer::param.gainADCLeft =
						Mixer::clampGain(Mixer::param.gainADCLeft + step);
			if (cmd == 'A')
				Mixer::param.gainADCLeft =
						Mixer::clampGain(Mixer::param.gainADCLeft - step);
		} else if (state.selectedIdx == 1) {
			if (cmd == 'D')
				Mixer::param.gainADCRight =
						Mixer::clampGain(Mixer::param.gainADCRight + step);
			if (cmd == 'A')
				Mixer::param.gainADCRight =
						Mixer::clampGain(Mixer::param.gainADCRight - step);
		} else if (state.selectedIdx == 2) {
			if (cmd == 'D')
				Mixer::param.gainMainUSB =
						Mixer::clampGain(Mixer::param.gainMainUSB + step);
			if (cmd == 'A')
				Mixer::param.gainMainUSB =
						Mixer::clampGain(Mixer::param.gainMainUSB - step);
		} else if (state.selectedIdx == 3) {
			if (cmd == 'D')
				Mixer::param.gainFrontUSB =
						Mixer::clampGain(Mixer::param.gainFrontUSB + step);
			if (cmd == 'A')
				Mixer::param.gainFrontUSB =
						Mixer::clampGain(Mixer::param.gainFrontUSB - step);
		} else if (state.selectedIdx == 4) {
			if (cmd == 'D')
				Mixer::param.gainRearUSB =
						Mixer::clampGain(Mixer::param.gainRearUSB + step);
			if (cmd == 'A')
				Mixer::param.gainRearUSB =
						Mixer::clampGain(Mixer::param.gainRearUSB - step);
		}

		if (cmd == 'E')
			state.currentMode = UI::Mode::SelectChannel;
	}
	isRenderNeeded = true;
}
} // namespace UI
