#ifndef __DAC_H
#define __DAC_H

#include "Global.h"
#include "SGTL5000.h"
#include "i2c.h"
#include "i2s.h"
#include "main.h"

class Dac {
   private:
    uint16_t readRegisterFull(uint16_t reg);
    void writeRegisterFull(uint16_t reg, uint16_t data);
    uint16_t readRegisterMask(uint16_t reg, uint16_t mask, uint16_t shift);
    void writeRegisterMask(uint16_t reg, uint16_t mask, uint16_t shift, uint16_t input);

   public:
    Dac();
    ~Dac();

    uint16_t i2s_data[4];
    uint16_t i2s_counter;

    void initialize();
    void enableAudioPostProcessor();
    void disableAudioPostProcessor();
    void enableSurround();
    void disableSurround();
    void setSurround(uint8_t width);
    void enableBassEnhance();
    void disableBassEnhance();
    void setBassEnhance(uint8_t lrLevel, uint8_t bassLevel);
    void enable5BandEq();
    void disable5BandEq();
    void set5BandEQ_Freq00(uint8_t level);
    void set5BandEQ_Freq01(uint8_t level);
    void set5BandEQ_Freq02(uint8_t level);
    void set5BandEQ_Freq03(uint8_t level);
    void set5BandEQ_Freq04(uint8_t level);
    void muteHeadphone();
    void unmuteHeadphone();
    void setHeadphoneVolume(uint8_t dBData);
    void muteLineout();
    void unmuteLineout();
    void setLineoutVolume(uint8_t dBData);
    void audioOn();
    void audioOff();
    void enableAVC();
    void disableAVC();

    void testOn();
    void testOff();
};

#endif
