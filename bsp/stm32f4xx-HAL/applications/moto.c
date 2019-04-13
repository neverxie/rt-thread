#include <stm32f4xx_hal.h>

void moto_init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStructure.Pin =  GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // PC2
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET); // PC3
}

INIT_APP_EXPORT(moto_init);
