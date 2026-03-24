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
#include <algorithm>
#include <array>
#include "arm_math.h"
#include "audio_constants.hpp"
#include "effector.hpp"
#include "audio_ring_buffer.hpp"
#include "bass_preamp.hpp"
#include "bass_distortion.hpp"
#include "auto_wah.hpp"
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

/* USER CODE BEGIN PV */
// Constants
constexpr uint32_t kSamplesRate{ 48000 };			// 48kHz / 1000ms = 48samples/ms
constexpr uint32_t kChannels{ 2 };					// Stereo(2ch)
constexpr uint32_t kLatencyMs{ 1 };					// 2 frames

constexpr uint32_t kFramesPerBlock{ 48 };
constexpr uint32_t kSamplesPerBlock{ kFramesPerBlock * kChannels };
constexpr uint32_t kBufferSize{ kSamplesPerBlock * 2 }; // Double buffering

// Buffer SEttings
__attribute__((section(".sram4"), aligned(4)))
std::array<int32_t, kBufferSize> txBuffer{};

__attribute__((section(".sram4"), aligned(4)))
std::array<int32_t, kBufferSize> rxBuffer{};

AudioRingBuffer<int16_t, 4096> mainUsbBuffer;

// Effector Settings
const int kMaxChainCount = 5;
static std::array<Effector*, kMaxChainCount> effectChain{};
static std::array<bool, kMaxChainCount> isBypassed{};

static std::array<float32_t, kFramesPerBlock> s_left{};
static std::array<float32_t, kFramesPerBlock> s_right{};

// Instances
float sampleRate{ Constants::Config::kDefaultSampleRate };
BassPreAmp sadowskyPreAmp{ sampleRate };
bassDistortion distortion{};
AutoWah autoWah{ sampleRate };

// Debugging
uint32_t d_available{};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_BDMA_Init(void);
static void MX_GPIO_Init(void);
static void MX_SAI4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  isBypassed.fill(true);

  effectChain[0] = &sadowskyPreAmp;
  isBypassed[0] = false;

  effectChain[1] = &autoWah;
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  /* USER CODE BEGIN 2 */
  auto* pTx{ reinterpret_cast<uint8_t*>(txBuffer.data()) };
  auto* pRx{ reinterpret_cast<uint8_t*>(rxBuffer.data()) };

  if (HAL_SAI_Transmit_DMA(&hsai_BlockB4, pTx, kBufferSize) != HAL_OK) {
  	Error_Handler();
  }
  if (HAL_SAI_Receive_DMA(&hsai_BlockA4, pRx, kBufferSize) != HAL_OK) {
  	Error_Handler();
  }

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  	if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED) {
				isBypassed[1] = !isBypassed[1];
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
				HAL_Delay(300);
			}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI4A|RCC_PERIPHCLK_SAI4B;
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
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SAI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SAI4_Init(void)
{

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
  if (HAL_SAI_InitProtocol(&hsai_BlockA4, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK)
  {
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
  if (HAL_SAI_InitProtocol(&hsai_BlockB4, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SAI4_Init 2 */

  /* USER CODE END SAI4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_BDMA_Init(void)
{

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
static void MX_GPIO_Init(void)
{
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
	void mainUsbBufferWrite(int16_t* data, uint32_t length) { mainUsbBuffer.write(data, length); }
	void mainUsbBufferReset() {	mainUsbBuffer.reset(); }
	int32_t mainUsbBufferGetAvailableFrames(void) { return (int32_t)mainUsbBuffer.getAvailableFrames(); }

	static void handleAudioBlock(uint32_t start, uint32_t end) {
		const uint32_t numSamples{ end - start };
		const uint32_t numFrames{ numSamples / 2 };
		d_available = mainUsbBuffer.getAvailableFrames();

		// --- 1: De-interleave and Normalize (Mono Conversion) ---
		for (uint32_t i = start, j = 0; i < end; i += 2, ++j) {
			// Ignore right channel signal (rxBuffer[i + 1])
			s_left[j]  = static_cast<float32_t>(rxBuffer[i]) * Constants::Math::kInt32ToFloat;
			s_right[j] = static_cast<float32_t>(rxBuffer[i]) * Constants::Math::kInt32ToFloat;
		}

		// --- 2: Sequential Effect Chain Processing ---
		for (uint32_t i = 0; i < kMaxChainCount; ++i) {
			if (effectChain[i] != nullptr && !isBypassed[i]) {
				effectChain[i]->process(s_left.data(), s_right.data(), numFrames);
			}
		}

		// --- 3: Mixing Multi Channel Signals ---
		static std::array<int16_t, kFramesPerBlock * 2> usb_temp_i16;

		mainUsbBuffer.read(usb_temp_i16.data(), numSamples);

		for (uint32_t j = 0; j < numFrames; ++j) {
			float32_t usb_l = static_cast<float32_t>(usb_temp_i16[j * 2]) * Constants::Math::kInt16ToFloat;
			float32_t usb_r = static_cast<float32_t>(usb_temp_i16[j * 2 + 1]) * Constants::Math::kInt16ToFloat;

			s_left[j]  += usb_l;
			s_right[j] += usb_r;
		}

		// --- 4: Denormalize, Saturate, and Re-interleave ---
		for (uint32_t i = start, j = 0; i < end; i += 2, ++j) {
			txBuffer[i]     = __SSAT(static_cast<int32_t>(s_left[j] * Constants::Math::kFloatToInt32), 31);
			txBuffer[i + 1] = __SSAT(static_cast<int32_t>(s_right[j] * Constants::Math::kFloatToInt32), 31);
		}
	}

	void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
		if (hsai == &hsai_BlockA4) {
			handleAudioBlock(0, kBufferSize / 2);
		}
	}

	void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
		if (hsai == &hsai_BlockA4) {
			handleAudioBlock(kBufferSize / 2, kBufferSize);
		}
	}
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{

  /* Disables the MPU */
  HAL_MPU_Disable();
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
