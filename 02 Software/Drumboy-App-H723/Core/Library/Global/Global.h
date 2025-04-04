#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdint.h>

#include "fatfs.h"
#include "main.h"
#include "sdmmc.h"
#include "string.h"
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
/* Application Version -------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kVersionMajor = 0x01;
const uint8_t kVersionMinor = 0x01;

////////////////////////////////////////////////////////////////////////////////
/* Global Functions ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

#define STM32_UNIQUE_ID ((uint32_t *)0x1FF1E800)

const int32_t INT24_MAX = 8388607;
const int32_t INT24_MIN = -8388608;

// (*STM32_UNIQUE_ID) == 2424897;
// (*STM32_UNIQUE_ID) == 2424884;

// volatile uint16_t* ptr = (volatile uint16_t*)(kOscBasic_RamAddress_Library[0]);
// viewRef.debug(0, *(ptr + 4), 5);

template <typename T, int N>
int size(T (&a)[N]) {
    return N - 1;
}

template <typename T>
void swap(T &a, T &b) {
    T temp = a;
    a = b;
    b = temp;
}

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

inline float amplitudeTodB(float amplitude) {
    return 20.0f * log10(amplitude);
}

inline float dBToAmplitude(float dB) {
    return pow(10.0f, dB / 20.0f);
}

const int32_t kAudioSampleRate = 44100;

typedef enum {
    FILE_NONE = 0x00,
    FILE_MISSING = 0x01,
    FILE_INCOMPATIBLE = 0x02,
    FILE_INACTIVE = 0x03,
    FILE_ACTIVE = 0x04,
} FileStatus;

#define LED0_ON HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET)
#define LED1_ON HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED2_ON HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
#define SYNC_OUT_ON HAL_GPIO_WritePin(SYNC_OUT_GPIO_Port, SYNC_OUT_Pin, GPIO_PIN_SET)

#define LED0_OFF HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET)
#define LED1_OFF HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED2_OFF HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)
#define SYNC_OUT_OFF HAL_GPIO_WritePin(SYNC_OUT_GPIO_Port, SYNC_OUT_Pin, GPIO_PIN_RESET)

#define LED0_TOGGLE HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin)
#define LED1_TOGGLE HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)
#define LED2_TOGGLE HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin)

////////////////////////////////////////////////////////////////////////////////
/* Ram Addresses -------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

/* ITCMRAM 0x00000000 64KB ---------------------------------------------------*/

/* DTCMRAM 0x20000000 128KB --------------------------------------------------*/

/* RAM_D1 0x24000000 320KB ---------------------------------------------------*/

/* RAM_D2 0x30000000 32KB ----------------------------------------------------*/

#define RAM_ICON_SELECT_ADDRESS 0x300011B0
#define RAM_ICON_SELECT_PALETTE_ADDRESS 0x300011B0 // 128 bytes
#define RAM_ICON_SELECT_DATA_ADDRESS 0x30001230
#define RAM_ICON_SELECT_ON_DATA_ADDRESS 0x30001230  // 121 bytes
#define RAM_ICON_SELECT_OFF_DATA_ADDRESS 0x300012A9 // 121 bytes

#define RAM_ICON_ALERT_ADDRESS 0x30001322
#define RAM_ICON_ALERT_PALETTE_ADDRESS 0x30001322 // 128 bytes
#define RAM_ICON_ALERT_DATA_ADDRESS 0x300013A2
#define RAM_ICON_ALERT_L_DATA_ADDRESS 0x300013A2 // 289 bytes
#define RAM_ICON_ALERT_R_DATA_ADDRESS 0x300014C3 // 289 bytes

#define RAM_ICON_PLAY_ADDRESS 0x300015E4
#define RAM_ICON_PLAY_PALETTE_ADDRESS 0x300015E4 // 128 bytes
#define RAM_ICON_PLAY_DATA_ADDRESS 0x30001664
#define RAM_ICON_PLAY_RESET_ON_DATA_ADDRESS 0x30001664   // 81 bytes
#define RAM_ICON_PLAY_STOP_ON_DATA_ADDRESS 0x300016B5    // 81 bytes
#define RAM_ICON_PLAY_PLAY_ON_DATA_ADDRESS 0x30001706    // 81 bytes
#define RAM_ICON_PLAY_RECORD_ON_DATA_ADDRESS 0x30001757  // 81 bytes
#define RAM_ICON_PLAY_RESET_OFF_DATA_ADDRESS 0x300017A8  // 81 bytes
#define RAM_ICON_PLAY_STOP_OFF_DATA_ADDRESS 0x300017F9   // 81 bytes
#define RAM_ICON_PLAY_PLAY_OFF_DATA_ADDRESS 0x3000184A   // 81 bytes
#define RAM_ICON_PLAY_RECORD_OFF_DATA_ADDRESS 0x3000189B // 81 bytes

#define RAM_ICON_LAYER_ADDRESS 0x300018EC
#define RAM_ICON_LAYER_PALETTE_ADDRESS 0x300018EC // 1.280 bytes
#define RAM_ICON_LAYER_DATA_ADDRESS 0x30001DEC
#define RAM_ICON_LAYER_S_ON_0_DATA_ADDRESS 0x30001DEC // 121 bytes
#define RAM_ICON_LAYER_S_ON_1_DATA_ADDRESS 0x30001E65 // 121 bytes
#define RAM_ICON_LAYER_S_OFF_DATA_ADDRESS 0x30001EDE  // 121 bytes
#define RAM_ICON_LAYER_L_OFF_DATA_ADDRESS 0x30001F57  // 121 bytes
#define RAM_ICON_LAYER_L_ON_DATA_ADDRESS 0x30001FD0   // 121 bytes
#define RAM_ICON_LAYER_D_ON_DATA_ADDRESS 0x30002049   // 121 bytes
#define RAM_ICON_LAYER_D_OFF_DATA_ADDRESS 0x300020C2  // 121 bytes

/* RAM_D3 0x38000000 16KB ----------------------------------------------------*/

/* SDRAM 0xC00000000 128MB ---------------------------------------------------*/

#define RAM_INST_ADDRESS 0xC0000000

#define RAM_SAMPLE_ADDRESS 0xC0000000

const uint32_t kSampleSize = 240000; // 5.0 seconds
const uint32_t kSampleByteSize = kSampleSize * 3;

#define RAM_SAMPLE_0A RAM_SAMPLE_ADDRESS
#define RAM_SAMPLE_0B RAM_SAMPLE_ADDRESS + (1 * kSampleByteSize)
#define RAM_SAMPLE_1A RAM_SAMPLE_ADDRESS + (2 * kSampleByteSize)
#define RAM_SAMPLE_1B RAM_SAMPLE_ADDRESS + (3 * kSampleByteSize)
#define RAM_SAMPLE_2A RAM_SAMPLE_ADDRESS + (4 * kSampleByteSize)
#define RAM_SAMPLE_2B RAM_SAMPLE_ADDRESS + (5 * kSampleByteSize)
#define RAM_SAMPLE_3A RAM_SAMPLE_ADDRESS + (6 * kSampleByteSize)
#define RAM_SAMPLE_3B RAM_SAMPLE_ADDRESS + (7 * kSampleByteSize)
#define RAM_SAMPLE_4A RAM_SAMPLE_ADDRESS + (8 * kSampleByteSize)
#define RAM_SAMPLE_4B RAM_SAMPLE_ADDRESS + (9 * kSampleByteSize)
#define RAM_SAMPLE_5A RAM_SAMPLE_ADDRESS + (10 * kSampleByteSize)
#define RAM_SAMPLE_5B RAM_SAMPLE_ADDRESS + (11 * kSampleByteSize)
#define RAM_SAMPLE_6A RAM_SAMPLE_ADDRESS + (12 * kSampleByteSize)
#define RAM_SAMPLE_6B RAM_SAMPLE_ADDRESS + (13 * kSampleByteSize)
#define RAM_SAMPLE_7A RAM_SAMPLE_ADDRESS + (14 * kSampleByteSize)
#define RAM_SAMPLE_7B RAM_SAMPLE_ADDRESS + (15 * kSampleByteSize)
#define RAM_SAMPLE_8A RAM_SAMPLE_ADDRESS + (16 * kSampleByteSize)
#define RAM_SAMPLE_8B RAM_SAMPLE_ADDRESS + (17 * kSampleByteSize)
#define RAM_SAMPLE_9A RAM_SAMPLE_ADDRESS + (18 * kSampleByteSize)
#define RAM_SAMPLE_9B RAM_SAMPLE_ADDRESS + (19 * kSampleByteSize)

const uint32_t kRamLayerAddressLibrary[10][2] = {
    {RAM_SAMPLE_0A, RAM_SAMPLE_0B},
    {RAM_SAMPLE_1A, RAM_SAMPLE_1B},
    {RAM_SAMPLE_2A, RAM_SAMPLE_2B},
    {RAM_SAMPLE_3A, RAM_SAMPLE_3B},
    {RAM_SAMPLE_4A, RAM_SAMPLE_4B},
    {RAM_SAMPLE_5A, RAM_SAMPLE_5B},
    {RAM_SAMPLE_6A, RAM_SAMPLE_6B},
    {RAM_SAMPLE_7A, RAM_SAMPLE_7B},
    {RAM_SAMPLE_8A, RAM_SAMPLE_8B},
    {RAM_SAMPLE_9A, RAM_SAMPLE_9B}};

#define RAM_DELAY_ADDRESS 0xC0DBBA00

const uint32_t kDelaySize = 96000;
const uint32_t kDelayByteSize = kDelaySize * 4;

#define RAM_DELAY_0 RAM_DELAY_ADDRESS
#define RAM_DELAY_1 RAM_DELAY_ADDRESS + (1 * kDelayByteSize)

#define RAM_CHORUS_ADDRESS 0xC0E77200

const uint32_t kChorusSize = 24000;
const uint32_t kChorusByteSize = kChorusSize * 4;

#define RAM_CHORUS_0 RAM_CHORUS_ADDRESS
#define RAM_CHORUS_1 RAM_CHORUS_ADDRESS + (1 * kChorusByteSize)

#define RAM_METRO_ADDRESS 0xC0EA6000

const uint32_t kMetroSize = 9600; // 0.2 seconds
const uint32_t kMetroByteSize = kMetroSize * 3;

#define RAM_METRO_0A RAM_METRO_ADDRESS
#define RAM_METRO_0B RAM_METRO_ADDRESS + (1 * kMetroByteSize)
#define RAM_METRO_1A RAM_METRO_ADDRESS + (2 * kMetroByteSize)
#define RAM_METRO_1B RAM_METRO_ADDRESS + (3 * kMetroByteSize)
#define RAM_METRO_2A RAM_METRO_ADDRESS + (4 * kMetroByteSize)
#define RAM_METRO_2B RAM_METRO_ADDRESS + (5 * kMetroByteSize)
#define RAM_METRO_3A RAM_METRO_ADDRESS + (6 * kMetroByteSize)
#define RAM_METRO_3B RAM_METRO_ADDRESS + (7 * kMetroByteSize)
#define RAM_METRO_4A RAM_METRO_ADDRESS + (8 * kMetroByteSize)
#define RAM_METRO_4B RAM_METRO_ADDRESS + (9 * kMetroByteSize)

const uint32_t kRamMetronomeAddressLibrary[5][2] = {
    {RAM_METRO_0A, RAM_METRO_0B},
    {RAM_METRO_1A, RAM_METRO_1B},
    {RAM_METRO_2A, RAM_METRO_2B},
    {RAM_METRO_3A, RAM_METRO_3B},
    {RAM_METRO_4A, RAM_METRO_4B},
};

#define RAM_IMAGE_LOGO_ADDRESS 0xC0FB9640            // 600px * 100px
#define RAM_IMAGE_LOGO_PALETTE_ADDRESS 0xC0FB9640    //    128 bytes
#define RAM_IMAGE_LOGO_DATA_ADDRESS 0xC0FB9640 + 128 // 60.000 bytes

#define RAM_IMAGE_MENU_ADDRESS 0xC0FC8120            // 814px * 107px
#define RAM_IMAGE_MENU_PALETTE_ADDRESS 0xC0FC8120    //    128 bytes
#define RAM_IMAGE_MENU_DATA_ADDRESS 0xC0FC8120 + 128 // 87.098 bytes

#define RAM_IMAGE_LAYER_ADDRESS 0xC0FDD5DA             // 814px * 33px
#define RAM_IMAGE_LAYER_PALETTE_ADDRESS 0xC0FDD5DA     //  1.280 bytes
#define RAM_IMAGE_LAYER_DATA_ADDRESS 0xC0FDD5DA + 1280 // 26.862 bytes

////////////////////////////////////////////////////////////////////////////////
/* Sdram Constants -----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

#define SDRAM_START_ADDRESS ((uint32_t)0xC0000000)
#define SDRAM_END_ADDRESS ((uint32_t)0xC0FFFFF0)

#define SDRAM_MEMORY_WIDTH FMC_SDRAM_MEM_BUS_WIDTH_16
#define SDCLOCK_PERIOD FMC_SDRAM_CLOCK_PERIOD_2

#define SDRAM_TIMEOUT ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1 ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2 ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4 ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8 ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2 ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3 ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((uint16_t)0x0200)

////////////////////////////////////////////////////////////////////////////////
/* Sd Constants --------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

struct Sd {
    FATFS fs;
    DIR dir;
    FIL file;
    FILINFO fileInfo;
    FRESULT fresult;

    UINT byteswritten;
    UINT bytesread;

    bool detect = false;
    bool ready = false;
    bool getLibrary = false;
    char label[32] = "";
    DWORD serial = 0;
    DWORD serialTemp = 0;

    uint16_t totalSpace = 0;
    uint16_t freeSpace = 0;
    uint16_t usedSpace = 0;

    Sd() {}
    ~Sd() {}
};

/*----------------------------------------------------------------------------*/

typedef enum {
    SD_OK = 0x00,
    SD_ERROR = 0x01,
    SD_ERROR_DETECT = 0x02,
    SD_ERROR_MOUNT = 0x03,
    SD_ERROR_SERIAL = 0x04,
    SD_ERROR_SYSTEMFOLDER = 0x05,
    SD_ERROR_SAMPLEFOLDER = 0x06,
    SD_ERROR_FILEFOLDER = 0x07,
    SD_ERROR_DRUMKITFOLDER = 0x08,
    SD_ERROR_SOUNDFOLDER = 0x09,
    SD_ERROR_IMAGEFOLDER = 0x0A,
    SD_ERROR_FIRMWAREFOLDER = 0x0B,
    SD_ERROR_SYSTEMFILE = 0x0C
} SdResult;

/*----------------------------------------------------------------------------*/

const char kSdAlertTextInsert[] = "INSERT SDCARD";
const char kSdAlertTextFormat[] = "SDCARD FORMAT ERROR";
const char kSdAlertTextSerial[] = "SDCARD SERIAL ERROR";
const char kSdAlertTextSystemFolder[] = "SYSTEM FOLDER ERROR";
const char kSdAlertTextSampleFolder[] = "SAMPLE FOLDER ERROR";
const char kSdAlertTextFileFolder[] = "FILE FOLDER ERROR";
const char kSdAlertTextDrumkitFolder[] = "DRUMKIT FOLDER ERROR";
const char kSdAlertTextSoundFolder[] = "SOUND FOLDER ERROR";
const char kSdAlertTextImageFolder[] = "IMAGE FOLDER ERROR";
const char kSdAlertTextFirmwareFolder[] = "FWARE FOLDER ERROR";
const char kSdAlertTextSystemFile[] = "SYSTEM FILE ERROR";
const char kSdAlertTextAnalyze[] = "ANALYZING SDCARD";

////////////////////////////////////////////////////////////////////////////////
/* Lcd Constants -------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

#define LCD_DATA_GPIO_Port GPIOB

#define LCD_CS_HIGH LCD_CS_GPIO_Port->BSRR = LCD_CS_Pin
#define LCD_CS_LOW LCD_CS_GPIO_Port->BSRR = LCD_CS_Pin << 16U
#define LCD_RS_HIGH LCD_RS_GPIO_Port->BSRR = LCD_RS_Pin
#define LCD_RS_LOW LCD_RS_GPIO_Port->BSRR = LCD_RS_Pin << 16U
#define LCD_WR_HIGH LCD_WR_GPIO_Port->BSRR = LCD_WR_Pin
#define LCD_WR_LOW LCD_WR_GPIO_Port->BSRR = LCD_WR_Pin << 16U
#define LCD_RD_HIGH LCD_RD_GPIO_Port->BSRR = LCD_RD_Pin
#define LCD_RD_LOW LCD_RD_GPIO_Port->BSRR = LCD_RD_Pin << 16U
#define LCD_RESET_HIGH LCD_RESET_GPIO_Port->BSRR = LCD_RESET_Pin
#define LCD_RESET_LOW LCD_RESET_GPIO_Port->BSRR = LCD_RESET_Pin << 16U
#define LCD_BL_HIGH LCD_BL_GPIO_Port->BSRR = LCD_BL_Pin
#define LCD_BL_LOW LCD_BL_GPIO_Port->BSRR = LCD_BL_Pin << 16U

/*----------------------------------------------------------------------------*/

#define MADCTL_MY 0x80  // Bottom to top
#define MADCTL_MX 0x40  // Right to left
#define MADCTL_MV 0x20  // Row/Column exchange
#define MADCTL_ML 0x10  // LCD refresh Bottom to top
#define MADCTL_RGB 0x00 // Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 // Blue-Green-Red pixel order
#define MADCTL_MH 0x04  // LCD refresh right to left

/*----------------------------------------------------------------------------*/

const uint16_t kLCD_WIDTH = 480;
const uint16_t kLCD_HEIGHT = 854;

// R: 5 bit, G: 6 bit, B: 5bit
// https://lvgl.io/tools/imageconverter
// http://www.rinkydinkelectronics.com/calc_rgb565.php

typedef enum {
    RGB16 = 0x00,
    RGB24 = 0x01,
} RGBMode;

typedef uint16_t RGB16Color;
typedef uint32_t RGB24Color;

struct RGB24RawColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

/*----------------------------------------------------------------------------*/

const RGB16Color WHITE = 0xFFFF;
const RGB16Color BLACK = 0x0000;
const RGB16Color GRAY = 0x8C51;
const RGB16Color YELLOW = 0xFFE0;
const RGB16Color GREEN = 0x07E0;
const RGB16Color CYAN = 0x07FF;
const RGB16Color MAGENTA = 0xF81F;
const RGB16Color BLUE = 0x001F;
const RGB16Color RED = 0xF800;
const RGB16Color ORANGE = 0xFD20;
const RGB16Color PINK = 0xF81F;

const RGB16Color GRAY_05 = 0x0861;
const RGB16Color GRAY_10 = 0x18C3;
const RGB16Color GRAY_15 = 0x2124;
const RGB16Color GRAY_20 = 0x3186;
const RGB16Color GRAY_25 = 0x4208;
const RGB16Color GRAY_30 = 0x4A69;
const RGB16Color GRAY_35 = 0x5ACB;
const RGB16Color GRAY_40 = 0x632C;
const RGB16Color GRAY_45 = 0x738E;
const RGB16Color GRAY_50 = 0x8410;
const RGB16Color GRAY_55 = 0x8C71;
const RGB16Color GRAY_60 = 0x9CD3;
const RGB16Color GRAY_65 = 0xA534;
const RGB16Color GRAY_70 = 0xB596;
const RGB16Color GRAY_75 = 0xBDF7;
const RGB16Color GRAY_80 = 0xCE79;
const RGB16Color GRAY_85 = 0xDEDB;
const RGB16Color GRAY_90 = 0xE73C;
const RGB16Color GRAY_95 = 0xF79E;

/*----------------------------------------------------------------------------*/

const RGB16Color kLayerColorPalette[] = {
    0xFFE0,
    0x7FE0,
    0x07E0,
    0x07EF,
    0x07FF,
    0x03FF,
    0x001F,
    0x781F,
    0xF81F,
    0xF80F};

/*----------------------------------------------------------------------------*/

typedef enum {
    PORTRAIT_0 = 0x00,
    PORTRAIT_1 = 0x02,
    LANDSCAPE_0 = 0x01,
    LANDSCAPE_1 = 0x03
} LcdRotation;

/*----------------------------------------------------------------------------*/

typedef enum {
    FONT_05x07 = 0x00,
    FONT_07x09 = 0x01,
    FONT_10x14 = 0x02,
    FONT_14x18 = 0x03
} LcdFont;

/*----------------------------------------------------------------------------*/

typedef enum {
    LEFT = 0x00,
    RIGHT = 0x01,
    CENTER = 0x02
} LcdAlign;

/*----------------------------------------------------------------------------*/

const uint16_t kSongWidth = 561;
const uint16_t kSongHeight = 11;
const uint16_t kSongX = 251;
const uint16_t kSongOffsetY = 4;

const uint16_t kPlayWidth = 561;
const uint16_t kPlayHeight = 1;
const uint16_t kPlayX = 251;
const uint16_t kPlayY = 113;

const RGB16Color kPlayColor0 = BLACK;
const RGB16Color kPlayColor1 = WHITE;

const uint16_t kImageLogoWidth = 450;
const uint16_t kImageLogoHeight = 80;
const uint16_t kImageLogoX = 202;
const uint16_t kImageLogoY = 200;
const uint16_t kImageLogoPalette = 64;

const uint16_t kImageMenuWidth = 814;
const uint16_t kImageMenuHeight = 107;
const uint16_t kImageMenuX = 20;
const uint16_t kImageMenuY = 20;
const uint16_t kImageMenuPalette = 64;

const uint16_t kImageLayerWidth = 814;
const uint16_t kImageLayerHeight = 33;
const uint16_t kImageLayerX = 20;
const uint16_t kImageLayerY[] = {127, 160, 193, 226, 259, 292, 325, 358, 391, 424};
const uint16_t kImageLayerOffsetY = 33;
const uint16_t kImageLayerPalette = 640;

const uint16_t kLimitAlertX = 234;
const uint16_t kLimitAlertY = 109;

/*----------------------------------------------------------------------------*/

const uint8_t kIconMenuWidth = 20;
const uint8_t kIconMenuHeight = 20;
const uint16_t kIconMenuX = 92;
const uint16_t kIconMenuY = 50;

const uint8_t kIconSelectWidth = 11;
const uint8_t kIconSelectHeight = 11;
const uint16_t kIconSelect4X[] = {182 + 12, 345 + 12, 508 + 12, 671 + 12};
const uint16_t kIconSelect8X[] = {182 + 12, 263 + 12, 345 + 12, 426 + 12, 508 + 12, 589 + 12, 671 + 12, 752 + 12};
const uint16_t kIconSelectY = 77;

const uint8_t kIconAlertWidth = 17;
const uint8_t kIconAlertHeight = 17;
const uint16_t kIconAlertX[] = {317, 520};
const uint16_t kIconAlertY = 260;

const uint8_t kIconPlayWidth = 9;
const uint8_t kIconPlayHeight = 9;
const uint16_t kIconPlayY = 109;
const uint16_t kIconResetX = 47;
const uint16_t kIconPlayX = 65;
const uint16_t kIconStopX = 82;
const uint16_t kIconRecordX = 98;

const uint8_t kIconLayerWidth = 11;
const uint8_t kIconLayerHeight = 11;
const uint16_t kIconLayerTabX = 26;
const uint16_t kIconLayerTabY = 4;

const uint16_t kIconLayerMenuX = 161;
const uint16_t kIconLayerMenuY = 33;

const uint16_t kIconPalette = 64;
const uint16_t kIconLayerPalette = 640;

const uint16_t kIconMuteX = 166;
const uint16_t kIconFillX = 187;
const uint16_t kIconStyleX = 208;

/*----------------------------------------------------------------------------*/

const uint16_t kBeatGraphStartX = 449;
const uint16_t kBeatGraphEndX = 730;
const uint16_t kBeatGraphWidth = 282;
const uint16_t kBeatGraphStartY = 33;
const uint16_t kBeatGraphEndY = 87;
const uint8_t kBeatGraphHeight = 55;
const uint16_t kBeatGraphStartTimeX = 370;
const uint16_t kBeatGraphEndTimeX = 755;
const uint16_t kBeatGraphTimeY = 80;

/*----------------------------------------------------------------------------*/

const uint8_t kMinMenu4Tab = 0;
const uint8_t kMaxMenu4Tab = 3;

const uint8_t kMinMenu8Tab = 0;
const uint8_t kMaxMenu8Tab = 7;

const uint16_t kMenuIconX = 83;
const uint16_t kMenuIconY = 53;

const uint16_t kMenuLine4X[] = {182, 345, 508, 671};
const uint16_t kMenuLine8X[] = {182, 263, 345, 426, 508, 589, 671, 752};
const uint16_t kMenuLineY = 30;
const uint16_t kMenuLineHeight = 60;

const uint16_t kMenuHeader4X[] = {182 + 12, 345 + 12, 508 + 12, 671 + 12};
const uint16_t kMenuHeader8X[] = {182 + 12, 263 + 12, 345 + 12, 426 + 12, 508 + 12, 589 + 12, 671 + 12, 752 + 12};

const uint16_t kMenuHeaderY = 33;

const uint16_t kMenuData4X[] = {345 - 11, 508 - 11, 671 - 11, 834 - 11};
const uint16_t kMenuData8X[] = {263 - 11, 345 - 11, 426 - 11, 508 - 11, 589 - 11, 671 - 11, 752 - 11, 834 - 11};
const uint16_t kMenuDataY = 78;

const uint16_t kMenuNumberX = 182 - 28;
const uint16_t kMenuNumberY = 33;

const uint8_t kMenuHeaderTextSize = 10;
const uint8_t kMenuDataTextSize = 10;
const uint8_t kMenuNumberTextSize = 2;

const uint8_t kMenuHeaderShortTextSize = 4;
const uint8_t kMenuDataShortTextSize = 4;

const uint8_t kMenuSignTextSize = 3;

const uint16_t kMenuBoxX[] = {345 - 20, 508 - 20, 671 - 20, 834 - 20};
const uint16_t kMenuBoxY = 33;
const uint8_t kMenuBoxWidth = 9;
const uint8_t kMenuBoxHeight = 9;

const uint16_t kMainMenuX[] = {224, 305, 387, 468, 550, 631, 713, 794};
const uint16_t kMainMenuHeaderY = 39;
const uint16_t kMainMenuDataY = 72;

/*----------------------------------------------------------------------------*/

const char kHeaderActive[] = "ACTIVE    ";
const char kHeaderAnalyze[] = "ANALYZE   ";
const char kHeaderBar[] = "BAR       ";
const char kHeaderBlank[] = "          ";
const char kHeaderClear[] = "CLEAR     ";
const char kHeaderCurve[] = "CURVE     ";
const char kHeaderCutoff[] = "CUTOFF    ";
const char kHeaderDecay[] = "DECAY     ";
const char kHeaderDry[] = "DRY       ";
const char kHeaderEffectA[] = "EFFECT 1  ";
const char kHeaderEffectB[] = "EFFECT 2  ";
const char kHeaderEq[] = "PARAM EQ  ";
const char kHeaderFeedback[] = "FEEDBACK  ";
const char kHeaderFill[] = "FILL      ";
const char kHeaderFilterA[] = "FILTER 1  ";
const char kHeaderFilterB[] = "FILTER 2  ";
const char kHeaderFrequency[] = "FREQ      ";
const char kHeaderInst[] = "INST      ";
const char kHeaderLimiter[] = "LIMITER   ";
const char kHeaderLoad[] = "LOAD      ";
const char kHeaderMeasure[] = "MEASURE   ";
const char kHeaderMetronome[] = "METRONOME ";
const char kHeaderMix[] = "MIX       ";
const char kHeaderNew[] = "NEW       ";
const char kHeaderPan[] = "PAN       ";
const char kHeaderPlay[] = "PLAY      ";
const char kHeaderPrecount[] = "PRECOUNT  ";
const char kHeaderRate[] = "RATE      ";
const char kHeaderReverb[] = "REVERB    ";
const char kHeaderResonance[] = "RES       ";
const char kHeaderQuantize[] = "QUANTIZE  ";
const char kHeaderSample[] = "SAMPLE    ";
const char kHeaderSave[] = "SAVE      ";
const char kHeaderSize[] = "SIZE      ";
const char kHeaderSlope[] = "SLOPE     ";
const char kHeaderSurround[] = "SURROUND  ";
const char kHeaderTempo[] = "TEMPO     ";
const char kHeaderTime[] = "TIME      ";
const char kHeaderType[] = "TYPE      ";
const char kHeaderVolume[] = "VOLUME    ";
const char kHeaderWet[] = "WET       ";

const char kHeaderLow[] = "LOW       ";
const char kHeaderMid[] = "MID       ";
const char kHeaderHigh[] = "HIGH      ";

const char kHeaderEqFreq[6][3] = {"LS", "P ", "P ", "P ", "P ", "HS"};

const char kDataBlank[] = "          ";
const char kDataDashL[] = "--------  ";
const char kDataDashR[] = "  --------";
const char kDataDashShortL[] = "--- ";
const char kDataDashShortR[] = " ---";
const char kDataStart[] = "S";
const char kDataEnd[] = "E";
const char kDataSelect[] = "    SELECT";
const char kDataOn[] = "        ON";
const char kDataOff[] = "       OFF";

const char kDataShortLOn[] = "ON  ";
const char kDataShortLOff[] = "OFF ";
const char kDataShortROn[] = "  ON";
const char kDataShortROff[] = " OFF";

const char kDataDot[] = ".";
const char kDataPlus[] = "+";
const char kDataTimeBlank[] = "--.--.--";

////////////////////////////////////////////////////////////////////////////////
/* Keyboard Constants --------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    PASSIVE = 0x00,
    PREWAIT = 0x01,
    READ = 0x02,
    POSTWAIT = 0x03
} ButtonState;

/*----------------------------------------------------------------------------*/

struct Keyboard {
    ButtonState leftButtonState = PASSIVE;
    ButtonState rightButtonState = PASSIVE;
    uint16_t leftButtonCounter = 0;
    uint16_t rightButtonCounter = 0;
    uint16_t longButtonCounter = 0;
    int16_t leftButtonTemp = 0;
    int16_t rightButtonTemp = 0;
    int16_t leftButton = -1;
    int16_t rightButton = -1;
    bool leftButtonPress = false;
    bool rightButtonPress = false;
    bool layerButtonPress[10];
    bool muteButtonPress = false;
    bool fillButtonPress = false;
    bool styleButtonPress = false;
    bool instButtonPress = false;
    bool songButtonPress = false;
    bool bankButtonPress = false;
};

const uint8_t kLongButtonCountLow = 5;
const uint8_t kLongButtonCountHigh = 10;

#define CT0_SCL_HIGH CT0_SCL_GPIO_Port->BSRR = CT0_SCL_Pin
#define CT0_SCL_LOW CT0_SCL_GPIO_Port->BSRR = CT0_SCL_Pin << 16U
#define CT1_SCL_HIGH CT1_SCL_GPIO_Port->BSRR = CT1_SCL_Pin
#define CT1_SCL_LOW CT1_SCL_GPIO_Port->BSRR = CT1_SCL_Pin << 16U

#define CT0_SDO_READ CT0_SDO_GPIO_Port->IDR &CT0_SDO_Pin
#define CT1_SDO_READ CT1_SDO_GPIO_Port->IDR &CT1_SDO_Pin

/*----------------------------------------------------------------------------*/

typedef enum {
    KEY_RESET = 0x06 + 1,
    KEY_PLAYSTOP = 0x07 + 1,
    KEY_REC = 0x01 + 1,
    KEY_UP = 0x00 + 1,
    KEY_DOWN = 0x08 + 1,
    KEY_LEFT = 0x04 + 1,
    KEY_RIGHT = 0x03 + 1,
    KEY_CENTER = 0x0E + 1,
    KEY_ADD = 0x05 + 1,
    KEY_ERASE = 0x02 + 1,
    KEY_COPY = 0x0B + 1,
    KEY_PASTE = 0x0C + 1,
    KEY_POWER = 0x0A + 1,
    KEY_TAB_0 = 0x09 + 1,
    KEY_TAB_1 = 0x0D + 1,
    LEFT_RELEASE = 0x00
} KeyLeft;

typedef enum {
    KEY_FILE = 0x06 + 1,
    KEY_DRUMKIT = 0x07 + 1,
    KEY_SYSTEM = 0x01 + 1,
    KEY_RHYTHM = 0x05 + 1,
    KEY_METRONOME = 0x00 + 1,
    KEY_EQ = 0x02 + 1,
    KEY_FILTER = 0x04 + 1,
    KEY_EFFECT = 0x0E + 1,
    KEY_REVERB = 0x03 + 1,
    KEY_MUTE = 0x0B + 1,
    KEY_FILL = 0x08 + 1,
    KEY_STYLE = 0x0C + 1,
    KEY_MODE_INST = 0x0A + 1,
    KEY_MODE_SONG = 0x09 + 1,
    KEY_BANK = 0x0D + 1,
    RIGHT_RELEASE = 0x00
} KeyRight;

////////////////////////////////////////////////////////////////////////////////
/* Controller Constants ------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    INIT_MENU = 0x00,
    MAIN_MENU = 0x01,
    FILE_MENU = 0x02,
    DRUMKIT_MENU = 0x03,
    SYSTEM_MENU = 0x04,
    RHYTHM_MENU = 0x05,
    METRO_MENU = 0x06,
    EQ_MENU = 0x07,
    FILTER_0_MENU = 0x08,
    FILTER_1_MENU = 0x09,
    EFFECT_0_MENU = 0x0A,
    EFFECT_1_MENU = 0x0B,
    REVERB_MENU = 0x0C,
    LAYER_INST_MENU = 0x0D,
    LAYER_SONG_MENU = 0x0E
} Menu;

typedef enum {
    TAB_0 = 0x00,
    TAB_1 = 0x01
} LayerTab;

typedef enum {
    TRIG_MEASURE = 0x00,
    TRIG_BAR = 0x01,
    TRIG_SONG = 0x02
} TriggerMode;

typedef enum {
    ALERT_MEASUREUP = 0x00,
    ALERT_MEASUREDOWN = 0x01,
    ALERT_BARUP = 0x02,
    ALERT_BARDOWN = 0x03,
    ALERT_QUANTIZEUP = 0x04,
    ALERT_QUANTIZEDOWN = 0x05,
    ALERT_NEWFILE = 0x06,
    ALERT_LOADFILE = 0x07,
    ALERT_SAVEFILE = 0x08,
    ALERT_CLEARFILE = 0x09,
    ALERT_OVERWRITEFILE = 0x0A,
    ALERT_NEWDRUMKIT = 0x0B,
    ALERT_LOADDRUMKIT = 0x0C,
    ALERT_SAVEDRUMKIT = 0x0D,
    ALERT_CLEARDRUMKIT = 0x0E,
    ALERT_OVERWRITEDRUMKIT = 0x0F,
    ALERT_MISSINGSAMPLE = 0x10,
    ALERT_MISSINGSAMPLES = 0x11,
    ALERT_LOADSUCCESS = 0x12,
    ALERT_LOADERROR = 0x13,
    ALERT_SAVESUCCESS = 0x14,
    ALERT_SAVEERROR = 0x15,
    ALERT_CLEARSUCCESS = 0x16,
    ALERT_CLEARERROR = 0x17,
    ALERT_SHUTDOWN = 0x18,
    ALERT_OFF = 0x20
} AlertType;

const char kAlertTextResetPlay[] = "RESET PLAY?";

const char kAlertTextNewFile[] = "NEW FILE?";
const char kAlertTextLoadFile[] = "LOAD FILE?";
const char kAlertTextSaveFile[] = "SAVE FILE?";
const char kAlertTextClearFile[] = "CLEAR FILE?";
const char kAlertTextOverwriteFile[] = "OVERWRITE FILE?";

const char kAlertTextNewDrumkit[] = "NEW DRUMKIT?";
const char kAlertTextLoadDrumkit[] = "LOAD DRUMKIT?";
const char kAlertTextSaveDrumkit[] = "SAVE DRUMKIT?";
const char kAlertTextClearDrumkit[] = "CLEAR DRUMKIT?";
const char kAlertTextOverwriteDrumkit[] = "OVERWRITE DRUMKIT?";
const char kAlertTextMissingSample[] = "MISSING SAMPLE";
const char kAlertTextMissingSamples[] = "MISSING SAMPLES";

const char kAlertTextLoadSuccess[] = "LOAD -> SUCCESS";
const char kAlertTextLoadError[] = "LOAD -> ERROR";
const char kAlertTextSaveSuccess[] = "SAVE -> SUCCESS";
const char kAlertTextSaveError[] = "SAVE -> ERROR";
const char kAlertTextClearSuccess[] = "CLEAR -> SUCCESS";
const char kAlertTextClearError[] = "CLEAR -> ERROR";

struct Icon {
    bool flag = false;
    bool mode = false;
};

const uint16_t kInstLibraryMaxSize = 500;
const uint16_t kInstSampleLibraryMaxSize = 5000;

const uint8_t kFileNameSize = 32;
const uint8_t kFileNameArraySize = 35;

/*----------------------------------------------------------------------------*/

struct NumberData {
    char nameShortL[5];
    char nameShortR[5];
    char nameLongR[11];
};

const struct NumberData kNumberDataLibrary[] = {
    {"00  ", "  00", "        00"},
    {"01  ", "  01", "        01"},
    {"02  ", "  02", "        02"},
    {"03  ", "  03", "        03"},
    {"04  ", "  04", "        04"},
    {"05  ", "  05", "        05"},
    {"06  ", "  06", "        06"},
    {"07  ", "  07", "        07"},
    {"08  ", "  08", "        08"},
    {"09  ", "  09", "        09"},
    {"10  ", "  10", "        10"}};

/*----------------------------------------------------------------------------*/

struct FloatData {
    char nameInt[11];
    char nameFloat[11];
    float stdMultiplier;
    float pow2Multiplier;
    float pow4Multiplier;
};

const struct FloatData kFloatDataLibrary[] = {
    {"        00", "      0.00", 0.000000, 0.000000, 0.000000},
    {"        05", "      0.05", 0.050000, 0.002500, 0.000006},
    {"        10", "      0.10", 0.100000, 0.010000, 0.000100},
    {"        15", "      0.15", 0.150000, 0.022500, 0.000506},
    {"        20", "      0.20", 0.200000, 0.040000, 0.001600},
    {"        25", "      0.25", 0.250000, 0.062500, 0.003906},
    {"        30", "      0.30", 0.300000, 0.090000, 0.008100},
    {"        35", "      0.35", 0.350000, 0.122500, 0.015006},
    {"        40", "      0.40", 0.400000, 0.160000, 0.025600},
    {"        45", "      0.45", 0.450000, 0.202500, 0.041006},
    {"        50", "      0.50", 0.500000, 0.250000, 0.062500},
    {"        55", "      0.55", 0.550000, 0.302500, 0.091506},
    {"        60", "      0.60", 0.600000, 0.360000, 0.129600},
    {"        65", "      0.65", 0.650000, 0.422500, 0.178506},
    {"        70", "      0.70", 0.700000, 0.490000, 0.240100},
    {"        75", "      0.75", 0.750000, 0.562500, 0.316406},
    {"        80", "      0.80", 0.800000, 0.640000, 0.409600},
    {"        85", "      0.85", 0.850000, 0.722500, 0.522006},
    {"        90", "      0.90", 0.900000, 0.810000, 0.656100},
    {"        95", "      0.95", 0.950000, 0.902500, 0.814506},
    {"       100", "      1.00", 1.000000, 1.000000, 1.000000}};

/*
dBvalue = 20.0 * log10 (linear);
// dB = 20*log(linear)

linear = pow (10.0, (0.05 * dBvalue));
// linear = 10^(dB/20)

000     -51.5dB     inf             0x7F        127
005     -26.0dB     -26.02dB        0x4C        76
010     -20.0dB     -20.00dB        0x40        64
015     -16.5dB     -16.48dB        0x39        57
020     -14.0dB     -13.98dB        0x34        52
025     -12.0dB     -12.04dB        0x30        48
030     -10.5dB     -10.46dB        0x2D        45
035     -9.0dB      -9.12dB         0x2A        42
040     -8.0dB      -7.96dB         0x28        40
045     -7.0dB      -6.94dB         0x26        38
050     -6.0dB      -6.02dB         0x24        36
055     -5.0dB      -5.19dB         0x22        34
060     -4.0dB      -4.44dB         0x20        32
065     -3.5dB      -3.74dB         0x1F        31
070     -3.0dB      -3.10dB         0x1E        30
075     -2.5dB      -2.50dB         0x1D        29
080     -2.0dB      -1.94dB         0x1C        28
085     -1.5dB      -1.41dB         0x1B        27
090     -1.0dB      -0.92dB         0x1A        26
095     -0.5dB      -0.45dB         0x19        25
100     0dB         0dB             0x18        24
*/

const uint8_t kHeadPhoneDecibelData[] = {
    0x7F,
    0x4C,
    0x40,
    0x39,
    0x34,
    0x30,
    0x2D,
    0x2A,
    0x28,
    0x26,
    0x24,
    0x22,
    0x20,
    0x1F,
    0x1E,
    0x1D,
    0x1C,
    0x1B,
    0x1A,
    0x19,
    0x18};

////////////////////////////////////////////////////////////////////////////////
/* File Constants ------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const char kFileRef[] = "RW_DRUMBOY_FILE          ";
const char kFileStart[] = "System/File/File_";
const char kFileEnd[] = ".rws";

const uint16_t kFileByteSize = 10960;
const uint8_t kFileLibrarySize = 100;

const uint8_t kMinFile = 0;
const uint8_t kMaxFile = 99;

////////////////////////////////////////////////////////////////////////////////
/* Drumkit Constants ---------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const char kDrumkitRef[] = "RW_DRUMBOY_DRUMKIT       ";
const char kDrumkitStart[] = "System/Drumkit/Drumkit_";
const char kDrumkitEnd[] = ".rws";

const uint16_t kDrumkitByteSize = 1300;
const uint8_t kDrumkitLibrarySize = 100;

const uint8_t kMinDrumkit = 0;
const uint8_t kMaxDrumkit = 99;

////////////////////////////////////////////////////////////////////////////////
/* System Constants ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinSystemVolume = 0;
const uint8_t kMaxSystemVolume = 20;

const uint8_t kMinSystemPan = 0;
const uint8_t kMaxSystemPan = 20;

const uint8_t kMinSystemMidiIn = 0;
const uint8_t kMaxSystemMidiIn = 16;

const uint8_t kMinSystemMidiOut = 0;
const uint8_t kMaxSystemMidiOut = 16;

const uint8_t kMinSystemSyncIn = 0;
const uint8_t kMaxSystemSyncIn = 1;

const uint8_t kMinSystemSyncOut = 0;
const uint8_t kMaxSystemSyncOut = 2;

const uint8_t kInitialSystemVolume = 15;
const uint8_t kInitialSystemPan = 10;
const uint8_t kInitialSystemLimiter = true;
const uint8_t kInitialSystemMidiIn = 0;
const uint8_t kInitialSystemMidiOut = 0;
const uint8_t kInitialSystemSyncIn = 0;
const uint8_t kInitialSystemSyncOut = 0;

struct SystemVolumeData {
    char nameLongR[11];
    float data;
};

const struct SystemVolumeData kSystemVolumeDataLibrary[] = {
    {"        00", 0.0000}, {"        05", 0.0025}, {"        10", 0.0100}, {"        15", 0.0225}, {"        20", 0.0400}, {"        25", 0.0625}, {"        30", 0.0900}, {"        35", 0.1225}, {"        40", 0.1600}, {"        45", 0.2025}, {"        50", 0.2500}, {"        55", 0.3025}, {"        60", 0.3600}, {"        65", 0.4225}, {"        70", 0.4900}, {"        75", 0.5625}, {"        80", 0.6400}, {"        85", 0.7225}, {"        90", 0.8100}, {"        95", 0.9025}, {"       100", 1.00}};

struct SystemPanData {
    char nameShortR[5];
    uint8_t left;
    uint8_t right;
};

const struct SystemPanData kSystemPanDataLibrary[] = {
    {"L+10", 20, 0}, {" L+9", 20, 2}, {" L+8", 20, 4}, {" L+7", 20, 6}, {" L+6", 20, 8}, {" L+5", 20, 10}, {" L+4", 20, 12}, {" L+3", 20, 14}, {" L+2", 20, 16}, {" L+1", 20, 18}, {"  LR", 20, 20}, {" R+1", 18, 20}, {" R+2", 16, 20}, {" R+3", 14, 20}, {" R+4", 12, 20}, {" R+5", 10, 20}, {" R+6", 8, 20}, {" R+7", 6, 20}, {" R+8", 4, 20}, {" R+9", 2, 20}, {"R+10", 0, 20}};

struct SystemMidiData {
    char nameShortR[5];
    bool active;
    uint8_t channel;
};

const struct SystemMidiData kSystemMidiDataLibrary[]{
    {" OFF", false, 0}, {"  01", true, 0}, {"  02", true, 1}, {"  03", true, 2}, {"  04", true, 3}, {"  05", true, 4}, {"  06", true, 5}, {"  07", true, 6}, {"  08", true, 7}, {"  09", true, 8}, {"  10", true, 9}, {"  11", true, 10}, {"  12", true, 11}, {"  13", true, 12}, {"  14", true, 13}, {"  15", true, 14}, {"  16", true, 15}};

struct SystemSyncInData {
    char nameShortR[5];
    bool tempoTrigger;
    bool playTrigger;
    bool beatTrigger;
};

const struct SystemSyncInData kSystemSyncInDataLibrary[] = {
    {" OFF", false, false, false}, {"  RW", true, true, false}};

struct SystemSyncOutData {
    char nameShortR[5];
    bool tempoTrigger;
    bool playTrigger;
    bool beatTrigger;
};

const struct SystemSyncOutData kSystemSyncOutDataLibrary[] = {
    {" OFF", false, false, false}, {"  RW", true, true, false}, {" STD", false, false, true}};

typedef enum {
    SYNC_TEMPO = 0x00,
    SYNC_RESET = 0xC8,      // 200
    SYNC_TRIG_RESET = 0xC9, // 201
    SYNC_PLAY = 0xCA,       // 202
    SYNC_TRIG_STOP = 0xCB,  // 203
    SYNC_STOP = 0xCC,       // 204
    SYNC_RECORD = 0xCD,     // 205
} SyncCommand;

typedef enum {
    SYS_ACTION_NONE = 0x00,
    SYS_ACTION_UP = 0x01,
    SYS_ACTION_DOWN = 0x02
} SystemTransitionAction;

struct SystemVolumeTransition {
    bool active;

    float targetVolume;
    SystemTransitionAction actionVolume;
};

struct SystemPanTransition {
    bool active;

    float targetVolumeLeft;
    SystemTransitionAction actionVolumeLeft;

    float targetVolumeRight;
    SystemTransitionAction actionVolumeRight;
};

struct Midi {
    bool rxActive = false;
    bool txActive = false;

    uint8_t rxChannel = 0;
    uint8_t txChannel = 0;

    uint8_t rxNoteOnReadStage = 0;
    uint8_t rxNoteOnWriteFlag = 0;
    uint8_t rxNoteOnKey = 0;
    uint8_t rxNoteOnVelocity = 0;

    uint8_t rxNoteOffReadStage = 0;
    uint8_t rxNoteOffWriteFlag = 0;
    uint8_t rxNoteOffKey = 0;
    uint8_t rxNoteOffVelocity = 0;

    bool txTriggerNoteOn[10] = {false};
    bool txTriggerNoteOff[10] = {false};

    const uint8_t txData[10] = {0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D};
};

struct Sync {
    bool syncInTempo = false;
    bool syncInPlay = false;
    bool syncInBeat = false;

    bool syncOutTempo = false;
    bool syncOutPlay = false;
    bool syncOutBeat = false;

    bool slaveMode = false;
    bool masterMode = false;
};

struct Sys {
    uint8_t volume = kInitialSystemVolume;
    uint8_t pan = kInitialSystemPan;
    bool limiter = kInitialSystemLimiter;
    uint8_t midiIn = kInitialSystemMidiIn;
    uint8_t midiOut = kInitialSystemMidiOut;
    uint8_t syncIn = kInitialSystemSyncIn;
    uint8_t syncOut = kInitialSystemSyncOut;

    Midi midi;
    Sync sync;

    float volumeFloat = kSystemVolumeDataLibrary[volume].data;
    float volumeLeftFloat = kSystemVolumeDataLibrary[kSystemPanDataLibrary[pan].left].data;
    float volumeRightFloat = kSystemVolumeDataLibrary[kSystemPanDataLibrary[pan].right].data;

    SystemVolumeTransition volumeTransition;
    SystemPanTransition panTransition;
};

////////////////////////////////////////////////////////////////////////////////
/* Rhythm Constants ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinTempo = 60;
const uint8_t kMaxTempo = 160;

const uint8_t kMinMeasure = 1;
const uint8_t kMaxMeasure = 8;

const uint8_t kMinBar = 1;
const uint8_t kMaxBar = 8;

const uint8_t kMinQuantize = 0;
const uint8_t kMaxQuantize = 6;

const uint8_t kInitialTempo = 120;
const uint8_t kInitialMeasure = 4;
const uint8_t kInitialBar = 4;
const uint8_t kInitialQuantize = 2;

const uint16_t kMeasureInterval = 800;
const uint16_t kMeasureHalfInterval = kMeasureInterval / 2;
const uint16_t kMinSongInterval = 800;
const uint16_t kMaxSongInterval = 64000;
const float kQuantizeInterval[] = {50, 100, 200, 400, 800, 1600, 3200};

const uint16_t kSecondsinOneMinute = 60;
const uint32_t kMicroSecondsinOneSecond = 1000000;
const uint32_t kMicroSecondsinOneMinute = 60000000;

/*----------------------------------------------------------------------------*/

struct QuantizeData {
    char nameShortR[3];
    char nameLongR[11];
};

const struct QuantizeData kQuantizeDataLibrary[] = {
    {"64", "      1:64"},
    {"32", "      1:32"},
    {"16", "      1:16"},
    {"08", "       1:8"},
    {"04", "       1:4"},
    {"02", "       1:2"},
    {"01", "       1:1"}};

/*----------------------------------------------------------------------------*/

struct Rhythm {
    uint8_t tempo = kInitialTempo;
    uint8_t measure = kInitialMeasure;
    uint8_t bar = kInitialBar;
    uint8_t quantize = kInitialQuantize;
    uint8_t measureTotal = kInitialMeasure * kInitialBar;

    bool measureLock = true;
    bool barLock = true;
    bool quantizeLock = true;
};

////////////////////////////////////////////////////////////////////////////////
/* Metronome Constants -------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinMetroSample = 0;
const uint8_t kMaxMetroSample = 4;

const uint8_t kMinMetroVolume = 0;
const uint8_t kMaxMetroVolume = 20;

const bool kInitialMetroActive = true;
const bool kInitialMetroPrecount = true;
const uint8_t kInitialMetroSample = 0;
const uint8_t kInitialMetroVolume = 15;

struct MetroSampleData {
    char nameLongR[11];
};

const struct MetroSampleData kMetroSampleDataLibrary[] = {
    {"     KNOCK"},
    {"     BLOCK"},
    {"    TONGUE"},
    {"     TEETH"},
    {"      CLAP"}};

struct MetronomeVolumeData {
    char nameLongR[11];
    float data;
};

const struct MetronomeVolumeData kMetronomeVolumeDataLibrary[] = {
    {"       000", 0.0000}, {"       005", 0.0025}, {"       010", 0.0100}, {"       015", 0.0225}, {"       020", 0.0400}, {"       025", 0.0625}, {"       030", 0.0900}, {"       035", 0.1225}, {"       040", 0.1600}, {"       045", 0.2025}, {"       050", 0.2500}, {"       055", 0.3025}, {"       060", 0.3600}, {"       065", 0.4225}, {"       070", 0.4900}, {"       075", 0.5625}, {"       080", 0.6400}, {"       085", 0.7225}, {"       090", 0.8100}, {"       095", 0.9025}, {"       100", 1.00}};

typedef enum {
    MET_ACTION_NONE = 0x00,
    MET_ACTION_UP = 0x01,
    MET_ACTION_DOWN = 0x02
} MetronomeTransitionAction;

struct MetronomeVolumeTransition {
    bool active;

    float targetVolume;
    MetronomeTransitionAction actionVolume;
};

struct Metronome {
    bool active = kInitialMetroActive;
    bool precount = kInitialMetroPrecount;
    uint8_t sample = kInitialMetroSample;
    uint8_t volume = kInitialMetroVolume;

    float volumeFloat = kMetronomeVolumeDataLibrary[volume].data;

    MetronomeVolumeTransition volumeTransition;

    bool precountState = false;
    uint16_t precounter = 0;
    uint16_t precounterMax = kMeasureInterval * kInitialMeasure;

    bool countDownFlag = false;
    int8_t countDown = kInitialMeasure;
};

////////////////////////////////////////////////////////////////////////////////
/* Lpf Constants -------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const bool kInitialLpfActive = true;
const uint16_t kInitialLpfFreq = 5000;
const float kInitialLpfDry = 0.75;
const float kInitialLpfWet = 0.25;

struct Lpf {
    bool active = kInitialLpfActive;
    uint8_t freq = kInitialLpfFreq;

    float dry = kInitialLpfDry;
    float wet = kInitialLpfWet;

    float a0;
    float a1;
    float a2;
    float b1;
    float b2;

    int32_t dataIn[3] = {0};
    int32_t dataOut[3] = {0};

    void initialize() {
        calculateFilterCoef();
    }

    void calculateFilterCoef() {
        float Q = 0.707;
        float norm;
        float K = tan(M_PI * freq / kAudioSampleRate);

        norm = 1.0f / (1.0f + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2.0f * a0;
        a2 = a0;
        b1 = 2.0f * (K * K - 1.0f) * norm;
        b2 = (1.0f - K / Q + K * K) * norm;
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Eq Constants --------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinEqQ = 0;
const uint8_t kMaxEqQ = 7;

const uint8_t kMinEqFreq = 0;
const uint8_t kMaxEqFreq = 46;

const uint8_t kMinEqGain = 0;
const uint8_t kMaxEqGain = 36;

const bool kInitialEqActive = true;

const uint8_t kEqGainZero = 24;

const uint8_t kInitialEqFreqLowShelf = 4;
const uint8_t kInitialEqGainLowShelf = 28;

const uint8_t kInitialEqFreqHighShelf = 31;
const uint8_t kInitialEqGainHighShelf = 20;

const uint8_t kInitialEqQPeak[4] = {0, 0, 0, 0};
const uint8_t kInitialEqFreqPeak[4] = {9, 12, 27, 29};
const uint8_t kInitialEqGainPeak[4] = {26, 24, 24, 22};

typedef enum {
    EQ_LOWPASS = 0x00,
    EQ_HIGHPASS = 0x01,
    EQ_BANDPASS = 0x02,
    EQ_NOTCH = 0x03,
    EQ_PEAK = 0x04,
    EQ_LOWSHELF = 0x05,
    EQ_HIGHSHELF = 0x06
} EqType;

struct EqQData {
    char nameShortR[5];
    float data;
};

const struct EqQData kEqQDataLibrary[] = {
    {"  01", 1.0f}, {"  02", 2.0f}, {"  03", 3.0f}, {"  04", 4.0f}, {"  05", 5.0f}, {"  06", 6.0f}, {"  07", 7.0f}, {"  08", 8.0f}};

struct EqFreqData {
    char nameShortR[5];
    uint16_t data;
};

const EqFreqData kEqFreqDataLibrary[] = {
    {"  10", 10}, {"  20", 20}, {"  30", 30}, {"  40", 40}, {"  50", 50}, {"  60", 60}, {"  70", 70}, {"  80", 80}, {"  90", 90}, {" 100", 100}, {" 150", 150}, {" 200", 200}, {" 250", 250}, {" 300", 300}, {" 350", 350}, {" 400", 400}, {" 450", 450}, {" 500", 500}, {" 550", 550}, {" 600", 600}, {" 650", 650}, {" 700", 700}, {" 750", 750}, {" 800", 800}, {" 850", 850}, {" 900", 900}, {" 950", 950}, {"  1K", 1000}, {"  2K", 2000}, {"  3K", 3000}, {"  4K", 4000}, {"  5K", 5000}, {"  6K", 6000}, {"  7K", 7000}, {"  8K", 8000}, {"  9K", 9000}, {" 10K", 10000}, {" 11K", 11000}, {" 12K", 12000}, {" 13K", 13000}, {" 14K", 14000}, {" 15K", 15000}, {" 16K", 16000}, {" 17K", 17000}, {" 18K", 18000}, {" 19K", 19000}, {" 20K", 20000}};

struct EqGainData {
    char nameShortR[5];
    int16_t data;
};

const struct EqGainData kEqGainDataLibrary[] = {
    {" -24", -24}, {" -23", -23}, {" -22", -22}, {" -21", -21}, {" -20", -20}, {" -19", -19}, {" -18", -18}, {" -17", -17}, {" -16", -16}, {" -15", -15}, {" -14", -14}, {" -13", -13}, {" -12", -12}, {" -11", -11}, {" -10", -10}, {" -09", -9}, {" -08", -8}, {" -07", -7}, {" -06", -6}, {" -05", -5}, {" -04", -4}, {" -03", -3}, {" -02", -2}, {" -01", -1}, {" +00", 0}, {" +01", 1}, {" +02", 2}, {" +03", 3}, {" +04", 4}, {" +05", 5}, {" +06", 6}, {" +07", 7}, {" +08", 8}, {" +09", 9}, {" +10", 10}, {" +11", 11}, {" +12", 12}};

struct EqFilter {
    float a0 = 0;
    float a1 = 0;
    float a2 = 0;
    float b1 = 0;
    float b2 = 0;

    int32_t dataIn[3] = {0};
    int32_t dataOut[3] = {0};
};

typedef enum {
    EQ_ACTION_NONE = 0x00,
    EQ_ACTION_UP = 0x01,
    EQ_ACTION_DOWN = 0x02
} EqTransitionAction;

typedef enum {
    EQ_MODE_NONE = 0x00,
    EQ_MODE_ACTIVE = 0x01,
} EqTransitionMode;

typedef enum {
    EQ_PHASE_NONE = 0x00,
    EQ_PHASE_A = 0x01,
    EQ_PHASE_B = 0x02
} EqTransitionPhase;

struct EqGenTransition {
    bool active;

    EqTransitionMode mode;
    EqTransitionPhase phase;

    bool activeActive;
    bool targetActive;

    float activeDry;
    float targetDry;
    EqTransitionAction actionDry;

    float activeWet;
    float targetWet;
    EqTransitionAction actionWet;
};

struct Eq {
    bool active = kInitialEqActive;

    EqGenTransition genTransition;

    uint8_t freqLowShelf;
    uint8_t gainLowShelf;
    EqFilter filterLowShelf;

    uint8_t freqHighShelf;
    uint8_t gainHighShelf;
    EqFilter filterHighShelf;

    uint8_t qPeak[4] = {kInitialEqQPeak[0], kInitialEqQPeak[1], kInitialEqQPeak[2], kInitialEqQPeak[3]};
    uint8_t freqPeak[4] = {kInitialEqFreqPeak[0], kInitialEqFreqPeak[1], kInitialEqFreqPeak[2], kInitialEqFreqPeak[3]};
    uint8_t gainPeak[4] = {kInitialEqGainPeak[0], kInitialEqGainPeak[1], kInitialEqGainPeak[2], kInitialEqGainPeak[3]};

    EqFilter filterPeak[4];

    void initialize() {
        active = kInitialEqActive;

        freqLowShelf = kInitialEqFreqLowShelf;
        gainLowShelf = kInitialEqGainLowShelf;

        freqHighShelf = kInitialEqFreqHighShelf;
        gainHighShelf = kInitialEqGainHighShelf;

        calculateCoefLowShelf();
        calculateCoefHighShelf();
        calculateCoefPeak(0);
        calculateCoefPeak(1);
        calculateCoefPeak(2);
        calculateCoefPeak(3);
    }

    void calculateCoefLowShelf() {
        calculateCoef(EQ_LOWSHELF, kAudioSampleRate, kEqFreqDataLibrary[freqLowShelf].data, 0.707, kEqGainDataLibrary[gainLowShelf].data, filterLowShelf);
    }

    void calculateCoefHighShelf() {
        calculateCoef(EQ_HIGHSHELF, kAudioSampleRate, kEqFreqDataLibrary[freqHighShelf].data, 0.707, kEqGainDataLibrary[gainHighShelf].data, filterHighShelf);
    }

    void calculateCoefPeak(uint8_t peakNum) {
        calculateCoef(EQ_PEAK, kAudioSampleRate, kEqFreqDataLibrary[freqPeak[peakNum]].data, kEqQDataLibrary[qPeak[peakNum]].data, kEqGainDataLibrary[gainPeak[peakNum]].data, filterPeak[peakNum]);
    }

    void calculateCoef(uint8_t type, uint32_t sampleRate, uint32_t freq, float Q, float gain, EqFilter &filter) {
        float a0, a1, a2, b1, b2, norm;

        float PI = 3.141592653f;
        float SQ = 1.414213562f;
        float V = pow(10, fabs(gain) / 20.0);
        float K = tan(PI * freq / sampleRate);

        switch (type) {
        case EQ_LOWPASS:
            norm = 1.0f / (1.0f + K / Q + K * K);
            a0 = K * K * norm;
            a1 = 2.0f * a0;
            a2 = a0;
            b1 = 2.0f * (K * K - 1.0f) * norm;
            b2 = (1.0f - K / Q + K * K) * norm;
            break;

        case EQ_HIGHPASS:
            norm = 1 / (1 + K / Q + K * K);
            a0 = 1 * norm;
            a1 = -2 * a0;
            a2 = a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;

        case EQ_BANDPASS:
            norm = 1 / (1 + K / Q + K * K);
            a0 = K / Q * norm;
            a1 = 0;
            a2 = -a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;

        case EQ_NOTCH:
            norm = 1 / (1 + K / Q + K * K);
            a0 = (1 + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = a0;
            b1 = a1;
            b2 = (1 - K / Q + K * K) * norm;
            break;

        case EQ_PEAK:
            if (gain >= 0) { // boost
                norm = 1 / (1 + 1 / Q * K + K * K);
                a0 = (1 + V / Q * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - V / Q * K + K * K) * norm;
                b1 = a1;
                b2 = (1 - 1 / Q * K + K * K) * norm;
            } else { // cut
                norm = 1 / (1 + V / Q * K + K * K);
                a0 = (1 + 1 / Q * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - 1 / Q * K + K * K) * norm;
                b1 = a1;
                b2 = (1 - V / Q * K + K * K) * norm;
            }
            break;

        case EQ_LOWSHELF:
            if (gain >= 0) { // boost
                norm = 1 / (1 + SQ * K + K * K);
                a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
                a1 = 2 * (V * K * K - 1) * norm;
                a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - SQ * K + K * K) * norm;
            } else { // cut
                norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
                a0 = (1 + SQ * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - SQ * K + K * K) * norm;
                b1 = 2 * (V * K * K - 1) * norm;
                b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
            }
            break;

        case EQ_HIGHSHELF:
            if (gain >= 0) { // boost
                norm = 1 / (1 + SQ * K + K * K);
                a0 = (V + sqrt(2 * V) * K + K * K) * norm;
                a1 = 2 * (K * K - V) * norm;
                a2 = (V - sqrt(2 * V) * K + K * K) * norm;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - SQ * K + K * K) * norm;
            } else { // cut
                norm = 1 / (V + sqrt(2 * V) * K + K * K);
                a0 = (1 + SQ * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - SQ * K + K * K) * norm;
                b1 = 2 * (K * K - V) * norm;
                b2 = (V - sqrt(2 * V) * K + K * K) * norm;
            }
            break;

        default:
            break;
        }

        filter.a0 = a0;
        filter.a1 = a1;
        filter.a2 = a2;
        filter.b1 = b1;
        filter.b2 = b2;
    }

    void cleanMemory() {
        // low shelf
        filterLowShelf.dataIn[0] = 0;
        filterLowShelf.dataIn[1] = 0;
        filterLowShelf.dataIn[2] = 0;

        filterLowShelf.dataOut[0] = 0;
        filterLowShelf.dataOut[1] = 0;
        filterLowShelf.dataOut[2] = 0;
        // high shelf
        filterHighShelf.dataIn[0] = 0;
        filterHighShelf.dataIn[1] = 0;
        filterHighShelf.dataIn[2] = 0;

        filterHighShelf.dataOut[0] = 0;
        filterHighShelf.dataOut[1] = 0;
        filterHighShelf.dataOut[2] = 0;
        // peak
        for (uint8_t i = 0; i < 4; i++) {
            filterPeak[i].dataIn[0] = 0;
            filterPeak[i].dataIn[1] = 0;
            filterPeak[i].dataIn[2] = 0;

            filterPeak[i].dataOut[0] = 0;
            filterPeak[i].dataOut[1] = 0;
            filterPeak[i].dataOut[2] = 0;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Filter Constants ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinFilterType = 1;
const uint8_t kMaxFilterType = 4;

const uint8_t kMinFilterFreq = 0;
const uint8_t kMaxFilterFreq = 46;

const uint8_t kMinFilterRes = 0;
const uint8_t kMaxFilterRes = 10;

const uint8_t kMinFilterSlope = 0;
const uint8_t kMaxFilterSlope = 1;

const uint8_t kMinFilterDry = 0;
const uint8_t kMaxFilterDry = 20;

const uint8_t kMinFilterWet = 0;
const uint8_t kMaxFilterWet = 20;

const bool kInitialFilterActive = false;
const uint8_t kInitialFilterType = 1;
const uint8_t kInitialFilterFreq = 27;
const uint8_t kInitialFilterRes = 0;
const uint8_t kInitialFilterSlope = 1;
const uint8_t kInitialFilterDry = 0;
const uint8_t kInitialFilterWet = 20;

const uint8_t kFilterLibrarySize = 2;

typedef enum {
    FIL_OFF = 0x00,
    FIL_LPF = 0x01,
    FIL_HPF = 0x02,
    FIL_BPF = 0x03,
    FIL_BSF = 0x04
} FilterType;

struct FilterTypeData {
    char nameShortL[5];
    char nameShortR[5];
};

const struct FilterTypeData kFilterTypeDataLibrary[] = {
    {"--- ", " ---"}, {"LPF ", " LPF"}, {"HPF ", " HPF"}, {"BPF ", " BPF"}, {"BSF ", " BSF"}};

struct FilterFreqData {
    char nameShortL[5];
    char nameShortR[5];
    uint16_t data;
};

const FilterFreqData kFilterFreqDataLibrary[] = {
    {"10  ", "  10", 10}, {"20  ", "  20", 20}, {"30  ", "  30", 30}, {"40  ", "  40", 40}, {"50  ", "  50", 50}, {"60  ", "  60", 60}, {"70  ", "  70", 70}, {"80  ", "  80", 80}, {"90  ", "  90", 90}, {"100 ", " 100", 100}, {"150 ", " 150", 150}, {"200 ", " 200", 200}, {"250 ", " 250", 250}, {"300 ", " 300", 300}, {"350 ", " 350", 350}, {"400 ", " 400", 400}, {"450 ", " 450", 450}, {"500 ", " 500", 500}, {"550 ", " 550", 550}, {"600 ", " 600", 600}, {"650 ", " 650", 650}, {"700 ", " 700", 700}, {"750 ", " 750", 750}, {"800 ", " 800", 800}, {"850 ", " 850", 850}, {"900 ", " 900", 900}, {"950 ", " 950", 950}, {"1K  ", "  1K", 1000}, {"2K  ", "  2K", 2000}, {"3K  ", "  3K", 3000}, {"4K  ", "  4K", 4000}, {"5K  ", "  5K", 5000}, {"6K  ", "  6K", 6000}, {"7K  ", "  7K", 7000}, {"8K  ", "  8K", 8000}, {"9K  ", "  9K", 9000}, {"10K ", " 10K", 10000}, {"11K ", " 11K", 11000}, {"12K ", " 12K", 12000}, {"13K ", " 13K", 13000}, {"14K ", " 14K", 14000}, {"15K ", " 15K", 15000}, {"16K ", " 16K", 16000}, {"17K ", " 17K", 17000}, {"18K ", " 18K", 18000}, {"19K ", " 19K", 19000}, {"20K ", " 20K", 20000}};

struct FilterResData {
    char nameShortL[5];
    char nameShortR[5];
    float data;
};

const struct FilterResData kFilterResDataLibrary[] = {
    {"00  ", "  00", 0.7f}, {"10  ", "  10", 1.0f}, {"20  ", "  20", 1.5f}, {"30  ", "  30", 2.0f}, {"40  ", "  40", 2.5f}, {"50  ", "  50", 3.0f}, {"60  ", "  60", 3.5f}, {"70  ", "  70", 4.0f}, {"80  ", "  80", 4.5f}, {"90  ", "  90", 5.0f}, {"100 ", " 100", 5.5f}};

struct FilterSlopeData {
    char nameShortL[5];
    char nameShortR[5];
};

const struct FilterSlopeData kFilterSlopeDataLibrary[] = {
    {"06  ", "  06"}, {"12  ", "  12"}};

struct FilterMixData {
    char nameShortL[5];
    char nameShortR[5];
    float data;
};

const struct FilterMixData kFilterMixDataLibrary[] = {
    {"00  ", "  00", 0.00}, {"05  ", "  05", 0.05}, {"10  ", "  10", 0.10}, {"15  ", "  15", 0.15}, {"20  ", "  20", 0.20}, {"25  ", "  25", 0.25}, {"30  ", "  30", 0.30}, {"35  ", "  35", 0.35}, {"40  ", "  40", 0.40}, {"45  ", "  45", 0.45}, {"50  ", "  50", 0.50}, {"55  ", "  55", 0.55}, {"60  ", "  60", 0.60}, {"65  ", "  65", 0.65}, {"70  ", "  70", 0.70}, {"75  ", "  75", 0.75}, {"80  ", "  80", 0.80}, {"85  ", "  85", 0.85}, {"90  ", "  90", 0.90}, {"95  ", "  95", 0.95}, {"100 ", " 100", 1.00}};

typedef enum {
    FIL_ACTION_NONE = 0x00,
    FIL_ACTION_UP = 0x01,
    FIL_ACTION_DOWN = 0x02
} FilterTransitionAction;

typedef enum {
    FIL_MODE_NONE = 0x00,
    FIL_MODE_ACTIVE = 0x01,
    FIL_MODE_TYPE = 0x02,
} FilterTransitionMode;

typedef enum {
    FIL_PHASE_NONE = 0x00,
    FIL_PHASE_A = 0x01,
    FIL_PHASE_B = 0x02
} FilterTransitionPhase;

struct FilterGenTransition {
    bool active;

    FilterTransitionMode mode;
    FilterTransitionPhase phase;

    bool activeActive;
    bool targetActive;

    uint8_t activeType;
    uint8_t targetType;

    float activeDry;
    float targetDry;
    FilterTransitionAction actionDry;

    float activeWet;
    float targetWet;
    FilterTransitionAction actionWet;
};

struct FilterMixTransition {
    bool active;

    float targetDry;
    FilterTransitionAction actionDry;

    float targetWet;
    FilterTransitionAction actionWet;
};

struct Filter {
    uint8_t num;
    bool active;
    uint8_t type;
    uint8_t freq;
    uint8_t res;
    uint8_t slope;
    uint8_t dry;
    uint8_t wet;

    float dryFloat;
    float wetFloat;

    FilterGenTransition genTransition;
    FilterMixTransition mixTransition;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    float a0;
    float a1;
    float a2;
    float b1;
    float b2;

    int32_t dataIn[3] = {0};
    int32_t dataOut[3] = {0};

    void initialize() {
        active = kInitialFilterActive;
        type = kInitialFilterType;
        freq = kInitialFilterFreq;
        res = kInitialFilterRes;
        slope = kInitialFilterSlope;
        dry = kInitialFilterDry;
        wet = kInitialFilterWet;

        calculateCoef();
        cleanMemory();
    }

    void calculateCoef() {
        uint32_t sampleRate = 44100;
        uint32_t frequency = kFilterFreqDataLibrary[freq].data;
        float Q = kFilterResDataLibrary[res].data;
        float norm;

        float K = tan(M_PI * frequency / sampleRate);

        switch (type) {
        case FIL_OFF:
            break;

        case FIL_LPF:
            norm = 1.0f / (1.0f + K / Q + K * K);
            a0 = K * K * norm;
            a1 = 2.0f * a0;
            a2 = a0;
            b1 = 2.0f * (K * K - 1.0f) * norm;
            b2 = (1.0f - K / Q + K * K) * norm;
            break;

        case FIL_HPF:
            norm = 1 / (1 + K / Q + K * K);
            a0 = 1 * norm;
            a1 = -2 * a0;
            a2 = a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;

        case FIL_BPF:
            norm = 1 / (1 + K / Q + K * K);
            a0 = K / Q * norm;
            a1 = 0;
            a2 = -a0;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - K / Q + K * K) * norm;
            break;

        case FIL_BSF:
            norm = 1 / (1 + K / Q + K * K);
            a0 = (1 + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = a0;
            b1 = a1;
            b2 = (1 - K / Q + K * K) * norm;
            break;
        }
    }

    void cleanMemory() {
        dataIn[0] = 0;
        dataIn[1] = 0;
        dataIn[2] = 0;

        dataOut[0] = 0;
        dataOut[1] = 0;
        dataOut[2] = 0;
    }

    Filter(uint8_t num_) : num(num_) {}
    ~Filter() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Delay Constants ----------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint16_t kDelayBufferSize = 96000;

const uint8_t kMinDelayTime = 0;
const uint8_t kMaxDelayTime = 5;

const uint8_t kMinDelayLevel = 0;
const uint8_t kMaxDelayLevel = 16;

const uint8_t kMinDelayFeedback = 0;
const uint8_t kMaxDelayFeedback = 16;

const uint8_t kMinDelayDry = 0;
const uint8_t kMaxDelayDry = 20;

const uint8_t kMinDelayWet = 0;
const uint8_t kMaxDelayWet = 20;

const uint32_t kMinDelayInterval = 0;
const uint32_t kMaxDelayInterval = kDelaySize - 1;

const bool kInitialDelayActive = false;
const uint8_t kInitialDelayTime = 2;
const uint8_t kInitialDelayLevel = 10;
const uint8_t kInitialDelayFeedback = 5;
const uint8_t kInitialDelayDry = 20;
const uint8_t kInitialDelayWet = 10;

struct DelayTimeData {
    char nameShortR[5];
    float data;
};

const struct DelayTimeData kDelayTimeDataLibrary[] = {
    {" 1/8", 0.1250}, {" 1/6", 0.1666}, {" 1/4", 0.2500}, {" 1/3", 0.3333}, {" 1/2", 0.5000}, {" 1/1", 1.0000}};

struct DelayLevelData {
    char nameShortR[5];
    float data;
};

const struct DelayLevelData kDelayLevelDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct DelayFeedbackData {
    char nameShortR[5];
    float data;
};

const struct DelayFeedbackData kDelayFeedbackDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct Delay {
    bool active;

    uint8_t aTime;
    uint8_t bLevel;
    uint8_t cFeedback;
    uint8_t dDry;
    uint8_t eWet;

    float time;
    float level;
    float feedback;

    uint32_t lag;
    uint32_t playInterval;
    uint32_t recordInterval;
    const uint32_t kDelayBufferSize = kDelaySize;
    int32_t *delayBuffer;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialDelayActive;

        aTime = kInitialDelayTime;
        bLevel = kInitialDelayLevel;
        cFeedback = kInitialDelayFeedback;
        dDry = kInitialDelayDry;
        eWet = kInitialDelayWet;

        time = kDelayTimeDataLibrary[kInitialDelayTime].data;
        level = kDelayLevelDataLibrary[kInitialDelayLevel].data;
        feedback = kDelayFeedbackDataLibrary[kInitialDelayFeedback].data;

        lag = ((60 * kAudioSampleRate) / kInitialTempo) * kDelayTimeDataLibrary[kInitialDelayTime].data;
        playInterval = 0;
        recordInterval = lag;
    }

    void update(uint16_t tempo) {
        lag = (uint32_t)(((60 * kAudioSampleRate) / tempo) * time);
        int32_t playInterval_ = recordInterval - lag;
        if (playInterval_ < 0)
            playInterval_ += kDelayBufferSize;
        playInterval = playInterval_;
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Chorus Constants ---------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint16_t kChorusBufferSize = 5000;

const uint8_t kMinChorusTime = 0;
const uint8_t kMaxChorusTime = 8;

const uint8_t kMinChorusFeedback = 0;
const uint8_t kMaxChorusFeedback = 19;

const uint8_t kMinChorusRate = 0;
const uint8_t kMaxChorusRate = 25;

const uint8_t kMinChorusDry = 0;
const uint8_t kMaxChorusDry = 20;

const uint8_t kMinChorusWet = 0;
const uint8_t kMaxChorusWet = 20;

const bool kInitialChorusActive = false;
const uint8_t kInitialChorusTime = 0;
const uint8_t kInitialChorusFeedback = 5;
const uint8_t kInitialChorusRate = 4;
const uint8_t kInitialChorusDry = 15;
const uint8_t kInitialChorusWet = 15;

struct ChorusTimeData {
    char nameShortR[5];
    float data;
};

const struct ChorusTimeData kChorusTimeDataLibrary[] = {
    {"  10", 0.010}, {"  15", 0.015}, {"  20", 0.020}, {"  25", 0.025}, {"  30", 0.030}, {"  35", 0.035}, {"  40", 0.040}, {"  45", 0.045}, {"  50", 0.050}};

struct ChorusFeedbackData {
    char nameShortR[5];
    float data;
};

const struct ChorusFeedbackData kChorusFeedbackDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct ChorusRateData {
    char nameShortR[5];
    float data;
};

const struct ChorusRateData kChorusRateDataLibrary[] = {
    {" 0.1", 0.10}, {" 0.2", 0.20}, {" 0.3", 0.30}, {" 0.4", 0.40}, {" 0.5", 0.50}, {" 0.6", 0.60}, {" 0.7", 0.70}, {" 0.8", 0.80}, {" 0.9", 0.90}, {" 1.0", 1.00}, {" 1.5", 1.50}, {" 2.0", 2.00}, {" 2.5", 2.50}, {" 3.0", 3.00}, {" 3.5", 3.50}, {" 4.0", 4.00}, {" 4.5", 4.50}, {" 5.0", 5.00}, {" 5.5", 5.50}, {" 6.0", 6.00}, {" 6.5", 6.50}, {" 7.0", 7.00}, {" 7.5", 7.50}, {" 8.0", 8.00}, {" 8.5", 8.50}, {" 9.0", 9.00}};

struct ChorusDelay {
    float time;
    float depth;
    float rate;
    float mix;

    uint16_t lag;
    uint16_t playInterval;
    float chorusInterval;

    float shiftFreq;
    float shiftMax;
    float shiftMin;
    float shiftInterval;
    float shiftInc;
};

struct Chorus {
    bool active;

    uint8_t aTime;
    uint8_t bFeedback;
    uint8_t cRate;
    uint8_t dDry;
    uint8_t eWet;

    float time;
    float feedback;
    float rate;

    float depth;
    uint16_t recordInterval;
    const float delayCoef[2] = {1, 0.75};
    ChorusDelay chorusDelay[2];

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialChorusActive;

        aTime = kInitialChorusTime;
        bFeedback = kInitialChorusFeedback;
        cRate = kInitialChorusRate;
        dDry = kInitialChorusDry;
        eWet = kInitialChorusWet;

        time = kChorusTimeDataLibrary[kInitialChorusTime].data;
        feedback = kChorusFeedbackDataLibrary[kInitialChorusFeedback].data;
        rate = kChorusRateDataLibrary[kInitialChorusRate].data;

        depth = 0.80;
        recordInterval = 0;

        for (uint8_t i = 0; i < 2; i++) {
            ChorusDelay &cD = chorusDelay[i];
            cD.time = time * delayCoef[i];
            cD.depth = depth * delayCoef[i];
            cD.rate = rate * delayCoef[i];
            cD.mix = 0.50;

            cD.lag = cD.time * kAudioSampleRate;
            cD.playInterval = (kChorusBufferSize - 1) - cD.lag;

            cD.shiftFreq = cD.rate * 2;
            cD.shiftMax = (cD.depth * cD.lag) - 2;
            cD.shiftMin = -cD.shiftMax;
            cD.shiftInterval = cD.shiftMin;
            cD.shiftInc = ((cD.shiftMax - cD.shiftMin) * cD.shiftFreq) / kAudioSampleRate;
        }
    }

    void update() {
        for (uint8_t i = 0; i < 2; i++) {
            ChorusDelay &cD = chorusDelay[i];
            cD.time = time * delayCoef[i];
            cD.depth = depth * delayCoef[i];
            cD.rate = rate * delayCoef[i];
            cD.mix = 0.50;

            cD.lag = cD.time * kAudioSampleRate;
            int32_t playInterval_ = recordInterval - cD.lag;
            if (playInterval_ < 0)
                playInterval_ += kChorusBufferSize;
            cD.playInterval = playInterval_;

            cD.shiftFreq = cD.rate * 2;
            cD.shiftMax = (cD.depth * cD.lag) - 2;
            cD.shiftMin = -cD.shiftMax;
            cD.shiftInc = ((cD.shiftMax - cD.shiftMin) * cD.shiftFreq) / kAudioSampleRate;
        }
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Flanger Constants --------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint16_t kFlangerBufferSize = 250;

const uint8_t kMinFlangerTime = 0;
const uint8_t kMaxFlangerTime = 8;

const uint8_t kMinFlangerFeedback = 0;
const uint8_t kMaxFlangerFeedback = 19;

const uint8_t kMinFlangerRate = 0;
const uint8_t kMaxFlangerRate = 25;

const uint8_t kMinFlangerDry = 0;
const uint8_t kMaxFlangerDry = 20;

const uint8_t kMinFlangerWet = 0;
const uint8_t kMaxFlangerWet = 20;

const bool kInitialFlangerActive = false;
const uint8_t kInitialFlangerTime = 3;
const uint8_t kInitialFlangerFeedback = 5;
const uint8_t kInitialFlangerRate = 4;
const uint8_t kInitialFlangerDry = 15;
const uint8_t kInitialFlangerWet = 15;

struct FlangerTimeData {
    char nameShortR[5];
    float data;
};

const struct FlangerTimeData kFlangerTimeDataLibrary[] = {
    {" 1.0", 0.0010}, {" 1.5", 0.0015}, {" 2.0", 0.0020}, {" 2.5", 0.0025}, {" 3.0", 0.0030}, {" 3.5", 0.0035}, {" 4.0", 0.0040}, {" 4.5", 0.0045}, {" 5.0", 0.0050}};

struct FlangerFeedbackData {
    char nameShortR[5];
    float data;
};

const struct FlangerFeedbackData kFlangerFeedbackDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct FlangerRateData {
    char nameShortR[5];
    float data;
};

const struct FlangerRateData kFlangerRateDataLibrary[] = {
    {" 0.1", 0.10}, {" 0.2", 0.20}, {" 0.3", 0.30}, {" 0.4", 0.40}, {" 0.5", 0.50}, {" 0.6", 0.60}, {" 0.7", 0.70}, {" 0.8", 0.80}, {" 0.9", 0.90}, {" 1.0", 1.00}, {" 1.5", 1.50}, {" 2.0", 2.00}, {" 2.5", 2.50}, {" 3.0", 3.00}, {" 3.5", 3.50}, {" 4.0", 4.00}, {" 4.5", 4.50}, {" 5.0", 5.00}, {" 5.5", 5.50}, {" 6.0", 6.00}, {" 6.5", 6.50}, {" 7.0", 7.00}, {" 7.5", 7.50}, {" 8.0", 8.00}, {" 8.5", 8.50}, {" 9.0", 9.00}};

struct Flanger {
    bool active;

    uint8_t aTime;
    uint8_t bFeedback;
    uint8_t cRate;
    uint8_t dDry;
    uint8_t eWet;

    float time;
    float feedback;
    float rate;

    float depth;
    uint16_t lag;
    uint16_t recordInterval;
    uint16_t playInterval;
    float flangerInterval;
    float shiftInterval;
    float shiftFreq;
    float shiftMin;
    float shiftMax;
    float shiftInc;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialFlangerActive;

        aTime = kInitialFlangerTime;
        bFeedback = kInitialFlangerFeedback;
        cRate = kInitialFlangerRate;
        dDry = kInitialFlangerDry;
        eWet = kInitialFlangerWet;

        time = kFlangerTimeDataLibrary[kInitialFlangerTime].data;
        feedback = kFlangerFeedbackDataLibrary[kInitialFlangerFeedback].data;
        rate = kFlangerRateDataLibrary[kInitialFlangerRate].data;

        depth = 0.90;
        recordInterval = 0;
        lag = time * kAudioSampleRate;
        playInterval = (kFlangerBufferSize - 1) - lag;
        shiftInterval = 0;
        shiftFreq = rate * 2;
        shiftMin = 0;
        shiftMax = (depth * lag) - 2;
        shiftInc = (shiftMax * shiftFreq) / kAudioSampleRate;
    }

    void update() {
        lag = time * kAudioSampleRate;
        int32_t playInterval_ = recordInterval - lag;
        if (playInterval_ < 0)
            playInterval_ += kFlangerBufferSize;
        playInterval = playInterval_;

        shiftFreq = rate * 2;
        shiftMin = 0;
        shiftMax = (depth * lag) - 2;
        shiftInc = (shiftMax * shiftFreq) / kAudioSampleRate;
    }

    void cleanMemory() {
        recordInterval = 0;
        playInterval = (kFlangerBufferSize - 1) - lag;
        shiftInterval = 0;
        shiftInc = (shiftMax * shiftFreq) / kAudioSampleRate;
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Phaser Constants ---------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinPhaserStartFreq = 0;
const uint8_t kMaxPhaserStartFreq = 46;

const uint8_t kMinPhaserEndFreq = 0;
const uint8_t kMaxPhaserEndFreq = 46;

const uint8_t kMinPhaserRate = 0;
const uint8_t kMaxPhaserRate = 25;

const uint8_t kMinPhaserDry = 0;
const uint8_t kMaxPhaserDry = 20;

const uint8_t kMinPhaserWet = 0;
const uint8_t kMaxPhaserWet = 20;

const bool kInitialPhaserActive = false;
const uint8_t kInitialPhaserStartFreq = 27;
const uint8_t kInitialPhaserEndFreq = 31;
const uint8_t kInitialPhaserRate = 4;
const uint8_t kInitialPhaserDry = 15;
const uint8_t kInitialPhaserWet = 15;

struct PhaserFreqData {
    char nameShortR[5];
    uint16_t data;
};

const struct PhaserFreqData kPhaserFreqDataLibrary[] = {
    {"  10", 10}, {"  20", 20}, {"  30", 30}, {"  40", 40}, {"  50", 50}, {"  60", 60}, {"  70", 70}, {"  80", 80}, {"  90", 90}, {" 100", 100}, {" 150", 150}, {" 200", 200}, {" 250", 250}, {" 300", 300}, {" 350", 350}, {" 400", 400}, {" 450", 450}, {" 500", 500}, {" 550", 550}, {" 600", 600}, {" 650", 650}, {" 700", 700}, {" 750", 750}, {" 800", 800}, {" 850", 850}, {" 900", 900}, {" 950", 950}, {"  1K", 1000}, {"  2K", 2000}, {"  3K", 3000}, {"  4K", 4000}, {"  5K", 5000}, {"  6K", 6000}, {"  7K", 7000}, {"  8K", 8000}, {"  9K", 9000}, {" 10K", 10000}, {" 11K", 11000}, {" 12K", 12000}, {" 13K", 13000}, {" 14K", 14000}, {" 15K", 15000}, {" 16K", 16000}, {" 17K", 17000}, {" 18K", 18000}, {" 19K", 19000}, {" 20K", 20000}};

struct PhaserRateData {
    char nameShortR[5];
    float data;
};

const struct PhaserRateData kPhaserRateDataLibrary[] = {
    {" 0.1", 0.10}, {" 0.2", 0.20}, {" 0.3", 0.30}, {" 0.4", 0.40}, {" 0.5", 0.50}, {" 0.6", 0.60}, {" 0.7", 0.70}, {" 0.8", 0.80}, {" 0.9", 0.90}, {" 1.0", 1.00}, {" 1.5", 1.50}, {" 2.0", 2.00}, {" 2.5", 2.50}, {" 3.0", 3.00}, {" 3.5", 3.50}, {" 4.0", 4.00}, {" 4.5", 4.50}, {" 5.0", 5.00}, {" 5.5", 5.50}, {" 6.0", 6.00}, {" 6.5", 6.50}, {" 7.0", 7.00}, {" 7.5", 7.50}, {" 8.0", 8.00}, {" 8.5", 8.50}, {" 9.0", 9.00}};

struct Phaser {
    bool active;

    uint8_t aStartFreq;
    uint8_t bEndFreq;
    uint8_t cRate;
    uint8_t dDry;
    uint8_t eWet;

    uint16_t startFreq;
    uint16_t endFreq;
    float rate;

    float centerFreq;
    float depthFreq;
    float lfo;
    float dataX;
    float dataY;
    float Ts;

    float ff[2] = {0, 0};
    float fb[2] = {0, 0};
    float Q = 0.5;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialPhaserActive;

        aStartFreq = kInitialPhaserStartFreq;
        bEndFreq = kInitialPhaserEndFreq;
        cRate = kInitialPhaserRate;
        dDry = kInitialPhaserDry;
        eWet = kInitialPhaserWet;

        startFreq = kPhaserFreqDataLibrary[kInitialPhaserStartFreq].data;
        endFreq = kPhaserFreqDataLibrary[kInitialPhaserEndFreq].data;
        rate = kPhaserRateDataLibrary[kInitialPhaserRate].data;

        centerFreq = startFreq + ((endFreq - startFreq) / 2.0);
        depthFreq = (endFreq - centerFreq) * 0.9;
        lfo = 0;
        dataX = 0;
        dataY = 0;
        Ts = 1.0f / kAudioSampleRate;
    }

    void update() {
        centerFreq = startFreq + ((endFreq - startFreq) / 2.0);
        depthFreq = (endFreq - centerFreq) * 0.9;
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Compressor Constants -----------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinCompressorThreshold = 0;
const uint8_t kMaxCompressorThreshold = 24;

const uint8_t kMinCompressorRate = 0;
const uint8_t kMaxCompressorRate = 29;

const uint8_t kMinCompressorAttackTime = 0;
const uint8_t kMaxCompressorAttackTime = 18;

const uint8_t kMinCompressorReleaseTime = 0;
const uint8_t kMaxCompressorReleaseTime = 18;

const uint8_t kMinCompressorDry = 0;
const uint8_t kMaxCompressorDry = 20;

const uint8_t kMinCompressorWet = 0;
const uint8_t kMaxCompressorWet = 20;

const bool kInitialCompressorActive = false;
const uint8_t kInitialCompressorThreshold = 18;
const uint8_t kInitialCompressorRate = 3;
const uint8_t kInitialCompressorAttackTime = 0;
const uint8_t kInitialCompressorReleaseTime = 9;
const uint8_t kInitialCompressorMix = 20;

struct CompressorThresholdData {
    char nameShortR[5];
    float data;
};

const struct CompressorThresholdData kCompressorThresholdDataLibrary[] = {
    {" -24", -24.0}, {" -23", -23.0}, {" -22", -22.0}, {" -21", -21.0}, {" -20", -20.0}, {" -19", -19.0}, {" -18", -18.0}, {" -17", -17.0}, {" -16", -16.0}, {" -15", -15.0}, {" -14", -14.0}, {" -13", -13.0}, {" -12", -12.0}, {" -11", -11.0}, {" -10", -10.0}, {" -09", -9.0}, {" -08", -8.0}, {" -07", -7.0}, {" -06", -6.0}, {" -05", -5.0}, {" -04", -4.0}, {" -03", -3.0}, {" -02", -2.0}, {" -01", -1.0}, {"  00", 0.0}};

struct CompressorRateData {
    char nameShortR[5];
    float data;
};

const struct CompressorRateData kCompressorRateDataLibrary[] = {
    {"  01", 1.0}, {"  02", 2.0}, {"  03", 3.0}, {"  04", 4.0}, {"  05", 5.0}, {"  06", 6.0}, {"  07", 7.0}, {"  08", 8.0}, {"  09", 9.0}, {"  10", 10.0}, {"  11", 11.0}, {"  12", 12.0}, {"  13", 13.0}, {"  14", 14.0}, {"  15", 15.0}, {"  16", 16.0}, {"  17", 17.0}, {"  18", 18.0}, {"  19", 19.0}, {"  20", 20.0}, {"  21", 21.0}, {"  22", 22.0}, {"  23", 23.0}, {"  24", 24.0}, {"  25", 25.0}, {"  26", 26.0}, {"  27", 27.0}, {"  28", 28.0}, {"  29", 29.0}, {"  30", 30.0}};

struct CompressorAttackTimeData {
    char nameShortR[5];
    float data;
};

const struct CompressorAttackTimeData kCompressorAttackTimeDataLibrary[] = {
    {"  01", 0.001}, {"  02", 0.002}, {"  03", 0.003}, {"  04", 0.004}, {"  05", 0.005}, {"  06", 0.006}, {"  07", 0.007}, {"  08", 0.008}, {"  09", 0.009}, {"  10", 0.010}, {"  20", 0.020}, {"  30", 0.030}, {"  40", 0.040}, {"  50", 0.050}, {"  60", 0.060}, {"  70", 0.070}, {"  80", 0.080}, {"  90", 0.090}, {" 100", 0.100}};

struct CompressorReleaseTimeData {
    char nameShortR[5];
    float data;
};

const struct CompressorReleaseTimeData kCompressorReleaseTimeDataLibrary[] = {
    {"  10", 0.010}, {"  20", 0.020}, {"  30", 0.030}, {"  40", 0.040}, {"  50", 0.050}, {"  60", 0.060}, {"  70", 0.070}, {"  80", 0.080}, {"  90", 0.090}, {" 100", 0.100}, {" 200", 0.200}, {" 300", 0.300}, {" 400", 0.400}, {" 500", 0.500}, {" 600", 0.600}, {" 700", 0.700}, {" 800", 0.800}, {" 900", 0.900}, {"  1K", 1.000}};

struct Compressor {
    bool active;

    uint8_t aThreshold;
    uint8_t bRate;
    uint8_t cAttackTime;
    uint8_t dReleaseTime;
    uint8_t eMix;

    float threshold;
    float rate;
    float attackTime;
    float releaseTime;

    float attackAlpha;
    float releaseAlpha;
    float gainSmoothPrev;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialCompressorActive;

        aThreshold = kInitialCompressorThreshold;
        bRate = kInitialCompressorRate;
        cAttackTime = kInitialCompressorAttackTime;
        dReleaseTime = kInitialCompressorReleaseTime;
        eMix = kInitialCompressorMix;

        threshold = kCompressorThresholdDataLibrary[kInitialCompressorThreshold].data;
        rate = kCompressorRateDataLibrary[kInitialCompressorRate].data;
        attackTime = kCompressorAttackTimeDataLibrary[kInitialCompressorAttackTime].data;
        releaseTime = kCompressorReleaseTimeDataLibrary[kInitialCompressorReleaseTime].data;

        attackAlpha = exp(-log(9) / (kAudioSampleRate * attackTime));
        releaseAlpha = exp(-log(9) / (kAudioSampleRate * releaseTime));
        gainSmoothPrev = 0;
    }

    void update() {
        attackAlpha = exp(-log(9) / (kAudioSampleRate * attackTime));
        releaseAlpha = exp(-log(9) / (kAudioSampleRate * releaseTime));
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Expander Constants -------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinExpanderThreshold = 0;
const uint8_t kMaxExpanderThreshold = 24;

const uint8_t kMinExpanderRate = 0;
const uint8_t kMaxExpanderRate = 29;

const uint8_t kMinExpanderAttackTime = 0;
const uint8_t kMaxExpanderAttackTime = 18;

const uint8_t kMinExpanderReleaseTime = 0;
const uint8_t kMaxExpanderReleaseTime = 18;

const uint8_t kMinExpanderDry = 0;
const uint8_t kMaxExpanderDry = 20;

const uint8_t kMinExpanderWet = 0;
const uint8_t kMaxExpanderWet = 20;

const bool kInitialExpanderActive = false;
const uint8_t kInitialExpanderThreshold = 12;
const uint8_t kInitialExpanderRate = 3;
const uint8_t kInitialExpanderAttackTime = 0;
const uint8_t kInitialExpanderReleaseTime = 13;
const uint8_t kInitialExpanderMix = 20;

struct ExpanderThresholdData {
    char nameShortR[5];
    float data;
};

const struct ExpanderThresholdData kExpanderThresholdDataLibrary[] = {
    {" -24", -24.0}, {" -23", -23.0}, {" -22", -22.0}, {" -21", -21.0}, {" -20", -20.0}, {" -19", -19.0}, {" -18", -18.0}, {" -17", -17.0}, {" -16", -16.0}, {" -15", -15.0}, {" -14", -14.0}, {" -13", -13.0}, {" -12", -12.0}, {" -11", -11.0}, {" -10", -10.0}, {" -09", -9.0}, {" -08", -8.0}, {" -07", -7.0}, {" -06", -6.0}, {" -05", -5.0}, {" -04", -4.0}, {" -03", -3.0}, {" -02", -2.0}, {" -01", -1.0}, {"  00", 0.0}};

struct ExpanderRateData {
    char nameShortR[5];
    float data;
};

const struct ExpanderRateData kExpanderRateDataLibrary[] = {
    {"  01", 1.0}, {"  02", 2.0}, {"  03", 3.0}, {"  04", 4.0}, {"  05", 5.0}, {"  06", 6.0}, {"  07", 7.0}, {"  08", 8.0}, {"  09", 9.0}, {"  10", 10.0}, {"  11", 11.0}, {"  12", 12.0}, {"  13", 13.0}, {"  14", 14.0}, {"  15", 15.0}, {"  16", 16.0}, {"  17", 17.0}, {"  18", 18.0}, {"  19", 19.0}, {"  20", 20.0}, {"  21", 21.0}, {"  22", 22.0}, {"  23", 23.0}, {"  24", 24.0}, {"  25", 25.0}, {"  26", 26.0}, {"  27", 27.0}, {"  28", 28.0}, {"  29", 29.0}, {"  30", 30.0}};

struct ExpanderAttackTimeData {
    char nameShortR[5];
    float data;
};

const struct ExpanderAttackTimeData kExpanderAttackTimeDataLibrary[] = {
    {"  01", 0.001}, {"  02", 0.002}, {"  03", 0.003}, {"  04", 0.004}, {"  05", 0.005}, {"  06", 0.006}, {"  07", 0.007}, {"  08", 0.008}, {"  09", 0.009}, {"  10", 0.010}, {"  20", 0.020}, {"  30", 0.030}, {"  40", 0.040}, {"  50", 0.050}, {"  60", 0.060}, {"  70", 0.070}, {"  80", 0.080}, {"  90", 0.090}, {" 100", 0.100}};

struct ExpanderReleaseTimeData {
    char nameShortR[5];
    float data;
};

const struct ExpanderReleaseTimeData kExpanderReleaseTimeDataLibrary[] = {
    {"  10", 0.010}, {"  20", 0.020}, {"  30", 0.030}, {"  40", 0.040}, {"  50", 0.050}, {"  60", 0.060}, {"  70", 0.070}, {"  80", 0.080}, {"  90", 0.090}, {" 100", 0.100}, {" 200", 0.200}, {" 300", 0.300}, {" 400", 0.400}, {" 500", 0.500}, {" 600", 0.600}, {" 700", 0.700}, {" 800", 0.800}, {" 900", 0.900}, {"  1K", 1.000}};

struct Expander {
    bool active;

    uint8_t aThreshold;
    uint8_t bRate;
    uint8_t cAttackTime;
    uint8_t dReleaseTime;
    uint8_t eMix;

    float threshold;
    float rate;
    float attackTime;
    float releaseTime;

    float attackAlpha;
    float releaseAlpha;
    float gainSmoothPrev;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialExpanderActive;

        aThreshold = kInitialExpanderThreshold;
        bRate = kInitialExpanderRate;
        cAttackTime = kInitialExpanderAttackTime;
        dReleaseTime = kInitialExpanderReleaseTime;
        eMix = kInitialExpanderMix;

        threshold = kExpanderThresholdDataLibrary[kInitialExpanderThreshold].data;
        rate = kExpanderRateDataLibrary[kInitialExpanderRate].data;
        attackTime = kExpanderAttackTimeDataLibrary[kInitialExpanderAttackTime].data;
        releaseTime = kExpanderReleaseTimeDataLibrary[kInitialExpanderReleaseTime].data;

        attackAlpha = exp(-log(9) / (kAudioSampleRate * attackTime));
        releaseAlpha = exp(-log(9) / (kAudioSampleRate * releaseTime));
        gainSmoothPrev = -144;
    }

    void update() {
        attackAlpha = exp(-log(9) / (kAudioSampleRate * attackTime));
        releaseAlpha = exp(-log(9) / (kAudioSampleRate * releaseTime));
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Overdrive Constants ------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinOverdriveGain = 0;
const uint8_t kMaxOverdriveGain = 24;

const uint8_t kMinOverdriveThreshold = 0;
const uint8_t kMaxOverdriveThreshold = 24;

const uint8_t kMinOverdriveTone = 0;
const uint8_t kMaxOverdriveTone = 46;

const uint8_t kMinOverdriveDry = 0;
const uint8_t kMaxOverdriveDry = 20;

const uint8_t kMinOverdriveWet = 0;
const uint8_t kMaxOverdriveWet = 20;

const bool kInitialOverdriveActive = false;
const uint8_t kInitialOverdriveGain = 15;
const uint8_t kInitialOverdriveThreshold = 18;
const uint8_t kInitialOverdriveTone = 31;
const uint8_t kInitialOverdriveDry = 5;
const uint8_t kInitialOverdriveWet = 15;

struct OverdriveGainData {
    char nameShortR[5];
    float data;
};

const struct OverdriveGainData kOverdriveGainDataLibrary[] = {
    {"  00", 0.0}, {"  01", 1.0}, {"  02", 2.0}, {"  03", 3.0}, {"  04", 4.0}, {"  05", 5.0}, {"  06", 6.0}, {"  07", 7.0}, {"  08", 8.0}, {"  09", 9.0}, {"  10", 10.0}, {"  11", 11.0}, {"  12", 12.0}, {"  13", 13.0}, {"  14", 14.0}, {"  15", 15.0}, {"  16", 16.0}, {"  17", 17.0}, {"  18", 18.0}, {"  19", 19.0}, {"  20", 20.0}, {"  21", 21.0}, {"  22", 22.0}, {"  23", 23.0}, {"  24", 24.0}};

struct OverdriveThresholdData {
    char nameShortR[5];
    float data;
};

const struct OverdriveThresholdData kOverdriveThresholdDataLibrary[] = {
    {" -24", -24.0}, {" -23", -23.0}, {" -22", -22.0}, {" -21", -21.0}, {" -20", -20.0}, {" -19", -19.0}, {" -18", -18.0}, {" -17", -17.0}, {" -16", -16.0}, {" -15", -15.0}, {" -14", -14.0}, {" -13", -13.0}, {" -12", -12.0}, {" -11", -11.0}, {" -10", -10.0}, {" -09", -9.0}, {" -08", -8.0}, {" -07", -7.0}, {" -06", -6.0}, {" -05", -5.0}, {" -04", -4.0}, {" -03", -3.0}, {" -02", -2.0}, {" -01", -1.0}, {"  00", 0.0}};

struct OverdriveToneData {
    char nameShortR[5];
    float data;
};

const struct OverdriveToneData kOverdriveToneDataLibrary[] = {
    {"  10", 10}, {"  20", 20}, {"  30", 30}, {"  40", 40}, {"  50", 50}, {"  60", 60}, {"  70", 70}, {"  80", 80}, {"  90", 90}, {" 100", 100}, {" 150", 150}, {" 200", 200}, {" 250", 250}, {" 300", 300}, {" 350", 350}, {" 400", 400}, {" 450", 450}, {" 500", 500}, {" 550", 550}, {" 600", 600}, {" 650", 650}, {" 700", 700}, {" 750", 750}, {" 800", 800}, {" 850", 850}, {" 900", 900}, {" 950", 950}, {"  1K", 1000}, {"  2K", 2000}, {"  3K", 3000}, {"  4K", 4000}, {"  5K", 5000}, {"  6K", 6000}, {"  7K", 7000}, {"  8K", 8000}, {"  9K", 9000}, {" 10K", 10000}, {" 11K", 11000}, {" 12K", 12000}, {" 13K", 13000}, {" 14K", 14000}, {" 15K", 15000}, {" 16K", 16000}, {" 7K", 17000}, {" 18K", 18000}, {" 19K", 19000}, {" 20K", 20000}};

struct Overdrive {
    bool active;

    uint8_t aGain;
    uint8_t bThreshold;
    uint8_t cTone;
    uint8_t dDry;
    uint8_t eWet;

    float gaindB;
    float gain;
    float thresholddB;
    float threshold;
    float tone;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    // filter data

    float a0;
    float a1;
    float a2;
    float b1;
    float b2;

    int32_t dataIn[3] = {0};
    int32_t dataOut[3] = {0};

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialOverdriveActive;

        aGain = kInitialOverdriveGain;
        bThreshold = kInitialOverdriveThreshold;
        cTone = kInitialOverdriveTone;
        dDry = kInitialOverdriveDry;
        eWet = kInitialOverdriveWet;

        gaindB = kOverdriveGainDataLibrary[kInitialOverdriveGain].data;
        thresholddB = kOverdriveThresholdDataLibrary[kInitialOverdriveThreshold].data;
        tone = kOverdriveToneDataLibrary[kInitialOverdriveTone].data;

        gain = pow(10.0f, gaindB / 20.0f);
        threshold = pow(10.0f, thresholddB / 20.0f);

        calculateFilterCoef();
    }

    void update() {
        gain = pow(10.0f, gaindB / 20.0f);
        threshold = pow(10.0f, thresholddB / 20.0f);

        calculateFilterCoef();
    }

    void calculateFilterCoef() {
        float Q = 0.707;
        float norm;
        float K = tan(M_PI * tone / kAudioSampleRate);

        norm = 1.0f / (1.0f + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2.0f * a0;
        a2 = a0;
        b1 = 2.0f * (K * K - 1.0f) * norm;
        b2 = (1.0f - K / Q + K * K) * norm;
    }

    void cleanMemory() {
        dataIn[0] = 0;
        dataIn[1] = 0;
        dataIn[2] = 0;
        dataOut[0] = 0;
        dataOut[1] = 0;
        dataOut[2] = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Distortion Constants -----------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinDistortionGain = 0;
const uint8_t kMaxDistortionGain = 24;

const uint8_t kMinDistortionThreshold = 0;
const uint8_t kMaxDistortionThreshold = 24;

const uint8_t kMinDistortionTone = 0;
const uint8_t kMaxDistortionTone = 46;

const uint8_t kMinDistortionDry = 0;
const uint8_t kMaxDistortionDry = 20;

const uint8_t kMinDistortionWet = 0;
const uint8_t kMaxDistortionWet = 20;

const bool kInitialDistortionActive = false;
const uint8_t kInitialDistortionGain = 15;
const uint8_t kInitialDistortionThreshold = 18;
const uint8_t kInitialDistortionTone = 31;
const uint8_t kInitialDistortionDry = 5;
const uint8_t kInitialDistortionWet = 15;

struct DistortionGainData {
    char nameShortR[5];
    float data;
};

const struct DistortionGainData kDistortionGainDataLibrary[] = {
    {"  00", 0.0}, {"  01", 1.0}, {"  02", 2.0}, {"  03", 3.0}, {"  04", 4.0}, {"  05", 5.0}, {"  06", 6.0}, {"  07", 7.0}, {"  08", 8.0}, {"  09", 9.0}, {"  10", 10.0}, {"  11", 11.0}, {"  12", 12.0}, {"  13", 13.0}, {"  14", 14.0}, {"  15", 15.0}, {"  16", 16.0}, {"  17", 17.0}, {"  18", 18.0}, {"  19", 19.0}, {"  20", 20.0}, {"  21", 21.0}, {"  22", 22.0}, {"  23", 23.0}, {"  24", 24.0}};

struct DistortionThresholdData {
    char nameShortR[5];
    float data;
};

const struct DistortionThresholdData kDistortionThresholdDataLibrary[] = {
    {" -24", -24.0}, {" -23", -23.0}, {" -22", -22.0}, {" -21", -21.0}, {" -20", -20.0}, {" -19", -19.0}, {" -18", -18.0}, {" -17", -17.0}, {" -16", -16.0}, {" -15", -15.0}, {" -14", -14.0}, {" -13", -13.0}, {" -12", -12.0}, {" -11", -11.0}, {" -10", -10.0}, {" -09", -9.0}, {" -08", -8.0}, {" -07", -7.0}, {" -06", -6.0}, {" -05", -5.0}, {" -04", -4.0}, {" -03", -3.0}, {" -02", -2.0}, {" -01", -1.0}, {"  00", 0.0}};

struct DistortionToneData {
    char nameShortR[5];
    float data;
};

const struct DistortionToneData kDistortionToneDataLibrary[] = {
    {"  10", 10}, {"  20", 20}, {"  30", 30}, {"  40", 40}, {"  50", 50}, {"  60", 60}, {"  70", 70}, {"  80", 80}, {"  90", 90}, {" 100", 100}, {" 150", 150}, {" 200", 200}, {" 250", 250}, {" 300", 300}, {" 350", 350}, {" 400", 400}, {" 450", 450}, {" 500", 500}, {" 550", 550}, {" 600", 600}, {" 650", 650}, {" 700", 700}, {" 750", 750}, {" 800", 800}, {" 850", 850}, {" 900", 900}, {" 950", 950}, {"  1K", 1000}, {"  2K", 2000}, {"  3K", 3000}, {"  4K", 4000}, {"  5K", 5000}, {"  6K", 6000}, {"  7K", 7000}, {"  8K", 8000}, {"  9K", 9000}, {" 10K", 10000}, {" 11K", 11000}, {" 12K", 12000}, {" 13K", 13000}, {" 14K", 14000}, {" 15K", 15000}, {" 16K", 16000}, {" 17K", 17000}, {" 18K", 18000}, {" 19K", 19000}, {" 20K", 20000}};

struct Distortion {
    bool active;

    uint8_t aGain;
    uint8_t bThreshold;
    uint8_t cTone;
    uint8_t dDry;
    uint8_t eWet;

    float gaindB;
    float gain;
    float thresholddB;
    float threshold;
    float tone;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    // filter data

    float a0;
    float a1;
    float a2;
    float b1;
    float b2;

    int32_t dataIn[3] = {0};
    int32_t dataOut[3] = {0};

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialDistortionActive;

        aGain = kInitialDistortionGain;
        bThreshold = kInitialDistortionThreshold;
        cTone = kInitialDistortionTone;
        dDry = kInitialDistortionDry;
        eWet = kInitialDistortionWet;

        gaindB = kDistortionGainDataLibrary[kInitialDistortionGain].data;
        thresholddB = kDistortionThresholdDataLibrary[kInitialDistortionThreshold].data;
        tone = kDistortionToneDataLibrary[kInitialDistortionTone].data;

        gain = pow(10.0f, gaindB / 20.0f);
        threshold = pow(10.0f, thresholddB / 20.0f);

        calculateFilterCoef();
    }

    void update() {
        gain = pow(10.0f, gaindB / 20.0f);
        threshold = pow(10.0f, thresholddB / 20.0f);

        calculateFilterCoef();
    }

    void calculateFilterCoef() {
        float Q = 0.707;
        float norm;
        float K = tan(M_PI * tone / kAudioSampleRate);

        norm = 1.0f / (1.0f + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2.0f * a0;
        a2 = a0;
        b1 = 2.0f * (K * K - 1.0f) * norm;
        b2 = (1.0f - K / Q + K * K) * norm;
    }

    void cleanMemory() {
        dataIn[0] = 0;
        dataIn[1] = 0;
        dataIn[2] = 0;
        dataOut[0] = 0;
        dataOut[1] = 0;
        dataOut[2] = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Effect-Bitcrusher Constants -----------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinBitcrusherResolution = 0;
const uint8_t kMaxBitcrusherResolution = 23;

const uint8_t kMinBitcrusherSampleRate = 0;
const uint8_t kMaxBitcrusherSampleRate = 23;

const uint8_t kMinBitcrusherThreshold = 0;
const uint8_t kMaxBitcrusherThreshold = 24;

const uint8_t kMinBitcrusherDry = 0;
const uint8_t kMaxBitcrusherDry = 20;

const uint8_t kMinBitcrusherWet = 0;
const uint8_t kMaxBitcrusherWet = 20;

const bool kInitialBitcrusherActive = false;
const uint8_t kInitialBitcrusherResolution = 7;
const uint8_t kInitialBitcrusherSampleRate = 0;
const uint8_t kInitialBitcrusherThreshold = 24;
const uint8_t kInitialBitcrusherDry = 5;
const uint8_t kInitialBitcrusherWet = 15;

struct BitcrusherResolutionData {
    char nameShortR[5];
    uint8_t data;
};

const struct BitcrusherResolutionData kBitcrusherResolutionDataLibrary[] = {
    {"  01", 1}, {"  02", 2}, {"  03", 3}, {"  04", 4}, {"  05", 5}, {"  06", 6}, {"  07", 7}, {"  08", 8}, {"  09", 9}, {"  10", 10}, {"  11", 11}, {"  12", 12}, {"  13", 13}, {"  14", 14}, {"  15", 15}, {"  16", 16}, {"  17", 17}, {"  18", 18}, {"  19", 19}, {"  20", 20}, {"  21", 21}, {"  22", 22}, {"  23", 23}, {"  24", 24}};

struct BitcrusherSampleRateData {
    char nameShortR[5];
    uint8_t data;
};

const struct BitcrusherSampleRateData kBitcrusherSampleRateDataLibrary[] = {
    {"  01", 1}, {"  02", 2}, {"  03", 3}, {"  04", 4}, {"  05", 5}, {"  06", 6}, {"  07", 7}, {"  08", 8}, {"  09", 9}, {"  10", 10}, {"  11", 11}, {"  12", 12}, {"  13", 13}, {"  14", 14}, {"  15", 15}, {"  16", 16}, {"  17", 17}, {"  18", 18}, {"  19", 19}, {"  20", 20}, {"  21", 21}, {"  22", 22}, {"  23", 23}, {"  24", 24}};

struct BitcrusherThresholdData {
    char nameShortR[5];
    float data;
};

const struct BitcrusherThresholdData kBitcrusherThresholdDataLibrary[] = {
    {" -24", -24.0}, {" -23", -23.0}, {" -22", -22.0}, {" -21", -21.0}, {" -20", -20.0}, {" -19", -19.0}, {" -18", -18.0}, {" -17", -17.0}, {" -16", -16.0}, {" -15", -15.0}, {" -14", -14.0}, {" -13", -13.0}, {" -12", -12.0}, {" -11", -11.0}, {" -10", -10.0}, {" -09", -9.0}, {" -08", -8.0}, {" -07", -7.0}, {" -06", -6.0}, {" -05", -5.0}, {" -04", -4.0}, {" -03", -3.0}, {" -02", -2.0}, {" -01", -1.0}, {"  00", 0.0}};

typedef enum {
    BIT_CLIP = 0x00,
    BIT_FOLD = 0x01,
} BitcrusherMode;

struct Bitcrusher {
    bool active;

    uint8_t aResolution;
    uint8_t bSampleRate;
    uint8_t cThreshold;
    uint8_t dDry;
    uint8_t eWet;

    uint8_t resolution;
    uint8_t sampleRate;
    int8_t threshold;

    BitcrusherMode mode;

    uint16_t sampleCounter;
    uint16_t sampleCounterMax;
    uint32_t resModifier;
    float limitMultiplier;
    int32_t limitPos;
    int32_t limitNeg;
    int32_t sampleData;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    void initialize() {
        reset();
    }

    void reset() {
        active = kInitialBitcrusherActive;

        aResolution = kInitialBitcrusherResolution;
        bSampleRate = kInitialBitcrusherSampleRate;
        cThreshold = kInitialBitcrusherThreshold;
        dDry = kInitialBitcrusherDry;
        eWet = kInitialBitcrusherWet;

        mode = BIT_FOLD;

        resolution = kBitcrusherResolutionDataLibrary[kInitialBitcrusherResolution].data;
        sampleRate = kBitcrusherSampleRateDataLibrary[kInitialBitcrusherSampleRate].data;
        threshold = kBitcrusherThresholdDataLibrary[kInitialBitcrusherThreshold].data;

        sampleCounter = 0;
        sampleCounterMax = sampleRate;

        resModifier = 0;

        for (uint8_t i = 0; i < resolution; i++) {
            resModifier <<= 1;
            resModifier += 1;
        }

        for (uint8_t j = 0; j < (24 - resolution); j++) {
            resModifier <<= 1;
        }

        resModifier += 0xFF000000;

        limitMultiplier = pow(10.0, threshold / 20.0);
        limitPos = limitMultiplier * INT24_MAX;
        limitNeg = -limitPos;

        sampleData = 0;
    }

    void update() {
        sampleCounterMax = sampleRate;

        resModifier = 0;

        for (uint8_t i = 0; i < resolution; i++) {
            resModifier <<= 1;
            resModifier += 1;
        }

        for (uint8_t j = 0; j < (24 - resolution); j++) {
            resModifier <<= 1;
        }

        resModifier += 0xFF000000;

        limitMultiplier = pow(10.0, threshold / 20.0);
        limitPos = limitMultiplier * INT24_MAX;
        limitNeg = -limitPos;
    }

    void cleanMemory() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Effect Constants ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinEffectType = 0;
const uint8_t kMaxEffectType = 8;

const bool kInitialEffectActive = false;
const uint8_t kInitialEffectType = 0;

typedef enum {
    EF_DELAY = 0x00,
    EF_CHORUS = 0x01,
    EF_FLANGER = 0x02,
    EF_PHASER = 0x03,
    EF_COMPRESSOR = 0x04,
    EF_EXPANDER = 0x05,
    EF_OVERDRIVE = 0x06,
    EF_DISTORTION = 0x07,
    EF_BITCRUSHER = 0x08
} EffectType;

struct EffectTypeData {
    char nameShortL[5];
    char nameShortR[5];
};

const struct EffectTypeData kEffectTypeDataLibrary[] = {
    {"DEL ", " DEL"}, {"CHO ", " CHO"}, {"FLA ", " FLA"}, {"PHA ", " PHA"}, {"COM ", " COM"}, {"EXP ", " EXP"}, {"OVE ", " OVE"}, {"DIS ", " DIS"}, {"BIT ", " BIT"}};

struct EffectMixData {
    char nameShortR[5];
    float data;
};

const struct EffectMixData kEffectMixDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

const uint8_t kEffectLibrarySize = 2;
const uint8_t kSubEffectLibrarySize = 9;

const char aHeader[kSubEffectLibrarySize][5] = {"TIME", "TIME", "TIME", "STAR", "THRE", "THRE", "GAIN", "GAIN", "RESO"};
const char bHeader[kSubEffectLibrarySize][5] = {"LEV ", "FEED", "FEED", "END ", "RATE", "RATE", "THRE", "THRE", "SAMP"};
const char cHeader[kSubEffectLibrarySize][5] = {"FEED", "RATE", "RATE", "RATE", "ATT ", "ATT ", "TONE", "TONE", "THRE"};
const char dHeader[kSubEffectLibrarySize][5] = {"DRY ", "DRY ", "DRY ", "DRY ", "REL ", "REL ", "DRY ", "DRY ", "DRY "};
const char eHeader[kSubEffectLibrarySize][5] = {"WET ", "WET ", "WET ", "WET ", "MIX ", "MIX ", "WET ", "WET ", "WET "};

const char aSign[kSubEffectLibrarySize][4] = {"  %", " MS", " MS", " HZ", " DB", " DB", " DB", " DB", " BR"};
const char bSign[kSubEffectLibrarySize][4] = {"  %", "  %", "  %", " HZ", "  X", "  X", " DB", " DB", "  %"};
const char cSign[kSubEffectLibrarySize][4] = {"  %", " HZ", " HZ", " HZ", " MS", " MS", " HZ", " HZ", " DB"};
const char dSign[kSubEffectLibrarySize][4] = {"  %", "  %", "  %", "  %", " MS", " MS", "  %", "  %", "  %"};
const char eSign[kSubEffectLibrarySize][4] = {"  %", "  %", "  %", "  %", "  %", "  %", "  %", "  %", "  %"};

struct SubEffect {
    bool activeA;
    bool activeB;
    bool activeC;
    bool activeD;
    bool activeE;

    uint8_t kMinAData;
    uint8_t kMaxAData;

    uint8_t kMinBData;
    uint8_t kMaxBData;

    uint8_t kMinCData;
    uint8_t kMaxCData;

    uint8_t kMinDData;
    uint8_t kMaxDData;

    uint8_t kMinEData;
    uint8_t kMaxEData;

    uint8_t kInitialAData;
    uint8_t kInitialBData;
    uint8_t kInitialCData;
    uint8_t kInitialDData;
    uint8_t kInitialEData;

    uint8_t aData;
    uint8_t bData;
    uint8_t cData;
    uint8_t dData;
    uint8_t eData;
};

typedef enum {
    EF_ACTION_NONE = 0x00,
    EF_ACTION_UP = 0x01,
    EF_ACTION_DOWN = 0x02
} EffectTransitionAction;

typedef enum {
    EF_MODE_NONE = 0x00,
    EF_MODE_ACTIVE = 0x01,
    EF_MODE_TYPE = 0x02,
    EF_MODE_TIME = 0x03,
    EF_MODE_FEEDBACK = 0x04
} EffectTransitionMode;

typedef enum {
    EF_PHASE_NONE = 0x00,
    EF_PHASE_A = 0x01,
    EF_PHASE_B = 0x02
} EffectTransitionPhase;

struct EffectGenTransition {
    bool active;

    EffectTransitionMode mode;
    EffectTransitionPhase phase;

    bool activeActive;
    bool targetActive;

    uint8_t activeType;
    uint8_t targetType;

    float activeDry;
    float targetDry;
    EffectTransitionAction actionDry;

    float activeWet;
    float targetWet;
    EffectTransitionAction actionWet;

    float activeRecordWet;
    float targetRecordWet;
    EffectTransitionAction actionRecordWet;
};

struct EffectMixTransition {
    bool active;

    float targetDry;
    EffectTransitionAction actionDry;

    float targetWet;
    EffectTransitionAction actionWet;
};

struct Effect {
    uint8_t num;
    bool active;
    uint8_t type;

    float dryFloat;
    float wetFloat;

    float recordWetFloat;

    SubEffect subEffect[kSubEffectLibrarySize];

    EffectGenTransition genTransition;
    EffectMixTransition mixTransition;

    Delay delay;
    Chorus chorus;
    Flanger flanger;
    Phaser phaser;
    Compressor compressor;
    Expander expander;
    Overdrive overdrive;
    Distortion distortion;
    Bitcrusher bitcrusher;

    uint32_t delayAddress;
    uint32_t chorusAddress;
    int32_t flangerBuffer[kFlangerBufferSize];

    void initialize() {
        switch (num) {
        case 0:
            delayAddress = RAM_DELAY_0;
            chorusAddress = RAM_CHORUS_0;
            break;

        case 1:
            delayAddress = RAM_DELAY_1;
            chorusAddress = RAM_CHORUS_1;
            break;
        }

        reset();
    }

    void reset() {
        active = kInitialEffectActive;
        type = kInitialEffectType;

        // delay
        subEffect[0].activeA = true;
        subEffect[0].activeB = true;
        subEffect[0].activeC = true;
        subEffect[0].activeD = true;
        subEffect[0].activeE = true;

        subEffect[0].kMinAData = kMinDelayTime;
        subEffect[0].kMaxAData = kMaxDelayTime;

        subEffect[0].kMinBData = kMinDelayLevel;
        subEffect[0].kMaxBData = kMaxDelayLevel;

        subEffect[0].kMinCData = kMinDelayFeedback;
        subEffect[0].kMaxCData = kMaxDelayFeedback;

        subEffect[0].kMinDData = kMinDelayDry;
        subEffect[0].kMaxDData = kMaxDelayDry;

        subEffect[0].kMinEData = kMinDelayWet;
        subEffect[0].kMaxEData = kMaxDelayWet;

        subEffect[0].kInitialAData = kInitialDelayTime;
        subEffect[0].kInitialBData = kInitialDelayLevel;
        subEffect[0].kInitialCData = kInitialDelayFeedback;
        subEffect[0].kInitialDData = kInitialDelayDry;
        subEffect[0].kInitialEData = kInitialDelayWet;

        subEffect[0].aData = subEffect[0].kInitialAData;
        subEffect[0].bData = subEffect[0].kInitialBData;
        subEffect[0].cData = subEffect[0].kInitialCData;
        subEffect[0].dData = subEffect[0].kInitialDData;
        subEffect[0].eData = subEffect[0].kInitialEData;

        // chorus
        subEffect[1].activeA = true;
        subEffect[1].activeB = true;
        subEffect[1].activeC = true;
        subEffect[1].activeD = true;
        subEffect[1].activeE = true;

        subEffect[1].kMinAData = kMinChorusTime;
        subEffect[1].kMaxAData = kMaxChorusTime;

        subEffect[1].kMinBData = kMinChorusFeedback;
        subEffect[1].kMaxBData = kMaxChorusFeedback;

        subEffect[1].kMinCData = kMinChorusRate;
        subEffect[1].kMaxCData = kMaxChorusRate;

        subEffect[1].kMinDData = kMinChorusDry;
        subEffect[1].kMaxDData = kMaxChorusDry;

        subEffect[1].kMinEData = kMinChorusWet;
        subEffect[1].kMaxEData = kMaxChorusWet;

        subEffect[1].kInitialAData = kInitialChorusTime;
        subEffect[1].kInitialBData = kInitialChorusFeedback;
        subEffect[1].kInitialCData = kInitialChorusRate;
        subEffect[1].kInitialDData = kInitialChorusDry;
        subEffect[1].kInitialEData = kInitialChorusWet;

        subEffect[1].aData = subEffect[1].kInitialAData;
        subEffect[1].bData = subEffect[1].kInitialBData;
        subEffect[1].cData = subEffect[1].kInitialCData;
        subEffect[1].dData = subEffect[1].kInitialDData;
        subEffect[1].eData = subEffect[1].kInitialEData;

        // flanger
        subEffect[2].activeA = true;
        subEffect[2].activeB = true;
        subEffect[2].activeC = true;
        subEffect[2].activeD = true;
        subEffect[2].activeE = true;

        subEffect[2].kMinAData = kMinFlangerTime;
        subEffect[2].kMaxAData = kMaxFlangerTime;

        subEffect[2].kMinBData = kMinFlangerFeedback;
        subEffect[2].kMaxBData = kMaxFlangerFeedback;

        subEffect[2].kMinCData = kMinFlangerRate;
        subEffect[2].kMaxCData = kMaxFlangerRate;

        subEffect[2].kMinDData = kMinFlangerDry;
        subEffect[2].kMaxDData = kMaxFlangerDry;

        subEffect[2].kMinEData = kMinFlangerWet;
        subEffect[2].kMaxEData = kMaxFlangerWet;

        subEffect[2].kInitialAData = kInitialFlangerTime;
        subEffect[2].kInitialBData = kInitialFlangerFeedback;
        subEffect[2].kInitialCData = kInitialFlangerRate;
        subEffect[2].kInitialDData = kInitialFlangerDry;
        subEffect[2].kInitialEData = kInitialFlangerWet;

        subEffect[2].aData = subEffect[2].kInitialAData;
        subEffect[2].bData = subEffect[2].kInitialBData;
        subEffect[2].cData = subEffect[2].kInitialCData;
        subEffect[2].dData = subEffect[2].kInitialDData;
        subEffect[2].eData = subEffect[2].kInitialEData;

        // phaser
        subEffect[3].activeA = true;
        subEffect[3].activeB = true;
        subEffect[3].activeC = true;
        subEffect[3].activeD = true;
        subEffect[3].activeE = true;

        subEffect[3].kMinAData = kMinPhaserStartFreq;
        subEffect[3].kMaxAData = kMaxPhaserStartFreq;

        subEffect[3].kMinBData = kMinPhaserEndFreq;
        subEffect[3].kMaxBData = kMaxPhaserEndFreq;

        subEffect[3].kMinCData = kMinPhaserRate;
        subEffect[3].kMaxCData = kMaxPhaserRate;

        subEffect[3].kMinDData = kMinPhaserDry;
        subEffect[3].kMaxDData = kMaxPhaserDry;

        subEffect[3].kMinEData = kMinPhaserWet;
        subEffect[3].kMaxEData = kMaxPhaserWet;

        subEffect[3].kInitialAData = kInitialPhaserStartFreq;
        subEffect[3].kInitialBData = kInitialPhaserEndFreq;
        subEffect[3].kInitialCData = kInitialPhaserRate;
        subEffect[3].kInitialDData = kInitialPhaserDry;
        subEffect[3].kInitialEData = kInitialPhaserWet;

        subEffect[3].aData = subEffect[3].kInitialAData;
        subEffect[3].bData = subEffect[3].kInitialBData;
        subEffect[3].cData = subEffect[3].kInitialCData;
        subEffect[3].dData = subEffect[3].kInitialDData;
        subEffect[3].eData = subEffect[3].kInitialEData;

        // compressor
        subEffect[4].activeA = true;
        subEffect[4].activeB = true;
        subEffect[4].activeC = true;
        subEffect[4].activeD = true;
        subEffect[4].activeE = true;

        subEffect[4].kMinAData = kMinCompressorThreshold;
        subEffect[4].kMaxAData = kMaxCompressorThreshold;

        subEffect[4].kMinBData = kMinCompressorRate;
        subEffect[4].kMaxBData = kMaxCompressorRate;

        subEffect[4].kMinCData = kMinCompressorAttackTime;
        subEffect[4].kMaxCData = kMaxCompressorAttackTime;

        subEffect[4].kMinDData = kMinCompressorReleaseTime;
        subEffect[4].kMaxDData = kMaxCompressorReleaseTime;

        subEffect[4].kMinEData = kMinCompressorWet;
        subEffect[4].kMaxEData = kMaxCompressorWet;

        subEffect[4].kInitialAData = kInitialCompressorThreshold;
        subEffect[4].kInitialBData = kInitialCompressorRate;
        subEffect[4].kInitialCData = kInitialCompressorAttackTime;
        subEffect[4].kInitialDData = kInitialCompressorReleaseTime;
        subEffect[4].kInitialEData = kInitialCompressorMix;

        subEffect[4].aData = subEffect[4].kInitialAData;
        subEffect[4].bData = subEffect[4].kInitialBData;
        subEffect[4].cData = subEffect[4].kInitialCData;
        subEffect[4].dData = subEffect[4].kInitialDData;
        subEffect[4].eData = subEffect[4].kInitialEData;

        // expander
        subEffect[5].activeA = true;
        subEffect[5].activeB = true;
        subEffect[5].activeC = true;
        subEffect[5].activeD = true;
        subEffect[5].activeE = true;

        subEffect[5].kMinAData = kMinExpanderThreshold;
        subEffect[5].kMaxAData = kMaxExpanderThreshold;

        subEffect[5].kMinBData = kMinExpanderRate;
        subEffect[5].kMaxBData = kMaxExpanderRate;

        subEffect[5].kMinCData = kMinExpanderAttackTime;
        subEffect[5].kMaxCData = kMaxExpanderAttackTime;

        subEffect[5].kMinDData = kMinExpanderReleaseTime;
        subEffect[5].kMaxDData = kMaxExpanderReleaseTime;

        subEffect[5].kMinEData = kMinExpanderWet;
        subEffect[5].kMaxEData = kMaxExpanderWet;

        subEffect[5].kInitialAData = kInitialExpanderThreshold;
        subEffect[5].kInitialBData = kInitialExpanderRate;
        subEffect[5].kInitialCData = kInitialExpanderAttackTime;
        subEffect[5].kInitialDData = kInitialExpanderReleaseTime;
        subEffect[5].kInitialEData = kInitialExpanderMix;

        subEffect[5].aData = subEffect[5].kInitialAData;
        subEffect[5].bData = subEffect[5].kInitialBData;
        subEffect[5].cData = subEffect[5].kInitialCData;
        subEffect[5].dData = subEffect[5].kInitialDData;
        subEffect[5].eData = subEffect[5].kInitialEData;

        // overdrive
        subEffect[6].activeA = true;
        subEffect[6].activeB = true;
        subEffect[6].activeC = true;
        subEffect[6].activeD = true;
        subEffect[6].activeE = true;

        subEffect[6].kMinAData = kMinOverdriveGain;
        subEffect[6].kMaxAData = kMaxOverdriveGain;

        subEffect[6].kMinBData = kMinOverdriveThreshold;
        subEffect[6].kMaxBData = kMaxOverdriveThreshold;

        subEffect[6].kMinCData = kMinOverdriveTone;
        subEffect[6].kMaxCData = kMaxOverdriveTone;

        subEffect[6].kMinDData = kMinOverdriveDry;
        subEffect[6].kMaxDData = kMaxOverdriveDry;

        subEffect[6].kMinEData = kMinOverdriveWet;
        subEffect[6].kMaxEData = kMaxOverdriveWet;

        subEffect[6].kInitialAData = kInitialOverdriveGain;
        subEffect[6].kInitialBData = kInitialOverdriveThreshold;
        subEffect[6].kInitialCData = kInitialOverdriveTone;
        subEffect[6].kInitialDData = kInitialOverdriveDry;
        subEffect[6].kInitialEData = kInitialOverdriveWet;

        subEffect[6].aData = subEffect[6].kInitialAData;
        subEffect[6].bData = subEffect[6].kInitialBData;
        subEffect[6].cData = subEffect[6].kInitialCData;
        subEffect[6].dData = subEffect[6].kInitialDData;
        subEffect[6].eData = subEffect[6].kInitialEData;

        // distortion
        subEffect[7].activeA = true;
        subEffect[7].activeB = true;
        subEffect[7].activeC = true;
        subEffect[7].activeD = true;
        subEffect[7].activeE = true;

        subEffect[7].kMinAData = kMinDistortionGain;
        subEffect[7].kMaxAData = kMaxDistortionGain;

        subEffect[7].kMinBData = kMinDistortionThreshold;
        subEffect[7].kMaxBData = kMaxDistortionThreshold;

        subEffect[7].kMinCData = kMinDistortionTone;
        subEffect[7].kMaxCData = kMaxDistortionTone;

        subEffect[7].kMinDData = kMinDistortionDry;
        subEffect[7].kMaxDData = kMaxDistortionDry;

        subEffect[7].kMinEData = kMinDistortionWet;
        subEffect[7].kMaxEData = kMaxDistortionWet;

        subEffect[7].kInitialAData = kInitialDistortionGain;
        subEffect[7].kInitialBData = kInitialDistortionThreshold;
        subEffect[7].kInitialCData = kInitialDistortionTone;
        subEffect[7].kInitialDData = kInitialDistortionDry;
        subEffect[7].kInitialEData = kInitialDistortionWet;

        subEffect[7].aData = subEffect[7].kInitialAData;
        subEffect[7].bData = subEffect[7].kInitialBData;
        subEffect[7].cData = subEffect[7].kInitialCData;
        subEffect[7].dData = subEffect[7].kInitialDData;
        subEffect[7].eData = subEffect[7].kInitialEData;

        // bitcrusher
        subEffect[8].activeA = true;
        subEffect[8].activeB = true;
        subEffect[8].activeC = true;
        subEffect[8].activeD = true;
        subEffect[8].activeE = true;

        subEffect[8].kMinAData = kMinBitcrusherResolution;
        subEffect[8].kMaxAData = kMaxBitcrusherResolution;

        subEffect[8].kMinBData = kMinBitcrusherSampleRate;
        subEffect[8].kMaxBData = kMaxBitcrusherSampleRate;

        subEffect[8].kMinCData = kMinBitcrusherThreshold;
        subEffect[8].kMaxCData = kMaxBitcrusherThreshold;

        subEffect[8].kMinDData = kMinBitcrusherDry;
        subEffect[8].kMaxDData = kMaxBitcrusherDry;

        subEffect[8].kMinEData = kMinBitcrusherWet;
        subEffect[8].kMaxEData = kMaxBitcrusherWet;

        subEffect[8].kInitialAData = kInitialBitcrusherResolution;
        subEffect[8].kInitialBData = kInitialBitcrusherSampleRate;
        subEffect[8].kInitialCData = kInitialBitcrusherThreshold;
        subEffect[8].kInitialDData = kInitialBitcrusherDry;
        subEffect[8].kInitialEData = kInitialBitcrusherWet;

        subEffect[8].aData = subEffect[8].kInitialAData;
        subEffect[8].bData = subEffect[8].kInitialBData;
        subEffect[8].cData = subEffect[8].kInitialCData;
        subEffect[8].dData = subEffect[8].kInitialDData;
        subEffect[8].eData = subEffect[8].kInitialEData;

        delay.reset();
        chorus.reset();
        flanger.reset();
        phaser.reset();
        compressor.reset();
        expander.reset();
        overdrive.reset();
        distortion.reset();
        bitcrusher.reset();

        memset((uint8_t *)delayAddress, 0x00, kDelayByteSize);
        memset((uint8_t *)chorusAddress, 0x00, kChorusByteSize);
        memset(flangerBuffer, 0x00, sizeof(flangerBuffer));
    }

    Effect(uint8_t num_) : num(num_) {}
    ~Effect() {}
};

////////////////////////////////////////////////////////////////////////////////
/* Reverb Constants ----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinReverbSize = 0;
const uint8_t kMaxReverbSize = 20;

const uint8_t kMinReverbDecay = 0;
const uint8_t kMaxReverbDecay = 20;

const uint8_t kMinReverbPreDelay = 0;
const uint8_t kMaxReverbPreDelay = 19;

const uint8_t kMinReverbSurround = 0;
const uint8_t kMaxReverbSurround = 20;

const uint8_t kMinReverbDry = 0;
const uint8_t kMaxReverbDry = 20;

const uint8_t kMinReverbWet = 0;
const uint8_t kMaxReverbWet = 20;

const bool kInitialReverbActive = true;
const uint8_t kInitialReverbSize = 5;
const uint8_t kInitialReverbDecay = 10;
const uint8_t kInitialReverbPreDelay = 5;
const uint8_t kInitialReverbSurround = 5;
const uint8_t kInitialReverbDry = 20;
const uint8_t kInitialReverbWet = 10;

const float kInitialReverbCFeedback = 0.84;
const float kInitialReverbCDecay1 = 0.20;
const float kInitialReverbCDecay2 = 0.80;

struct ReverbSizeData {
    char nameShortR[5];
    float data;
};

const struct ReverbSizeData kReverbSizeDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct ReverbDecayData {
    char nameShortR[5];
    float data;
};

const struct ReverbDecayData kReverbDecayDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

struct ReverbPreDelayData {
    char nameShortR[5];
    float data;
};

const struct ReverbPreDelayData kReverbPreDelayDataLibrary[] = {
    {"  00", 0.000}, {"  01", 0.001}, {"  02", 0.002}, {"  03", 0.003}, {"  04", 0.004}, {"  05", 0.005}, {"  06", 0.006}, {"  07", 0.007}, {"  08", 0.008}, {"  09", 0.009}, {"  10", 0.010}, {"  20", 0.020}, {"  30", 0.030}, {"  40", 0.040}, {"  50", 0.050}, {"  60", 0.060}, {"  70", 0.070}, {"  80", 0.080}, {"  90", 0.090}, {" 100", 0.100}};

struct ReverbSurroundData {
    char nameShortR[5];
    float data;
};

const struct ReverbSurroundData kReverbSurroundDataLibrary[] = {
    {"  00", 0.000}, {"  05", 0.001}, {"  10", 0.002}, {"  15", 0.003}, {"  20", 0.004}, {"  25", 0.005}, {"  30", 0.006}, {"  35", 0.007}, {"  40", 0.008}, {"  45", 0.009}, {"  50", 0.010}, {"  55", 0.011}, {"  60", 0.012}, {"  65", 0.013}, {"  70", 0.014}, {"  75", 0.015}, {"  80", 0.016}, {"  85", 0.017}, {"  90", 0.018}, {"  95", 0.019}, {" 100", 0.020}};

struct ReverbMixData {
    char nameShortR[5];
    float data;
};

const struct ReverbMixData kReverbMixDataLibrary[] = {
    {"  00", 0.00}, {"  05", 0.05}, {"  10", 0.10}, {"  15", 0.15}, {"  20", 0.20}, {"  25", 0.25}, {"  30", 0.30}, {"  35", 0.35}, {"  40", 0.40}, {"  45", 0.45}, {"  50", 0.50}, {"  55", 0.55}, {"  60", 0.60}, {"  65", 0.65}, {"  70", 0.70}, {"  75", 0.75}, {"  80", 0.80}, {"  85", 0.85}, {"  90", 0.90}, {"  95", 0.95}, {" 100", 1.00}};

typedef enum {
    REV_ACTION_NONE = 0x00,
    REV_ACTION_UP = 0x01,
    REV_ACTION_DOWN = 0x02
} ReverbTransitionAction;

typedef enum {
    REV_MODE_NONE = 0x00,
    REV_MODE_ACTIVE = 0x01,
    REV_MODE_PREDELAY = 0x02,
    REV_MODE_SURROUND = 0x03
} ReverbTransitionMode;

typedef enum {
    REV_PHASE_NONE = 0x00,
    REV_PHASE_A = 0x01,
    REV_PHASE_B = 0x02
} ReverbTransitionPhase;

struct ReverbGenTransition {
    bool active;

    ReverbTransitionMode mode;
    ReverbTransitionPhase phase;

    bool activeActive;
    bool targetActive;

    uint8_t activeType;
    uint8_t targetType;

    float activeDry;
    float targetDry;
    ReverbTransitionAction actionDry;

    float activeWet;
    float targetWet;
    ReverbTransitionAction actionWet;
};

struct ReverbMixTransition {
    bool active;

    float targetDry;
    ReverbTransitionAction actionDry;

    float targetWet;
    ReverbTransitionAction actionWet;
};

struct Reverb {
    bool active;
    uint8_t size;
    uint8_t decay;
    uint8_t preDelay;
    uint8_t surround;
    uint8_t dry;
    uint8_t wet;

    float dryFloat;
    float wetFloat;

    ReverbGenTransition genTransition;
    ReverbMixTransition mixTransition;

    const bool limitMix = true;
    const uint8_t limitMixData = 30;

    uint16_t preDelay_lag;
    uint16_t preDelay_playInterval;
    uint16_t preDelay_recordInterval;

    const static uint16_t kPreDelayBufferSize = 4500;
    int32_t preDelayBuffer[kPreDelayBufferSize] = {0};

    uint16_t surround_lag;
    uint16_t surround_playInterval;
    uint16_t surround_recordInterval;

    const static uint16_t kSurroundBufferSize = 900;
    int32_t surroundBuffer[kSurroundBufferSize] = {0};

    const static uint16_t comb1Size = 1116;
    const static uint16_t comb2Size = 1188;
    const static uint16_t comb3Size = 1277;
    const static uint16_t comb4Size = 1356;
    const static uint16_t comb5Size = 1422;
    const static uint16_t comb6Size = 1491;
    const static uint16_t comb7Size = 1557;
    const static uint16_t comb8Size = 1617;

    uint16_t comb1Index = 0;
    uint16_t comb2Index = 0;
    uint16_t comb3Index = 0;
    uint16_t comb4Index = 0;
    uint16_t comb5Index = 0;
    uint16_t comb6Index = 0;
    uint16_t comb7Index = 0;
    uint16_t comb8Index = 0;

    int32_t comb1Filter = 0;
    int32_t comb2Filter = 0;
    int32_t comb3Filter = 0;
    int32_t comb4Filter = 0;
    int32_t comb5Filter = 0;
    int32_t comb6Filter = 0;
    int32_t comb7Filter = 0;
    int32_t comb8Filter = 0;

    int32_t comb1Out = 0;
    int32_t comb2Out = 0;
    int32_t comb3Out = 0;
    int32_t comb4Out = 0;
    int32_t comb5Out = 0;
    int32_t comb6Out = 0;
    int32_t comb7Out = 0;
    int32_t comb8Out = 0;

    int32_t comb1Buffer[comb1Size];
    int32_t comb2Buffer[comb2Size];
    int32_t comb3Buffer[comb3Size];
    int32_t comb4Buffer[comb4Size];
    int32_t comb5Buffer[comb5Size];
    int32_t comb6Buffer[comb6Size];
    int32_t comb7Buffer[comb7Size];
    int32_t comb8Buffer[comb8Size];

    float combFeedback; // 0.70 - 0.98
    float combDecay1;   // 0.00 - 0.40
    float combDecay2;   // 0.60 - 1.00

    const static uint16_t apass1Size = 225;
    const static uint16_t apass2Size = 556;
    const static uint16_t apass3Size = 441;
    const static uint16_t apass4Size = 341;

    uint16_t apass1Index = 0;
    uint16_t apass2Index = 0;
    uint16_t apass3Index = 0;
    uint16_t apass4Index = 0;

    int32_t apass1Out = 0;
    int32_t apass2Out = 0;
    int32_t apass3Out = 0;
    int32_t apass4Out = 0;

    int32_t apass1Buffer[apass1Size];
    int32_t apass2Buffer[apass2Size];
    int32_t apass3Buffer[apass3Size];
    int32_t apass4Buffer[apass4Size];

    float apassFeedback = 0.5;

    void initialize() {
        active = kInitialReverbActive;
        size = kInitialReverbSize;
        decay = kInitialReverbDecay;
        preDelay = kInitialReverbPreDelay;
        surround = kInitialReverbSurround;
        dry = kInitialReverbDry;
        wet = kInitialReverbWet;

        dryFloat = kReverbMixDataLibrary[dry].data;
        wetFloat = kReverbMixDataLibrary[wet].data;

        combFeedback = kInitialReverbCFeedback;
        combDecay1 = kInitialReverbCDecay1;
        combDecay2 = kInitialReverbCDecay2;

        cleanCombBuffer();
        cleanApassBuffer();
        cleanPreDelayBuffer();
        cleanWidthDelayBuffer();

        preDelay_lag = kAudioSampleRate * kReverbPreDelayDataLibrary[kInitialReverbPreDelay].data;
        preDelay_playInterval = 0;
        preDelay_recordInterval = preDelay_lag;

        surround_lag = kAudioSampleRate * kReverbSurroundDataLibrary[kInitialReverbSurround].data;
        surround_playInterval = 0;
        surround_recordInterval = surround_lag;
    }

    void cleanPreDelayBuffer() {
        memset(preDelayBuffer, 0x00, sizeof(preDelayBuffer));
    }

    void cleanWidthDelayBuffer() {
        memset(surroundBuffer, 0x00, sizeof(surroundBuffer));
    }

    void cleanCombBuffer() {
        memset(comb1Buffer, 0x00, sizeof(comb1Buffer));
        memset(comb2Buffer, 0x00, sizeof(comb2Buffer));
        memset(comb3Buffer, 0x00, sizeof(comb3Buffer));
        memset(comb4Buffer, 0x00, sizeof(comb4Buffer));
        memset(comb5Buffer, 0x00, sizeof(comb5Buffer));
        memset(comb6Buffer, 0x00, sizeof(comb6Buffer));
        memset(comb7Buffer, 0x00, sizeof(comb7Buffer));
        memset(comb8Buffer, 0x00, sizeof(comb8Buffer));
    }

    void cleanApassBuffer() {
        memset(apass1Buffer, 0x00, sizeof(apass1Buffer));
        memset(apass2Buffer, 0x00, sizeof(apass2Buffer));
        memset(apass3Buffer, 0x00, sizeof(apass3Buffer));
        memset(apass4Buffer, 0x00, sizeof(apass4Buffer));
    }

    void cleanMemory() {
        cleanPreDelayBuffer();
        cleanWidthDelayBuffer();
        cleanCombBuffer();
        cleanApassBuffer();
    }

    void setSize(float size_) {
        if ((size_ <= 1.0f) && (size >= 0.0f)) {
            combFeedback = (size_ * 0.28) + 0.70;
        }
    }

    void setDecay(float decay_) {
        if ((decay_ <= 1.0f) && (decay >= 0.0f)) {
            combDecay1 = decay_ * 0.40;
            combDecay2 = 1.0 - combDecay1;
        }
    }

    void setPreDelay(float preDelay_) {
        preDelay_lag = kAudioSampleRate * preDelay_;
        int16_t playInterval_ = preDelay_recordInterval - preDelay_lag;
        if (playInterval_ < 0)
            playInterval_ += kPreDelayBufferSize;
        preDelay_playInterval = playInterval_;
    }

    void setSurround(float surround_) {
        surround_lag = kAudioSampleRate * surround_;
        int16_t playInterval_ = surround_recordInterval - surround_lag;
        if (playInterval_ < 0)
            playInterval_ += kSurroundBufferSize;
        surround_playInterval = playInterval_;
    }
};

////////////////////////////////////////////////////////////////////////////////
/* Layer Constants -----------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

const uint8_t kMinLayerVolume = 0;
const uint8_t kMaxLayerVolume = 100;

const uint8_t kMinLayerSpeed = 0;
const uint8_t kMaxLayerSpeed = 6;

const uint8_t kMinLayerFill = 0;
const uint8_t kMaxLayerFill = 27;

const uint8_t kInitialLayerVolume = 75;
const uint8_t kInitialLayerSpeed = 2;
const bool kInitialLayerReverse = false;
const bool kInitialLayerNormalize = true;

const uint8_t kLayerLibrarySize = 10;
const uint16_t kBeatLibrarySize = 64;
const uint16_t kBeatMicroLibrarySize = 12;

const bool kInitialLayerMute = false;
const bool kInitialLayerFill = true;
const bool kInitialLayerStyle = true;

const float kLayerVolumeCoef = 0.80;

struct LayerSpeedData {
    char nameShort[5];
    float increment;
};

const LayerSpeedData kLayerSpeedDataLibrary[] = {
    {"0.50", 0.50}, {"0.75", 0.75}, {"1.00", 1.00}, {"1.25", 1.25}, {"1.50", 1.50}, {"1.75", 1.75}, {"2.00", 2.00}};

/*----------------------------------------------------------------------------*/

const uint8_t instFileNameSize = 45;
const uint8_t sampleFileNameSize = 75;

const char kBlankInstData[] = "-                                             ";
const char kBlankSampleData[] = "-                                                                           ";

#define CH_NA 0x00
#define CH_MONO 0x01
#define CH_STEREO 0x02

#define BD_NA 0x00
#define BD_08 0x01
#define BD_16 0x02
#define BD_24 0x03
#define BD_32 0x04

#define FR_NA 0x00
#define FR_008kHz 0x01
#define FR_011kHz 0x02
#define FR_016kHz 0x03
#define FR_022kHz 0x04
#define FR_024kHz 0x05
#define FR_044kHz 0x06
#define FR_048kHz 0x07
#define FR_088kHz 0x08
#define FR_096kHz 0x09
#define FR_176kHz 0x0A
#define FR_192kHz 0x0B

struct ChannelData {
    char name[2];
    uint8_t channel;
};

const struct ChannelData kChannelDataLibrary[] = {
    {"X", 0x00},
    {"M", 0x01},
    {"S", 0x02}};

struct BitdepthData {
    char name[6];
    uint8_t bitdepth;
};

const struct BitdepthData kBitdepthDataLibrary[] = {
    {"     ", 0x00},
    {"08BIT", 0x08},
    {"16BIT", 0x10},
    {"24BIT", 0x18},
    {"32BIT", 0x20}};

struct FrequencyData {
    char name[7];
    uint32_t frequency;
};

const struct FrequencyData kFrequencyDataLibrary[] = {
    {"      ", 0},
    {"  8kHZ", 8000},
    {" 11kHz", 11025},
    {" 16kHz", 16000},
    {" 22kHz", 22050},
    {" 24kHz", 24000},
    {" 44kHz", 44100},
    {" 48kHz", 48000},
    {" 88kHz", 88200},
    {" 96kHz", 96000},
    {"176kHz", 176000},
    {"192kHz", 192000}};

struct InstData {
    char num[4 + 1] = "----";
    char fileName[kFileNameSize + 10] = ""; // Data/InstName.ins
    char folderName[kFileNameSize + 10] = "";
    char nameLong[kFileNameSize + 1] = "";
    char nameShortL[10 + 1] = "--------  ";
    char nameShortR[10 + 1] = "  --------";
};

struct SampleData {
    char num[4 + 1] = "----";
    char fileName[(kFileNameSize * 2) + 10] = ""; // Sample/InstName/SampleName.wav
    char nameLong[kFileNameSize + 1] = "";
    char nameShortR[10 + 1] = "  --------";

    uint8_t channel = CH_NA;
    uint8_t bitdepth = BD_NA;
    uint8_t frequency = FR_NA;

    float coefSampleSize;
    uint32_t sampleByteSize;
    uint32_t sampleSize;
};

struct SampleSector {
    uint32_t size;
    float normMultiplier = 1.0f;
};

struct riff_chunk {
    uint32_t chunkId;    // 00 --> 0x46464952
    uint32_t chunkSize;  // 04 --> --
    uint32_t fileFormat; // 08 --> 0x45564157
};

struct fmt_chunk {
    uint32_t chunkId;      // 00 --> 0x20746D66
    uint32_t chunkSize;    // 04 --> 16 | 18 | 40
    uint16_t audioFormat;  // 08 --> 0x01 | 0x03
    uint16_t nbrChannels;  // 10 --> 1 | 2
    uint32_t sampleRate;   // 12
    uint32_t byteRate;     // 16
    uint16_t blockAlign;   // 20
    uint16_t bitPerSample; // 22

    uint32_t chunkStartByte;
};

struct data_chunk {
    uint32_t chunkId;   // 00 --> 0x61746164
    uint32_t chunkSize; // 04 --> --

    uint32_t chunkStartByte;
};

struct WavData {
    struct riff_chunk riff_chunk;
    struct fmt_chunk fmt_chunk;
    struct data_chunk data_chunk;
};

/*----------------------------------------------------------------------------*/

struct FillData {
    char nameLong[11];
    uint8_t step;
    float timeLibrary[16];
    uint8_t volumeLibrary[16];
};

const struct FillData kFillDataLibrary[] = {
    {"     SOLID", 1, {0.000}, {100}},
    {"      FLAT", 1, {0.000}, {50}},
    {"     MUTED", 1, {0.000}, {0}},
    {"     SWING", 3, {0.000, 0.500, 0.875}, {100, 75, 50}},

    {"   SOLID_A", 4, {0.000, 0.250, 0.500, 0.750}, {100, 80, 100, 80}},
    {"   SOLID_B", 6, {0.000, 0.166, 0.333, 0.500, 0.666, 0.833}, {100, 80, 80, 100, 80, 80}},
    {"   SOLID_C", 8, {0.000, 0.125, 0.250, 0.375, 0.500, 0.625, 0.750, 0.875}, {100, 80, 100, 80, 100, 80, 100, 80}},

    {"    LINK_A", 5, {0.000, 0.500, 0.625, 0.750, 0.875}, {100, 40, 60, 80, 100}},
    {"    LINK_B", 7, {0.000, 0.500, 0.583, 0.666, 0.750, 0.833, 0.916}, {100, 25, 40, 55, 70, 85, 100}},
    {"    LINK_C", 9, {0.000, 0.500, 0.562, 0.625, 0.687, 0.750, 0.812, 0.875, 0.937}, {100, 30, 40, 50, 60, 70, 80, 90, 100}},

    {"    RISE_A", 4, {0.000, 0.250, 0.500, 0.750}, {40, 60, 80, 100}},
    {"    RISE_B", 6, {0.000, 0.166, 0.333, 0.500, 0.666, 0.833}, {25, 40, 55, 70, 85, 100}},
    {"    RISE_C", 8, {0.000, 0.125, 0.250, 0.375, 0.500, 0.625, 0.750, 0.875}, {30, 40, 50, 60, 70, 80, 90, 100}},

    {"    FALL_A", 4, {0.000, 0.250, 0.500, 0.750}, {100, 80, 60, 40}},
    {"    FALL_B", 6, {0.000, 0.166, 0.333, 0.500, 0.666, 0.833}, {100, 85, 70, 55, 40, 25}},
    {"    FALL_C", 8, {0.000, 0.125, 0.250, 0.375, 0.500, 0.625, 0.750, 0.875}, {100, 90, 80, 70, 60, 50, 40, 30}},

    {"   WAVE_1A", 9, {0.000, 0.166, 0.333, 0.500, 0.583, 0.666, 0.750, 0.833, 0.916}, {40, 60, 80, 100, 90, 80, 70, 60, 50}},
    {"   WAVE_1B", 10, {0.000, 0.250, 0.500, 0.562, 0.625, 0.687, 0.750, 0.812, 0.875, 0.937}, {50, 75, 100, 90, 80, 70, 60, 50, 40, 30}},
    {"   WAVE_1C", 12, {0.000, 0.125, 0.250, 0.375, 0.500, 0.562, 0.625, 0.687, 0.750, 0.812, 0.875, 0.937}, {40, 55, 70, 85, 100, 90, 80, 70, 60, 50, 40, 30}},

    {"   WAVE_2A", 9, {0.000, 0.083, 0.166, 0.250, 0.333, 0.416, 0.500, 0.666, 0.833}, {40, 50, 60, 70, 80, 90, 100, 75, 50}},
    {"   WAVE_2B", 10, {0.000, 0.062, 0.125, 0.187, 0.250, 0.312, 0.375, 0.437, 0.500, 0.750}, {20, 30, 40, 50, 60, 70, 80, 90, 100, 75}},
    {"   WAVE_2C", 12, {0.000, 0.062, 0.125, 0.187, 0.250, 0.312, 0.375, 0.437, 0.500, 0.625, 0.750, 0.875}, {20, 30, 40, 50, 60, 70, 80, 90, 100, 80, 60, 40}},

    {"   WAVE_3A", 9, {0.000, 0.083, 0.166, 0.250, 0.333, 0.416, 0.500, 0.666, 0.833}, {100, 90, 80, 70, 60, 50, 40, 60, 80}},
    {"   WAVE_3B", 10, {0.000, 0.062, 0.125, 0.187, 0.250, 0.312, 0.375, 0.437, 0.500, 0.750}, {100, 90, 80, 70, 60, 50, 40, 30, 20, 60}},
    {"   WAVE_3C", 12, {0.000, 0.062, 0.125, 0.187, 0.250, 0.312, 0.375, 0.437, 0.500, 0.625, 0.750, 0.875}, {100, 90, 80, 70, 60, 50, 40, 30, 20, 40, 60, 80}},

    {"   WAVE_4A", 9, {0.000, 0.166, 0.333, 0.500, 0.583, 0.666, 0.750, 0.833, 0.916}, {100, 80, 60, 40, 50, 60, 70, 80, 90}},
    {"   WAVE_4B", 10, {0.000, 0.250, 0.500, 0.562, 0.625, 0.687, 0.750, 0.812, 0.875, 0.937}, {100, 60, 20, 30, 40, 50, 60, 70, 80, 90}},
    {"   WAVE_4C", 12, {0.000, 0.125, 0.250, 0.375, 0.500, 0.562, 0.625, 0.687, 0.750, 0.812, 0.875, 0.937}, {100, 80, 60, 40, 20, 30, 40, 50, 60, 70, 80, 90}}};

/*----------------------------------------------------------------------------*/

struct BeatMicro {
    uint8_t data = 0x00; // 0 active, 0000000 volume
    uint16_t interval = 0x00;

    BeatMicro() {}
    ~BeatMicro() {}

    void setInterval(uint16_t interval_) { interval = interval_; }
    void setActive(bool active) {
        data &= 0b01111111;
        data += (active << 7);
    }
    void setVolume(uint8_t volume) {
        data &= 0b10000000;
        data += volume;
    }
    void set(uint16_t interval_, uint8_t volume_) {
        setActive(true);
        setVolume(volume_);
        interval = interval_;
    }
    void reset() {
        data = 0;
        interval = 0;
    }
    bool getActive() const { return (data >> 7); }
    uint8_t getVolume() const { return (data & 0b01111111); }
    uint16_t getInterval() const { return interval; }
};

/*----------------------------------------------------------------------------*/

struct Beat {
    uint8_t data = 0x00; // 0 active, 0000000 fill
    uint16_t startInterval = 0x00;
    uint16_t endInterval = 0x00;
    BeatMicro beatMicroLibrary[kBeatMicroLibrarySize];

    Beat() {}
    ~Beat() {}

    void set(uint16_t startInterval_, uint16_t endInterval_, uint8_t fill_) {
        setActive(true);
        setStartInterval(startInterval_);
        setEndInterval(endInterval_);
        setFill(fill_);
    }
    void reset() {
        setActive(false);
        setStartInterval(0);
        setEndInterval(0);
        resetFill();
    }
    void setData(uint8_t data_) { data = data_; }
    uint8_t getData() const { return data; }
    void setActive(bool active_) {
        data &= 0b01111111;
        data += (active_ << 7);
    }
    bool getActive() const { return (data >> 7); }
    void setFill(uint8_t fill) {
        resetFill();
        data += fill;
        uint8_t step = kFillDataLibrary[fill].step;
        uint16_t totalInterval = endInterval - startInterval;
        for (uint8_t i = 0; i < step; i++) {
            uint16_t beatMicroInterval = startInterval + (uint16_t)(totalInterval * kFillDataLibrary[fill].timeLibrary[i]);
            uint8_t beatMicroVolume = kFillDataLibrary[fill].volumeLibrary[i];
            beatMicroLibrary[i].set(beatMicroInterval, beatMicroVolume);
        }
    }
    uint8_t getFill() const { return (data & 0b01111111); }
    void resetFill() {
        data &= 0b10000000;
        for (uint8_t i = 0; i < kBeatMicroLibrarySize; i++) {
            beatMicroLibrary[i].reset();
        }
    }
    void setStartInterval(uint16_t startInterval_) { startInterval = startInterval_; }
    uint16_t getStartInterval() const { return startInterval; }
    void setEndInterval(uint16_t endInterval_) { endInterval = endInterval_; }
    uint16_t getEndInterval() const { return endInterval; }
};

/*----------------------------------------------------------------------------*/

const uint8_t kBankLibrarySize = 5;
const uint8_t kInitialBank = 0;

struct Bank {
    Beat beatLibrary[kBeatLibrarySize];

    int8_t lastActiveBeatNum = -1;
    int8_t playBeatNum = -1;
    int8_t playBeatMicroNum = -1;
};

/*----------------------------------------------------------------------------*/

struct Layer {
    uint8_t number;

    bool selected = false;
    int16_t instSelected = -1;
    int16_t instLoaded = -1;
    int16_t sampleSelected = -1;
    int16_t sampleLoaded = -1;
    uint8_t volume = kInitialLayerVolume;
    uint8_t speed = kInitialLayerSpeed;
    bool reverse = kInitialLayerReverse;
    bool normalize = kInitialLayerNormalize;

    bool mute = kInitialLayerMute;
    bool muteFlag = false;
    uint16_t muteInterval = 0;
    bool muteAction = false;

    bool fill = kInitialLayerFill;
    bool fillFlag = false;
    uint16_t fillInterval = 0;
    bool fillAction = false;

    bool style = kInitialLayerStyle;
    bool styleFlag = false;
    uint16_t styleInterval = 0;
    bool styleAction = false;

    Bank bankLibrary[kBankLibrarySize];

    bool sampleSelectedReadError = false;
    bool sampleSelectedTypeError = false;

    struct InstData instSelectedData;
    struct SampleData sampleSelectedData;
    struct WavData wavSelectedData;

    struct InstData instLoadedData;
    struct SampleData sampleLoadedData;
    struct WavData wavLoadedData;

    uint8_t playSampleSector = 0;
    uint8_t writeSampleSector = 0;

    SampleSector sampleSector[2];

    bool samplePlay = false;

    Layer(uint8_t number_) : number(number_) {}
    ~Layer() {}
};

////////////////////////////////////////////////////////////////////////////////
/* PlayData Constants --------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////

struct MetroPlayData {
    bool active;
    uint32_t counter;
    uint32_t counterMax;
    uint32_t ramAddress;
    float volumeMultiplier;
};

struct BeatPlayData {
    bool active;
    float counter;
    int32_t counterMax;
    uint32_t ramAddress;
    float volumeMultiplier;
    uint8_t speed;
    bool reverse;
    bool normalize;

    float increment;
};

struct LayerPlayData {
    uint8_t beatCount;
    BeatPlayData beatData;
};

struct SongPlayData {
    LayerPlayData layerData[10];
};

const uint8_t wavIntro[44] = {0x52, 0x49, 0x46, 0x46, 0xAC, 0x58, 0x01, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x88, 0x58, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x10, 0xB1, 0x02, 0x00};

#endif
