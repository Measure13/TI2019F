/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "USART_HMI.h"
#include "arm_math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POLY 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t cap_paper[MAX_PAPER_NUM + 1]; // less paper more cap more count
bool volatile mode = true;
static const float32_t inv_array[] = {149999.999999994f, -2024999.9999998903f, 7199999.999999538f, -9374999.999999303f, 4049999.9999996773f, -28499.999999998538f, 452249.99999997334f, -1727999.9999998873f, 2343749.9999998324f, -1039499.9999999213f, 1983.3333333332082f, -35099.999999997715f, 145599.99999999028f, -208333.33333331897f, 95849.99999999322f, -59.999999999995495f, 1147.4999999999177f, -5119.999999999648f, 7812.49999999948f, -3779.999999999754f, 0.6666666666666087f, -13.499999999998941f, 63.99999999999545f, -104.16666666665994f, 53.99999999999682f};
static float32_t X_30[(MAX_PAPER_NUM + 1) * POLY];
static float32_t Y_5[POLY];
static float32_t coef_5[POLY];
static float32_t Y_30[MAX_PAPER_NUM + 1];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void testing_transition(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Fit_Cap_Curve(void)
{
	volatile arm_status ast;
  arm_matrix_instance_f32 inv = {POLY, POLY, (float32_t*)inv_array};
  arm_matrix_instance_f32 Y = {MAX_PAPER_NUM + 1, 1, (float32_t*)Y_30};
  for (uint8_t i = 0; i < POLY; ++i)
  {
    Y_5[i] = (float32_t)cap_paper[i * 5 + 10];
  }
  arm_matrix_instance_f32 y = {POLY, 1, (float32_t*)Y_5};
  arm_matrix_instance_f32 coef = {POLY, 1, (float32_t*)coef_5};
  ast = arm_mat_mult_f32(&inv, &y, &coef);
  for (uint8_t i = 5; i < MAX_PAPER_NUM + 1; ++i)
  {
    for (uint8_t j = 0; j < POLY; ++j)
    {
      X_30[i * POLY + j] = 1.0f / powf(i, POLY - 1 - j);
    }
  }
  arm_matrix_instance_f32 X = {MAX_PAPER_NUM + 1, POLY, (float32_t*)X_30};
  ast = arm_mat_mult_f32(&X, &coef, &Y);
  for (uint8_t i = 1; i < MAX_PAPER_NUM + 1; ++i)
  {
    if (cap_paper[i] == 0.0f)
    {
      cap_paper[i] = (uint32_t)Y_30[i];
    }
  }
}

uint8_t Get_Paper_Number(uint32_t tim)
{
  for (uint8_t i = 1; i <= MAX_PAPER_NUM; ++i)
  {
    if (tim >= cap_paper[i])
    {
      return i;
    }
    else
    {
      if (i == MAX_PAPER_NUM)
      {
        return MAX_PAPER_NUM;
      }
      else if (1.0f / (tim * tim) < (1.0f / (cap_paper[i] * cap_paper[i]) + 1.0f / (cap_paper[i + 1] * cap_paper[i + 1]) / 2))
      {
        return i;
      }
    }
    
  }
  return 0;
}

void testing_transition(void)
{
  cap_paper[10] = 0x23d1;
  cap_paper[15] = 0x1b90;
  cap_paper[20] = 0x16a5;
  cap_paper[25] = 0x1354;
  cap_paper[30] = 0x1206;
	Fit_Cap_Curve();
	mode = FLAG_MEASURE;
	for (uint8_t i = 0; i < 12; ++i)
	{
		printf("vis b%d,0\xff\xff\xff", i);
		HAL_Delay(1);
	}
	printf("vis bt0,0\xff\xff\xff");
	HAL_Delay(1);
	printf("vis n0,0\xff\xff\xff");
	HAL_Delay(1);
	UARTHMI_Visibility_Change(1, 0);
}
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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
 
  UARTHMI_Forget_It();
  UARTHMI_Reset();
  HAL_Delay(150);
  testing_transition();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // if (TIM_final != 0)
    // {
    //   printf("\nTIME:%lu %.10lfs\n", TIM_final, (double)(TIM_final + 1) / (TIM2_CLOCK));
    //   TIM_final = 0;
    //   __HAL_TIM_SET_COUNTER(&htim2, 0);
    //   __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
    //   HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
    //   HAL_TIM_Base_Start_IT(&htim2);
    // }
    if (mode)
    {
      if (end_flag)
      {
        end_flag = false;
        printf("n0.pco=64512\xff\xff\xff");
      }
    }
    else
    {
      if (recving)
      {
        while (htim2.Instance->DIER & TIM_IT_CC2);
        if (short_circuit)
        {
          UARTHMI_Visibility_Change(2, 1);
          short_circuit = false;
        }
        else
        {
          UARTHMI_Visibility_Change(2, 0);
          UARTHMI_Send_Number(1, Get_Paper_Number(TIM_final));
			BEEP;
			HAL_Delay(500);
			NOBB;
        }
		recving = false;
      }
    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
