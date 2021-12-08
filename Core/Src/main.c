/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

int sin_x(uint16_t angle);
const uint16_t sin_table[1024];

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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
const int PWM_MAX = 3361;  // Set this to the PWM generator counter period
const int BLDC_MAGNETS_PER_REV = 7; // Set this to the number of magnet pairs in the motor

volatile uint32_t motor_phase = 0;      // Desired phase location of the motor, in rotations, 0.32 format
volatile uint32_t motor_velocity = (0xFFFFFFFF / 1000);   // Desired motor velocity, in rotations per second per tick, 0.32 format
volatile uint16_t overall_power = 0xFFFF;      // 0xFFFF is full power

enum {
	CONTROL_6_PHASE,
	CONTROL_SINUSOIDAL
};
volatile int control_mode = CONTROL_SINUSOIDAL;

#define A_OFFSET (0)
#define B_OFFSET (2*1024/3)
#define C_OFFSET (1024/3)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
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
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  // Set all PWM outputs to idle, by setting the compare value to zero.
  // (For timer mode 1, the output is active only when the counter is below the compare value.)
  TIM2->CCR1 = 0;
  TIM2->CCR2 = 0;
  TIM2->CCR3 = 0;
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;

  // Now start up all the timers - 6 PWMs and one periodic timer
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_Base_Start_IT(&htim3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 3360;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3360;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 84;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	// This gets called every millisecond.
	// Compute the next desired motor position, and update
	// the PWM settings.
	int control_phase;
	int on_val = (overall_power * PWM_MAX) >> 16;

	// Update desired motor position
	motor_phase += motor_velocity;

	// Convert motor position to phase position in terms of number of complete electrical phases
	uint32_t phase_position = 0xffff & ((motor_phase >> 16) * BLDC_MAGNETS_PER_REV);

	// Determine PWM settings from desired phase position
	switch (control_mode)
	{
	case CONTROL_6_PHASE:
		control_phase = (phase_position * 6) >> 16;  // results in number from 0 to 5
		switch (control_phase)
		{
		case 0:
			// A+, B-
			TIM1->CCR1 = on_val;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = PWM_MAX;
			TIM2->CCR3 = 0;
			break;
		case 1:
			// A+, C-
			TIM1->CCR1 = on_val;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = PWM_MAX;
			break;
		case 2:
			// B+, C-
			TIM1->CCR1 = 0;
			TIM1->CCR2 = on_val;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = PWM_MAX;
			break;
		case 3:
			// B+, A-
			TIM1->CCR1 = 0;
			TIM1->CCR2 = on_val;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = PWM_MAX;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = 0;
			break;
		case 4:
			// C+, A-
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = on_val;
			TIM2->CCR1 = PWM_MAX;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = 0;
			break;
		case 5:
			// C+, B-
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = on_val;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = PWM_MAX;
			TIM2->CCR3 = 0;
			break;
		}
		break;
	case CONTROL_SINUSOIDAL:
		control_phase = (phase_position * 1024) >> 16;  // results in number from 0 to 1023
		switch (control_phase * 6 / 1024)
		{
		case 0:
			// PWM: A+, C+,   B- on
			TIM1->CCR1 = (on_val * sin_x(control_phase + A_OFFSET)) >> 15;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = (on_val * sin_x(control_phase + C_OFFSET)) >> 15;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = PWM_MAX;
			TIM2->CCR3 = 0;
			break;
		case 1:
			// PSM: C-, B-    A+ on
			TIM1->CCR1 = PWM_MAX;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = (on_val * -sin_x(control_phase + B_OFFSET)) >> 15;
			TIM2->CCR3 = (on_val * -sin_x(control_phase + C_OFFSET)) >> 15;
			break;
		case 2:
			// PWM: A+, B+,    C- on
			TIM1->CCR1 = (on_val * sin_x(control_phase + A_OFFSET)) >> 15;
			TIM1->CCR2 = (on_val * sin_x(control_phase + B_OFFSET)) >> 15;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = 0;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = PWM_MAX;
			break;
		case 3:
			// PWM: A-, C-    B+ on
			TIM1->CCR1 = 0;
			TIM1->CCR2 = PWM_MAX;
			TIM1->CCR3 = 0;
			TIM2->CCR1 = (on_val * -sin_x(control_phase + A_OFFSET)) >> 15;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = (on_val * -sin_x(control_phase + C_OFFSET)) >> 15;
			break;
		case 4:
			// PWM: B+, C+,   A- on
			TIM1->CCR1 = 0;
			TIM1->CCR2 = (on_val * sin_x(control_phase + B_OFFSET)) >> 15;
			TIM1->CCR3 = (on_val * sin_x(control_phase + C_OFFSET)) >> 15;
			TIM2->CCR1 = PWM_MAX;
			TIM2->CCR2 = 0;
			TIM2->CCR3 = 0;
			break;
		case 5:
			// PWM: A-, B-    C+ on
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = PWM_MAX;
			TIM2->CCR1 = (on_val * -sin_x(control_phase + A_OFFSET)) >> 15;
			TIM2->CCR2 = (on_val * -sin_x(control_phase + B_OFFSET)) >> 15;
			TIM2->CCR3 = 0;
			break;
		}
		break;
	}

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
}

// returns sin(angle), signed form, +- 0x8000.
// angle is assumed to be in 1024ths of a full cycle
int sin_x(uint16_t angle)
{
	return ((int) sin_table[angle % 1024] - 0x8000);
}
const uint16_t sin_table[1024] = {
		0x8000,0x80c9,0x8192,0x825b,0x8324,0x83ee,0x84b7,0x8580,
		0x8649,0x8712,0x87db,0x88a4,0x896c,0x8a35,0x8afe,0x8bc6,
		0x8c8e,0x8d57,0x8e1f,0x8ee7,0x8fae,0x9076,0x913e,0x9205,
		0x92cc,0x9393,0x945a,0x9521,0x95e7,0x96ad,0x9773,0x9839,
		0x98fe,0x99c4,0x9a89,0x9b4d,0x9c12,0x9cd6,0x9d9a,0x9e5e,
		0x9f21,0x9fe4,0xa0a7,0xa169,0xa22b,0xa2ed,0xa3af,0xa470,
		0xa530,0xa5f1,0xa6b1,0xa770,0xa830,0xa8ef,0xa9ad,0xaa6b,
		0xab29,0xabe6,0xaca3,0xad5f,0xae1b,0xaed7,0xaf92,0xb04d,
		0xb107,0xb1c0,0xb27a,0xb332,0xb3ea,0xb4a2,0xb559,0xb610,
		0xb6c6,0xb77c,0xb831,0xb8e5,0xb999,0xba4d,0xbb00,0xbbb2,
		0xbc64,0xbd15,0xbdc6,0xbe76,0xbf25,0xbfd4,0xc082,0xc12f,
		0xc1dc,0xc288,0xc334,0xc3df,0xc489,0xc533,0xc5dc,0xc684,
		0xc72c,0xc7d3,0xc879,0xc91f,0xc9c3,0xca67,0xcb0b,0xcbae,
		0xcc4f,0xccf1,0xcd91,0xce31,0xced0,0xcf6e,0xd00b,0xd0a8,
		0xd144,0xd1df,0xd279,0xd313,0xd3ac,0xd443,0xd4db,0xd571,
		0xd606,0xd69b,0xd72f,0xd7c2,0xd854,0xd8e5,0xd975,0xda05,
		0xda93,0xdb21,0xdbae,0xdc3a,0xdcc5,0xdd4f,0xddd9,0xde61,
		0xdee9,0xdf6f,0xdff5,0xe07a,0xe0fd,0xe180,0xe202,0xe283,
		0xe303,0xe382,0xe400,0xe47d,0xe4fa,0xe575,0xe5ef,0xe668,
		0xe6e0,0xe758,0xe7ce,0xe843,0xe8b7,0xe92b,0xe99d,0xea0e,
		0xea7e,0xeaed,0xeb5b,0xebc8,0xec34,0xec9f,0xed09,0xed72,
		0xedda,0xee41,0xeea7,0xef0b,0xef6f,0xefd1,0xf033,0xf093,
		0xf0f2,0xf150,0xf1ad,0xf209,0xf264,0xf2be,0xf316,0xf36e,
		0xf3c4,0xf41a,0xf46e,0xf4c1,0xf513,0xf564,0xf5b3,0xf602,
		0xf64f,0xf69b,0xf6e6,0xf730,0xf779,0xf7c1,0xf807,0xf84d,
		0xf891,0xf8d4,0xf916,0xf956,0xf996,0xf9d4,0xfa11,0xfa4d,
		0xfa88,0xfac1,0xfafa,0xfb31,0xfb67,0xfb9c,0xfbd0,0xfc02,
		0xfc33,0xfc63,0xfc92,0xfcc0,0xfcec,0xfd17,0xfd42,0xfd6a,
		0xfd92,0xfdb8,0xfdde,0xfe01,0xfe24,0xfe46,0xfe66,0xfe85,
		0xfea3,0xfec0,0xfedb,0xfef5,0xff0e,0xff26,0xff3c,0xff52,
		0xff66,0xff79,0xff8a,0xff9b,0xffaa,0xffb8,0xffc4,0xffd0,
		0xffda,0xffe3,0xffeb,0xfff1,0xfff6,0xfffa,0xfffd,0xffff,
		0xffff,0xfffe,0xfffc,0xfff8,0xfff4,0xffee,0xffe7,0xffdf,
		0xffd5,0xffca,0xffbe,0xffb1,0xffa2,0xff93,0xff82,0xff6f,
		0xff5c,0xff47,0xff31,0xff1a,0xff02,0xfee8,0xfece,0xfeb1,
		0xfe94,0xfe76,0xfe56,0xfe35,0xfe13,0xfdf0,0xfdcb,0xfda5,
		0xfd7e,0xfd56,0xfd2d,0xfd02,0xfcd6,0xfca9,0xfc7b,0xfc4b,
		0xfc1b,0xfbe9,0xfbb6,0xfb82,0xfb4c,0xfb16,0xfade,0xfaa5,
		0xfa6b,0xfa2f,0xf9f3,0xf9b5,0xf976,0xf936,0xf8f5,0xf8b2,
		0xf86f,0xf82a,0xf7e4,0xf79d,0xf755,0xf70c,0xf6c1,0xf675,
		0xf629,0xf5db,0xf58c,0xf53b,0xf4ea,0xf498,0xf444,0xf3ef,
		0xf399,0xf342,0xf2ea,0xf291,0xf237,0xf1db,0xf17f,0xf121,
		0xf0c3,0xf063,0xf002,0xefa0,0xef3d,0xeed9,0xee74,0xee0e,
		0xeda6,0xed3e,0xecd5,0xec6a,0xebff,0xeb92,0xeb24,0xeab6,
		0xea46,0xe9d6,0xe964,0xe8f1,0xe87d,0xe809,0xe793,0xe71c,
		0xe6a4,0xe62c,0xe5b2,0xe537,0xe4bc,0xe43f,0xe3c1,0xe343,
		0xe2c3,0xe243,0xe1c1,0xe13f,0xe0bc,0xe037,0xdfb2,0xdf2c,
		0xdea5,0xde1d,0xdd94,0xdd0a,0xdc80,0xdbf4,0xdb68,0xdada,
		0xda4c,0xd9bd,0xd92d,0xd89c,0xd80b,0xd778,0xd6e5,0xd651,
		0xd5bc,0xd526,0xd48f,0xd3f8,0xd35f,0xd2c6,0xd22c,0xd192,
		0xd0f6,0xd05a,0xcfbd,0xcf1f,0xce80,0xcde1,0xcd41,0xcca0,
		0xcbff,0xcb5c,0xcab9,0xca16,0xc971,0xc8cc,0xc826,0xc77f,
		0xc6d8,0xc630,0xc588,0xc4de,0xc434,0xc38a,0xc2de,0xc232,
		0xc186,0xc0d9,0xc02b,0xbf7c,0xbecd,0xbe1e,0xbd6d,0xbcbd,
		0xbc0b,0xbb59,0xbaa6,0xb9f3,0xb940,0xb88b,0xb7d6,0xb721,
		0xb66b,0xb5b5,0xb4fe,0xb446,0xb38e,0xb2d6,0xb21d,0xb164,
		0xb0aa,0xafef,0xaf34,0xae79,0xadbd,0xad01,0xac45,0xab88,
		0xaaca,0xaa0c,0xa94e,0xa88f,0xa7d0,0xa711,0xa651,0xa591,
		0xa4d0,0xa40f,0xa34e,0xa28c,0xa1ca,0xa108,0xa045,0x9f83,
		0x9ebf,0x9dfc,0x9d38,0x9c74,0x9bb0,0x9aeb,0x9a26,0x9961,
		0x989c,0x97d6,0x9710,0x964a,0x9584,0x94bd,0x93f7,0x9330,
		0x9269,0x91a1,0x90da,0x9012,0x8f4b,0x8e83,0x8dbb,0x8cf3,
		0x8c2a,0x8b62,0x8a99,0x89d1,0x8908,0x883f,0x8776,0x86ad,
		0x85e4,0x851b,0x8452,0x8389,0x82c0,0x81f7,0x812d,0x8064,
		0x7f9b,0x7ed2,0x7e08,0x7d3f,0x7c76,0x7bad,0x7ae4,0x7a1b,
		0x7952,0x7889,0x77c0,0x76f7,0x762e,0x7566,0x749d,0x73d5,
		0x730c,0x7244,0x717c,0x70b4,0x6fed,0x6f25,0x6e5e,0x6d96,
		0x6ccf,0x6c08,0x6b42,0x6a7b,0x69b5,0x68ef,0x6829,0x6763,
		0x669e,0x65d9,0x6514,0x644f,0x638b,0x62c7,0x6203,0x6140,
		0x607c,0x5fba,0x5ef7,0x5e35,0x5d73,0x5cb1,0x5bf0,0x5b2f,
		0x5a6e,0x59ae,0x58ee,0x582f,0x5770,0x56b1,0x55f3,0x5535,
		0x5477,0x53ba,0x52fe,0x5242,0x5186,0x50cb,0x5010,0x4f55,
		0x4e9b,0x4de2,0x4d29,0x4c71,0x4bb9,0x4b01,0x4a4a,0x4994,
		0x48de,0x4829,0x4774,0x46bf,0x460c,0x4559,0x44a6,0x43f4,
		0x4342,0x4292,0x41e1,0x4132,0x4083,0x3fd4,0x3f26,0x3e79,
		0x3dcd,0x3d21,0x3c75,0x3bcb,0x3b21,0x3a77,0x39cf,0x3927,
		0x3880,0x37d9,0x3733,0x368e,0x35e9,0x3546,0x34a3,0x3400,
		0x335f,0x32be,0x321e,0x317f,0x30e0,0x3042,0x2fa5,0x2f09,
		0x2e6d,0x2dd3,0x2d39,0x2ca0,0x2c07,0x2b70,0x2ad9,0x2a43,
		0x29ae,0x291a,0x2887,0x27f4,0x2763,0x26d2,0x2642,0x25b3,
		0x2525,0x2497,0x240b,0x237f,0x22f5,0x226b,0x21e2,0x215a,
		0x20d3,0x204d,0x1fc8,0x1f43,0x1ec0,0x1e3e,0x1dbc,0x1d3c,
		0x1cbc,0x1c3e,0x1bc0,0x1b43,0x1ac8,0x1a4d,0x19d3,0x195b,
		0x18e3,0x186c,0x17f6,0x1782,0x170e,0x169b,0x1629,0x15b9,
		0x1549,0x14db,0x146d,0x1400,0x1395,0x132a,0x12c1,0x1259,
		0x11f1,0x118b,0x1126,0x10c2,0x105f,0xffd,0xf9c,0xf3c,
		0xede,0xe80,0xe24,0xdc8,0xd6e,0xd15,0xcbd,0xc66,
		0xc10,0xbbb,0xb67,0xb15,0xac4,0xa73,0xa24,0x9d6,
		0x98a,0x93e,0x8f3,0x8aa,0x862,0x81b,0x7d5,0x790,
		0x74d,0x70a,0x6c9,0x689,0x64a,0x60c,0x5d0,0x594,
		0x55a,0x521,0x4e9,0x4b3,0x47d,0x449,0x416,0x3e4,
		0x3b4,0x384,0x356,0x329,0x2fd,0x2d2,0x2a9,0x281,
		0x25a,0x234,0x20f,0x1ec,0x1ca,0x1a9,0x189,0x16b,
		0x14e,0x131,0x117,0xfd,0xe5,0xce,0xb8,0xa3,
		0x90,0x7d,0x6c,0x5d,0x4e,0x41,0x35,0x2a,
		0x20,0x18,0x11,0xb,0x7,0x3,0x1,0x0,
		0x0,0x2,0x5,0x9,0xe,0x14,0x1c,0x25,
		0x2f,0x3b,0x47,0x55,0x64,0x75,0x86,0x99,
		0xad,0xc3,0xd9,0xf1,0x10a,0x124,0x13f,0x15c,
		0x17a,0x199,0x1b9,0x1db,0x1fe,0x221,0x247,0x26d,
		0x295,0x2bd,0x2e8,0x313,0x33f,0x36d,0x39c,0x3cc,
		0x3fd,0x42f,0x463,0x498,0x4ce,0x505,0x53e,0x577,
		0x5b2,0x5ee,0x62b,0x669,0x6a9,0x6e9,0x72b,0x76e,
		0x7b2,0x7f8,0x83e,0x886,0x8cf,0x919,0x964,0x9b0,
		0x9fd,0xa4c,0xa9b,0xaec,0xb3e,0xb91,0xbe5,0xc3b,
		0xc91,0xce9,0xd41,0xd9b,0xdf6,0xe52,0xeaf,0xf0d,
		0xf6c,0xfcc,0x102e,0x1090,0x10f4,0x1158,0x11be,0x1225,
		0x128d,0x12f6,0x1360,0x13cb,0x1437,0x14a4,0x1512,0x1581,
		0x15f1,0x1662,0x16d4,0x1748,0x17bc,0x1831,0x18a7,0x191f,
		0x1997,0x1a10,0x1a8a,0x1b05,0x1b82,0x1bff,0x1c7d,0x1cfc,
		0x1d7c,0x1dfd,0x1e7f,0x1f02,0x1f85,0x200a,0x2090,0x2116,
		0x219e,0x2226,0x22b0,0x233a,0x23c5,0x2451,0x24de,0x256c,
		0x25fa,0x268a,0x271a,0x27ab,0x283d,0x28d0,0x2964,0x29f9,
		0x2a8e,0x2b24,0x2bbc,0x2c53,0x2cec,0x2d86,0x2e20,0x2ebb,
		0x2f57,0x2ff4,0x3091,0x312f,0x31ce,0x326e,0x330e,0x33b0,
		0x3451,0x34f4,0x3598,0x363c,0x36e0,0x3786,0x382c,0x38d3,
		0x397b,0x3a23,0x3acc,0x3b76,0x3c20,0x3ccb,0x3d77,0x3e23,
		0x3ed0,0x3f7d,0x402b,0x40da,0x4189,0x4239,0x42ea,0x439b,
		0x444d,0x44ff,0x45b2,0x4666,0x471a,0x47ce,0x4883,0x4939,
		0x49ef,0x4aa6,0x4b5d,0x4c15,0x4ccd,0x4d85,0x4e3f,0x4ef8,
		0x4fb2,0x506d,0x5128,0x51e4,0x52a0,0x535c,0x5419,0x54d6,
		0x5594,0x5652,0x5710,0x57cf,0x588f,0x594e,0x5a0e,0x5acf,
		0x5b8f,0x5c50,0x5d12,0x5dd4,0x5e96,0x5f58,0x601b,0x60de,
		0x61a1,0x6265,0x6329,0x63ed,0x64b2,0x6576,0x663b,0x6701,
		0x67c6,0x688c,0x6952,0x6a18,0x6ade,0x6ba5,0x6c6c,0x6d33,
		0x6dfa,0x6ec1,0x6f89,0x7051,0x7118,0x71e0,0x72a8,0x7371,
		0x7439,0x7501,0x75ca,0x7693,0x775b,0x7824,0x78ed,0x79b6,
		0x7a7f,0x7b48,0x7c11,0x7cdb,0x7da4,0x7e6d,0x7f36,0x8000,
};

/* USER CODE END 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
