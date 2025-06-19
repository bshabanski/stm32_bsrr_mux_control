#include "stm32f1xx_hal.h"

TIM_HandleTypeDef htim2;

const uint32_t gpioStates[4] = {
    0x3,  // Hi-Z
    0xE,  // Reset phase
    0xB,  // Break-Before-Make
    0x9   // Pre-charge phase
};
const uint32_t timerPeriods[4] = {
    41 - 1,
    17 - 1,
    2 - 1,
    2 - 1
};

volatile uint8_t currentStateIndex = 0;

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_All;  // Enable all 16 if you're using PA0–PA11
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 3 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = timerPeriods[0];
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Update GPIO state
        uint32_t pinsToSet = gpioStates[currentStateIndex];
        uint32_t pinsToReset = ~pinsToSet & 0x0000FFFF;  // Only care about 16 GPIO bits

        GPIOA->BSRR = (pinsToReset << 16) | pinsToSet;

        // Update timer period
        currentStateIndex = (currentStateIndex + 1) % 4;
        __HAL_TIM_SET_AUTORELOAD(&htim2, timerPeriods[currentStateIndex]);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();  // Your clock setup

    MX_GPIO_Init();
    MX_TIM2_Init();

    while (1)
    {
        // Nothing needed here — GPIO updates on timer interrupts
    }
}
