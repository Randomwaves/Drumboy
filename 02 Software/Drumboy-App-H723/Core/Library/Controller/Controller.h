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

extern "C" uint8_t syncInData[2];
extern "C" uint8_t syncOutData[2];

extern "C" uint8_t midiRxData;

extern "C" UART_HandleTypeDef huart4; // sync-out
extern "C" UART_HandleTypeDef huart7; // sync-in

extern "C" UART_HandleTypeDef huart1; // midi-rx
extern "C" UART_HandleTypeDef huart6; // midi-tx

extern "C" TIM_HandleTypeDef htim5;  // transition timer
extern "C" TIM_HandleTypeDef htim6;  // left button timer
extern "C" TIM_HandleTypeDef htim7;  // right button timer
extern "C" TIM_HandleTypeDef htim12; // updown long press timer
extern "C" TIM_HandleTypeDef htim13; // button long press timer
extern "C" TIM_HandleTypeDef htim14; // play timer
extern "C" TIM_HandleTypeDef htim15; // text timer
extern "C" TIM_HandleTypeDef htim16; // power button timer
extern "C" TIM_HandleTypeDef htim17; // sd check timer
extern "C" TIM_HandleTypeDef htim23; // beat sync timer
extern "C" TIM_HandleTypeDef htim24; // limit alert timer

class Controller {
private:
    Sys system;
    Rhythm rhythm;
    Metronome metronome;
    Lpf lpf;
    Eq eq;
    Filter filter[kFilterLibrarySize];
    Effect effect[kEffectLibrarySize];
    Reverb reverb;

    Layer layerLibrary[kLayerLibrarySize];

    MetroPlayData mD;
    SongPlayData sD;

    int32_t audioMetronome; // 32-bit
    int32_t audioSong;      // 24-bit
    int32_t audioLpf;       // 24-bit
    int32_t audioEq;        // 24-bit
    int32_t audioFilter;    // 24-bit
    int32_t audioEffect;    // 24-bit
    int32_t audioReverb_L;  // 32-bit
    int32_t audioReverb_R;  // 32-bit
    int32_t audioSend_L;    // 32-bit
    int32_t audioSend_R;    // 32-bit

    int8_t selectedLayerNum;
    int8_t selectedBeatNum;

    uint8_t activeBankNum;
    uint8_t targetBankNum;
    bool bankShiftFlag;
    bool bankActionFlag;

    Menu menu;
    Menu preMenu;
    int8_t menuTab;
    int8_t preMenuTab;
    int8_t preFill;

    int8_t subMenuTab;

    LayerTab layerTab;

    bool sdInsertCheck;

    uint16_t fileLibrarySize;
    uint16_t drumkitLibrarySize;
    uint16_t instLibrarySize;
    uint16_t sampleLibrarySize[kInstLibraryMaxSize];
    uint32_t totalSampleLibrarySize;

    uint16_t songInterval;
    uint16_t barInterval;
    uint16_t measureInterval;
    uint16_t playInterval;
    uint16_t stopInterval;
    uint16_t resetInterval;

    bool stopFlag;
    bool resetFlag;

    bool alertFlag;
    AlertType alertType;

    Icon resetIcon;
    Icon playIcon;
    Icon stopIcon;
    Icon recordIcon;

    bool resetPlayFlag;

    FileStatus fileStatus;
    FileStatus drumkitStatus;

    TriggerMode triggerMode;

    bool playActive;
    bool recordActive;

    uint32_t playTimerPeriod;
    uint32_t measureTimerPeriod;

    uint16_t playX;
    float playXRatio;
    RGB16Color playColor;

    bool power;

    bool powerButtonFlag;
    uint8_t powerButtonCounter;

    bool mainMenuButtonFlag;

    bool upButtonFlag;
    bool downButtonFlag;
    uint8_t upDownButtonCounter;

    bool layerBeatButtonFlag;
    bool layerClearButtonFlag;

    bool layerInstCopyButtonFlag;
    bool layerInstPasteButtonFlag;

    bool layerFillCopyButtonFlag;
    bool layerFillPasteButtonFlag;

    bool layerSongCopyButtonFlag;
    bool layerSongPasteButtonFlag;

    bool layerInstPasteActionFlag;
    bool layerSongPasteActionFlag;

    bool rhythmUnlockFlag;
    bool rhythmLockFlag;

    bool textCopyFlag;
    bool textPasteFlag;
    bool textClearFlag;

    bool limitAlertShowFlag;
    bool limitAlertClearFlag;
    bool limitAlertActive;

    int8_t copyLayerInstNum;
    int8_t copyLayerFillNum;
    int8_t copyLayerSongNum;
    int8_t copyBankSongNum;

    uint8_t layerBeatButtonStage;

    int8_t fileMenuCounter;
    int8_t drumkitMenuCounter;

    uint8_t transitionShowFlag;
    bool transitionClearFlag;

    bool animation;
    bool textShow;

    bool sampleFadeOut;

public:
    Controller();
    ~Controller();

    Lcd lcd;
    Dac dac;
    Sd sd;
    Keyboard keyboard;

    Menu getMenu() { return menu; }
    Sys getSystem() { return system; }

    bool midiTxBusy;

    bool getPower() { return power; }

    void initialize();
    void systemStart();
    void systemReset();
    void systemUpdate_A();
    void systemUpdate_B();
    bool sendMidiCommand(uint8_t command_, uint8_t data0_, uint8_t data1_);
    void receiveMidiCommand();
    bool sendSyncCommand(uint8_t data_);
    void receiveSyncCommand();
    void calculateSongInterval();
    uint16_t calculateTriggerInterval();
    void adjustMeasureBarTiming();
    void updatePlayTimerPeriod();

    /* Button functions --------------------------------------------------------*/

    void button_check();

    /* Keyboard functions ----------------------------------------------------*/

    void keyboard_initialize();
    void keyboard_check_A();
    void keyboard_check_B();
    void keyboard_check_C();
    void keyboard_enable();
    void keyboard_disable();

    LayerTab getLayerTab() const {
        return layerTab;
    }
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

    /* Timer functions -------------------------------------------------------*/

    void startTransitionTimer() {
        HAL_TIM_Base_Start_IT(&htim5);
    }
    void stopTransitionTimer() {
        HAL_TIM_Base_Stop_IT(&htim5);
        __HAL_TIM_SET_COUNTER(&htim5, 0);
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
    void startUpDownButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim12);
    }
    void stopUpDownButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim12);
        __HAL_TIM_SET_COUNTER(&htim12, 0);
    }
    void startLongButtonTimer() {
        HAL_TIM_Base_Start_IT(&htim13);
    }
    void stopLongButtonTimer() {
        HAL_TIM_Base_Stop_IT(&htim13);
        __HAL_TIM_SET_COUNTER(&htim13, 0);
    }
    void startPlayTimer() {
        HAL_TIM_Base_Start_IT(&htim14);
    }
    void stopPlayTimer() {
        HAL_TIM_Base_Stop_IT(&htim14);
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
    void startBeatSyncTimer() {
        HAL_TIM_Base_Start_IT(&htim23);
    }
    void stopBeatSyncTimer() {
        HAL_TIM_Base_Stop_IT(&htim23);
        __HAL_TIM_SET_COUNTER(&htim23, 0);
    }
    void startLimitAlertTimer() {
        HAL_TIM_Base_Start_IT(&htim24);
    }
    void stopLimitAlertTimer() {
        HAL_TIM_Base_Stop_IT(&htim24);
        __HAL_TIM_SET_COUNTER(&htim24, 0);
    }

    /* Dac functions ---------------------------------------------------------*/

    void dac_initialize();

    /* Sd functions ----------------------------------------------------------*/

    SdResult sd_initialize();
    SdResult sd_reinitialize();
    SdResult sd_detect();
    SdResult sd_mount();
    SdResult sd_unmount();
    SdResult sd_getLabel();
    SdResult sd_setLabel();
    SdResult sd_getSpace();
    SdResult sd_checkFileExist(const char *fileAddress);
    SdResult sd_checkFolderExist(const char *folderAddress);
    SdResult sd_loadImage(char *fileAddress, uint32_t paletteAddress, uint32_t dataAddress, uint16_t paletteSize, uint16_t width, uint16_t height, RGBMode mode);
    SdResult load16BitAudio(char *fileAddress, uint32_t ramAddress, uint32_t sampleSize);
    SdResult load24BitAudio(char *fileAddress, uint32_t ramAddress, uint32_t sampleSize);
    SdResult sd_checkMetronome();
    SdResult sd_loadMetronome();
    void sd_getLibraries();
    SdResult sd_getFileLibrary();
    SdResult sd_getDrumkitLibrary();
    void sd_getInstLibrary();
    void sd_getSampleLibrary();
    SdResult sd_checkFile(uint8_t fileNum_);
    SdResult sd_loadFile(uint8_t fileNum_);
    SdResult sd_saveFile(uint8_t fileNum_);
    SdResult sd_clearFile(uint8_t fileNum_);
    SdResult sd_checkDrumkit(uint8_t kitNum_);
    SdResult sd_loadDrumkit(bool mode_, uint8_t kitNum_);
    SdResult sd_saveDrumkit(bool mode_, uint8_t kitNum_);
    SdResult sd_clearDrumkit(bool mode_, uint8_t kitNum_);
    SdResult sd_checkSamplesInUse();
    FRESULT sd_createDirectory(const char *path);
    FRESULT sd_deleteDirectory(const char *path);

    static inline int compareWords(const void *str1, const void *str2) {
        return strcmp(*(char **)str1, *(char **)str2);
    }

    static inline void sortWords(char *words[], int count) {
        qsort(words, count, sizeof(words[0]), compareWords);
    }

    /* Sdram functions -------------------------------------------------------*/

    void sdram_write16BitAudio(uint32_t ramAddress_, int16_t data_);
    int16_t sdram_read16BitAudio(uint32_t ramAddress_);
    void sdram_write24BitAudio(uint32_t ramAddress_, int32_t data_);
    int32_t sdram_read24BitAudio(uint32_t ramAddress_);
    void sdram_fadeOut24BitAudio(uint32_t ramAddress_, uint32_t sampleSize_, uint16_t fadeOutSize_);

    /* Lcd functions ---------------------------------------------------------*/

    void lcd_initialize();
    void lcd_test();
    void lcd_update();
    void lcd_drawLogo();
    void lcd_drawPage();
    // alert functions
    void lcd_drawAlert();
    void lcd_clearAlert();
    // limit functions
    void lcd_drawLimitAlert();
    // sd functions
    void lcd_drawSdDataIntro();
    void lcd_drawSdData();
    void lcd_clearSdData();
    void lcd_drawSdAlert(SdResult result_);
    void lcd_clearSdAlert();
    void lcd_drawInitSdAlert(SdResult result_);
    void lcd_clearInitSdAlert();
    // layer functions
    void lcd_drawLayerTab();
    void lcd_drawLayerSelect();
    void lcd_clearLayerSelect();
    // menu functions
    void lcd_drawMenuIcon(Menu menu_);
    void lcd_drawFileBox(uint8_t menuTab_, FileStatus status_);
    void lcd_clearFileBox(uint8_t menuTab_);
    void lcd_transitionMenu();
    void lcd_transitionSelect();
    void lcd_setMenuHeaderState(RGB16Color color_);
    void lcd_setMenuDataState(RGB16Color color_);
    void lcd_setMenuNumState(RGB16Color color_);
    void lcd_setMenuSignState(RGB16Color color_, LcdFont font_);
    void lcd_setMainMenuHeaderState();
    void lcd_setLayerDataState(uint8_t layerNum_);
    // main menu functions
    void lcd_drawMainMenu();
    void lcd_drawMain_TempoData();
    void lcd_drawMain_MeasureData();
    void lcd_drawMain_BarData();
    void lcd_drawMain_QuantizeData();
    void lcd_drawMain_Filter0Data();
    void lcd_drawMain_Filter1Data();
    void lcd_drawMain_Effect0Data();
    void lcd_drawMain_Effect1Data();
    // file menu functions
    void lcd_drawFileMenu();
    void lcd_drawFile_NewData();
    void lcd_drawFile_LoadData();
    void lcd_drawFile_SaveData();
    void lcd_drawFile_ClearData();
    // drumkit menu functions
    void lcd_drawDrumkitMenu();
    void lcd_drawDrumkit_NewData();
    void lcd_drawDrumkit_LoadData();
    void lcd_drawDrumkit_SaveData();
    void lcd_drawDrumkit_ClearData();
    // system menu functions
    void lcd_drawSystemMenu();
    void lcd_drawSystem_VolumeData();
    void lcd_drawSystem_PanData();
    void lcd_drawSystem_LimiterData();
    void lcd_drawSystem_MidiInData();
    void lcd_drawSystem_MidiOutData();
    void lcd_drawSystem_SyncInData();
    void lcd_drawSystem_SyncOutData();
    // rhythm menu functions
    void lcd_drawRhythmMenu();
    void lcd_drawRhythm_TempoData();
    void lcd_drawRhythm_MeasureData();
    void lcd_drawRhythm_BarData();
    void lcd_drawRhythm_QuantizeData();
    // metronome menu functions
    void lcd_drawMetroMenu();
    void lcd_drawMetro_ActiveData();
    void lcd_drawMetro_PrecountData();
    void lcd_drawMetro_SampleData();
    void lcd_drawMetro_VolumeData();
    // eq menu functions
    void lcd_drawEqMenu();
    void lcd_drawEq_ActiveData();
    void lcd_drawEq_LowShelfData();
    void lcd_drawEq_HighShelfData();
    void lcd_drawEq_PeakData(uint8_t peakNum_);
    // filter menu functions
    void lcd_drawFilterMenu(uint8_t filterNum_);
    void lcd_drawFilter_ActiveData(uint8_t filterNum_);
    void lcd_drawFilter_TypeData(uint8_t filterNum_);
    void lcd_drawFilter_FreqData(uint8_t filterNum_);
    void lcd_drawFilter_ResData(uint8_t filterNum_);
    void lcd_drawFilter_SlopeData(uint8_t filterNum_);
    void lcd_drawFilter_DryData(uint8_t filterNum_);
    void lcd_drawFilter_WetData(uint8_t filterNum_);
    // effect menu functions
    void lcd_drawEffectMenu(uint8_t effectNum_);
    void lcd_drawEffect_ActiveData(uint8_t effectNum_);
    void lcd_drawEffect_TypeData(uint8_t effectNum_);
    void lcd_drawEffect_AData(uint8_t effectNum_);
    void lcd_drawEffect_BData(uint8_t effectNum_);
    void lcd_drawEffect_CData(uint8_t effectNum_);
    void lcd_drawEffect_DData(uint8_t effectNum_);
    void lcd_drawEffect_EData(uint8_t effectNum_);
    // reverb menu functions
    void lcd_drawReverbMenu();
    void lcd_drawReverb_ActiveData();
    void lcd_drawReverb_SizeData();
    void lcd_drawReverb_DecayData();
    void lcd_drawReverb_PreDelayData();
    void lcd_drawReverb_SurroundData();
    void lcd_drawReverb_DryData();
    void lcd_drawReverb_WetData();
    // layer inst menu functions
    void lcd_drawLayerInstMenu();
    void lcd_drawLayerInst_InstData();
    void lcd_drawLayerInst_SampleData();
    void lcd_drawLayerInst_VolumeData();
    void lcd_drawLayerInst_SpeedData();
    void lcd_drawLayerInst_ReverseData();
    void lcd_drawLayerInst_NormalizeData();
    void lcd_drawTab_InstData(uint8_t layerNum_);
    // layer song menu functions
    void lcd_drawLayerSongMenu();
    void lcd_drawLayerSong_BeatFillData();
    void lcd_drawLayerSong_BeatGraphData();
    // layer menu functions
    void lcd_drawLayer_Mute(uint8_t layerNum_);
    void lcd_drawLayer_Fill(uint8_t layerNum_);
    void lcd_drawLayer_Style(uint8_t layerNum_);
    // bank functions
    void lcd_drawBank(uint8_t bankNum_, bool mode_);
    void lcd_drawBankShift();
    // transition functions
    void lcd_drawTransition();
    // play functions
    void lcd_drawPlay();
    void lcd_drawIcon();
    void lcd_drawText();
    void lcd_drawCountDown();
    void lcd_clearCountDown();
    void lcd_restartPlay();
    void lcd_resetPlay();
    void lcd_redrawPlay();
    void lcd_cleanEndPlay();
    void lcd_invertPlayColor();
    void lcd_resetPlayColor();
    // song functions
    void lcd_calculateSongX();
    void lcd_drawMeasureBar();
    void lcd_drawBeat(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_, bool selected_);
    void lcd_clearBeat(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_);
    void lcd_drawSong(uint8_t layerNum_, uint8_t bankNum);
    void lcd_clearSong(uint8_t layerNum_, uint8_t bankNum);
    void lcd_clearInterval(uint8_t layerNum_, uint8_t bankNum, uint16_t startInterval_, uint16_t endInterval_);

    /* Main functions --------------------------------------------------------*/

    void main_select();

    /* File functions --------------------------------------------------------*/

    void file_select();

    void file_menuRight();
    void file_menuLeft();
    void file_menuUp();
    void file_menuDown();

    void file_newSelect();
    void file_newAction();
    void file_loadSelect();
    void file_loadAction();
    void file_saveSelect();
    void file_saveAction();
    void file_clearSelect();
    void file_clearAction();

    /* Drumkit functions -----------------------------------------------------*/

    void drumkit_select();

    void drumkit_menuRight();
    void drumkit_menuLeft();
    void drumkit_menuUp();
    void drumkit_menuDown();

    void drumkit_newSelect();
    void drumkit_newAction();
    void drumkit_loadSelect();
    void drumkit_loadAction();
    void drumkit_saveSelect();
    void drumkit_saveAction();
    void drumkit_clearSelect();
    void drumkit_clearAction();

    /* System functions ------------------------------------------------------*/

    void system_select();
    void system_reset();

    void system_menuRight();
    void system_menuLeft();
    void system_menuUp();
    void system_menuDown();

    void system_setVolume(uint8_t volume_);
    void system_setPan(uint8_t pan_);
    void system_setLimiter(bool mode_);
    void system_setMidiIn(uint8_t mode_);
    void system_setMidiOut(uint8_t mode_);
    void system_setSyncIn(uint8_t mode_);
    void system_setSyncOut(uint8_t mode_);

    void system_volumeTransition(float volumeFloat_);
    void system_panTransition(float volumeLeftFloat_, float volumeRightFloat_);
    void system_calculateVolumeTransition();
    void system_calculatePanTransition();

    /* Rhythm functions ------------------------------------------------------*/

    void preMenuLayerClear();

    void rhythm_select();
    void rhythm_reset();

    void rhythm_menuRight();
    void rhythm_menuLeft();
    void rhythm_menuUp();
    void rhythm_menuDown();

    void rhythm_setTempo(uint8_t tempo_);
    void rhythm_setMeasure(uint8_t measure_);
    void rhythm_setBar(uint8_t bar_);
    void rhythm_setQuantize(uint8_t quantize_);

    /* Metronome functions ---------------------------------------------------*/

    void metro_select();
    void metro_reset();

    void metro_menuRight();
    void metro_menuLeft();
    void metro_menuUp();
    void metro_menuDown();

    void metro_setActive(bool active_);
    void metro_setPrecount(bool precount_);
    void metro_setSample(uint8_t sample_);
    void metro_setVolume(uint8_t volume_);

    void metro_volumeTransition(float volumeFloat);
    void metro_calculateVolumeTransition();

    /* Eq functions ----------------------------------------------------------*/

    void eq_select();
    void eq_reset();

    void eq_menuRight();
    void eq_menuLeft();
    void eq_menuUp();
    void eq_menuDown();

    void eq_setActive(bool active_);
    void eq_setFreqLowShelf(uint8_t freq_);
    void eq_setGainLowShelf(uint8_t gain_);
    void eq_setFreqHighShelf(uint8_t freq_);
    void eq_setGainHighShelf(uint8_t gain_);
    void eq_setFreqPeak(uint8_t peakNum_, uint8_t freq_);
    void eq_setGainPeak(uint8_t peakNum_, uint8_t gain_);
    void eq_setQPeak(uint8_t peakNum_, uint8_t q_);

    void eq_genTransition(EqTransitionMode mode_, bool activeActive_, bool targetActive_);
    void eq_calculateActiveTransition();

    /* Filter functions ------------------------------------------------------*/

    void filter_select();
    void filter_reset(uint8_t filterNum_);

    void filter_menuRight();
    void filter_menuLeft();
    void filter_menuUp();
    void filter_menuDown();

    void filter_setActive(uint8_t filterNum_, bool active_);
    void filter_setType(uint8_t filterNum_, uint8_t type_);
    void filter_setFreq(uint8_t filterNum_, uint8_t freq_);
    void filter_setRes(uint8_t filterNum_, uint8_t res_);
    void filter_setSlope(uint8_t filterNum_, uint8_t slope_);
    void filter_setDry(uint8_t filterNum_, uint8_t dry_);
    void filter_setWet(uint8_t filterNum_, uint8_t wet_);

    void filter_genTransition(uint8_t filterNum_, FilterTransitionMode mode_, bool activeActive_, bool targetActive_, uint8_t activeType_, uint8_t targetType_);
    void filter_mixTransition(uint8_t filterNum_, float dryFloat, float wetFloat);
    void filter_calculateGenTransition(uint8_t filterNum_);
    void filter_calculateMixTransition(uint8_t filterNum_);

    /* Effect functions ------------------------------------------------------*/

    void effect_select();
    void effect_reset(uint8_t effectNum_);

    void effect_menuRight();
    void effect_menuLeft();
    void effect_menuUp();
    void effect_menuDown();

    void effect_setActive(uint8_t effectNum_, bool active_);
    void effect_setType(uint8_t effectNum_, uint8_t type_);
    void effect_setAData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t aData_);
    void effect_setBData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t bData_);
    void effect_setCData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t cData_);
    void effect_setDData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t dData_);
    void effect_setEData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t eData_);

    void effect_genTransition(uint8_t effectNum_, EffectTransitionMode mode_, bool activeActive_, bool targetActive_, uint8_t activeType_, uint8_t targetType_);
    void effect_mixTransition(uint8_t effectNum_, float dryFloat_, float wetFloat_);
    void effect_calculateGenTransition(uint8_t effectNum_);
    void effect_calculateMixTransition(uint8_t effectNum_);

    void effect_cleanMemory(uint8_t effectNum_, uint8_t type_);

    /* Reverb functions ------------------------------------------------------*/

    void reverb_select();
    void reverb_reset();

    void reverb_menuRight();
    void reverb_menuLeft();
    void reverb_menuUp();
    void reverb_menuDown();

    void reverb_setActive(bool active_);
    void reverb_setSize(uint8_t size_);
    void reverb_setDecay(uint8_t decay_);
    void reverb_setPreDelay(uint8_t preDelay_);
    void reverb_setSurround(uint8_t surround_);
    void reverb_setDry(uint8_t dry_);
    void reverb_setWet(uint8_t wet_);

    void reverb_genTransition(ReverbTransitionMode mode_, bool activeActive_, bool targetActive_);
    void reverb_mixTransition(float dryFloat, float wetFloat);
    void reverb_calculateGenTransition();
    void reverb_calculateMixTransition();

    /* Layer functions -------------------------------------------------------*/

    void layer_setTab(LayerTab layerTab_);
    void layer_playBeat(uint8_t layerNum_);

    /* Layer Inst functions --------------------------------------------------*/

    void layerInst_select(uint8_t layerNum_);
    void layerInst_reset(uint8_t layerNum_);

    void layerInst_menuRight();
    void layerInst_menuLeft();
    void layerInst_menuUp();
    void layerInst_menuDown();

    void layerInst_setInstSelected(uint8_t layerNum_, int16_t inst_);
    void layerInst_setInstLoaded(uint8_t layerNum_);
    void layerInst_setSampleSelected(uint8_t layerNum_, int16_t sample_);
    void layerInst_setSampleLoaded(uint8_t layerNum_);
    void layerInst_setVolume(uint8_t layerNum_, uint8_t volume_);
    void layerInst_setSpeed(uint8_t layerNum_, uint8_t speed_);
    void layerInst_setReverse(uint8_t layerNum_, bool reverse_);
    void layerInst_setNormalize(uint8_t layerNum_, bool normalize_);

    void layerInst_setMute(uint8_t layerNum_, bool mute_);
    void layerInst_setFill(uint8_t layerNum_, bool fill_);
    void layerInst_setStyle(uint8_t layerNum_, bool style_);

    /* Layer Song functions --------------------------------------------------*/

    void layerSong_select(uint8_t layerNum_, uint8_t bankNum_);
    void layerSong_reset(uint8_t layerNum_, uint8_t bankNum_);

    void layerSong_menuRight();
    void layerSong_menuLeft();
    void layerSong_menuUp();
    void layerSong_menuDown();

    void layerSong_setBeat(uint8_t layerNum_, uint8_t bankNum_, uint16_t interval_, uint8_t fill_);
    void layerSong_setBeatFill(uint8_t fill_);
    void layerSong_generateBeat(uint8_t type);
    void layerSong_resetSelectedBeat();
    void layerSong_resetBeats(uint8_t layerNum_, uint8_t bankNum_, uint16_t startInterval_);
    void layerSong_resetAllBeats(uint8_t layerNum_, uint8_t bankNum_);
    void layerSong_quantizeActiveBeats(uint8_t layerNum_, uint8_t bankNum_);
    void layerSong_arrangeActiveBeats(uint8_t layerNum_, uint8_t bankNum_, bool duplicate_, bool collect_, bool sort_, bool lastFill_);
    void layerSong_calculateLastActiveBeatNum(uint8_t layerNum_, uint8_t bankNum_);
    void layerSong_calculatePlayBeatNum(uint8_t layerNum_, uint8_t bankNum_, uint16_t playInterval_);
    void layerSong_calculateBeatFill(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_, uint8_t fill_);

    /* Bank functions --------------------------------------------------------*/

    void bank_select(uint8_t bank_);
    void bank_trigger(uint8_t bank_);

    /* Play functions --------------------------------------------------------*/

    void record();
    void play();
    void stop();
    void reset();

    void triggerStop();
    void triggerReset();

    /* Interrupt functions ---------------------------------------------------*/

    void interruptPlay();

    void interruptAudioMetronome();
    void interruptAudioSong();
    void interruptAudioLpf();
    void interruptAudioEq();
    void interruptAudioFilter();
    void interruptAudioEffect();
    void interruptAudioReverb();
    void interruptAudioSend();

    int32_t processAudioFilter(uint8_t filterNum_, int32_t audio_);
    int32_t processAudioEffect(uint8_t effectNum_, int32_t audio_);

    void interruptTransition();

    void interruptLeftButtonTrigger();
    void interruptRightButtonTrigger();
    void interruptLeftButtonRead();
    void interruptRightButtonRead();
    void interruptUpDownButtonRead();
    void interruptLongButtonRead();
    void interruptPowerButtonRead();

    void interruptText();
    void interruptSd();
    void interruptBeatSync();
    void interruptLimitAlert();

    /* Debug functions -------------------------------------------------------*/

    void check(int32_t num_, uint8_t line_) {
        lcd.setAlignment(LEFT);
        lcd.setFont(FONT_05x07);
        lcd.setForeColor(WHITE);
        lcd.setBackColor(BLACK);
        lcd.drawNumber(num_, 8, 30, 30 + (line_ * 10));
    }
};

#endif
