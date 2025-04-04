#include "Controller.h"

Controller::Controller()
    : lcd(LANDSCAPE_1, WHITE, BLACK, FONT_07x09),
      dac(),
      sd(),
      keyboard(),

      sdBusy(false),
      sdInsertCheck(false),

      powerButtonFlag(false),
      powerButtonCounter(0),

      power(false),

      lcdTest(false),
      buttonTrigger(false) {}

Controller::~Controller() {}

void Controller::initialize() {
    keyboard_initialize();
    // dac_initialize();
    // lcd_initialize();
    LED0_ON;

    startPowerButtonTimer();
    stopPowerButtonTimer();
}

void Controller::systemStart() {
    // lcd.displayOn();
    // dac.audioOn();

    sd_initialize();
    updateFirmware();
    startApplication();
}

void Controller::systemReset() {
    lcd.displayOff();
    HAL_Delay(1000);
    dac.audioOff();
    NVIC_SystemReset();
}

void Controller::systemUpdate() {
    keyboard_check();
    button_check();
}

void Controller::updateFirmware() {
    FwResult fwResult = FW_ERROR;

    if (sd_mount() == SD_OK) {
        sd.fresult = f_findfirst(&sd.dir, &sd.fileInfo, "", "Drumboy_Fw?*.bin");
        if (sd.fileInfo.fname[0] != '\0') {
            if (sd_checkFileExist(sd.fileInfo.fname) == SD_OK) {
                if (f_open(&sd.file, sd.fileInfo.fname, FA_READ) == FR_OK) {
                    char data[40] = "";
                    char headerTitle[] = "RW_DRUMBOY_FIRMWARE     ";

                    if (f_read(&sd.file, data, 24, &sd.bytesread) == FR_OK) {
                        lcd.initialize();
                        lcd.displayOn();

                        HAL_Delay(500);

                        if (strcmp(data, headerTitle) == 0) {
                            memset(data, 0x00, sizeof(data));
                            if (f_read(&sd.file, data, 12, &sd.bytesread) == FR_OK) {
                                uint16_t *versionMajor = (uint16_t *)&(data[0]);
                                uint16_t *versionMinor = (uint16_t *)&(data[2]);
                                uint32_t *firmwareSize = (uint32_t *)&(data[4]);
                                uint32_t *crcRef = (uint32_t *)&(data[8]);

                                uint32_t readSize = 10240;
                                uint32_t fileSize = f_size(&sd.file);
                                uint16_t readCount = *firmwareSize / readSize;
                                uint16_t remainder = *firmwareSize % readSize;

                                if ((*versionMajor < 10) && (*versionMinor < 100)) {
                                    if (((*firmwareSize + 36) == fileSize) && (*firmwareSize < 917504)) {
                                        for (uint16_t i = 0; i < (readCount); i++) {
                                            f_read(&sd.file, (uint8_t *)(RAM_FIRMWARE_ADDRESS + (i * readSize)), readSize, &sd.bytesread);
                                        }
                                        f_read(&sd.file, (uint8_t *)(RAM_FIRMWARE_ADDRESS + (readCount * readSize)), remainder, &sd.bytesread);

                                        uint32_t crcValue = 0;
                                        uint8_t *ptr = (uint8_t *)(RAM_FIRMWARE_ADDRESS);

                                        for (uint32_t i = 0; i < (*firmwareSize); i++) {
                                            crcValue = crc_update(crcValue, ptr, 1);
                                            ptr += 1;
                                        }

                                        if (crcValue == (*crcRef)) {
                                            lcd.drawSdFirmwareAlert(true);

                                            lcd.setForeColor(GREEN);
                                            lcd.setBackColor(BLACK);
                                            lcd.setFont(FONT_07x09);
                                            lcd.setAlignment(CENTER);
                                            uint16_t xPos = 327;
                                            uint16_t yPos = 382;
                                            lcd.drawHLine(xPos, yPos, 200);
                                            lcd.drawHLine(xPos, yPos + 24, 200);
                                            lcd.drawHalfCircle(xPos - 1, yPos + 12, 12, 1);
                                            lcd.drawHalfCircle(xPos + 200, yPos + 12, 12, 3);

                                            lcd.drawText("UPDATING FIRMWARE", 17, 427, 390);

                                            lcd.setForeColor(GRAY_25);
                                            lcd.drawHLine(327, 335, 200);

                                            lcd.setForeColor(GREEN);
                                            lcd.setBackColor(BLACK);
                                            lcd.setFont(FONT_05x07);
                                            lcd.setAlignment(CENTER);
                                            char textVersion[7];
                                            sprintf(textVersion, "V%01d.%02d", *versionMajor, *versionMinor);
                                            lcd.drawText(textVersion, strlen(textVersion), 427, 308);

                                            if (flash_eraseSector(FLASH_FIRMWARE_ADDRESS, fileSize / 4) == HAL_OK) {
                                                HAL_Delay(250);
                                                if (flash_writeData(FLASH_FIRMWARE_ADDRESS, (uint32_t *)(RAM_FIRMWARE_ADDRESS), (*firmwareSize) / 4) == HAL_OK) {
                                                    fwResult = FW_OK;
                                                    lcd.setForeColor(GREEN);
                                                    lcd.setBackColor(BLACK);
                                                    for (uint8_t i = 0; i < 200; i++) {
                                                        lcd.drawPixel(327 + i, 335);
                                                        HAL_Delay(5);
                                                    }
                                                    HAL_Delay(500);
                                                }
                                            }
                                        } else {
                                            fwResult = FW_ERROR_CRC;
                                        }
                                    } else {
                                        fwResult = FW_ERROR_SIZE;
                                    }
                                } else {
                                    fwResult = FW_ERROR_VERSION;
                                }
                            } else {
                                fwResult = FW_ERROR_FORMAT;
                            }
                        }

                        if (fwResult == FW_OK) {
                            lcd.setForeColor(GREEN);
                            lcd.setBackColor(BLACK);
                            lcd.setFont(FONT_07x09);
                            lcd.setAlignment(CENTER);
                            lcd.drawText(" FIRMWARE UPDATED ", 18, 427, 390);
                        } else {
                            lcd.drawSdFirmwareAlert(false);

                            lcd.setForeColor(RED);
                            lcd.setBackColor(BLACK);
                            lcd.setFont(FONT_07x09);
                            lcd.setAlignment(CENTER);
                            uint16_t xPos = 327;
                            uint16_t yPos = 382;
                            lcd.drawHLine(xPos, yPos, 200);
                            lcd.drawHLine(xPos, yPos + 24, 200);
                            lcd.drawHalfCircle(xPos - 1, yPos + 12, 12, 1);
                            lcd.drawHalfCircle(xPos + 200, yPos + 12, 12, 3);
                            lcd.drawText("  FIRMWARE ERROR  ", 18, 427, 390);

                            lcd.setFont(FONT_05x07);
                            switch (fwResult) {
                                case FW_ERROR:
                                    lcd.drawText("FORMAT-CHECK", 12, 427, 330);
                                    break;

                                case FW_ERROR_FORMAT:
                                    lcd.drawText("FORMAT-CHECK", 12, 427, 330);
                                    break;

                                case FW_ERROR_VERSION:
                                    lcd.drawText("VERSION-CHECK", 13, 427, 330);
                                    break;

                                case FW_ERROR_SIZE:
                                    lcd.drawText("SIZE-CHECK", 10, 427, 330);
                                    break;

                                case FW_ERROR_CRC:
                                    lcd.drawText("CRC-CHECK", 9, 427, 330);
                                    break;

                                default:
                                    break;
                            }
                        }
                    }
                    f_close(&sd.file);
                    if (fwResult == FW_OK) {
                        sd_deleteDirectory("System/Firmware");
                        sd_createDirectory("System/Firmware");
                        char fName[sizeof(sd.fileInfo.fname) + 16];
                        strcpy(fName, "System/Firmware/");
                        strcat(fName, sd.fileInfo.fname);
                        f_rename(sd.fileInfo.fname, fName);
                    } else {
                        f_unlink(sd.fileInfo.fname);
                    }
                    HAL_Delay(2500);
                    lcd.clearSdFirmwareAlert();
                }
                f_close(&sd.file);
            }
        }
    }
}

void Controller::startApplication() {
    void (*SysMemBootJump)(void);
    SysMemBootJump = (void (*)(void))(*((uint32_t *)(FLASH_FIRMWARE_ADDRESS + 4)));
    __set_MSP(*(uint32_t *)FLASH_FIRMWARE_ADDRESS);
    SysMemBootJump();

    while (true) {
        HAL_Delay(500);
        LED1_TOGGLE;
    }
}

/* Crc functions -------------------------------------------------------------*/

uint32_t Controller::crc_update(uint32_t initial, const void *buf, uint32_t len) {
    uint32_t c = initial ^ 0xFFFFFFFF;
    const uint8_t *u = static_cast<const uint8_t *>(buf);
    for (uint32_t i = 0; i < len; ++i) {
        c = kCrcTable[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

/* Flash functions -----------------------------------------------------------*/

uint32_t Controller::flash_getSector(uint32_t address) {
    uint32_t sector = 0;

    if ((address >= 0x08000000) && (address < 0x08020000)) {
        sector = FLASH_SECTOR_0;
    }

    else if ((address >= 0x08020000) && (address < 0x08040000)) {
        sector = FLASH_SECTOR_1;
    }

    else if ((address >= 0x08040000) && (address < 0x08060000)) {
        sector = FLASH_SECTOR_2;
    }

    else if ((address >= 0x08060000) && (address < 0x08080000)) {
        sector = FLASH_SECTOR_3;
    }

    else if ((address >= 0x08080000) && (address < 0x080A0000)) {
        sector = FLASH_SECTOR_4;
    }

    else if ((address >= 0x080A0000) && (address < 0x080C0000)) {
        sector = FLASH_SECTOR_5;
    }

    else if ((address >= 0x080C0000) && (address < 0x080E0000)) {
        sector = FLASH_SECTOR_6;
    }

    else if ((address >= 0x080E0000) && (address < 0x08100000)) {
        sector = FLASH_SECTOR_7;
    }

    return sector;
}

uint32_t Controller::flash_eraseSector(uint32_t startSectorAddress, uint32_t wordSize) {
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError;
    int sofar = 0;
    uint8_t flashWord = 8;

    HAL_FLASH_Unlock();

    uint32_t startSector = flash_getSector(startSectorAddress);
    uint32_t endSectorAddress = startSectorAddress + (wordSize * 4);
    uint32_t endSector = flash_getSector(endSectorAddress);

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.Sector = startSector;
    EraseInitStruct.NbSectors = (endSector - startSector) + 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
        return HAL_FLASH_GetError();
    }

    HAL_FLASH_Lock();
    return 0;
}

uint32_t Controller::flash_writeData(uint32_t startSectorAddress, uint32_t *data, uint32_t wordSize) {
    // 256 bits for STM32H72x/3X devices (8x 32bits words)

    int sofar = 0;
    uint8_t flashWord = 8;

    HAL_FLASH_Unlock();

    while (sofar < wordSize) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, startSectorAddress, (uint32_t)&data[sofar]) == HAL_OK) {
            startSectorAddress += 4 * flashWord;
            sofar += flashWord;
        } else {
            return HAL_FLASH_GetError();
        }
    }

    HAL_FLASH_Lock();
    return 0;
}

void Controller::flash_readData(uint32_t StartSectorAddress, uint32_t *data, uint32_t wordSize) {
    while (1) {
        *data = *(__IO uint32_t *)StartSectorAddress;
        StartSectorAddress += 4;
        data++;
        if (!(wordSize--)) break;
    }
}

/* Button functions --------------------------------------------------------*/

void Controller::button_check() {
    if (buttonTrigger) {
        switch (lcdTest) {
            case false:
                LED1_ON;
                buttonTrigger = false;
                lcdTest = true;
                lcd_initialize();
                lcd.displayOn();
                lcd_test();
                break;

            case true:
                LED1_OFF;
                buttonTrigger = false;
                lcdTest = false;
                lcd.displayOff();
                lcd.fillScreen(BLACK);
                break;
        }
    }
}

/* Keyboard functions --------------------------------------------------------*/

void Controller::keyboard_initialize() {
    CT0_SCL_HIGH;
    CT1_SCL_HIGH;
}

void Controller::keyboard_check() {
    // power = true
    if (power) {
        // left button
        if (keyboard.leftButton >= 0) {
            // LED0_TOGGLE;
            check(keyboard.leftButton, 2);
            switch (keyboard.leftButton) {
                case KEY_POWER:
                    powerButtonFlag = true;
                    powerButtonCounter = 0;
                    startPowerButtonTimer();
                    break;

                case LEFT_RELEASE:
                    if (powerButtonFlag) {
                        stopPowerButtonTimer();
                        powerButtonFlag = false;
                        powerButtonCounter = 0;
                    }
                    break;

                default:
                    break;
            }
            keyboard.leftButton = -1;
        }

        // right button
        if (keyboard.rightButton >= 0) {
            // LED1_TOGGLE;
            // check(keyboard.rightButton, 0);
            keyboard.rightButton = -1;
        }
    }
    // power = false
    else {
        if (keyboard.leftButton >= 0) {
            // LED0_TOGGLE;
            // check(keyboard.leftButton, 0);
            switch (keyboard.leftButton) {
                case KEY_POWER:
                    powerButtonFlag = true;
                    powerButtonCounter = 0;
                    startPowerButtonTimer();
                    break;

                case LEFT_RELEASE:
                    if (powerButtonFlag) {
                        stopPowerButtonTimer();
                        powerButtonFlag = false;
                        powerButtonCounter = 0;
                    }
                    break;

                default:
                    break;
            }
            keyboard.leftButton = -1;
        }
    }
}

/* Dac functions -------------------------------------------------------------*/

void Controller::dac_initialize() {
    dac.initialize();
}

/* Sd functions --------------------------------------------------------------*/

void Controller::sd_initialize() {
    FATFS_UnLinkDriver(SDPath);
    FATFS_LinkDriver(&SD_Driver, SDPath);
}

SdResult Controller::sd_detect() {
    SdResult sdResult;
    (HAL_GPIO_ReadPin(SDMMC2_DETECT_GPIO_Port, SDMMC2_DETECT_Pin)) ? sdResult = SD_ERROR : sdResult = SD_OK;
    return sdResult;
}

SdResult Controller::sd_mount() {
    SdResult sdResult;
    (f_mount(&sd.fs, SDPath, 0) == FR_OK) ? sdResult = SD_OK : sdResult = SD_ERROR;
    return sdResult;
}

SdResult Controller::sd_unmount() {
    SdResult sdResult;
    (f_mount(0, SDPath, 0) == FR_OK) ? sdResult = SD_OK : sdResult = SD_ERROR;
    return sdResult;
}

SdResult Controller::sd_getSpace() {
    SdResult sdResult;
    FATFS *fs_ptr = &sd.fs;
    uint32_t freeCluster;
    if (f_getfree(SDPath, (DWORD *)&freeCluster, &fs_ptr) == FR_OK) {
        uint32_t totalBlocks = (sd.fs.n_fatent - 2) * sd.fs.csize;
        uint32_t freeBlocks = freeCluster * sd.fs.csize;
        sd.totalSpace = totalBlocks / 2000;
        sd.freeSpace = freeBlocks / 2000;
        sd.usedSpace = sd.totalSpace - sd.freeSpace;
        sdResult = SD_OK;
    } else {
        sd.totalSpace = 0;
        sd.freeSpace = 0;
        sd.usedSpace = 0;
        sdResult = SD_ERROR;
    }
    return sdResult;
}

SdResult Controller::sd_checkFileExist(const char *fileAddress) {
    SdResult sdResult;
    if ((f_stat(fileAddress, &sd.fileInfo) == FR_OK) && ((sd.fileInfo.fattrib & AM_DIR) == false)) {
        sdResult = SD_OK;
    } else {
        sdResult = SD_ERROR;
    }
    return sdResult;
}

SdResult Controller::sd_checkFolderExist(const char *folderAddress) {
    SdResult sdResult;
    if ((f_stat(folderAddress, &sd.fileInfo) == FR_OK) && (sd.fileInfo.fattrib & AM_DIR)) {
        sdResult = SD_OK;
    } else {
        sdResult = SD_ERROR;
    }
    return sdResult;
}

FRESULT Controller::sd_createDirectory(const char *path) {
    FRESULT res;
    res = f_mkdir(path);
    return res;
}

FRESULT Controller::sd_deleteDirectory(const char *path) {
    FRESULT res;
    DIR dir;
    FILINFO fileInfo;
    char file[64] = "";
    bool listFile = true;

    res = f_opendir(&dir, path);

    while (listFile) {
        res = f_readdir(&dir, &fileInfo);
        if ((res == FR_OK) && (fileInfo.fname[0] != 0)) {
            memset(file, 0x00, strlen(file));
            sprintf((char *)file, "%s/%s", path, fileInfo.fname);
            (fileInfo.fattrib & AM_DIR) ? sd_deleteDirectory(file) : f_unlink(file);
        } else {
            listFile = false;
        }
    }

    f_closedir(&dir);
    f_unlink(path);
    return res;
}

/* Lcd functions -------------------------------------------------------------*/

void Controller::lcd_initialize() {
    lcd.initialize();
}

void Controller::lcd_test() {
    lcd.setForeColor(GRAY_75);
    lcd.setBackColor(BLACK);

    uint16_t boxSize = 26;

    for (uint8_t i = 0; i < 33; i++) {
        uint16_t x = 9 + (i * boxSize);
        lcd.drawVLine(x, 6, 468);
    }

    for (uint8_t i = 0; i < 19; i++) {
        uint16_t y = 6 + (i * boxSize);
        lcd.drawHLine(9, y, 833);
    }

    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);

    for (uint8_t i = 4; i < 29; i++) {
        uint16_t x = 9 + (i * boxSize);
        lcd.drawVLine(x, 6 + (3 * 26), 312);
    }

    for (uint8_t i = 3; i < 16; i++) {
        uint16_t y = 6 + (i * boxSize);
        lcd.drawHLine(9 + (4 * 26), y, 624);
    }

    uint16_t xPos = 9;
    uint16_t yPos = 6;

    lcd.setForeColor(BLACK);
    lcd.fillRect(xPos + (0 * 26) + 1, yPos + (0 * 26) + 1, (boxSize * 3) - 1, (boxSize * 3) - 1);
    lcd.fillRect(xPos + (0 * 26) + 1, yPos + (15 * 26) + 1, (boxSize * 3) - 1, (boxSize * 3) - 1);
    lcd.fillRect(xPos + (29 * 26) + 1, yPos + (0 * 26) + 1, (boxSize * 3) - 1, (boxSize * 3) - 1);
    lcd.fillRect(xPos + (29 * 26) + 1, yPos + (15 * 26) + 1, (boxSize * 3) - 1, (boxSize * 3) - 1);

    RGB16Color cArray01[] = {BLACK, RED, PINK, BLUE, CYAN, GREEN, YELLOW, WHITE};

    for (uint8_t i = 0; i < 8; i++) {
        lcd.setForeColor(cArray01[i]);
        lcd.fillRect(xPos + (4 * 26) + (i * 3 * 26) + 1, yPos + (3 * 26) + 1, (boxSize * 3) - 1, (boxSize * 3) - 1);
    }

    RGB16Color cArray02[] = {0x10A2, 0x18E3, 0x2945, 0x3186, 0x39E7, 0x4228, 0x528A, 0x5ACB, 0x632C, 0x6B6D, 0x7BCF, 0x8410, 0x8C71, 0x94B2, 0xA514, 0xAD55, 0xB5B6, 0xBDF7, 0xCE59, 0xD69A, 0xDEFB, 0xE73C, 0xF79E, 0xFFFF};

    for (uint8_t i = 0; i < 24; i++) {
        lcd.setForeColor(cArray02[i]);
        lcd.fillRect(xPos + (4 * 26) + (i * 26) + 1, yPos + (6 * 26) + 1, (boxSize * 1) - 1, (boxSize * 2) - 1);
    }

    lcd.setForeColor(BLACK);
    lcd.fillRect(xPos + (4 * 26) + (9 * 26) + 1, yPos + (8 * 26) + 1, (boxSize * 6) - 1, (boxSize * 2) - 1);
    lcd.setForeColor(BLACK);
    lcd.fillRect(xPos + (4 * 26) + (15 * 26) + 1, yPos + (8 * 26) + 1, (boxSize * 9) - 1, (boxSize * 2) - 1);

    for (uint8_t i = 0; i < 59; i++) {
        lcd.setForeColor(WHITE);
        lcd.fillRect(xPos + (4 * 26) + (i * 4), yPos + (8 * 26) + 1, 2, (boxSize * 2) - 1);
        lcd.setForeColor(BLACK);
        lcd.fillRect(xPos + (4 * 26) + (i * 4) + 2, yPos + (8 * 26) + 1, 2, (boxSize * 2) - 1);
    }

    for (uint8_t i = 0; i < 5; i++) {
        lcd.setForeColor(WHITE);
        lcd.fillRect(xPos + (4 * 26) + (15 * 26) + (i * 52), yPos + (8 * 26) + 1, 26, (boxSize * 2) - 1);
    }

    lcd.setAlignment(CENTER);
    lcd.setForeColor(WHITE);
    lcd.setFont(FONT_05x07);
    lcd.drawText("RANDOMWAVES", 11, 427, 236);

    lcd.setForeColor(WHITE);
    lcd.fillRect(xPos + (4 * 26) + 1, yPos + (10 * 26) + 1, (boxSize * 24) - 1, (boxSize * 2) - 1);

    RGB16Color redArray[] = {0xFD34, 0xFCB2, 0xFC30, 0xFBCF, 0xFB4D, 0xFACB, 0xFA49, 0xF9E7, 0xF965, 0xF8E3, 0xF861, 0xF800, 0xF800, 0xE000, 0xD000, 0xC800, 0xB800, 0xA800, 0x9800, 0x8800, 0x7800, 0x6800, 0x5800, 0x4000};

    RGB16Color greenArray[] = {0xAFF5, 0x97F2, 0x87F0, 0x7FEF, 0x6FED, 0x5FEB, 0x4FE9, 0x3FE7, 0x2FE5, 0x1FE3, 0x0FE1, 0x07E0, 0x07E0, 0x0780, 0x0700, 0x0680, 0x0600, 0x05A0, 0x0520, 0x04A0, 0x0420, 0x03C0, 0x0340, 0x02C0};

    RGB16Color blueArray[] = {0xA53F, 0x94BF, 0x843F, 0x7BDF, 0x6B5F, 0x5ADF, 0x4A5F, 0x39FF, 0x297F, 0x18FF, 0x087F, 0x001F, 0x001F, 0x001E, 0x001C, 0x001A, 0x0018, 0x0016, 0x0014, 0x0012, 0x0010, 0x000F, 0x000D, 0x000B};

    for (uint8_t i = 0; i < 24; i++) {
        lcd.setForeColor(redArray[i]);
        lcd.fillRect(xPos + (4 * 26) + (i * 26) + 1, yPos + (12 * 26) + 1, (boxSize * 1) - 1, (boxSize * 1) - 1);
        lcd.setForeColor(greenArray[i]);
        lcd.fillRect(xPos + (4 * 26) + (i * 26) + 1, yPos + (13 * 26) + 1, (boxSize * 1) - 1, (boxSize * 1) - 1);
        lcd.setForeColor(blueArray[i]);
        lcd.fillRect(xPos + (4 * 26) + (i * 26) + 1, yPos + (14 * 26) + 1, (boxSize * 1) - 1, (boxSize * 1) - 1);
    }

    lcd.setFont(FONT_10x14);
    lcd.setAlignment(CENTER);
    lcd.setForeColor(GRAY_75);
    lcd.drawText("01", 2, xPos + 41, yPos + 33);
    lcd.drawText("02", 2, xPos + 795, yPos + 33);
    lcd.drawText("03", 2, xPos + 41, yPos + 423);
    lcd.drawText("04", 2, xPos + 795, yPos + 423);
}

/* Interrupt functions -------------------------------------------------------*/

void Controller::interruptLeftButtonTrigger() {
    keyboard.leftButtonState = PREWAIT;
    keyboard.leftButtonCounter = 0;
    startLeftButtonTimer();
}

void Controller::interruptRightButtonTrigger() {
    keyboard.rightButtonState = PREWAIT;
    keyboard.rightButtonCounter = 0;
    startRightButtonTimer();
}

void Controller::interruptLeftButtonRead() {
    switch (keyboard.leftButtonState) {
        case PASSIVE:
            break;

        case PREWAIT:
            if (keyboard.leftButtonCounter < 50) {
                keyboard.leftButtonCounter += 1;
            } else {
                keyboard.leftButtonState = READ;
                keyboard.leftButtonCounter = 0;
            }
            break;

        case READ:
            if (keyboard.leftButtonCounter < 32) {
                switch (keyboard.leftButtonCounter % 2) {
                    case 0:
                        CT0_SCL_LOW;
                        break;

                    case 1:
                        bool value = CT0_SDO_READ;
                        if (!value) {
                            keyboard.leftButtonTemp = (keyboard.leftButtonCounter / 2) + 1;
                        }
                        CT0_SCL_HIGH;
                        break;
                }
                keyboard.leftButtonCounter += 1;
            } else {
                keyboard.leftButtonState = POSTWAIT;
                keyboard.leftButtonCounter = 0;
            }
            break;

        case POSTWAIT:
            if (keyboard.leftButtonCounter < 1000) {
                keyboard.leftButtonCounter += 1;
            } else {
                keyboard.leftButtonState = PASSIVE;
                keyboard.leftButtonCounter = 0;
                keyboard.leftButton = keyboard.leftButtonTemp;
                keyboard.leftButtonTemp = 0;
                stopLeftButtonTimer();
            }
            break;

        default:
            break;
    }
}

void Controller::interruptRightButtonRead() {
    switch (keyboard.rightButtonState) {
        case PASSIVE:
            break;

        case PREWAIT:
            if (keyboard.rightButtonCounter < 50) {
                keyboard.rightButtonCounter += 1;
            } else {
                keyboard.rightButtonState = READ;
                keyboard.rightButtonCounter = 0;
            }
            break;

        case READ:
            if (keyboard.rightButtonCounter < 32) {
                switch (keyboard.rightButtonCounter % 2) {
                    case 0:
                        CT1_SCL_LOW;
                        break;

                    case 1:
                        bool value = CT1_SDO_READ;
                        if (!value) {
                            keyboard.rightButtonTemp = (keyboard.rightButtonCounter / 2) + 1;
                        }
                        CT1_SCL_HIGH;
                        break;
                }
                keyboard.rightButtonCounter += 1;
            } else {
                keyboard.rightButtonState = POSTWAIT;
                keyboard.rightButtonCounter = 0;
            }
            break;

        case POSTWAIT:
            if (keyboard.rightButtonCounter < 1000) {
                keyboard.rightButtonCounter += 1;
            } else {
                keyboard.rightButtonState = PASSIVE;
                keyboard.rightButtonCounter = 0;
                keyboard.rightButton = keyboard.rightButtonTemp;
                keyboard.rightButtonTemp = 0;
                stopRightButtonTimer();
            }
            break;

        default:
            break;
    }
}

void Controller::interruptPowerButtonRead() {
    if (powerButtonFlag) {
        if (powerButtonCounter < 3) {
            powerButtonCounter += 1;
        } else {
            stopPowerButtonTimer();
            powerButtonFlag = false;
            powerButtonCounter = 0;
            (!power) ? power = true : power = false;
        }
    }
}

void Controller::interruptSd() {
    if ((sd.detect) && (sd_detect() == SD_ERROR)) {
        sd.detect = false;
        sd.ready = false;
    }
}
