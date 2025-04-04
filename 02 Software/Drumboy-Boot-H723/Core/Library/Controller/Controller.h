#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <cctype>

#include "Dac.h"
#include "Global.h"
#include "Lcd.h"
#include "fatfs.h"
#include "main.h"
#include "sdmmc.h"
#include "stm32h7xx_hal.h"
#include "string.h"

extern "C" TIM_HandleTypeDef htim6;   // left button timer
extern "C" TIM_HandleTypeDef htim7;   // right button timer
extern "C" TIM_HandleTypeDef htim13;  // button long press timer
extern "C" TIM_HandleTypeDef htim14;  // play timer
extern "C" TIM_HandleTypeDef htim15;  // text timer
extern "C" TIM_HandleTypeDef htim16;  // power button timer
extern "C" TIM_HandleTypeDef htim17;  // sd check timer
extern "C" TIM_HandleTypeDef htim23;  // beat sync timer

extern "C" uint32_t crcValue;

extern "C" uint8_t dataSd[1024];
extern "C" uint8_t dataFlash[1024];

class Controller {
   private:
    bool sdBusy;
    bool sdInsertCheck;

    bool power;

    bool powerButtonFlag;
    uint8_t powerButtonCounter;

    bool lcdTest;
    bool buttonTrigger;

   public:
    Controller();
    ~Controller();

    Lcd lcd;
    Dac dac;
    Sd sd;
    Keyboard keyboard;

    bool getPower() { return power; }
    void setPower(bool power_) { power = power_; }

    bool getLcdTest() { return lcdTest; }
    void setLcdTest(bool test_) { lcdTest = test_; }

    bool getButtonTrigger() { return buttonTrigger; }
    void setButtonTrigger(bool trigger_) { buttonTrigger = trigger_; }

    void initialize();
    void systemStart();
    void systemReset();
    void systemUpdate();
    void updateFirmware();
    void startApplication();

    /* Crc functions -----------------------------------------------------------*/

    uint32_t crc_update(uint32_t initial, const void* buf, uint32_t len);

    /* Flash functions ---------------------------------------------------------*/

    uint32_t flash_getSector(uint32_t address);
    uint32_t flash_eraseSector(uint32_t startSectorAddress, uint32_t wordSize);
    uint32_t flash_writeData(uint32_t StartSectorAddress, uint32_t* data, uint32_t wordSize);
    void flash_readData(uint32_t StartSectorAddress, uint32_t* data, uint32_t wordSize);

    /* Button functions --------------------------------------------------------*/

    void button_check();

    /* Keyboard functions ------------------------------------------------------*/

    void keyboard_initialize();
    void keyboard_check();

    void setLeftButtonState(ButtonState state_) {
        keyboard.leftButtonState = state_;
    }
    ButtonState getLeftButtonState() const {
        return keyboard.leftButtonState;
    }
    void setRightButtonState(ButtonState state_) {
        keyboard.rightButtonState = state_;
    }
    ButtonState getRightButtonState() const {
        return keyboard.rightButtonState;
    }
    void startLeftButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim6);
    }
    void stopLeftButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim6);
    }
    void startRightButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim7);
    }
    void stopRightButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim7);
    }
    void startLongButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim13);
    }
    void stopLongButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim13);
        __HAL_TIM_SET_COUNTER(&htim13, 0);
    }
    void startTextTimer() {
        HAL_TIM_Base_Start_IT(&htim15);
    }
    void stopTextTimer() {
        HAL_TIM_Base_Stop_IT(&htim15);
        __HAL_TIM_SET_COUNTER(&htim15, 0);
    }
    void startPowerButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim16);
        powerButtonCounter = 0;
    }
    void stopPowerButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim16);
        __HAL_TIM_SET_COUNTER(&htim16, 0);
    }
    void startSdTimer() {
        HAL_TIM_Base_Start_IT(&htim17);
    }
    void stopSdTimer() {
        HAL_TIM_Base_Stop_IT(&htim17);
        __HAL_TIM_SET_COUNTER(&htim17, 0);
    }

    /* Dac functions -----------------------------------------------------------*/

    void dac_initialize();

    /* Sd functions ------------------------------------------------------------*/

    void sd_initialize();
    SdResult sd_detect();
    SdResult sd_mount();
    SdResult sd_unmount();
    SdResult sd_getSpace();
    SdResult sd_checkFileExist(const char* fileAddress);
    SdResult sd_checkFolderExist(const char* folderAddress);
    FRESULT sd_createDirectory(const char* path);
    FRESULT sd_deleteDirectory(const char* path);

    /* Lcd functions -----------------------------------------------------------*/

    void lcd_initialize();
    void lcd_test();

    /* Interrupt functions -----------------------------------------------------*/

    void interruptLeftButtonTrigger();
    void interruptRightButtonTrigger();
    void interruptLeftButtonRead();
    void interruptRightButtonRead();
    void interruptPowerButtonRead();

    void interruptSd();

    /* Debug functions ---------------------------------------------------------*/

    void check(int32_t num_, uint8_t line_) {
        lcd.setAlignment(LEFT);
        lcd.setFont(FONT_05x07);
        lcd.setForeColor(WHITE);
        lcd.setBackColor(BLACK);
        lcd.drawNumber(num_, 8, 30, 30 + (line_ * 10));
    }
};

#endif
