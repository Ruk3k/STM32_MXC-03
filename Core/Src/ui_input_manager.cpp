#include "ui_input_manager.hpp"

#include "ui_controller.hpp"

namespace {

constexpr uint32_t UserButtonLongPressMs{500};

struct UserButtonState {
  bool isTracking{false};
  bool isLongPressHandled{false};
  uint32_t pressStartTick{0};
};

struct UiInputState {
  TIM_HandleTypeDef *encoderTimer{nullptr};
  UART_HandleTypeDef *uart{nullptr};
  uint8_t uartRxCmd{0};
  volatile bool userButtonPressed{false};
  uint16_t prevEncoderRawCount{0};
  int16_t encoderSubCount{0};
  UserButtonState userButton{};
};

UiInputState uiInputState{};

void processEncoder(UiInputState &state) {
  const uint16_t rawCurrentCount{
      static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(state.encoderTimer))};
  const int16_t rawDelta{
      static_cast<int16_t>(rawCurrentCount - state.prevEncoderRawCount)};
  state.prevEncoderRawCount = rawCurrentCount;

  // Keep quadrature x4 decoding and emit one UI step per 4 counts.
  state.encoderSubCount =
      static_cast<int16_t>(state.encoderSubCount + rawDelta);

  while (state.encoderSubCount >= 4) {
    UI::processCommand('D');
    state.encoderSubCount = static_cast<int16_t>(state.encoderSubCount - 4);
  }
  while (state.encoderSubCount <= -4) {
    UI::processCommand('A');
    state.encoderSubCount = static_cast<int16_t>(state.encoderSubCount + 4);
  }
}

void processUserButton(UiInputState &state) {
  if (state.userButtonPressed) {
    state.userButtonPressed = false;
    if (!state.userButton.isTracking &&
        BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET) {
      state.userButton.isTracking = true;
      state.userButton.isLongPressHandled = false;
      state.userButton.pressStartTick = HAL_GetTick();
    }
  }

  if (state.userButton.isTracking) {
    if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET) {
      if (!state.userButton.isLongPressHandled &&
          (HAL_GetTick() - state.userButton.pressStartTick) >=
              UserButtonLongPressMs) {
        UI::processCommand('R');
        state.userButton.isLongPressHandled = true;
      }
    } else {
      if (!state.userButton.isLongPressHandled) {
        UI::processCommand('E');
      }
      state.userButton.isTracking = false;
    }
  }
}

void renderIfNeeded() {
  if (UI::isRenderNeeded) {
    UI::isRenderNeeded = false;
    UI::render();
  }
}

} // namespace

namespace UiInput {

void initialize(TIM_HandleTypeDef &encoderTimer, UART_HandleTypeDef &uart) {
  uiInputState.encoderTimer = &encoderTimer;
  uiInputState.uart = &uart;
  uiInputState.prevEncoderRawCount =
      static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(uiInputState.encoderTimer));

  HAL_UART_Receive_IT(uiInputState.uart, &uiInputState.uartRxCmd, 1);
}

void processLoop() {
  processEncoder(uiInputState);
  processUserButton(uiInputState);
  renderIfNeeded();
}

void onUartRxComplete(UART_HandleTypeDef *huart) {
  if (uiInputState.uart == nullptr || huart != uiInputState.uart) {
    return;
  }

  UI::processCommand(static_cast<char>(uiInputState.uartRxCmd));
  HAL_UART_Receive_IT(uiInputState.uart, &uiInputState.uartRxCmd, 1);
}

void onUserButtonInterrupt(Button_TypeDef button) {
  if (button == BUTTON_USER) {
    uiInputState.userButtonPressed = true;
  }
}

} // namespace UiInput
