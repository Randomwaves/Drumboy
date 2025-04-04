#include "main.h"

#include "Controller.h"
#include "dma.h"
#include "fatfs.h"
#include "fmc.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "sdmmc.h"
#include "tim.h"
#include "usart.h"

extern "C" void SystemClock_Config();
extern "C" void PeriphCommonClock_Config();
extern "C" void System_Initialize();
extern "C" void Sdram_Initialize();

uint8_t syncInData[2];
uint8_t syncOutData[2];

uint8_t midiRxData;

Controller controller;

int main() {
    HAL_Init();

    SystemClock_Config();
    PeriphCommonClock_Config();

    System_Initialize();
    Sdram_Initialize();

    controller.initialize();

    while (!controller.getPower()) {
        controller.systemUpdate_A();
    }

    controller.systemStart();

    while (controller.getPower()) {
        controller.systemUpdate_B();
    }

    controller.systemReset();
}

extern "C" void SystemClock_Config(void) {
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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 68;
    RCC_OscInitStruct.PLL.PLLP = 1;
    RCC_OscInitStruct.PLL.PLLQ = 3;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 6144;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
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

    /** Enables the Clock Security System
     */
    HAL_RCC_EnableCSS();
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
extern "C" void PeriphCommonClock_Config(void) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_SPI3;
    PeriphClkInitStruct.PLL2.PLL2M = 1;
    PeriphClkInitStruct.PLL2.PLL2N = 30;
    PeriphClkInitStruct.PLL2.PLL2P = 1;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 1;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }
}

extern "C" void System_Initialize() {
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_FMC_Init();
    MX_FATFS_Init();
    MX_I2C3_Init();
    MX_I2S3_Init();
    MX_SDMMC2_SD_Init();
    MX_UART4_Init();
    MX_UART7_Init();
    MX_USART1_UART_Init();
    MX_USART6_UART_Init();
    MX_TIM5_Init();
    MX_TIM6_Init();
    MX_TIM7_Init();
    MX_TIM12_Init();
    MX_TIM13_Init();
    MX_TIM14_Init();
    MX_TIM15_Init();
    MX_TIM16_Init();
    MX_TIM17_Init();
    MX_TIM23_Init();
    MX_TIM24_Init();
}

extern "C" void Sdram_Initialize() {
    SDRAM_HandleTypeDef *hsdram = &hsdram1;
    FMC_SDRAM_CommandTypeDef *Command;

    __IO uint32_t tmpmrd = 0;
    /* Step 3:  Configure a clock configuration enable command */
    Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 4: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 5: Configure a PALL (precharge all) command */
    Command->CommandMode = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 6 : Configure a Auto-Refresh command */
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 4;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 7: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_2 |
             SDRAM_MODEREG_OPERATING_MODE_STANDARD |
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 8: Set the refresh rate counter */
    /* (15.62 us x Freq) - 20 */
    /* Set the device refresh counter */
    HAL_SDRAM_ProgramRefreshRate(hsdram, 1854);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // trigger left button
    if ((GPIO_Pin == CT0_SDO_Pin) && (controller.getLeftButtonState() == PASSIVE)) {
        controller.interruptLeftButtonTrigger();
    }
    // trigger right button
    if ((GPIO_Pin == CT1_SDO_Pin) && (controller.getRightButtonState() == PASSIVE)) {
        controller.interruptRightButtonTrigger();
    }
    // trigger layer0 button
    if (GPIO_Pin == CT2_D0_Pin) {
        (controller.getLayerTab() == TAB_0) ? controller.keyboard.layerButtonPress[4] = true : controller.keyboard.layerButtonPress[9] = true;
    }
    // trigger layer1 button
    if (GPIO_Pin == CT2_D1_Pin) {
        (controller.getLayerTab() == TAB_0) ? controller.keyboard.layerButtonPress[3] = true : controller.keyboard.layerButtonPress[8] = true;
    }
    // trigger layer2 button
    if (GPIO_Pin == CT2_D2_Pin) {
        (controller.getLayerTab() == TAB_0) ? controller.keyboard.layerButtonPress[2] = true : controller.keyboard.layerButtonPress[7] = true;
    }
    // trigger layer3 button
    if (GPIO_Pin == CT2_D3_Pin) {
        (controller.getLayerTab() == TAB_0) ? controller.keyboard.layerButtonPress[1] = true : controller.keyboard.layerButtonPress[6] = true;
    }
    // trigger layer4 button
    if (GPIO_Pin == CT2_D4_Pin) {
        (controller.getLayerTab() == TAB_0) ? controller.keyboard.layerButtonPress[0] = true : controller.keyboard.layerButtonPress[5] = true;
    }
    // trigger sync in
    /*
    if (GPIO_Pin == SYNC_IN_Pin) {
        if ((controller.getMenu() != INIT_MENU) && (controller.getSystem().syncIn))
            HAL_UART_Receive_DMA(&huart7, syncInData, sizeof(syncInData));
    }
    */
    // trigger button
    if (GPIO_Pin == BUTTON_Pin) {
    }
};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // effect transition timer
    if (htim->Instance == TIM5) {
        controller.interruptTransition();
    }
    // left button
    if ((htim->Instance == TIM6) && ((controller.getLeftButtonState() != PASSIVE))) {
        controller.interruptLeftButtonRead();
    }
    // right button
    if ((htim->Instance == TIM7) && ((controller.getRightButtonState() != PASSIVE))) {
        controller.interruptRightButtonRead();
    }
    // updown button
    if (htim->Instance == TIM12) {
        controller.interruptUpDownButtonRead();
    }
    // long button
    if (htim->Instance == TIM13) {
        controller.interruptLongButtonRead();
    }
    // play
    if (htim->Instance == TIM14) {
        controller.interruptPlay();
    }
    // text
    if (htim->Instance == TIM15) {
        controller.interruptText();
    }
    // power button
    if (htim->Instance == TIM16) {
        controller.interruptPowerButtonRead();
    }
    // sd check
    if (htim->Instance == TIM17) {
        controller.interruptSd();
    }
    // beat sync timer
    if (htim->Instance == TIM23) {
        controller.interruptBeatSync();
    }
    // limit alert timer
    if (htim->Instance == TIM24) {
        controller.interruptLimitAlert();
    }
};

void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim) {}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
    if (hi2s == &hi2s3) {
        controller.interruptAudioSend();
        controller.interruptAudioMetronome();
        controller.interruptAudioSong();
        controller.interruptAudioLpf();
        controller.interruptAudioEq();
        controller.interruptAudioFilter();
        controller.interruptAudioEffect();
        controller.interruptAudioReverb();
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s) {}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart4) {
    }

    if (huart == &huart6) {
        controller.midiTxBusy = false;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart7) {
        controller.receiveSyncCommand();
        HAL_UART_Receive_DMA(&huart7, syncInData, sizeof(syncInData));
    }

    if (huart == &huart1) {
        controller.receiveMidiCommand();
        HAL_UART_Receive_DMA(&huart1, &midiRxData, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart7) {
        HAL_UART_Receive_DMA(&huart7, syncInData, sizeof(syncInData));
    }

    if (huart == &huart1) {
        HAL_UART_Receive_DMA(&huart1, &midiRxData, 1);
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line) {}

#endif
