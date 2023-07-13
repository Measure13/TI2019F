/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
bool volatile short_circuit = false;
bool volatile ch2_end_flag = false, ch3_end_flag = false;
uint32_t TIM2_Array[TIM2_MEDIAN_WINDOW];
uint32_t TIM3_Array[TIM3_MEDIAN_WINDOW];
uint16_t volatile TIM_IC_cnt = 0;
uint32_t TIM_final = 0;
uint8_t paper_num = 0;
static uint8_t CH2_cnt = 0, CH3_cnt = 0;
/* USER CODE END 0 */

TIM_HandleTypeDef htim2;

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0x200000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA1     ------> TIM2_CH2
    PA2     ------> TIM2_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
    PA1     ------> TIM2_CH2
    PA2     ------> TIM2_CH3
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
float median_f(float* data, uint8_t len, bool flag)
{
    if (len == 0)
    {
        return data[0];
    }
    float exchange = 0.0f;
    if (flag)
    {
        for (uint8_t i = 0; i < len - 1; ++i)
        {
            for (uint8_t j = i + 1; j < len; ++j)
            {
                if (data[i] < data[j])
                {
                    exchange = data[j];
                    data[j] = data[i];
                    data[i] = exchange;
                }
            }
        }
        if (len % 2)
        {
            return (data[len / 2] + data[len / 2 - 1]) / 2;
        }
        else
        {
            return data[len / 2];
        }
    }
    else
    {
        float* room = (float*)malloc(sizeof(float) * len * 2 + 1);
        for (uint8_t i = 0; i <= len; ++i)
        {
            room[len - i] = (data - i)[0];
            room[len + i] = (data + i)[0];
        }
        for (uint8_t i = 0; i < (2 * len); ++i)
        {
            for (uint8_t j = i + 1; j < (2 * len + 1); ++j)
            {
                if (room[i] < room[j])
                {
                    exchange = room[j];
                    room[j] = room[i];
                    room[i] = exchange;
                }
            }
        }
        exchange = room[len];
        free(room);
        return exchange;
    }
}

/// @brief get the median from a array
/// @param data the pointer to the array, whereas the position is determined by `flag`
/// @param len the length of the array
/// @param flag true for pointer to the head, false for pointer to the middle
/// @return the median(uint32)
uint32_t median_u(uint32_t* data, uint8_t len, bool flag)
{
    if (len == 0)
    {
        return data[0];
    }
    uint32_t exchange = 0.0f;
    if (flag)
    {
        for (uint8_t i = 0; i < len - 1; ++i)
        {
            for (uint8_t j = i + 1; j < len; ++j)
            {
                if (data[i] < data[j])
                {
                    exchange = data[j];
                    data[j] = data[i];
                    data[i] = exchange;
                }
            }
        }
        if (len % 2)
        {
            return (data[len / 2] + data[len / 2 - 1]) / 2;
        }
        else
        {
            return data[len / 2];
        }
    }
    else
    {
        uint32_t* room = (uint32_t*)malloc(sizeof(uint32_t) * len * 2 + 1);
        for (uint8_t i = 0; i <= len; ++i)
        {
            room[len - i] = (data - i)[0];
            room[len + i] = (data + i)[0];
        }
        for (uint8_t i = 0; i < (2 * len); ++i)
        {
            for (uint8_t j = i + 1; j < (2 * len + 1); ++j)
            {
                if (room[i] < room[j])
                {
                    exchange = room[j];
                    room[j] = room[i];
                    room[i] = exchange;
                }
            }
        }
        exchange = room[len];
        free(room);
        return exchange;
    }
}

void TIM2_Start(void)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC2);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC3);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_Base_Start_IT(&htim2);
}

void TIM3_Start(void)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC2);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC3);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
  HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
  if (mode == FLAG_MEASURE){
    short_circuit = true;
    HAL_TIM_Base_Stop_IT(&htim2);
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
  }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_IC_CaptureCallback could be implemented in the user file
   */
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {
    ++CH2_cnt;
    if (CH2_cnt >= 128)
    {
		  CH2_cnt = 0;
      TIM2_Array[TIM_IC_cnt++] = htim2.Instance->CCR2;
      __HAL_TIM_SET_COUNTER(&htim2, 0);
      if (TIM_IC_cnt >= TIM2_MEDIAN_WINDOW)
      {
        HAL_TIM_Base_Stop_IT(&htim2);
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
        TIM_final = median_u(TIM2_Array, TIM2_MEDIAN_WINDOW, true);
        if (mode == FLAG_CALIBRA)
        {
          cap_paper[paper_num] = TIM_final;
        }
        TIM_IC_cnt = 0;
        ch2_end_flag = true;
      }
    }
  }
  else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
  {
    ++CH3_cnt;
    if (CH3_cnt >= 8) //! because the 16 prescale in hardware
    {
		  CH3_cnt = 0;
      TIM3_Array[TIM_IC_cnt++] = htim2.Instance->CCR3;
      __HAL_TIM_SET_COUNTER(&htim2, 0);
      if (TIM_IC_cnt >= TIM3_MEDIAN_WINDOW)
      {
        HAL_TIM_Base_Stop_IT(&htim2);
        HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
        TIM_final = median_u(TIM3_Array, TIM3_MEDIAN_WINDOW, true);
        TIM_IC_cnt = 0;
        ch3_end_flag = true;
      }
    }
  }
}
/* USER CODE END 1 */
