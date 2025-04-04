#include "Dac.h"

Dac::Dac()
    : i2s_data{0, 0, 0, 0} {}

Dac::~Dac() {}

uint16_t Dac::readRegisterFull(uint16_t reg) {
    uint8_t txData[2];
    uint8_t rxData[2];

    txData[0] = (reg >> 8) & 0xFF;
    txData[1] = reg & 0xFF;

    HAL_I2C_Master_Transmit(&hi2c3, SGTL5000_I2C_ADDRESS, txData, 2, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c3, SGTL5000_I2C_ADDRESS, rxData, 2, HAL_MAX_DELAY);

    uint16_t data = (rxData[0] << 8) | rxData[1];

    return data;
}

void Dac::writeRegisterFull(uint16_t reg, uint16_t data) {
    uint8_t txData[4];

    txData[0] = (reg >> 8) & 0xFF;
    txData[1] = reg & 0xFF;
    txData[2] = (data >> 8) & 0xFF;
    txData[3] = data & 0xFF;

    HAL_I2C_Master_Transmit(&hi2c3, SGTL5000_I2C_ADDRESS, txData, 4, HAL_MAX_DELAY);
}

uint16_t Dac::readRegisterMask(uint16_t reg, uint16_t mask, uint16_t shift) {
    uint8_t txData[2];
    uint8_t rxData[2];

    txData[0] = (reg >> 8) & 0xFF;
    txData[1] = reg & 0xFF;

    HAL_I2C_Master_Transmit(&hi2c3, SGTL5000_I2C_ADDRESS, txData, 2, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c3, SGTL5000_I2C_ADDRESS, rxData, 2, HAL_MAX_DELAY);

    uint16_t data = (rxData[0] << 8) | rxData[1];

    data = (data & mask) >> shift;

    return data;
}

void Dac::writeRegisterMask(uint16_t reg, uint16_t mask, uint16_t shift, uint16_t input) {
    uint8_t txData[4];
    uint8_t rxData[2];

    txData[0] = (reg >> 8) & 0xFF;
    txData[1] = reg & 0xFF;

    HAL_I2C_Master_Transmit(&hi2c3, SGTL5000_I2C_ADDRESS, txData, 2, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c3, SGTL5000_I2C_ADDRESS, rxData, 2, HAL_MAX_DELAY);

    uint16_t data = (rxData[0] << 8) | rxData[1];

    data = (data & (~mask)) + (input << shift);

    txData[2] = (data >> 8) & 0xFF;
    txData[3] = data & 0xFF;

    HAL_I2C_Master_Transmit(&hi2c3, SGTL5000_I2C_ADDRESS, txData, 4, HAL_MAX_DELAY);
}

void Dac::initialize() {
    /*
    if (HAL_I2C_IsDeviceReady(&hi2c3, SGTL5000_I2C_ADDRESS, 1, 10) == HAL_OK) {
        // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
    */

    writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_LEFT_MASK, ANA_HP_CTRL_HP_VOL_LEFT_SHIFT, 0x7F);
    writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_RIGHT_MASK, ANA_HP_CTRL_HP_VOL_RIGHT_SHIFT, 0x7F);

    writeRegisterFull(ANA_POWER_REG, 0x4060);
    writeRegisterFull(LINREG_CTRL_REG, 0x006C);
    writeRegisterFull(REF_CTRL_REG, 0x01F2);
    writeRegisterFull(LINE_OUT_CTRL_REG, 0x0F22);
    writeRegisterFull(SHORT_CTRL_REG, 0x4446);
    writeRegisterFull(ANA_CTRL_REG, 0x0137);

    // DAP Control
    writeRegisterMask(SSS_CTRL_REG, SSS_CTRL_DAP_MIX_SELECT_MASK, SSS_CTRL_DAP_MIX_SELECT_SHIFT, 0x01);
    writeRegisterMask(SSS_CTRL_REG, SSS_CTRL_DAP_SELECT_MASK, SSS_CTRL_DAP_SELECT_SHIFT, 0x01);
    writeRegisterMask(SSS_CTRL_REG, SSS_CTRL_DAC_SELECT_MASK, SSS_CTRL_DAC_SELECT_SHIFT, 0x01);
    writeRegisterMask(DAP_CONTROL_REG, DAP_CONTROL_DAP_EN_MASK, DAP_CONTROL_DAP_EN_SHIFT, 0x01);

    writeRegisterMask(DAP_AUDIO_EQ_REG, DAP_AUDIO_EQ_EN_MASK, DAP_AUDIO_EQ_EN_SHIFT, 0x00);

    // writeRegisterMask(DAP_PEQ_REG, DAP_PEQ_EN_MASK, DAP_PEQ_EN_SHIFT, 0x07);

    writeRegisterMask(REF_CTRL_REG, REF_CTRL_SMALL_POP_MASK, REF_CTRL_SMALL_POP_SHIFT, 0x01);

    // __HAL_UNLOCK(&hi2s3);
    // __HAL_I2S_ENABLE(&hi2s3);
    // HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)i2s_data, 4);

    writeRegisterFull(ANA_POWER_REG, 0x40FF);
    writeRegisterFull(DIG_POWER_REG, 0x0073);

    // HAL_Delay(400);

    writeRegisterMask(CLK_CTRL_REG, CLK_CTRL_SYS_FS_MASK, CLK_CTRL_SYS_FS_SHIFT, 0x01);  // 0x00 = 32 kHz, 0x01 = 44.1 kHz, 0x02 = 48 kHz, 0x03 = 96 kHz
    writeRegisterMask(I2S_CTRL_REG, I2S_CTRL_DLEN_MASK, I2S_CTRL_DLEN_SHIFT, 0x01);      // 0x00 = 32 bits, 0x01 = 24 bits, 0x02 = 20 bits, 0x03 = 16 bits
    writeRegisterMask(I2S_CTRL_REG, I2S_CTRL_SCLK_INV_MASK, I2S_CTRL_SCLK_INV_SHIFT, 0x00);

    writeRegisterMask(SSS_CTRL_REG, SSS_CTRL_DAP_SELECT_MASK, SSS_CTRL_DAP_SELECT_SHIFT, 0x01);  // 0x01
    writeRegisterMask(SSS_CTRL_REG, SSS_CTRL_DAC_SELECT_MASK, SSS_CTRL_DAC_SELECT_SHIFT, 0x03);  // 0x03
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_SELECT_HP_MASK, ANA_CTRL_SELECT_HP_SHIFT, 0x00);

    writeRegisterMask(DAP_CONTROL_REG, DAP_CONTROL_DAP_EN_MASK, DAP_CONTROL_DAP_EN_SHIFT, 0x01);

    // AVC
    // enableAVC();
    // Surround
    // enableSurround();
    // setSurround(0x07);
    // Bass Enhance
    // disableBassEnhance();
    // setBassEnhance(0x05, 0x1F);
    // DAC Volume
    writeRegisterMask(ADCDAC_CTRL_REG, ADCDAC_CTRL_DAC_MUTE_LEFT_MASK, ADCDAC_CTRL_DAC_MUTE_LEFT_SHIFT, 0x00);
    writeRegisterMask(ADCDAC_CTRL_REG, ADCDAC_CTRL_DAC_MUTE_RIGHT_MASK, ADCDAC_CTRL_DAC_MUTE_RIGHT_SHIFT, 0x00);
    writeRegisterMask(DAC_VOL_REG, DAC_VOL_LEFT_MASK, DAC_VOL_LEFT_SHIFT, 0x3C);
    writeRegisterMask(DAC_VOL_REG, DAC_VOL_RIGHT_MASK, DAC_VOL_RIGHT_SHIFT, 0x3C);
    // HP Volume
    writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_LEFT_MASK, ANA_HP_CTRL_HP_VOL_LEFT_SHIFT, 0x18);    // min 0x7F     max 0x17
    writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_RIGHT_MASK, ANA_HP_CTRL_HP_VOL_RIGHT_SHIFT, 0x18);  // min 0x7F     max 0x17
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_HP_MASK, ANA_CTRL_MUTE_HP_SHIFT, 0x00);
    // Lineout Volume
    writeRegisterMask(LINE_OUT_VOL_REG, LINE_OUT_VOL_LO_VOL_LEFT_MASK, LINE_OUT_VOL_LO_VOL_LEFT_SHIFT, 0x19);
    writeRegisterMask(LINE_OUT_VOL_REG, LINE_OUT_VOL_LO_VOL_RIGHT_MASK, LINE_OUT_VOL_LO_VOL_RIGHT_SHIFT, 0x19);
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_LO_MASK, ANA_CTRL_MUTE_LO_SHIFT, 0x00);
}

void Dac::enableAudioPostProcessor() {
    writeRegisterMask(DAP_CONTROL_REG, DAP_CONTROL_DAP_EN_MASK, DAP_CONTROL_DAP_EN_SHIFT, 0x01);
}

void Dac::disableAudioPostProcessor() {
    writeRegisterMask(DAP_CONTROL_REG, DAP_CONTROL_DAP_EN_MASK, DAP_CONTROL_DAP_EN_SHIFT, 0x00);
}

void Dac::enableSurround() {
    writeRegisterMask(DAP_SGTL_SURROUND_REG, DAP_SGTL_SURROUND_SELECT_MASK, DAP_SGTL_SURROUND_SELECT_SHIFT, 0x02);  // 0x02 mono, 0x03 stereo
}

void Dac::disableSurround() {
    writeRegisterMask(DAP_SGTL_SURROUND_REG, DAP_SGTL_SURROUND_SELECT_MASK, DAP_SGTL_SURROUND_SELECT_SHIFT, 0x00);
}

void Dac::setSurround(uint8_t width) {
    if ((width >= 0x00) && (width <= 0x07)) {
        uint8_t currentWidth = readRegisterMask(DAP_SGTL_SURROUND_REG, DAP_SGTL_SURROUND_WIDTH_CONTROL_MASK, DAP_SGTL_SURROUND_WIDTH_CONTROL_SHIFT);
        uint8_t step = abs(width - currentWidth);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (width > currentWidth) ? currentWidth += 1 : currentWidth -= 1;
                writeRegisterMask(DAP_SGTL_SURROUND_REG, DAP_SGTL_SURROUND_WIDTH_CONTROL_MASK, DAP_SGTL_SURROUND_WIDTH_CONTROL_SHIFT, currentWidth);
            }
        }
    }
}

void Dac::enableBassEnhance() {
    writeRegisterMask(DAP_BASS_ENHANCE_REG, DAP_BASS_ENHANCE_EN_MASK, DAP_BASS_ENHANCE_EN_SHIFT, 0x01);
}

void Dac::disableBassEnhance() {
    writeRegisterMask(DAP_BASS_ENHANCE_REG, DAP_BASS_ENHANCE_EN_MASK, DAP_BASS_ENHANCE_EN_SHIFT, 0x00);
}

void Dac::setBassEnhance(uint8_t lrLevel, uint8_t bassLevel) {
    if ((lrLevel >= 0x00) && (lrLevel <= 0x3F)) {
        uint8_t currentLevel = readRegisterMask(
            DAP_BASS_ENHANCE_CTRL_REG, DAP_BASS_ENHANCE_CTRL_LR_LEVEL_MASK, DAP_BASS_ENHANCE_CTRL_LR_LEVEL_SHIFT);
        uint8_t step = abs(lrLevel - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (lrLevel > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_BASS_ENHANCE_CTRL_REG, DAP_BASS_ENHANCE_CTRL_LR_LEVEL_MASK, DAP_BASS_ENHANCE_CTRL_LR_LEVEL_SHIFT, currentLevel);
            }
        }
    }

    if ((bassLevel >= 0x00) && (bassLevel <= 0x7F)) {
        uint8_t currentLevel = readRegisterMask(DAP_BASS_ENHANCE_CTRL_REG, DAP_BASS_ENHANCE_BASS_LEVEL_MASK, DAP_BASS_ENHANCE_BASS_LEVEL_SHIFT);
        uint8_t step = abs(bassLevel - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (bassLevel > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_BASS_ENHANCE_CTRL_REG, DAP_BASS_ENHANCE_BASS_LEVEL_MASK, DAP_BASS_ENHANCE_BASS_LEVEL_SHIFT, currentLevel);
            }
        }
    }
}

void Dac::enable5BandEq() {
    writeRegisterMask(DAP_AUDIO_EQ_REG, DAP_AUDIO_EQ_EN_MASK, DAP_AUDIO_EQ_EN_SHIFT, 0x03);
}

void Dac::disable5BandEq() {
    writeRegisterMask(DAP_AUDIO_EQ_REG, DAP_AUDIO_EQ_EN_MASK, DAP_AUDIO_EQ_EN_SHIFT, 0x00);
}

void Dac::set5BandEQ_Freq00(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x5F)) {
        uint8_t currentLevel = readRegisterMask(DAP_AUDIO_EQ_BAND0_REG, DAP_AUDIO_EQ_BAND0_VOLUME_MASK, DAP_AUDIO_EQ_BAND0_VOLUME_SHIFT);
        uint8_t step = abs(level - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (level > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_AUDIO_EQ_BAND0_REG, DAP_AUDIO_EQ_BAND0_VOLUME_MASK, DAP_AUDIO_EQ_BAND0_VOLUME_SHIFT, currentLevel);
                HAL_Delay(1);
            }
        }
    }
}

void Dac::set5BandEQ_Freq01(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x5F)) {
        uint8_t currentLevel = readRegisterMask(DAP_AUDIO_EQ_BAND1_REG, DAP_AUDIO_EQ_BAND1_VOLUME_MASK, DAP_AUDIO_EQ_BAND1_VOLUME_SHIFT);
        uint8_t step = abs(level - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (level > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_AUDIO_EQ_BAND1_REG, DAP_AUDIO_EQ_BAND1_VOLUME_MASK, DAP_AUDIO_EQ_BAND1_VOLUME_SHIFT, currentLevel);
                HAL_Delay(1);
            }
        }
    }
}

void Dac::set5BandEQ_Freq02(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x5F)) {
        uint8_t currentLevel = readRegisterMask(DAP_AUDIO_EQ_BAND2_REG, DAP_AUDIO_EQ_BAND2_VOLUME_MASK, DAP_AUDIO_EQ_BAND2_VOLUME_SHIFT);
        uint8_t step = abs(level - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (level > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_AUDIO_EQ_BAND2_REG, DAP_AUDIO_EQ_BAND2_VOLUME_MASK, DAP_AUDIO_EQ_BAND2_VOLUME_SHIFT, currentLevel);
            }
        }
    }
}

void Dac::set5BandEQ_Freq03(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x5F)) {
        uint8_t currentLevel = readRegisterMask(DAP_AUDIO_EQ_BAND3_REG, DAP_AUDIO_EQ_BAND3_VOLUME_MASK, DAP_AUDIO_EQ_BAND3_VOLUME_SHIFT);
        uint8_t step = abs(level - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (level > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_AUDIO_EQ_BAND3_REG, DAP_AUDIO_EQ_BAND3_VOLUME_MASK, DAP_AUDIO_EQ_BAND3_VOLUME_SHIFT, currentLevel);
            }
        }
    }
}

void Dac::set5BandEQ_Freq04(uint8_t level) {
    if ((level >= 0x00) && (level <= 0x5F)) {
        uint8_t currentLevel = readRegisterMask(DAP_AUDIO_EQ_BAND4_REG, DAP_AUDIO_EQ_BAND4_VOLUME_MASK, DAP_AUDIO_EQ_BAND4_VOLUME_SHIFT);
        uint8_t step = abs(level - currentLevel);
        if (step != 0) {
            for (uint8_t i = 0; i < step; i++) {
                (level > currentLevel) ? currentLevel += 1 : currentLevel -= 1;
                writeRegisterMask(DAP_AUDIO_EQ_BAND4_REG, DAP_AUDIO_EQ_BAND4_VOLUME_MASK, DAP_AUDIO_EQ_BAND4_VOLUME_SHIFT, currentLevel);
            }
        }
    }
}

void Dac::muteHeadphone() {
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_HP_MASK, ANA_CTRL_MUTE_HP_SHIFT, 0x01);
}

void Dac::unmuteHeadphone() {
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_HP_MASK, ANA_CTRL_MUTE_HP_SHIFT, 0x00);
}

void Dac::setHeadphoneVolume(uint8_t dBData) {
    uint8_t currentdBData = readRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_LEFT_MASK, ANA_HP_CTRL_HP_VOL_LEFT_SHIFT);
    if (currentdBData != dBData) {
        if ((currentdBData == 0x7F) && (dBData != 0x7F)) unmuteHeadphone();
        if ((currentdBData != 0x7F) && (dBData == 0x7F)) muteHeadphone();
        int8_t increment;
        (currentdBData < dBData) ? increment = 1 : increment = -1;
        while (currentdBData != dBData) {
            currentdBData += increment;
            writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_LEFT_MASK, ANA_HP_CTRL_HP_VOL_LEFT_SHIFT, currentdBData);
            writeRegisterMask(ANA_HP_CTRL_REG, ANA_HP_CTRL_HP_VOL_RIGHT_MASK, ANA_HP_CTRL_HP_VOL_RIGHT_SHIFT, currentdBData);
        }
    }
}

void Dac::muteLineout() {
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_LO_MASK, ANA_CTRL_MUTE_LO_SHIFT, 0x01);
}

void Dac::unmuteLineout() {
    writeRegisterMask(ANA_CTRL_REG, ANA_CTRL_MUTE_LO_MASK, ANA_CTRL_MUTE_LO_SHIFT, 0x00);
}

void setLineoutVolume(uint8_t dBData) {}

void Dac::audioOn() {}

void Dac::audioOff() {}

void Dac::enableAVC() {
    writeRegisterMask(DAP_AVC_CTRL_REG, DAP_AVC_CTRL_EN_MASK, DAP_AVC_CTRL_EN_SHIFT, 0x01);
}

void Dac::disableAVC() {
    writeRegisterMask(DAP_AVC_CTRL_REG, DAP_AVC_CTRL_EN_MASK, DAP_AVC_CTRL_EN_SHIFT, 0x00);
}

void Dac::testOn() {
    int filterCoef[7][5] = {
        {266072, -514164, 248121, 514165, -252048},
        {262120, -522148, 260093, 522149, -260069},
        {256275, -512012, 256275, 512013, -250406},
        {206760, -391574, 206760, 391575, -151375},
        {268252, -367346, 155924, 367347, -162031},
        {263811, -234013, 204217, 234014, -205882},
        {321343, -9222, 249942, 112935, -205427}};

    for (int i = 0; i < 7; i++) {
        writeRegisterFull(DAP_COEF_WR_B0_MSB_REG, (filterCoef[i][0] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B0_LSB_REG, filterCoef[i][0] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_B1_MSB_REG, (filterCoef[i][1] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B1_LSB_REG, filterCoef[i][1] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_B2_MSB_REG, (filterCoef[i][2] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B2_LSB_REG, filterCoef[i][2] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_A1_MSB_REG, (filterCoef[i][3] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_A1_LSB_REG, filterCoef[i][3] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_A2_MSB_REG, (filterCoef[i][4] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_A2_LSB_REG, filterCoef[i][4] & 0x0F);

        writeRegisterFull(DAP_FILTER_COEF_ACCESS_REG, 0x0101 + i);
    }

    // writeRegisterMask(DAP_AUDIO_EQ_BAND0_REG, DAP_AUDIO_EQ_BAND4_VOLUME_MASK, DAP_AUDIO_EQ_BAND4_VOLUME_SHIFT, 0x5F);
    // writeRegisterMask(DAP_AUDIO_EQ_BAND4_REG, DAP_AUDIO_EQ_BAND4_VOLUME_MASK, DAP_AUDIO_EQ_BAND4_VOLUME_SHIFT, 0x5F);
}

void Dac::testOff() {
    int filterCoef[7][5] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}};

    for (int i = 0; i < 7; i++) {
        writeRegisterFull(DAP_COEF_WR_B0_MSB_REG, (filterCoef[i][0] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B0_LSB_REG, filterCoef[i][0] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_B1_MSB_REG, (filterCoef[i][1] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B1_LSB_REG, filterCoef[i][1] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_B2_MSB_REG, (filterCoef[i][2] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_B2_LSB_REG, filterCoef[i][2] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_A1_MSB_REG, (filterCoef[i][3] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_A1_LSB_REG, filterCoef[i][3] & 0x0F);
        writeRegisterFull(DAP_COEF_WR_A2_MSB_REG, (filterCoef[i][4] >> 4) & 0xFFFF);
        writeRegisterFull(DAP_COEF_WR_A2_LSB_REG, filterCoef[i][4] & 0x0F);

        writeRegisterFull(DAP_FILTER_COEF_ACCESS_REG, 0x0101 + i);
    }
}
