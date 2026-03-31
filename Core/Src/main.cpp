/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.cpp
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "audio_system.hpp"
#include "display_engine.hpp"
#include "effects_chain.hpp"
#include "ui_controller.hpp"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

SAI_HandleTypeDef hsai_BlockA4;
SAI_HandleTypeDef hsai_BlockB4;
DMA_HandleTypeDef hdma_sai4_a;
DMA_HandleTypeDef hdma_sai4_b;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef hcom_uart[];
uint8_t uartRxCmd{0};
volatile bool userButtonPressed{false};
uint16_t prevEncoderRawCount{0};
constexpr uint32_t UserButtonLongPressMs{500};

uint32_t cnt{0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_BDMA_Init(void);
static void MX_GPIO_Init(void);
static void MX_SAI4_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */
  // Initialize effects chain
  FX::initializeChain();

  // Initialize UI
  UI::initialize();
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_BDMA_Init();
  MX_GPIO_Init();
  MX_SAI4_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  auto *pMonitorTx{reinterpret_cast<uint8_t *>(monitorTxBuffer.data())};
  auto *pADCRx{reinterpret_cast<uint8_t *>(ADCRxBuffer.data())};

  if (HAL_SAI_Transmit_DMA(&hsai_BlockB4, pMonitorTx,
                           AudioConfig::DMABufferSize) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_SAI_Receive_DMA(&hsai_BlockA4, pADCRx, AudioConfig::DMABufferSize) !=
      HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL) != HAL_OK) {
    Error_Handler();
  }
  prevEncoderRawCount = static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(&htim3));

  UI::render();
  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time
   * it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity
   */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  HAL_UART_Receive_IT(&hcom_uart[COM1], &uartRxCmd, 1);

  bool isUserButtonTracking{false};
  bool isUserButtonLongPressHandled{false};
  uint32_t userButtonPressStartTick{0};
  int16_t encoderSubCount{0};

  while (1) {
    const uint16_t rawCurrentCount{
        static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(&htim3))};
    const int16_t rawDelta{
        static_cast<int16_t>(rawCurrentCount - prevEncoderRawCount)};
    prevEncoderRawCount = rawCurrentCount;

    // Keep quadrature x4 decoding and emit one UI step per 4 counts.
    encoderSubCount = static_cast<int16_t>(encoderSubCount + rawDelta);

    while (encoderSubCount >= 4) {
      UI::processCommand('D');
      encoderSubCount = static_cast<int16_t>(encoderSubCount - 4);
    }
    while (encoderSubCount <= -4) {
      UI::processCommand('A');
      encoderSubCount = static_cast<int16_t>(encoderSubCount + 4);
    }

    if (userButtonPressed) {
      userButtonPressed = false;
      if (!isUserButtonTracking &&
          BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET) {
        isUserButtonTracking = true;
        isUserButtonLongPressHandled = false;
        userButtonPressStartTick = HAL_GetTick();
      }
    }

    if (isUserButtonTracking) {
      if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET) {
        if (!isUserButtonLongPressHandled &&
            (HAL_GetTick() - userButtonPressStartTick) >=
                UserButtonLongPressMs) {
          UI::processCommand('R');
          isUserButtonLongPressHandled = true;
        }
      } else {
        if (!isUserButtonLongPressHandled) {
          UI::processCommand('E');
        }
        isUserButtonTracking = false;
      }
    }

    if (UI::isRenderNeeded) {
      UI::isRenderNeeded = false;
      UI::render();
    }
    cnt = rawCurrentCount;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void) {
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
   */
  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_SAI4A | RCC_PERIPHCLK_SAI4B;
  PeriphClkInitStruct.PLL3.PLL3M = 5;
  PeriphClkInitStruct.PLL3.PLL3N = 192;
  PeriphClkInitStruct.PLL3.PLL3P = 25;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Sai4AClockSelection = RCC_SAI4ACLKSOURCE_PLL3;
  PeriphClkInitStruct.Sai4BClockSelection = RCC_SAI4BCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief SAI4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SAI4_Init(void) {

  /* USER CODE BEGIN SAI4_Init 0 */

  /* USER CODE END SAI4_Init 0 */

  /* USER CODE BEGIN SAI4_Init 1 */

  /* USER CODE END SAI4_Init 1 */
  hsai_BlockA4.Instance = SAI4_Block_A;
  hsai_BlockA4.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai_BlockA4.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA4.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA4.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA4.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA4.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
  hsai_BlockA4.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA4.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA4.Init.CompandingMode = SAI_NOCOMPANDING;
  if (HAL_SAI_InitProtocol(&hsai_BlockA4, SAI_I2S_STANDARD,
                           SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK) {
    Error_Handler();
  }
  hsai_BlockB4.Instance = SAI4_Block_B;
  hsai_BlockB4.Init.AudioMode = SAI_MODESLAVE_TX;
  hsai_BlockB4.Init.Synchro = SAI_SYNCHRONOUS;
  hsai_BlockB4.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockB4.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockB4.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockB4.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockB4.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockB4.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  if (HAL_SAI_InitProtocol(&hsai_BlockB4, SAI_I2S_STANDARD,
                           SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI4_Init 2 */

  /* USER CODE END SAI4_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 10;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_BDMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_BDMA_CLK_ENABLE();

  /* DMA interrupt init */
  /* BDMA_Channel0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BDMA_Channel0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(BDMA_Channel0_IRQn);
  /* BDMA_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BDMA_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(BDMA_Channel1_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// --- Hardware / HAL Callbacks (C Linkage) ---
// These functions must use C linkage to be visible to the STM32 HAL drivers.
extern "C" {
void mainUSBRxBufferReset() { mainUSBRxBuffer.reset(); }
void mainUSBRxBufferWrite(int16_t *data, uint32_t length) {
  mainUSBRxBuffer.write(data, length);
}
int32_t mainUSBRxBufferGetAvailableFrames(void) {
  return (int32_t)mainUSBRxBuffer.getAvailableFrames();
}

static inline void processADCMonoLeftChannelInput(uint32_t start,
                                                  uint32_t numFrames) {
  for (uint32_t i = start, j = 0; j < numFrames; ++j, i += 2) {
    float32_t leftSample{static_cast<float32_t>(ADCRxBuffer[i]) *
                         Audio::Convert::Int32ToFloat};
    Mixer::chLeft[j] = leftSample * Mixer::param.gainADCLeft;
    Mixer::chRight[j] = leftSample * Mixer::param.gainADCLeft;
  }
}

static inline void processADCMonoRightChannelInput(uint32_t start,
                                                   uint32_t numFrames) {
  for (uint32_t i = start, j = 0; i < numFrames; ++j, i += 2) {
    float32_t rightSample{static_cast<float32_t>(ADCRxBuffer[i + 1]) *
                          Audio::Convert::Int32ToFloat};
    Mixer::chLeft[j] += rightSample * Mixer::param.gainADCRight;
    Mixer::chRight[j] += rightSample * Mixer::param.gainADCRight;
  }
}

static inline void processADCStereoInput(uint32_t start, uint32_t numFrames) {
  for (uint32_t i = start, j = 0; i < numFrames; ++j, i += 2) {
    float32_t leftSample{static_cast<float32_t>(ADCRxBuffer[i]) *
                         Audio::Convert::Int32ToFloat};
    float32_t rightSample{static_cast<float32_t>(ADCRxBuffer[i + 1]) *
                          Audio::Convert::Int32ToFloat};
    Mixer::chLeft[j] = leftSample * Mixer::param.gainADCLeft;
    Mixer::chRight[j] = rightSample * Mixer::param.gainADCRight;
  }
}

static inline void processFX(uint32_t numFrames) {
  for (uint32_t i = 0; i < FX::MaxChainCount; ++i) {
    if (FX::effectChain[i] != nullptr && !FX::isBypassed[i]) {
      FX::effectChain[i]->process(Mixer::chLeft.data(), Mixer::chRight.data(),
                                  numFrames);
    }
  }
}

static inline void mixMainUSBAudio(uint32_t numFrames, uint32_t numSamples) {
  static std::array<int16_t, AudioConfig::FramesPerBlock * 2> usbTemp;

  std::fill(usbTemp.begin(), usbTemp.end(), 0);
  mainUSBRxBuffer.read(usbTemp.data(), numSamples);

  for (uint32_t j = 0; j < numFrames; ++j) {
    float32_t usbLeft =
        static_cast<float32_t>(usbTemp[j * 2]) * Audio::Convert::Int16ToFloat;
    float32_t usbRight = static_cast<float32_t>(usbTemp[j * 2 + 1]) *
                         Audio::Convert::Int16ToFloat;
    Mixer::chLeft[j] += usbLeft * Mixer::param.gainMainUSB;
    Mixer::chRight[j] += usbRight * Mixer::param.gainMainUSB;
  }
}

static inline void processOutput(uint32_t start, uint32_t numFrames) {
  for (uint32_t i = start, j = 0; j < numFrames; ++j, i += 2) {
    monitorTxBuffer[i] =
        __SSAT(static_cast<int32_t>(Mixer::chLeft[j] * Mixer::param.gainMaster *
                                    Audio::Convert::FloatToInt32),
               32);
    monitorTxBuffer[i + 1] = __SSAT(
        static_cast<int32_t>(Mixer::chRight[j] * Mixer::param.gainMaster *
                             Audio::Convert::FloatToInt32),
        32);
  }
}

static void handleAudioBlock(uint32_t start, uint32_t end) {
  const uint32_t numSamples{end - start};
  const uint32_t numFrames{numSamples / 2};

  processADCMonoLeftChannelInput(start, numFrames);
  processADCMonoRightChannelInput(start, numFrames);
  processFX(numFrames);
  mixMainUSBAudio(numFrames, numSamples);
  processOutput(start, numFrames);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
  if (hsai == &hsai_BlockA4) {
    handleAudioBlock(0, AudioConfig::DMABufferSize / 2);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
  if (hsai == &hsai_BlockA4) {
    handleAudioBlock(AudioConfig::DMABufferSize / 2,
                     AudioConfig::DMABufferSize);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART3) {
    UI::processCommand(static_cast<char>(uartRxCmd));
    HAL_UART_Receive_IT(huart, &uartRxCmd, 1);
  }
}

void BSP_PB_Callback(Button_TypeDef Button) {
  if (Button == BUTTON_USER) {
    userButtonPressed = true;
  }
}
}
/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {

  /* Disables the MPU */
  HAL_MPU_Disable();
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
