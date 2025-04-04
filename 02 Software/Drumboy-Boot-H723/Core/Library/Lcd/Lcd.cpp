#include "Lcd.h"

Lcd::Lcd(LcdRotation rotation_, RGB16Color foreColor_, RGB16Color backColor_, LcdFont font_)
    : rotation(rotation_),
      foreColor(foreColor_),
      backColor(backColor_),
      fontAlignment(LEFT) {
    switch (rotation) {
        case 0:
            width = kLCD_WIDTH;
            height = kLCD_HEIGHT;
            break;
        case 1:
            width = kLCD_HEIGHT;
            height = kLCD_WIDTH;
            break;
        case 2:
            width = kLCD_WIDTH;
            height = kLCD_HEIGHT;
            break;
        case 3:
            width = kLCD_HEIGHT;
            height = kLCD_WIDTH;
            break;
    }
    setFont(font_);
}

Lcd::~Lcd() {}

/* Private functions ---------------------------------------------------------*/

void Lcd::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(LCD_SET_COLUMN_ADDRESS_0);
    writeData(x0 >> 8);
    writeCommand(LCD_SET_COLUMN_ADDRESS_1);
    writeData(x0 & 0xFF);
    writeCommand(LCD_SET_COLUMN_ADDRESS_2);
    writeData(x1 >> 8);
    writeCommand(LCD_SET_COLUMN_ADDRESS_3);
    writeData(x1 & 0xFF);

    writeCommand(LCD_SET_ROW_ADDRESS_0);
    writeData(y0 >> 8);
    writeCommand(LCD_SET_ROW_ADDRESS_1);
    writeData(y0 & 0xFF);
    writeCommand(LCD_SET_ROW_ADDRESS_2);
    writeData(y1 >> 8);
    writeCommand(LCD_SET_ROW_ADDRESS_3);
    writeData(y1 & 0xFF);

    writeCommand(LCD_WRITE_MEMORY_START);
}

void Lcd::clearAddressWindow() {
    setAddressWindow(0, 0, width - 1, height - 1);
}

void Lcd::fastFill(RGB16Color color, uint32_t pixel) {
    LCD_DATA_GPIO_Port->ODR = color;

    uint32_t blocks;
    blocks = pixel / 16;

    for (uint32_t i = 0; i < blocks; i++) {
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }

    if ((pixel % 16) != 0) {
        for (uint8_t i = 0; i < (pixel % 16) + 1; i++) {
            LCD_WR_LOW;
            LCD_WR_HIGH;
        }
    }
}

uint8_t Lcd::getLetter(char letter) {
    switch (letter) {
        case '0':
            return 0;
            break;
        case '1':
            return 1;
            break;
        case '2':
            return 2;
            break;
        case '3':
            return 3;
            break;
        case '4':
            return 4;
            break;
        case '5':
            return 5;
            break;
        case '6':
            return 6;
            break;
        case '7':
            return 7;
            break;
        case '8':
            return 8;
            break;
        case '9':
            return 9;
            break;
        case 'A':
        case 'a':
            return 10;
            break;
        case 'B':
        case 'b':
            return 11;
            break;
        case 'C':
        case 'c':
            return 12;
            break;
        case 'D':
        case 'd':
            return 13;
            break;
        case 'E':
        case 'e':
            return 14;
            break;
        case 'F':
        case 'f':
            return 15;
            break;
        case 'G':
        case 'g':
            return 16;
            break;
        case 'H':
        case 'h':
            return 17;
            break;
        case 'I':
        case 'i':
            return 18;
            break;
        case 'J':
        case 'j':
            return 19;
            break;
        case 'K':
        case 'k':
            return 20;
            break;
        case 'L':
        case 'l':
            return 21;
            break;
        case 'M':
        case 'm':
            return 22;
            break;
        case 'N':
        case 'n':
            return 23;
            break;
        case 'O':
        case 'o':
            return 24;
            break;
        case 'P':
        case 'p':
            return 25;
            break;
        case 'Q':
        case 'q':
            return 26;
            break;
        case 'R':
        case 'r':
            return 27;
            break;
        case 'S':
        case 's':
            return 28;
            break;
        case 'T':
        case 't':
            return 29;
            break;
        case 'U':
        case 'u':
            return 30;
            break;
        case 'V':
        case 'v':
            return 31;
            break;
        case 'W':
        case 'w':
            return 32;
            break;
        case 'X':
        case 'x':
            return 33;
            break;
        case 'Y':
        case 'y':
            return 34;
            break;
        case 'Z':
        case 'z':
            return 35;
            break;
        case '/':
            return 36;
            break;
        case '-':
            return 37;
            break;
        case '+':
            return 38;
            break;
        case '%':
            return 39;
            break;
        case '.':
            return 40;
            break;
        case ',':
            return 41;
            break;
        case ':':
            return 42;
            break;
        case '<':
            return 43;
            break;
        case '>':
            return 44;
            break;
        case '[':
            return 45;
            break;
        case ']':
            return 46;
            break;
        case '_':
            return 47;
            break;
        case '|':
            return 48;
            break;
        case '?':
            return 49;
            break;
        case '#':
            return 50;
            break;
        case '!':
            return 51;
            break;
        case ' ':
            return 52;
            break;
        default:
            return 52;
            break;
    }
}

RGB16Color Lcd::alphaColor(RGB16Color color_, float alpha) {
    uint16_t red = (uint16_t)((color_ >> 11) * alpha);
    uint16_t green = (uint16_t)(((color_ >> 5) & 0b00111111) * alpha);
    uint16_t blue = (uint16_t)((color_ & 0b00011111) * alpha);
    RGB16Color color = (red << 11) | (green << 5) | (blue);
    return color;
}

RGB16Color Lcd::alphaColor(struct RGB24RawColor color_, float alpha) {
    uint16_t red = (uint16_t)(color_.red * alpha) >> 3;
    uint16_t green = (uint16_t)(color_.green * alpha) >> 2;
    uint16_t blue = (uint16_t)(color_.blue * alpha) >> 3;
    RGB16Color color = (red << 11) | (green << 5) | (blue);
    return color;
}

RGB16Color Lcd::convertColor(struct RGB24RawColor color_) {
    uint16_t red = color_.red >> 3;
    uint16_t green = color_.green >> 2;
    uint16_t blue = color_.blue >> 3;
    RGB16Color color = (red << 11) | (green << 5) | (blue);
    return color;
}

/* Public functions ----------------------------------------------------------*/

void Lcd::setRotation(LcdRotation rotation_) {
    rotation = rotation_;
    writeCommand(LCD_SET_ADDRESS_MODE);
    switch (rotation) {
        case PORTRAIT_0:
            writeData(0x00);
            width = kLCD_WIDTH;
            height = kLCD_HEIGHT;
            break;
        case LANDSCAPE_0:
            writeData(MADCTL_MV | MADCTL_MX);
            width = kLCD_HEIGHT;
            height = kLCD_WIDTH;
            break;
        case PORTRAIT_1:
            writeData(MADCTL_MX | MADCTL_MY);
            width = kLCD_WIDTH;
            height = kLCD_HEIGHT;
            break;
        case LANDSCAPE_1:
            writeData(MADCTL_MV | MADCTL_MY);
            width = kLCD_HEIGHT;
            height = kLCD_WIDTH;
            break;
    }
}

void Lcd::setFont(LcdFont font_) {
    font = font_;
    switch (font) {
        case FONT_05x07:
            fontData = &kFontData_05x07[0][0];
            fontWidth = kFontWidth_05x07;
            fontHeight = kFontHeight_05x07;
            fontSpacing = kFontSpacing_05x07;
            break;
        case FONT_07x09:
            fontData = &kFontData_07x09[0][0];
            fontWidth = kFontWidth_07x09;
            fontHeight = kFontHeight_07x09;
            fontSpacing = kFontSpacing_07x09;
            break;
        case FONT_10x14:
            fontData = &kFontData_10x14[0][0];
            fontWidth = kFontWidth_10x14;
            fontHeight = kFontHeight_10x14;
            fontSpacing = kFontSpacing_10x14;
            break;
        case FONT_14x18:
            fontData = &kFontData_14x18[0][0];
            fontWidth = kFontWidth_14x18;
            fontHeight = kFontHeight_14x18;
            fontSpacing = kFontSpacing_14x18;
            break;
    }
}

void Lcd::initialize() {
    LCD_BL_LOW;
    LCD_RD_HIGH;
    LCD_WR_HIGH;
    LCD_CS_LOW;

    HAL_Delay(250);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(120);

    writeCommand(0xFF00);
    writeData(0xAA);
    writeCommand(0xFF01);
    writeData(0x55);
    writeCommand(0xFF02);
    writeData(0x25);
    writeCommand(0xFF03);
    writeData(0x01);

    writeCommand(0xF40F);
    writeData(0x03);
    writeCommand(0xF410);
    writeData(0x21);
    writeCommand(0xF411);
    writeData(0x43);
    writeCommand(0xF412);
    writeData(0x10);
    writeCommand(0xF413);
    writeData(0x34);
    writeCommand(0xF414);
    writeData(0x52);

    /// #LV2 Page 1 enable
    writeCommand(0xF000);
    writeData(0x55);
    writeCommand(0xF001);
    writeData(0xAA);
    writeCommand(0xF002);
    writeData(0x52);
    writeCommand(0xF003);
    writeData(0x08);
    writeCommand(0xF004);
    writeData(0x01);

    /// #AVDD Set AVDD 5.2V
    writeCommand(0xB000);
    writeData(0x0D);
    writeCommand(0xB001);
    writeData(0x0D);
    writeCommand(0xB002);
    writeData(0x0D);

    // #AVDD ratio
    writeCommand(0xB600);
    writeData(0x44);
    writeCommand(0xB601);
    writeData(0x44);
    writeCommand(0xB602);
    writeData(0x44);

    // #AVEE -5.2V
    writeCommand(0xB100);
    writeData(0x0D);
    writeCommand(0xB101);
    writeData(0x0D);
    writeCommand(0xB102);
    writeData(0x0D);

    // #AVEE ratio
    writeCommand(0xB700);
    writeData(0x34);
    writeCommand(0xB701);
    writeData(0x34);
    writeCommand(0xB702);
    writeData(0x34);

    // #VCL  -2.5V
    writeCommand(0xB200);
    writeData(0x00);
    writeCommand(0xB201);
    writeData(0x00);
    writeCommand(0xB202);
    writeData(0x00);

    // #VCL ratio
    writeCommand(0xB800);
    writeData(0x34);
    writeCommand(0xB801);
    writeData(0x34);
    writeCommand(0xB802);
    writeData(0x34);

    // #VGH 15V  (Free pump)
    writeCommand(0xBF00);
    writeData(0x01);
    writeCommand(0xB300);
    writeData(0x0f);
    writeCommand(0xB301);
    writeData(0x0f);
    writeCommand(0xB302);
    writeData(0x0f);

    // #VGH ratio
    writeCommand(0xB900);
    writeData(0x34);
    writeCommand(0xB901);
    writeData(0x34);
    writeCommand(0xB902);
    writeData(0x34);

    // #VGL_REG -10V
    writeCommand(0xB500);
    writeData(0x08);
    writeCommand(0xB501);
    writeData(0x08);
    writeCommand(0xB502);
    writeData(0x08);

    writeCommand(0xC200);
    writeData(0x03);

    // #VGLX ratio
    writeCommand(0xBA00);
    writeData(0x24);
    writeCommand(0xBA01);
    writeData(0x24);
    writeCommand(0xBA02);
    writeData(0x24);

    // #VGMP/VGSP 4.5V/0V
    writeCommand(0xBC00);
    writeData(0x00);
    writeCommand(0xBC01);
    writeData(0x78);
    writeCommand(0xBC02);
    writeData(0x00);

    // #VGMN/VGSN -4.5V/0V
    writeCommand(0xBD00);
    writeData(0x00);
    writeCommand(0xBD01);
    writeData(0x78);
    writeCommand(0xBD02);
    writeData(0x00);

    // #VCOM  -1.575V
    writeCommand(0xBE00);
    writeData(0x00);
    writeCommand(0xBE01);
    writeData(0x6F);  /// 0x66

    // #Gamma Setting
    writeCommand(0xD100);
    writeData(0x00);
    writeCommand(0xD101);
    writeData(0x08);
    writeCommand(0xD102);
    writeData(0x00);
    writeCommand(0xD103);
    writeData(0x09);
    writeCommand(0xD104);
    writeData(0x00);
    writeCommand(0xD105);
    writeData(0x11);
    writeCommand(0xD106);
    writeData(0x00);
    writeCommand(0xD107);
    writeData(0x24);
    writeCommand(0xD108);
    writeData(0x00);
    writeCommand(0xD109);
    writeData(0x3A);
    writeCommand(0xD10A);
    writeData(0x00);
    writeCommand(0xD10B);
    writeData(0x69);
    writeCommand(0xD10C);
    writeData(0x00);
    writeCommand(0xD10D);
    writeData(0x95);
    writeCommand(0xD10E);
    writeData(0x00);
    writeCommand(0xD10F);
    writeData(0xDB);
    writeCommand(0xD110);
    writeData(0x01);
    writeCommand(0xD111);
    writeData(0x0E);
    writeCommand(0xD112);
    writeData(0x01);
    writeCommand(0xD113);
    writeData(0x54);
    writeCommand(0xD114);
    writeData(0x01);
    writeCommand(0xD115);
    writeData(0x85);
    writeCommand(0xD116);
    writeData(0x01);
    writeCommand(0xD117);
    writeData(0xC8);
    writeCommand(0xD118);
    writeData(0x01);
    writeCommand(0xD119);
    writeData(0xFC);
    writeCommand(0xD11A);
    writeData(0x01);
    writeCommand(0xD11B);
    writeData(0xFD);
    writeCommand(0xD11C);
    writeData(0x02);
    writeCommand(0xD11D);
    writeData(0x29);
    writeCommand(0xD11E);
    writeData(0x02);
    writeCommand(0xD11F);
    writeData(0x56);
    writeCommand(0xD120);
    writeData(0x02);
    writeCommand(0xD121);
    writeData(0x6F);
    writeCommand(0xD122);
    writeData(0x02);
    writeCommand(0xD123);
    writeData(0x8C);
    writeCommand(0xD124);
    writeData(0x02);
    writeCommand(0xD125);
    writeData(0x9E);
    writeCommand(0xD126);
    writeData(0x02);
    writeCommand(0xD127);
    writeData(0xB4);
    writeCommand(0xD128);
    writeData(0x02);
    writeCommand(0xD129);
    writeData(0xC2);
    writeCommand(0xD12A);
    writeData(0x02);
    writeCommand(0xD12B);
    writeData(0xD6);
    writeCommand(0xD12C);
    writeData(0x02);
    writeCommand(0xD12D);
    writeData(0xE4);
    writeCommand(0xD12E);
    writeData(0x02);
    writeCommand(0xD12F);
    writeData(0xF9);
    writeCommand(0xD130);
    writeData(0x03);
    writeCommand(0xD131);
    writeData(0x25);
    writeCommand(0xD132);
    writeData(0x03);
    writeCommand(0xD133);
    writeData(0x8E);

    writeCommand(0xD200);
    writeData(0x00);
    writeCommand(0xD201);
    writeData(0x08);
    writeCommand(0xD202);
    writeData(0x00);
    writeCommand(0xD203);
    writeData(0x09);
    writeCommand(0xD204);
    writeData(0x00);
    writeCommand(0xD205);
    writeData(0x11);
    writeCommand(0xD206);
    writeData(0x00);
    writeCommand(0xD207);
    writeData(0x24);
    writeCommand(0xD208);
    writeData(0x00);
    writeCommand(0xD209);
    writeData(0x3A);
    writeCommand(0xD20A);
    writeData(0x00);
    writeCommand(0xD20B);
    writeData(0x69);
    writeCommand(0xD20C);
    writeData(0x00);
    writeCommand(0xD20D);
    writeData(0x95);
    writeCommand(0xD20E);
    writeData(0x00);
    writeCommand(0xD20F);
    writeData(0xDB);
    writeCommand(0xD210);
    writeData(0x01);
    writeCommand(0xD211);
    writeData(0x0E);
    writeCommand(0xD212);
    writeData(0x01);
    writeCommand(0xD213);
    writeData(0x54);
    writeCommand(0xD214);
    writeData(0x01);
    writeCommand(0xD215);
    writeData(0x85);
    writeCommand(0xD216);
    writeData(0x01);
    writeCommand(0xD217);
    writeData(0xC8);
    writeCommand(0xD218);
    writeData(0x01);
    writeCommand(0xD219);
    writeData(0xFC);
    writeCommand(0xD21A);
    writeData(0x01);
    writeCommand(0xD21B);
    writeData(0xFD);
    writeCommand(0xD21C);
    writeData(0x02);
    writeCommand(0xD21D);
    writeData(0x29);
    writeCommand(0xD21E);
    writeData(0x02);
    writeCommand(0xD21F);
    writeData(0x56);
    writeCommand(0xD220);
    writeData(0x02);
    writeCommand(0xD221);
    writeData(0x6F);
    writeCommand(0xD222);
    writeData(0x02);
    writeCommand(0xD223);
    writeData(0x8C);
    writeCommand(0xD224);
    writeData(0x02);
    writeCommand(0xD225);
    writeData(0x9E);
    writeCommand(0xD226);
    writeData(0x02);
    writeCommand(0xD227);
    writeData(0xB4);
    writeCommand(0xD228);
    writeData(0x02);
    writeCommand(0xD229);
    writeData(0xC2);
    writeCommand(0xD22A);
    writeData(0x02);
    writeCommand(0xD22B);
    writeData(0xD6);
    writeCommand(0xD22C);
    writeData(0x02);
    writeCommand(0xD22D);
    writeData(0xE4);
    writeCommand(0xD22E);
    writeData(0x02);
    writeCommand(0xD22F);
    writeData(0xF9);
    writeCommand(0xD230);
    writeData(0x03);
    writeCommand(0xD231);
    writeData(0x25);
    writeCommand(0xD232);
    writeData(0x03);
    writeCommand(0xD233);
    writeData(0x8E);

    writeCommand(0xD300);
    writeData(0x00);
    writeCommand(0xD301);
    writeData(0x08);
    writeCommand(0xD302);
    writeData(0x00);
    writeCommand(0xD303);
    writeData(0x09);
    writeCommand(0xD304);
    writeData(0x00);
    writeCommand(0xD305);
    writeData(0x11);
    writeCommand(0xD306);
    writeData(0x00);
    writeCommand(0xD307);
    writeData(0x24);
    writeCommand(0xD308);
    writeData(0x00);
    writeCommand(0xD309);
    writeData(0x3A);
    writeCommand(0xD30A);
    writeData(0x00);
    writeCommand(0xD30B);
    writeData(0x69);
    writeCommand(0xD30C);
    writeData(0x00);
    writeCommand(0xD30D);
    writeData(0x95);
    writeCommand(0xD30E);
    writeData(0x00);
    writeCommand(0xD30F);
    writeData(0xDB);
    writeCommand(0xD310);
    writeData(0x01);
    writeCommand(0xD311);
    writeData(0x0E);
    writeCommand(0xD312);
    writeData(0x01);
    writeCommand(0xD313);
    writeData(0x54);
    writeCommand(0xD314);
    writeData(0x01);
    writeCommand(0xD315);
    writeData(0x85);
    writeCommand(0xD316);
    writeData(0x01);
    writeCommand(0xD317);
    writeData(0xC8);
    writeCommand(0xD318);
    writeData(0x01);
    writeCommand(0xD319);
    writeData(0xFC);
    writeCommand(0xD31A);
    writeData(0x01);
    writeCommand(0xD31B);
    writeData(0xFD);
    writeCommand(0xD31C);
    writeData(0x02);
    writeCommand(0xD31D);
    writeData(0x29);
    writeCommand(0xD31E);
    writeData(0x02);
    writeCommand(0xD31F);
    writeData(0x56);
    writeCommand(0xD320);
    writeData(0x02);
    writeCommand(0xD321);
    writeData(0x6F);
    writeCommand(0xD322);
    writeData(0x02);
    writeCommand(0xD323);
    writeData(0x8C);
    writeCommand(0xD324);
    writeData(0x02);
    writeCommand(0xD325);
    writeData(0x9E);
    writeCommand(0xD326);
    writeData(0x02);
    writeCommand(0xD327);
    writeData(0xB4);
    writeCommand(0xD328);
    writeData(0x02);
    writeCommand(0xD329);
    writeData(0xC2);
    writeCommand(0xD32A);
    writeData(0x02);
    writeCommand(0xD32B);
    writeData(0xD6);
    writeCommand(0xD32C);
    writeData(0x02);
    writeCommand(0xD32D);
    writeData(0xE4);
    writeCommand(0xD32E);
    writeData(0x02);
    writeCommand(0xD32F);
    writeData(0xF9);
    writeCommand(0xD330);
    writeData(0x03);
    writeCommand(0xD331);
    writeData(0x25);
    writeCommand(0xD332);
    writeData(0x03);
    writeCommand(0xD333);
    writeData(0x8E);

    writeCommand(0xD400);
    writeData(0x00);
    writeCommand(0xD401);
    writeData(0x08);
    writeCommand(0xD402);
    writeData(0x00);
    writeCommand(0xD403);
    writeData(0x09);
    writeCommand(0xD404);
    writeData(0x00);
    writeCommand(0xD405);
    writeData(0x11);
    writeCommand(0xD406);
    writeData(0x00);
    writeCommand(0xD407);
    writeData(0x24);
    writeCommand(0xD408);
    writeData(0x00);
    writeCommand(0xD409);
    writeData(0x3A);
    writeCommand(0xD40A);
    writeData(0x00);
    writeCommand(0xD40B);
    writeData(0x69);
    writeCommand(0xD40C);
    writeData(0x00);
    writeCommand(0xD40D);
    writeData(0x95);
    writeCommand(0xD40E);
    writeData(0x00);
    writeCommand(0xD40F);
    writeData(0xDB);
    writeCommand(0xD410);
    writeData(0x01);
    writeCommand(0xD411);
    writeData(0x0E);
    writeCommand(0xD412);
    writeData(0x01);
    writeCommand(0xD413);
    writeData(0x54);
    writeCommand(0xD414);
    writeData(0x01);
    writeCommand(0xD415);
    writeData(0x85);
    writeCommand(0xD416);
    writeData(0x01);
    writeCommand(0xD417);
    writeData(0xC8);
    writeCommand(0xD418);
    writeData(0x01);
    writeCommand(0xD419);
    writeData(0xFC);
    writeCommand(0xD41A);
    writeData(0x01);
    writeCommand(0xD41B);
    writeData(0xFD);
    writeCommand(0xD41C);
    writeData(0x02);
    writeCommand(0xD41D);
    writeData(0x29);
    writeCommand(0xD41E);
    writeData(0x02);
    writeCommand(0xD41F);
    writeData(0x56);
    writeCommand(0xD420);
    writeData(0x02);
    writeCommand(0xD421);
    writeData(0x6F);
    writeCommand(0xD422);
    writeData(0x02);
    writeCommand(0xD423);
    writeData(0x8C);
    writeCommand(0xD424);
    writeData(0x02);
    writeCommand(0xD425);
    writeData(0x9E);
    writeCommand(0xD426);
    writeData(0x02);
    writeCommand(0xD427);
    writeData(0xB4);
    writeCommand(0xD428);
    writeData(0x02);
    writeCommand(0xD429);
    writeData(0xC2);
    writeCommand(0xD42A);
    writeData(0x02);
    writeCommand(0xD42B);
    writeData(0xD6);
    writeCommand(0xD42C);
    writeData(0x02);
    writeCommand(0xD42D);
    writeData(0xE4);
    writeCommand(0xD42E);
    writeData(0x02);
    writeCommand(0xD42F);
    writeData(0xF9);
    writeCommand(0xD430);
    writeData(0x03);
    writeCommand(0xD431);
    writeData(0x25);
    writeCommand(0xD432);
    writeData(0x03);
    writeCommand(0xD433);
    writeData(0x8E);

    writeCommand(0xD500);
    writeData(0x00);
    writeCommand(0xD501);
    writeData(0x08);
    writeCommand(0xD502);
    writeData(0x00);
    writeCommand(0xD503);
    writeData(0x09);
    writeCommand(0xD504);
    writeData(0x00);
    writeCommand(0xD505);
    writeData(0x11);
    writeCommand(0xD506);
    writeData(0x00);
    writeCommand(0xD507);
    writeData(0x24);
    writeCommand(0xD508);
    writeData(0x00);
    writeCommand(0xD509);
    writeData(0x3A);
    writeCommand(0xD50A);
    writeData(0x00);
    writeCommand(0xD50B);
    writeData(0x69);
    writeCommand(0xD50C);
    writeData(0x00);
    writeCommand(0xD50D);
    writeData(0x95);
    writeCommand(0xD50E);
    writeData(0x00);
    writeCommand(0xD50F);
    writeData(0xDB);
    writeCommand(0xD510);
    writeData(0x01);
    writeCommand(0xD511);
    writeData(0x0E);
    writeCommand(0xD512);
    writeData(0x01);
    writeCommand(0xD513);
    writeData(0x54);
    writeCommand(0xD514);
    writeData(0x01);
    writeCommand(0xD515);
    writeData(0x85);
    writeCommand(0xD516);
    writeData(0x01);
    writeCommand(0xD517);
    writeData(0xC8);
    writeCommand(0xD518);
    writeData(0x01);
    writeCommand(0xD519);
    writeData(0xFC);
    writeCommand(0xD51A);
    writeData(0x01);
    writeCommand(0xD51B);
    writeData(0xFD);
    writeCommand(0xD51C);
    writeData(0x02);
    writeCommand(0xD51D);
    writeData(0x29);
    writeCommand(0xD51E);
    writeData(0x02);
    writeCommand(0xD51F);
    writeData(0x56);
    writeCommand(0xD520);
    writeData(0x02);
    writeCommand(0xD521);
    writeData(0x6F);
    writeCommand(0xD522);
    writeData(0x02);
    writeCommand(0xD523);
    writeData(0x8C);
    writeCommand(0xD524);
    writeData(0x02);
    writeCommand(0xD525);
    writeData(0x9E);
    writeCommand(0xD526);
    writeData(0x02);
    writeCommand(0xD527);
    writeData(0xB4);
    writeCommand(0xD528);
    writeData(0x02);
    writeCommand(0xD529);
    writeData(0xC2);
    writeCommand(0xD52A);
    writeData(0x02);
    writeCommand(0xD52B);
    writeData(0xD6);
    writeCommand(0xD52C);
    writeData(0x02);
    writeCommand(0xD52D);
    writeData(0xE4);
    writeCommand(0xD52E);
    writeData(0x02);
    writeCommand(0xD52F);
    writeData(0xF9);
    writeCommand(0xD530);
    writeData(0x03);
    writeCommand(0xD531);
    writeData(0x25);
    writeCommand(0xD532);
    writeData(0x03);
    writeCommand(0xD533);
    writeData(0x8E);

    writeCommand(0xD600);
    writeData(0x00);
    writeCommand(0xD601);
    writeData(0x08);
    writeCommand(0xD602);
    writeData(0x00);
    writeCommand(0xD603);
    writeData(0x09);
    writeCommand(0xD604);
    writeData(0x00);
    writeCommand(0xD605);
    writeData(0x11);
    writeCommand(0xD606);
    writeData(0x00);
    writeCommand(0xD607);
    writeData(0x24);
    writeCommand(0xD608);
    writeData(0x00);
    writeCommand(0xD609);
    writeData(0x3A);
    writeCommand(0xD60A);
    writeData(0x00);
    writeCommand(0xD60B);
    writeData(0x69);
    writeCommand(0xD60C);
    writeData(0x00);
    writeCommand(0xD60D);
    writeData(0x95);
    writeCommand(0xD60E);
    writeData(0x00);
    writeCommand(0xD60F);
    writeData(0xDB);
    writeCommand(0xD610);
    writeData(0x01);
    writeCommand(0xD611);
    writeData(0x0E);
    writeCommand(0xD612);
    writeData(0x01);
    writeCommand(0xD613);
    writeData(0x54);
    writeCommand(0xD614);
    writeData(0x01);
    writeCommand(0xD615);
    writeData(0x85);
    writeCommand(0xD616);
    writeData(0x01);
    writeCommand(0xD617);
    writeData(0xC8);
    writeCommand(0xD618);
    writeData(0x01);
    writeCommand(0xD619);
    writeData(0xFC);
    writeCommand(0xD61A);
    writeData(0x01);
    writeCommand(0xD61B);
    writeData(0xFD);
    writeCommand(0xD61C);
    writeData(0x02);
    writeCommand(0xD61D);
    writeData(0x29);
    writeCommand(0xD61E);
    writeData(0x02);
    writeCommand(0xD61F);
    writeData(0x56);
    writeCommand(0xD620);
    writeData(0x02);
    writeCommand(0xD621);
    writeData(0x6F);
    writeCommand(0xD622);
    writeData(0x02);
    writeCommand(0xD623);
    writeData(0x8C);
    writeCommand(0xD624);
    writeData(0x02);
    writeCommand(0xD625);
    writeData(0x9E);
    writeCommand(0xD626);
    writeData(0x02);
    writeCommand(0xD627);
    writeData(0xB4);
    writeCommand(0xD628);
    writeData(0x02);
    writeCommand(0xD629);
    writeData(0xC2);
    writeCommand(0xD62A);
    writeData(0x02);
    writeCommand(0xD62B);
    writeData(0xD6);
    writeCommand(0xD62C);
    writeData(0x02);
    writeCommand(0xD62D);
    writeData(0xE4);
    writeCommand(0xD62E);
    writeData(0x02);
    writeCommand(0xD62F);
    writeData(0xF9);
    writeCommand(0xD630);
    writeData(0x03);
    writeCommand(0xD631);
    writeData(0x25);
    writeCommand(0xD632);
    writeData(0x03);
    writeCommand(0xD633);
    writeData(0x8E);

    // #LV2 Page 0 enable
    writeCommand(0xF000);
    writeData(0x55);
    writeCommand(0xF001);
    writeData(0xAA);
    writeCommand(0xF002);
    writeData(0x52);
    writeCommand(0xF003);
    writeData(0x08);
    writeCommand(0xF004);
    writeData(0x00);

    // #480x854
    writeCommand(0xB500);
    writeData(0x6B);

    // #Display control
    writeCommand(0xB100);
    writeData(0xFC);
    writeCommand(0xB101);
    writeData(0x00);

    // #Source hold time
    writeCommand(0xB600);
    writeData(0x05);

    // #Gate EQ control
    writeCommand(0xB700);
    writeData(0x70);
    writeCommand(0xB701);
    writeData(0x70);

    // #Source EQ control (Mode 2)
    writeCommand(0xB800);
    writeData(0x01);
    writeCommand(0xB801);
    writeData(0x03);
    writeCommand(0xB802);
    writeData(0x03);
    writeCommand(0xB803);
    writeData(0x03);

    // #Inversion mode  (2-dot)
    writeCommand(0xBC00);
    writeData(0x00);
    writeCommand(0xBC01);
    writeData(0x00);
    writeCommand(0xBC02);
    writeData(0x00);

    // #Frame rate
    writeCommand(0xBD00);
    writeData(0x01);
    writeCommand(0xBD01);
    writeData(0x6C);
    writeCommand(0xBD02);
    writeData(0x1E);
    writeCommand(0xBD03);
    writeData(0x1D);
    writeCommand(0xBD04);
    writeData(0x00);

    // #Timing control 4H w/ 4-delay_ms(
    writeCommand(0xC900);
    writeData(0xD0);
    writeCommand(0xC901);
    writeData(0x02);
    writeCommand(0xC902);
    writeData(0x50);
    writeCommand(0xC903);
    writeData(0x50);
    writeCommand(0xC904);
    writeData(0x50);

    writeCommand(0x3A00);
    writeData(0x55);  // 0x77

    setRotation(rotation);

    writeCommand(0x3500);
    writeData(0x00);
    writeCommand(0x4400);
    writeData(0x01);
    writeCommand(0x4401);
    writeData(0x0F);

    writeCommand(0x1100);
    HAL_Delay(120);
    writeCommand(0x2900);
}

void Lcd::clearScreen() {
    fillScreen(backColor);
}

void Lcd::fillScreen() {
    fillScreen(foreColor);
}

void Lcd::fillScreen(RGB16Color color) {
    clearAddressWindow();
    LCD_RS_HIGH;
    fastFill(color, width * height);
}

void Lcd::drawPixel(uint16_t x, uint16_t y) {
    if ((x < width) && (y < height)) {
        setAddressWindow(x, y, x, y);
        LCD_RS_HIGH;
        LCD_DATA_GPIO_Port->ODR = foreColor;
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
}

void Lcd::drawHLine(uint16_t x, uint16_t y, uint16_t l) {
    if ((x < width) && (y < height)) {
        if ((x + l - 1) >= width) {
            l = width - x;
        }
        setAddressWindow(x, y, x + l - 1, y);
        LCD_RS_HIGH;
        fastFill(foreColor, l);
    }
}

void Lcd::drawVLine(uint16_t x, uint16_t y, uint16_t l) {
    if ((x < width) && (y < height)) {
        if ((y + l - 1) >= height) {
            l = height - y;
        }
        setAddressWindow(x, y, x, y + l - 1);
        LCD_RS_HIGH;
        fastFill(foreColor, l);
    }
}

void Lcd::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0);
        } else {
            drawPixel(x0, y0);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Lcd::drawRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h) {
    if ((x < width) && (y < height)) {
        if ((x + w - 1) >= width) {
            w = width - x;
        }
        if ((y + h - 1) >= height) {
            h = height - y;
        }
        drawHLine(x, y, w);
        drawHLine(x, y + h - 1, w);
        drawVLine(x, y, h);
        drawVLine(x + w - 1, y, h);
    }
}

void Lcd::fillRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h) {
    if ((x < width) && (y < height)) {
        if ((x + w - 1) >= width) {
            w = width - x;
        }
        if ((y + h - 1) >= height) {
            h = height - y;
        }
        setAddressWindow(x, y, x + w - 1, y + h - 1);
        LCD_RS_HIGH;
        fastFill(foreColor, w * h);
    }
}

void Lcd::drawCircle(uint16_t x, uint16_t y, uint16_t r) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t xc = 0;
    int16_t yc = r;
    drawPixel(x, y + r);
    drawPixel(x, y - r);
    drawPixel(x + r, y);
    drawPixel(x - r, y);
    while (xc < yc) {
        if (f >= 0) {
            yc--;
            ddF_y += 2;
            f += ddF_y;
        }
        xc++;
        ddF_x += 2;
        f += ddF_x;
        drawPixel(x + xc, y + yc);
        drawPixel(x - xc, y + yc);
        drawPixel(x + xc, y - yc);
        drawPixel(x - xc, y - yc);
        drawPixel(x + yc, y + xc);
        drawPixel(x - yc, y + xc);
        drawPixel(x + yc, y - xc);
        drawPixel(x - yc, y - xc);
    }
}

void Lcd::drawCircleHelper(uint16_t x, uint16_t y, uint16_t r, uint8_t cornername) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t xh = 0;
    int16_t yh = r;

    while (xh < yh) {
        if (f >= 0) {
            yh--;
            ddF_y += 2;
            f += ddF_y;
        }
        xh++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x + xh, y + yh);
            drawPixel(x + yh, y + xh);
        }
        if (cornername & 0x2) {
            drawPixel(x + xh, y - yh);
            drawPixel(x + yh, y - xh);
        }
        if (cornername & 0x8) {
            drawPixel(x - yh, y + xh);
            drawPixel(x - xh, y + yh);
        }
        if (cornername & 0x1) {
            drawPixel(x - yh, y - xh);
            drawPixel(x - xh, y - yh);
        }
    }
}

void Lcd::drawQuarterCircle(uint16_t x, uint16_t y, uint16_t r, uint8_t quarter) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t xc = 0;
    int16_t yc = r;
    switch (quarter) {
        case 0:
            drawPixel(x, y - r);
            drawPixel(x + r, y);
            break;

        case 1:
            drawPixel(x, y - r);
            drawPixel(x - r, y);
            break;

        case 2:
            drawPixel(x, y + r);
            drawPixel(x - r, y);
            break;

        case 3:
            drawPixel(x, y + r);
            drawPixel(x + r, y);
            break;

        default:
            break;
    }

    while (xc < yc) {
        if (f >= 0) {
            yc--;
            ddF_y += 2;
            f += ddF_y;
        }
        xc++;
        ddF_x += 2;
        f += ddF_x;
        switch (quarter) {
            case 0:
                drawPixel(x + xc, y - yc);
                drawPixel(x + yc, y - xc);
                break;

            case 1:
                drawPixel(x - xc, y - yc);
                drawPixel(x - yc, y - xc);
                break;

            case 2:
                drawPixel(x - xc, y + yc);
                drawPixel(x - yc, y + xc);
                break;

            case 3:
                drawPixel(x + xc, y + yc);
                drawPixel(x + yc, y + xc);
                break;

            default:
                break;
        }
    }
}

void Lcd::drawHalfCircle(uint16_t x, uint16_t y, uint16_t r, uint8_t half) {
    uint8_t h0;
    uint8_t h1;

    switch (half) {
        case 0:
            h0 = 0;
            h1 = 1;
            break;

        case 1:
            h0 = 1;
            h1 = 2;
            break;

        case 2:
            h0 = 2;
            h1 = 3;
            break;

        case 3:
            h0 = 3;
            h1 = 0;
            break;

        default:
            h0 = 0;
            h1 = 1;
            break;
    }

    drawQuarterCircle(x, y, r, h0);
    drawQuarterCircle(x, y, r, h1);
    /*
    while (xc < yc) {
    if (f >= 0) {
    yc--;
    ddF_y += 2;
    f += ddF_y;
  }
  xc++;
  ddF_x += 2;
  f += ddF_x;
  switch (half) {
  case 0:
  drawPixel(x + xc, y - yc);
  drawPixel(x + yc, y - xc);
  break;

  case 1:
  drawPixel(x - xc, y - yc);
  drawPixel(x - yc, y - xc);
  break;

  case 2:
  drawPixel(x - xc, y + yc);
  drawPixel(x - yc, y + xc);
  break;

  case 3:
  drawPixel(x + xc, y + yc);
  drawPixel(x + yc, y + xc);
  break;

  default:
  break;
  }
  }
  */
}

void Lcd::fillCircle(uint16_t x, uint16_t y, uint16_t r) {
    drawVLine(x, y - r, 2 * r + 1);
    fillCircleHelper(x, y, r, 3, 0);
}

void Lcd::fillCircleHelper(uint16_t x, uint16_t y, uint16_t r, uint8_t cornername, uint16_t delta) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t xh = 0;
    int16_t yh = r;

    while (xh < yh) {
        if (f >= 0) {
            yh--;
            ddF_y += 2;
            f += ddF_y;
        }
        xh++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x1) {
            drawVLine(x + xh, y - yh, 2 * yh + 1 + delta);
            drawVLine(x + yh, y - xh, 2 * xh + 1 + delta);
        }
        if (cornername & 0x2) {
            drawVLine(x - xh, y - yh, 2 * yh + 1 + delta);
            drawVLine(x - yh, y - xh, 2 * xh + 1 + delta);
        }
    }
}

void Lcd::drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    drawLine(x0, y0, x1, y1);
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x0, y0);
}

void Lcd::fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    int16_t a, b, y, last;

    if (y0 > y1) {
        swap(y0, y1);
        swap(x0, x1);
    }
    if (y1 > y2) {
        swap(y2, y1);
        swap(x2, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
        swap(x0, x1);
    }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        drawHLine(a, y0, b - a + 1);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    if (y1 == y2) {
        last = y1;
    } else {
        last = y1 - 1;
    }

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) swap(a, b);
        drawHLine(a, y, b - a + 1);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) swap(a, b);
        drawHLine(a, y, b - a + 1);
    }
}

void Lcd::drawText(const char text[], uint16_t textLength, uint16_t x, uint16_t y) {
    uint16_t wText = textLength * (fontWidth + fontSpacing);
    uint16_t hText = fontHeight;
    uint16_t xPos;
    uint16_t yPos = y;
    // set start position
    switch (fontAlignment) {
        case LEFT:
            xPos = x;
            break;
        case CENTER:
            xPos = x - (wText / 2);
            break;
        case RIGHT:
            xPos = x - wText;
            break;
        default:
            xPos = x;
            break;
    }
    // draw Text
    setAddressWindow(xPos, yPos, xPos + wText - 1, y + hText - 1);
    LCD_RS_HIGH;
    // row --> letter --> column
    for (uint16_t row = 0; row < hText; row++) {
        for (uint16_t letter = 0; letter < textLength; letter++) {
            // draw letter
            for (uint16_t column = 0; column < fontWidth; column++) {
                uint16_t offset = (getLetter(text[letter]) * fontHeight) + row;
                bool pixel = (*(fontData + offset) >> (fontWidth - column - 1)) & 0x01;
                if (pixel) {
                    LCD_DATA_GPIO_Port->ODR = foreColor;
                } else {
                    LCD_DATA_GPIO_Port->ODR = backColor;
                }
                LCD_WR_LOW;
                LCD_WR_HIGH;
            }
            // draw spacing
            LCD_DATA_GPIO_Port->ODR = backColor;
            for (uint8_t i = 0; i < fontSpacing; i++) {
                LCD_WR_LOW;
                LCD_WR_HIGH;
            }
        }
    }
}

void Lcd::drawNumber(int32_t num, uint8_t numSize, uint16_t x, uint16_t y) {
    if ((numSize > 0) && (numSize <= 10)) {
        char numChar[numSize];
        switch (numSize) {
            case 1:
                sprintf(numChar, "%d", num);
                break;

            case 2:
                sprintf(numChar, "%2d", num);
                break;

            case 3:
                sprintf(numChar, "%3d", num);
                break;

            case 4:
                sprintf(numChar, "%4d", num);
                break;

            case 5:
                sprintf(numChar, "%5d", num);
                break;

            case 6:
                sprintf(numChar, "%6d", num);
                break;

            case 7:
                sprintf(numChar, "%7d", num);
                break;

            case 8:
                sprintf(numChar, "%8d", num);
                break;

            case 9:
                sprintf(numChar, "%9d", num);
                break;

            case 10:
                sprintf(numChar, "%10d", num);
                break;
        }
        drawText(numChar, numSize, x, y);
    }
}

void Lcd::clearHLine(uint16_t x, uint16_t y, uint16_t l) {
    if ((x < width) && (y < height)) {
        if ((x + l - 1) >= width) {
            l = width - x;
        }
        setAddressWindow(x, y, x + l - 1, y);
        LCD_RS_HIGH;
        fastFill(backColor, l);
    }
}

void Lcd::clearVLine(uint16_t x, uint16_t y, uint16_t l) {
    if ((x < width) && (y < height)) {
        if ((y + l - 1) >= height) {
            l = height - y;
        }
        setAddressWindow(x, y, x, y + l - 1);
        LCD_RS_HIGH;
        fastFill(backColor, l);
    }
}

void Lcd::clearRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h) {
    if ((x < width) && (y < height)) {
        if ((x + w - 1) >= width) {
            w = width - x;
        }
        if ((y + h - 1) >= height) {
            h = height - y;
        }
        setAddressWindow(x, y, x + w - 1, y + h - 1);
        LCD_RS_HIGH;
        fastFill(backColor, w * h);
    }
}

void Lcd::drawRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // draw image
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    LCD_RS_HIGH;
    for (uint32_t i = 0; i < (w * h); i++) {
        LCD_DATA_GPIO_Port->ODR = indexPtr[pixelPtr[i]];
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
}

void Lcd::drawRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, float alpha, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // create 16 bit color index
    RGB16Color indexRGB16[indexSize];
    for (uint8_t i = 0; i < indexSize; i++) {
        indexRGB16[i] = alphaColor(indexPtr[i], alpha);
    }
    // draw image
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    LCD_RS_HIGH;
    for (uint32_t j = 0; j < (w * h); j++) {
        LCD_DATA_GPIO_Port->ODR = indexRGB16[pixelPtr[j]];
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
}

void Lcd::drawRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // create 16 bit color index
    RGB16Color indexRGB16[indexSize];
    for (uint8_t i = 0; i < indexSize; i++) {
        indexRGB16[i] = convertColor(indexPtr[i]);
    }
    // draw image
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    LCD_RS_HIGH;
    for (uint32_t j = 0; j < (w * h); j++) {
        LCD_DATA_GPIO_Port->ODR = indexRGB16[pixelPtr[j]];
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
}

void Lcd::drawRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, float alpha, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // create 16 bit color index
    RGB16Color indexRGB16[indexSize];
    for (uint8_t i = 0; i < indexSize; i++) {
        indexRGB16[i] = alphaColor(indexPtr[i], alpha);
    }
    // draw image
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    LCD_RS_HIGH;
    for (uint32_t j = 0; j < (w * h); j++) {
        LCD_DATA_GPIO_Port->ODR = indexRGB16[pixelPtr[j]];
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
}

void Lcd::fadeRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool mode, uint8_t step, uint16_t delay) {
    RGB16Color indexRGB16[indexSize];
    float alphaStep = 1.0f / step;
    float alpha;

    (mode) ? alpha = 0.0 : alpha = 1.0;

    for (uint8_t i = 0; i < step; i++) {
        // generate index
        for (uint8_t j = 0; j < indexSize; j++) {
            indexRGB16[j] = alphaColor(indexPtr[j], alpha);
        }
        // draw image
        setAddressWindow(x, y, x + w - 1, y + h - 1);
        LCD_RS_HIGH;
        for (uint32_t k = 0; k < (w * h); k++) {
            LCD_DATA_GPIO_Port->ODR = indexRGB16[pixelPtr[k]];
            LCD_WR_LOW;
            LCD_WR_HIGH;
        }
        HAL_Delay(delay);
        (mode) ? alpha += alphaStep : alpha -= alphaStep;
    }
}

void Lcd::fadeRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool mode, uint8_t step, uint16_t delay) {
    RGB16Color indexRGB16[indexSize];
    float alphaStep = 1.0f / step;
    float alpha;

    (mode) ? alpha = 0.0 : alpha = 1.0;

    for (uint8_t i = 0; i < step; i++) {
        // generate index
        for (uint8_t j = 0; j < indexSize; j++) {
            indexRGB16[j] = alphaColor(indexPtr[j], alpha);
        }
        // draw image
        setAddressWindow(x, y, x + w - 1, y + h - 1);
        LCD_RS_HIGH;
        for (uint32_t k = 0; k < (w * h); k++) {
            LCD_DATA_GPIO_Port->ODR = indexRGB16[pixelPtr[k]];
            LCD_WR_LOW;
            LCD_WR_HIGH;
        }
        HAL_Delay(delay);
        (mode) ? alpha += alphaStep : alpha -= alphaStep;
    }
}

void Lcd::drawSdFirmwareAlert(bool type) {
    uint16_t x;
    uint16_t y;
    RGB16Color indexRGB16[kSdIconIndexSize];

    // draw sdcard icon

    if (type) {
        for (uint8_t i = 0; i < kSdIconIndexSize; i++) {
            uint16_t red = 0;  // kSdIconIndex[i] >> 3;
            uint16_t green = kSdIconIndex[i] >> 2;
            uint16_t blue = 0;  // kSdIconIndex[i] >> 3;
            indexRGB16[i] = (red << 11) | (green << 5) | (blue);
        }
    } else {
        for (uint8_t i = 0; i < kSdIconIndexSize; i++) {
            uint16_t red = kSdIconIndex[i] >> 3;
            uint16_t green = 0; // kSdIconIndex[i] >> 2;
            uint16_t blue = 0;  // kSdIconIndex[i] >> 3;
            indexRGB16[i] = (red << 11) | (green << 5) | (blue);
        }
    }

    x = 388;
    y = 190;
    setAddressWindow(x, y, x + kSdIconWidth - 1, y + kSdIconHeight - 1);
    LCD_RS_HIGH;
    for (uint32_t j = 0; j < (kSdIconWidth * kSdIconHeight); j++) {
        LCD_DATA_GPIO_Port->ODR = indexRGB16[kSdIconData[j]];
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }

    setForeColor(BLACK);
    (type) ? setBackColor(GREEN) : setBackColor(RED);
    setFont(FONT_07x09);
    setAlignment(CENTER);
    drawText("BIN", 3, 427, 270);
}

void Lcd::clearSdFirmwareAlert() {
    setForeColor(BLACK);
    fillRect(307, 190, 240, 240);
    setForeColor(WHITE);
}

void Lcd::invertDisplay(bool invert) {
    if (invert) {
        writeCommand(LCD_ENTER_INVERT_MODE);
    } else {
        writeCommand(LCD_EXIT_INVERT_MODE);
    }
}

void Lcd::displayOn() {
    writeCommand(LCD_SET_DISPLAY_ON);
    clearScreen();
    HAL_Delay(120);
    LCD_BL_HIGH;
}

void Lcd::displayOff() {
    LCD_BL_LOW;
    HAL_Delay(120);
    writeCommand(LCD_SET_DISPLAY_OFF);
}
