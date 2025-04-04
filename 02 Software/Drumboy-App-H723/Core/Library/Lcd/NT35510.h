#ifndef __NT35510_H
#define __NT35510_H

#include "main.h"

/* NT35510 Commands ----------------------------------------------------------*/

#define  LCD_NOP                          0x0000      //      C         0
#define  LCD_SOFT_RESET                   0x0100      //      C         0
#define  LCD_GET_ID                       0x0400      //      R         3
#define  LCD_GET_ERRORS                   0x0500      //      R         1
#define  LCD_GET_POWER_MODE               0x0A00      //      R         1
#define  LCD_GET_MADCTL                   0x0B00      //      R         1
#define  LCD_GET_PIXEL_FORMAT             0x0C00      //      R         1
#define  LCD_GET_IMAGE_MODE               0x0D00      //      R         1
#define  LCD_GET_SIGNAL_MODE              0x0E00      //      R         1
#define  LCD_GET_SELF_DIAGNOSTIC          0x0F00      //      R         1
#define  LCD_ENTER_SLEEP_MODE             0x1000      //      C         0
#define  LCD_EXIT_SLEEP_MODE              0x1100      //      C         0
#define  LCD_ENTER_PARTIAL_MODE           0x1200      //      C         0
#define  LCD_ENTER_NORMAL_MODE            0x1300      //      C         0
#define  LCD_EXIT_INVERT_MODE             0x2000      //      C         0
#define  LCD_ENTER_INVERT_MODE            0x2100      //      C         0
#define  LCD_ALL_PIXEL_OFF                0x2200      //      C         0
#define  LCD_ALL_PIXEL_ON                 0x2300      //      C         0
#define  LCD_SET_GAMMA_CURVE              0x2600      //      W         1
#define  LCD_SET_DISPLAY_OFF              0x2800      //      C         0
#define  LCD_SET_DISPLAY_ON               0x2900      //      C         0
#define  LCD_SET_COLUMN_ADDRESS_0         0x2A00      //      W         1
#define  LCD_SET_COLUMN_ADDRESS_1         0x2A01      //      W         1
#define  LCD_SET_COLUMN_ADDRESS_2         0x2A02      //      W         1
#define  LCD_SET_COLUMN_ADDRESS_3         0x2A03      //      W         1
#define  LCD_SET_ROW_ADDRESS_0            0x2B00      //      W         1
#define  LCD_SET_ROW_ADDRESS_1            0x2B01      //      W         1
#define  LCD_SET_ROW_ADDRESS_2            0x2B02      //      W         1
#define  LCD_SET_ROW_ADDRESS_3            0x2B03      //      W         1
#define  LCD_WRITE_MEMORY_START           0x2C00      //      W         var
#define  LCD_READ_MEMORY_START            0x2E00      //      R         var
#define  LCD_SET_PARTIAL_AREA             0x3000      //      W         4
#define  LCD_SET_TEAR_OFF                 0x3400      //      C         0
#define  LCD_SET_TEAR_ON                  0x3500      //      C         0
#define  LCD_SET_ADDRESS_MODE             0x3600      //      W         1
#define  LCD_EXIT_IDLE_MODE               0x3800      //      C         0
#define  LCD_ENTER_IDLE_MODE              0x3900      //      C         0
#define  LCD_SET_PIXEL_FORMAT             0x3A00      //      W         1
#define  LCD_WRITE_MEMORY_CONTINUE        0x3C00      //      W         var
#define  LCD_READ_MEMORY_CONTINUE         0x3E00      //      R         var
#define  LCD_SET_TEAR_SCANLINE            0x4400      //      W         2
#define  LCD_GET_TEAR_SCANLINE            0x4500      //      R         2
#define  LCD_DEEP_STANDBY_MODE_ON         0x4F00      //      C         0
#define  LCD_SET_DISPLAY_FUNCTION_CTRL    0xB600      //      W         3
#define  LCD_SET_POWER_CTRL_1             0xC000      //      W         2
#define  LCD_SET_POWER_CTRL_2             0xC100      //      W         2
#define  LCD_SET_POWER_CTRL_3             0xC200      //      W         1
#define  LCD_SET_POWER_CTRL_4             0xC300      //      W         1
#define  LCD_SET_POWER_CTRL_5             0xC400      //      W         1
#define  LCD_SET_VCOM_CTRL_1              0xC500      //      W         4
#define  LCD_SET_POSITIVE_GAMMA_CTRL      0xE000      //      W         15
#define  LCD_SET_NEGATIVE_GAMMA_CTRL      0xE100      //      W         15
#define  LCD_SET_DIGITAL_GAMMA_CTRL_1     0xE200      //      W         3
#define  LCD_SET_DIGITAL_GAMMA_CTRL_2     0xE300      //      W         3

#endif
