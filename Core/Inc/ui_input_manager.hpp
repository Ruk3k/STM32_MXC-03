#pragma once

#include "stm32h7xx_nucleo.h"

namespace UiInput {

/**
 * @brief Initialize input processing context.
 * @param encoderTimer Timer configured in encoder mode.
 * @param uart Uart handle used for command reception.
 */
void initialize(TIM_HandleTypeDef &encoderTimer, UART_HandleTypeDef &uart);

/**
 * @brief Process encoder/button events and trigger UI render when needed.
 */
void processLoop();

/**
 * @brief Handle UART RX complete callback.
 * @param huart UART handle from HAL callback.
 */
void onUartRxComplete(UART_HandleTypeDef *huart);

/**
 * @brief Handle USER button interrupt callback.
 * @param button Button ID from BSP callback.
 */
void onUserButtonInterrupt(Button_TypeDef button);

} // namespace UiInput
