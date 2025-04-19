#include "Controller.h"

Controller::Controller()
    : lcd(LANDSCAPE_1, WHITE, BLACK, FONT_07x09),
      dac(),
      sd(),
      keyboard(),

      rhythm(),
      metronome(),
      eq(),
      filter{{0}, {1}},
      effect{{0}, {1}},
      reverb(),

      audioMetronome(0),
      audioSong(0),
      audioLpf(0),
      audioEq(0),
      audioFilter(0),
      audioEffect(0),
      audioReverb_L(0),
      audioReverb_R(0),
      audioSend_L(0),
      audioSend_R(0),

      layerLibrary{{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}},

      midiTxBusy(false),

      selectedLayerNum(-1),
      selectedBeatNum(-1),

      activeBankNum(0),
      targetBankNum(0),
      bankShiftFlag(false),
      bankActionFlag(false),

      menu(INIT_MENU),
      preMenu(INIT_MENU),
      menuTab(-1),
      preMenuTab(-1),
      preFill(-1),
      subMenuTab(-1),

      layerTab(TAB_0),

      fileLibrarySize(0),
      drumkitLibrarySize(0),
      instLibrarySize(0),
      sampleLibrarySize{},
      totalSampleLibrarySize(0),

      songInterval(kMeasureInterval * kInitialMeasure * kInitialBar),
      barInterval(kMeasureInterval * kInitialMeasure),
      measureInterval(kMeasureInterval),
      playInterval(0),
      stopInterval(0),
      resetInterval(0),

      stopFlag(false),
      resetFlag(false),

      alertFlag(false),
      alertType(ALERT_OFF),

      sdInsertCheck(false),

      resetPlayFlag(false),

      fileStatus(FILE_NONE),
      drumkitStatus(FILE_NONE),

      playActive(false),
      recordActive(false),

      triggerMode(TRIG_BAR),

      playTimerPeriod(0),
      measureTimerPeriod(0),

      playX(0),
      playXRatio((float)songInterval / kPlayWidth),
      playColor(kPlayColor0),

      power(true),

      powerButtonFlag(false),
      powerButtonCounter(0),

      mainMenuButtonFlag(false),

      upButtonFlag(false),
      downButtonFlag(false),
      upDownButtonCounter(0),

      layerBeatButtonFlag(false),
      layerClearButtonFlag(false),

      layerInstCopyButtonFlag(false),
      layerInstPasteButtonFlag(false),

      layerFillCopyButtonFlag(false),
      layerFillPasteButtonFlag(false),

      layerSongCopyButtonFlag(false),
      layerSongPasteButtonFlag(false),

      layerInstPasteActionFlag(false),
      layerSongPasteActionFlag(false),

      rhythmUnlockFlag(false),
      rhythmLockFlag(false),

      textCopyFlag(false),
      textPasteFlag(false),
      textClearFlag(false),

      limitAlertShowFlag(false),
      limitAlertClearFlag(false),

      copyLayerInstNum(-1),
      copyLayerFillNum(-1),
      copyLayerSongNum(-1),
      copyBankSongNum(-1),

      layerBeatButtonStage(0),

      fileMenuCounter(-1),
      drumkitMenuCounter(-1),

      transitionShowFlag(0),
      transitionClearFlag(false),

      animation(true),
      textShow(false),

      sampleFadeOut(true) {}

Controller::~Controller() {}

void Controller::initialize() {
    lpf.initialize();
    eq.initialize();
    filter[0].initialize();
    filter[1].initialize();
    effect[0].initialize();
    effect[1].initialize();
    reverb.initialize();

    filter_reset(0);
    filter_reset(1);

    effect_setDData(0, EF_DELAY, kInitialDelayDry);
    effect_setEData(0, EF_DELAY, kInitialDelayWet);
    effect_setDData(1, EF_DELAY, kInitialDelayDry);
    effect_setEData(1, EF_DELAY, kInitialDelayWet);

    keyboard_initialize();
    dac_initialize();
    lcd_initialize();

    startTransitionTimer();
    stopTransitionTimer();

    startTextTimer();
    stopTextTimer();

    startPowerButtonTimer();
    stopPowerButtonTimer();

    startBeatSyncTimer();
    stopBeatSyncTimer();

    startLimitAlertTimer();
    stopLimitAlertTimer();

    LED0_ON;
    LED1_ON;
    LED2_ON;
}

void Controller::systemStart() {
    lcd.displayOn();
    dac.audioOn();

    HAL_Delay(500);

    sd_initialize();

    menu = MAIN_MENU;

    lcd_drawLogo();
    lcd_drawPage();

    updatePlayTimerPeriod();

    startSdTimer();
    startTransitionTimer();

    HAL_HalfDuplex_EnableTransmitter(&huart4);
    HAL_UART_Receive_DMA(&huart7, syncInData, 2);

    HAL_HalfDuplex_EnableReceiver(&huart1);
    HAL_HalfDuplex_EnableTransmitter(&huart6);

    HAL_UART_Receive_IT(&huart1, &midiRxData, 1);

    HAL_Delay(100);
}

void Controller::systemReset() {
    lcd.displayOff();
    HAL_Delay(1000);
    dac.audioOff();
    NVIC_SystemReset();
}

void Controller::systemUpdate_A() {
    keyboard_check_A();
}

void Controller::systemUpdate_B() {
    keyboard_check_B();
    lcd_update();

    if (!sd.ready) {
        sd_reinitialize();
    }

    if (sd.getLibrary) {
        sd.getLibrary = false;
        sd_getLibraries();
    }

    if (layerInstPasteActionFlag) {
        layerInstPasteActionFlag = false;
        layerInst_setInstSelected(selectedLayerNum, layerLibrary[copyLayerInstNum].instLoaded);
        layerInst_setInstLoaded(selectedLayerNum);
        layerInst_setSampleSelected(selectedLayerNum, layerLibrary[copyLayerInstNum].sampleLoaded);
        layerInst_setSampleLoaded(selectedLayerNum);
        layerInst_setVolume(selectedLayerNum, layerLibrary[copyLayerInstNum].volume);
        layerInst_setSpeed(selectedLayerNum, layerLibrary[copyLayerInstNum].speed);
        layerInst_setReverse(selectedLayerNum, layerLibrary[copyLayerInstNum].reverse);
        layerInst_setNormalize(selectedLayerNum, layerLibrary[copyLayerInstNum].normalize);
    }

    if (layerSongPasteActionFlag) {
        layerSongPasteActionFlag = false;
        for (uint8_t i = 0; i <= layerLibrary[copyLayerSongNum].bankLibrary[copyBankSongNum].lastActiveBeatNum; i++) {
            layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].beatLibrary[i] = layerLibrary[copyLayerSongNum].bankLibrary[copyBankSongNum].beatLibrary[i];
        }
        selectedBeatNum = 0;
        layerSong_calculateLastActiveBeatNum(selectedLayerNum, activeBankNum);
        layerSong_calculatePlayBeatNum(selectedLayerNum, activeBankNum, playInterval);

        lcd_drawLayerSong_BeatFillData();
        lcd_drawLayerSong_BeatGraphData();
        lcd_drawSong(selectedLayerNum, activeBankNum);
    }

    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        if ((system.midi.txActive) && (system.midi.txTriggerNoteOn[i]) && (!midiTxBusy)) {
            if (sendMidiCommand(0x90 + system.midi.txChannel, system.midi.txData[i], 0x64)) {
                system.midi.txTriggerNoteOn[i] = false;
            }
        }

        if ((system.midi.txActive) && (system.midi.txTriggerNoteOff[i]) && (!midiTxBusy)) {
            if (sendMidiCommand(0x80 + system.midi.txChannel, system.midi.txData[i], 0x64)) {
                system.midi.txTriggerNoteOff[i] = false;
            }
        }
    }

    if (system.midi.rxNoteOnWriteFlag == 1) {
        uint8_t layerNum = system.midi.rxNoteOnKey % 12;
        if (layerNum < kLayerLibrarySize)
            layer_playBeat(layerNum);
        system.midi.rxNoteOnWriteFlag = 0;
    }

    if (system.midi.rxNoteOffWriteFlag == 1) {
        system.midi.rxNoteOffWriteFlag = 0;
    }
}

bool Controller::sendMidiCommand(uint8_t command_, uint8_t data0_, uint8_t data1_) {
    if (!midiTxBusy) {
        uint8_t dataSend[3] = {command_, data0_, data1_};
        midiTxBusy = true;
        HAL_UART_Transmit_DMA(&huart6, dataSend, 3);
        return true;
    } else {
        return false;
    }
}

void Controller::receiveMidiCommand() {
    if (system.midi.rxActive) {
        uint8_t rxData = midiRxData;
        // note on
        if ((system.midi.rxNoteOnReadStage == 0) && (rxData == (0x90 + system.midi.rxChannel))) {
            system.midi.rxNoteOnReadStage = 1;
        } else if (system.midi.rxNoteOnReadStage == 1) {
            system.midi.rxNoteOnKey = rxData;
            system.midi.rxNoteOnReadStage = 2;
        } else if (system.midi.rxNoteOnReadStage == 2) {
            system.midi.rxNoteOnVelocity = rxData;
            system.midi.rxNoteOnReadStage = 0;
            system.midi.rxNoteOffVelocity = rxData;
            if (system.midi.rxNoteOffVelocity == 0) {
                // Treat as note off
                system.midi.rxNoteOffKey = system.midi.rxNoteOnKey;
                system.midi.rxNoteOffVelocity = 0;
                system.midi.rxNoteOffWriteFlag = 1;
            } else {
                // Normal note on
                system.midi.rxNoteOnWriteFlag = 1;
            }
        }
    }
}

bool Controller::sendSyncCommand(uint8_t data_) {
    syncOutData[0] = 100;
    syncOutData[1] = data_;

    // check(syncOutData[0], 0);
    // check(syncOutData[1], 1);

    while (HAL_UART_GetState(&huart4) != HAL_UART_STATE_READY) {
    }
    if (HAL_UART_Transmit_DMA(&huart4, syncOutData, sizeof(syncOutData)) == HAL_OK) {
        return true;
    } else {
        return false;
    }
}

void Controller::receiveSyncCommand() {
    if ((system.syncIn) && (syncInData[0] == 100) && (syncInData[1])) {
        switch (syncInData[1]) {
        case SYNC_RESET:
            reset();
            break;

        case SYNC_TRIG_RESET:
            triggerReset();
            break;

        case SYNC_STOP:
            stop();
            break;

        case SYNC_TRIG_STOP:
            triggerStop();
            break;

        case SYNC_PLAY:
            play();
            break;

        case SYNC_RECORD:
            record();
            break;

        default:
            if ((syncInData[1] >= kMinTempo) && (syncInData[1] <= kMaxTempo)) {
                rhythm_setTempo(syncInData[1]);
            }
            break;
        }
    }

    // check(syncInData[0], 4);
    // check(syncInData[1], 5);

    syncInData[0] = 0x00;
    syncInData[1] = 0x00;
}

void Controller::calculateSongInterval() {
    songInterval = kMeasureInterval * rhythm.measure * rhythm.bar;
    barInterval = kMeasureInterval * rhythm.measure;
    measureInterval = kMeasureInterval;
    rhythm.measureTotal = rhythm.measure * rhythm.bar;
    metronome.precounterMax = barInterval;
}

uint16_t Controller::calculateTriggerInterval() {
    uint16_t interval;
    switch (triggerMode) {
    case TRIG_MEASURE:
        interval = ((playInterval / measureInterval + 1) * measureInterval) - 1;
        break;

    case TRIG_BAR:
        interval = ((playInterval / barInterval + 1) * barInterval) - 1;
        break;

    case TRIG_SONG:
        interval = songInterval - 1;
        break;
    }
    return interval;
}

void Controller::adjustMeasureBarTiming() {
    calculateSongInterval();
    // clear beat if its time exceeds song interval
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        for (uint8_t j = 0; j < kBankLibrarySize; j++) {
            layerSong_resetBeats(i, j, songInterval);
        }
    }
}

void Controller::updatePlayTimerPeriod() {
    playTimerPeriod = kMicroSecondsinOneMinute / (rhythm.tempo * kMeasureInterval);
    measureTimerPeriod = 2646000 / rhythm.tempo;
    __HAL_TIM_SET_AUTORELOAD(&htim14, playTimerPeriod - 1);
}

/* Button functions --------------------------------------------------------*/

void Controller::button_check() {}

/* Keyboard functions --------------------------------------------------------*/

void Controller::keyboard_initialize() {
    CT0_SCL_HIGH;
    CT1_SCL_HIGH;
}

void Controller::keyboard_check_A() {
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

void Controller::keyboard_check_B() {
    // left button
    if (keyboard.leftButton >= 0) {
        // LED0_TOGGLE;
        // check(keyboard.leftButton, 0);
        switch (keyboard.leftButton) {
        case KEY_RESET:
            if ((!alertFlag) && (!system.sync.slaveMode)) {
                (playActive) ? triggerReset() : reset();
            }
            break;

        case KEY_PLAYSTOP:
            if ((!alertFlag) && (!system.sync.slaveMode)) {
                ((playActive) && (!stopFlag)) ? triggerStop() : play();
            }
            break;

        case KEY_REC:
            if ((!alertFlag) && (!system.sync.slaveMode)) {
                record();
            }
            break;

        case KEY_UP:
            if (!alertFlag) {
                upDownButtonCounter = 0;
                upButtonFlag = true;
                startUpDownButtonTimer();
                switch (menu) {
                case FILE_MENU:
                    file_menuUp();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuUp();
                    break;

                case SYSTEM_MENU:
                    system_menuUp();
                    break;

                case RHYTHM_MENU:
                    rhythm_menuUp();
                    break;

                case METRO_MENU:
                    metro_menuUp();
                    break;

                case EQ_MENU:
                    eq_menuUp();
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    filter_menuUp();
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    effect_menuUp();
                    break;

                case REVERB_MENU:
                    reverb_menuUp();
                    break;

                case LAYER_INST_MENU:
                    layerInst_menuUp();
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuUp();
                    break;
                }
            }
            break;

        case KEY_DOWN:
            if (!alertFlag) {
                upDownButtonCounter = 0;
                downButtonFlag = true;
                startUpDownButtonTimer();
                switch (menu) {
                case FILE_MENU:
                    file_menuDown();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuDown();
                    break;

                case SYSTEM_MENU:
                    system_menuDown();
                    break;

                case RHYTHM_MENU:
                    rhythm_menuDown();
                    break;

                case METRO_MENU:
                    metro_menuDown();
                    break;

                case EQ_MENU:
                    eq_menuDown();
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    filter_menuDown();
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    effect_menuDown();
                    break;

                case REVERB_MENU:
                    reverb_menuDown();
                    break;

                case LAYER_INST_MENU:
                    layerInst_menuDown();
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuDown();
                    break;
                }
            }
            break;

        case KEY_LEFT:
            if (!alertFlag) {
                switch (menu) {
                case FILE_MENU:
                    file_menuLeft();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuLeft();
                    break;

                case SYSTEM_MENU:
                    system_menuLeft();
                    break;

                case RHYTHM_MENU:
                    rhythm_menuLeft();
                    break;

                case METRO_MENU:
                    metro_menuLeft();
                    break;

                case EQ_MENU:
                    eq_menuLeft();
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    filter_menuLeft();
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    effect_menuLeft();
                    break;

                case REVERB_MENU:
                    reverb_menuLeft();
                    break;

                case LAYER_INST_MENU:
                    layerInst_menuLeft();
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuLeft();
                    break;
                }
            }
            break;

        case KEY_RIGHT:
            if (!alertFlag) {
                switch (menu) {
                case FILE_MENU:
                    file_menuRight();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuRight();
                    break;

                case SYSTEM_MENU:
                    system_menuRight();
                    break;

                case RHYTHM_MENU:
                    rhythm_menuRight();
                    break;

                case METRO_MENU:
                    metro_menuRight();
                    break;

                case EQ_MENU:
                    eq_menuRight();
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    filter_menuRight();
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    effect_menuRight();
                    break;

                case REVERB_MENU:
                    reverb_menuRight();
                    break;

                case LAYER_INST_MENU:
                    layerInst_menuRight();
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuRight();
                    break;
                }
            }
            break;

        case KEY_CENTER:
            mainMenuButtonFlag = true;
            startLongButtonTimer();
            break;

        case KEY_ADD:
            if (alertFlag) {
                alertFlag = false;
                switch (alertType) {
                case ALERT_MEASUREUP:
                    lcd_clearAlert();
                    rhythm_setMeasure(rhythm.measure + 1);
                    break;

                case ALERT_MEASUREDOWN:
                    lcd_clearAlert();
                    rhythm_setMeasure(rhythm.measure - 1);
                    break;

                case ALERT_BARUP:
                    lcd_clearAlert();
                    rhythm_setBar(rhythm.bar + 1);
                    break;

                case ALERT_BARDOWN:
                    lcd_clearAlert();
                    rhythm_setBar(rhythm.bar - 1);
                    break;

                case ALERT_QUANTIZEUP:
                    lcd_clearAlert();
                    rhythm_setQuantize(rhythm.quantize + 1);
                    break;

                case ALERT_QUANTIZEDOWN:
                    lcd_clearAlert();
                    rhythm_setQuantize(rhythm.quantize - 1);
                    break;

                case ALERT_NEWFILE:
                    file_newAction();
                    break;

                case ALERT_LOADFILE:
                    file_loadAction();
                    break;

                case ALERT_SAVEFILE:
                case ALERT_OVERWRITEFILE:
                    file_saveAction();
                    break;

                case ALERT_CLEARFILE:
                    file_clearAction();
                    break;

                case ALERT_NEWDRUMKIT:
                    drumkit_newAction();
                    break;

                case ALERT_LOADDRUMKIT:
                    drumkit_loadAction();
                    break;

                case ALERT_SAVEDRUMKIT:
                case ALERT_OVERWRITEDRUMKIT:
                    drumkit_saveAction();
                    break;

                case ALERT_CLEARDRUMKIT:
                    drumkit_clearAction();
                    break;
                }
            } else if (keyboard.muteButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (!layerLibrary[i].mute)
                        layerInst_setMute(i, true);
                }
            } else if (keyboard.fillButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (!layerLibrary[i].fill)
                        layerInst_setFill(i, true);
                }
            } else if (keyboard.styleButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (!layerLibrary[i].style)
                        layerInst_setStyle(i, true);
                }
            } else if ((menu == LAYER_SONG_MENU) && (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum == -1)) {
                layerBeatButtonFlag = true;
                layerBeatButtonStage = 0;
                startLongButtonTimer();
            } else if (menu == FILE_MENU) {
                switch (menuTab) {
                case 0:
                    file_newSelect();
                    break;

                case 1:
                    file_loadSelect();
                    break;

                case 2:
                    file_saveSelect();
                    break;

                case 3:
                    file_clearSelect();
                    break;
                }
            } else if (menu == DRUMKIT_MENU) {
                switch (menuTab) {
                case 0:
                    drumkit_newSelect();
                    break;

                case 1:
                    drumkit_loadSelect();
                    break;

                case 2:
                    drumkit_saveSelect();
                    break;

                case 3:
                    drumkit_clearSelect();
                    break;
                }
            } else if (menu == RHYTHM_MENU) {
                rhythmLockFlag = true;
                startLongButtonTimer();
            } else if (menu == LAYER_INST_MENU) {
                if (!((strcmp(layerLibrary[selectedLayerNum].instSelectedData.fileName, layerLibrary[selectedLayerNum].instLoadedData.fileName) == 0) && (strcmp(layerLibrary[selectedLayerNum].sampleSelectedData.fileName, layerLibrary[selectedLayerNum].sampleLoadedData.fileName) == 0))) {
                    layerInst_setInstLoaded(selectedLayerNum);
                    layerInst_setSampleLoaded(selectedLayerNum);
                }
            }
            break;

        case KEY_ERASE:
            if (alertFlag) {
                lcd_clearAlert();
                alertFlag = false;
            } else if (keyboard.muteButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (layerLibrary[i].mute)
                        layerInst_setMute(i, false);
                }
            } else if (keyboard.fillButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (layerLibrary[i].fill)
                        layerInst_setFill(i, false);
                }
            } else if (keyboard.styleButtonPress) {
                for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                    if (layerLibrary[i].style)
                        layerInst_setStyle(i, false);
                }
            } else if ((menu == LAYER_SONG_MENU) &&
                       (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1)) {
                layerClearButtonFlag = true;
                startLongButtonTimer();
            } else if (menu == RHYTHM_MENU) {
                rhythmUnlockFlag = true;
                startLongButtonTimer();
            }
            break;

        case KEY_COPY:
            if (menu == LAYER_INST_MENU) {
                layerInstCopyButtonFlag = true;
                startLongButtonTimer();
            } else if ((menu == LAYER_SONG_MENU) && (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1)) {
                layerFillCopyButtonFlag = true;
                layerSongCopyButtonFlag = true;
                startLongButtonTimer();
            }
            break;

        case KEY_PASTE:
            if (menu == LAYER_INST_MENU) {
                layerInstPasteButtonFlag = true;
                startLongButtonTimer();
            } else if (menu == LAYER_SONG_MENU) {
                layerFillPasteButtonFlag = true;
                layerSongPasteButtonFlag = true;
                startLongButtonTimer();
            }
            break;

        case KEY_POWER:
            powerButtonFlag = true;
            powerButtonCounter = 0;
            startPowerButtonTimer();
            break;

        case KEY_TAB_0:
            if (keyboard.muteButtonPress) {
                for (uint8_t i = 0; i < 5; i++) {
                    layerInst_setMute(i, !layerLibrary[i].mute);
                }
            } else if (keyboard.fillButtonPress) {
                for (uint8_t i = 0; i < 5; i++) {
                    layerInst_setFill(i, !layerLibrary[i].fill);
                }
            } else if (keyboard.styleButtonPress) {
                for (uint8_t i = 0; i < 5; i++) {
                    layerInst_setStyle(i, !layerLibrary[i].style);
                }
            } else {
                layer_setTab(TAB_0);
            }
            break;

        case KEY_TAB_1:
            if (keyboard.muteButtonPress) {
                for (uint8_t i = 5; i < 10; i++) {
                    layerInst_setMute(i, !layerLibrary[i].mute);
                }
            } else if (keyboard.fillButtonPress) {
                for (uint8_t i = 5; i < 10; i++) {
                    layerInst_setFill(i, !layerLibrary[i].fill);
                }
            } else if (keyboard.styleButtonPress) {
                for (uint8_t i = 5; i < 10; i++) {
                    layerInst_setStyle(i, !layerLibrary[i].style);
                }
            } else {
                layer_setTab(TAB_1);
            }
            break;

        case LEFT_RELEASE:
            if (powerButtonFlag) {
                stopPowerButtonTimer();
                powerButtonFlag = false;
                powerButtonCounter = 0;
            } else if (upButtonFlag) {
                stopUpDownButtonTimer();
                upButtonFlag = false;
                upDownButtonCounter = 0;
            } else if (downButtonFlag) {
                stopUpDownButtonTimer();
                downButtonFlag = false;
                upDownButtonCounter = 0;
            } else if (layerBeatButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerBeatButtonFlag = false;
                layerBeatButtonStage = 0;
            } else if (layerClearButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerClearButtonFlag = false;
                layerSong_resetSelectedBeat();
            } else if (layerInstCopyButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerInstCopyButtonFlag = false;
            } else if (layerInstPasteButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = false;
                layerInstPasteButtonFlag = false;
            } else if (layerFillCopyButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                if (layerSongCopyButtonFlag) {
                    layerFillCopyButtonFlag = false;
                    layerSongCopyButtonFlag = false;
                    if (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1) {
                        copyLayerFillNum = layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].beatLibrary[selectedBeatNum].getFill();
                    }
                } else {
                    layerFillCopyButtonFlag = false;
                }
            } else if (layerFillPasteButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                if (layerSongPasteButtonFlag) {
                    if ((copyLayerFillNum != -1) && (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1)) {
                        layerSong_setBeatFill(copyLayerFillNum);
                    }
                    layerFillPasteButtonFlag = false;
                    layerSongPasteButtonFlag = false;
                } else {
                    layerFillPasteButtonFlag = false;
                }
            } else if (mainMenuButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                mainMenuButtonFlag = false;
                if (menu == EQ_MENU) {
                    switch (subMenuTab) {
                    case 0:
                        subMenuTab = 1;
                        break;

                    case 1:
                        if ((menuTab == 2) || (menuTab == 7)) {
                            subMenuTab = 0;
                        } else {
                            subMenuTab = 2;
                        }
                        break;

                    case 2:
                        subMenuTab = 0;
                        break;

                    default:
                        break;
                    }
                    lcd_transitionSelect();
                }
            } else if (rhythmUnlockFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                rhythmUnlockFlag = false;
            } else if (rhythmLockFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                rhythmLockFlag = false;
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
        switch (keyboard.rightButton) {
        case KEY_FILE:
            if (!alertFlag)
                file_select();
            break;

        case KEY_DRUMKIT:
            if (!alertFlag)
                drumkit_select();
            break;

        case KEY_SYSTEM:
            if (!alertFlag)
                system_select();
            break;

        case KEY_RHYTHM:
            if (!alertFlag)
                rhythm_select();
            break;

        case KEY_METRONOME:
            if (!alertFlag)
                metro_select();
            break;

        case KEY_EQ:
            if (!alertFlag)
                eq_select();
            break;

        case KEY_FILTER:
            if (!alertFlag)
                filter_select();
            break;

        case KEY_EFFECT:
            if (!alertFlag)
                effect_select();
            break;

        case KEY_REVERB:
            if (!alertFlag)
                reverb_select();
            break;

        case KEY_MUTE:
            keyboard.muteButtonPress = true;
            break;

        case KEY_FILL:
            keyboard.fillButtonPress = true;
            break;

        case KEY_STYLE:
            keyboard.styleButtonPress = true;
            break;

        case KEY_MODE_INST:
            keyboard.instButtonPress = true;
            break;

        case KEY_MODE_SONG:
            keyboard.songButtonPress = true;
            break;

        case KEY_BANK:
            keyboard.bankButtonPress = true;
            break;

        case RIGHT_RELEASE:
            keyboard.muteButtonPress = false;
            keyboard.fillButtonPress = false;
            keyboard.styleButtonPress = false;
            keyboard.instButtonPress = false;
            keyboard.songButtonPress = false;
            keyboard.bankButtonPress = false;
            break;

        default:
            break;
        }
        keyboard.rightButton = -1;
    }

    // layer buttons
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        if (keyboard.layerButtonPress[i]) {
            // LED2_TOGGLE;
            if (keyboard.muteButtonPress) {
                layerInst_setMute(i, !layerLibrary[i].mute);
            } else if (keyboard.fillButtonPress) {
                layerInst_setFill(i, !layerLibrary[i].fill);
            } else if (keyboard.styleButtonPress) {
                layerInst_setStyle(i, !layerLibrary[i].style);
            } else if (keyboard.instButtonPress) {
                layerInst_select(i);
            } else if (keyboard.songButtonPress) {
                layerSong_select(i, activeBankNum);
            } else if (keyboard.bankButtonPress) {
                (playActive) ? bank_trigger(i % 5) : bank_select(i % 5);
            } else {
                layer_playBeat(i);
            }
            keyboard.layerButtonPress[i] = false;
        }
    }
}

void Controller::keyboard_check_C() {
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

void Controller::keyboard_enable() {
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Controller::keyboard_disable() {
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI2_IRQn);
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}

/* Dac functions -------------------------------------------------------------*/

void Controller::dac_initialize() {
    dac.initialize();
}

/* Sd functions --------------------------------------------------------------*/

SdResult Controller::sd_initialize() {
    SdResult sdResult = SD_ERROR;

    while (sdResult != SD_OK) {
        if (sd_detect() == SD_OK) {
            sd.detect = true;
            if (sdInsertCheck) {
                lcd.drawInitSdReadAlert();
                sdInsertCheck = false;
            }
            FATFS_UnLinkDriver(SDPath);
            FATFS_LinkDriver(&SD_Driver, SDPath);
            if ((sd_mount() == SD_OK) && (sd_getLabel() == SD_OK) && (sd_getSpace() == SD_OK)) {
                sd.serialTemp = sd.serial;
                if (sd_checkFolderExist("System") == SD_OK) {
                    if (sd_checkFolderExist("Sample") == SD_OK) {
                        if (sd_checkFolderExist("System/File") == SD_OK) {
                            if (sd_checkFolderExist("System/Drumkit") == SD_OK) {
                                if (sd_checkFolderExist("System/Sound") == SD_OK) {
                                    if (sd_checkFolderExist("System/Image") == SD_OK) {
                                        if (sd_checkFolderExist("System/Firmware") == SD_OK) {
                                            if ((sd_checkMetronome() == SD_OK) && (sd_loadMetronome() == SD_OK) &&
                                                (sd_loadImage("System/Image/Image_Logo.rwi", RAM_IMAGE_LOGO_PALETTE_ADDRESS, RAM_IMAGE_LOGO_DATA_ADDRESS, kImageLogoPalette, kImageLogoWidth, kImageLogoHeight, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Image_Menu.rwi", RAM_IMAGE_MENU_PALETTE_ADDRESS, RAM_IMAGE_MENU_DATA_ADDRESS, kImageMenuPalette, kImageMenuWidth, kImageMenuHeight, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Image_Layer.rwi", RAM_IMAGE_LAYER_PALETTE_ADDRESS, RAM_IMAGE_LAYER_DATA_ADDRESS, kImageLayerPalette, kImageLayerWidth, kImageLayerHeight, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Icon_Select.rwi", RAM_ICON_SELECT_PALETTE_ADDRESS, RAM_ICON_SELECT_DATA_ADDRESS, kIconPalette, kIconSelectWidth, kIconSelectHeight * 2, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Icon_Alert.rwi", RAM_ICON_ALERT_PALETTE_ADDRESS, RAM_ICON_ALERT_DATA_ADDRESS, kIconPalette, kIconAlertWidth, kIconAlertHeight * 2, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Icon_Play.rwi", RAM_ICON_PLAY_PALETTE_ADDRESS, RAM_ICON_PLAY_DATA_ADDRESS, kIconPalette, kIconPlayWidth, kIconPlayHeight * 8, RGB16) == SD_OK) &&
                                                (sd_loadImage("System/Image/Icon_Layer.rwi", RAM_ICON_LAYER_PALETTE_ADDRESS, RAM_ICON_LAYER_DATA_ADDRESS, kIconLayerPalette, kIconLayerWidth, kIconLayerHeight * 7, RGB16) == SD_OK)) {
                                                sd.ready = true;
                                                sd.getLibrary = true;
                                                sdResult = SD_OK;
                                            } else {
                                                sdResult = SD_ERROR_SYSTEMFILE;
                                            }
                                        } else {
                                            sdResult = SD_ERROR_FIRMWAREFOLDER;
                                        }
                                    } else {
                                        sdResult = SD_ERROR_IMAGEFOLDER;
                                    }
                                } else {
                                    sdResult = SD_ERROR_SOUNDFOLDER;
                                }
                            } else {
                                sdResult = SD_ERROR_DRUMKITFOLDER;
                            }
                        } else {
                            sdResult = SD_ERROR_FILEFOLDER;
                        }
                    } else {
                        sdResult = SD_ERROR_SAMPLEFOLDER;
                    }
                } else {
                    sdResult = SD_ERROR_SYSTEMFOLDER;
                }
            } else {
                sdResult = SD_ERROR_MOUNT;
            }
        } else {
            sd.detect = false;
            sdResult = SD_ERROR_DETECT;
        }

        if (sdResult != SD_OK) {
            lcd_drawInitSdAlert(sdResult);
            sdInsertCheck = true;
            if (sdResult == SD_ERROR_DETECT) {
                while (sd_detect() != SD_OK) {
                    keyboard_check_C();
                    if (!power)
                        systemReset();
                }
            } else {
                sd_unmount();
                while (sd_detect() == SD_OK) {
                    keyboard_check_C();
                    if (!power)
                        systemReset();
                }
            }
            HAL_Delay(500);
        }
    }
    lcd_clearInitSdAlert();

    return sdResult;
}

SdResult Controller::sd_reinitialize() {
    SdResult sdResult = SD_ERROR;

    while (sdResult != SD_OK) {
        if (sd_detect() == SD_OK) {
            sd.detect = true;
            FATFS_UnLinkDriver(SDPath);
            FATFS_LinkDriver(&SD_Driver, SDPath);
            if ((sd_mount() == SD_OK) && (sd_getLabel() == SD_OK) && (sd_getSpace() == SD_OK)) {
                if (sd.serial == sd.serialTemp) {
                    if (sd_checkFolderExist("System") == SD_OK) {
                        if (sd_checkFolderExist("Sample") == SD_OK) {
                            if (sd_checkFolderExist("System/File") == SD_OK) {
                                if (sd_checkFolderExist("System/Drumkit") == SD_OK) {
                                    if (sd_checkFolderExist("System/Sound") == SD_OK) {
                                        if (sd_checkFolderExist("System/Image") == SD_OK) {
                                            if (sd_checkFolderExist("System/Firmware") == SD_OK) {
                                                sd.ready = true;
                                                sd.getLibrary = true;
                                                sdResult = SD_OK;
                                            } else {
                                                sdResult = SD_ERROR_FIRMWAREFOLDER;
                                            }
                                        } else {
                                            sdResult = SD_ERROR_IMAGEFOLDER;
                                        }
                                    } else {
                                        sdResult = SD_ERROR_SOUNDFOLDER;
                                    }
                                } else {
                                    sdResult = SD_ERROR_DRUMKITFOLDER;
                                }
                            } else {
                                sdResult = SD_ERROR_FILEFOLDER;
                            }
                        } else {
                            sdResult = SD_ERROR_SAMPLEFOLDER;
                        }
                    } else {
                        sdResult = SD_ERROR_SYSTEMFOLDER;
                    }
                } else {
                    sdResult = SD_ERROR_SERIAL;
                }
            } else {
                sdResult = SD_ERROR_MOUNT;
            }
        } else {
            sd.detect = false;
            sdResult = SD_ERROR_DETECT;
        }

        if (sdResult != SD_OK) {
            lcd_drawSdAlert(sdResult);
            if (sdResult == SD_ERROR_DETECT) {
                while (sd_detect() != SD_OK) {
                    lcd_update();
                    keyboard_check_C();
                    if (!power)
                        systemReset();
                }
            } else {
                sd_unmount();
                while (sd_detect() == SD_OK) {
                    lcd_update();
                    keyboard_check_C();
                    if (!power)
                        systemReset();
                }
            }
            HAL_Delay(500);
        }
    }
    lcd_clearSdAlert();
    lcd_clearSdData();
    // clear pre-alert
    alertFlag = false;
    alertType = ALERT_OFF;
    // go to main menu
    main_select();

    return sdResult;
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

SdResult Controller::sd_getLabel() {
    SdResult sdResult;
    (f_getlabel(SDPath, sd.label, &sd.serial) == FR_OK) ? sdResult = SD_OK : sdResult = SD_ERROR;
    return sdResult;
}

SdResult Controller::sd_setLabel() {
    SdResult sdResult;
    (f_setlabel("DRUMBOY")) ? sdResult = SD_OK : sdResult = SD_ERROR;
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

SdResult Controller::sd_loadImage(char *fileAddress, uint32_t paletteAddress, uint32_t dataAddress, uint16_t paletteSize, uint16_t width, uint16_t height, RGBMode mode) {
    SdResult sdResult = SD_ERROR;
    char data[20] = "";
    char headerTitle[] = "RW_IMAGE  ";
    char headerIndex[] = "_PA";
    char headerData[] = "_DA";
    uint32_t errorCount = 0;

    uint16_t rgb;
    (mode) ? rgb = 24 : rgb = 16;

    // disable keyboard
    keyboard_disable();

    if (f_open(&sd.file, fileAddress, FA_READ) == FR_OK) {
        if (f_read(&sd.file, data, 10, &sd.bytesread) == FR_OK) {
            if (strcmp(data, headerTitle) == 0) {
                memset(data, 0x00, sizeof(data));
                if (f_read(&sd.file, data, 8, &sd.bytesread) == FR_OK) {
                    uint16_t *paletteSize_ = (uint16_t *)&(data[0]);
                    uint16_t *rgb_ = (uint16_t *)&(data[2]);
                    uint16_t *width_ = (uint16_t *)&(data[4]);
                    uint16_t *height_ = (uint16_t *)&(data[6]);
                    if ((paletteSize == *paletteSize_) && (rgb == *rgb_) && (width == *width_) && (height == *height_)) {
                        memset(data, 0, sizeof(data));
                        f_read(&sd.file, data, 3, &sd.bytesread);
                        if (strcmp(data, headerIndex) == 0) {
                            memset(data, 0, sizeof(data));
                            if (mode == RGB16) {
                                const uint16_t readByteSize = 512;
                                uint16_t paletteByteSize = paletteSize * 2;
                                uint16_t readCountTotal = (paletteByteSize / readByteSize);
                                uint16_t remainderByteSize = (paletteByteSize % readByteSize);
                                uint8_t dataRead[readByteSize] = {0};
                                uint32_t pointerOffset = 0;

                                for (uint16_t i = 0; i < readCountTotal; i++) {
                                    if (f_read(&sd.file, &dataRead, readByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t j = 0; j < readByteSize; j++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(paletteAddress + pointerOffset);
                                        *writePtr = dataRead[j];
                                        pointerOffset += 1;
                                    }
                                    memset(dataRead, 0x00, readByteSize);
                                }
                                if (remainderByteSize) {
                                    if (f_read(&sd.file, &dataRead, remainderByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t k = 0; k < remainderByteSize; k++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(paletteAddress + pointerOffset);
                                        *writePtr = dataRead[k];
                                        pointerOffset += 1;
                                    }
                                }
                            } else if (mode == RGB24) {
                                const uint16_t readByteSize = 512;
                                uint16_t paletteByteSize = paletteSize * 3;
                                uint16_t readCountTotal = (paletteByteSize / readByteSize);
                                uint16_t remainderByteSize = (paletteByteSize % readByteSize);
                                uint8_t dataRead[readByteSize] = {0};
                                uint32_t pointerOffset = 0;

                                for (uint16_t i = 0; i < readCountTotal; i++) {
                                    if (f_read(&sd.file, &dataRead, readByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t j = 0; j < readByteSize; j++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(paletteAddress + pointerOffset);
                                        *writePtr = dataRead[j];
                                        pointerOffset += 1;
                                    }
                                    memset(dataRead, 0x00, readByteSize);
                                }
                                if (remainderByteSize) {
                                    if (f_read(&sd.file, &dataRead, remainderByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t k = 0; k < remainderByteSize; k++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(paletteAddress + pointerOffset);
                                        *writePtr = dataRead[k];
                                        pointerOffset += 1;
                                    }
                                }
                            }

                            f_read(&sd.file, data, 3, &sd.bytesread);
                            if (strcmp(data, headerData) == 0) {
                                memset(data, 0, sizeof(data));
                                const uint16_t readByteSize = 512;
                                uint32_t pixelByteSize = width * height;
                                uint16_t readCountTotal = (pixelByteSize / readByteSize);
                                uint16_t remainderByteSize = pixelByteSize % readByteSize;
                                uint8_t dataRead[readByteSize] = {0};
                                uint32_t pointerOffset = 0;
                                uint32_t errorCount = 0;

                                for (uint32_t i = 0; i < readCountTotal; i++) {
                                    if (f_read(&sd.file, &dataRead, readByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t j = 0; j < readByteSize; j++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(dataAddress + pointerOffset);
                                        *writePtr = dataRead[j];
                                        pointerOffset += 1;
                                    }
                                }
                                if (remainderByteSize) {
                                    if (f_read(&sd.file, &dataRead, remainderByteSize, &sd.bytesread) != FR_OK)
                                        errorCount += 1;
                                    for (uint16_t k = 0; k < remainderByteSize; k++) {
                                        volatile uint8_t *writePtr = (volatile uint8_t *)(dataAddress + pointerOffset);
                                        *writePtr = dataRead[k];
                                        pointerOffset += 1;
                                    }
                                }
                                if (errorCount == 0)
                                    sdResult = SD_OK;
                            }
                        }
                    }
                }
            }
        }
    }
    f_close(&sd.file);
    // enable keyboard
    keyboard_enable();
    return sdResult;
}

SdResult Controller::load16BitAudio(char *fileAddress, uint32_t ramAddress, uint32_t sampleSize) {
    SdResult sdResult = SD_ERROR;
    struct WavData wavData;

    // disable keyboard
    keyboard_disable();
    // read wav file
    if (f_open(&sd.file, fileAddress, FA_READ) == FR_OK) {
        if (f_read(&sd.file, &wavData.riff_chunk, 12, &sd.bytesread) == FR_OK) {
            // read riff_chunk
            if ((wavData.riff_chunk.chunkId == 0x46464952) && (wavData.riff_chunk.fileFormat == 0x45564157)) {
                uint32_t chunkSize = wavData.riff_chunk.chunkSize + 8;
                // read fmt_chunk
                for (uint32_t i = 0; i < (chunkSize - 24); i++) {
                    f_lseek(&sd.file, i);
                    f_read(&sd.file, &wavData.fmt_chunk, 24, &sd.bytesread);
                    if (wavData.fmt_chunk.chunkId == 0x20746D66) {
                        wavData.fmt_chunk.chunkStartByte = i;
                        break;
                    }
                }
                // check fmt_chunk
                if ((wavData.fmt_chunk.chunkId == 0x20746D66) &&
                    (wavData.fmt_chunk.chunkSize == 16) &&
                    (wavData.fmt_chunk.audioFormat == 0x01) &&
                    (wavData.fmt_chunk.nbrChannels == 0x01) &&
                    (wavData.fmt_chunk.sampleRate == 48000) &&
                    (wavData.fmt_chunk.bitPerSample == 16)) {
                    // read data_chunk
                    for (uint32_t j = 12; j < (chunkSize - 8); j++) {
                        f_lseek(&sd.file, j);
                        f_read(&sd.file, &wavData.data_chunk, 8, &sd.bytesread);
                        if ((wavData.data_chunk.chunkId == 0x61746164)) {
                            wavData.data_chunk.chunkStartByte = j;
                            break;
                        }
                    }
                    // check data chunk
                    uint32_t byteSize = sampleSize * 2;
                    if ((wavData.data_chunk.chunkId == 0x61746164) && (wavData.data_chunk.chunkSize == byteSize)) {
                        if (f_read(&sd.file, (uint8_t *)ramAddress, byteSize, &sd.bytesread) == FR_OK) {
                            sdResult = SD_OK;
                        }
                    }
                }
            }
        }
    }
    f_close(&sd.file);
    // enable keyboard
    keyboard_enable();
    return sdResult;
}

SdResult Controller::load24BitAudio(char *fileAddress, uint32_t ramAddress, uint32_t sampleSize) {
    SdResult sdResult = SD_ERROR;
    struct WavData wavData;

    // disable keyboard
    keyboard_disable();
    // read wav file
    if (f_open(&sd.file, fileAddress, FA_READ) == FR_OK) {
        if (f_read(&sd.file, &wavData.riff_chunk, 12, &sd.bytesread) == FR_OK) {
            // read riff_chunk
            if ((wavData.riff_chunk.chunkId == 0x46464952) && (wavData.riff_chunk.fileFormat == 0x45564157)) {
                uint32_t chunkSize = wavData.riff_chunk.chunkSize + 8;
                // read fmt_chunk
                for (uint32_t i = 0; i < (chunkSize - 24); i++) {
                    f_lseek(&sd.file, i);
                    f_read(&sd.file, &wavData.fmt_chunk, 24, &sd.bytesread);
                    if (wavData.fmt_chunk.chunkId == 0x20746D66) {
                        wavData.fmt_chunk.chunkStartByte = i;
                        break;
                    }
                }
                // check fmt_chunk
                if ((wavData.fmt_chunk.chunkId == 0x20746D66) &&
                    (wavData.fmt_chunk.chunkSize == 16) &&
                    (wavData.fmt_chunk.audioFormat == 0x01) &&
                    (wavData.fmt_chunk.nbrChannels == 0x01) &&
                    (wavData.fmt_chunk.sampleRate == 48000) &&
                    (wavData.fmt_chunk.bitPerSample == 24)) {
                    // read data_chunk
                    for (uint32_t j = 12; j < (chunkSize - 8); j++) {
                        f_lseek(&sd.file, j);
                        f_read(&sd.file, &wavData.data_chunk, 8, &sd.bytesread);
                        if ((wavData.data_chunk.chunkId == 0x61746164)) {
                            wavData.data_chunk.chunkStartByte = j;
                            break;
                        }
                    }
                    // check data chunk
                    uint32_t byteSize = sampleSize * 3;
                    if ((wavData.data_chunk.chunkId == 0x61746164) && (wavData.data_chunk.chunkSize == byteSize)) {
                        if (f_read(&sd.file, (uint8_t *)ramAddress, byteSize, &sd.bytesread) == FR_OK) {
                            sdResult = SD_OK;
                        }
                    }
                }
            }
        }
    }
    f_close(&sd.file);
    // enable keyboard
    keyboard_enable();
    return sdResult;
}

SdResult Controller::sd_checkMetronome() {
    return sd_checkFileExist("System/Sound/Metronome.wav");
}

SdResult Controller::sd_loadMetronome() {
    SdResult sdResult = load24BitAudio("System/Sound/Metronome.wav", RAM_METRO_ADDRESS, kMetroSize * 10);
    return sdResult;
}

void Controller::sd_getLibraries() {
    lcd_clearSdData();
    lcd_drawSdDataIntro();

    sd_deleteDirectory("System/Data");

    sd_getFileLibrary();
    sd_getDrumkitLibrary();
    sd_getInstLibrary();
    sd_getSampleLibrary();
    sd_checkSamplesInUse();

    lcd_drawSdData();
}

SdResult Controller::sd_getFileLibrary() {
    SdResult result = SD_ERROR;
    char refRead[sizeof(kFileRef)] = {};
    char data;

    // disable keyboard
    keyboard_disable();
    // read file library
    fileLibrarySize = 0;
    for (uint8_t i = 0; i < kFileLibrarySize; i++) {
        char fileName[50];
        char fileStart[] = "System/File/File_";
        char fileEnd[] = ".rws";
        char fileNum[3];

        sprintf(fileNum, "%03d", (i + 1));
        strcpy(fileName, fileStart);
        strcat(fileName, fileNum);
        strcat(fileName, fileEnd);

        if (f_open(&sd.file, fileName, FA_READ) == FR_OK) {
            if (f_read(&sd.file, refRead, strlen(kFileRef), &sd.bytesread) == 0) {
                if (f_read(&sd.file, &data, 1, &sd.bytesread) == FR_OK) {
                    if (data == '*')
                        fileLibrarySize += 1;
                }
            }
        }
        f_close(&sd.file);
    }

    result = SD_OK;
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_getDrumkitLibrary() {
    SdResult result = SD_ERROR;
    char refRead[sizeof(kDrumkitRef)] = {};
    char data;

    // disable keyboard
    keyboard_disable();
    // read drumkit library
    drumkitLibrarySize = 0;
    for (uint8_t i = 0; i < kDrumkitLibrarySize; i++) {
        char fileName[50];
        char fileStart[] = "System/Drumkit/Drumkit_";
        char fileEnd[] = ".rws";
        char fileNum[3];

        sprintf(fileNum, "%03d", (i + 1));
        strcpy(fileName, fileStart);
        strcat(fileName, fileNum);
        strcat(fileName, fileEnd);

        if (f_open(&sd.file, fileName, FA_READ) == FR_OK) {
            if (f_read(&sd.file, refRead, strlen(kDrumkitRef), &sd.bytesread) == 0) {
                if (f_read(&sd.file, &data, 1, &sd.bytesread) == FR_OK) {
                    if (data == '*')
                        drumkitLibrarySize += 1;
                }
            }
        }
        f_close(&sd.file);
    }

    result = SD_OK;
    // enable keyboard
    keyboard_enable();
    return result;
}

void Controller::sd_getInstLibrary() {
    bool listFolder = true;
    instLibrarySize = 0;

    // disable keyboard
    keyboard_disable();
    // read inst library
    if (f_opendir(&sd.dir, "/Sample\0") == FR_OK) {
        while (listFolder) {
            if ((f_readdir(&sd.dir, &sd.fileInfo) == FR_OK) && (sd.fileInfo.fname[0] != 0) && (sd.fileInfo.fattrib & AM_DIR) && (strlen(sd.fileInfo.fname) <= kFileNameSize) && (instLibrarySize <= kInstLibraryMaxSize)) {
                char *ptr = (char *)(RAM_INST_ADDRESS + (instLibrarySize * kFileNameSize));
                memset(ptr, 0x00, kFileNameSize);
                strncpy(ptr, sd.fileInfo.fname, kFileNameSize);
                instLibrarySize += 1;
            } else if (sd.fileInfo.fname[0] == 0) {
                listFolder = false;
            }
        }
    }
    f_closedir(&sd.dir);

    char *nameArray[instLibrarySize];
    for (uint16_t i = 0; i < instLibrarySize; i++) {
        nameArray[i] = (char *)(RAM_INST_ADDRESS + (i * kFileNameSize));
    }
    sortWords(nameArray, instLibrarySize);

    if ((f_mkdir("System/Data") == FR_OK) && (f_open(&sd.file, "System/Data/Inst.lib", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)) {
        f_write(&sd.file, "RW_DRUMBOY_INST          ", 25, &sd.byteswritten);
        for (uint16_t i = 0; i < instLibrarySize; i++) {
            f_write(&sd.file, nameArray[i], kFileNameSize, &sd.byteswritten);
        }
        f_write(&sd.file, "EOF", 3, &sd.byteswritten);
    }
    f_close(&sd.file);
    // enable keyboard
    keyboard_enable();
}

void Controller::sd_getSampleLibrary() {
    totalSampleLibrarySize = 0;
    // disable keyboard
    keyboard_disable();
    // read sample library
    for (uint16_t i = 0; i < instLibrarySize; i++) {
        bool listFile;
        char instName[kFileNameSize + 1] = "";
        char instFolderName[kFileNameSize + 10] = "Sample/";
        char instFileName[kFileNameSize + 10] = "System/Data/";
        char extension[] = ".ins";
        sampleLibrarySize[i] = 0;

        if (f_open(&sd.file, "System/Data/Inst.lib", FA_READ) == FR_OK) {
            if (f_lseek(&sd.file, 25 + (i * kFileNameSize)) == FR_OK) {
                if (f_read(&sd.file, instName, kFileNameSize, &sd.bytesread) == FR_OK) {
                    strncat(instFolderName, instName, kFileNameSize);
                    strncat(instFileName, instName, kFileNameSize);
                    strncat(instFileName, extension, 4);

                    sd.fresult = f_findfirst(&sd.dir, &sd.fileInfo, instFolderName, "?*.WAV");
                    if (sd.fileInfo.fname[0] == '\0') {
                        listFile = false;
                        sampleLibrarySize[i] = 0;
                    } else {
                        listFile = true;
                        while ((sd.fresult == FR_OK) && (sd.fileInfo.fname[0]) && (sampleLibrarySize[i] <= kInstSampleLibraryMaxSize)) {
                            if (!(sd.fileInfo.fattrib & AM_HID) && (!(sd.fileInfo.fattrib & AM_DIR))) {
                                char *ptr = (char *)(RAM_SAMPLE_ADDRESS + (sampleLibrarySize[i] * kFileNameSize));
                                memset(ptr, 0x00, kFileNameSize);
                                strncpy(ptr, sd.fileInfo.fname, kFileNameSize);
                                sampleLibrarySize[i] += 1;
                            }
                            f_findnext(&sd.dir, &sd.fileInfo);
                        }
                        f_closedir(&sd.dir);
                    }
                }
            }
        }
        f_close(&sd.file);

        char *nameArray[sampleLibrarySize[i]];
        for (uint16_t j = 0; j < sampleLibrarySize[i]; j++) {
            nameArray[j] = (char *)(RAM_SAMPLE_ADDRESS + (j * kFileNameSize));
        }
        sortWords(nameArray, sampleLibrarySize[i]);

        if (f_open(&sd.file, instFileName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            f_write(&sd.file, "RW_DRUMBOY_SAMPLE        ", 25, &sd.byteswritten);
            if (sampleLibrarySize[i]) {
                for (uint16_t j = 0; j < sampleLibrarySize[i]; j++) {
                    f_write(&sd.file, nameArray[j], kFileNameSize, &sd.byteswritten);
                }
            }
            f_write(&sd.file, "EOF", 3, &sd.byteswritten);
        }
        f_close(&sd.file);
        totalSampleLibrarySize += sampleLibrarySize[i];
    }
    // enable keyboard
    keyboard_enable();
}

SdResult Controller::sd_checkFile(uint8_t fileNum_) {
    SdResult result = SD_ERROR;
    fileStatus = FILE_NONE;
    char refRead[sizeof(kFileRef)] = {};
    char data;

    char fileName[50];
    char fileNum[4];
    sprintf(fileNum, "%03d", (fileNum_ + 1));
    strcpy(fileName, kFileStart);
    strcat(fileName, fileNum);
    strcat(fileName, kFileEnd);

    if (f_open(&sd.file, fileName, FA_READ) == FR_OK) {
        if (f_read(&sd.file, refRead, strlen(kFileRef), &sd.bytesread) == FR_OK) {
            if (strncmp(kFileRef, refRead, strlen(kFileRef)) == 0) {
                f_read(&sd.file, &data, 1, &sd.bytesread);
                (data == '*') ? fileStatus = FILE_ACTIVE : fileStatus = FILE_INACTIVE;
            } else {
                fileStatus = FILE_INCOMPATIBLE;
            }
        } else {
            fileStatus = FILE_INCOMPATIBLE;
        }
    } else {
        fileStatus = FILE_MISSING;
    }
    f_close(&sd.file);
    result = SD_OK;
    return result;
}

SdResult Controller::sd_loadFile(uint8_t fileNum_) {
    SdResult result = SD_ERROR;

    char fileName[50];
    char fileNum[4];
    sprintf(fileNum, "%03d", (fileNum_ + 1));
    strcpy(fileName, kFileStart);
    strcat(fileName, fileNum);
    strcat(fileName, kFileEnd);

    // 000.000 *                 1 byte
    // 000.001 main            140 bytes
    // 000.200 layer            70 bytes
    // 000.300 inst&sample    1000 bytes
    // 001.300 song           9660 bytes
    // total size            10960 bytes

    // disable keyboard
    keyboard_disable();
    // load file
    if ((sd_checkFile(fileNum_) == SD_OK) && (fileStatus == FILE_ACTIVE)) {
        char data[kFileByteSize] = {};
        if (f_open(&sd.file, fileName, FA_READ) == FR_OK) {
            if (f_lseek(&sd.file, strlen(kFileRef)) == FR_OK) {
                const uint16_t readByteSize = 512;
                const uint16_t fileByteSize = kFileByteSize;
                uint16_t readCountTotal = (fileByteSize / readByteSize);
                uint16_t remainderByteSize = (fileByteSize % readByteSize);
                uint8_t dataRead[readByteSize] = {0};
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < readCountTotal; i++) {
                    if (f_read(&sd.file, &dataRead, readByteSize, &sd.bytesread) != FR_OK)
                        errorCount += 1;
                    for (uint16_t j = 0; j < readByteSize; j++) {
                        volatile uint8_t *writePtr = (volatile uint8_t *)(data + pointerOffset);
                        *writePtr = dataRead[j];
                        pointerOffset += 1;
                    }
                    memset(dataRead, 0x00, readByteSize);
                }

                if (remainderByteSize) {
                    if (f_read(&sd.file, &dataRead, remainderByteSize, &sd.bytesread) != FR_OK)
                        errorCount += 1;
                    for (uint16_t k = 0; k < remainderByteSize; k++) {
                        volatile uint8_t *writePtr = (volatile uint8_t *)(data + pointerOffset);
                        *writePtr = dataRead[k];
                        pointerOffset += 1;
                    }
                }

                f_close(&sd.file);
                if (errorCount == 0) {
                    // write rhythmData
                    rhythm_setTempo(data[1]);
                    rhythm_setMeasure(data[2]);
                    rhythm_setBar(data[3]);
                    rhythm_setQuantize(data[4]);
                    // write metronomeData
                    metro_setActive((bool)data[5]);
                    metro_setPrecount(data[6]);
                    metro_setSample(data[7]);
                    metro_setVolume(data[8]);
                    // write eqData
                    eq_setActive((bool)data[9]);
                    eq_setFreqLowShelf(data[10]);
                    eq_setGainLowShelf(data[11]);
                    eq_setFreqHighShelf(data[12]);
                    eq_setGainHighShelf(data[13]);
                    eq_setQPeak(0, data[14]);
                    eq_setQPeak(1, data[15]);
                    eq_setQPeak(2, data[16]);
                    eq_setQPeak(3, data[17]);
                    eq_setFreqPeak(0, data[18]);
                    eq_setFreqPeak(1, data[19]);
                    eq_setFreqPeak(2, data[20]);
                    eq_setFreqPeak(3, data[21]);
                    eq_setGainPeak(0, data[22]);
                    eq_setGainPeak(1, data[23]);
                    eq_setGainPeak(2, data[24]);
                    eq_setGainPeak(3, data[25]);
                    // write filterData
                    filter_setActive(0, (bool)data[26]);
                    filter_setType(0, data[27]);
                    filter_setFreq(0, data[28]);
                    filter_setRes(0, data[29]);
                    filter_setSlope(0, data[30]);
                    filter_setDry(0, data[31]);
                    filter_setWet(0, data[32]);
                    filter_setActive(1, (bool)data[33]);
                    filter_setType(1, data[34]);
                    filter_setFreq(1, data[35]);
                    filter_setRes(1, data[36]);
                    filter_setSlope(1, data[37]);
                    filter_setDry(1, data[38]);
                    filter_setWet(1, data[39]);
                    // write effectData
                    effect_setActive(0, (bool)data[40]);
                    effect_setType(0, data[41]);
                    effect_setActive(1, (bool)data[42]);
                    effect_setType(1, data[43]);
                    for (uint8_t i = 0; i < kEffectLibrarySize; i++) {
                        for (uint8_t j = 0; j < kSubEffectLibrarySize; j++) {
                            uint16_t baseNum = 44 + (45 * i) + (5 * j);
                            effect_setAData(i, j, data[baseNum + 0]);
                            effect_setBData(i, j, data[baseNum + 1]);
                            effect_setCData(i, j, data[baseNum + 2]);
                            effect_setDData(i, j, data[baseNum + 3]);
                            effect_setEData(i, j, data[baseNum + 4]);
                        }
                    }
                    // write reverbData
                    reverb_setActive(data[134]);
                    reverb_setSize(data[135]);
                    reverb_setDecay(data[136]);
                    reverb_setPreDelay(data[137]);
                    reverb_setSurround(data[138]);
                    reverb_setDry(data[139]);
                    reverb_setWet(data[140]);
                    // write layerData
                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        uint16_t offset = 200 + (i * 7);
                        layerInst_setVolume(i, data[offset + 0]);
                        layerInst_setSpeed(i, data[offset + 1]);
                        layerInst_setReverse(i, data[offset + 2]);
                        layerInst_setNormalize(i, data[offset + 3]);
                        layerInst_setMute(i, (bool)data[offset + 4]);
                        layerInst_setFill(i, (bool)data[offset + 5]);
                        layerInst_setStyle(i, (bool)data[offset + 6]);
                    }

                    // read instData & sampleData
                    char instName[kLayerLibrarySize][kFileNameSize + 1];
                    uint16_t instNum[kLayerLibrarySize];
                    bool instActive[kLayerLibrarySize];
                    bool instMissing[kLayerLibrarySize];

                    char sampleName[kLayerLibrarySize][kFileNameSize + 1];
                    uint16_t sampleNum[kLayerLibrarySize];
                    bool sampleActive[kLayerLibrarySize];
                    bool sampleMissing[kLayerLibrarySize];

                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        Layer &layer = layerLibrary[i];
                        uint16_t layerOffset = 300 + (i * 100);

                        (data[layerOffset] == 'x') ? instActive[i] = true : instActive[i] = false;
                        (data[layerOffset + 50] == 'x') ? sampleActive[i] = true : sampleActive[i] = false;

                        memcpy(instName[i], &(data[layerOffset + 1]), kFileNameSize + 1);
                        memcpy(sampleName[i], &(data[layerOffset + 50 + 1]), kFileNameSize + 1);
                    }

                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        // write inst data
                        if (instActive[i]) {
                            if ((f_open(&sd.file, "System/Data/Inst.lib", FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                                char searchName[kFileNameSize + 1];
                                bool searchActive = true;
                                uint16_t searchCounter = 0;

                                while (searchActive) {
                                    memset(searchName, 0x00, sizeof(searchName));
                                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);

                                    if (strcmp(instName[i], searchName) == 0) {
                                        instNum[i] = searchCounter;
                                        instMissing[i] = false;
                                        searchActive = false;
                                    } else if (searchCounter >= instLibrarySize) {
                                        instNum[i] = -1;
                                        instMissing[i] = true;
                                        searchActive = false;
                                    }
                                    searchCounter += 1;
                                }
                            } else {
                                instMissing[i] = true;
                            }
                            f_close(&sd.file);
                        } else {
                            instNum[i] = -1;
                            instMissing[i] = false;
                        }
                        if (instMissing[i]) {
                            layerInst_setInstSelected(i, -1);
                            layerInst_setInstLoaded(i);
                            layerInst_setSampleSelected(i, -1);
                            layerInst_setSampleLoaded(i);
                        } else {
                            layerInst_setInstSelected(i, instNum[i]);
                            layerInst_setInstLoaded(i);
                        }
                        // write sampleData
                        if ((sampleActive[i]) && (instActive[i]) && (!instMissing[i])) {
                            char instFileName[kFileNameSize + 15] = "System/Data/";
                            strncat(instFileName, instName[i], kFileNameSize);
                            strncat(instFileName, ".ins", 4);

                            if ((f_open(&sd.file, instFileName, FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                                char searchName[kFileNameSize + 1];
                                bool searchActive = true;
                                uint16_t searchCounter = 0;

                                while (searchActive) {
                                    memset(searchName, 0x00, kFileNameSize);
                                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);

                                    if (strcmp(sampleName[i], searchName) == 0) {
                                        sampleNum[i] = searchCounter;
                                        sampleMissing[i] = false;
                                        searchActive = false;
                                    } else if (searchCounter >= sampleLibrarySize[instNum[i]]) {
                                        sampleNum[i] = -1;
                                        sampleMissing[i] = true;
                                        searchActive = false;
                                    }
                                    searchCounter += 1;
                                }
                            } else {
                                sampleMissing[i] = true;
                            }
                            f_close(&sd.file);
                        } else {
                            sampleNum[i] = -1;
                            sampleMissing[i] = false;
                        }
                        if (sampleMissing[i]) {
                            layerInst_setInstSelected(i, -1);
                            layerInst_setInstLoaded(i);
                            layerInst_setSampleSelected(i, -1);
                            layerInst_setSampleLoaded(i);
                        } else {
                            layerInst_setSampleSelected(i, sampleNum[i]);
                            layerInst_setSampleLoaded(i);
                        }
                    }
                    // write songData
                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        Layer &layer = layerLibrary[i];
                        uint16_t offsetLayer = 1300 + (i * 966);
                        for (uint8_t j = 0; j < kBankLibrarySize; j++) {
                            layerSong_resetAllBeats(i, j);
                            Bank &bank = layer.bankLibrary[j];
                            uint16_t offsetBank = offsetLayer + 1 + (j * 193);
                            for (int8_t k = (kBeatLibrarySize - 1); k >= 0; k--) {
                                uint16_t offsetBeat = offsetBank + 1 + (k * 3);
                                uint8_t dataBeat = data[offsetBeat];
                                bool active = dataBeat >> 7;
                                if (active) {
                                    uint8_t startInterval0 = data[offsetBeat + 1];
                                    uint8_t startInterval1 = data[offsetBeat + 2];
                                    uint16_t startInterval = ((uint16_t)startInterval0 << 8) + startInterval1;
                                    uint8_t fill = (dataBeat & 0b01111111);
                                    layerSong_setBeat(i, j, startInterval, fill);
                                }
                            }
                            layerSong_arrangeActiveBeats(i, j, true, true, true, true);
                        }
                    }
                    uint8_t missingSample = 0;
                    for (uint8_t j = 0; j < kLayerLibrarySize; j++) {
                        if (instMissing[j])
                            missingSample += 1;
                        if (sampleMissing[j])
                            missingSample += 1;
                    }
                    if (missingSample) {
                        (missingSample == 1) ? alertType = ALERT_MISSINGSAMPLE : alertType = ALERT_MISSINGSAMPLES;
                        lcd_drawAlert();
                        HAL_Delay(1000);
                        result = SD_ERROR;
                    } else {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_saveFile(uint8_t fileNum_) {
    SdResult result = SD_ERROR;

    char eof[] = "EOF";
    char fileName[50];
    char fileNum[4];
    sprintf(fileNum, "%03d", (fileNum_ + 1));
    strcpy(fileName, kFileStart);
    strcat(fileName, fileNum);
    strcat(fileName, kFileEnd);

    // 000.000 *                 1 byte
    // 000.001 main            140 bytes
    // 000.200 layer            70 bytes
    // 000.300 inst&sample    1000 bytes
    // 001.300 song           9660 bytes
    // total size            10960 bytes

    // write fileData
    uint8_t data[kFileByteSize] = {};
    data[0] = '*';
    // write rhythmData
    data[1] = rhythm.tempo;
    data[2] = rhythm.measure;
    data[3] = rhythm.bar;
    data[4] = rhythm.quantize;
    // write metronomeData
    data[5] = metronome.active;
    data[6] = metronome.precount;
    data[7] = metronome.sample;
    data[8] = metronome.volume;
    // write eqData
    data[9] = eq.active;
    data[10] = eq.freqLowShelf;
    data[11] = eq.gainLowShelf;
    data[12] = eq.freqHighShelf;
    data[13] = eq.gainHighShelf;
    data[14] = eq.qPeak[0];
    data[15] = eq.qPeak[1];
    data[16] = eq.qPeak[2];
    data[17] = eq.qPeak[3];
    data[18] = eq.freqPeak[0];
    data[19] = eq.freqPeak[1];
    data[20] = eq.freqPeak[2];
    data[21] = eq.freqPeak[3];
    data[22] = eq.gainPeak[0];
    data[23] = eq.gainPeak[1];
    data[24] = eq.gainPeak[2];
    data[25] = eq.gainPeak[3];
    // write filterData
    data[26] = filter[0].active;
    data[27] = filter[0].type;
    data[28] = filter[0].freq;
    data[29] = filter[0].res;
    data[30] = filter[0].slope;
    data[31] = filter[0].dry;
    data[32] = filter[0].wet;
    data[33] = filter[1].active;
    data[34] = filter[1].type;
    data[35] = filter[1].freq;
    data[36] = filter[1].res;
    data[37] = filter[1].slope;
    data[38] = filter[1].dry;
    data[39] = filter[1].wet;
    // write effectData
    data[40] = effect[0].active;
    data[41] = effect[0].type;
    data[42] = effect[1].active;
    data[43] = effect[1].type;
    for (uint8_t i = 0; i < kEffectLibrarySize; i++) {
        for (uint8_t j = 0; j < kSubEffectLibrarySize; j++) {
            uint16_t baseNum = 44 + (45 * i) + (5 * j);
            data[baseNum + 0] = effect[i].subEffect[j].aData;
            data[baseNum + 1] = effect[i].subEffect[j].bData;
            data[baseNum + 2] = effect[i].subEffect[j].cData;
            data[baseNum + 3] = effect[i].subEffect[j].dData;
            data[baseNum + 4] = effect[i].subEffect[j].eData;
        }
    }
    // write reverbData
    data[134] = reverb.active;
    data[135] = reverb.size;
    data[136] = reverb.decay;
    data[137] = reverb.preDelay;
    data[138] = reverb.surround;
    data[139] = reverb.dry;
    data[140] = reverb.wet;
    // write layerData
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        uint16_t offset = 200 + (i * 7);
        data[offset + 0] = layer.volume;
        data[offset + 1] = layer.speed;
        data[offset + 2] = layer.reverse;
        data[offset + 3] = layer.normalize;
        data[offset + 4] = layer.mute;
        data[offset + 5] = layer.fill;
        data[offset + 6] = layer.style;
    }
    // write instData & sampleData
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        uint16_t offset = 300 + (i * 100);

        // write instData
        if (layer.instLoaded != -1) {
            data[offset] = 'x';
            for (uint8_t j = 0; j < kFileNameSize; j++) {
                data[offset + 1 + j] = layer.instLoadedData.nameLong[j];
            }
        } else {
            data[offset] = 'o';
        }
        // write sampleData
        if (layer.sampleLoaded != -1) {
            data[offset + 50] = 'x';
            for (uint8_t j = 0; j < kFileNameSize; j++) {
                data[offset + 50 + 1 + j] = layer.sampleLoadedData.nameLong[j];
            }
        } else {
            data[offset + 50] = 'o';
        }
    }
    // write songData
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        uint16_t offsetLayer = 1300 + (i * 966);
        data[offsetLayer] = 'l';
        for (uint8_t j = 0; j < kBankLibrarySize; j++) {
            Bank &bank = layer.bankLibrary[j];
            uint16_t offsetBank = offsetLayer + 1 + (j * 193);
            data[offsetBank] = 'b';
            for (uint8_t k = 0; k < kBeatLibrarySize; k++) {
                Beat &beat = bank.beatLibrary[k];
                uint16_t offsetBeat = offsetBank + 1 + (k * 3);
                data[offsetBeat] = beat.getData();
                uint16_t startInterval = beat.getStartInterval();
                data[offsetBeat + 1] = (uint8_t)(startInterval >> 8);
                data[offsetBeat + 2] = (uint8_t)(startInterval & 0xFF);
            }
        }
    }
    // disable keyboard
    keyboard_disable();
    // write data to sdcard
    if (sd_checkFile(fileNum_) == SD_OK) {
        if (f_open(&sd.file, fileName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            if (f_write(&sd.file, kFileRef, strlen(kFileRef), &sd.byteswritten) == FR_OK) {
                const uint16_t writeByteSize = 512;
                const uint16_t fileByteSize = kFileByteSize;
                uint16_t writeCountTotal = (fileByteSize / writeByteSize);
                uint16_t remainderByteSize = (fileByteSize % writeByteSize);
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < writeCountTotal; i++) {
                    if (f_write(&sd.file, (data + pointerOffset), writeByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                    pointerOffset += 512;
                }

                if (remainderByteSize) {
                    if (f_write(&sd.file, (data + pointerOffset), remainderByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                }

                if (errorCount == 0) {
                    if (f_write(&sd.file, eof, strlen(eof), &sd.byteswritten) == FR_OK) {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_clearFile(uint8_t fileNum_) {
    SdResult result = SD_ERROR;

    char eof[] = "EOF";
    char fileName[50];
    char fileNum[4];
    sprintf(fileNum, "%03d", (fileNum_ + 1));
    strcpy(fileName, kFileStart);
    strcat(fileName, fileNum);
    strcat(fileName, kFileEnd);

    // 000.000 *                 1 byte
    // 000.001 main            140 bytes
    // 000.200 layer            70 bytes
    // 000.300 inst&sample    1000 bytes
    // 001.300 song           9660 bytes
    // total size            10960 bytes

    const char data[kFileByteSize + 1] = "-                                                                                                                                                                                                                                                                                                           o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                lb                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                b                                                                                                                                                                                                ";

    // disable keyboard
    keyboard_disable();
    if (sd_checkFile(fileNum_) == SD_OK) {
        if (f_open(&sd.file, fileName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            if (f_write(&sd.file, kFileRef, strlen(kFileRef), &sd.byteswritten) == FR_OK) {
                const uint16_t writeByteSize = 512;
                const uint16_t fileByteSize = kFileByteSize;
                uint16_t writeCountTotal = (fileByteSize / writeByteSize);
                uint16_t remainderByteSize = (fileByteSize % writeByteSize);
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < writeCountTotal; i++) {
                    if (f_write(&sd.file, (data + pointerOffset), writeByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                    pointerOffset += 512;
                }

                if (remainderByteSize) {
                    if (f_write(&sd.file, (data + pointerOffset), remainderByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                }

                if (errorCount == 0) {
                    if (f_write(&sd.file, eof, strlen(eof), &sd.byteswritten) == FR_OK) {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_checkDrumkit(uint8_t kitNum_) {
    SdResult result = SD_ERROR;
    drumkitStatus = FILE_NONE;
    char refRead[sizeof(kDrumkitRef)] = {};
    char data;

    char drumkitName[50];
    char drumkitNum[4];
    sprintf(drumkitNum, "%03d", (kitNum_ + 1));
    strcpy(drumkitName, kDrumkitStart);
    strcat(drumkitName, drumkitNum);
    strcat(drumkitName, kDrumkitEnd);

    if (f_open(&sd.file, drumkitName, FA_READ) == FR_OK) {
        if (f_read(&sd.file, refRead, strlen(kDrumkitRef), &sd.bytesread) == FR_OK) {
            if (strncmp(kDrumkitRef, refRead, strlen(kDrumkitRef)) == 0) {
                f_read(&sd.file, &data, 1, &sd.bytesread);
                (data == '*') ? drumkitStatus = FILE_ACTIVE : drumkitStatus = FILE_INACTIVE;
            } else {
                drumkitStatus = FILE_INCOMPATIBLE;
            }
        } else {
            drumkitStatus = FILE_INCOMPATIBLE;
        }
    } else {
        drumkitStatus = FILE_MISSING;
    }
    f_close(&sd.file);
    result = SD_OK;
    return result;
}

SdResult Controller::sd_loadDrumkit(bool mode_, uint8_t kitNum_) {
    SdResult result = SD_ERROR;

    char drumkitName[50];
    char drumkitNum[4];
    sprintf(drumkitNum, "%03d", (kitNum_ + 1));
    strcpy(drumkitName, kDrumkitStart);
    strcat(drumkitName, drumkitNum);
    strcat(drumkitName, kDrumkitEnd);

    // 000.000 *              0001 byte
    // 000.001 main           0132 bytes
    // 000.200 layer          0070 bytes
    // 000.300 inst&sample    1000 bytes
    // total size             1300 bytes

    // disable keyboard
    keyboard_disable();
    // load drumkit
    if ((sd_checkDrumkit(kitNum_) == SD_OK) && (drumkitStatus == FILE_ACTIVE)) {
        char data[kDrumkitByteSize] = {0x00};
        if (f_open(&sd.file, drumkitName, FA_READ) == FR_OK) {
            if (f_lseek(&sd.file, strlen(kDrumkitRef)) == FR_OK) {
                const uint16_t readByteSize = 512;
                const uint16_t drumkitByteSize = kDrumkitByteSize;
                uint16_t readCountTotal = (drumkitByteSize / readByteSize);
                uint16_t remainderByteSize = (drumkitByteSize % readByteSize);
                uint8_t dataRead[readByteSize] = {0};
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < readCountTotal; i++) {
                    if (f_read(&sd.file, &dataRead, readByteSize, &sd.bytesread) != FR_OK)
                        errorCount += 1;
                    for (uint16_t j = 0; j < readByteSize; j++) {
                        volatile uint8_t *writePtr = (volatile uint8_t *)(data + pointerOffset);
                        *writePtr = dataRead[j];
                        pointerOffset += 1;
                    }
                    memset(dataRead, 0x00, readByteSize);
                }

                if (remainderByteSize) {
                    if (f_read(&sd.file, &dataRead, remainderByteSize, &sd.bytesread) != FR_OK)
                        errorCount += 1;
                    for (uint16_t k = 0; k < remainderByteSize; k++) {
                        volatile uint8_t *writePtr = (volatile uint8_t *)(data + pointerOffset);
                        *writePtr = dataRead[k];
                        pointerOffset += 1;
                    }
                }

                f_close(&sd.file);
                if (errorCount == 0) {
                    // write eqData
                    eq_setActive((bool)data[1]);
                    eq_setFreqLowShelf(data[2]);
                    eq_setGainLowShelf(data[3]);
                    eq_setFreqHighShelf(data[4]);
                    eq_setGainHighShelf(data[5]);
                    eq_setQPeak(0, data[6]);
                    eq_setQPeak(1, data[7]);
                    eq_setQPeak(2, data[8]);
                    eq_setQPeak(3, data[9]);
                    eq_setFreqPeak(0, data[10]);
                    eq_setFreqPeak(1, data[11]);
                    eq_setFreqPeak(2, data[12]);
                    eq_setFreqPeak(3, data[13]);
                    eq_setGainPeak(0, data[14]);
                    eq_setGainPeak(1, data[15]);
                    eq_setGainPeak(2, data[16]);
                    eq_setGainPeak(3, data[17]);
                    // write filterData
                    filter_setActive(0, (bool)data[18]);
                    filter_setType(0, data[19]);
                    filter_setFreq(0, data[20]);
                    filter_setRes(0, data[21]);
                    filter_setSlope(0, data[22]);
                    filter_setDry(0, data[23]);
                    filter_setWet(0, data[24]);
                    filter_setActive(1, (bool)data[25]);
                    filter_setType(1, data[26]);
                    filter_setFreq(1, data[27]);
                    filter_setRes(1, data[28]);
                    filter_setSlope(1, data[29]);
                    filter_setDry(1, data[30]);
                    filter_setWet(1, data[31]);
                    // write effectData
                    effect_setActive(0, (bool)data[32]);
                    effect_setType(0, data[33]);
                    effect_setActive(1, (bool)data[34]);
                    effect_setType(1, data[35]);
                    for (uint8_t i = 0; i < kEffectLibrarySize; i++) {
                        for (uint8_t j = 0; j < kSubEffectLibrarySize; j++) {
                            uint16_t baseNum = 36 + (45 * i) + (5 * j);
                            effect_setAData(i, j, data[baseNum + 0]);
                            effect_setBData(i, j, data[baseNum + 1]);
                            effect_setCData(i, j, data[baseNum + 2]);
                            effect_setDData(i, j, data[baseNum + 3]);
                            effect_setEData(i, j, data[baseNum + 4]);
                        }
                    }
                    // write reverbData
                    reverb_setActive(data[126]);
                    reverb_setSize(data[127]);
                    reverb_setDecay(data[128]);
                    reverb_setPreDelay(data[129]);
                    reverb_setSurround(data[130]);
                    reverb_setDry(data[131]);
                    reverb_setWet(data[132]);
                    // write layerData
                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        uint16_t offset = 200 + (i * 7);
                        layerInst_setVolume(i, data[offset + 0]);
                        layerInst_setSpeed(i, data[offset + 1]);
                        layerInst_setReverse(i, data[offset + 2]);
                        layerInst_setNormalize(i, data[offset + 3]);
                        layerInst_setMute(i, (bool)data[offset + 4]);
                        layerInst_setFill(i, (bool)data[offset + 5]);
                        layerInst_setStyle(i, (bool)data[offset + 6]);
                    }
                    // read instData & sampleData
                    char instName[kLayerLibrarySize][kFileNameSize + 1];
                    uint16_t instNum[kLayerLibrarySize];
                    bool instActive[kLayerLibrarySize];
                    bool instMissing[kLayerLibrarySize] = {false};

                    char sampleName[kLayerLibrarySize][kFileNameSize + 1];
                    uint16_t sampleNum[kLayerLibrarySize];
                    bool sampleActive[kLayerLibrarySize];
                    bool sampleMissing[kLayerLibrarySize];

                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        Layer &layer = layerLibrary[i];
                        uint16_t offset = 300 + (i * 100);

                        (data[offset] == 'x') ? instActive[i] = true : instActive[i] = false;
                        (data[offset + 50] == 'x') ? sampleActive[i] = true : sampleActive[i] = false;

                        memcpy(instName[i], &(data[offset + 1]), kFileNameSize + 1);
                        memcpy(sampleName[i], &(data[offset + 50 + 1]), kFileNameSize + 1);
                    }

                    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                        // write instData
                        if (instActive[i]) {
                            if ((f_open(&sd.file, "System/Data/Inst.lib", FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                                char searchName[kFileNameSize + 1];
                                bool searchActive = true;
                                uint16_t searchCounter = 0;

                                while (searchActive) {
                                    memset(searchName, 0x00, sizeof(searchName));
                                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);

                                    if (strcmp(instName[i], searchName) == 0) {
                                        instNum[i] = searchCounter;
                                        instMissing[i] = false;
                                        searchActive = false;
                                    } else if (searchCounter >= instLibrarySize) {
                                        instNum[i] = -1;
                                        instMissing[i] = true;
                                        searchActive = false;
                                    }
                                    searchCounter += 1;
                                }
                            } else {
                                instMissing[i] = true;
                            }
                            f_close(&sd.file);
                        } else {
                            instNum[i] = -1;
                            instMissing[i] = false;
                        }
                        if (instMissing[i]) {
                            layerInst_setInstSelected(i, -1);
                            layerInst_setInstLoaded(i);
                            layerInst_setSampleSelected(i, -1);
                            layerInst_setSampleLoaded(i);
                        } else {
                            layerInst_setInstSelected(i, instNum[i]);
                            layerInst_setInstLoaded(i);
                        }
                        // write sampleData
                        if ((sampleActive[i]) && (instActive[i]) && (!instMissing[i])) {
                            char instFileName[kFileNameSize + 15] = "System/Data/";
                            strncat(instFileName, instName[i], kFileNameSize);
                            strncat(instFileName, ".ins", 4);

                            if ((f_open(&sd.file, instFileName, FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                                char searchName[kFileNameSize + 1];
                                bool searchActive = true;
                                uint16_t searchCounter = 0;

                                while (searchActive) {
                                    memset(searchName, 0x00, kFileNameSize);
                                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);

                                    if (strcmp(sampleName[i], searchName) == 0) {
                                        sampleNum[i] = searchCounter;
                                        sampleMissing[i] = false;
                                        searchActive = false;
                                    } else if (searchCounter >= sampleLibrarySize[instNum[i]]) {
                                        sampleNum[i] = -1;
                                        sampleMissing[i] = true;
                                        searchActive = false;
                                    }
                                    searchCounter += 1;
                                }
                            } else {
                                sampleMissing[i] = true;
                            }
                            f_close(&sd.file);
                        } else {
                            sampleNum[i] = -1;
                            sampleMissing[i] = false;
                        }
                        if (sampleMissing[i]) {
                            layerInst_setInstSelected(i, -1);
                            layerInst_setInstLoaded(i);
                            layerInst_setSampleSelected(i, -1);
                            layerInst_setSampleLoaded(i);
                        } else {
                            layerInst_setSampleSelected(i, sampleNum[i]);
                            layerInst_setSampleLoaded(i);
                        }
                    }

                    uint8_t missingSample = 0;
                    for (uint8_t j = 0; j < kLayerLibrarySize; j++) {
                        if (instMissing[j])
                            missingSample += 1;
                        if (sampleMissing[j])
                            missingSample += 1;
                    }
                    if (missingSample) {
                        (missingSample == 1) ? alertType = ALERT_MISSINGSAMPLE : alertType = ALERT_MISSINGSAMPLES;
                        lcd_drawAlert();
                        HAL_Delay(1000);
                        result = SD_ERROR;
                    } else {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }

    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_saveDrumkit(bool mode_, uint8_t kitNum_) {
    SdResult result = SD_ERROR;

    char eof[] = "EOF";
    char drumkitName[50];
    char drumkitNum[4];
    sprintf(drumkitNum, "%03d", (kitNum_ + 1));
    strcpy(drumkitName, kDrumkitStart);
    strcat(drumkitName, drumkitNum);
    strcat(drumkitName, kDrumkitEnd);

    // 000.000 *              0001 byte
    // 000.001 main           0132 bytes
    // 000.200 layer          0070 bytes
    // 000.300 inst&sample    1000 bytes
    // total size             1300 bytes

    // write drumkit
    uint8_t data[kDrumkitByteSize] = {0x00};
    data[0] = '*';
    // write eqData
    data[1] = eq.active;
    data[2] = eq.freqLowShelf;
    data[3] = eq.gainLowShelf;
    data[4] = eq.freqHighShelf;
    data[5] = eq.gainHighShelf;
    data[6] = eq.qPeak[0];
    data[7] = eq.qPeak[1];
    data[8] = eq.qPeak[2];
    data[9] = eq.qPeak[3];
    data[10] = eq.freqPeak[0];
    data[11] = eq.freqPeak[1];
    data[12] = eq.freqPeak[2];
    data[13] = eq.freqPeak[3];
    data[14] = eq.gainPeak[0];
    data[15] = eq.gainPeak[1];
    data[16] = eq.gainPeak[2];
    data[17] = eq.gainPeak[3];
    // write filterData
    data[18] = filter[0].active;
    data[19] = filter[0].type;
    data[20] = filter[0].freq;
    data[21] = filter[0].res;
    data[22] = filter[0].slope;
    data[23] = filter[0].dry;
    data[24] = filter[0].wet;
    data[25] = filter[1].active;
    data[26] = filter[1].type;
    data[27] = filter[1].freq;
    data[28] = filter[1].res;
    data[29] = filter[1].slope;
    data[30] = filter[1].dry;
    data[31] = filter[1].wet;
    // write effectData
    data[32] = effect[0].active;
    data[33] = effect[0].type;
    data[34] = effect[1].active;
    data[35] = effect[1].type;
    for (uint8_t i = 0; i < kEffectLibrarySize; i++) {
        for (uint8_t j = 0; j < kSubEffectLibrarySize; j++) {
            uint16_t baseNum = 36 + (45 * i) + (5 * j);
            data[baseNum + 0] = effect[i].subEffect[j].aData;
            data[baseNum + 1] = effect[i].subEffect[j].bData;
            data[baseNum + 2] = effect[i].subEffect[j].cData;
            data[baseNum + 3] = effect[i].subEffect[j].dData;
            data[baseNum + 4] = effect[i].subEffect[j].eData;
        }
    }
    // write reverbData
    data[126] = reverb.active;
    data[127] = reverb.size;
    data[128] = reverb.decay;
    data[129] = reverb.preDelay;
    data[130] = reverb.surround;
    data[131] = reverb.dry;
    data[132] = reverb.wet;
    // write layerData
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        uint16_t offset = 200 + (i * 7);
        data[offset + 0] = layer.volume;
        data[offset + 1] = layer.speed;
        data[offset + 2] = layer.reverse;
        data[offset + 3] = layer.normalize;
        data[offset + 4] = layer.mute;
        data[offset + 5] = layer.fill;
        data[offset + 6] = layer.style;
    }
    // write instData & sampleData
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        uint16_t offset = 300 + (i * 100);

        // write instData
        if (layer.instLoaded != -1) {
            data[offset] = 'x';
            for (uint8_t j = 0; j < kFileNameSize; j++) {
                data[offset + 1 + j] = layer.instLoadedData.nameLong[j];
            }
        } else {
            data[offset] = 'o';
        }
        // write sampleData
        if (layer.sampleLoaded != -1) {
            data[offset + 50] = 'x';
            for (uint8_t j = 0; j < kFileNameSize; j++) {
                data[offset + 50 + 1 + j] = layer.sampleLoadedData.nameLong[j];
            }
        } else {
            data[offset + 50] = 'o';
        }
    }

    // disable keyboard
    keyboard_disable();
    // write data to sdcard
    if (sd_checkDrumkit(kitNum_) == SD_OK) {
        if (f_open(&sd.file, drumkitName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            if (f_write(&sd.file, kDrumkitRef, strlen(kDrumkitRef), &sd.byteswritten) == FR_OK) {
                const uint16_t writeByteSize = 512;
                const uint16_t drumkitByteSize = kDrumkitByteSize;
                uint16_t writeCountTotal = (drumkitByteSize / writeByteSize);
                uint16_t remainderByteSize = (drumkitByteSize % writeByteSize);
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < writeCountTotal; i++) {
                    if (f_write(&sd.file, (data + pointerOffset), writeByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                    pointerOffset += 512;
                }

                if (remainderByteSize) {
                    if (f_write(&sd.file, (data + pointerOffset), remainderByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                }

                if (errorCount == 0) {
                    if (f_write(&sd.file, eof, 3, &sd.byteswritten) == FR_OK) {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_clearDrumkit(bool mode_, uint8_t kitNum_) {
    SdResult result = SD_ERROR;

    char eof[] = "EOF";
    char drumkitName[50];
    char drumkitNum[4];
    sprintf(drumkitNum, "%03d", (kitNum_ + 1));
    strcpy(drumkitName, kDrumkitStart);
    strcat(drumkitName, drumkitNum);
    strcat(drumkitName, kDrumkitEnd);

    // 000.000 *              0001 byte
    // 000.001 main           0132 bytes
    // 000.200 layer          0070 bytes
    // 000.300 inst&sample    1000 bytes
    // total size             1300 bytes

    const char data[kDrumkitByteSize + 1] = "-                                                                                                                                                                                                                                                                                                           o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 o                                                 ";

    // disable keyboard
    keyboard_disable();
    // clear drumkit
    if (sd_checkDrumkit(kitNum_) == SD_OK) {
        if (f_open(&sd.file, drumkitName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
            if (f_write(&sd.file, kDrumkitRef, strlen(kDrumkitRef), &sd.byteswritten) == FR_OK) {
                const uint16_t writeByteSize = 512;
                const uint16_t drumkitByteSize = kDrumkitByteSize;
                uint16_t writeCountTotal = (drumkitByteSize / writeByteSize);
                uint16_t remainderByteSize = (drumkitByteSize % writeByteSize);
                uint32_t pointerOffset = 0;
                uint32_t errorCount = 0;

                for (uint16_t i = 0; i < writeCountTotal; i++) {
                    if (f_write(&sd.file, (data + pointerOffset), writeByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                    pointerOffset += 512;
                }

                if (remainderByteSize) {
                    if (f_write(&sd.file, (data + pointerOffset), remainderByteSize, &sd.byteswritten) != FR_OK)
                        errorCount += 1;
                }

                if (errorCount == 0) {
                    if (f_write(&sd.file, eof, strlen(eof), &sd.byteswritten) == FR_OK) {
                        result = SD_OK;
                    }
                }
            }
        }
        f_close(&sd.file);
    }
    // enable keyboard
    keyboard_enable();
    return result;
}

SdResult Controller::sd_checkSamplesInUse() {
    SdResult result;
    uint8_t missingSample = 0;
    bool instMissing[kLayerLibrarySize] = {false};
    bool sampleMissing[kLayerLibrarySize] = {false};

    // disable keyboard
    keyboard_disable();
    // check samples
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        // check instData
        if ((layer.instLoaded != -1)) {
            if ((f_open(&sd.file, "System/Data/Inst.lib", FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                char searchName[kFileNameSize + 1];
                bool searchActive = true;
                uint16_t searchCounter = 0;

                while (searchActive) {
                    memset(searchName, 0x00, kFileNameSize);
                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);
                    if (strcmp(layer.instLoadedData.nameLong, searchName) == 0) {
                        layer.instLoaded = searchCounter;
                        instMissing[i] = false;
                        searchActive = false;
                    } else if (searchCounter >= instLibrarySize) {
                        instMissing[i] = true;
                        searchActive = false;
                    }
                    searchCounter += 1;
                }
            } else {
                instMissing[i] = true;
            }
            f_close(&sd.file);
            if (instMissing[i]) {
                layerInst_setInstSelected(i, -1);
                layerInst_setInstLoaded(i);
                layerInst_setSampleSelected(i, -1);
                layerInst_setSampleLoaded(i);
            }
        }
        // check sampleData
        if ((layer.instLoaded != -1) && (layer.sampleLoaded != -1) && (!instMissing[i])) {
            char instFileName[kFileNameSize + 15] = "System/Data/";
            strncat(instFileName, layer.instLoadedData.nameLong, kFileNameSize);
            strncat(instFileName, ".ins", 4);
            if ((f_open(&sd.file, instFileName, FA_READ) == FR_OK) && (f_lseek(&sd.file, 25) == FR_OK)) {
                char searchName[kFileNameSize + 1];
                bool searchActive = true;
                uint16_t searchCounter = 0;
                while (searchActive) {
                    memset(searchName, 0x00, kFileNameSize);
                    f_read(&sd.file, searchName, kFileNameSize, &sd.bytesread);
                    if (strcmp(layer.sampleLoadedData.nameLong, searchName) == 0) {
                        layer.sampleLoaded = searchCounter;
                        sampleMissing[i] = false;
                        searchActive = false;
                    } else if (searchCounter >= sampleLibrarySize[layer.instLoaded]) {
                        sampleMissing[i] = true;
                        searchActive = false;
                    }
                    searchCounter += 1;
                }
            } else {
                sampleMissing[i] = true;
            }
            f_close(&sd.file);
            if (sampleMissing[i]) {
                layerInst_setInstSelected(i, -1);
                layerInst_setInstLoaded(i);
                layerInst_setSampleSelected(i, -1);
                layerInst_setSampleLoaded(i);
            }
        }
    }

    for (uint8_t j = 0; j < kLayerLibrarySize; j++) {
        if (instMissing[j])
            missingSample += 1;
        if (sampleMissing[j])
            missingSample += 1;
    }

    if (missingSample) {
        (missingSample == 1) ? alertType = ALERT_MISSINGSAMPLE : alertType = ALERT_MISSINGSAMPLES;
        lcd_drawAlert();
        HAL_Delay(1000);
        lcd_clearAlert();
        result = SD_ERROR;
    } else {
        result = SD_OK;
    }

    f_open(&sd.file, "System/Data/Inst.lib", FA_READ);
    f_close(&sd.file);

    // enable keyboard
    keyboard_enable();
    return result;
}

FRESULT Controller::sd_createDirectory(const char *path) {
    FRESULT res;
    res = f_mkdir(path);
    return res;
}

FRESULT Controller::sd_deleteDirectory(const char *path) {
    FRESULT result;
    DIR dir;
    FILINFO fileInfo;
    char file[64] = "";
    bool listFile = true;

    result = f_opendir(&dir, path);
    if (result)
        return result;

    while (listFile) {
        result = f_readdir(&dir, &fileInfo);
        if ((result == FR_OK) && (fileInfo.fname[0] != 0)) {
            memset(file, 0x00, strlen(file));
            sprintf((char *)file, "%s/%s", path, fileInfo.fname);
            (fileInfo.fattrib & AM_DIR) ? sd_deleteDirectory(file) : f_unlink(file);
        } else {
            listFile = false;
        }
    }

    f_closedir(&dir);
    f_unlink(path);
    return result;
}

/* Sdram functions -------------------------------------------------------------*/

void Controller::sdram_write16BitAudio(uint32_t ramAddress_, int16_t data_) {
    *(__IO int16_t *)(ramAddress_) = data_;
}

int16_t Controller::sdram_read16BitAudio(uint32_t ramAddress_) {
    return (int16_t)(*(__IO int16_t *)(ramAddress_));
}

void Controller::sdram_write24BitAudio(uint32_t ramAddress_, int32_t data_) {
    uint8_t a = (uint8_t)((data_) & 0xFF);
    uint8_t b = (uint8_t)((data_ >> 8) & 0xFF);
    uint8_t c = (uint8_t)((data_ >> 16) & 0xFF);

    *(__IO uint8_t *)(ramAddress_) = a;
    *(__IO uint8_t *)(ramAddress_ + 1) = b;
    *(__IO uint8_t *)(ramAddress_ + 2) = c;
}

int32_t Controller::sdram_read24BitAudio(uint32_t ramAddress_) {
    volatile uint8_t *ptrA = (volatile uint8_t *)(ramAddress_);
    volatile uint8_t *ptrB = (volatile uint8_t *)(ramAddress_ + 1);
    volatile uint8_t *ptrC = (volatile uint8_t *)(ramAddress_ + 2);
    uint8_t d;
    (*ptrC >> 7) ? d = 0xFF : d = 0x00;

    int32_t audioData = (int32_t)((d << 24) | ((*ptrC) << 16) | ((*ptrB) << 8) | (*ptrA));
    return audioData;
}

void Controller::sdram_fadeOut24BitAudio(uint32_t ramAddress_, uint32_t sampleSize_, uint16_t fadeOutSize_) {
    uint32_t address = ramAddress_ + (3 * (sampleSize_ - fadeOutSize_));
    float decrement = 1.0f / fadeOutSize_;
    float multiplier = 1.0f;
    for (uint16_t i = 0; i < fadeOutSize_; i++) {
        uint32_t sampleAddress = address + (3 * i);
        int32_t input = sdram_read24BitAudio(sampleAddress);
        int32_t output = (int32_t)(input * multiplier);
        sdram_write24BitAudio(sampleAddress, output);
        multiplier -= decrement;
    }
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

void Controller::lcd_update() {
    lcd_drawPlay();
    lcd_drawIcon();
    lcd_drawText();
    lcd_drawLimitAlert();
    lcd_drawBankShift();
    lcd_drawCountDown();
    lcd_drawTransition();
}

void Controller::lcd_drawLogo() {
    if (animation) {
        const RGB16Color *indexPtr = (const RGB16Color *)(RAM_IMAGE_LOGO_PALETTE_ADDRESS);
        const uint8_t *dataPtr = (const uint8_t *)(RAM_IMAGE_LOGO_DATA_ADDRESS);
        lcd.fadeRGB16Image(indexPtr, dataPtr, kImageLogoPalette, kImageLogoX, kImageLogoY, kImageLogoWidth, kImageLogoHeight, true, 40, 10);
        HAL_Delay(1000);
        lcd.fadeRGB16Image(indexPtr, dataPtr, kImageLogoPalette, kImageLogoX, kImageLogoY, kImageLogoWidth, kImageLogoHeight, false, 40, 10);
        HAL_Delay(500);
        lcd.setForeColor(WHITE);
        lcd.setBackColor(BLACK);
        lcd.clearScreen();
    }
}

void Controller::lcd_drawPage() {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    uint8_t step = 20;
    uint8_t delay = 15;

    indexPtr = (const RGB16Color *)(RAM_IMAGE_MENU_PALETTE_ADDRESS);
    dataPtr = (const uint8_t *)(RAM_IMAGE_MENU_DATA_ADDRESS);

    if (animation) {
        lcd.fadeRGB16Image(indexPtr, dataPtr, 64, kImageMenuX, kImageMenuY, kImageMenuWidth, kImageMenuHeight, true, step, delay);
    } else {
        lcd.drawRGB16Image(indexPtr, dataPtr, 64, kImageMenuX, kImageMenuY, kImageMenuWidth, kImageMenuHeight);
    }

    for (uint8_t i = 0; i < 10; i++) {
        uint16_t y = kImageLayerY[i];
        indexPtr = (const RGB16Color *)(RAM_IMAGE_LAYER_PALETTE_ADDRESS + (i * 128));
        dataPtr = (const uint8_t *)(RAM_IMAGE_LAYER_DATA_ADDRESS);
        uint16_t height;
        (i < 9) ? height = kImageLayerHeight : height = 19;
        if (animation) {
            lcd.fadeRGB16Image(indexPtr, dataPtr, 64, kImageLayerX, y, kImageLayerWidth, height, true, step, delay);
        } else {
            lcd.drawRGB16Image(indexPtr, dataPtr, 64, kImageLayerX, y, kImageLayerWidth, height);
        }
    }

    lcd_drawLayerTab();
}

// alert functions

void Controller::lcd_drawAlert() {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_ALERT_PALETTE_ADDRESS);

    dataPtr = (const uint8_t *)(RAM_ICON_ALERT_L_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconAlertX[0], kIconAlertY, kIconAlertWidth, kIconAlertHeight);

    dataPtr = (const uint8_t *)(RAM_ICON_ALERT_R_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconAlertX[1], kIconAlertY, kIconAlertWidth, kIconAlertHeight);

    lcd.setBackColor(BLACK);
    lcd.setForeColor(kLayerColorPalette[4]);
    lcd.clearRect(334, 260, 186, 17);

    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);

    const char *alertText;

    switch (alertType) {
    case ALERT_MEASUREUP:
    case ALERT_MEASUREDOWN:
    case ALERT_BARUP:
    case ALERT_BARDOWN:
    case ALERT_QUANTIZEUP:
    case ALERT_QUANTIZEDOWN:
        alertText = kAlertTextResetPlay;
        break;

    case ALERT_NEWFILE:
        alertText = kAlertTextNewFile;
        break;

    case ALERT_LOADFILE:
        alertText = kAlertTextLoadFile;
        break;

    case ALERT_SAVEFILE:
        alertText = kAlertTextSaveFile;
        break;

    case ALERT_CLEARFILE:
        alertText = kAlertTextClearFile;
        break;

    case ALERT_OVERWRITEFILE:
        alertText = kAlertTextOverwriteFile;
        break;

    case ALERT_NEWDRUMKIT:
        alertText = kAlertTextNewDrumkit;
        break;

    case ALERT_LOADDRUMKIT:
        alertText = kAlertTextLoadDrumkit;
        break;

    case ALERT_SAVEDRUMKIT:
        alertText = kAlertTextSaveDrumkit;
        break;

    case ALERT_CLEARDRUMKIT:
        alertText = kAlertTextClearDrumkit;
        break;

    case ALERT_OVERWRITEDRUMKIT:
        alertText = kAlertTextOverwriteDrumkit;
        break;

    case ALERT_LOADSUCCESS:
        alertText = kAlertTextLoadSuccess;
        break;

    case ALERT_LOADERROR:
        alertText = kAlertTextLoadError;
        break;

    case ALERT_SAVESUCCESS:
        alertText = kAlertTextSaveSuccess;
        break;

    case ALERT_SAVEERROR:
        alertText = kAlertTextSaveError;
        break;

    case ALERT_CLEARSUCCESS:
        alertText = kAlertTextClearSuccess;
        break;

    case ALERT_CLEARERROR:
        alertText = kAlertTextClearError;
        break;

    case ALERT_MISSINGSAMPLE:
        alertText = kAlertTextMissingSample;
        break;

    case ALERT_MISSINGSAMPLES:
        alertText = kAlertTextMissingSamples;
        break;
    }

    lcd.drawText(alertText, strlen(alertText), 427, 264);
}

void Controller::lcd_clearAlert() {
    lcd.setForeColor(kLayerColorPalette[4]);
    lcd.fillRect(317, 260, 220, 17);
    lcd_drawSong(4, activeBankNum);
}

// limit alert functions

void Controller::lcd_drawLimitAlert() {
    if (limitAlertShowFlag) {
        limitAlertShowFlag = false;
        stopLimitAlertTimer();
        startLimitAlertTimer();
        if (!limitAlertActive) {
            limitAlertActive = true;
            lcd.setBackColor(BLACK);
            lcd.setFont(FONT_07x09);
            lcd.setAlignment(CENTER);
            lcd.setForeColor(kLayerColorPalette[9]);
            lcd.drawText("!", 1, kLimitAlertX, kLimitAlertY);
        }
    } else if (limitAlertClearFlag) {
        limitAlertClearFlag = false;
        limitAlertActive = false;
        lcd.setFont(FONT_07x09);
        lcd.setAlignment(CENTER);
        lcd.setForeColor(kLayerColorPalette[9]);
        lcd.setBackColor(BLACK);
        lcd.drawText(" ", 1, kLimitAlertX, kLimitAlertY);
    }
}

// sd functions

void Controller::lcd_drawSdDataIntro() {
    lcd.setForeColor(kLayerColorPalette[9]);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_05x07);
    lcd.setAlignment(CENTER);
    lcd.drawText("[ ANALYZING SDCARD ]", 20, 427, 458);
}

void Controller::lcd_drawSdData() {
    lcd.setBackColor(BLACK);
    lcd.setForeColor(kLayerColorPalette[9]);
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_05x07);

    char tab[] = "   |   ";
    char dot = '.';
    char textTitle[] = "DRUMBOY V";
    char textFree[] = "MB FREE";
    char textFile[] = " FILES";
    char textDrumkit[] = " DRUMKITS";
    char textInst[] = " INSTS";
    char textSample[] = " SAMPLES";

    char sdText[110] = {};
    char versionMajor[3];
    char versionMinor[3];
    sprintf(versionMajor, "%01d", kVersionMajor);
    sprintf(versionMinor, "%02d", kVersionMinor);
    strncpy(sdText, textTitle, 9);
    strncat(sdText, versionMajor, 2);
    strncat(sdText, &dot, 1);
    strncat(sdText, versionMinor, 2);
    strcat(sdText, tab);

    if (sd.ready) {
        char numFree[6];
        sprintf(numFree, "%05d", sd.freeSpace);
        strcat(sdText, numFree);
    } else {
        strcat(sdText, "-----");
    }
    strcat(sdText, textFree);
    strcat(sdText, tab);

    if (sd.ready) {
        char numFile[4];
        sprintf(numFile, "%03d", fileLibrarySize);
        strcat(sdText, numFile);
    } else {
        strcat(sdText, "---");
    }
    strcat(sdText, textFile);
    strcat(sdText, tab);

    if (sd.ready) {
        char numDrumkit[4];
        sprintf(numDrumkit, "%03d", drumkitLibrarySize);
        strcat(sdText, numDrumkit);
    } else {
        strcat(sdText, "---");
    }
    strcat(sdText, textDrumkit);
    strcat(sdText, tab);

    if (sd.ready) {
        char numInst[4];
        sprintf(numInst, "%04d", instLibrarySize);
        strcat(sdText, numInst);
    } else {
        strcat(sdText, "----");
    }
    strcat(sdText, textInst);
    strcat(sdText, tab);

    if (sd.ready) {
        char numSample[4];
        if (totalSampleLibrarySize < 10000) {
            sprintf(numSample, "%04d", totalSampleLibrarySize);
            strcat(sdText, numSample);
        } else {
            sprintf(numSample, "%04d", totalSampleLibrarySize / 1000);
            strcat(sdText, numSample);
            strcat(sdText, "K");
        }
    } else {
        strcat(sdText, "----");
    }
    strcat(sdText, textSample);

    lcd.drawText(sdText, strlen(sdText), 427, 458);
}

void Controller::lcd_clearSdData() {
    lcd.setForeColor(BLACK);
    lcd.fillRect(20, 458, 814, 7);
}

void Controller::lcd_drawSdAlert(SdResult result_) {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_ALERT_PALETTE_ADDRESS);

    dataPtr = (const uint8_t *)(RAM_ICON_ALERT_L_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconAlertX[0], kIconAlertY, kIconAlertWidth, kIconAlertHeight);

    dataPtr = (const uint8_t *)(RAM_ICON_ALERT_R_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconAlertX[1], kIconAlertY, kIconAlertWidth, kIconAlertHeight);

    lcd.setBackColor(BLACK);
    lcd.setForeColor(kLayerColorPalette[4]);
    lcd.clearRect(334, 260, 186, 17);

    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);

    const char *alertPtr;

    switch (result_) {
    case SD_ERROR_DETECT:
        alertPtr = kSdAlertTextInsert;
        break;

    case SD_ERROR_MOUNT:
        alertPtr = kSdAlertTextFormat;
        break;

    case SD_ERROR_SERIAL:
        alertPtr = kSdAlertTextSerial;
        break;

    case SD_ERROR_SYSTEMFOLDER:
        alertPtr = kSdAlertTextSystemFolder;
        break;

    case SD_ERROR_SAMPLEFOLDER:
        alertPtr = kSdAlertTextSampleFolder;
        break;

    case SD_ERROR_SYSTEMFILE:
        alertPtr = kSdAlertTextSystemFile;
        break;
    }

    lcd.drawText(alertPtr, strlen(alertPtr), 427, 264);
}

void Controller::lcd_clearSdAlert() {
    lcd.setForeColor(kLayerColorPalette[4]);
    lcd.fillRect(317, 260, 220, 17);
    lcd_drawSong(4, activeBankNum);
}

void Controller::lcd_drawInitSdAlert(SdResult result_) {
    lcd.drawInitSdAlert(result_);
}

void Controller::lcd_clearInitSdAlert() { lcd.clearInitSdAlert(); }

// layer functions

void Controller::lcd_drawLayerTab() {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr1;
    const uint8_t *dataPtr2;

    switch (layerTab) {
    case TAB_0:
        dataPtr1 = (const uint8_t *)(RAM_ICON_LAYER_S_ON_0_DATA_ADDRESS);
        dataPtr2 = (const uint8_t *)(RAM_ICON_LAYER_S_OFF_DATA_ADDRESS);
        break;

    case TAB_1:
        dataPtr1 = (const uint8_t *)(RAM_ICON_LAYER_S_OFF_DATA_ADDRESS);
        dataPtr2 = (const uint8_t *)(RAM_ICON_LAYER_S_ON_0_DATA_ADDRESS);
        break;
    }

    for (uint8_t i = 0; i < 5; i++) {
        indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (i * 128));
        lcd.drawRGB16Image(indexPtr, dataPtr1, 64, kIconLayerTabX, 127 + kIconLayerTabY + (i * kImageLayerHeight), kIconLayerWidth, kIconLayerHeight);
    }

    for (uint8_t i = 0; i < 5; i++) {
        indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + ((i + 5) * 128));
        lcd.drawRGB16Image(indexPtr, dataPtr2, 64, kIconLayerTabX, 127 + kIconLayerTabY + ((i + 5) * kImageLayerHeight), kIconLayerWidth, kIconLayerHeight);
    }

    if (selectedLayerNum != -1) {
        indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (selectedLayerNum * 128));
        dataPtr1 = (const uint8_t *)(RAM_ICON_LAYER_S_ON_1_DATA_ADDRESS);
        lcd.drawRGB16Image(indexPtr, dataPtr1, 64, kIconLayerTabX, 127 + kIconLayerTabY + (selectedLayerNum * kImageLayerHeight), kIconLayerWidth, kIconLayerHeight);
    }
}

void Controller::lcd_drawLayerSelect() {
    const RGB16Color *indexPtr =
        (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (selectedLayerNum * 128));
    const uint8_t *dataPtr = (const uint8_t *)(RAM_ICON_LAYER_S_ON_1_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconLayerTabX, 127 + kIconLayerTabY + (selectedLayerNum * kImageLayerHeight), kIconLayerWidth, kIconLayerHeight);
}

void Controller::lcd_clearLayerSelect() {
    const RGB16Color *indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (selectedLayerNum * 128));
    const uint8_t *dataPtr;
    (((selectedLayerNum < 5) && (layerTab == TAB_0)) || ((selectedLayerNum >= 5) && (layerTab == TAB_1))) ? dataPtr = (const uint8_t *)(RAM_ICON_LAYER_S_ON_0_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_LAYER_S_OFF_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconLayerTabX, 127 + kIconLayerTabY + (selectedLayerNum * kImageLayerHeight), kIconLayerWidth, kIconLayerHeight);
}

// menu functions

void Controller::lcd_drawMenuIcon(Menu menu_) {
    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_10x14);
    lcd.setAlignment(LEFT);
    lcd.setSpacing(4);

    switch (menu_) {
    case MAIN_MENU:
        lcd.clearRect(kMenuIconX, kMenuIconY, 38, 15);
        for (uint8_t i = 0; i < 5; i++) {
            for (uint8_t j = 0; j < 2; j++) {
                lcd.fillRect(kMenuIconX + 1 + (i * 8), kMenuIconY + 1 + (j * 8), 4, 4);
            }
        }
        break;

    case FILE_MENU:
        lcd.drawText("FIL", 3, kMenuIconX, kMenuIconY);
        break;

    case DRUMKIT_MENU:
        lcd.drawText("DRU", 3, kMenuIconX, kMenuIconY);
        break;

    case SYSTEM_MENU:
        lcd.drawText("MAS", 3, kMenuIconX, kMenuIconY);
        break;

    case RHYTHM_MENU:
        lcd.drawText("RHY", 3, kMenuIconX, kMenuIconY);
        break;

    case METRO_MENU:
        lcd.drawText("MET", 3, kMenuIconX, kMenuIconY);
        break;

    case EQ_MENU:
        lcd.drawText("PEQ", 3, kMenuIconX, kMenuIconY);
        break;

    case FILTER_0_MENU:
    case FILTER_1_MENU:
        lcd.drawText("FIL", 3, kMenuIconX, kMenuIconY);
        break;

    case EFFECT_0_MENU:
    case EFFECT_1_MENU:
        lcd.drawText("EFF", 3, kMenuIconX, kMenuIconY);
        break;

    case REVERB_MENU:
        lcd.drawText("REV", 3, kMenuIconX, kMenuIconY);
        break;

    case LAYER_INST_MENU:
        lcd.drawText("INS", 3, kMenuIconX, kMenuIconY);
        break;

    case LAYER_SONG_MENU:
        lcd.drawText("SON", 3, kMenuIconX, kMenuIconY);
        break;
    }
}

void Controller::lcd_drawFileBox(uint8_t menuTab_, FileStatus status_) {
    uint16_t xPos = kMenuBoxX[menuTab_];
    uint16_t yPos = kMenuBoxY;

    switch (status_) {
    case FILE_NONE:
        lcd.setForeColor(MAGENTA);
        lcd.setBackColor(BLACK);
        lcd.clearRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        lcd.drawRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        break;

    case FILE_MISSING:
        lcd.setForeColor(MAGENTA);
        lcd.setBackColor(BLACK);
        lcd.clearRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        lcd.drawRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        break;

    case FILE_INCOMPATIBLE:
        lcd.setForeColor(MAGENTA);
        lcd.setBackColor(BLACK);
        lcd.clearRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        lcd.fillRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        break;

    case FILE_INACTIVE:
        lcd.setForeColor(WHITE);
        lcd.setBackColor(BLACK);
        lcd.clearRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        lcd.drawRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        break;

    case FILE_ACTIVE:
        lcd.setForeColor(WHITE);
        lcd.setBackColor(BLACK);
        lcd.fillRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
        break;
    }
}

void Controller::lcd_clearFileBox(uint8_t menuTab_) {
    uint16_t xPos = kMenuBoxX[menuTab_];
    uint16_t yPos = kMenuBoxY;
    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);
    lcd.clearRect(xPos, yPos, kMenuBoxWidth, kMenuBoxHeight);
}

void Controller::lcd_transitionMenu() {
    uint8_t preMenuType;
    uint8_t menuType;

    switch (preMenu) {
    case MAIN_MENU:
        preMenuType = 0;
        break;

    case FILE_MENU:
    case DRUMKIT_MENU:
    case RHYTHM_MENU:
    case METRO_MENU:
        preMenuType = 1;
        break;

    case LAYER_INST_MENU:
        preMenuType = 2;
        break;

    case SYSTEM_MENU:
    case EQ_MENU:
    case FILTER_0_MENU:
    case FILTER_1_MENU:
    case EFFECT_0_MENU:
    case EFFECT_1_MENU:
    case REVERB_MENU:
        preMenuType = 3;
        break;

    case LAYER_SONG_MENU:
        preMenuType = 4;
        break;
    }

    switch (menu) {
    case MAIN_MENU:
        menuType = 0;
        break;

    case FILE_MENU:
    case DRUMKIT_MENU:
    case RHYTHM_MENU:
    case METRO_MENU:
        menuType = 1;
        break;

    case LAYER_INST_MENU:
        menuType = 2;
        break;

    case SYSTEM_MENU:
    case EQ_MENU:
    case FILTER_0_MENU:
    case FILTER_1_MENU:
    case EFFECT_0_MENU:
    case EFFECT_1_MENU:
    case REVERB_MENU:
        menuType = 3;
        break;

    case LAYER_SONG_MENU:
        menuType = 4;
        break;
    }

    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);

    switch (preMenuType) {
    case 0: // main
        lcd.clearRect(kMenuLine4X[0] + 1, 29, 161, 61);
        lcd.clearRect(kMenuLine4X[1] + 1, 29, 161, 61);
        lcd.clearRect(kMenuLine4X[2] + 1, 29, 161, 61);
        lcd.clearRect(kMenuLine4X[3] + 1, 29, 161, 61);
        break;

    case 1: // file / drumkit / rhythm / metro
        if (menuType != preMenuType) {
        }
        switch (preMenu) {
        case FILE_MENU:
            fileStatus = FILE_NONE;
            for (uint8_t i = 1; i < 4; i++) {
                lcd_clearFileBox(i);
            }
            break;

        case DRUMKIT_MENU:
            drumkitStatus = FILE_NONE;
            for (uint8_t i = 1; i < 4; i++) {
                lcd_clearFileBox(i);
            }
            break;

        case RHYTHM_MENU:
            if (menu != MAIN_MENU) {
                lcd.clearRect(kMenuData4X[0] - 42, kMenuHeaderY + 1, 42, 7);
                lcd.clearRect(kMenuData4X[1] - 10, kMenuHeaderY, 10, 10);
                lcd.clearRect(kMenuData4X[2] - 10, kMenuHeaderY, 10, 10);
                lcd.clearRect(kMenuData4X[3] - 10, kMenuHeaderY, 10, 10);
            }
            rhythm.measureLock = true;
            rhythm.barLock = true;
            rhythm.quantizeLock = true;
            break;

        default:
            break;
        }

        break;

    case 2: // inst
        if (menuType != preMenuType) {
            lcd.clearRect(kMenuData4X[0] - 30, kMenuHeaderY + 1, 30, 7);
            lcd.clearRect(kMenuData4X[1] - 30, kMenuHeaderY + 1, 30, 7);
            lcd.clearRect(kMenuData4X[1] - 80, kMenuHeaderY + 23, 80, 7);
            lcd.clearRect(kMenuLine4X[2], kMenuLineY, 162, 60);
            lcd.clearRect(kMenuLine4X[3], kMenuLineY, 162, 60);
        }
        break;

    case 3: // system / eq / filter / effect / reverb
        if (menuType != preMenuType) {
            lcd.clearRect(kMenuLine4X[1] + 1, 29, 161, 61);
            lcd.clearRect(kMenuLine4X[2] + 1, 29, 161, 61);
            lcd.clearRect(kMenuLine4X[3] + 1, 29, 161, 61);
        }
        break;

    case 4: // song
        if (menuType != preMenuType) {
            lcd.clearRect(kMenuLine4X[1] + 1, 29, 487, 61);
            lcd.clearRect(kMenuData4X[0] - 35, kMenuHeaderY, 35, 9);
        }
        break;
    }

    switch (menuType) {
    case 0: // main
        lcd.clearRect(182 + 1, 29, 650, 61);
        for (uint8_t i = 1; i < 8; i++) {
            lcd.drawVLine(kMenuLine8X[i], kMenuLineY, kMenuLineHeight);
        }
        break;

    case 1: // file / drumkit / rhythm / metro
        for (uint8_t i = 1; i < 4; i++) {
            lcd.drawVLine(kMenuLine4X[i], kMenuLineY, kMenuLineHeight);
        }
        break;

    case 2: // inst
        if (menuType != preMenuType) {
            lcd.clearRect(kMenuLine4X[2], kMenuHeaderY, 324, 54);
            lcd.drawVLine(kMenuLine4X[1], kMenuLineY, kMenuLineHeight);
            for (int i = 3; i < 7; i++) {
                lcd.drawVLine(kMenuLine8X[i + 1], kMenuLineY, kMenuLineHeight);
            }
        }
        break;

    case 3: // system / eq / filter / effect / reverb
        if ((menuType != preMenuType) || (preMenu == EQ_MENU) || (menu == EQ_MENU)) {
            lcd.clearRect(kMenuLine4X[1] + 1, 29, 161, 61);
            lcd.clearRect(kMenuLine4X[2] + 1, 29, 161, 61);
            lcd.clearRect(kMenuLine4X[3] + 1, 29, 161, 61);
        }

        for (int i = 2; i < 8; i++) {
            lcd.drawVLine(kMenuLine8X[i], kMenuLineY, kMenuLineHeight);
        }
        break;

    case 4: // song
        if (preMenu != LAYER_SONG_MENU) {
            preFill = -1;
            lcd.clearRect(kMenuLine4X[1] + 1, 29, 487, 61);
        }
        lcd.drawVLine(kMenuLine4X[1], kMenuLineY, kMenuLineHeight);
        break;
    }
}

void Controller::lcd_transitionSelect() {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_SELECT_PALETTE_ADDRESS);

    // clear premenutab selection
    if (preMenuTab != -1) {
        uint8_t refMenu;
        if (preMenu == menu) {
            refMenu = menu;
        } else {
            refMenu = preMenu;
            preMenu = menu;
        }
        dataPtr = (const uint8_t *)(RAM_ICON_SELECT_OFF_DATA_ADDRESS);
        switch (refMenu) {
        case EQ_MENU:
            if (preMenuTab == 0) {
                lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect8X[preMenuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            } else {
                lcd.setForeColor(WHITE);
                lcd.setBackColor(BLACK);
                lcd.setAlignment(LEFT);
                lcd.setFont(FONT_05x07);
                lcd.drawText(" ", 1, kMenuHeader8X[preMenuTab] + 24, 52);
                lcd.drawText(" ", 1, kMenuHeader8X[preMenuTab] + 24, 66);
                lcd.drawText(" ", 1, kMenuHeader8X[preMenuTab] + 24, 80);
            }
            break;

        case SYSTEM_MENU:
        case FILTER_0_MENU:
        case FILTER_1_MENU:
        case EFFECT_0_MENU:
        case EFFECT_1_MENU:
        case REVERB_MENU:
        case LAYER_INST_MENU:
            lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect8X[preMenuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            break;

        default:
            lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect4X[preMenuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            break;
        }
    }

    // draw menutab selection
    if (menuTab != -1) {
        dataPtr = (const uint8_t *)(RAM_ICON_SELECT_ON_DATA_ADDRESS);
        switch (menu) {
        case EQ_MENU:
            if (menuTab == 0) {
                lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect8X[menuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            } else {
                lcd.setForeColor(WHITE);
                lcd.setBackColor(BLACK);
                lcd.setAlignment(LEFT);
                lcd.setFont(FONT_05x07);
                (subMenuTab == 0) ? lcd.drawText(">", 1, kMenuHeader8X[menuTab] + 24, 80) : lcd.drawText(" ", 1, kMenuHeader8X[menuTab] + 24, 80);
                (subMenuTab == 1) ? lcd.drawText(">", 1, kMenuHeader8X[menuTab] + 24, 66) : lcd.drawText(" ", 1, kMenuHeader8X[menuTab] + 24, 66);
                (subMenuTab == 2) ? lcd.drawText(">", 1, kMenuHeader8X[menuTab] + 24, 52) : lcd.drawText(" ", 1, kMenuHeader8X[menuTab] + 24, 52);
            }
            break;

        case SYSTEM_MENU:
        case FILTER_0_MENU:
        case FILTER_1_MENU:
        case EFFECT_0_MENU:
        case EFFECT_1_MENU:
        case REVERB_MENU:
        case LAYER_INST_MENU:
            lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect8X[menuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            break;

        default:
            lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconSelect4X[menuTab], kIconSelectY, kIconSelectWidth, kIconSelectHeight);
            break;
        }
    }
}

void Controller::lcd_setMenuHeaderState(RGB16Color color_) {
    lcd.setForeColor(color_);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_07x09);
    lcd.setAlignment(LEFT);
}

void Controller::lcd_setMenuDataState(RGB16Color color_) {
    lcd.setForeColor(color_);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_07x09);
    lcd.setAlignment(RIGHT);
}

void Controller::lcd_setMenuNumState(RGB16Color color_) {
    lcd.setForeColor(color_);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_05x07);
    lcd.setAlignment(RIGHT);
}

void Controller::lcd_setMenuSignState(RGB16Color color_, LcdFont font_) {
    lcd.setForeColor(color_);
    lcd.setBackColor(BLACK);
    lcd.setFont(font_);
    lcd.setAlignment(RIGHT);
}

void Controller::lcd_setMainMenuHeaderState() {
    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);
    lcd.setFont(FONT_05x07);
    lcd.setAlignment(CENTER);
}

void Controller::lcd_setLayerDataState(uint8_t layerNum_) {
    lcd.setForeColor(BLACK);
    lcd.setBackColor(kLayerColorPalette[layerNum_]);
    lcd.setFont(FONT_07x09);
    lcd.setAlignment(LEFT);
}

// main menu functions

void Controller::lcd_drawMainMenu() {
    lcd_drawMenuIcon(MAIN_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMainMenuHeaderState();
    for (uint8_t i = 1; i < 8; i++) {
        lcd.drawVLine(kMenuLine8X[i], kMenuLineY, kMenuLineHeight);
    }

    char title[8][4] = {"TEM", "MEA", "BAR", "QUA", "FL1", "FL2", "EF1", "EF2"};

    for (uint8_t i = 0; i < 8; i++) {
        lcd.drawText(title[i], 3, kMainMenuX[i], kMainMenuHeaderY);
    }

    lcd_drawMain_TempoData();
    lcd_drawMain_MeasureData();
    lcd_drawMain_BarData();
    lcd_drawMain_QuantizeData();
    lcd_drawMain_Filter0Data();
    lcd_drawMain_Filter1Data();
    lcd_drawMain_Effect0Data();
    lcd_drawMain_Effect1Data();
}

void Controller::lcd_drawMain_TempoData() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[0]);
    char kTempoData[3];
    sprintf(kTempoData, "%03d", rhythm.tempo);
    lcd.drawText(kTempoData, 3, kMainMenuX[0], kMainMenuDataY);
}

void Controller::lcd_drawMain_MeasureData() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[1]);
    char kMeasureData[2];
    sprintf(kMeasureData, "%02d", rhythm.measure);
    lcd.drawText(kMeasureData, 2, kMainMenuX[1], kMainMenuDataY);
}

void Controller::lcd_drawMain_BarData() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[2]);
    char kBarData[2];
    sprintf(kBarData, "%02d", rhythm.bar);
    lcd.drawText(kBarData, 2, kMainMenuX[2], kMainMenuDataY);
}

void Controller::lcd_drawMain_QuantizeData() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[3]);
    lcd.drawText(kQuantizeDataLibrary[rhythm.quantize].nameShortR, 2, kMainMenuX[3], kMainMenuDataY);
}

void Controller::lcd_drawMain_Filter0Data() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[4]);
    (filter[0].active) ? lcd.drawText(kFilterTypeDataLibrary[filter[0].type].nameShortL, 3, kMainMenuX[4], kMainMenuDataY) : lcd.drawText("OFF", 3, kMainMenuX[4], kMainMenuDataY);
}

void Controller::lcd_drawMain_Filter1Data() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[5]);
    (filter[1].active) ? lcd.drawText(kFilterTypeDataLibrary[filter[1].type].nameShortL, 3, kMainMenuX[5], kMainMenuDataY) : lcd.drawText("OFF", 3, kMainMenuX[5], kMainMenuDataY);
}

void Controller::lcd_drawMain_Effect0Data() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[7]);
    (effect[0].active) ? lcd.drawText(kEffectTypeDataLibrary[effect[0].type].nameShortL, 3, kMainMenuX[6], kMainMenuDataY) : lcd.drawText("OFF", 3, kMainMenuX[6], kMainMenuDataY);
}

void Controller::lcd_drawMain_Effect1Data() {
    lcd.setAlignment(CENTER);
    lcd.setFont(FONT_07x09);
    lcd.setForeColor(kLayerColorPalette[8]);
    (effect[1].active) ? lcd.drawText(kEffectTypeDataLibrary[effect[1].type].nameShortL, 3, kMainMenuX[7], kMainMenuDataY) : lcd.drawText("OFF", 3, kMainMenuX[7], kMainMenuDataY);
}

// file menu functions

void Controller::lcd_drawFileMenu() {
    lcd_drawMenuIcon(FILE_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderNew, kMenuHeaderTextSize, kMenuHeader4X[0], kMenuHeaderY);
    lcd.drawText(kHeaderLoad, kMenuHeaderTextSize, kMenuHeader4X[1], kMenuHeaderY);
    lcd.drawText(kHeaderSave, kMenuHeaderTextSize, kMenuHeader4X[2], kMenuHeaderY);
    lcd.drawText(kHeaderClear, kMenuHeaderTextSize, kMenuHeader4X[3], kMenuHeaderY);

    lcd_drawFile_NewData();
    lcd_drawFile_LoadData();
    lcd_drawFile_SaveData();
    lcd_drawFile_ClearData();
}

void Controller::lcd_drawFile_NewData() {
    lcd_setMenuDataState(WHITE);
    const char *text;
    (menuTab == 0) ? text = kDataSelect : text = kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[0], kMenuDataY);
}

void Controller::lcd_drawFile_LoadData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char file[11] = "  FILE_";
    char num[4];
    sprintf(num, "%03d", fileMenuCounter + 1);
    strncat(file, num, 3);
    (menuTab == 1) ? text = file : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[1], kMenuDataY);
}

void Controller::lcd_drawFile_SaveData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char file[11] = "  FILE_";
    char num[4];
    sprintf(num, "%03d", fileMenuCounter + 1);
    strncat(file, num, 3);
    (menuTab == 2) ? text = file : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[2], kMenuDataY);
}

void Controller::lcd_drawFile_ClearData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char file[11] = "  FILE_";
    char num[4];
    sprintf(num, "%03d", fileMenuCounter + 1);
    strncat(file, num, 3);
    (menuTab == 3) ? text = file : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[3], kMenuDataY);
}

// drumkit menu functions

void Controller::lcd_drawDrumkitMenu() {
    lcd_drawMenuIcon(DRUMKIT_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderNew, kMenuHeaderTextSize, kMenuHeader4X[0], kMenuHeaderY);
    lcd.drawText(kHeaderLoad, kMenuHeaderTextSize, kMenuHeader4X[1], kMenuHeaderY);
    lcd.drawText(kHeaderSave, kMenuHeaderTextSize, kMenuHeader4X[2], kMenuHeaderY);
    lcd.drawText(kHeaderClear, kMenuHeaderTextSize, kMenuHeader4X[3], kMenuHeaderY);

    lcd_drawDrumkit_NewData();
    lcd_drawDrumkit_LoadData();
    lcd_drawDrumkit_SaveData();
    lcd_drawDrumkit_ClearData();
}

void Controller::lcd_drawDrumkit_NewData() {
    lcd_setMenuDataState(WHITE);
    const char *text;
    (menuTab == 0) ? text = kDataSelect : text = kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[0], kMenuDataY);
}

void Controller::lcd_drawDrumkit_LoadData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char drum[11] = "  DRUM_";
    char num[4];
    sprintf(num, "%03d", drumkitMenuCounter + 1);
    strncat(drum, num, 3);
    (menuTab == 1) ? text = drum : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[1], kMenuDataY);
}

void Controller::lcd_drawDrumkit_SaveData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char drum[11] = "  DRUM_";
    char num[4];
    sprintf(num, "%03d", drumkitMenuCounter + 1);
    strncat(drum, num, 3);
    (menuTab == 2) ? text = drum : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[2], kMenuDataY);
}

void Controller::lcd_drawDrumkit_ClearData() {
    lcd_setMenuDataState(WHITE);
    char *text;
    char drum[11] = "  DRUM_";
    char num[4];
    sprintf(num, "%03d", drumkitMenuCounter + 1);
    strncat(drum, num, 3);
    (menuTab == 3) ? text = drum : text = (char *)kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[3], kMenuDataY);
}

// system menu functions

void Controller::lcd_drawSystemMenu() {
    lcd_drawMenuIcon(SYSTEM_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderVolume, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("PAN ", kMenuHeaderShortTextSize, kMenuHeader8X[2] - 2, kMenuHeaderY);
    lcd.drawText("LIM ", kMenuHeaderShortTextSize, kMenuHeader8X[3] - 2, kMenuHeaderY);
    lcd.drawText("MIDI", kMenuHeaderShortTextSize, kMenuHeader8X[4] - 2, kMenuHeaderY);
    lcd.drawText("MIDI", kMenuHeaderShortTextSize, kMenuHeader8X[5] - 2, kMenuHeaderY);
    lcd.drawText("SYNC", kMenuHeaderShortTextSize, kMenuHeader8X[6] - 2, kMenuHeaderY);
    lcd.drawText("SYNC", kMenuHeaderShortTextSize, kMenuHeader8X[7] - 2, kMenuHeaderY);

    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(" IN", kMenuSignTextSize, kMenuData8X[4] + 2, 60);
    lcd.drawText("OUT", kMenuSignTextSize, kMenuData8X[5] + 2, 60);
    lcd.drawText(" IN", kMenuSignTextSize, kMenuData8X[6] + 2, 60);
    lcd.drawText("OUT", kMenuSignTextSize, kMenuData8X[7] + 2, 60);

    lcd_drawSystem_VolumeData();
    lcd_drawSystem_PanData();
    lcd_drawSystem_LimiterData();
    lcd_drawSystem_MidiInData();
    lcd_drawSystem_MidiOutData();
    lcd_drawSystem_SyncInData();
    lcd_drawSystem_SyncOutData();
}

void Controller::lcd_drawSystem_VolumeData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemVolumeDataLibrary[system.volume].nameLongR, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);
}

void Controller::lcd_drawSystem_PanData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemPanDataLibrary[system.pan].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 2, kMenuDataY);
}

void Controller::lcd_drawSystem_LimiterData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (system.limiter) ? textPtr = kDataShortROn : textPtr = kDataShortROff;
    lcd.drawText(textPtr, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
}

void Controller::lcd_drawSystem_MidiInData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemMidiDataLibrary[system.midiIn].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
}

void Controller::lcd_drawSystem_MidiOutData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemMidiDataLibrary[system.midiOut].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
}

void Controller::lcd_drawSystem_SyncInData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemSyncInDataLibrary[system.syncIn].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
}

void Controller::lcd_drawSystem_SyncOutData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kSystemSyncOutDataLibrary[system.syncOut].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
}

// rhythm menu functions

void Controller::lcd_drawRhythmMenu() {
    lcd_drawMenuIcon(RHYTHM_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderTempo, kMenuHeaderTextSize, kMenuHeader4X[0], kMenuHeaderY);
    lcd.drawText(kHeaderMeasure, kMenuHeaderTextSize, kMenuHeader4X[1], kMenuHeaderY);
    lcd.drawText(kHeaderBar, kMenuHeaderTextSize, kMenuHeader4X[2], kMenuHeaderY);
    lcd.drawText(kHeaderQuantize, kMenuHeaderTextSize, kMenuHeader4X[3], kMenuHeaderY);

    lcd_drawRhythm_TempoData();
    lcd_drawRhythm_MeasureData();
    lcd_drawRhythm_BarData();
    lcd_drawRhythm_QuantizeData();
}

void Controller::lcd_drawRhythm_TempoData() {
    lcd_setMenuDataState(WHITE);
    char kData[kMenuDataTextSize];
    sprintf(kData, "       %03d", rhythm.tempo);
    lcd.drawText(kData, kMenuDataTextSize, kMenuData4X[0], kMenuDataY);

    if (system.sync.slaveMode) {
        lcd_setMenuNumState(MAGENTA);
        lcd.drawText(" SLAVE", 6, kMenuData4X[0], kMenuHeaderY + 1);
    } else if (system.sync.masterMode) {
        lcd_setMenuNumState(YELLOW);
        lcd.drawText("MASTER", 6, kMenuData4X[0], kMenuHeaderY + 1);
    }
}

void Controller::lcd_drawRhythm_MeasureData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kNumberDataLibrary[rhythm.measure].nameLongR, kMenuDataTextSize, kMenuData4X[1], kMenuDataY);

    lcd_setMenuSignState(WHITE, FONT_07x09);
    (rhythm.measureLock) ? lcd.setForeColor(MAGENTA) : lcd.setForeColor(BLACK);
    lcd.drawText("%", 1, kMenuData4X[1], kMenuHeaderY);
}

void Controller::lcd_drawRhythm_BarData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kNumberDataLibrary[rhythm.bar].nameLongR, kMenuDataTextSize, kMenuData4X[2], kMenuDataY);

    lcd_setMenuSignState(WHITE, FONT_07x09);
    (rhythm.measureLock) ? lcd.setForeColor(MAGENTA) : lcd.setForeColor(BLACK);
    lcd.drawText("%", 1, kMenuData4X[2], kMenuHeaderY);
}

void Controller::lcd_drawRhythm_QuantizeData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kQuantizeDataLibrary[rhythm.quantize].nameLongR, kMenuDataTextSize, kMenuData4X[3], kMenuDataY);

    lcd_setMenuSignState(WHITE, FONT_07x09);
    (rhythm.measureLock) ? lcd.setForeColor(MAGENTA) : lcd.setForeColor(BLACK);
    lcd.drawText("%", 1, kMenuData4X[3], kMenuHeaderY);
}

// metronome menu functions

void Controller::lcd_drawMetroMenu() {
    lcd_drawMenuIcon(METRO_MENU);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderMetronome, kMenuHeaderTextSize, kMenuHeader4X[0], kMenuHeaderY);
    lcd.drawText(kHeaderPrecount, kMenuHeaderTextSize, kMenuHeader4X[1], kMenuHeaderY);
    lcd.drawText(kHeaderSample, kMenuHeaderTextSize, kMenuHeader4X[2], kMenuHeaderY);
    lcd.drawText(kHeaderVolume, kMenuHeaderTextSize, kMenuHeader4X[3], kMenuHeaderY);

    lcd_drawMetro_ActiveData();
    lcd_drawMetro_PrecountData();
    lcd_drawMetro_SampleData();
    lcd_drawMetro_VolumeData();
}

void Controller::lcd_drawMetro_ActiveData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (metronome.active) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData4X[0], kMenuDataY);
}

void Controller::lcd_drawMetro_PrecountData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (metronome.precount) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData4X[1], kMenuDataY);
}

void Controller::lcd_drawMetro_SampleData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kMetroSampleDataLibrary[metronome.sample].nameLongR, kMenuDataTextSize, kMenuData4X[2], kMenuDataY);
}

void Controller::lcd_drawMetro_VolumeData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kMetronomeVolumeDataLibrary[metronome.volume].nameLongR, kMenuDataTextSize, kMenuData4X[3], kMenuDataY);
}

// eq menu functions

void Controller::lcd_drawEqMenu() {
    lcd_drawMenuIcon(EQ_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderActive, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);

    lcd_setMenuHeaderState(CYAN);
    for (uint16_t i = 0; i < 6; i++) {
        lcd.drawText(kHeaderEqFreq[i], 2, kMenuHeader8X[i + 2] - 2, kMenuHeaderY);
    }

    lcd.setFont(FONT_05x07);
    for (uint8_t i = 0; i < 6; i++) {
        if ((i > 0) && (i < 5)) {
            lcd.drawText("Q ", 2, kMenuHeader8X[i + 2] - 2, 52);
        }
        lcd.drawText("HZ", 2, kMenuHeader8X[i + 2] - 2, 66);
        lcd.drawText("DB", 2, kMenuHeader8X[i + 2] - 2, 80);
    }

    lcd.setForeColor(WHITE);
    lcd.drawHLine(401, 33, 6);
    lcd.drawLine(406, 33, 411, 38);
    lcd.drawHLine(411, 38, 6);
    lcd.drawHLine(808, 38, 6);
    lcd.drawLine(813, 38, 818, 33);
    lcd.drawHLine(818, 33, 6);

    for (uint8_t i = 0; i < 4; i++) {
        uint16_t xPos = kMenuData8X[i + 3] - 17;
        lcd.drawHLine(xPos, 38, 5);
        lcd.drawLine(xPos + 4, 38, xPos + 9, 33);
        lcd.drawLine(xPos + 9, 33, xPos + 14, 38);
        lcd.drawHLine(xPos + 14, 38, 5);
    }

    lcd_drawEq_ActiveData();
    lcd_drawEq_LowShelfData();
    lcd_drawEq_HighShelfData();
    for (uint8_t i = 0; i < 4; i++) {
        lcd_drawEq_PeakData(i);
    }
}

void Controller::lcd_drawEq_ActiveData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (eq.active) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);
}

void Controller::lcd_drawEq_LowShelfData() {
    lcd_setMenuDataState(WHITE);
    lcd.setFont(FONT_05x07);
    lcd.drawText(kEqFreqDataLibrary[eq.freqLowShelf].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 4, 66);
    lcd.drawText(kEqGainDataLibrary[eq.gainLowShelf].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 4, 80);
}

void Controller::lcd_drawEq_HighShelfData() {
    lcd_setMenuDataState(WHITE);
    lcd.setFont(FONT_05x07);
    lcd.drawText(kEqFreqDataLibrary[eq.freqHighShelf].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 4, 66);
    lcd.drawText(kEqGainDataLibrary[eq.gainHighShelf].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 4, 80);
}

void Controller::lcd_drawEq_PeakData(uint8_t peakNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.setFont(FONT_05x07);
    lcd.setForeColor(GRAY_60);
    lcd.drawText(kEqQDataLibrary[eq.qPeak[peakNum_]].nameShortR, kMenuDataShortTextSize, kMenuData8X[3 + peakNum_] + 4, 52);
    lcd.setForeColor(WHITE);
    lcd.drawText(kEqFreqDataLibrary[eq.freqPeak[peakNum_]].nameShortR, kMenuDataShortTextSize, kMenuData8X[3 + peakNum_] + 4, 66);
    lcd.drawText(kEqGainDataLibrary[eq.gainPeak[peakNum_]].nameShortR, kMenuDataShortTextSize, kMenuData8X[3 + peakNum_] + 4, 80);
}

// filter menu functions

void Controller::lcd_drawFilterMenu(uint8_t filterNum_) {
    lcd_drawMenuIcon(FILTER_0_MENU);

    lcd_setMenuHeaderState(CYAN);
    (filterNum_ == 0) ? lcd.drawText("01", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY) : lcd.drawText("02", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderActive, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("TYPE", kMenuHeaderShortTextSize, kMenuHeader8X[2] - 2, kMenuHeaderY);
    lcd.drawText("FREQ", kMenuHeaderShortTextSize, kMenuHeader8X[3] - 2, kMenuHeaderY);
    lcd.drawText("RES ", kMenuHeaderShortTextSize, kMenuHeader8X[4] - 2, kMenuHeaderY);
    lcd.drawText("SLO ", kMenuHeaderShortTextSize, kMenuHeader8X[5] - 2, kMenuHeaderY);
    lcd.drawText("DRY ", kMenuHeaderShortTextSize, kMenuHeader8X[6] - 2, kMenuHeaderY);
    lcd.drawText("WET ", kMenuHeaderShortTextSize, kMenuHeader8X[7] - 2, kMenuHeaderY);

    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText("   ", kMenuSignTextSize, kMenuData8X[2] + 2, 60);
    lcd.drawText(" HZ", kMenuSignTextSize, kMenuData8X[3] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[4] + 2, 60);
    lcd.drawText(" DB", kMenuSignTextSize, kMenuData8X[5] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[6] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[7] + 2, 60);

    lcd_drawFilter_ActiveData(filterNum_);
    lcd_drawFilter_TypeData(filterNum_);
    lcd_drawFilter_FreqData(filterNum_);
    lcd_drawFilter_ResData(filterNum_);
    lcd_drawFilter_SlopeData(filterNum_);
    lcd_drawFilter_DryData(filterNum_);
    lcd_drawFilter_WetData(filterNum_);
}

void Controller::lcd_drawFilter_ActiveData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (filter[filterNum_].active) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);
}

void Controller::lcd_drawFilter_TypeData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterTypeDataLibrary[filter[filterNum_].type].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 2, kMenuDataY);
}

void Controller::lcd_drawFilter_FreqData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterFreqDataLibrary[filter[filterNum_].freq].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
}

void Controller::lcd_drawFilter_ResData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterResDataLibrary[filter[filterNum_].res].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
}

void Controller::lcd_drawFilter_SlopeData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterSlopeDataLibrary[filter[filterNum_].slope].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
}

void Controller::lcd_drawFilter_DryData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterMixDataLibrary[filter[filterNum_].dry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
}

void Controller::lcd_drawFilter_WetData(uint8_t filterNum_) {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kFilterMixDataLibrary[filter[filterNum_].wet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
}

// effect menu functions

void Controller::lcd_drawEffectMenu(uint8_t effectNum_) {
    lcd_drawMenuIcon(EFFECT_0_MENU);

    lcd_setMenuHeaderState(CYAN);
    (effectNum_ == 0) ? lcd.drawText("01", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY) : lcd.drawText("02", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderActive, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);

    lcd_setMenuHeaderState(CYAN);
    lcd_drawEffect_ActiveData(effectNum_);
    lcd_drawEffect_TypeData(effectNum_);
    lcd_drawEffect_AData(effectNum_);
    lcd_drawEffect_BData(effectNum_);
    lcd_drawEffect_CData(effectNum_);
    lcd_drawEffect_DData(effectNum_);
    lcd_drawEffect_EData(effectNum_);
}

void Controller::lcd_drawEffect_ActiveData(uint8_t effectNum_) {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (effect[effectNum_].active) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);
}

void Controller::lcd_drawEffect_TypeData(uint8_t effectNum_) {
    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("TYPE", kMenuHeaderShortTextSize, kMenuHeader8X[2] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText("   ", kMenuSignTextSize, kMenuData8X[2] + 2, 60);
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kEffectTypeDataLibrary[effect[effectNum_].type].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 2, kMenuDataY);
}

void Controller::lcd_drawEffect_AData(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText(aHeader[type_], kMenuHeaderShortTextSize, kMenuHeader8X[3] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(aSign[type_], kMenuSignTextSize, kMenuData8X[3] + 2, 60);
    lcd_setMenuDataState(WHITE);

    switch (type_) {
    case EF_DELAY:
        lcd.drawText(kDelayTimeDataLibrary[effect_.delay.aTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_CHORUS:
        lcd.drawText(kChorusTimeDataLibrary[effect_.chorus.aTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_FLANGER:
        lcd.drawText(kFlangerTimeDataLibrary[effect_.flanger.aTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_PHASER:
        lcd.drawText(kPhaserFreqDataLibrary[effect_.phaser.aStartFreq].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_COMPRESSOR:
        lcd.drawText(kCompressorThresholdDataLibrary[effect_.compressor.aThreshold].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_EXPANDER:
        lcd.drawText(kExpanderThresholdDataLibrary[effect_.expander.aThreshold].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_OVERDRIVE:
        lcd.drawText(kOverdriveGainDataLibrary[effect_.overdrive.aGain].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_DISTORTION:
        lcd.drawText(kDistortionGainDataLibrary[effect_.distortion.aGain].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;

    case EF_BITCRUSHER:
        lcd.drawText(kBitcrusherResolutionDataLibrary[effect_.bitcrusher.aResolution].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
        break;
    }
}

void Controller::lcd_drawEffect_BData(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText(bHeader[type_], kMenuHeaderShortTextSize, kMenuHeader8X[4] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(bSign[type_], kMenuSignTextSize, kMenuData8X[4] + 2, 60);
    lcd_setMenuDataState(WHITE);

    switch (type_) {
    case EF_DELAY:
        lcd.drawText(kDelayLevelDataLibrary[effect_.delay.bLevel].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_CHORUS:
        lcd.drawText(kChorusFeedbackDataLibrary[effect_.chorus.bFeedback].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_FLANGER:
        lcd.drawText(kFlangerFeedbackDataLibrary[effect_.flanger.bFeedback].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_PHASER:
        lcd.drawText(kPhaserFreqDataLibrary[effect_.phaser.bEndFreq].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_COMPRESSOR:
        lcd.drawText(kCompressorRateDataLibrary[effect_.compressor.bRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_EXPANDER:
        lcd.drawText(kExpanderRateDataLibrary[effect_.expander.bRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_OVERDRIVE:
        lcd.drawText(kOverdriveThresholdDataLibrary[effect_.overdrive.bThreshold].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_DISTORTION:
        lcd.drawText(kDistortionThresholdDataLibrary[effect_.distortion.bThreshold].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;

    case EF_BITCRUSHER:
        lcd.drawText(kBitcrusherSampleRateDataLibrary[effect_.bitcrusher.bSampleRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
        break;
    }
}

void Controller::lcd_drawEffect_CData(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText(cHeader[type_], kMenuHeaderShortTextSize, kMenuHeader8X[5] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(cSign[type_], kMenuSignTextSize, kMenuData8X[5] + 2, 60);
    lcd_setMenuDataState(WHITE);

    switch (type_) {
    case EF_DELAY:
        lcd.drawText(kDelayFeedbackDataLibrary[effect_.delay.cFeedback].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_CHORUS:
        lcd.drawText(kChorusRateDataLibrary[effect_.chorus.cRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_FLANGER:
        lcd.drawText(kFlangerRateDataLibrary[effect_.flanger.cRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_PHASER:
        lcd.drawText(kPhaserRateDataLibrary[effect_.phaser.cRate].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_COMPRESSOR:
        lcd.drawText(kCompressorAttackTimeDataLibrary[effect_.compressor.cAttackTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_EXPANDER:
        lcd.drawText(kExpanderAttackTimeDataLibrary[effect_.expander.cAttackTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_OVERDRIVE:
        lcd.drawText(kOverdriveToneDataLibrary[effect_.overdrive.cTone].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_DISTORTION:
        lcd.drawText(kDistortionToneDataLibrary[effect_.distortion.cTone].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;

    case EF_BITCRUSHER:
        lcd.drawText(kBitcrusherThresholdDataLibrary[effect_.bitcrusher.cThreshold].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
        break;
    }
}

void Controller::lcd_drawEffect_DData(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText(dHeader[type_], kMenuHeaderShortTextSize, kMenuHeader8X[6] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(dSign[type_], kMenuSignTextSize, kMenuData8X[6] + 2, 60);
    lcd_setMenuDataState(WHITE);

    switch (type_) {
    case EF_DELAY:
        lcd.drawText(kEffectMixDataLibrary[effect_.delay.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_CHORUS:
        lcd.drawText(kEffectMixDataLibrary[effect_.chorus.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_FLANGER:
        lcd.drawText(kEffectMixDataLibrary[effect_.flanger.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_PHASER:
        lcd.drawText(kEffectMixDataLibrary[effect_.phaser.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_COMPRESSOR:
        lcd.drawText(kCompressorReleaseTimeDataLibrary[effect_.compressor.dReleaseTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_EXPANDER:
        lcd.drawText(kExpanderReleaseTimeDataLibrary[effect_.expander.dReleaseTime].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_OVERDRIVE:
        lcd.drawText(kEffectMixDataLibrary[effect_.overdrive.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_DISTORTION:
        lcd.drawText(kEffectMixDataLibrary[effect_.distortion.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;

    case EF_BITCRUSHER:
        lcd.drawText(kEffectMixDataLibrary[effect_.bitcrusher.dDry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
        break;
    }
}

void Controller::lcd_drawEffect_EData(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText(eHeader[type_], kMenuHeaderShortTextSize, kMenuHeader8X[7] - 2, kMenuHeaderY);
    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText(eSign[type_], kMenuSignTextSize, kMenuData8X[7] + 2, 60);
    lcd_setMenuDataState(WHITE);

    switch (type_) {
    case EF_DELAY:
        lcd.drawText(kEffectMixDataLibrary[effect_.delay.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_CHORUS:
        lcd.drawText(kEffectMixDataLibrary[effect_.chorus.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_FLANGER:
        lcd.drawText(kEffectMixDataLibrary[effect_.flanger.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_PHASER:
        lcd.drawText(kEffectMixDataLibrary[effect_.phaser.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_COMPRESSOR:
        lcd.drawText(kEffectMixDataLibrary[effect_.compressor.eMix].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_EXPANDER:
        lcd.drawText(kEffectMixDataLibrary[effect_.expander.eMix].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_OVERDRIVE:
        lcd.drawText(kEffectMixDataLibrary[effect_.overdrive.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_DISTORTION:
        lcd.drawText(kEffectMixDataLibrary[effect_.distortion.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;

    case EF_BITCRUSHER:
        lcd.drawText(kEffectMixDataLibrary[effect_.bitcrusher.eWet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
        break;
    }
}

// reverb menu functions

void Controller::lcd_drawReverbMenu() {
    lcd_drawMenuIcon(REVERB_MENU);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("  ", kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderActive, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);

    lcd_setMenuHeaderState(CYAN);
    lcd.drawText("SIZE", kMenuHeaderShortTextSize, kMenuHeader8X[2] - 2, kMenuHeaderY);
    lcd.drawText("DEC ", kMenuHeaderShortTextSize, kMenuHeader8X[3] - 2, kMenuHeaderY);
    lcd.drawText("PRE ", kMenuHeaderShortTextSize, kMenuHeader8X[4] - 2, kMenuHeaderY);
    lcd.drawText("SUR ", kMenuHeaderShortTextSize, kMenuHeader8X[5] - 2, kMenuHeaderY);
    lcd.drawText("DRY ", kMenuHeaderShortTextSize, kMenuHeader8X[6] - 2, kMenuHeaderY);
    lcd.drawText("WET ", kMenuHeaderShortTextSize, kMenuHeader8X[7] - 2, kMenuHeaderY);

    lcd_setMenuSignState(CYAN, FONT_05x07);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[2] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[3] + 2, 60);
    lcd.drawText(" MS", kMenuSignTextSize, kMenuData8X[4] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[5] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[6] + 2, 60);
    lcd.drawText("  %", kMenuSignTextSize, kMenuData8X[7] + 2, 60);

    lcd_drawReverb_ActiveData();
    lcd_drawReverb_SizeData();
    lcd_drawReverb_DecayData();
    lcd_drawReverb_PreDelayData();
    lcd_drawReverb_SurroundData();
    lcd_drawReverb_DryData();
    lcd_drawReverb_WetData();
}

void Controller::lcd_drawReverb_ActiveData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (reverb.active) ? textPtr = kDataOn : textPtr = kDataOff;
    lcd.drawText(textPtr, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);
}

void Controller::lcd_drawReverb_SizeData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbSizeDataLibrary[reverb.size].nameShortR, kMenuDataShortTextSize, kMenuData8X[2] + 2, kMenuDataY);
}

void Controller::lcd_drawReverb_DecayData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbMixDataLibrary[reverb.decay].nameShortR, kMenuDataShortTextSize, kMenuData8X[3] + 2, kMenuDataY);
}

void Controller::lcd_drawReverb_PreDelayData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbPreDelayDataLibrary[reverb.preDelay].nameShortR, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
}

void Controller::lcd_drawReverb_SurroundData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbSurroundDataLibrary[reverb.surround].nameShortR, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
}

void Controller::lcd_drawReverb_DryData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbMixDataLibrary[reverb.dry].nameShortR, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
}

void Controller::lcd_drawReverb_WetData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kReverbMixDataLibrary[reverb.wet].nameShortR, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
}

// layer inst menu functions

void Controller::lcd_drawLayerInstMenu() {
    lcd_drawMenuIcon(LAYER_INST_MENU);

    lcd_setMenuHeaderState(kLayerColorPalette[selectedLayerNum]);
    char kData[kMenuNumberTextSize + 1];
    sprintf(kData, "%02d", selectedLayerNum + 1);
    lcd.drawText(kData, kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderInst, kMenuHeaderTextSize, kMenuHeader8X[0], kMenuHeaderY);
    lcd.drawText(kHeaderSample, kMenuHeaderTextSize, kMenuHeader8X[2], kMenuHeaderY);

    lcd_setMenuHeaderState(kLayerColorPalette[selectedLayerNum]);
    lcd.drawText("LEV ", kMenuHeaderShortTextSize, kMenuHeader8X[4] - 2, kMenuHeaderY);
    lcd.drawText("SPE ", kMenuHeaderShortTextSize, kMenuHeader8X[5] - 2, kMenuHeaderY);
    lcd.drawText("REV ", kMenuHeaderShortTextSize, kMenuHeader8X[6] - 2, kMenuHeaderY);
    lcd.drawText("NORM", kMenuHeaderShortTextSize, kMenuHeader8X[7] - 2, kMenuHeaderY);

    lcd_drawLayerInst_InstData();
    lcd_drawLayerInst_SampleData();
    lcd_drawLayerInst_VolumeData();
    lcd_drawLayerInst_SpeedData();
    lcd_drawLayerInst_ReverseData();
    lcd_drawLayerInst_NormalizeData();
}

void Controller::lcd_drawLayerInst_InstData() {
    Layer &layer = layerLibrary[selectedLayerNum];

    lcd_setMenuDataState(WHITE);
    (strcmp(layer.instSelectedData.fileName, layer.instLoadedData.fileName) == 0) ? lcd.setForeColor(WHITE) : lcd.setForeColor(GRAY_60);
    lcd.drawText(layer.instSelectedData.nameShortR, kMenuDataTextSize, kMenuData8X[1], kMenuDataY);

    lcd_setMenuNumState(kLayerColorPalette[selectedLayerNum]);
    lcd.drawText(layer.instSelectedData.num, 4, kMenuData8X[1], kMenuHeaderY + 1);
}

void Controller::lcd_drawLayerInst_SampleData() {
    Layer &layer = layerLibrary[selectedLayerNum];

    lcd_setMenuDataState(WHITE);
    ((strcmp(layer.instSelectedData.fileName, layer.instLoadedData.fileName) == 0) && (strcmp(layer.sampleSelectedData.fileName, layer.sampleLoadedData.fileName) == 0)) ? lcd.setForeColor(WHITE) : lcd.setForeColor(GRAY_60);
    lcd.drawText(layer.sampleSelectedData.nameShortR, kMenuDataTextSize, kMenuData4X[1], kMenuDataY);

    lcd_setMenuNumState(kLayerColorPalette[selectedLayerNum]);
    lcd.drawText(layer.sampleSelectedData.num, 4, kMenuData8X[3], kMenuHeaderY + 1);

    uint16_t xPos = kMenuData4X[1];
    uint16_t yPos = kMenuHeaderY + 23;
    if (layer.sampleSelected != -1) {
        if (layer.sampleSelectedReadError) {
            lcd.setForeColor(kLayerColorPalette[9]);
            lcd.drawText(" READ ERROR", 11, xPos, yPos);
        } else if (layer.sampleSelectedTypeError) {
            lcd.setForeColor(kLayerColorPalette[9]);
            lcd.drawText(" TYPE ERROR", 11, xPos, yPos);
        } else {
            lcd.setSpacing(2);
            lcd.setForeColor(GRAY_60);
            lcd.drawText(kFrequencyDataLibrary[layer.sampleSelectedData.frequency].name, 6, xPos - 35, yPos);
            lcd.drawText(kBitdepthDataLibrary[layer.sampleSelectedData.bitdepth].name, 2, xPos - 14, yPos);
            lcd.drawText(kChannelDataLibrary[layer.sampleSelectedData.channel].name, 1, xPos, yPos);
            lcd.drawText("-", 1, xPos - 28, yPos);
            lcd.drawText("-", 1, xPos - 7, yPos);
        }
    } else {
        lcd.clearRect(xPos - 80, yPos, 80, 8);
    }

    // lcd.drawNumber(layer.playSampleSector, 2, 50, 50);
    // lcd.drawNumber(layer.sampleSector[layer.playSampleSector].size, 5, 50, 65);
}

void Controller::lcd_drawLayerInst_VolumeData() {
    lcd_setMenuDataState(WHITE);
    char kData[kMenuDataShortTextSize + 1];
    sprintf(kData, " %03d", layerLibrary[selectedLayerNum].volume);
    lcd.drawText(kData, kMenuDataShortTextSize, kMenuData8X[4] + 2, kMenuDataY);
}

void Controller::lcd_drawLayerInst_SpeedData() {
    lcd_setMenuDataState(WHITE);
    lcd.drawText(kLayerSpeedDataLibrary[layerLibrary[selectedLayerNum].speed].nameShort, kMenuDataShortTextSize, kMenuData8X[5] + 2, kMenuDataY);
}

void Controller::lcd_drawLayerInst_ReverseData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (layerLibrary[selectedLayerNum].reverse) ? textPtr = kDataShortROn : textPtr = kDataShortROff;
    lcd.drawText(textPtr, kMenuDataShortTextSize, kMenuData8X[6] + 2, kMenuDataY);
}

void Controller::lcd_drawLayerInst_NormalizeData() {
    lcd_setMenuDataState(WHITE);
    const char *textPtr;
    (layerLibrary[selectedLayerNum].normalize) ? textPtr = kDataShortROn : textPtr = kDataShortROff;
    lcd.drawText(textPtr, kMenuDataShortTextSize, kMenuData8X[7] + 2, kMenuDataY);
}

void Controller::lcd_drawTab_InstData(uint8_t layerNum_) {
    Layer &layer = layerLibrary[layerNum_];
    lcd_setLayerDataState(layerNum_);
    lcd.drawText(layer.instLoadedData.nameShortL, kMenuDataTextSize, 47, kImageLayerY[layerNum_] + 5);
}

// layer song menu functions

void Controller::lcd_drawLayerSongMenu() {
    lcd_drawMenuIcon(LAYER_SONG_MENU);

    lcd_setMenuHeaderState(kLayerColorPalette[selectedLayerNum]);
    char kData[kMenuNumberTextSize + 1];
    sprintf(kData, "%02d", selectedLayerNum + 1);
    lcd.drawText(kData, kMenuNumberTextSize, kMenuNumberX, kMenuNumberY);

    lcd_setMenuHeaderState(WHITE);
    lcd.drawText(kHeaderFill, kMenuHeaderTextSize, kMenuHeader4X[0], kMenuHeaderY);
    lcd.drawText(kHeaderBlank, kMenuHeaderTextSize, kMenuHeader4X[1], kMenuHeaderY);
    // draw graph base
    lcd.drawVLine(kBeatGraphEndX, kBeatGraphStartY, kBeatGraphHeight);
    lcd.drawHLine(kBeatGraphStartX, kBeatGraphEndY, kBeatGraphWidth);
    // draw graph text
    lcd.setFont(FONT_05x07);
    lcd.drawText(kDataStart, 1, kBeatGraphStartTimeX + 49, kMenuHeaderY);
    lcd.drawText(kDataEnd, 1, kBeatGraphEndTimeX, kMenuHeaderY);
    // draw data
    lcd_drawLayerSong_BeatFillData();
    lcd_drawLayerSong_BeatGraphData();

    if (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1)
        lcd_drawBeat(selectedLayerNum, activeBankNum, 0, true);
}

void Controller::lcd_drawLayerSong_BeatFillData() {
    Layer &layer = layerLibrary[selectedLayerNum];
    Bank &bank = layer.bankLibrary[activeBankNum];

    lcd_setMenuDataState(WHITE);
    const char *text;
    (selectedBeatNum != -1) ? text = kFillDataLibrary[bank.beatLibrary[selectedBeatNum].getFill()].nameLong : text = kDataDashR;
    lcd.drawText(text, kMenuDataTextSize, kMenuData4X[0], kMenuDataY);

    lcd_setMenuNumState(kLayerColorPalette[selectedLayerNum]);
    char numText[6];
    if (bank.lastActiveBeatNum != -1) {
        char num0[3];
        char num1[3];
        sprintf(num0, "%02d", selectedBeatNum + 1);
        sprintf(num1, "%02d", bank.lastActiveBeatNum + 1);
        strcpy(numText, num0);
        strcat(numText, "|");
        strcat(numText, num1);
    } else {
        strcpy(numText, "--|--");
    }
    lcd.drawText(numText, 5, kMenuData4X[0], kMenuHeaderY + 1);
}

void Controller::lcd_drawLayerSong_BeatGraphData() {
    Layer &layer = layerLibrary[selectedLayerNum];
    Bank &bank = layer.bankLibrary[activeBankNum];

    lcd.setForeColor(WHITE);
    lcd.setBackColor(BLACK);
    lcd.setAlignment(LEFT);
    lcd.setFont(FONT_05x07);

    // draw text
    if (selectedBeatNum != -1) {
        char *text;
        Beat &beat = bank.beatLibrary[selectedBeatNum];
        uint8_t fill = beat.getFill();
        uint8_t step = kFillDataLibrary[fill].step;
        uint16_t startInterval = beat.getStartInterval();
        uint16_t endInterval = beat.getEndInterval();
        uint8_t startBar = startInterval / barInterval;
        uint8_t startMeasure = (startInterval % barInterval) / measureInterval;
        uint8_t startRemainder = ((float)(startInterval % measureInterval) / measureInterval) * 100;
        uint8_t endBar = endInterval / barInterval;
        uint8_t endMeasure = (endInterval % barInterval) / measureInterval;
        uint8_t endRemainder = ((float)(endInterval % measureInterval) / measureInterval) * 100;

        lcd.drawText(kDataDot, 1, kBeatGraphStartTimeX + 14, kBeatGraphTimeY);
        lcd.drawText(kDataDot, 1, kBeatGraphStartTimeX + 35, kBeatGraphTimeY);
        sprintf(text, "%02d", startBar);
        lcd.drawText(text, 2, kBeatGraphStartTimeX, kBeatGraphTimeY);
        sprintf(text, "%02d", startMeasure);
        lcd.drawText(text, 2, kBeatGraphStartTimeX + 21, kBeatGraphTimeY);
        sprintf(text, "%02d", startRemainder);
        lcd.drawText(text, 2, kBeatGraphStartTimeX + 42, kBeatGraphTimeY);

        lcd.drawText(kDataDot, 1, kBeatGraphEndTimeX + 14, kBeatGraphTimeY);
        lcd.drawText(kDataDot, 1, kBeatGraphEndTimeX + 35, kBeatGraphTimeY);
        sprintf(text, "%02d", endBar);
        lcd.drawText(text, 2, kBeatGraphEndTimeX, kBeatGraphTimeY);
        sprintf(text, "%02d", endMeasure);
        lcd.drawText(text, 2, kBeatGraphEndTimeX + 21, kBeatGraphTimeY);
        sprintf(text, "%02d", endRemainder);
        lcd.drawText(text, 2, kBeatGraphEndTimeX + 42, kBeatGraphTimeY);
    } else {
        // lcd.clearRect(420, 28, 32, 5);
        lcd.drawText(kDataTimeBlank, 8, kBeatGraphStartTimeX, kBeatGraphTimeY);
        lcd.drawText(kDataTimeBlank, 8, kBeatGraphEndTimeX, kBeatGraphTimeY);
    }

    // draw graph
    if (selectedBeatNum != -1) {
        Beat &beat = bank.beatLibrary[selectedBeatNum];
        uint8_t fill = beat.getFill();
        uint8_t step = kFillDataLibrary[fill].step;

        if (fill != preFill) {
            // clear graph
            lcd.clearRect(kBeatGraphStartX + 1, kBeatGraphStartY, kBeatGraphWidth - 2, kBeatGraphHeight - 1);
            // draw graph base
            lcd.drawVLine(kBeatGraphStartX, kBeatGraphStartY, kBeatGraphHeight);
            for (uint8_t i = 0; i < 18; i++) {
                lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
                lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 2), kBeatGraphStartY + (i * 3));
                lcd.drawPixel(kBeatGraphStartX + (3 * kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
            }
            // draw graph data
            lcd.setForeColor(WHITE);
            for (uint8_t i = 0; i < step; i++) {
                uint16_t xGraph = kBeatGraphStartX + kFillDataLibrary[fill].timeLibrary[i] * kBeatGraphWidth;
                uint8_t yGraphSize = ((float)(kFillDataLibrary[fill].volumeLibrary[i]) / 100) * 55;
                uint16_t yGraph = kBeatGraphStartY + (55 - yGraphSize);
                lcd.fillRect(xGraph, yGraph, 5, yGraphSize);
            }
            preFill = fill;
        }
    } else if ((preMenu != LAYER_SONG_MENU) || (preFill != -1)) {
        // clear graph
        lcd.clearRect(kBeatGraphStartX, kBeatGraphStartY, kBeatGraphWidth - 2, kBeatGraphHeight - 1);
        // draw graph base
        lcd.drawVLine(kBeatGraphStartX, kBeatGraphStartY, kBeatGraphHeight);
        for (uint8_t i = 0; i < 18; i++) {
            lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
            lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 2), kBeatGraphStartY + (i * 3));
            lcd.drawPixel(kBeatGraphStartX + (3 * kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
        }
        preFill = -1;
    } else {
        // clear graph
        lcd.clearRect(kBeatGraphStartX, kBeatGraphStartY, kBeatGraphWidth - 2, kBeatGraphHeight - 1);
        // draw graph base
        lcd.drawVLine(kBeatGraphStartX, kBeatGraphStartY, kBeatGraphHeight);
        for (uint8_t i = 0; i < 18; i++) {
            lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
            lcd.drawPixel(kBeatGraphStartX + (kBeatGraphWidth / 2), kBeatGraphStartY + (i * 3));
            lcd.drawPixel(kBeatGraphStartX + (3 * kBeatGraphWidth / 4), kBeatGraphStartY + (i * 3));
        }
        preFill = -1;
    }
}

// layer menu functions

void Controller::lcd_drawLayer_Mute(uint8_t layerNum_) {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (layerNum_ * 128));
    (layerLibrary[layerNum_].mute) ? dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_OFF_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconMuteX, kImageLayerY[layerNum_] + 4, kIconLayerWidth, kIconLayerHeight);
}

void Controller::lcd_drawLayer_Fill(uint8_t layerNum_) {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (layerNum_ * 128));
    (layerLibrary[layerNum_].fill) ? dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_OFF_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconFillX, kImageLayerY[layerNum_] + 4, kIconLayerWidth, kIconLayerHeight);
}

void Controller::lcd_drawLayer_Style(uint8_t layerNum_) {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;

    indexPtr = (const RGB16Color *)(RAM_ICON_LAYER_PALETTE_ADDRESS + (layerNum_ * 128));
    (layerLibrary[layerNum_].style) ? dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_LAYER_L_OFF_DATA_ADDRESS);
    lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconStyleX, kImageLayerY[layerNum_] + 4, kIconLayerWidth, kIconLayerHeight);
}

// bank functions

void Controller::lcd_drawBank(uint8_t bankNum_, bool mode_) {
    lcd.setAlignment(LEFT);
    lcd.setFont(FONT_05x07);
    lcd.setBackColor(BLACK);
    (mode_) ? lcd.setForeColor(GREEN) : lcd.setForeColor(YELLOW);
    lcd.drawText(kNumberDataLibrary[bankNum_ + 1].nameShortL, 2, 132, 110);
}

void Controller::lcd_drawBankShift() {
    if (bankActionFlag) {
        bankActionFlag = false;
        lcd_drawBank(activeBankNum, false);
        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            for (uint8_t j = 0; j < kBankLibrarySize; j++) {
                lcd_drawSong(i, j);
            }
        }
    }
}

// transition functions

void Controller::lcd_drawTransition() {
    if (transitionShowFlag) {
        lcd.setFont(FONT_05x07);
        lcd.setAlignment(RIGHT);
        (transitionShowFlag == 1) ? lcd.setForeColor(MAGENTA) : lcd.setForeColor(GREEN);
        lcd.setBackColor(BLACK);
        lcd.drawText("PASS", 4, kMenuData8X[1], kMenuHeaderY + 2);

        transitionShowFlag = 0;
    }

    if (transitionClearFlag) {
        lcd.setFont(FONT_05x07);
        lcd.setAlignment(RIGHT);
        lcd.setForeColor(MAGENTA);
        lcd.setBackColor(BLACK);
        lcd.drawText("    ", 4, kMenuData8X[1], kMenuHeaderY + 2);

        transitionClearFlag = false;
    }
}

// play functions

void Controller::lcd_drawPlay() {
    if ((playActive) && (!metronome.precountState)) {
        if (((playInterval / playXRatio) > playX) && (playX < kPlayWidth)) {
            lcd.setForeColor(playColor);
            lcd.drawVLine(kPlayX + playX, kPlayY, 1);
            playX += 1;
        }
    }

    if (resetPlayFlag) {
        lcd_resetPlay();
        resetPlayFlag = false;
    }
}

void Controller::lcd_drawIcon() {
    const RGB16Color *indexPtr;
    const uint8_t *dataPtr;
    uint16_t xPos;

    indexPtr = (const RGB16Color *)(RAM_ICON_PLAY_PALETTE_ADDRESS);

    if (resetIcon.flag) {
        (resetIcon.mode) ? dataPtr = (const uint8_t *)(RAM_ICON_PLAY_RESET_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_PLAY_RESET_OFF_DATA_ADDRESS);
        lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconResetX, kIconPlayY, kIconPlayWidth, kIconPlayHeight);
        resetIcon.flag = false;
    }

    if (playIcon.flag) {
        (playIcon.mode) ? dataPtr = (const uint8_t *)(RAM_ICON_PLAY_PLAY_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_PLAY_PLAY_OFF_DATA_ADDRESS);
        lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconPlayX, kIconPlayY, kIconPlayWidth, kIconPlayHeight);
        playIcon.flag = false;
    }

    if (stopIcon.flag) {
        (stopIcon.mode) ? dataPtr = (const uint8_t *)(RAM_ICON_PLAY_STOP_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_PLAY_STOP_OFF_DATA_ADDRESS);
        lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconStopX, kIconPlayY, kIconPlayWidth, kIconPlayHeight);
        stopIcon.flag = false;
    }

    if (recordIcon.flag) {
        (recordIcon.mode) ? dataPtr = (const uint8_t *)(RAM_ICON_PLAY_RECORD_ON_DATA_ADDRESS) : dataPtr = (const uint8_t *)(RAM_ICON_PLAY_RECORD_OFF_DATA_ADDRESS);
        lcd.drawRGB16Image(indexPtr, dataPtr, 64, kIconRecordX, kIconPlayY, kIconPlayWidth, kIconPlayHeight);
        recordIcon.flag = false;
    }
}

void Controller::lcd_drawText() {
    lcd.setBackColor(BLACK);
    lcd.setAlignment(LEFT);
    lcd.setFont(FONT_05x07);
    if (textCopyFlag) {
        textCopyFlag = false;
        textShow = true;
        startTextTimer();
        lcd.setForeColor(GREEN);
        lcd.drawText("C", 1, 117, 110);
    } else if (textPasteFlag) {
        textPasteFlag = false;
        textShow = true;
        startTextTimer();
        lcd.setForeColor(YELLOW);
        lcd.drawText("P", 1, 117, 110);
    } else if (textClearFlag) {
        textClearFlag = false;
        lcd.drawText(" ", 1, 117, 110);
    }
}

void Controller::lcd_drawCountDown() {
    if (metronome.countDownFlag) {
        if (metronome.countDown > 0) {
            lcd.setAlignment(LEFT);
            lcd.setFont(FONT_05x07);
            lcd.setBackColor(BLACK);
            lcd.setForeColor(kLayerColorPalette[9]);
            char kCountDownData[1];
            sprintf(kCountDownData, "%01d", metronome.countDown);
            lcd.drawText(kCountDownData, 1, 117, 110);
            metronome.countDown -= 1;
        } else {
            lcd_clearCountDown();
        }
        metronome.countDownFlag = false;
    }
}

void Controller::lcd_clearCountDown() {
    lcd.setBackColor(BLACK);
    lcd.clearRect(117, 110, 5, 7);
}

void Controller::lcd_restartPlay() {
    lcd_invertPlayColor();
    playX = 0;
}

void Controller::lcd_resetPlay() {
    playX = 0;
    lcd_calculateSongX();
    lcd.setForeColor(kPlayColor1);
    lcd.drawHLine(kPlayX, kPlayY, kPlayWidth);
    lcd_resetPlayColor();
    lcd_invertPlayColor();
}

void Controller::lcd_redrawPlay() {
    playX = playInterval / playXRatio;
    lcd.setForeColor(playColor);
    lcd.drawHLine(kPlayX, kPlayY, kPlayWidth);
    lcd_invertPlayColor();
}

void Controller::lcd_cleanEndPlay() {
    (lcd.getForeColor() == kPlayColor0) ? lcd.setForeColor(kPlayColor1) : lcd.setForeColor(kPlayColor0);
    lcd.drawHLine(kPlayX + playX + 1, kPlayY, kPlayWidth - playX);
}

void Controller::lcd_invertPlayColor() {
    (playColor == kPlayColor0) ? playColor = kPlayColor1 : playColor = kPlayColor0;
}

void Controller::lcd_resetPlayColor() {
    playColor = kPlayColor1;
}

// song functions

void Controller::lcd_calculateSongX() {
    playXRatio = (float)songInterval / kPlayWidth;
}

void Controller::lcd_drawMeasureBar() {
    float barX = (float)kPlayWidth / rhythm.bar;
    float measureX = (float)kPlayWidth / (rhythm.measure * rhythm.bar);
    lcd.setBackColor(BLACK);
    for (uint8_t i = 0; i < 9; i++) {
        lcd.clearRect(kPlayX + 2, kImageLayerY[i] + 24, kPlayWidth - 4, 3);
        for (uint8_t j = 1; j < (rhythm.measure * rhythm.bar); j++) {
            lcd.setForeColor(kLayerColorPalette[i]);
            uint16_t posX = kPlayX + int(j * measureX);
            if (j % rhythm.measure == 0) {
                lcd.fillRect(posX - 1, kImageLayerY[i] + 24, 3, 3);
            } else {
                lcd.fillRect(posX, kImageLayerY[i] + 24, 1, 3);
            }
        }
    }
}

void Controller::lcd_drawBeat(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_, bool selected_) {
    if (bankNum_ == activeBankNum) {
        Layer &layer = layerLibrary[layerNum_];
        Bank &bank = layer.bankLibrary[bankNum_];
        Beat &beat = bank.beatLibrary[beatNum_];

        bool active = beat.getActive();
        uint8_t fill = beat.getFill();

        lcd.setBackColor(kLayerColorPalette[layerNum_]);

        if (selected_) {
            (layerNum_ < 5) ? lcd.setForeColor(RED) : lcd.setForeColor(WHITE);
        } else {
            lcd.setForeColor(BLACK);
        }

        if (active) {
            // calculate intervals
            uint16_t startInterval = beat.getStartInterval();
            uint16_t endInterval = beat.getEndInterval();
            // calculate x - y positions
            uint16_t xStart = kSongX + (startInterval * kSongWidth / songInterval);
            uint16_t xEnd = kSongX + (endInterval * kSongWidth / songInterval) - 1;
            uint16_t yStart = kImageLayerY[layerNum_] + kSongOffsetY;
            // if (fill == FILL_RESET) {}
            // single fill
            if ((fill == 0) || (fill == 1)) {
                lcd.clearRect(xStart, yStart, xEnd - xStart + 1, kSongHeight);
                lcd.drawRect(xStart, yStart, 2, kSongHeight);
            }
            // silent fill
            else if (fill == 2) {
                lcd.clearRect(xStart, yStart, xEnd - xStart + 1, kSongHeight);
                for (uint8_t i = 0; i < 4; i++) {
                    lcd.drawPixel(xStart, yStart + (i * 3));
                }
            }
            // other fills
            else {
                lcd.fillRect(xStart, yStart, xEnd - xStart, kSongHeight);
            }
        }
    }
}

void Controller::lcd_clearBeat(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_) {
    if (bankNum_ == activeBankNum) {
        Layer &layer = layerLibrary[layerNum_];
        Bank &bank = layer.bankLibrary[bankNum_];
        Beat &beat = bank.beatLibrary[beatNum_];

        bool active = beat.getActive();
        bool fill = beat.getFill();
        // calculate intervals
        uint16_t startInterval = beat.getStartInterval();
        uint16_t endInterval = beat.getEndInterval();
        // calculate x - y positions
        uint16_t xStart = kSongX + (startInterval * kSongWidth / songInterval);
        uint16_t xEnd = kSongX + (endInterval * kSongWidth / songInterval);
        uint16_t yStart = kImageLayerY[layerNum_] + kSongOffsetY;

        lcd.setForeColor(kLayerColorPalette[layerNum_]);
        lcd.fillRect(xStart, yStart, xEnd - xStart, kSongHeight);
    }
}

void Controller::lcd_drawSong(uint8_t layerNum_, uint8_t bankNum_) {
    if (bankNum_ == activeBankNum) {
        Layer &layer = layerLibrary[layerNum_];
        Bank &bank = layer.bankLibrary[bankNum_];

        lcd_clearSong(layerNum_, bankNum_);
        if (bank.lastActiveBeatNum != -1) {
            for (uint8_t i = 0; i <= bank.lastActiveBeatNum; i++) {
                Beat &beat = bank.beatLibrary[i];
                if (beat.getActive()) {
                    bool selected;
                    ((layerNum_ == selectedLayerNum) && (i == selectedBeatNum)) ? selected = true : selected = false;
                    lcd_drawBeat(layerNum_, bankNum_, i, selected);
                }
            }
        }
    }
}

void Controller::lcd_clearSong(uint8_t layerNum_, uint8_t bankNum_) {
    if (bankNum_ == activeBankNum) {
        uint16_t yStart = kImageLayerY[layerNum_] + kSongOffsetY;
        lcd.setForeColor(kLayerColorPalette[layerNum_]);
        lcd.fillRect(kSongX, yStart, kSongWidth + 1, kSongHeight + 1);
    }
}

void Controller::lcd_clearInterval(uint8_t layerNum_, uint8_t bankNum_, uint16_t startInterval_, uint16_t endInterval_) {
    if (bankNum_ == activeBankNum) {
        uint16_t xStart = kSongX + (startInterval_ * kSongWidth / songInterval);
        uint16_t xEnd = kSongX + (endInterval_ * kSongWidth / songInterval);
        uint16_t yStart = kImageLayerY[layerNum_] + kSongOffsetY;
        lcd.setForeColor(kLayerColorPalette[layerNum_]);
        lcd.fillRect(xStart, yStart, xEnd - xStart, kSongHeight + 1);
    }
}

/* Main functions ------------------------------------------------------------*/

void Controller::main_select() {
    if (menu != MAIN_MENU) {
        preMenu = menu;
        menu = MAIN_MENU;
        preMenuTab = menuTab;
        menuTab = -1;
        subMenuTab = -1;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawMainMenu();
    }
}

/* File functions ------------------------------------------------------------*/

void Controller::file_select() {
    if (menu != FILE_MENU) {
        preMenu = menu;
        menu = FILE_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawFileMenu();
    }
}

void Controller::file_menuRight() {
    if (menuTab < kMaxMenu4Tab) {
        preMenuTab = menuTab;
        menuTab += 1;
        fileMenuCounter = 0;
        sd_checkFile(fileMenuCounter);
        lcd_transitionSelect();

        switch (menuTab) {
        case 1:
            lcd_drawFileBox(1, fileStatus);
            lcd_drawFile_NewData();
            lcd_drawFile_LoadData();
            break;

        case 2:
            lcd_clearFileBox(1);
            lcd_drawFileBox(2, fileStatus);
            lcd_drawFile_LoadData();
            lcd_drawFile_SaveData();
            break;

        case 3:
            lcd_clearFileBox(2);
            lcd_drawFileBox(3, fileStatus);
            lcd_drawFile_SaveData();
            lcd_drawFile_ClearData();
            break;
        }
    }
}

void Controller::file_menuLeft() {
    if (menuTab > kMinMenu4Tab) {
        preMenuTab = menuTab;
        menuTab -= 1;
        fileMenuCounter = 0;
        sd_checkFile(fileMenuCounter);
        lcd_transitionSelect();

        switch (menuTab) {
        case 0:
            lcd_clearFileBox(1);
            lcd_drawFile_NewData();
            lcd_drawFile_LoadData();
            break;

        case 1:
            lcd_clearFileBox(2);
            lcd_drawFileBox(1, fileStatus);
            lcd_drawFile_LoadData();
            lcd_drawFile_SaveData();
            break;

        case 2:
            lcd_clearFileBox(3);
            lcd_drawFileBox(2, fileStatus);
            lcd_drawFile_SaveData();
            lcd_drawFile_ClearData();
            break;
        }
    }
}

void Controller::file_menuUp() {
    (fileMenuCounter < kMaxFile) ? fileMenuCounter += 1 : fileMenuCounter = kMinFile;
    sd_checkFile(fileMenuCounter);

    switch (menuTab) {
    case 0:
        break;

    case 1:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_LoadData();
        break;

    case 2:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_SaveData();
        break;

    case 3:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_ClearData();
        break;
    }
}

void Controller::file_menuDown() {
    (fileMenuCounter > kMinFile) ? fileMenuCounter -= 1 : fileMenuCounter = kMaxFile;
    sd_checkFile(fileMenuCounter);

    switch (menuTab) {
    case 0:
        break;

    case 1:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_LoadData();
        break;

    case 2:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_SaveData();
        break;

    case 3:
        lcd_drawFileBox(menuTab, fileStatus);
        lcd_drawFile_ClearData();
        break;
    }
}

void Controller::file_newSelect() {
    alertFlag = true;
    alertType = ALERT_NEWFILE;
    lcd_drawAlert();
}

void Controller::file_newAction() {
    lcd_clearAlert();
    reset();

    layer_setTab(TAB_0);
    activeBankNum = 0;
    lcd_drawBank(activeBankNum, false);

    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        layerInst_reset(i);
        for (uint8_t j = 0; j < kBankLibrarySize; j++) {
            layerSong_reset(i, j);
        }
    }

    rhythm_reset();
    metro_reset();
    eq_reset();
    filter_reset(0);
    filter_reset(1);
    effect_reset(0);
    effect_reset(1);
    reverb_reset();

    copyLayerInstNum = -1;
    copyLayerFillNum = -1;
    copyLayerSongNum = -1;
    copyBankSongNum = -1;
}

void Controller::file_loadSelect() {
    if (fileStatus == FILE_ACTIVE) {
        alertFlag = true;
        alertType = ALERT_LOADFILE;
        lcd_drawAlert();
    }
}

void Controller::file_loadAction() {
    lcd_clearAlert();

    layer_setTab(TAB_0);
    activeBankNum = 0;
    lcd_drawBank(activeBankNum, false);

    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        layerInst_reset(i);
        for (uint8_t j = 0; j < kBankLibrarySize; j++) {
            layerSong_reset(i, j);
        }
    }

    switch (sd_loadFile(fileMenuCounter)) {
    case SD_OK:
        alertType = ALERT_LOADSUCCESS;
        break;

    case SD_ERROR:
        alertType = ALERT_LOADERROR;
        break;

    case SD_ERROR_DETECT:
        alertType = ALERT_MISSINGSAMPLE;
        break;
    }

    copyLayerInstNum = -1;
    copyLayerFillNum = -1;
    copyLayerSongNum = -1;
    copyBankSongNum = -1;

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

void Controller::file_saveSelect() {
    alertFlag = true;
    (fileStatus == FILE_ACTIVE) ? alertType = ALERT_OVERWRITEFILE : alertType = ALERT_SAVEFILE;
    lcd_drawAlert();
}

void Controller::file_saveAction() {
    lcd_clearAlert();
    (sd_saveFile(fileMenuCounter) == SD_OK) ? alertType = ALERT_SAVESUCCESS : alertType = ALERT_SAVEERROR;

    sd_checkFile(fileMenuCounter);
    lcd_drawFileBox(menuTab, fileStatus);

    sd_getFileLibrary();
    lcd_drawSdData();

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

void Controller::file_clearSelect() {
    if (fileStatus != FILE_INACTIVE) {
        alertFlag = true;
        alertType = ALERT_CLEARFILE;
        lcd_drawAlert();
    }
}

void Controller::file_clearAction() {
    lcd_clearAlert();
    (sd_clearFile(fileMenuCounter) == SD_OK) ? alertType = ALERT_CLEARSUCCESS : alertType = ALERT_CLEARERROR;

    sd_checkFile(fileMenuCounter);
    lcd_drawFileBox(menuTab, fileStatus);

    sd_getFileLibrary();
    lcd_drawSdData();

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

/* Drumkit functions ---------------------------------------------------------*/

void Controller::drumkit_select() {
    if (menu != DRUMKIT_MENU) {
        preMenu = menu;
        menu = DRUMKIT_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawDrumkitMenu();
    }
}

void Controller::drumkit_menuRight() {
    if (menuTab < kMaxMenu4Tab) {
        preMenuTab = menuTab;
        menuTab += 1;
        drumkitMenuCounter = 0;
        sd_checkDrumkit(drumkitMenuCounter);
        lcd_transitionSelect();

        switch (menuTab) {
        case 1:
            lcd_drawFileBox(1, drumkitStatus);
            lcd_drawDrumkit_NewData();
            lcd_drawDrumkit_LoadData();
            break;

        case 2:
            lcd_clearFileBox(1);
            lcd_drawFileBox(2, drumkitStatus);
            lcd_drawDrumkit_LoadData();
            lcd_drawDrumkit_SaveData();
            break;

        case 3:
            lcd_clearFileBox(2);
            lcd_drawFileBox(3, drumkitStatus);
            lcd_drawDrumkit_SaveData();
            lcd_drawDrumkit_ClearData();
            break;
        }
    }
}

void Controller::drumkit_menuLeft() {
    if (menuTab > kMinMenu4Tab) {
        preMenuTab = menuTab;
        menuTab -= 1;
        drumkitMenuCounter = 0;
        sd_checkDrumkit(drumkitMenuCounter);
        lcd_transitionSelect();

        switch (menuTab) {
        case 0:
            lcd_clearFileBox(1);
            lcd_drawDrumkit_NewData();
            lcd_drawDrumkit_LoadData();
            break;

        case 1:
            lcd_clearFileBox(2);
            lcd_drawFileBox(1, drumkitStatus);
            lcd_drawDrumkit_LoadData();
            lcd_drawDrumkit_SaveData();
            break;

        case 2:
            lcd_clearFileBox(3);
            lcd_drawFileBox(2, drumkitStatus);
            lcd_drawDrumkit_SaveData();
            lcd_drawDrumkit_ClearData();
            break;
        }
    }
}

void Controller::drumkit_menuUp() {
    (drumkitMenuCounter < kMaxDrumkit) ? drumkitMenuCounter += 1 : drumkitMenuCounter = kMinFile;
    sd_checkDrumkit(drumkitMenuCounter);

    switch (menuTab) {
    case 0:
        break;

    case 1:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_LoadData();
        break;

    case 2:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_SaveData();
        break;

    case 3:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_ClearData();
        break;
    }
}

void Controller::drumkit_menuDown() {
    (drumkitMenuCounter > kMinDrumkit) ? drumkitMenuCounter -= 1 : drumkitMenuCounter = kMaxFile;
    sd_checkDrumkit(drumkitMenuCounter);

    switch (menuTab) {
    case 0:
        break;

    case 1:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_LoadData();
        break;

    case 2:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_SaveData();
        break;

    case 3:
        lcd_drawFileBox(menuTab, drumkitStatus);
        lcd_drawDrumkit_ClearData();
        break;
    }
}

void Controller::drumkit_newSelect() {
    alertFlag = true;
    alertType = ALERT_NEWDRUMKIT;
    lcd_drawAlert();
}

void Controller::drumkit_newAction() {
    lcd_clearAlert();

    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        layerInst_reset(i);
    }

    eq_reset();
    filter_reset(0);
    filter_reset(1);
    effect_reset(0);
    effect_reset(1);
    reverb_reset();

    copyLayerInstNum = -1;
    copyLayerFillNum = -1;
}

void Controller::drumkit_loadSelect() {
    if (drumkitStatus == FILE_ACTIVE) {
        alertFlag = true;
        alertType = ALERT_LOADDRUMKIT;
        lcd_drawAlert();
    }
}

void Controller::drumkit_loadAction() {
    lcd_clearAlert();
    switch (sd_loadDrumkit(0, drumkitMenuCounter)) {
    case SD_OK:
        alertType = ALERT_LOADSUCCESS;
        break;

    case SD_ERROR:
        alertType = ALERT_LOADERROR;
        break;

    case SD_ERROR_DETECT:
        alertType = ALERT_MISSINGSAMPLE;
        break;
    }

    copyLayerInstNum = -1;
    copyLayerFillNum = -1;

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

void Controller::drumkit_saveSelect() {
    alertFlag = true;
    (drumkitStatus == FILE_ACTIVE) ? alertType = ALERT_OVERWRITEDRUMKIT : alertType = ALERT_SAVEDRUMKIT;
    lcd_drawAlert();
}

void Controller::drumkit_saveAction() {
    lcd_clearAlert();
    (sd_saveDrumkit(0, drumkitMenuCounter) == SD_OK) ? alertType = ALERT_SAVESUCCESS : alertType = ALERT_SAVEERROR;

    sd_checkDrumkit(drumkitMenuCounter);
    lcd_drawFileBox(menuTab, drumkitStatus);

    sd_getDrumkitLibrary();
    lcd_drawSdData();

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

void Controller::drumkit_clearSelect() {
    if (drumkitStatus != FILE_INACTIVE) {
        alertFlag = true;
        alertType = ALERT_CLEARDRUMKIT;
        lcd_drawAlert();
    }
}

void Controller::drumkit_clearAction() {
    lcd_clearAlert();
    (sd_clearDrumkit(0, drumkitMenuCounter) == SD_OK) ? alertType = ALERT_CLEARSUCCESS : alertType = ALERT_CLEARERROR;

    sd_checkDrumkit(drumkitMenuCounter);
    lcd_drawFileBox(menuTab, drumkitStatus);

    sd_getDrumkitLibrary();
    lcd_drawSdData();

    lcd_drawAlert();
    HAL_Delay(1000);
    lcd_clearAlert();
}

/* System functions ----------------------------------------------------------*/

void Controller::system_select() {
    if (menu != SYSTEM_MENU) {
        preMenu = menu;
        menu = SYSTEM_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawSystemMenu();
    }
}

void Controller::system_reset() {
    system_setVolume(kInitialSystemVolume);
    system_setPan(kInitialSystemPan);
    system_setLimiter(kInitialSystemLimiter);
    system_setMidiIn(kInitialSystemMidiIn);
    system_setMidiOut(kInitialSystemMidiOut);
    system_setSyncIn(kInitialSystemSyncIn);
    system_setSyncOut(kInitialSystemSyncOut);
}

void Controller::system_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 0) ? menuTab += 2 : menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::system_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 2) ? menuTab -= 2 : menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::system_menuUp() {
    switch (menuTab) {
    case 0:
        if (system.volume < kMaxSystemVolume) {
            system_setVolume(system.volume + 1);
        }
        break;

    case 2:
        if (system.pan < kMaxSystemPan) {
            system_setPan(system.pan + 1);
        }
        break;

    case 3:
        system_setLimiter(!system.limiter);
        break;

    case 4:
        if (system.midiIn < kMaxSystemMidiIn) {
            system_setMidiIn(system.midiIn + 1);
        }
        break;

    case 5:
        if (system.midiOut < kMaxSystemMidiOut) {
            system_setMidiOut(system.midiOut + 1);
        }
        break;

    case 6:
        if (system.syncIn < kMaxSystemSyncIn) {
            system_setSyncIn(system.syncIn + 1);
        } else {
            system_setSyncIn(kMinSystemSyncIn);
        }
        break;

    case 7:
        if (system.syncOut < kMaxSystemSyncOut) {
            system_setSyncOut(system.syncOut + 1);
        } else {
            system_setSyncOut(kMinSystemSyncOut);
        }
        break;
    }
}

void Controller::system_menuDown() {
    switch (menuTab) {
    case 0:
        if (system.volume > kMinSystemVolume) {
            system_setVolume(system.volume - 1);
        }
        break;

    case 2:
        if (system.pan > kMinSystemPan) {
            system_setPan(system.pan - 1);
        }
        break;

    case 3:
        system_setLimiter(!system.limiter);
        break;

    case 4:
        if (system.midiIn > kMinSystemMidiIn) {
            system_setMidiIn(system.midiIn - 1);
        }
        break;

    case 5:
        if (system.midiOut > kMinSystemMidiOut) {
            system_setMidiOut(system.midiOut - 1);
        }
        break;

    case 6:
        if (system.syncIn > kMinSystemSyncIn) {
            system_setSyncIn(system.syncIn - 1);
        } else {
            system_setSyncIn(kMaxSystemSyncIn);
        }
        break;

    case 7:
        if (system.syncOut > kMinSystemSyncOut) {
            system_setSyncOut(system.syncOut - 1);
        } else {
            system_setSyncOut(kMaxSystemSyncOut);
        }
        break;
    }
}

void Controller::system_setVolume(uint8_t volume_) {
    if ((volume_ >= kMinSystemVolume) && (volume_ <= kMaxSystemVolume)) {
        // update data
        system.volume = volume_;

        float targetVolume = kSystemVolumeDataLibrary[system.volume].data;
        system_volumeTransition(targetVolume);

        // update lcd
        if (menu == SYSTEM_MENU)
            lcd_drawSystem_VolumeData();
    }
}

void Controller::system_setPan(uint8_t pan_) {
    if ((pan_ >= kMinSystemPan) && (pan_ <= kMaxSystemPan)) {
        // update data
        system.pan = pan_;

        float targetVolumeLeft = kSystemVolumeDataLibrary[kSystemPanDataLibrary[system.pan].left].data;
        float targetVolumeRight = kSystemVolumeDataLibrary[kSystemPanDataLibrary[system.pan].right].data;
        system_panTransition(targetVolumeLeft, targetVolumeRight);

        // update lcd
        if (menu == SYSTEM_MENU)
            lcd_drawSystem_PanData();
    }
}

void Controller::system_setLimiter(bool mode_) {
    // uodate data
    system.limiter = mode_;
    // update lcd
    if (menu == SYSTEM_MENU) {
        lcd_drawSystem_LimiterData();
    }
}

void Controller::system_setMidiIn(uint8_t mode_) {
    if ((mode_ >= kMinSystemMidiIn) && (mode_ <= kMaxSystemMidiIn)) {
        // update data
        system.midiIn = mode_;
        system.midi.rxActive = kSystemMidiDataLibrary[system.midiIn].active;
        system.midi.rxChannel = kSystemMidiDataLibrary[system.midiIn].channel;
        // update lcd
        if (menu == SYSTEM_MENU) {
            lcd_drawSystem_MidiInData();
        }
    }
}

void Controller::system_setMidiOut(uint8_t mode_) {
    if ((mode_ >= kMinSystemMidiOut) && (mode_ <= kMaxSystemMidiOut)) {
        // update data
        system.midiOut = mode_;
        system.midi.txActive = kSystemMidiDataLibrary[system.midiOut].active;
        system.midi.txChannel = kSystemMidiDataLibrary[system.midiOut].channel;
        // update lcd
        if (menu == SYSTEM_MENU) {
            lcd_drawSystem_MidiOutData();
        }
    }
}

void Controller::system_setSyncIn(uint8_t mode_) {
    if ((mode_ >= kMinSystemSyncIn) && (mode_ <= kMaxSystemSyncIn)) {
        // update data
        system.syncIn = mode_;
        system.sync.syncInTempo = kSystemSyncInDataLibrary[system.syncIn].tempoTrigger;
        system.sync.syncInPlay = kSystemSyncInDataLibrary[system.syncIn].playTrigger;
        system.sync.syncInBeat = kSystemSyncInDataLibrary[system.syncIn].beatTrigger;
        if (system.syncIn) {
            system.sync.slaveMode = true;
            system.sync.masterMode = false;
        } else if (system.syncOut) {
            system.sync.slaveMode = false;
            system.sync.masterMode = true;
        } else {
            system.sync.slaveMode = false;
            system.sync.masterMode = false;
        }
        // update lcd
        if (menu == SYSTEM_MENU)
            lcd_drawSystem_SyncInData();
    }
}

void Controller::system_setSyncOut(uint8_t mode_) {
    if ((mode_ >= kMinSystemSyncOut) && (mode_ <= kMaxSystemSyncOut)) {
        // update data
        system.syncOut = mode_;
        switch (mode_) {
        case 0: // off
            HAL_GPIO_WritePin(SYNC_OUT_RX_GPIO_Port, SYNC_OUT_RX_Pin, GPIO_PIN_RESET);
            break;
        case 1: // mode-rw
            HAL_GPIO_WritePin(SYNC_OUT_RX_GPIO_Port, SYNC_OUT_RX_Pin, GPIO_PIN_SET);
            break;
        case 2: // mode-st
            HAL_GPIO_WritePin(SYNC_OUT_RX_GPIO_Port, SYNC_OUT_RX_Pin, GPIO_PIN_RESET);
            break;
        }
        system.sync.syncOutTempo = kSystemSyncOutDataLibrary[system.syncOut].tempoTrigger;
        system.sync.syncOutPlay = kSystemSyncOutDataLibrary[system.syncOut].playTrigger;
        system.sync.syncOutBeat = kSystemSyncOutDataLibrary[system.syncOut].beatTrigger;
        if (system.syncIn) {
            system.sync.slaveMode = true;
            system.sync.masterMode = false;
        } else if (system.syncOut) {
            system.sync.slaveMode = false;
            system.sync.masterMode = true;
        } else {
            system.sync.slaveMode = false;
            system.sync.masterMode = false;
        }
        // update lcd
        if (menu == SYSTEM_MENU)
            lcd_drawSystem_SyncOutData();
    }
}

void Controller::system_volumeTransition(float volumeFloat_) {
    SystemVolumeTransition &vTransition = system.volumeTransition;

    vTransition.targetVolume = volumeFloat_;
    system_calculateVolumeTransition();
    vTransition.active = true;
}

void Controller::system_panTransition(float volumeLeftFloat_, float volumeRightFloat_) {
    SystemPanTransition &pTransition = system.panTransition;

    pTransition.targetVolumeLeft = volumeLeftFloat_;
    pTransition.targetVolumeRight = volumeRightFloat_;

    system_calculatePanTransition();
    pTransition.active = true;
}

void Controller::system_calculateVolumeTransition() {
    SystemVolumeTransition &vTransition = system.volumeTransition;

    if (system.volumeFloat == vTransition.targetVolume) {
        vTransition.actionVolume = SYS_ACTION_NONE;
    } else if (system.volumeFloat < vTransition.targetVolume) {
        vTransition.actionVolume = SYS_ACTION_UP;
    } else if (system.volumeFloat > vTransition.targetVolume) {
        vTransition.actionVolume = SYS_ACTION_DOWN;
    }
}

void Controller::system_calculatePanTransition() {
    SystemPanTransition &pTransition = system.panTransition;

    if (system.volumeLeftFloat == pTransition.targetVolumeLeft) {
        pTransition.actionVolumeLeft = SYS_ACTION_NONE;
    } else if (system.volumeLeftFloat < pTransition.targetVolumeLeft) {
        pTransition.actionVolumeLeft = SYS_ACTION_UP;
    } else if (system.volumeLeftFloat > pTransition.targetVolumeLeft) {
        pTransition.actionVolumeLeft = SYS_ACTION_DOWN;
    }

    if (system.volumeRightFloat == pTransition.targetVolumeRight) {
        pTransition.actionVolumeRight = SYS_ACTION_NONE;
    } else if (system.volumeRightFloat < pTransition.targetVolumeRight) {
        pTransition.actionVolumeRight = SYS_ACTION_UP;
    } else if (system.volumeRightFloat > pTransition.targetVolumeRight) {
        pTransition.actionVolumeRight = SYS_ACTION_DOWN;
    }
}

/* Rhythm functions ----------------------------------------------------------*/

void Controller::preMenuLayerClear() {
    if (preMenu == LAYER_INST_MENU) {
        lcd_clearLayerSelect();
        selectedLayerNum = -1;
    }

    if (preMenu == LAYER_SONG_MENU) {
        if ((layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1) && (selectedBeatNum != -1))
            lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, false);
        lcd_clearLayerSelect();
        selectedLayerNum = -1;
        selectedBeatNum = -1;
    }
}

void Controller::rhythm_select() {
    if (menu != RHYTHM_MENU) {
        preMenu = menu;
        menu = RHYTHM_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawRhythmMenu();
    }

    // if (!wavReadFlag) wavReadFlag = true;  // wavTest
}

void Controller::rhythm_reset() {
    rhythm_setTempo(kInitialTempo);
    rhythm_setMeasure(kInitialMeasure);
    rhythm_setBar(kInitialBar);
    rhythm_setQuantize(kInitialQuantize);
}

void Controller::rhythm_menuRight() {
    if (menuTab < kMaxMenu4Tab) {
        preMenuTab = menuTab;
        menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::rhythm_menuLeft() {
    if (menuTab > kMinMenu4Tab) {
        preMenuTab = menuTab;
        menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::rhythm_menuUp() {
    switch (menuTab) {
    case 0:
        if ((rhythm.tempo < kMaxTempo) && (!system.sync.slaveMode)) {
            rhythm_setTempo(rhythm.tempo + 1);
        }
        break;

    case 1:
        if (!rhythm.measureLock) {
            if (rhythm.measure < kMaxMeasure) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_MEASUREUP;
                    lcd_drawAlert();
                } else {
                    rhythm_setMeasure(rhythm.measure + 1);
                }
            }
        }
        break;

    case 2:
        if (!rhythm.barLock) {
            if (rhythm.bar < kMaxBar) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_BARUP;
                    lcd_drawAlert();
                } else {
                    rhythm_setBar(rhythm.bar + 1);
                }
            }
        }
        break;

    case 3:
        if (!rhythm.quantizeLock) {
            if (rhythm.quantize < kMaxQuantize) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_QUANTIZEUP;
                    lcd_drawAlert();
                } else {
                    rhythm_setQuantize(rhythm.quantize + 1);
                }
            }
        }
        break;
    }
}

void Controller::rhythm_menuDown() {
    switch (menuTab) {
    case 0:
        if ((rhythm.tempo > kMinTempo) && (!system.sync.slaveMode)) {
            rhythm_setTempo(rhythm.tempo - 1);
        }
        break;

    case 1:
        if (!rhythm.measureLock) {
            if (rhythm.measure > kMinMeasure) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_MEASUREDOWN;
                    lcd_drawAlert();
                } else {
                    rhythm_setMeasure(rhythm.measure - 1);
                }
            }
        }
        break;

    case 2:
        if (!rhythm.barLock) {
            if (rhythm.bar > kMinBar) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_BARDOWN;
                    lcd_drawAlert();
                } else {
                    rhythm_setBar(rhythm.bar - 1);
                }
            }
        }
        break;

    case 3:
        if (!rhythm.quantizeLock) {
            if (rhythm.quantize > kMinQuantize) {
                if (playActive) {
                    alertFlag = true;
                    alertType = ALERT_QUANTIZEDOWN;
                    lcd_drawAlert();
                } else {
                    rhythm_setQuantize(rhythm.quantize - 1);
                }
            }
        }
        break;
    }
}

void Controller::rhythm_setTempo(uint8_t tempo_) {
    if ((tempo_ >= kMinTempo) && (tempo_ <= kMaxTempo)) {
        // sync data
        if (system.sync.syncOutTempo)
            sendSyncCommand(tempo_);
        // update data
        rhythm.tempo = tempo_;
        updatePlayTimerPeriod();
        effect[0].delay.update(tempo_);
        effect[1].delay.update(tempo_);
        // update lcd
        if (menu == RHYTHM_MENU)
            lcd_drawRhythm_TempoData();
        if (menu == MAIN_MENU)
            lcd_drawMain_TempoData();
    }
}

void Controller::rhythm_setMeasure(uint8_t measure_) {
    if ((measure_ >= kMinMeasure) && (measure_ <= kMaxMeasure)) {
        // update data
        rhythm.measure = measure_;
        calculateSongInterval();
        adjustMeasureBarTiming();
        // update lcd
        if (menu == RHYTHM_MENU)
            lcd_drawRhythm_MeasureData();
        if (menu == MAIN_MENU)
            lcd_drawMain_MeasureData();
        // update song
        lcd_calculateSongX();
        lcd_resetPlay();
        lcd_drawMeasureBar();
        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            lcd_drawSong(i, activeBankNum);
        }
        reset();
    }
}

void Controller::rhythm_setBar(uint8_t bar_) {
    if ((bar_ >= kMinBar) && (bar_ <= kMaxBar)) {
        // update data
        rhythm.bar = bar_;
        calculateSongInterval();
        adjustMeasureBarTiming();
        // update lcd
        if (menu == RHYTHM_MENU)
            lcd_drawRhythm_BarData();
        if (menu == MAIN_MENU)
            lcd_drawMain_BarData();
        // update song
        lcd_calculateSongX();
        lcd_resetPlay();
        lcd_drawMeasureBar();
        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            lcd_drawSong(i, activeBankNum);
        }
        reset();
    }
}

void Controller::rhythm_setQuantize(uint8_t quantize_) {
    if ((quantize_ >= kMinQuantize) && (quantize_ <= kMaxQuantize)) {
        // update data
        rhythm.quantize = quantize_;
        // update lcd
        if (menu == RHYTHM_MENU)
            lcd_drawRhythm_QuantizeData();
        if (menu == MAIN_MENU)
            lcd_drawMain_QuantizeData();
        // quantize beats
        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            for (uint8_t j = 0; j < kBankLibrarySize; j++) {
                layerSong_quantizeActiveBeats(i, j);
            }
        }
        // update song
        for (uint8_t j = 0; j < kLayerLibrarySize; j++) {
            lcd_drawSong(j, activeBankNum);
        }
        reset();
    }
}

/* Metronome functions -------------------------------------------------------*/

void Controller::metro_select() {
    if (menu != METRO_MENU) {
        preMenu = menu;
        menu = METRO_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawMetroMenu();
    }
}

void Controller::metro_reset() {
    metro_setActive(kInitialMetroActive);
    metro_setPrecount(kInitialMetroPrecount);
    metro_setSample(kInitialMetroSample);
    metro_setVolume(kInitialMetroVolume);
}

void Controller::metro_menuRight() {
    if (menuTab < kMaxMenu4Tab) {
        preMenuTab = menuTab;
        menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::metro_menuLeft() {
    if (menuTab > kMinMenu4Tab) {
        preMenuTab = menuTab;
        menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::metro_menuUp() {
    switch (menuTab) {
    case 0:
        metro_setActive(!metronome.active);
        break;

    case 1:
        metro_setPrecount(!metronome.precount);
        break;

    case 2:
        (metronome.sample < kMaxMetroSample) ? metro_setSample(metronome.sample + 1) : metro_setSample(kMinMetroSample);
        break;

    case 3:
        if (metronome.volume < kMaxMetroVolume) {
            metro_setVolume(metronome.volume + 1);
        }
        break;
    }
}

void Controller::metro_menuDown() {
    switch (menuTab) {
    case 0:
        metro_setActive(!metronome.active);
        break;

    case 1:
        metro_setPrecount(!metronome.precount);
        break;

    case 2:
        (metronome.sample > kMinMetroSample) ? metro_setSample(metronome.sample - 1) : metro_setSample(kMaxMetroSample);
        break;

    case 3:
        if (metronome.volume > kMinMetroVolume) {
            metro_setVolume(metronome.volume - 1);
        }
        break;
    }
}

void Controller::metro_setActive(bool active_) {
    // update data
    metronome.active = active_;
    // update lcd
    if (menu == METRO_MENU)
        lcd_drawMetro_ActiveData();
}

void Controller::metro_setPrecount(bool precount_) {
    // update data
    metronome.precount = precount_;
    calculateSongInterval();
    // update lcd
    if (menu == METRO_MENU)
        lcd_drawMetro_PrecountData();
}

void Controller::metro_setSample(uint8_t sample_) {
    if ((sample_ >= kMinMetroSample) && (sample_ <= kMaxMetroSample)) {
        // update data
        metronome.sample = sample_;
        // update lcd
        if (menu == METRO_MENU)
            lcd_drawMetro_SampleData();
    }
}

void Controller::metro_setVolume(uint8_t volume_) {
    if ((volume_ >= kMinMetroVolume) && (volume_ <= kMaxMetroVolume)) {
        // update data
        metronome.volume = volume_;

        float targetVolume = kMetronomeVolumeDataLibrary[metronome.volume].data;
        metro_volumeTransition(targetVolume);

        // update lcd
        if (menu == METRO_MENU)
            lcd_drawMetro_VolumeData();
    }
}

void Controller::metro_volumeTransition(float volumeFloat_) {
    MetronomeVolumeTransition &vTransition = metronome.volumeTransition;

    vTransition.targetVolume = volumeFloat_;
    metro_calculateVolumeTransition();
    vTransition.active = true;
}

void Controller::metro_calculateVolumeTransition() {
    MetronomeVolumeTransition &vTransition = metronome.volumeTransition;

    if (metronome.volumeFloat == vTransition.targetVolume) {
        vTransition.actionVolume = MET_ACTION_NONE;
    } else if (metronome.volumeFloat < vTransition.targetVolume) {
        vTransition.actionVolume = MET_ACTION_UP;
    } else if (metronome.volumeFloat > vTransition.targetVolume) {
        vTransition.actionVolume = MET_ACTION_DOWN;
    }
}

/* Eq functions --------------------------------------------------------------*/

void Controller::eq_select() {
    if (menu != EQ_MENU) {
        preMenu = menu;
        menu = EQ_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawEqMenu();
    }
}

void Controller::eq_reset() {
    eq_setActive(kInitialEqActive);
    eq_setFreqLowShelf(kInitialEqFreqLowShelf);
    eq_setGainLowShelf(kInitialEqGainLowShelf);
    eq_setFreqHighShelf(kInitialEqFreqHighShelf);
    eq_setGainHighShelf(kInitialEqGainHighShelf);
    for (uint8_t i = 0; i < 4; i++) {
        eq_setFreqPeak(i, kInitialEqFreqPeak[i]);
        eq_setGainPeak(i, kInitialEqGainPeak[i]);
        eq_setQPeak(i, kInitialEqQPeak[i]);
    }
}

void Controller::eq_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        if ((menuTab == 6) && (subMenuTab == 2))
            subMenuTab = 1;
        preMenuTab = menuTab;
        (menuTab == 0) ? menuTab += 2 : menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::eq_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        if (menuTab == 2)
            subMenuTab = 0;
        if ((menuTab == 3) && (subMenuTab == 2))
            subMenuTab = 1;
        preMenuTab = menuTab;
        (menuTab == 2) ? menuTab -= 2 : menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::eq_menuUp() {
    switch (menuTab) {
    case 0:
        eq_setActive(!eq.active);
        break;

    case 2:
        switch (subMenuTab) {
        case 0:
            if (eq.gainLowShelf < kMaxEqGain) {
                eq_setGainLowShelf(eq.gainLowShelf + 1);
            }
            break;

        case 1:
            if (eq.freqLowShelf < kMaxEqFreq) {
                eq_setFreqLowShelf(eq.freqLowShelf + 1);
            }
            break;
        }
        break;

    case 3:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[0] < kMaxEqGain) {
                eq_setGainPeak(0, eq.gainPeak[0] + 1);
            }
            break;

        case 1:
            if (eq.freqPeak[0] < kMaxEqFreq) {
                eq_setFreqPeak(0, eq.freqPeak[0] + 1);
            }
            break;

        case 2:
            if (eq.qPeak[0] < kMaxEqQ) {
                eq_setQPeak(0, eq.qPeak[0] + 1);
            }
            break;
        }
        break;

    case 4:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[1] < kMaxEqGain) {
                eq_setGainPeak(1, eq.gainPeak[1] + 1);
            }
            break;

        case 1:
            if (eq.freqPeak[1] < kMaxEqFreq) {
                eq_setFreqPeak(1, eq.freqPeak[1] + 1);
            }
            break;

        case 2:
            if (eq.qPeak[1] < kMaxEqQ) {
                eq_setQPeak(1, eq.qPeak[1] + 1);
            }
            break;
        }
        break;

    case 5:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[2] < kMaxEqGain) {
                eq_setGainPeak(2, eq.gainPeak[2] + 1);
            }
            break;

        case 1:
            if (eq.freqPeak[2] < kMaxEqFreq) {
                eq_setFreqPeak(2, eq.freqPeak[2] + 1);
            }
            break;

        case 2:
            if (eq.qPeak[2] < kMaxEqQ) {
                eq_setQPeak(2, eq.qPeak[2] + 1);
            }
            break;
        }
        break;

    case 6:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[3] < kMaxEqGain) {
                eq_setGainPeak(3, eq.gainPeak[3] + 1);
            }
            break;

        case 1:
            if (eq.freqPeak[3] < kMaxEqFreq) {
                eq_setFreqPeak(3, eq.freqPeak[3] + 1);
            }
            break;

        case 2:
            if (eq.qPeak[3] < kMaxEqQ) {
                eq_setQPeak(3, eq.qPeak[3] + 1);
            }
            break;
        }
        break;

    case 7:
        switch (subMenuTab) {
        case 0:
            if (eq.gainHighShelf < kMaxEqGain) {
                eq_setGainHighShelf(eq.gainHighShelf + 1);
            }
            break;

        case 1:
            if (eq.freqHighShelf < kMaxEqFreq) {
                eq_setFreqHighShelf(eq.freqHighShelf + 1);
            }
            break;
        }
        break;
    }
}

void Controller::eq_menuDown() {
    switch (menuTab) {
    case 0:
        eq_setActive(!eq.active);
        break;

    case 2:
        switch (subMenuTab) {
        case 0:
            if (eq.gainLowShelf > kMinEqGain) {
                eq_setGainLowShelf(eq.gainLowShelf - 1);
            }
            break;

        case 1:
            if (eq.freqLowShelf > kMinEqFreq) {
                eq_setFreqLowShelf(eq.freqLowShelf - 1);
            }
            break;
        }
        break;

    case 3:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[0] > kMinEqGain) {
                eq_setGainPeak(0, eq.gainPeak[0] - 1);
            }
            break;

        case 1:
            if (eq.freqPeak[0] > kMinEqFreq) {
                eq_setFreqPeak(0, eq.freqPeak[0] - 1);
            }
            break;

        case 2:
            if (eq.qPeak[0] > kMinEqQ) {
                eq_setQPeak(0, eq.qPeak[0] - 1);
            }
            break;
        }
        break;

    case 4:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[1] > kMinEqGain) {
                eq_setGainPeak(1, eq.gainPeak[1] - 1);
            }
            break;

        case 1:
            if (eq.freqPeak[1] > kMinEqFreq) {
                eq_setFreqPeak(1, eq.freqPeak[1] - 1);
            }
            break;

        case 2:
            if (eq.qPeak[1] > kMinEqQ) {
                eq_setQPeak(1, eq.qPeak[1] - 1);
            }
            break;
        }
        break;

    case 5:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[2] > kMinEqGain) {
                eq_setGainPeak(2, eq.gainPeak[2] - 1);
            }
            break;

        case 1:
            if (eq.freqPeak[2] > kMinEqFreq) {
                eq_setFreqPeak(2, eq.freqPeak[2] - 1);
            }
            break;

        case 2:
            if (eq.qPeak[2] > kMinEqQ) {
                eq_setQPeak(2, eq.qPeak[2] - 1);
            }
            break;
        }
        break;

    case 6:
        switch (subMenuTab) {
        case 0:
            if (eq.gainPeak[3] > kMinEqGain) {
                eq_setGainPeak(3, eq.gainPeak[3] - 1);
            }
            break;

        case 1:
            if (eq.freqPeak[3] > kMinEqFreq) {
                eq_setFreqPeak(3, eq.freqPeak[3] - 1);
            }
            break;

        case 2:
            if (eq.qPeak[3] > kMinEqQ) {
                eq_setQPeak(3, eq.qPeak[3] - 1);
            }
            break;
        }
        break;

    case 7:
        switch (subMenuTab) {
        case 0:
            if (eq.gainHighShelf > kMinEqGain) {
                eq_setGainHighShelf(eq.gainHighShelf - 1);
            }
            break;

        case 1:
            if (eq.freqHighShelf > kMinEqFreq) {
                eq_setFreqHighShelf(eq.freqHighShelf - 1);
            }
        }
        break;
    }
}

void Controller::eq_setActive(bool active_) {
    if ((menu == EQ_MENU) && (!eq.genTransition.active)) {
        // update data
        // if (active_) eq.cleanMemory();
        eq_genTransition(EQ_MODE_ACTIVE, eq.active, active_);
        eq.active = active_;
        // update lcd
        lcd_drawEq_ActiveData();
    } else if (!eq.genTransition.active) {
        // update data
        eq.active = active_;
    }
}

void Controller::eq_setFreqLowShelf(uint8_t freq_) {
    if ((freq_ >= kMinEqFreq) && (freq_ <= kMaxEqFreq)) {
        // update data
        eq.freqLowShelf = freq_;
        eq.calculateCoefLowShelf();
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_LowShelfData();
    }
}

void Controller::eq_setGainLowShelf(uint8_t gain_) {
    if ((gain_ >= kMinEqGain) && (gain_ <= kMaxEqGain)) {
        // update data
        eq.gainLowShelf = gain_;
        eq.calculateCoefLowShelf();
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_LowShelfData();
    }
}

void Controller::eq_setFreqHighShelf(uint8_t freq_) {
    if ((freq_ >= kMinEqFreq) && (freq_ <= kMaxEqFreq)) {
        // update data
        eq.freqHighShelf = freq_;
        eq.calculateCoefHighShelf();
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_HighShelfData();
    }
}

void Controller::eq_setGainHighShelf(uint8_t gain_) {
    if ((gain_ >= kMinEqGain) && (gain_ <= kMaxEqGain)) {
        // update data
        eq.gainHighShelf = gain_;
        eq.calculateCoefHighShelf();
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_HighShelfData();
    }
}

void Controller::eq_setFreqPeak(uint8_t peakNum_, uint8_t freq_) {
    if ((freq_ >= kMinEqFreq) && (freq_ <= kMaxEqFreq)) {
        // update data
        eq.freqPeak[peakNum_] = freq_;
        eq.calculateCoefPeak(peakNum_);
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_PeakData(peakNum_);
    }
}

void Controller::eq_setGainPeak(uint8_t peakNum_, uint8_t gain_) {
    if ((gain_ >= kMinEqGain) && (gain_ <= kMaxEqGain)) {
        // update data
        eq.gainPeak[peakNum_] = gain_;
        eq.calculateCoefPeak(peakNum_);
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_PeakData(peakNum_);
    }
}

void Controller::eq_setQPeak(uint8_t peakNum_, uint8_t q_) {
    if ((q_ >= kMinEqQ) && (q_ <= kMaxEqQ)) {
        // update data
        eq.qPeak[peakNum_] = q_;
        eq.calculateCoefPeak(peakNum_);
        // update lcd
        if (menu == EQ_MENU)
            lcd_drawEq_PeakData(peakNum_);
    }
}

void Controller::eq_genTransition(EqTransitionMode mode_, bool activeActive_, bool targetActive_) {
    EqGenTransition &gTransition = eq.genTransition;

    gTransition.mode = mode_;
    gTransition.phase = EQ_PHASE_A;

    gTransition.activeActive = activeActive_;
    gTransition.targetActive = targetActive_;

    switch (mode_) {
    case EQ_MODE_NONE:
        break;

    case EQ_MODE_ACTIVE:
        switch (targetActive_) {
        case true:
            gTransition.activeDry = 1.0;
            gTransition.targetDry = 0.0;

            gTransition.activeWet = 0.0;
            gTransition.targetWet = 1.0;

            transitionShowFlag = 2;
            break;

        case false:
            gTransition.activeDry = 0.0;
            gTransition.targetDry = 1.0;

            gTransition.activeWet = 1.0;
            gTransition.targetWet = 0.0;

            transitionShowFlag = 1;
            break;
        }
        break;
    }

    eq_calculateActiveTransition();
    gTransition.active = true;
}

void Controller::eq_calculateActiveTransition() {
    EqGenTransition &gTransition = eq.genTransition;

    if (gTransition.activeDry == gTransition.targetDry) {
        gTransition.actionDry = EQ_ACTION_NONE;
    } else if (gTransition.activeDry < gTransition.targetDry) {
        gTransition.actionDry = EQ_ACTION_UP;
    } else if (gTransition.activeDry > gTransition.targetDry) {
        gTransition.actionDry = EQ_ACTION_DOWN;
    }

    if (gTransition.activeWet == gTransition.targetWet) {
        gTransition.actionWet = EQ_ACTION_NONE;
    } else if (gTransition.activeWet < gTransition.targetWet) {
        gTransition.actionWet = EQ_ACTION_UP;
    } else if (gTransition.activeWet > gTransition.targetWet) {
        gTransition.actionWet = EQ_ACTION_DOWN;
    }
}

/* Filter functions ---------------------------------------------------------*/

void Controller::filter_select() {
    if (menu == FILTER_0_MENU) {
        preMenu = menu;
        menu = FILTER_1_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawFilterMenu(1);
    } else if (menu == FILTER_1_MENU) {
        preMenu = menu;
        menu = FILTER_0_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawFilterMenu(0);
    } else {
        preMenu = menu;
        menu = FILTER_0_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawFilterMenu(0);
    }
}

void Controller::filter_reset(uint8_t filterNum_) {
    filter_setActive(filterNum_, kInitialFilterActive);
    filter_setType(filterNum_, kInitialFilterType);
    filter_setFreq(filterNum_, kInitialFilterFreq);
    filter_setRes(filterNum_, kInitialFilterRes);
    filter_setSlope(filterNum_, kInitialFilterSlope);
    filter_setDry(filterNum_, kInitialFilterDry);
    filter_setWet(filterNum_, kInitialFilterWet);

    Filter &filter_ = filter[filterNum_];

    filter_.dataIn[0] = 0;
    filter_.dataIn[1] = 0;
    filter_.dataIn[2] = 0;

    filter_.dataOut[0] = 0;
    filter_.dataOut[1] = 0;
    filter_.dataOut[2] = 0;
}

void Controller::filter_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 0) ? menuTab += 2 : menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::filter_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 2) ? menuTab -= 2 : menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::filter_menuUp() {
    uint8_t filterNum;
    (menu == FILTER_0_MENU) ? filterNum = 0 : filterNum = 1;

    switch (menuTab) {
    case 0:
        filter_setActive(filterNum, !filter[filterNum].active);
        break;

    case 2:
        if (filter[filterNum].type < kMaxFilterType) {
            filter_setType(filterNum, filter[filterNum].type + 1);
        } else {
            filter_setType(filterNum, kMinFilterType);
        }
        break;

    case 3:
        if (filter[filterNum].freq < kMaxFilterFreq) {
            filter_setFreq(filterNum, filter[filterNum].freq + 1);
        }
        break;

    case 4:
        if (filter[filterNum].res < kMaxFilterRes) {
            filter_setRes(filterNum, filter[filterNum].res + 1);
        }
        break;

    case 5:
        if (filter[filterNum].slope < kMaxFilterSlope) {
            filter_setSlope(filterNum, filter[filterNum].slope + 1);
        }
        break;

    case 6:
        if (filter[filterNum].dry < kMaxFilterDry) {
            filter_setDry(filterNum, filter[filterNum].dry + 1);
        } else if ((filter[filterNum].dry == kMaxFilterDry) && (filter[filterNum].wet > kMinFilterWet)) {
            filter_setWet(filterNum, filter[filterNum].wet - 1);
        }
        break;

    case 7:
        if (filter[filterNum].wet < kMaxFilterWet) {
            filter_setWet(filterNum, filter[filterNum].wet + 1);
        } else if ((filter[filterNum].wet == kMaxFilterWet) && (filter[filterNum].dry > kMinFilterDry)) {
            filter_setDry(filterNum, filter[filterNum].dry - 1);
        }
        break;
    }
}

void Controller::filter_menuDown() {
    uint8_t filterNum;
    (menu == FILTER_0_MENU) ? filterNum = 0 : filterNum = 1;

    switch (menuTab) {
    case 0:
        filter_setActive(filterNum, !filter[filterNum].active);
        break;

    case 2:
        if (filter[filterNum].type > kMinFilterType) {
            filter_setType(filterNum, filter[filterNum].type - 1);
        } else {
            filter_setType(filterNum, kMaxFilterType);
        }
        break;

    case 3:
        if (filter[filterNum].freq > kMinFilterFreq) {
            filter_setFreq(filterNum, filter[filterNum].freq - 1);
        }
        break;

    case 4:
        if (filter[filterNum].res > kMinFilterRes) {
            filter_setRes(filterNum, filter[filterNum].res - 1);
        }
        break;

    case 5:
        if (filter[filterNum].slope > kMinFilterSlope) {
            filter_setSlope(filterNum, filter[filterNum].slope - 1);
        }
        break;

    case 6:
        if (filter[filterNum].dry > kMinFilterDry) {
            filter_setDry(filterNum, filter[filterNum].dry - 1);
        }
        break;

    case 7:
        if (filter[filterNum].wet > kMinFilterWet) {
            filter_setWet(filterNum, filter[filterNum].wet - 1);
        }
        break;
    }
}

void Controller::filter_setActive(uint8_t filterNum_, bool active_) {
    Filter &filter_ = filter[filterNum_];

    if ((((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1))) && (!filter_.genTransition.active)) {
        // update data
        // if (active_) filter_.cleanMemory();
        filter_genTransition(filterNum_, FIL_MODE_ACTIVE, filter_.active, active_, filter_.type, filter_.type);
        filter_.active = active_;
        // update lcd
        lcd_drawFilter_ActiveData(filterNum_);
    } else if (!filter[filterNum_].genTransition.active) {
        // update data
        filter_.active = active_;
        // update lcd
        if (menu == MAIN_MENU) {
            switch (filterNum_) {
            case 0:
                lcd_drawMain_Filter0Data();
                break;

            case 1:
                lcd_drawMain_Filter1Data();
                break;
            }
        }
    }
}

void Controller::filter_setType(uint8_t filterNum_, uint8_t type_) {
    Filter &filter_ = filter[filterNum_];

    if ((((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1))) && (!filter_.genTransition.active)) {
        // update data
        if (filter_.active) {
            filter_genTransition(filterNum_, FIL_MODE_TYPE, filter_.active, filter_.active, filter_.type, type_);
            filter_.type = type_;
        } else {
            filter_.type = type_;
            filter_.calculateCoef();
        }
        // update lcd
        lcd_drawFilter_TypeData(filterNum_);
    } else if (!filter[filterNum_].genTransition.active) {
        // update data
        filter_.type = type_;
        filter_.calculateCoef();
        // update lcd
        if (menu == MAIN_MENU) {
            switch (filterNum_) {
            case 0:
                lcd_drawMain_Filter0Data();
                break;

            case 1:
                lcd_drawMain_Filter1Data();
                break;
            }
        }
    }
}

void Controller::filter_setFreq(uint8_t filterNum_, uint8_t freq_) {
    if ((freq_ >= kMinFilterFreq) && (freq_ <= kMaxFilterFreq)) {
        // update data
        filter[filterNum_].freq = freq_;
        filter[filterNum_].calculateCoef();
        // update lcd
        if (((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1)))
            lcd_drawFilter_FreqData(filterNum_);
    }
}

void Controller::filter_setRes(uint8_t filterNum_, uint8_t res_) {
    if ((res_ >= kMinFilterRes) && (res_ <= kMaxFilterRes)) {
        // update data
        filter[filterNum_].res = res_;
        filter[filterNum_].calculateCoef();
        // update lcd
        if (((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1)))
            lcd_drawFilter_ResData(filterNum_);
    }
}

void Controller::filter_setSlope(uint8_t filterNum_, uint8_t slope_) {
    if ((slope_ >= kMinFilterSlope) && (slope_ <= kMaxFilterSlope)) {
        // update data
        filter[filterNum_].slope = slope_;
        filter[filterNum_].calculateCoef();
        // update lcd
        if (((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1)))
            lcd_drawFilter_SlopeData(filterNum_);
    }
}

void Controller::filter_setDry(uint8_t filterNum_, uint8_t dry_) {
    Filter &filter_ = filter[filterNum_];

    if ((dry_ >= kMinFilterDry) && (dry_ <= kMaxFilterDry) && (!filter_.mixTransition.active)) {
        // update data
        uint8_t targetDry = dry_;
        uint8_t targetWet = filter_.wet;

        if ((filter_.limitMix) && ((targetDry + targetWet) > filter_.limitMixData)) {
            targetWet = filter_.limitMixData - targetDry;
        }

        filter_.dry = targetDry;
        filter_.wet = targetWet;

        float targetDryFloat = kFilterMixDataLibrary[filter_.dry].data;
        float targetWetFloat = kFilterMixDataLibrary[filter_.wet].data;

        if (filter_.active) {
            filter_mixTransition(filterNum_, targetDryFloat, targetWetFloat);
        } else {
            filter_.dryFloat = kFilterMixDataLibrary[filter_.dry].data;
            filter_.wetFloat = kFilterMixDataLibrary[filter_.wet].data;
        }

        // update lcd
        if (((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1))) {
            lcd_drawFilter_DryData(filterNum_);
            lcd_drawFilter_WetData(filterNum_);
        }
    }
}

void Controller::filter_setWet(uint8_t filterNum_, uint8_t wet_) {
    Filter &filter_ = filter[filterNum_];

    if ((wet_ >= kMinFilterWet) && (wet_ <= kMaxFilterWet) && (!filter_.mixTransition.active)) {
        // update data
        uint8_t targetDry = filter_.dry;
        uint8_t targetWet = wet_;

        if ((filter_.limitMix) && ((targetDry + targetWet) > filter_.limitMixData)) {
            targetDry = filter_.limitMixData - targetWet;
        }

        filter_.dry = targetDry;
        filter_.wet = targetWet;

        float targetDryFloat = kFilterMixDataLibrary[filter_.dry].data;
        float targetWetFloat = kFilterMixDataLibrary[filter_.wet].data;

        if (filter_.active) {
            filter_mixTransition(filterNum_, targetDryFloat, targetWetFloat);
        } else {
            filter_.dryFloat = kFilterMixDataLibrary[filter_.dry].data;
            filter_.wetFloat = kFilterMixDataLibrary[filter_.wet].data;
        }

        // update lcd
        if (((menu == FILTER_0_MENU) && (filterNum_ == 0)) || ((menu == FILTER_1_MENU) && (filterNum_ == 1))) {
            lcd_drawFilter_DryData(filterNum_);
            lcd_drawFilter_WetData(filterNum_);
        }
    }
}

void Controller::filter_genTransition(uint8_t filterNum_, FilterTransitionMode mode_, bool activeActive_, bool targetActive_, uint8_t activeType_, uint8_t targetType_) {
    Filter &filter_ = filter[filterNum_];
    FilterGenTransition &gTransition = filter_.genTransition;

    gTransition.mode = mode_;
    gTransition.phase = FIL_PHASE_A;

    gTransition.activeActive = activeActive_;
    gTransition.targetActive = targetActive_;

    gTransition.activeType = activeType_;
    gTransition.targetType = targetType_;

    switch (mode_) {
    case FIL_MODE_NONE:
        break;

    case FIL_MODE_ACTIVE:
        switch (targetActive_) {
        case true:
            gTransition.activeDry = 1.0;
            gTransition.targetDry = 0.0;

            gTransition.activeWet = 0.0;
            gTransition.targetWet = 1.0;

            filter_.calculateCoef();

            transitionShowFlag = 2;
            break;

        case false:
            gTransition.activeDry = 0.0;
            gTransition.targetDry = 1.0;

            gTransition.activeWet = 1.0;
            gTransition.targetWet = 0.0;

            transitionShowFlag = 1;
            break;
        }
        break;

    case FIL_MODE_TYPE:
        gTransition.activeDry = 0.0;
        gTransition.targetDry = 1.0;

        gTransition.activeWet = 1.0;
        gTransition.targetWet = 0.0;

        transitionShowFlag = 1;
        break;
    }

    filter_calculateGenTransition(filterNum_);
    gTransition.active = true;
}

void Controller::filter_mixTransition(uint8_t filterNum_, float dryFloat_, float wetFloat_) {
    Filter &filter_ = filter[filterNum_];
    FilterMixTransition &mTransition = filter_.mixTransition;

    mTransition.targetDry = dryFloat_;
    mTransition.targetWet = wetFloat_;

    filter_calculateMixTransition(filterNum_);
    mTransition.active = true;
}

void Controller::filter_calculateGenTransition(uint8_t filterNum_) {
    Filter &filter_ = filter[filterNum_];
    FilterGenTransition &gTransition = filter_.genTransition;

    if (gTransition.activeDry == gTransition.targetDry) {
        gTransition.actionDry = FIL_ACTION_NONE;
    } else if (gTransition.activeDry < gTransition.targetDry) {
        gTransition.actionDry = FIL_ACTION_UP;
    } else if (gTransition.activeDry > gTransition.targetDry) {
        gTransition.actionDry = FIL_ACTION_DOWN;
    }

    if (gTransition.activeWet == gTransition.targetWet) {
        gTransition.actionWet = FIL_ACTION_NONE;
    } else if (gTransition.activeWet < gTransition.targetWet) {
        gTransition.actionWet = FIL_ACTION_UP;
    } else if (gTransition.activeWet > gTransition.targetWet) {
        gTransition.actionWet = FIL_ACTION_DOWN;
    }
}

void Controller::filter_calculateMixTransition(uint8_t filterNum_) {
    Filter &filter_ = filter[filterNum_];
    FilterMixTransition &mTransition = filter_.mixTransition;

    if (filter_.dryFloat == mTransition.targetDry) {
        mTransition.actionDry = FIL_ACTION_NONE;
    } else if (filter_.dryFloat < mTransition.targetDry) {
        mTransition.actionDry = FIL_ACTION_UP;
    } else if (filter_.dryFloat > mTransition.targetDry) {
        mTransition.actionDry = FIL_ACTION_DOWN;
    }

    if (filter_.wetFloat == mTransition.targetWet) {
        mTransition.actionWet = FIL_ACTION_NONE;
    } else if (filter_.wetFloat < mTransition.targetWet) {
        mTransition.actionWet = FIL_ACTION_UP;
    } else if (filter_.wetFloat > mTransition.targetWet) {
        mTransition.actionWet = FIL_ACTION_DOWN;
    }
}

/* Effect functions ----------------------------------------------------------*/

void Controller::effect_select() {
    if (menu == EFFECT_0_MENU) {
        preMenu = menu;
        menu = EFFECT_1_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawEffectMenu(1);
    } else if (menu == EFFECT_1_MENU) {
        preMenu = menu;
        menu = EFFECT_0_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawEffectMenu(0);
    } else {
        preMenu = menu;
        menu = EFFECT_0_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawEffectMenu(0);
    }
}

void Controller::effect_reset(uint8_t effectNum_) {
    effect_setActive(effectNum_, kInitialEffectActive);
    effect_setType(effectNum_, kInitialEffectType);

    effect[effectNum_].reset();
}

void Controller::effect_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        preMenuTab = menuTab;
        if (menuTab == 0) {
            menuTab += 2;
        } else {
            menuTab += 1;
        }
        lcd_transitionSelect();
    }
}

void Controller::effect_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        preMenuTab = menuTab;
        if (menuTab == 2) {
            menuTab -= 2;
        } else {
            menuTab -= 1;
        }
        lcd_transitionSelect();
    }
}

void Controller::effect_menuUp() {
    uint8_t effectNum_;
    (menu == EFFECT_0_MENU) ? effectNum_ = 0 : effectNum_ = 1;
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;
    SubEffect &subEffect_ = effect[effectNum_].subEffect[type_];

    switch (menuTab) {
    case 0:
        effect_setActive(effectNum_, !effect[effectNum_].active);
        break;

    case 2:
        if (effect[effectNum_].type < kMaxEffectType) {
            effect_setType(effectNum_, effect[effectNum_].type + 1);
        } else {
            effect_setType(effectNum_, kMinEffectType);
        }
        break;

    case 3:
        if (subEffect_.aData < subEffect_.kMaxAData) {
            effect_setAData(effectNum_, type_, subEffect_.aData + 1);
        }
        break;

    case 4:
        if (subEffect_.bData < subEffect_.kMaxBData) {
            effect_setBData(effectNum_, type_, subEffect_.bData + 1);
        }
        break;

    case 5:
        if (subEffect_.cData < subEffect_.kMaxCData) {
            effect_setCData(effectNum_, type_, subEffect_.cData + 1);
        }
        break;

    case 6:
        if (subEffect_.dData < subEffect_.kMaxDData) {
            effect_setDData(effectNum_, type_, subEffect_.dData + 1);
        } else if ((type_ != EF_COMPRESSOR) && (type_ != EF_EXPANDER) && (subEffect_.dData == subEffect_.kMaxDData) && (subEffect_.eData > subEffect_.kMinEData)) {
            effect_setEData(effectNum_, type_, subEffect_.eData - 1);
        }
        break;

    case 7:
        if (subEffect_.eData < subEffect_.kMaxEData) {
            effect_setEData(effectNum_, type_, subEffect_.eData + 1);
        } else if ((type_ != EF_COMPRESSOR) && (type_ != EF_EXPANDER) && (subEffect_.eData == subEffect_.kMaxEData) && (subEffect_.dData > subEffect_.kMinDData)) {
            effect_setDData(effectNum_, type_, subEffect_.dData - 1);
        }
        break;
    }
}

void Controller::effect_menuDown() {
    uint8_t effectNum_;
    (menu == EFFECT_0_MENU) ? effectNum_ = 0 : effectNum_ = 1;
    Effect &effect_ = effect[effectNum_];
    uint8_t type_ = effect[effectNum_].type;
    SubEffect &subEffect_ = effect[effectNum_].subEffect[type_];

    switch (menuTab) {
    case 0:
        effect_setActive(effectNum_, !effect[effectNum_].active);
        break;

    case 2:
        if (effect[effectNum_].type > kMinEffectType) {
            effect_setType(effectNum_, effect[effectNum_].type - 1);
        } else {
            effect_setType(effectNum_, kMaxEffectType);
        }
        break;

    case 3:
        if (subEffect_.aData > subEffect_.kMinAData) {
            effect_setAData(effectNum_, type_, subEffect_.aData - 1);
        }
        break;

    case 4:
        if (subEffect_.bData > subEffect_.kMinBData) {
            effect_setBData(effectNum_, type_, subEffect_.bData - 1);
        }
        break;

    case 5:
        if (subEffect_.cData > subEffect_.kMinCData) {
            effect_setCData(effectNum_, type_, subEffect_.cData - 1);
        }
        break;

    case 6:
        if (subEffect_.dData > subEffect_.kMinDData) {
            effect_setDData(effectNum_, type_, subEffect_.dData - 1);
        }
        break;

    case 7:
        if (subEffect_.eData > subEffect_.kMinEData) {
            effect_setEData(effectNum_, type_, subEffect_.eData - 1);
        }
        break;
    }
}

void Controller::effect_setActive(uint8_t effectNum_, bool active_) {
    Effect &effect_ = effect[effectNum_];

    if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (!effect_.genTransition.active)) {
        // update data
        if (active_)
            effect_cleanMemory(effectNum_, effect_.type);
        effect_genTransition(effectNum_, EF_MODE_ACTIVE, effect_.active, active_, effect_.type, effect_.type);
        effect_.active = active_;
        // update lcd
        lcd_drawEffect_ActiveData(effectNum_);
    } else if (!effect[effectNum_].genTransition.active) {
        // update data
        effect_.active = active_;
        // update lcd
        if (menu == MAIN_MENU) {
            switch (effectNum_) {
            case 0:
                lcd_drawMain_Effect0Data();
                break;

            case 1:
                lcd_drawMain_Effect1Data();
                break;
            }
        }
    }
}

void Controller::effect_setType(uint8_t effectNum_, uint8_t type_) {
    Effect &effect_ = effect[effectNum_];

    if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (!effect_.genTransition.active)) {
        // update data
        if (effect_.active) {
            effect_cleanMemory(effectNum_, type_);
            effect_genTransition(effectNum_, EF_MODE_TYPE, effect_.active, effect_.active, effect_.type, type_);
            effect_.type = type_;
        } else {
            effect_.type = type_;
        }
        // update lcd
        lcd_drawEffect_TypeData(effectNum_);
        lcd_drawEffect_AData(effectNum_);
        lcd_drawEffect_BData(effectNum_);
        lcd_drawEffect_CData(effectNum_);
        lcd_drawEffect_DData(effectNum_);
        lcd_drawEffect_EData(effectNum_);
    } else if (!effect[effectNum_].genTransition.active) {
        // update data
        effect_cleanMemory(effectNum_, type_);
        effect_.type = type_;
        // update lcd
        if (menu == MAIN_MENU) {
            switch (effectNum_) {
            case 0:
                lcd_drawMain_Effect0Data();
                break;

            case 1:
                lcd_drawMain_Effect1Data();
                break;
            }
        }
    }
}

void Controller::effect_setAData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t aData_) {
    Effect &effect_ = effect[effectNum_];
    SubEffect &subEffect_ = effect[effectNum_].subEffect[subEffectNum_];

    Delay &delay = effect_.delay;
    Chorus &chorus = effect_.chorus;
    Flanger &flanger = effect_.flanger;
    Phaser &phaser = effect_.phaser;
    Compressor &compressor = effect_.compressor;
    Expander &expander = effect_.expander;
    Overdrive &overdrive = effect_.overdrive;
    Distortion &distortion = effect_.distortion;
    Bitcrusher &bitcrusher = effect_.bitcrusher;

    if ((aData_ >= subEffect_.kMinAData) && (aData_ <= subEffect_.kMaxAData)) {
        // update data
        switch (subEffectNum_) {
        case EF_DELAY: // time
            if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.active) && (!effect_.genTransition.active)) {
                effect_genTransition(effectNum_, EF_MODE_TIME, effect_.active, effect_.active, effect_.type, effect_.type);
                subEffect_.aData = aData_;
                delay.aTime = aData_;
            } else if (!effect[effectNum_].genTransition.active) {
                subEffect_.aData = aData_;
                delay.aTime = aData_;
                delay.time = kDelayTimeDataLibrary[aData_].data;
                delay.update(rhythm.tempo);
            }
            break;

        case EF_CHORUS: // time
            if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.active) && (!effect_.genTransition.active)) {
                effect_genTransition(effectNum_, EF_MODE_TIME, effect_.active, effect_.active, effect_.type, effect_.type);
                subEffect_.aData = aData_;
                chorus.aTime = aData_;
            } else if (!effect[effectNum_].genTransition.active) {
                subEffect_.aData = aData_;
                chorus.aTime = aData_;
                chorus.time = kChorusTimeDataLibrary[aData_].data;
                chorus.update();
            }
            break;

        case EF_FLANGER: // time
            if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.active) && (!effect_.genTransition.active)) {
                effect_genTransition(effectNum_, EF_MODE_TIME, effect_.active, effect_.active, effect_.type, effect_.type);
                subEffect_.aData = aData_;
                flanger.aTime = aData_;
            } else if (!effect[effectNum_].genTransition.active) {
                subEffect_.aData = aData_;
                flanger.aTime = aData_;
                flanger.time = kFlangerTimeDataLibrary[aData_].data;
                flanger.update();
            }
            break;

        case EF_PHASER: // startFreq
            if (aData_ <= phaser.bEndFreq) {
                subEffect_.aData = aData_;
                phaser.aStartFreq = aData_;
                phaser.startFreq = kPhaserFreqDataLibrary[aData_].data;
                phaser.update();
            }
            break;

        case EF_COMPRESSOR: // threshold
            subEffect_.aData = aData_;
            compressor.aThreshold = aData_;
            compressor.threshold = kCompressorThresholdDataLibrary[aData_].data;
            compressor.update();
            break;

        case EF_EXPANDER: // threshold
            subEffect_.aData = aData_;
            expander.aThreshold = aData_;
            expander.threshold = kExpanderThresholdDataLibrary[aData_].data;
            expander.update();
            break;

        case EF_OVERDRIVE: // gain
            subEffect_.aData = aData_;
            overdrive.aGain = aData_;
            overdrive.gaindB = kOverdriveGainDataLibrary[aData_].data;
            overdrive.update();
            break;

        case EF_DISTORTION: // gain
            subEffect_.aData = aData_;
            distortion.aGain = aData_;
            distortion.gaindB = kDistortionGainDataLibrary[aData_].data;
            distortion.update();
            break;

        case EF_BITCRUSHER: // resolution
            subEffect_.aData = aData_;
            bitcrusher.aResolution = aData_;
            bitcrusher.resolution = kBitcrusherResolutionDataLibrary[aData_].data;
            bitcrusher.update();
            break;
        }
        // update lcd
        if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.type == subEffectNum_)) {
            lcd_drawEffect_AData(effectNum_);
        }
    }
}

void Controller::effect_setBData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t bData_) {
    Effect &effect_ = effect[effectNum_];
    SubEffect &subEffect_ = effect[effectNum_].subEffect[subEffectNum_];

    Delay &delay = effect_.delay;
    Chorus &chorus = effect_.chorus;
    Flanger &flanger = effect_.flanger;
    Phaser &phaser = effect_.phaser;
    Compressor &compressor = effect_.compressor;
    Expander &expander = effect_.expander;
    Overdrive &overdrive = effect_.overdrive;
    Distortion &distortion = effect_.distortion;
    Bitcrusher &bitcrusher = effect_.bitcrusher;

    if ((bData_ >= subEffect_.kMinBData) && (bData_ <= subEffect_.kMaxBData)) {
        // update data
        switch (subEffectNum_) {
        case EF_DELAY: // level
            subEffect_.bData = bData_;
            delay.bLevel = bData_;
            delay.level = kDelayLevelDataLibrary[bData_].data;
            break;

        case EF_CHORUS: // feedback
            if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.active) && (!effect_.genTransition.active)) {
                effect_genTransition(effectNum_, EF_MODE_FEEDBACK, effect_.active, effect_.active, effect_.type, effect_.type);
                subEffect_.bData = bData_;
                chorus.bFeedback = bData_;
            } else if (!effect[effectNum_].genTransition.active) {
                subEffect_.bData = bData_;
                chorus.bFeedback = bData_;
                chorus.feedback = kChorusFeedbackDataLibrary[bData_].data;
                chorus.update();
            }
            break;

        case EF_FLANGER: // feedback
            if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.active) && (!effect_.genTransition.active)) {
                effect_genTransition(effectNum_, EF_MODE_FEEDBACK, effect_.active, effect_.active, effect_.type, effect_.type);
                subEffect_.bData = bData_;
                flanger.bFeedback = bData_;
            } else if (!effect[effectNum_].genTransition.active) {
                subEffect_.bData = bData_;
                flanger.bFeedback = bData_;
                flanger.feedback = kFlangerFeedbackDataLibrary[bData_].data;
                flanger.update();
            }
            break;

        case EF_PHASER: // endFreq
            if (bData_ >= phaser.aStartFreq) {
                subEffect_.bData = bData_;
                phaser.bEndFreq = bData_;
                phaser.endFreq = kPhaserFreqDataLibrary[bData_].data;
                phaser.update();
            }
            break;

        case EF_COMPRESSOR: // rate
            subEffect_.bData = bData_;
            compressor.bRate = bData_;
            compressor.rate = kCompressorRateDataLibrary[bData_].data;
            compressor.update();
            break;

        case EF_EXPANDER: // rate
            subEffect_.bData = bData_;
            expander.bRate = bData_;
            expander.rate = kExpanderRateDataLibrary[bData_].data;
            expander.update();
            break;

        case EF_OVERDRIVE: // threshold
            subEffect_.bData = bData_;
            overdrive.bThreshold = bData_;
            overdrive.thresholddB = kOverdriveThresholdDataLibrary[bData_].data;
            overdrive.update();
            break;

        case EF_DISTORTION: // threshold
            subEffect_.bData = bData_;
            distortion.bThreshold = bData_;
            distortion.thresholddB = kDistortionThresholdDataLibrary[bData_].data;
            distortion.update();
            break;

        case EF_BITCRUSHER: // sampleRate
            subEffect_.bData = bData_;
            bitcrusher.bSampleRate = bData_;
            bitcrusher.sampleRate = kBitcrusherSampleRateDataLibrary[bData_].data;
            bitcrusher.update();
            break;
        }
        // update lcd
        if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.type == subEffectNum_)) {
            lcd_drawEffect_BData(effectNum_);
        }
    }
}

void Controller::effect_setCData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t cData_) {
    Effect &effect_ = effect[effectNum_];
    SubEffect &subEffect_ = effect[effectNum_].subEffect[subEffectNum_];

    Delay &delay = effect_.delay;
    Chorus &chorus = effect_.chorus;
    Flanger &flanger = effect_.flanger;
    Phaser &phaser = effect_.phaser;
    Compressor &compressor = effect_.compressor;
    Expander &expander = effect_.expander;
    Overdrive &overdrive = effect_.overdrive;
    Distortion &distortion = effect_.distortion;
    Bitcrusher &bitcrusher = effect_.bitcrusher;

    if ((cData_ >= subEffect_.kMinCData) && (cData_ <= subEffect_.kMaxCData)) {
        // update data
        switch (subEffectNum_) {
        case EF_DELAY: // feedback
            subEffect_.cData = cData_;
            delay.cFeedback = cData_;
            delay.feedback = kDelayFeedbackDataLibrary[cData_].data;
            break;

        case EF_CHORUS: // rate
            subEffect_.cData = cData_;
            chorus.cRate = cData_;
            chorus.rate = kChorusRateDataLibrary[cData_].data;
            chorus.update();
            break;

        case EF_FLANGER: // rate
            subEffect_.cData = cData_;
            flanger.cRate = cData_;
            flanger.rate = kFlangerRateDataLibrary[cData_].data;
            flanger.update();
            break;

        case EF_PHASER: // rate
            subEffect_.cData = cData_;
            phaser.cRate = cData_;
            phaser.rate = kPhaserRateDataLibrary[cData_].data;
            phaser.update();
            break;

        case EF_COMPRESSOR: // time
            subEffect_.cData = cData_;
            compressor.cAttackTime = cData_;
            compressor.attackTime = kCompressorAttackTimeDataLibrary[cData_].data;
            compressor.update();
            break;

        case EF_EXPANDER: // time
            subEffect_.cData = cData_;
            expander.cAttackTime = cData_;
            expander.attackTime = kExpanderAttackTimeDataLibrary[cData_].data;
            expander.update();
            break;

        case EF_OVERDRIVE: // tone
            subEffect_.cData = cData_;
            overdrive.cTone = cData_;
            overdrive.tone = kOverdriveToneDataLibrary[cData_].data;
            overdrive.update();
            break;

        case EF_DISTORTION: // tone
            subEffect_.cData = cData_;
            distortion.cTone = cData_;
            distortion.tone = kDistortionToneDataLibrary[cData_].data;
            distortion.update();
            break;

        case EF_BITCRUSHER: // clipThreshold
            subEffect_.cData = cData_;
            bitcrusher.cThreshold = cData_;
            bitcrusher.threshold = kBitcrusherThresholdDataLibrary[cData_].data;
            bitcrusher.update();
            break;
        }
        // update lcd
        if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.type == subEffectNum_)) {
            lcd_drawEffect_CData(effectNum_);
        }
    }
}

void Controller::effect_setDData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t dData_) {
    Effect &effect_ = effect[effectNum_];
    SubEffect &subEffect_ = effect[effectNum_].subEffect[subEffectNum_];

    Delay &delay = effect_.delay;
    Chorus &chorus = effect_.chorus;
    Flanger &flanger = effect_.flanger;
    Phaser &phaser = effect_.phaser;
    Compressor &compressor = effect_.compressor;
    Expander &expander = effect_.expander;
    Overdrive &overdrive = effect_.overdrive;
    Distortion &distortion = effect_.distortion;
    Bitcrusher &bitcrusher = effect_.bitcrusher;

    if ((dData_ >= subEffect_.kMinDData) && (dData_ <= subEffect_.kMaxDData) && (!effect_.mixTransition.active)) {
        // update data
        uint8_t targetDry;
        uint8_t targetWet;

        float targetDryFloat;
        float targetWetFloat;

        switch (subEffectNum_) {
        case EF_DELAY:
            targetDry = dData_;
            targetWet = delay.eWet;

            if ((delay.limitMix) && ((targetDry + targetWet) > delay.limitMixData)) {
                targetWet = delay.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            delay.dDry = targetDry;
            delay.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[delay.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[delay.eWet].data;
            break;

        case EF_CHORUS:
            targetDry = dData_;
            targetWet = chorus.eWet;

            if ((chorus.limitMix) && ((targetDry + targetWet) > chorus.limitMixData)) {
                targetWet = chorus.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            chorus.dDry = targetDry;
            chorus.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[chorus.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[chorus.eWet].data;
            break;

        case EF_FLANGER:
            targetDry = dData_;
            targetWet = flanger.eWet;

            if ((flanger.limitMix) && ((targetDry + targetWet) > flanger.limitMixData)) {
                targetWet = flanger.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            flanger.dDry = targetDry;
            flanger.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[flanger.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[flanger.eWet].data;
            break;

        case EF_PHASER:
            targetDry = dData_;
            targetWet = phaser.eWet;

            if ((phaser.limitMix) && ((targetDry + targetWet) > phaser.limitMixData)) {
                targetWet = phaser.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            phaser.dDry = targetDry;
            phaser.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[phaser.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[phaser.eWet].data;
            break;

        case EF_COMPRESSOR:
            subEffect_.dData = dData_;
            compressor.dReleaseTime = dData_;
            compressor.releaseTime = kCompressorReleaseTimeDataLibrary[compressor.dReleaseTime].data;
            compressor.update();
            break;

        case EF_EXPANDER:
            subEffect_.dData = dData_;
            expander.dReleaseTime = dData_;
            expander.releaseTime = kExpanderReleaseTimeDataLibrary[expander.dReleaseTime].data;
            expander.update();
            break;

        case EF_OVERDRIVE:
            targetDry = dData_;
            targetWet = overdrive.eWet;

            if ((overdrive.limitMix) && ((targetDry + targetWet) > overdrive.limitMixData)) {
                targetWet = overdrive.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            overdrive.dDry = targetDry;
            overdrive.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[overdrive.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[overdrive.eWet].data;
            break;

        case EF_DISTORTION:
            targetDry = dData_;
            targetWet = distortion.eWet;

            if ((distortion.limitMix) && ((targetDry + targetWet) > distortion.limitMixData)) {
                targetWet = distortion.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            distortion.dDry = targetDry;
            distortion.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[distortion.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[distortion.eWet].data;
            break;

        case EF_BITCRUSHER:
            targetDry = dData_;
            targetWet = bitcrusher.eWet;

            if ((bitcrusher.limitMix) && ((targetDry + targetWet) > bitcrusher.limitMixData)) {
                targetWet = bitcrusher.limitMixData - targetDry;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            bitcrusher.dDry = targetDry;
            bitcrusher.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[bitcrusher.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[bitcrusher.eWet].data;
            break;
        }

        if (effect_.active) {
            effect_mixTransition(effectNum_, targetDryFloat, targetWetFloat);
        } else {
            effect_.dryFloat = targetDryFloat;
            effect_.wetFloat = targetWetFloat;
        }
        // update lcd
        if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.type == subEffectNum_)) {
            lcd_drawEffect_DData(effectNum_);
            lcd_drawEffect_EData(effectNum_);
        }
    }
}

void Controller::effect_setEData(uint8_t effectNum_, uint8_t subEffectNum_, uint8_t eData_) {
    Effect &effect_ = effect[effectNum_];
    SubEffect &subEffect_ = effect[effectNum_].subEffect[subEffectNum_];

    Delay &delay = effect_.delay;
    Chorus &chorus = effect_.chorus;
    Flanger &flanger = effect_.flanger;
    Phaser &phaser = effect_.phaser;
    Compressor &compressor = effect_.compressor;
    Expander &expander = effect_.expander;
    Overdrive &overdrive = effect_.overdrive;
    Distortion &distortion = effect_.distortion;
    Bitcrusher &bitcrusher = effect_.bitcrusher;

    if ((eData_ >= subEffect_.kMinEData) && (eData_ <= subEffect_.kMaxEData)) {
        // update data
        uint8_t targetDry;
        uint8_t targetWet;

        float targetDryFloat;
        float targetWetFloat;

        switch (subEffectNum_) {
        case EF_DELAY:
            targetDry = delay.dDry;
            targetWet = eData_;

            if ((delay.limitMix) && ((targetDry + targetWet) > delay.limitMixData)) {
                targetDry = delay.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            delay.dDry = targetDry;
            delay.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[delay.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[delay.eWet].data;
            break;

        case EF_CHORUS:
            targetDry = chorus.dDry;
            targetWet = eData_;

            if ((chorus.limitMix) && ((targetDry + targetWet) > chorus.limitMixData)) {
                targetDry = chorus.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            chorus.dDry = targetDry;
            chorus.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[chorus.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[chorus.eWet].data;
            break;

        case EF_FLANGER:
            targetDry = flanger.dDry;
            targetWet = eData_;

            if ((flanger.limitMix) && ((targetDry + targetWet) > flanger.limitMixData)) {
                targetDry = flanger.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            flanger.dDry = targetDry;
            flanger.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[flanger.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[flanger.eWet].data;
            break;

        case EF_PHASER:
            targetDry = phaser.dDry;
            targetWet = eData_;

            if ((phaser.limitMix) && ((targetDry + targetWet) > phaser.limitMixData)) {
                targetDry = phaser.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            phaser.dDry = targetDry;
            phaser.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[phaser.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[phaser.eWet].data;
            break;

        case EF_COMPRESSOR:
            subEffect_.eData = eData_;
            compressor.eMix = eData_;

            targetDryFloat = kEffectMixDataLibrary[20 - compressor.eMix].data;
            targetWetFloat = kEffectMixDataLibrary[compressor.eMix].data;
            break;

        case EF_EXPANDER:
            subEffect_.eData = eData_;
            expander.eMix = eData_;

            targetDryFloat = kEffectMixDataLibrary[20 - expander.eMix].data;
            targetWetFloat = kEffectMixDataLibrary[expander.eMix].data;
            break;

        case EF_OVERDRIVE:
            targetDry = overdrive.dDry;
            targetWet = eData_;

            if ((overdrive.limitMix) && ((targetDry + targetWet) > overdrive.limitMixData)) {
                targetDry = overdrive.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            overdrive.dDry = targetDry;
            overdrive.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[overdrive.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[overdrive.eWet].data;
            break;

        case EF_DISTORTION:
            targetDry = distortion.dDry;
            targetWet = eData_;

            if ((distortion.limitMix) && ((targetDry + targetWet) > distortion.limitMixData)) {
                targetDry = distortion.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            distortion.dDry = targetDry;
            distortion.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[distortion.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[distortion.eWet].data;
            break;

        case EF_BITCRUSHER:
            targetDry = bitcrusher.dDry;
            targetWet = eData_;

            if ((bitcrusher.limitMix) && ((targetDry + targetWet) > bitcrusher.limitMixData)) {
                targetDry = bitcrusher.limitMixData - targetWet;
            }

            subEffect_.dData = targetDry;
            subEffect_.eData = targetWet;

            bitcrusher.dDry = targetDry;
            bitcrusher.eWet = targetWet;

            targetDryFloat = kEffectMixDataLibrary[bitcrusher.dDry].data;
            targetWetFloat = kEffectMixDataLibrary[bitcrusher.eWet].data;
            break;
        }

        if (effect_.active) {
            effect_mixTransition(effectNum_, targetDryFloat, targetWetFloat);
        } else {
            effect_.dryFloat = targetDryFloat;
            effect_.wetFloat = targetWetFloat;
        }

        // update lcd
        if ((((menu == EFFECT_0_MENU) && (effectNum_ == 0)) || ((menu == EFFECT_1_MENU) && (effectNum_ == 1))) && (effect_.type == subEffectNum_)) {
            lcd_drawEffect_DData(effectNum_);
            lcd_drawEffect_EData(effectNum_);
        }
    }
}

void Controller::effect_genTransition(uint8_t effectNum_, EffectTransitionMode mode_, bool activeActive_, bool targetActive_, uint8_t activeType_, uint8_t targetType_) {
    Effect &effect_ = effect[effectNum_];
    EffectGenTransition &gTransition = effect_.genTransition;

    gTransition.mode = mode_;
    gTransition.phase = EF_PHASE_A;

    gTransition.activeActive = activeActive_;
    gTransition.targetActive = targetActive_;

    gTransition.activeType = activeType_;
    gTransition.targetType = targetType_;

    switch (mode_) {
    case EF_MODE_NONE:
        break;

    case EF_MODE_ACTIVE:
        switch (targetActive_) {
        case true:
            gTransition.activeDry = 1.0;
            gTransition.targetDry = 0.0;

            gTransition.activeWet = 0.0;
            gTransition.targetWet = 1.0;

            if ((activeType_ == EF_DELAY) || (activeType_ == EF_CHORUS) || (activeType_ == EF_FLANGER)) {
                gTransition.activeRecordWet = 0.0;
                gTransition.targetRecordWet = 1.0;
            }
            transitionShowFlag = 2;
            break;

        case false:
            gTransition.activeDry = 0.0;
            gTransition.targetDry = 1.0;

            gTransition.activeWet = 1.0;
            gTransition.targetWet = 0.0;

            if ((activeType_ == EF_DELAY) || (activeType_ == EF_CHORUS) || (activeType_ == EF_FLANGER)) {
                gTransition.activeRecordWet = 1.0;
                gTransition.targetRecordWet = 0.0;
            }
            transitionShowFlag = 1;
            break;
        }
        break;

    case EF_MODE_TYPE:
    case EF_MODE_TIME:
    case EF_MODE_FEEDBACK:
        gTransition.activeDry = 0.0;
        gTransition.targetDry = 1.0;

        gTransition.activeWet = 1.0;
        gTransition.targetWet = 0.0;

        if ((activeType_ == EF_DELAY) || (activeType_ == EF_CHORUS) || (activeType_ == EF_FLANGER)) {
            gTransition.activeRecordWet = 1.0;
            gTransition.targetRecordWet = 0.0;
        }

        transitionShowFlag = 1;
        break;
    }

    effect_calculateGenTransition(effectNum_);
    gTransition.active = true;
}

void Controller::effect_mixTransition(uint8_t effectNum_, float dryFloat_, float wetFloat_) {
    Effect &effect_ = effect[effectNum_];
    EffectMixTransition &mTransition = effect_.mixTransition;

    mTransition.targetDry = dryFloat_;
    mTransition.targetWet = wetFloat_;

    effect_calculateMixTransition(effectNum_);
    mTransition.active = true;
}

void Controller::effect_calculateGenTransition(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    EffectGenTransition &gTransition = effect_.genTransition;

    if (gTransition.activeDry == gTransition.targetDry) {
        gTransition.actionDry = EF_ACTION_NONE;
    } else if (gTransition.activeDry < gTransition.targetDry) {
        gTransition.actionDry = EF_ACTION_UP;
    } else if (gTransition.activeDry > gTransition.targetDry) {
        gTransition.actionDry = EF_ACTION_DOWN;
    }

    if (gTransition.activeWet == gTransition.targetWet) {
        gTransition.actionWet = EF_ACTION_NONE;
    } else if (gTransition.activeWet < gTransition.targetWet) {
        gTransition.actionWet = EF_ACTION_UP;
    } else if (gTransition.activeWet > gTransition.targetWet) {
        gTransition.actionWet = EF_ACTION_DOWN;
    }

    if (gTransition.activeRecordWet == gTransition.targetRecordWet) {
        gTransition.actionRecordWet = EF_ACTION_NONE;
    } else if (gTransition.activeRecordWet < gTransition.targetRecordWet) {
        gTransition.actionRecordWet = EF_ACTION_UP;
    } else if (gTransition.activeRecordWet > gTransition.targetRecordWet) {
        gTransition.actionRecordWet = EF_ACTION_DOWN;
    }
}

void Controller::effect_calculateMixTransition(uint8_t effectNum_) {
    Effect &effect_ = effect[effectNum_];
    EffectMixTransition &mTransition = effect_.mixTransition;

    if (effect_.dryFloat == mTransition.targetDry) {
        mTransition.actionDry = EF_ACTION_NONE;
    } else if (effect_.dryFloat < mTransition.targetDry) {
        mTransition.actionDry = EF_ACTION_UP;
    } else if (effect_.dryFloat > mTransition.targetDry) {
        mTransition.actionDry = EF_ACTION_DOWN;
    }

    if (effect_.wetFloat == mTransition.targetWet) {
        mTransition.actionWet = EF_ACTION_NONE;
    } else if (effect_.wetFloat < mTransition.targetWet) {
        mTransition.actionWet = EF_ACTION_UP;
    } else if (effect_.wetFloat > mTransition.targetWet) {
        mTransition.actionWet = EF_ACTION_DOWN;
    }
}

void Controller::effect_cleanMemory(uint8_t effectNum_, uint8_t type_) {
    Effect &effect_ = effect[effectNum_];

    switch (type_) {
    case EF_DELAY:
        effect_.delay.cleanMemory();
        memset((uint8_t *)effect_.delayAddress, 0x00, kDelayByteSize);
        break;

    case EF_CHORUS:
        effect_.chorus.cleanMemory();
        memset((uint8_t *)effect_.chorusAddress, 0x00, kChorusByteSize);
        break;

    case EF_FLANGER:
        effect_.flanger.cleanMemory();
        memset(effect_.flangerBuffer, 0x00, sizeof(effect_.flangerBuffer));
        break;

    case EF_PHASER:
        effect_.phaser.cleanMemory();
        memset(effect_.phaser.ff, 0x00, sizeof(effect_.phaser.ff));
        memset(effect_.phaser.fb, 0x00, sizeof(effect_.phaser.fb));
        break;

    case EF_COMPRESSOR:
        effect_.compressor.cleanMemory();
        break;

    case EF_EXPANDER:
        effect_.expander.cleanMemory();
        break;

    case EF_OVERDRIVE:
        effect_.overdrive.cleanMemory();
        break;

    case EF_DISTORTION:
        effect_.distortion.cleanMemory();
        break;

    case EF_BITCRUSHER:
        effect_.bitcrusher.cleanMemory();
        break;
    }
}

/* Reverb functions ----------------------------------------------------------*/

void Controller::reverb_select() {
    if (menu != REVERB_MENU) {
        preMenu = menu;
        menu = REVERB_MENU;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        preMenuLayerClear();
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawReverbMenu();
    }
}

void Controller::reverb_reset() {
    reverb_setActive(kInitialReverbActive);
    reverb_setSize(kInitialReverbSize);
    reverb_setDecay(kInitialReverbDecay);
    reverb_setPreDelay(kInitialReverbPreDelay);
    reverb_setSurround(kInitialReverbSurround);
    reverb_setDry(kInitialReverbDry);
    reverb_setWet(kInitialReverbWet);
}

void Controller::reverb_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 0) ? menuTab += 2 : menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::reverb_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        preMenuTab = menuTab;
        (menuTab == 2) ? menuTab -= 2 : menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::reverb_menuUp() {
    switch (menuTab) {
    case 0:
        reverb_setActive(!reverb.active);
        break;

    case 2:
        if (reverb.size < kMaxReverbSize) {
            reverb_setSize(reverb.size + 1);
        }
        break;

    case 3:
        if (reverb.decay < kMaxReverbDecay) {
            reverb_setDecay(reverb.decay + 1);
        }
        break;

    case 4:
        if (reverb.preDelay < kMaxReverbPreDelay) {
            reverb_setPreDelay(reverb.preDelay + 1);
        }
        break;

    case 5:
        if (reverb.surround < kMaxReverbSurround) {
            reverb_setSurround(reverb.surround + 1);
        }
        break;

    case 6:
        if (reverb.dry < kMaxReverbDry) {
            reverb_setDry(reverb.dry + 1);
        } else if ((reverb.dry == kMaxReverbDry) && (reverb.wet > kMinReverbWet)) {
            reverb_setWet(reverb.wet - 1);
        }
        break;

    case 7:
        if (reverb.wet < kMaxReverbWet) {
            reverb_setWet(reverb.wet + 1);
        } else if ((reverb.wet == kMaxReverbWet) && (reverb.dry > kMinReverbDry)) {
            reverb_setDry(reverb.dry - 1);
        }
        break;
    }
}

void Controller::reverb_menuDown() {
    switch (menuTab) {
    case 0:
        reverb_setActive(!reverb.active);
        break;

    case 2:
        if (reverb.size > kMinReverbSize) {
            reverb_setSize(reverb.size - 1);
        }
        break;

    case 3:
        if (reverb.decay > kMinReverbDecay) {
            reverb_setDecay(reverb.decay - 1);
        }
        break;

    case 4:
        if (reverb.preDelay > kMinReverbPreDelay) {
            reverb_setPreDelay(reverb.preDelay - 1);
        }
        break;

    case 5:
        if (reverb.surround > kMinReverbSurround) {
            reverb_setSurround(reverb.surround - 1);
        }
        break;

    case 6:
        if (reverb.dry > kMinReverbDry) {
            reverb_setDry(reverb.dry - 1);
        }
        break;

    case 7:
        if (reverb.wet > kMinReverbWet) {
            reverb_setWet(reverb.wet - 1);
        }
        break;
    }
}

void Controller::reverb_setActive(bool active_) {
    if ((menu == REVERB_MENU) && (!reverb.genTransition.active)) {
        // update data
        // if (active_) reverb.cleanMemory();
        reverb_genTransition(REV_MODE_ACTIVE, reverb.active, active_);
        reverb.active = active_;
        // update lcd
        lcd_drawReverb_ActiveData();
    } else if (!reverb.genTransition.active) {
        // update data
        reverb.active = active_;
    }
}

void Controller::reverb_setSize(uint8_t size_) {
    if ((size_ >= kMinReverbSize) && (size_ <= kMaxReverbSize)) {
        // update data
        reverb.size = size_;
        reverb.setSize(kReverbSizeDataLibrary[size_].data);
        // update lcd
        if (menu == REVERB_MENU)
            lcd_drawReverb_SizeData();
    }
}

void Controller::reverb_setDecay(uint8_t decay_) {
    if ((decay_ >= kMinReverbDecay) && (decay_ <= kMaxReverbDecay)) {
        // update data
        reverb.decay = decay_;
        reverb.setDecay(kReverbDecayDataLibrary[decay_].data);
        // update lcd
        if (menu == REVERB_MENU)
            lcd_drawReverb_DecayData();
    }
}

void Controller::reverb_setPreDelay(uint8_t preDelay_) {
    if ((preDelay_ >= kMinReverbPreDelay) && (preDelay_ <= kMaxReverbPreDelay)) {
        if ((menu == REVERB_MENU) && (!reverb.genTransition.active)) {
            // update data
            if (reverb.active) {
                reverb.preDelay = preDelay_;
                reverb_genTransition(REV_MODE_PREDELAY, reverb.active, reverb.active);
            } else {
                reverb.preDelay = preDelay_;
                reverb.setPreDelay(kReverbPreDelayDataLibrary[preDelay_].data);
            }
            // update lcd
            lcd_drawReverb_PreDelayData();
        } else if (!reverb.genTransition.active) {
            // update data
            reverb.preDelay = preDelay_;
            reverb.setPreDelay(kReverbPreDelayDataLibrary[preDelay_].data);
        }
    }
}

void Controller::reverb_setSurround(uint8_t surround_) {
    if ((surround_ >= kMinReverbSurround) && (surround_ <= kMaxReverbSurround)) {
        if ((menu == REVERB_MENU) && (!reverb.genTransition.active)) {
            // update data
            if (reverb.active) {
                reverb.surround = surround_;
                reverb_genTransition(REV_MODE_SURROUND, reverb.active, reverb.active);
            } else {
                reverb.surround = surround_;
                reverb.setSurround(kReverbSurroundDataLibrary[surround_].data);
            }
            // update lcd
            lcd_drawReverb_SurroundData();
        } else if (!reverb.genTransition.active) {
            // update data
            reverb.surround = surround_;
            reverb.setSurround(kReverbSurroundDataLibrary[surround_].data);
        }
    }
}

void Controller::reverb_setDry(uint8_t dry_) {
    if ((dry_ >= kMinReverbDry) && (dry_ <= kMaxReverbDry) && (!reverb.mixTransition.active)) {
        // update data
        uint8_t targetDry = dry_;
        uint8_t targetWet = reverb.wet;

        if ((reverb.limitMix) && ((targetDry + targetWet) > reverb.limitMixData)) {
            targetWet = reverb.limitMixData - targetDry;
        }

        reverb.dry = targetDry;
        reverb.wet = targetWet;

        float targetDryFloat = kReverbMixDataLibrary[reverb.dry].data;
        float targetWetFloat = kReverbMixDataLibrary[reverb.wet].data;

        if (reverb.active) {
            reverb_mixTransition(targetDryFloat, targetWetFloat);
        } else {
            reverb.dryFloat = targetDryFloat;
            reverb.wetFloat = targetWetFloat;
        }

        // update lcd
        if (menu == REVERB_MENU) {
            lcd_drawReverb_DryData();
            lcd_drawReverb_WetData();
        }
    }
}

void Controller::reverb_setWet(uint8_t wet_) {
    if ((wet_ >= kMinReverbWet) && (wet_ <= kMaxReverbWet) && (!reverb.mixTransition.active)) {
        // update data
        uint8_t targetDry = reverb.dry;
        uint8_t targetWet = wet_;

        if ((reverb.limitMix) && ((targetDry + targetWet) > reverb.limitMixData)) {
            targetDry = reverb.limitMixData - targetWet;
        }

        reverb.dry = targetDry;
        reverb.wet = targetWet;

        float targetDryFloat = kReverbMixDataLibrary[reverb.dry].data;
        float targetWetFloat = kReverbMixDataLibrary[reverb.wet].data;

        if (reverb.active) {
            reverb_mixTransition(targetDryFloat, targetWetFloat);
        } else {
            reverb.dryFloat = targetDryFloat;
            reverb.wetFloat = targetWetFloat;
        }

        // update lcd
        if (menu == REVERB_MENU) {
            lcd_drawReverb_DryData();
            lcd_drawReverb_WetData();
        }
    }
}

void Controller::reverb_genTransition(ReverbTransitionMode mode_, bool activeActive_, bool targetActive_) {
    ReverbGenTransition &gTransition = reverb.genTransition;

    gTransition.mode = mode_;
    gTransition.phase = REV_PHASE_A;

    gTransition.activeActive = activeActive_;
    gTransition.targetActive = targetActive_;

    switch (mode_) {
    case REV_MODE_NONE:
        break;

    case REV_MODE_ACTIVE:
        switch (targetActive_) {
        case true:
            gTransition.activeDry = 1.0;
            gTransition.targetDry = 0.0;

            gTransition.activeWet = 0.0;
            gTransition.targetWet = 1.0;

            transitionShowFlag = 2;
            break;

        case false:
            gTransition.activeDry = 0.0;
            gTransition.targetDry = 1.0;

            gTransition.activeWet = 1.0;
            gTransition.targetWet = 0.0;

            transitionShowFlag = 1;
            break;
        }
        break;

    case REV_MODE_PREDELAY:
    case REV_MODE_SURROUND:
        gTransition.activeDry = 0.0;
        gTransition.targetDry = 1.0;

        gTransition.activeWet = 1.0;
        gTransition.targetWet = 0.0;

        transitionShowFlag = 1;
        break;
    }

    reverb_calculateGenTransition();
    gTransition.active = true;
}

void Controller::reverb_mixTransition(float dryFloat_, float wetFloat_) {
    ReverbMixTransition &mTransition = reverb.mixTransition;

    mTransition.targetDry = dryFloat_;
    mTransition.targetWet = wetFloat_;

    reverb_calculateMixTransition();
    mTransition.active = true;
}

void Controller::reverb_calculateGenTransition() {
    ReverbGenTransition &gTransition = reverb.genTransition;

    if (gTransition.activeDry == gTransition.targetDry) {
        gTransition.actionDry = REV_ACTION_NONE;
    } else if (gTransition.activeDry < gTransition.targetDry) {
        gTransition.actionDry = REV_ACTION_UP;
    } else if (gTransition.activeDry > gTransition.targetDry) {
        gTransition.actionDry = REV_ACTION_DOWN;
    }

    if (gTransition.activeWet == gTransition.targetWet) {
        gTransition.actionWet = REV_ACTION_NONE;
    } else if (gTransition.activeWet < gTransition.targetWet) {
        gTransition.actionWet = REV_ACTION_UP;
    } else if (gTransition.activeWet > gTransition.targetWet) {
        gTransition.actionWet = REV_ACTION_DOWN;
    }
}

void Controller::reverb_calculateMixTransition() {
    ReverbMixTransition &mTransition = reverb.mixTransition;

    if (reverb.dryFloat == mTransition.targetDry) {
        mTransition.actionDry = REV_ACTION_NONE;
    } else if (reverb.dryFloat < mTransition.targetDry) {
        mTransition.actionDry = REV_ACTION_UP;
    } else if (reverb.dryFloat > mTransition.targetDry) {
        mTransition.actionDry = REV_ACTION_DOWN;
    }

    if (reverb.wetFloat == mTransition.targetWet) {
        mTransition.actionWet = REV_ACTION_NONE;
    } else if (reverb.wetFloat < mTransition.targetWet) {
        mTransition.actionWet = REV_ACTION_UP;
    } else if (reverb.wetFloat > mTransition.targetWet) {
        mTransition.actionWet = REV_ACTION_DOWN;
    }
}

/* Layer functions -----------------------------------------------------------*/

void Controller::layer_setTab(LayerTab layerTab_) {
    if (layerTab != layerTab_) {
        layerTab = layerTab_;
        lcd_drawLayerTab();
        if (selectedLayerNum != -1) {
            if (menu == LAYER_INST_MENU) {
                switch (layerTab) {
                case TAB_0:
                    layerInst_select(selectedLayerNum - 5);
                    break;

                case TAB_1:
                    layerInst_select(selectedLayerNum + 5);
                    break;
                }
            } else if (menu == LAYER_SONG_MENU) {
                switch (layerTab) {
                case TAB_0:
                    layerSong_select(selectedLayerNum - 5, activeBankNum);
                    break;

                case TAB_1:
                    layerSong_select(selectedLayerNum + 5, activeBankNum);
                    break;
                }
            }
        }
    }
}

void Controller::layer_playBeat(uint8_t layerNum_) {
    Layer &layer = layerLibrary[layerNum_];
    if ((!layer.mute) && (layer.samplePlay)) {
        LayerPlayData &lD = sD.layerData[layer.number];
        lD.beatData.active = true;
        lD.beatData.counterMax = layer.sampleSector[layer.playSampleSector].size;
        lD.beatData.ramAddress = kRamLayerAddressLibrary[layer.number][layer.playSampleSector];
        lD.beatData.volumeMultiplier = kLayerVolumeCoef * kFloatDataLibrary[layer.volume / 5].pow2Multiplier;
        lD.beatData.normalize = layer.normalize;
        if (lD.beatData.normalize)
            lD.beatData.volumeMultiplier *= layer.sampleSector[layer.playSampleSector].normMultiplier;
        if (layer.style) {
            lD.beatData.speed = layer.speed;
            lD.beatData.reverse = layer.reverse;
        } else {
            lD.beatData.speed = kInitialLayerSpeed;
            lD.beatData.reverse = kInitialLayerReverse;
        }

        lD.beatData.increment = kLayerSpeedDataLibrary[lD.beatData.speed].increment;
        (lD.beatData.reverse) ? lD.beatData.counter = lD.beatData.counterMax - 1 : lD.beatData.counter = 0;
    }

    if (recordActive)
        layerSong_setBeat(layerNum_, activeBankNum, playInterval, 0);
}

/* Layer Inst functions ------------------------------------------------------*/

void Controller::layerInst_select(uint8_t layerNum_) {
    if ((menu != LAYER_INST_MENU) || ((menu == LAYER_INST_MENU) && (selectedLayerNum != layerNum_))) {
        preMenu = menu;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        menu = LAYER_INST_MENU;

        Layer &layer = layerLibrary[layerNum_];

        layer.instSelected = layer.instLoaded;
        layer.sampleSelected = layer.sampleLoaded;

        layer.instSelectedData = layer.instLoadedData;
        layer.sampleSelectedData = layer.sampleLoadedData;
        layer.wavSelectedData = layer.wavLoadedData;

        preMenuLayerClear();
        selectedLayerNum = layerNum_;
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawLayerInstMenu();
        lcd_drawLayerSelect();
    }
}

void Controller::layerInst_reset(uint8_t layerNum_) {
    layerInst_setInstSelected(layerNum_, -1);
    layerInst_setInstLoaded(layerNum_);
    layerInst_setSampleSelected(layerNum_, -1);
    layerInst_setSampleLoaded(layerNum_);
    layerInst_setVolume(layerNum_, kInitialLayerVolume);
    layerInst_setSpeed(layerNum_, kInitialLayerSpeed);
    layerInst_setReverse(layerNum_, kInitialLayerReverse);
    layerInst_setNormalize(layerNum_, kInitialLayerNormalize);
    layerInst_setMute(layerNum_, false);
    layerInst_setFill(layerNum_, true);
    layerInst_setStyle(layerNum_, true);
}

void Controller::layerInst_menuRight() {
    if (menuTab < kMaxMenu8Tab) {
        preMenuTab = menuTab;
        ((menuTab == 0) || (menuTab == 2)) ? menuTab += 2 : menuTab += 1;
        lcd_transitionSelect();
    }
}

void Controller::layerInst_menuLeft() {
    if (menuTab > kMinMenu8Tab) {
        preMenuTab = menuTab;
        ((menuTab == 2) || (menuTab == 4)) ? menuTab -= 2 : menuTab -= 1;
        lcd_transitionSelect();
    }
}

void Controller::layerInst_menuUp() {
    Layer &layer = layerLibrary[selectedLayerNum];
    switch (menuTab) {
    case 0:
        if (instLibrarySize > 0) {
            (layer.instSelected < (instLibrarySize - 1)) ? layerInst_setInstSelected(selectedLayerNum, layer.instSelected + 1) : layerInst_setInstSelected(selectedLayerNum, -1);
        }
        break;

    case 2:
        if ((layer.instSelected != -1) && (sampleLibrarySize[layer.instSelected] > 0)) {
            (layer.sampleSelected < (sampleLibrarySize[layer.instSelected] - 1)) ? layerInst_setSampleSelected(selectedLayerNum, layer.sampleSelected + 1) : layerInst_setSampleSelected(selectedLayerNum, 0);
        }
        break;

    case 4:
        if (layer.volume < kMaxLayerVolume) {
            layerInst_setVolume(selectedLayerNum, layer.volume + 5);
        }
        break;

    case 5:
        if (layer.speed < kMaxLayerSpeed) {
            layerInst_setSpeed(selectedLayerNum, layer.speed + 1);
        }
        break;

    case 6:
        layerInst_setReverse(selectedLayerNum, !layer.reverse);
        break;

    case 7:
        layerInst_setNormalize(selectedLayerNum, !layer.normalize);
        break;
    }
}

void Controller::layerInst_menuDown() {
    Layer &layer = layerLibrary[selectedLayerNum];

    switch (menuTab) {
    case 0:
        if (instLibrarySize > 0) {
            (layer.instSelected > -1) ? layerInst_setInstSelected(selectedLayerNum, layer.instSelected - 1) : layerInst_setInstSelected(selectedLayerNum, instLibrarySize - 1);
        }
        break;

    case 2:
        if ((layer.instSelected != -1) && (sampleLibrarySize[layer.instSelected] > 0)) {
            (layer.sampleSelected > 0) ? layerInst_setSampleSelected(selectedLayerNum, layer.sampleSelected - 1) : layerInst_setSampleSelected(selectedLayerNum, sampleLibrarySize[layer.instSelected] - 1);
        }
        break;

    case 4:
        if (layer.volume > kMinLayerVolume) {
            layerInst_setVolume(selectedLayerNum, layer.volume - 5);
        }
        break;

    case 5:
        if (layer.speed > kMinLayerSpeed) {
            layerInst_setSpeed(selectedLayerNum, layer.speed - 1);
        }
        break;

    case 6:
        layerInst_setReverse(selectedLayerNum, !layer.reverse);
        break;

    case 7:
        layerInst_setNormalize(selectedLayerNum, !layer.normalize);
        break;
    }
}

void Controller::layerInst_setInstSelected(uint8_t layerNum_, int16_t inst_) {
    Layer &layer = layerLibrary[layerNum_];
    if ((inst_ >= -1) && (inst_ < instLibrarySize)) {
        // disable keyboard
        keyboard_disable();
        // update data
        layer.instSelected = inst_;
        // clear data
        memset(&(layer.instSelectedData), 0x00, sizeof(InstData));
        if (inst_ != -1) {
            // set num data
            sprintf(layer.instSelectedData.num, "%04d", layer.instSelected + 1);
            // read sd data
            char temp[kFileNameSize + 1];
            f_close(&sd.file);
            if ((f_open(&sd.file, "System/Data/Inst.lib", FA_READ) == FR_OK) && (f_lseek(&sd.file, 25 + (layer.instSelected * 32)) == FR_OK) && (f_read(&sd.file, temp, kFileNameSize, &sd.bytesread) == FR_OK)) {
                f_close(&sd.file);
                // write fileName
                strcpy(layer.instSelectedData.fileName, "System/Data/");
                strncat(layer.instSelectedData.fileName, temp, kFileNameSize);
                strncat(layer.instSelectedData.fileName, ".ins", 4);
                // write folderName
                strcpy(layer.instSelectedData.folderName, "Sample/");
                strncat(layer.instSelectedData.folderName, temp, kFileNameSize);
                strncat(layer.instSelectedData.folderName, "/", 1);
                // write nameLong
                strncpy(layer.instSelectedData.nameLong, temp, kFileNameSize);
                // write nameShort
                uint8_t length = strlen(temp);
                if (length > 10) {
                    strncpy(layer.instSelectedData.nameShortL, temp, 9);
                    strcat(layer.instSelectedData.nameShortL, "_");
                    strncpy(layer.instSelectedData.nameShortR, temp, 9);
                    strcat(layer.instSelectedData.nameShortR, "_");
                } else if (length == 10) {
                    strncpy(layer.instSelectedData.nameShortL, temp, length);
                    strncpy(layer.instSelectedData.nameShortR, temp, length);
                } else {
                    char bText[] = "          ";
                    uint8_t bLength = 10 - length;
                    strncpy(layer.instSelectedData.nameShortL, temp, length);
                    strncat(layer.instSelectedData.nameShortL, bText, bLength);
                    strncpy(layer.instSelectedData.nameShortR, bText, bLength);
                    strncat(layer.instSelectedData.nameShortR, temp, length);
                }
            }
        } else {
            // clear num data
            memset(layer.instSelectedData.num, '-', 4);
            // clear name data
            memcpy(layer.instSelectedData.nameShortL, kDataDashL, 11);
            memcpy(layer.instSelectedData.nameShortR, kDataDashR, 11);
        }
        // update sample
        ((inst_ != -1) && (sampleLibrarySize[inst_])) ? layerInst_setSampleSelected(layerNum_, 0) : layerInst_setSampleSelected(layerNum_, -1);
        f_close(&sd.file);
        // enable keyboard
        keyboard_enable();
        // update lcd
        if ((menu == LAYER_INST_MENU) && (layerNum_ == selectedLayerNum))
            lcd_drawLayerInst_InstData();
    }
}

void Controller::layerInst_setInstLoaded(uint8_t layerNum_) {
    Layer &layer = layerLibrary[layerNum_];
    if (strcmp(layer.instSelectedData.fileName, layer.instLoadedData.fileName) != 0) {
        // clear data
        memset(&(layer.instLoadedData), 0x00, sizeof(InstData));
        // copy data
        layer.instLoaded = layer.instSelected;
        layer.instLoadedData = layer.instSelectedData;
        // update lcd
        lcd_drawTab_InstData(layerNum_);
        if ((menu == LAYER_INST_MENU) && (layerNum_ == selectedLayerNum))
            lcd_drawLayerInst_InstData();
    }
}

void Controller::layerInst_setSampleSelected(uint8_t layerNum_, int16_t sample_) {
    Layer &layer = layerLibrary[layerNum_];
    if ((sample_ >= -1) && (sample_ < sampleLibrarySize[layerLibrary[layerNum_].instSelected])) {
        // disable keyboard
        keyboard_disable();
        // update data
        layer.sampleSelected = sample_;
        // clear data
        memset(&(layer.sampleSelectedData), 0x00, sizeof(SampleData));
        memset(&(layer.wavSelectedData), 0x00, sizeof(WavData));
        if (sample_ != -1) {
            // set num data
            sprintf(layer.sampleSelectedData.num, "%04d", layer.sampleSelected + 1);
            // read sd data
            char temp[kFileNameSize + 1];
            f_close(&sd.file);
            if ((f_open(&sd.file, layer.instSelectedData.fileName, FA_READ) == FR_OK) && (f_lseek(&sd.file, 25 + (layer.sampleSelected * kFileNameSize)) == FR_OK) && (f_read(&sd.file, temp, kFileNameSize, &sd.bytesread) == FR_OK)) {
                f_close(&sd.file);
                // write fileName
                strcpy(layer.sampleSelectedData.fileName, layer.instSelectedData.folderName);
                strncat(layer.sampleSelectedData.fileName, temp, kFileNameSize);
                // write nameLong
                strncpy(layer.sampleSelectedData.nameLong, temp, kFileNameSize);
                // write nameShort
                char *temp2 = strtok(temp, ".");
                uint8_t length = strlen(temp2);
                if (length > 10) {
                    strncpy(layer.sampleSelectedData.nameShortR, temp2, 9);
                    strcat(layer.sampleSelectedData.nameShortR, "_");
                } else if (length == 10) {
                    strncpy(layer.sampleSelectedData.nameShortR, temp2, length);
                } else {
                    char bText[] = "          ";
                    uint8_t bLength = 10 - length;
                    strncpy(layer.sampleSelectedData.nameShortR, bText, bLength);
                    strncat(layer.sampleSelectedData.nameShortR, temp2, length);
                }
                // update sdram
                bool sampleReadError = false;
                bool sampleTypeError = false;
                // read sd file
                if (f_open(&sd.file, layer.sampleSelectedData.fileName, FA_READ) == FR_OK) {
                    if (f_read(&sd.file, &layer.wavSelectedData.riff_chunk, 12, &sd.bytesread) == FR_OK) {
                        // read riff_chunk
                        if ((layer.wavSelectedData.riff_chunk.chunkId == 0x46464952) && (layer.wavSelectedData.riff_chunk.fileFormat == 0x45564157)) {
                            uint32_t chunkSize = layer.wavSelectedData.riff_chunk.chunkSize + 8;
                            // read fmt_chunk
                            for (uint32_t i = 12; i < (chunkSize - 24); i++) {
                                f_lseek(&sd.file, i);
                                f_read(&sd.file, &layer.wavSelectedData.fmt_chunk, 24, &sd.bytesread);
                                if (layer.wavSelectedData.fmt_chunk.chunkId == 0x20746D66) {
                                    layer.wavSelectedData.fmt_chunk.chunkStartByte = i;
                                    // check(layer.wavSelectedData.fmt_chunk.chunkStartByte, 0);
                                    break;
                                }
                            }
                            // check fmt_chunk
                            if ((layer.wavSelectedData.fmt_chunk.chunkId == 0x20746D66) &&
                                ((layer.wavSelectedData.fmt_chunk.chunkSize == 16) || (layer.wavSelectedData.fmt_chunk.chunkSize == 18) || (layer.wavSelectedData.fmt_chunk.chunkSize == 40)) &&
                                ((layer.wavSelectedData.fmt_chunk.audioFormat == 0x01) || (layer.wavSelectedData.fmt_chunk.audioFormat == 0x03)) &&
                                ((layer.wavSelectedData.fmt_chunk.nbrChannels == 0x01) || (layer.wavSelectedData.fmt_chunk.nbrChannels == 0x02)) &&
                                ((layer.wavSelectedData.fmt_chunk.sampleRate == 8000) || (layer.wavSelectedData.fmt_chunk.sampleRate == 16000) || (layer.wavSelectedData.fmt_chunk.sampleRate == 48000) || (layer.wavSelectedData.fmt_chunk.sampleRate == 11025) || (layer.wavSelectedData.fmt_chunk.sampleRate == 22050) || (layer.wavSelectedData.fmt_chunk.sampleRate == 44100) || (layer.wavSelectedData.fmt_chunk.sampleRate == 48000) || (layer.wavSelectedData.fmt_chunk.sampleRate == 88200) || (layer.wavSelectedData.fmt_chunk.sampleRate == 96000) || (layer.wavSelectedData.fmt_chunk.sampleRate == 176400) || (layer.wavSelectedData.fmt_chunk.sampleRate == 192000)) &&
                                ((layer.wavSelectedData.fmt_chunk.bitPerSample == 8) || (layer.wavSelectedData.fmt_chunk.bitPerSample == 16) || (layer.wavSelectedData.fmt_chunk.bitPerSample == 24) || (layer.wavSelectedData.fmt_chunk.bitPerSample == 32))) {
                                // read data_chunk
                                for (uint32_t j = 12; j < (chunkSize - 8); j++) {
                                    f_lseek(&sd.file, j);
                                    f_read(&sd.file, &layer.wavSelectedData.data_chunk, 8, &sd.bytesread);
                                    if (layer.wavSelectedData.data_chunk.chunkId == 0x61746164) {
                                        layer.wavSelectedData.data_chunk.chunkStartByte = j;
                                        // check(layer.wavData.data_chunk.chunkStartByte, 0);
                                        break;
                                    }
                                }
                                // check data_chunk
                                if (layer.wavSelectedData.data_chunk.chunkId == 0x61746164) {
                                    // analyze data
                                    layer.sampleSelectedData.coefSampleSize = 1.0f;
                                    // calculate sampleSize
                                    switch (layer.wavSelectedData.fmt_chunk.nbrChannels) {
                                    case 0x01:
                                        layer.sampleSelectedData.coefSampleSize *= 1;
                                        layer.sampleSelectedData.channel = CH_MONO;
                                        break;

                                    case 0x02:
                                        layer.sampleSelectedData.coefSampleSize *= 0.5;
                                        layer.sampleSelectedData.channel = CH_STEREO;
                                        break;
                                    }

                                    switch (layer.wavSelectedData.fmt_chunk.bitPerSample) {
                                    case 8:
                                        layer.sampleSelectedData.coefSampleSize *= 1;
                                        layer.sampleSelectedData.bitdepth = BD_08;
                                        break;

                                    case 16:
                                        layer.sampleSelectedData.coefSampleSize /= 2;
                                        layer.sampleSelectedData.bitdepth = BD_16;
                                        break;

                                    case 24:
                                        layer.sampleSelectedData.coefSampleSize /= 3;
                                        layer.sampleSelectedData.bitdepth = BD_24;
                                        break;

                                    case 32:
                                        layer.sampleSelectedData.coefSampleSize /= 4;
                                        layer.sampleSelectedData.bitdepth = BD_32;
                                        break;
                                    }

                                    switch (layer.wavSelectedData.fmt_chunk.sampleRate) {
                                    case 8000:
                                        layer.sampleSelectedData.coefSampleSize *= 6;
                                        layer.sampleSelectedData.frequency = FR_008kHz;
                                        break;

                                    case 11025:
                                        layer.sampleSelectedData.coefSampleSize *= 4;
                                        layer.sampleSelectedData.frequency = FR_011kHz;
                                        break;

                                    case 16000:
                                        layer.sampleSelectedData.coefSampleSize *= 3;
                                        layer.sampleSelectedData.frequency = FR_016kHz;
                                        break;

                                    case 22050:
                                        layer.sampleSelectedData.coefSampleSize *= 2;
                                        layer.sampleSelectedData.frequency = FR_022kHz;
                                        break;

                                    case 24000:
                                        layer.sampleSelectedData.coefSampleSize *= 2;
                                        layer.sampleSelectedData.frequency = FR_024kHz;
                                        break;

                                    case 44100:
                                        layer.sampleSelectedData.coefSampleSize *= 1;
                                        layer.sampleSelectedData.frequency = FR_044kHz;
                                        break;

                                    case 48000:
                                        layer.sampleSelectedData.coefSampleSize *= 1;
                                        layer.sampleSelectedData.frequency = FR_048kHz;
                                        break;

                                    case 88200:
                                        layer.sampleSelectedData.coefSampleSize *= 0.50;
                                        layer.sampleSelectedData.frequency = FR_088kHz;
                                        break;

                                    case 96000:
                                        layer.sampleSelectedData.coefSampleSize *= 0.50;
                                        layer.sampleSelectedData.frequency = FR_096kHz;
                                        break;

                                    case 176400:
                                        layer.sampleSelectedData.coefSampleSize *= 0.25;
                                        layer.sampleSelectedData.frequency = FR_176kHz;
                                        break;

                                    case 192000:
                                        layer.sampleSelectedData.coefSampleSize *= 0.25;
                                        layer.sampleSelectedData.frequency = FR_192kHz;
                                        break;
                                    }

                                    layer.sampleSelectedData.sampleByteSize = layer.wavSelectedData.data_chunk.chunkSize;
                                    layer.sampleSelectedData.sampleSize = layer.sampleSelectedData.sampleByteSize * layer.sampleSelectedData.coefSampleSize;

                                    // limit sampleSize
                                    if (layer.sampleSelectedData.sampleSize > kSampleSize) {
                                        layer.sampleSelectedData.sampleSize = kSampleSize;
                                        layer.sampleSelectedData.sampleByteSize = layer.sampleSelectedData.sampleSize / layer.sampleSelectedData.coefSampleSize;
                                    }
                                } else {
                                    sampleReadError = true;
                                }
                            } else {
                                sampleTypeError = true;
                            }
                        } else {
                            sampleReadError = true;
                        }
                    } else {
                        sampleReadError = true;
                    }
                } else {
                    sampleReadError = true;
                }
                f_close(&sd.file);

                if ((sampleReadError) || (sampleTypeError)) {
                    (layer.playSampleSector == 0) ? layer.writeSampleSector = 1 : layer.writeSampleSector = 0;
                    layer.playSampleSector = layer.writeSampleSector;
                    SampleSector &sampleSector = layer.sampleSector[layer.playSampleSector];
                    sampleSector.size = 0;
                    layer.sampleSelectedReadError = sampleReadError;
                    layer.sampleSelectedTypeError = sampleTypeError;
                    layer.sampleSelectedData.channel = CH_NA;
                    layer.sampleSelectedData.bitdepth = BD_NA;
                    layer.sampleSelectedData.frequency = FR_NA;
                }
            }
        } else {
            // clear data
            memset(layer.sampleSelectedData.num, '-', 4);
            memcpy(layer.sampleSelectedData.nameShortR, kDataDashR, 11);

            layer.sampleSelectedData.channel = CH_NA;
            layer.sampleSelectedData.bitdepth = BD_NA;
            layer.sampleSelectedData.frequency = FR_NA;

            layer.sampleSelectedData.coefSampleSize = 0;
            layer.sampleSelectedData.sampleByteSize = 0;
            layer.sampleSelectedData.sampleSize = 0;
        }

        f_close(&sd.file);
        // enable keyboard
        keyboard_enable();
        // update lcd
        if ((menu == LAYER_INST_MENU) && (layerNum_ == selectedLayerNum))
            lcd_drawLayerInst_SampleData();
    }
}

void Controller::layerInst_setSampleLoaded(uint8_t layerNum_) {
    f_close(&sd.file);
    Layer &layer = layerLibrary[layerNum_];
    if ((!layer.sampleSelectedReadError) && (!layer.sampleSelectedTypeError) && (layer.instSelected == layer.instLoaded)) {
        // disable keyboard
        keyboard_disable();
        // clear data
        memset(&(layer.sampleLoadedData), 0x00, sizeof(SampleData));
        memset(&(layer.wavLoadedData), 0x00, sizeof(WavData));
        if (layer.sampleSelected != -1) {
            // copy data
            layer.sampleLoaded = layer.sampleSelected;
            layer.sampleLoadedData = layer.sampleSelectedData;
            layer.wavLoadedData = layer.wavSelectedData;
            if ((menu == LAYER_INST_MENU) && (layerNum_ == selectedLayerNum)) {
                lcd_setMenuNumState(kLayerColorPalette[9]);
                lcd.drawText("    LOADING", 11, kMenuData4X[1], kMenuHeaderY + 23);
                HAL_Delay(100);
            }

            // read sample
            uint32_t rawDataSize = layer.sampleLoadedData.sampleByteSize;
            uint32_t rawArraySize = 36000;
            uint32_t rawArrayCounter = 0;
            uint32_t rawArrayCounterMax = rawDataSize / rawArraySize;
            uint32_t rawArrayRemainder = rawDataSize % rawArraySize;

            uint8_t rawArray[rawArraySize];

            uint8_t coefSamplerate;
            uint8_t coefChannel;
            uint8_t coefBps;

            uint32_t sdramDataCounter = 0;

            switch (layer.wavLoadedData.fmt_chunk.sampleRate) {
            case 8000:
            case 16000:
            case 11025:
            case 22050:
            case 44100:
            case 48000:
                coefSamplerate = 1;
                break;

            case 88200:
            case 96000:
                coefSamplerate = 2;
                break;

            case 176400:
            case 192000:
                coefSamplerate = 4;
                break;
            }

            switch (layer.wavLoadedData.fmt_chunk.nbrChannels) {
            case 1:
                coefChannel = 1;
                break;
            case 2:
                coefChannel = 2;
                break;
            }

            switch (layer.wavLoadedData.fmt_chunk.bitPerSample) {
            case 8:
                coefBps = 1;
                break;

            case 16:
                coefBps = 2;
                break;

            case 24:
                coefBps = 3;
                break;

            case 32:
                coefBps = 4;
                break;
            }

            uint32_t offsetSize = coefSamplerate * coefChannel * coefBps;
            uint32_t readArraySize;
            uint32_t readSampleSize;
            uint32_t tempSampleSize;

            int32_t minData = 0;
            int32_t maxData = 0;
            int32_t limitData;

            if ((f_open(&sd.file, layer.sampleLoadedData.fileName, FA_READ) == FR_OK) && (f_lseek(&sd.file, layer.wavLoadedData.data_chunk.chunkStartByte + 8) == FR_OK)) {
                // < 44100
                if (layer.wavLoadedData.fmt_chunk.sampleRate < 44100) {
                    uint16_t writeCount;
                    switch (layer.wavLoadedData.fmt_chunk.sampleRate) {
                    case 8000:
                        writeCount = 6;
                        break;

                    case 16000:
                        writeCount = 3;
                        break;

                    case 11025:
                        writeCount = 4;
                        break;

                    case 22050:
                        writeCount = 2;
                        break;
                    }

                    (layer.playSampleSector == 0) ? layer.writeSampleSector = 1 : layer.writeSampleSector = 0;
                    layer.sampleSector[layer.writeSampleSector].size = layer.sampleLoadedData.sampleSize - 1;

                    for (uint32_t i = 0; i <= rawArrayCounterMax; i++) {
                        // file to raw
                        (i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
                        if (f_read(&sd.file, (char *)&rawArray, readArraySize, &sd.bytesread) == FR_OK) {

                            // raw to temp
                            readSampleSize = readArraySize / offsetSize;
                            int32_t writeData;

                            for (uint32_t j = 0; j < readSampleSize; j++) {
                                if (layer.wavLoadedData.fmt_chunk.bitPerSample == 8) {
                                    int8_t *dataPtr = (int8_t *)&rawArray[j * offsetSize];
                                    int8_t temp = *dataPtr - 128;
                                    int32_t data = (int32_t)(temp << 16);
                                    if (temp >> 7)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 16) {
                                    int16_t *dataPtr = (int16_t *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)(*dataPtr << 8);
                                    if (*dataPtr >> 15)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 24) {
                                    int32_t *dataPtr = (int32_t *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)(*dataPtr & 0x00FFFFFF);
                                    if (data >> 23)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 32) {
                                    float *dataPtr = (float *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)((*dataPtr) * 8388607);
                                    writeData = data;
                                }

                                // temp to sdram
                                for (int k = 0; k < writeCount; k++) {
                                    if (writeData < minData)
                                        minData = writeData;
                                    if (writeData > maxData)
                                        maxData = writeData;
                                    sdram_write24BitAudio(kRamLayerAddressLibrary[layerNum_][layer.writeSampleSector] + (sdramDataCounter * 3), writeData);
                                    sdramDataCounter += 1;
                                }
                            }
                        }
                    }

                    if (sampleFadeOut) {
                        uint16_t fadeOutSize;
                        (layer.sampleLoadedData.sampleSize != kSampleSize) ? fadeOutSize = 100 : fadeOutSize = 1000;
                        sdram_fadeOut24BitAudio(kRamLayerAddressLibrary[layerNum_][layer.writeSampleSector], layer.sampleLoadedData.sampleSize, fadeOutSize);
                    }

                    minData *= -1;
                    (minData > maxData) ? limitData = minData : limitData = maxData;
                    layer.sampleSector[layer.writeSampleSector].normMultiplier = 8388607.0f / limitData;

                    layer.playSampleSector = layer.writeSampleSector;
                    layer.writeSampleSector != layer.writeSampleSector;
                    layer.samplePlay = true;
                }

                // >= 44100
                else if (layer.wavLoadedData.fmt_chunk.sampleRate >= 44100) {
                    (layer.playSampleSector == 0) ? layer.writeSampleSector = 1 : layer.writeSampleSector = 0;
                    layer.sampleSector[layer.writeSampleSector].size = layer.sampleLoadedData.sampleSize - 1;
                    for (uint32_t i = 0; i <= rawArrayCounterMax; i++) {
                        // file to raw
                        (i < rawArrayCounterMax) ? readArraySize = rawArraySize : readArraySize = rawArrayRemainder;
                        if (f_read(&sd.file, (char *)&rawArray, readArraySize, &sd.bytesread) == FR_OK) {

                            // raw to temp
                            readSampleSize = readArraySize / offsetSize;
                            int32_t writeData;

                            for (uint32_t j = 0; j < readSampleSize; j++) {
                                if (layer.wavLoadedData.fmt_chunk.bitPerSample == 8) {
                                    int8_t *dataPtr = (int8_t *)&rawArray[j * offsetSize];
                                    int8_t temp = *dataPtr - 128;
                                    int32_t data = (int32_t)(temp << 16);
                                    if (temp >> 7)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 16) {
                                    int16_t *dataPtr = (int16_t *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)(*dataPtr << 8);
                                    if (*dataPtr >> 15)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 24) {
                                    int32_t *dataPtr = (int32_t *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)(*dataPtr & 0x00FFFFFF);
                                    if (data >> 23)
                                        data |= (0xFF << 24);
                                    writeData = data;
                                } else if (layer.wavLoadedData.fmt_chunk.bitPerSample == 32) {
                                    float *dataPtr = (float *)&rawArray[j * offsetSize];
                                    int32_t data = (int32_t)((*dataPtr) * 8388607);
                                    writeData = data;
                                }

                                // temp to sdram
                                if (writeData < minData)
                                    minData = writeData;
                                if (writeData > maxData)
                                    maxData = writeData;
                                sdram_write24BitAudio(kRamLayerAddressLibrary[layerNum_][layer.writeSampleSector] + (sdramDataCounter * 3), writeData);
                                sdramDataCounter += 1;
                            }
                        }
                    }

                    if (sampleFadeOut) {
                        uint16_t fadeOutSize;
                        (layer.sampleLoadedData.sampleSize != kSampleSize) ? fadeOutSize = 100 : fadeOutSize = 1000;
                        sdram_fadeOut24BitAudio(kRamLayerAddressLibrary[layerNum_][layer.writeSampleSector], layer.sampleLoadedData.sampleSize, fadeOutSize);
                    }

                    minData *= -1;
                    (minData > maxData) ? limitData = minData : limitData = maxData;
                    layer.sampleSector[layer.writeSampleSector].normMultiplier = 8388607.0f / limitData;

                    layer.playSampleSector = layer.writeSampleSector;
                    layer.writeSampleSector != layer.writeSampleSector;
                    layer.samplePlay = true;
                }
                /*
                lcd.setForeColor(WHITE);
                lcd.setFont(FONT_05x07);
                lcd.setAlignment(LEFT);
                lcd.drawNumber(layer.playSampleSector, 2, 20, 50);
                lcd.drawNumber(layer.sampleSector[layer.playSampleSector].size, 6, 20, 65);

                for (uint8_t i = 0; i < 20; i++) {
                    uint32_t address = (uint32_t)(kRamLayerAddressLibrary[layerNum_][layer.writeSampleSector] + (i * 3));
                    int32_t x = sdram_read24BitAudio(address);
                    lcd.drawNumber(x, 10, 20, 80 + (i * 15));
                }
                */
            }
        } else {
            layer.samplePlay = false;
            // copy data
            layer.sampleLoaded = layer.sampleSelected;
            layer.sampleLoadedData = layer.sampleSelectedData;
            layer.wavLoadedData = layer.wavSelectedData;
        }
        f_close(&sd.file);
        // enable keyboard
        keyboard_enable();
        // update lcd
        if ((menu == LAYER_INST_MENU) && (layerNum_ == selectedLayerNum))
            lcd_drawLayerInst_SampleData();
        if (playActive) {
            lcd.setForeColor(playColor);
            lcd.drawHLine(kPlayX, kPlayY, playX);
            (playColor == kPlayColor0) ? lcd.setForeColor(kPlayColor1) : lcd.setForeColor(kPlayColor0);
            lcd.drawHLine(kPlayX + playX, kPlayY, kPlayWidth - playX);
        }
    }
}

void Controller::layerInst_setVolume(uint8_t layerNum_, uint8_t volume_) {
    if ((volume_ >= kMinLayerVolume) && (volume_ <= kMaxLayerVolume)) {
        // update data
        Layer &layer = layerLibrary[layerNum_];
        layer.volume = volume_;
        // update lcd
        if ((menu == LAYER_INST_MENU) && (selectedLayerNum == layer.number))
            lcd_drawLayerInst_VolumeData();
    }
}

void Controller::layerInst_setSpeed(uint8_t layerNum_, uint8_t speed_) {
    if ((speed_ >= kMinLayerSpeed) && (speed_ <= kMaxLayerSpeed)) {
        // update data
        Layer &layer = layerLibrary[layerNum_];
        layer.speed = speed_;
        // update lcd
        if ((menu == LAYER_INST_MENU) && (selectedLayerNum == layer.number))
            lcd_drawLayerInst_SpeedData();
    }
}

void Controller::layerInst_setReverse(uint8_t layerNum_, bool reverse_) {
    // update data
    Layer &layer = layerLibrary[layerNum_];
    layer.reverse = reverse_;
    // update lcd
    if ((menu == LAYER_INST_MENU) && (selectedLayerNum == layer.number))
        lcd_drawLayerInst_ReverseData();
}

void Controller::layerInst_setNormalize(uint8_t layerNum_, bool normalize_) {
    // update data
    Layer &layer = layerLibrary[layerNum_];
    layer.normalize = normalize_;
    // update lcd
    if ((menu == LAYER_INST_MENU) && (selectedLayerNum == layer.number))
        lcd_drawLayerInst_NormalizeData();
}

void Controller::layerInst_setMute(uint8_t layerNum_, bool mute_) {
    layerLibrary[layerNum_].mute = mute_;
    lcd_drawLayer_Mute(layerNum_);
}

void Controller::layerInst_setFill(uint8_t layerNum_, bool fill_) {
    layerLibrary[layerNum_].fill = fill_;
    lcd_drawLayer_Fill(layerNum_);
}

void Controller::layerInst_setStyle(uint8_t layerNum_, bool style_) {
    layerLibrary[layerNum_].style = style_;
    lcd_drawLayer_Style(layerNum_);
}

/* Layer Song functions ------------------------------------------------------*/

void Controller::layerSong_select(uint8_t layerNum_, uint8_t bankNum_) {
    if ((menu != LAYER_SONG_MENU) || ((menu == LAYER_SONG_MENU) && (selectedLayerNum != layerNum_))) {
        preMenu = menu;
        preMenuTab = menuTab;
        menuTab = 0;
        subMenuTab = 0;
        menu = LAYER_SONG_MENU;
        preMenuLayerClear();
        selectedLayerNum = layerNum_;
        (layerLibrary[selectedLayerNum].bankLibrary[bankNum_].lastActiveBeatNum != -1) ? selectedBeatNum = 0 : selectedBeatNum = -1;
        lcd_transitionMenu();
        lcd_transitionSelect();
        lcd_drawLayerSongMenu();
        lcd_drawLayerSelect();
    }
}

void Controller::layerSong_reset(uint8_t layerNum_, uint8_t bankNum_) {
    layerSong_resetAllBeats(layerNum_, bankNum_);
}

void Controller::layerSong_menuRight() {
    Layer &layer = layerLibrary[selectedLayerNum];

    if (layer.bankLibrary[activeBankNum].lastActiveBeatNum > 0) {
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, false);
        (selectedBeatNum < layer.bankLibrary[activeBankNum].lastActiveBeatNum) ? selectedBeatNum += 1 : selectedBeatNum = 0;
        // draw beat
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, true);
        // draw layer menu data
        lcd_drawLayerSong_BeatFillData();
        lcd_drawLayerSong_BeatGraphData();
    }
}

void Controller::layerSong_menuLeft() {
    Layer &layer = layerLibrary[selectedLayerNum];

    if (layer.bankLibrary[activeBankNum].lastActiveBeatNum > 0) {
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, false);
        (selectedBeatNum > 0) ? selectedBeatNum -= 1 : selectedBeatNum = layer.bankLibrary[activeBankNum].lastActiveBeatNum;
        // draw beat
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, true);
        // draw layer menu data
        lcd_drawLayerSong_BeatFillData();
        lcd_drawLayerSong_BeatGraphData();
    }
}

void Controller::layerSong_menuUp() {
    Layer &layer = layerLibrary[selectedLayerNum];

    if (layer.bankLibrary[activeBankNum].lastActiveBeatNum != -1) {
        uint8_t fill = layer.bankLibrary[activeBankNum].beatLibrary[selectedBeatNum].getFill();
        (fill < kMaxLayerFill) ? fill += 1 : fill = 0;
        layerSong_calculateBeatFill(selectedLayerNum, activeBankNum, selectedBeatNum, fill);
        layerSong_setBeatFill(fill);
    }
}

void Controller::layerSong_menuDown() {
    Layer &layer = layerLibrary[selectedLayerNum];

    if (layer.bankLibrary[activeBankNum].lastActiveBeatNum != -1) {
        uint8_t fill = layer.bankLibrary[activeBankNum].beatLibrary[selectedBeatNum].getFill();
        (fill > kMinLayerFill) ? fill -= 1 : fill = kMaxLayerFill;
        layerSong_calculateBeatFill(selectedLayerNum, activeBankNum, selectedBeatNum, fill);
        layerSong_setBeatFill(fill);
    }
}

void Controller::layerSong_setBeat(uint8_t layerNum_, uint8_t bankNum_, uint16_t interval_, uint8_t fill_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];

    // record beat
    if (bank.lastActiveBeatNum < kBeatLibrarySize) {
        uint16_t quantizeInterval = kQuantizeInterval[rhythm.quantize];
        uint16_t startInterval = ((interval_ / quantizeInterval) + ((interval_ % quantizeInterval) / (quantizeInterval / 2))) * quantizeInterval;
        uint16_t endInterval;

        // reset beat interval if it equals to song interval
        if (startInterval == songInterval)
            startInterval = 0;

        // calculate beatNum
        bool duplicate = false;
        uint8_t beatNum;
        if (bank.lastActiveBeatNum == -1) {
            beatNum = 0;
            endInterval = songInterval;
        } else {
            beatNum = 0;
            for (uint8_t i = 0; i <= bank.lastActiveBeatNum; i++) {
                // check duplicate
                if (bank.beatLibrary[i].getStartInterval() == startInterval) {
                    duplicate = true;
                    break;
                }
                // increment beatNum
                else if (bank.beatLibrary[i].getStartInterval() < startInterval) {
                    beatNum = i + 1;
                }
                // get beatNum
                else {
                    beatNum = i;
                    break;
                }
            }

            if (!duplicate) {
                // shift beats
                if (beatNum <= bank.lastActiveBeatNum) {
                    for (int8_t j = bank.lastActiveBeatNum; j >= beatNum; j--) {
                        bank.beatLibrary[j + 1] = bank.beatLibrary[j];
                    }
                }
                // reset previous fill
                if (beatNum > 0) {
                    bank.beatLibrary[beatNum - 1].setEndInterval(startInterval);
                    bank.beatLibrary[beatNum - 1].setFill(0);
                    lcd_drawBeat(layerNum_, bankNum_, beatNum - 1, false);
                }
                // calculate end interval
                if (beatNum == (bank.lastActiveBeatNum + 1)) {
                    endInterval = songInterval;
                } else {
                    endInterval = bank.beatLibrary[beatNum + 1].getStartInterval();
                }
            }
        }

        if (!duplicate) {
            // set beat
            bank.beatLibrary[beatNum].set(startInterval, endInterval, fill_);
            layerSong_calculateLastActiveBeatNum(layerNum_, bankNum_);
            layerSong_calculatePlayBeatNum(layerNum_, bankNum_, playInterval);
            // draw beat
            if ((menu == LAYER_SONG_MENU) && (selectedLayerNum == layerNum_)) {
                if (selectedBeatNum >= beatNum)
                    selectedBeatNum += 1;
                if (selectedBeatNum != -1) {
                    lcd_drawBeat(layerNum_, bankNum_, selectedBeatNum, false);
                }
                selectedBeatNum = beatNum;
                lcd_drawBeat(layerNum_, bankNum_, selectedBeatNum, true);
                lcd_drawLayerSong_BeatFillData();
                lcd_drawLayerSong_BeatGraphData();
            } else {
                lcd_drawBeat(layerNum_, bankNum_, beatNum, false);
            }
            // remove double beat sound
            int8_t playBeatNum = bank.playBeatNum;
            if (playBeatNum == beatNum) {
                if (playBeatNum < bank.lastActiveBeatNum) {
                    bank.playBeatNum += 1;
                } else {
                    bank.playBeatNum = 0;
                }
            }
        }
    }
}

void Controller::layerSong_setBeatFill(uint8_t fill_) {
    // set fill
    layerSong_calculateBeatFill(selectedLayerNum, activeBankNum, selectedBeatNum, fill_);
    layerSong_calculatePlayBeatNum(selectedLayerNum, activeBankNum, playInterval);
    // draw fill data
    lcd_drawLayerSong_BeatFillData();
    lcd_drawLayerSong_BeatGraphData();
    lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, true);
}

void Controller::layerSong_generateBeat(uint8_t type) {
    uint16_t interval;
    switch (type) {
    case 0:
        for (uint8_t i = 0; i < rhythm.bar; i++) {
            interval = i * barInterval;
            layerSong_setBeat(selectedLayerNum, activeBankNum, interval, 0);
        }
        break;

    case 1:
        for (uint8_t i = 0; i < rhythm.bar; i++) {
            interval = i * barInterval + (barInterval / 2);
            layerSong_setBeat(selectedLayerNum, activeBankNum, interval, 0);
        }
        break;

    case 2:
        for (uint8_t i = 0; i < rhythm.measureTotal; i++) {
            interval = i * measureInterval;
            if (interval % (barInterval / 2) != 0) {
                layerSong_setBeat(selectedLayerNum, activeBankNum, i * measureInterval, 0);
            }
        }
        break;
    }

    if (menu == LAYER_SONG_MENU) {
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, false);
        selectedBeatNum = 0;
        lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, true);
        lcd_drawLayerSong_BeatFillData();
        lcd_drawLayerSong_BeatGraphData();
    }

    layerSong_calculatePlayBeatNum(selectedLayerNum, activeBankNum, playInterval);
}

void Controller::layerSong_resetSelectedBeat() {
    if (selectedLayerNum != -1) {
        Layer &layer = layerLibrary[selectedLayerNum];
        Bank &bank = layer.bankLibrary[activeBankNum];
        if (bank.lastActiveBeatNum != -1) {
            // revise previous beat
            if (selectedBeatNum > 0) {
                if (selectedBeatNum != bank.lastActiveBeatNum) {
                    bank.beatLibrary[selectedBeatNum - 1].setEndInterval(bank.beatLibrary[selectedBeatNum + 1].getStartInterval());
                } else {
                    bank.beatLibrary[selectedBeatNum - 1].setEndInterval(songInterval);
                }
                bank.beatLibrary[selectedBeatNum - 1].setFill(0);
                lcd_drawBeat(selectedLayerNum, activeBankNum, (selectedBeatNum - 1), false);
            }

            // reset beat
            lcd_clearBeat(selectedLayerNum, activeBankNum, selectedBeatNum);
            bank.beatLibrary[selectedBeatNum].reset();

            // arrange beat library
            layerSong_arrangeActiveBeats(selectedLayerNum, activeBankNum, false, true, true, true);

            if (bank.lastActiveBeatNum != -1) {
                // set selectedBeatNum
                if (selectedBeatNum == 0) {
                    selectedBeatNum = bank.lastActiveBeatNum;
                } else {
                    selectedBeatNum -= 1;
                }
                // draw selected beat
                lcd_drawBeat(selectedLayerNum, activeBankNum, selectedBeatNum, true);
            } else {
                selectedBeatNum = -1;
            }
            // draw layer menu data
            lcd_drawLayerSong_BeatFillData();
            lcd_drawLayerSong_BeatGraphData();
        }
    }
}

void Controller::layerSong_resetBeats(uint8_t layerNum_, uint8_t bankNum_, uint16_t startInterval_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    if (bank.lastActiveBeatNum != -1) {
        bool reorder = false;
        for (uint8_t i = 0; i <= bank.lastActiveBeatNum; i++) {
            Beat &beat = bank.beatLibrary[i];
            if (beat.getStartInterval() >= startInterval_) {
                // reset previous beat fill
                if (i > 0) {
                    bank.beatLibrary[i - 1].setEndInterval(songInterval);
                    bank.beatLibrary[i - 1].setFill(0);
                }
                // reset beat
                beat.reset();
                reorder = true;
            }
        }
        layerSong_arrangeActiveBeats(layerNum_, bankNum_, reorder, reorder, reorder, true);
    }
}

void Controller::layerSong_resetAllBeats(uint8_t layerNum_, uint8_t bankNum_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    if (bank.lastActiveBeatNum != -1) {
        for (uint16_t i = 0; i < kBeatLibrarySize; i++) {
            bank.beatLibrary[i].reset();
        }
        bank.lastActiveBeatNum = -1;
        bank.playBeatNum = -1;
        lcd_clearSong(layerNum_, bankNum_);
        if ((selectedLayerNum == layerNum_) && (activeBankNum == bankNum_) && (menu = LAYER_SONG_MENU)) {
            selectedBeatNum = -1;
            lcd_drawLayerSong_BeatFillData();
            lcd_drawLayerSong_BeatGraphData();
        }
    }
}

void Controller::layerSong_quantizeActiveBeats(uint8_t layerNum_, uint8_t bankNum_) {
    uint16_t quantizeInterval = kQuantizeInterval[rhythm.quantize];
    // calculate beat's quantize intervals
    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        Layer &layer = layerLibrary[i];
        Bank &bank = layer.bankLibrary[bankNum_];
        if (bank.lastActiveBeatNum != -1) {
            for (uint8_t j = 0; j < kBeatLibrarySize; j++) {
                Beat &beat = bank.beatLibrary[j];
                if (beat.getActive()) {
                    uint16_t startInterval = beat.getStartInterval();
                    uint16_t endInterval = beat.getEndInterval();
                    if (startInterval != 0)
                        startInterval -= 1;
                    if (endInterval != 0)
                        endInterval -= 1;
                    startInterval = ((startInterval / quantizeInterval) + ((startInterval % quantizeInterval) / (quantizeInterval / 2))) * quantizeInterval;
                    endInterval = ((endInterval / quantizeInterval) + ((endInterval % quantizeInterval) / (quantizeInterval / 2))) * quantizeInterval;
                    if (endInterval > songInterval) {
                        endInterval = songInterval;
                    }
                    if (startInterval != songInterval) {
                        beat.setStartInterval(startInterval);
                        beat.setEndInterval(endInterval);
                        beat.setFill(0);
                    } else {
                        beat.reset();
                    }
                }
            }
            layerSong_arrangeActiveBeats(layerNum_, bankNum_, true, true, true, true);
        }
    }
}

void Controller::layerSong_arrangeActiveBeats(uint8_t layerNum_, uint8_t bankNum_, bool duplicate_, bool collect_, bool sort_, bool lastFill_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    if (bank.lastActiveBeatNum != -1) {
        // phase 01: reset duplicate active beats
        if (duplicate_) {
            for (uint8_t i = 0; i < kBeatLibrarySize; i++) {
                Beat &beat = bank.beatLibrary[i];
                if (beat.getActive()) {
                    uint16_t beatStartInterval = beat.getStartInterval();
                    for (uint8_t j = i + 1; j < kBeatLibrarySize; j++) {
                        Beat &nextBeat = bank.beatLibrary[j];
                        if (nextBeat.getActive()) {
                            uint16_t nextStartInterval = bank.beatLibrary[j].getStartInterval();
                            if (nextStartInterval == beatStartInterval) {
                                nextBeat.reset();
                            }
                        }
                    }
                }
            }
        }
        // phase 02: collect active beats
        if (collect_) {
            Beat tempLibrary[kBeatLibrarySize];
            int8_t tempCounter = -1;
            for (uint8_t i = 0; i < kBeatLibrarySize; i++) {
                if (bank.beatLibrary[i].getActive()) {
                    tempCounter += 1;
                    tempLibrary[tempCounter] = bank.beatLibrary[i];
                }
            }
            for (uint8_t j = 0; j < kBeatLibrarySize; j++) {
                bank.beatLibrary[j] = tempLibrary[j];
            }
            bank.lastActiveBeatNum = tempCounter;
        }
        // phase 03: sort active beats
        if ((sort_) && (bank.lastActiveBeatNum > 0)) {
            // a. sort ref start interval
            uint8_t min;
            Beat temp;
            for (uint8_t i = 0; i < (bank.lastActiveBeatNum - 1); i++) {
                min = i;
                for (uint8_t j = i + 1; j < bank.lastActiveBeatNum; j++) {
                    if (bank.beatLibrary[j].getStartInterval() < bank.beatLibrary[min].getStartInterval()) {
                        min = j;
                    }
                }
                temp = bank.beatLibrary[i];
                bank.beatLibrary[i] = bank.beatLibrary[min];
                bank.beatLibrary[min] = temp;
            }
            // b. set end interval
            for (uint8_t k = 0; k < bank.lastActiveBeatNum; k++) {
                bank.beatLibrary[k].setEndInterval(bank.beatLibrary[k + 1].getStartInterval());
            }
            bank.beatLibrary[bank.lastActiveBeatNum].setEndInterval(songInterval);
            // c. set fill
            for (uint8_t l = 0; l < bank.lastActiveBeatNum; l++) {
                uint8_t fill = bank.beatLibrary[l].getFill();
                bank.beatLibrary[l].setFill(fill);
            }
        }
        // phase 04: lastFill check
        if (lastFill_) {
            if (bank.lastActiveBeatNum != -1) {
                Beat &beat = bank.beatLibrary[bank.lastActiveBeatNum];
                if ((beat.getFill() != 0) && (beat.getEndInterval() != songInterval)) {
                    beat.setFill(0);
                }
            }
        }
        // phase 05: calculate play beat
        layerSong_calculatePlayBeatNum(layerNum_, bankNum_, playInterval);
    }
}

void Controller::layerSong_calculateLastActiveBeatNum(uint8_t layerNum_, uint8_t bankNum_) {
    int8_t lastActiveBeatNum = -1;
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    for (uint16_t i = 0; i < kBeatLibrarySize; i++) {
        if (bank.beatLibrary[i].getActive()) {
            lastActiveBeatNum = i;
        } else {
            break;
        }
    }
    bank.lastActiveBeatNum = lastActiveBeatNum;
}

void Controller::layerSong_calculatePlayBeatNum(uint8_t layerNum_, uint8_t bankNum_, uint16_t playInterval_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    if (bank.lastActiveBeatNum != -1) {
        // layer fill on
        if (layer.fill) {
            bool findFlag = false;
            for (uint8_t i = 0; i <= bank.lastActiveBeatNum; i++) {
                Beat &beat = bank.beatLibrary[i];
                for (uint8_t j = 0; j < kFillDataLibrary[beat.getFill()].step; j++) {
                    BeatMicro &beatMicro = beat.beatMicroLibrary[j];
                    if ((beat.getActive()) && (beatMicro.getActive()) && (beatMicro.getInterval() >= playInterval)) {
                        bank.playBeatNum = i;
                        bank.playBeatMicroNum = j;
                        findFlag = true;
                        break;
                    }
                }
                if (findFlag) {
                    break;
                }
            }
        }
        // layer fill off
        else {
            for (uint8_t i = 0; i <= bank.lastActiveBeatNum; i++) {
                Beat &beat = bank.beatLibrary[i];
                BeatMicro &beatMicro = beat.beatMicroLibrary[0];
                if ((beat.getActive()) && (beatMicro.getActive()) && (beatMicro.getInterval() >= playInterval)) {
                    bank.playBeatNum = i;
                    bank.playBeatMicroNum = 0;
                    break;
                }
            }
        }
    } else {
        bank.playBeatNum = -1;
        bank.playBeatMicroNum = -1;
    }
}

void Controller::layerSong_calculateBeatFill(uint8_t layerNum_, uint8_t bankNum_, uint8_t beatNum_, uint8_t fill_) {
    Layer &layer = layerLibrary[layerNum_];
    Bank &bank = layer.bankLibrary[bankNum_];
    uint16_t endInterval;
    (beatNum_ < bank.lastActiveBeatNum) ? endInterval = bank.beatLibrary[beatNum_ + 1].getStartInterval() : endInterval = songInterval;
    bank.beatLibrary[beatNum_].setEndInterval(endInterval);
    bank.beatLibrary[beatNum_].setFill(fill_);
}

/* Bank functions ----------------------------------------------------------*/

void Controller::bank_select(uint8_t bankNum_) {
    if (bankNum_ != activeBankNum) {
        uint8_t preBankNum = activeBankNum;
        activeBankNum = bankNum_;
        lcd_drawBank(bankNum_, false);

        if (menu == LAYER_SONG_MENU) {
            (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1) ? selectedBeatNum = 0 : selectedBeatNum = -1;
            lcd_drawLayerSong_BeatFillData();
            lcd_drawLayerSong_BeatGraphData();
        }

        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            lcd_clearSong(i, preBankNum);
            lcd_drawSong(i, activeBankNum);
            layerSong_calculatePlayBeatNum(i, activeBankNum, playInterval);
        }
    }
}

void Controller::bank_trigger(uint8_t bankNum_) {
    if (bankNum_ != activeBankNum) {
        bankShiftFlag = true;
        targetBankNum = bankNum_;
        lcd_drawBank(bankNum_, true);
    } else if ((bankNum_ == activeBankNum) && (bankShiftFlag) && (targetBankNum != activeBankNum)) {
        bankShiftFlag = false;
        targetBankNum = bankNum_;
        lcd_drawBank(bankNum_, false);
    }
}

/* Play functions ------------------------------------------------------------*/

void Controller::record() {
    if (system.sync.syncOutPlay)
        sendSyncCommand(SYNC_RECORD);

    // start play & record
    if ((!playActive) && (!recordActive)) {
        if ((metronome.active) && (metronome.precount)) {
            metronome.precountState = true;
            metronome.precounter = 0;
            metronome.countDown = rhythm.measure;
        }
        recordActive = true;

        recordIcon.flag = true;
        recordIcon.mode = true;

        stopFlag = false;
        resetFlag = false;
        playActive = true;

        resetIcon.flag = true;
        resetIcon.mode = false;

        playIcon.flag = true;
        playIcon.mode = true;

        stopIcon.flag = true;
        stopIcon.mode = false;

        startPlayTimer();
    }

    // continue play & stop record
    else if ((playActive) && (recordActive)) {
        if (metronome.precountState) {
            stop();
        } else {
            recordActive = false;

            stopFlag = false;
            resetFlag = false;

            recordIcon.flag = true;
            recordIcon.mode = false;
        }
    }

    // continue play & start record
    else if ((playActive) && (!recordActive)) {
        recordActive = true;

        stopFlag = false;
        resetFlag = false;

        recordIcon.flag = true;
        recordIcon.mode = true;
    }
}

void Controller::play() {
    if (system.sync.syncOutPlay)
        sendSyncCommand(SYNC_PLAY);

    stopFlag = false;
    resetFlag = false;
    playActive = true;

    resetIcon.flag = true;
    resetIcon.mode = false;

    playIcon.flag = true;
    playIcon.mode = true;

    stopIcon.flag = true;
    stopIcon.mode = false;

    startPlayTimer();
}

void Controller::stop() {
    if ((system.sync.syncOutPlay) && (!stopFlag))
        sendSyncCommand(SYNC_STOP);

    if (metronome.precountState) {
        metronome.precountState = false;
        metronome.precounter = 0;
        metronome.countDownFlag = false;
        metronome.countDown = 0;
        lcd_clearCountDown();
    }

    if (bankShiftFlag) {
        bankShiftFlag = false;
        targetBankNum = activeBankNum;
        lcd_drawBank(activeBankNum, false);
    }

    stopFlag = false;
    resetFlag = false;

    playActive = false;
    recordActive = false;

    resetIcon.flag = true;
    resetIcon.mode = false;

    playIcon.flag = true;
    playIcon.mode = false;

    stopIcon.flag = true;
    stopIcon.mode = true;

    recordIcon.flag = true;
    recordIcon.mode = false;

    stopPlayTimer();
}

void Controller::reset() {
    if ((system.sync.syncOutPlay) && (!resetFlag))
        sendSyncCommand(SYNC_RESET);

    if ((playInterval != 0) || (metronome.precountState)) {
        if (metronome.precountState) {
            metronome.precountState = false;
            metronome.precounter = 0;
            metronome.countDownFlag = false;
            metronome.countDown = 0;
            lcd_clearCountDown();
        }

        if (bankShiftFlag) {
            bankShiftFlag = false;
            targetBankNum = activeBankNum;
            lcd_drawBank(activeBankNum, false);
        }

        stopFlag = false;
        resetFlag = false;

        playActive = false;
        recordActive = false;

        resetIcon.flag = true;
        resetIcon.mode = true;

        playIcon.flag = true;
        playIcon.mode = false;

        stopIcon.flag = true;
        stopIcon.mode = false;

        recordIcon.flag = true;
        recordIcon.mode = false;

        stopPlayTimer();

        playInterval = 0;

        resetPlayFlag = true;

        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            layerSong_calculatePlayBeatNum(i, activeBankNum, playInterval);
        }
    }
}

void Controller::triggerStop() {
    if (system.sync.syncOutPlay)
        sendSyncCommand(SYNC_TRIG_STOP);

    if (metronome.precountState) {
        stop();
    } else {
        stopFlag = true;
        resetFlag = false;
        stopInterval = calculateTriggerInterval();

        resetIcon.flag = true;
        resetIcon.mode = false;

        // stopIcon.flag = true;
        // stopIcon.mode = true;
    }
}

void Controller::triggerReset() {
    if (system.sync.syncOutPlay)
        sendSyncCommand(SYNC_TRIG_RESET);

    if (metronome.precountState) {
        reset();
    } else {
        stopFlag = false;
        resetFlag = true;
        resetInterval = calculateTriggerInterval();

        // resetIcon.flag = true;
        // resetIcon.mode = true;

        stopIcon.flag = true;
        stopIcon.mode = false;
    }
}

/* Interrupt functions -------------------------------------------------------*/

void Controller::interruptPlay() {
    // metro pre count state
    if (metronome.precountState) {
        // trigger bar beat
        if ((metronome.precounter % barInterval == 0) && (!system.sync.slaveMode)) {
            mD.active = true;
            mD.counter = 0;
            mD.counterMax = kMetroSize - 1;
            mD.ramAddress = kRamMetronomeAddressLibrary[metronome.sample][0];
            mD.volumeMultiplier = 0.75f * metronome.volumeFloat;

            metronome.countDownFlag = true;
        }
        // trigger measure beat
        else if ((metronome.precounter % measureInterval == 0) && (!system.sync.slaveMode)) {
            mD.active = true;
            mD.counter = 0;
            mD.counterMax = kMetroSize - 1;
            mD.ramAddress = kRamMetronomeAddressLibrary[metronome.sample][1];
            mD.volumeMultiplier = 0.75f * metronome.volumeFloat;

            metronome.countDownFlag = true;
        }
        // increment metro count play interval
        if (metronome.precounter < metronome.precounterMax) {
            metronome.precounter += 1;
        } else {
            metronome.precounter = 0;
            metronome.precountState = false;
        }
    }

    // play & record state
    else {
        // check metronome
        if ((recordActive) && (metronome.active) && (playInterval != songInterval)) {
            // trigger bar beat
            if ((playInterval % barInterval == 0) && (!system.sync.slaveMode)) {
                mD.active = true;
                mD.counter = 0;
                mD.counterMax = kMetroSize - 1;
                mD.ramAddress = kRamMetronomeAddressLibrary[metronome.sample][0];
                mD.volumeMultiplier = 0.75f * metronome.volumeFloat;
            }
            // trigger measure beat
            else if ((playInterval % measureInterval == 0) && (!system.sync.slaveMode)) {
                mD.active = true;
                mD.counter = 0;
                mD.counterMax = kMetroSize - 1;
                mD.ramAddress = kRamMetronomeAddressLibrary[metronome.sample][1];
                mD.volumeMultiplier = 0.75f * metronome.volumeFloat;
            }
        }

        // beat sync
        if ((system.syncOut == 2) && (playInterval % kMeasureHalfInterval == 0)) {
            SYNC_OUT_ON;
            startBeatSyncTimer();
        }

        // check beat
        for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
            Layer &layer = layerLibrary[i];
            Bank &bank = layer.bankLibrary[activeBankNum];
            int8_t playBeatNum = bank.playBeatNum;
            int8_t playBeatMicroNum = bank.playBeatMicroNum;
            if ((bank.lastActiveBeatNum != -1)) {
                Beat &beat = bank.beatLibrary[playBeatNum];
                BeatMicro &beatMicro = beat.beatMicroLibrary[playBeatMicroNum];
                uint16_t beatMicroInterval = beatMicro.interval;
                // check playBeatMicro
                if (beatMicroInterval == playInterval) {
                    // trigger playBeatMicro
                    if ((!layer.mute) && (layer.samplePlay) && ((playBeatMicroNum == 0) || ((playBeatMicroNum != 0) && (layer.fill)))) {
                        LayerPlayData &lD = sD.layerData[layer.number];
                        lD.beatData.active = true;
                        lD.beatData.counterMax = layer.sampleSector[layer.playSampleSector].size;
                        lD.beatData.ramAddress = kRamLayerAddressLibrary[layer.number][layer.playSampleSector];
                        lD.beatData.volumeMultiplier = kLayerVolumeCoef * kFloatDataLibrary[layer.volume / 5].pow2Multiplier * kFloatDataLibrary[beatMicro.getVolume() / 5].pow2Multiplier;
                        if (lD.beatData.normalize)
                            lD.beatData.volumeMultiplier *= layer.sampleSector[layer.playSampleSector].normMultiplier;
                        if (layer.style) {
                            lD.beatData.speed = layer.speed;
                            lD.beatData.reverse = layer.reverse;
                        } else {
                            lD.beatData.speed = kInitialLayerSpeed;
                            lD.beatData.reverse = kInitialLayerReverse;
                        }

                        lD.beatData.increment = kLayerSpeedDataLibrary[lD.beatData.speed].increment;
                        (lD.beatData.reverse) ? lD.beatData.counter = lD.beatData.counterMax - 1 : lD.beatData.counter = 0;
                    }
                    // increment playBeatMicro
                    if ((playBeatMicroNum < (kFillDataLibrary[beat.getFill()].step - 1))) {
                        bank.playBeatMicroNum += 1;
                    }
                    // increment playBeat & playBeatMicro
                    else if (playBeatNum < bank.lastActiveBeatNum) {
                        playBeatNum += 1;
                        bank.playBeatNum += 1;
                        bank.playBeatMicroNum = 0;
                    }
                    // reset playBeat & playBeatMicro
                    else {
                        playBeatNum = 0;
                        bank.playBeatNum = 0;
                        bank.playBeatMicroNum = 0;
                    }
                }
            }
        }

        // increment play interval
        playInterval += 1;
        if (playInterval < songInterval) {
            if (stopFlag) {
                if (stopInterval == playInterval) {
                    stop();
                } else if (playInterval % (measureInterval / 2) == 0) {
                    stopIcon.flag = true;
                    stopIcon.mode = !stopIcon.mode;
                }
            }

            if (resetFlag) {
                if (resetInterval == playInterval) {
                    reset();
                } else if (playInterval % (measureInterval / 2) == 0) {
                    resetIcon.flag = true;
                    resetIcon.mode = !resetIcon.mode;
                }
            }
        } else {
            playInterval = 0;
            if (bankShiftFlag) {
                bankShiftFlag = false;
                activeBankNum = targetBankNum;
                bankActionFlag = true;
            }
            for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
                Layer &layer = layerLibrary[i];
                Bank &bank = layer.bankLibrary[activeBankNum];
                if (bank.lastActiveBeatNum != -1) {
                    bank.playBeatNum = 0;
                    bank.playBeatMicroNum = 0;
                } else {
                    bank.playBeatNum = -1;
                    bank.playBeatMicroNum = -1;
                }
            }
            lcd_restartPlay();
        }
    }
}

void Controller::interruptAudioMetronome() {
    audioMetronome = 0;

    if (mD.active) {
        uint32_t address = (uint32_t)(mD.ramAddress + (mD.counter * 3));
        audioMetronome = (sdram_read24BitAudio(address) << 8) * mD.volumeMultiplier;

        if (mD.counter < mD.counterMax) {
            mD.counter += 1;
        } else {
            mD.counter = 0;
            mD.active = false;
        }
    }
}

void Controller::interruptAudioSong() {
    audioSong = 0;

    for (uint8_t i = 0; i < kLayerLibrarySize; i++) {
        LayerPlayData &lD = sD.layerData[i];

        if (lD.beatData.active) {
            int32_t audioLayer = 0;

            float counterFloat = lD.beatData.counter;
            uint32_t counterInt = floor(lD.beatData.counter);
            float counterDiff = counterFloat - counterInt;

            // get data
            if (counterDiff == 0) {
                uint32_t ramAddress = lD.beatData.ramAddress + (counterInt * 3);
                audioLayer = sdram_read24BitAudio(ramAddress) * lD.beatData.volumeMultiplier;
            } else {
                uint32_t ramAddressLow = lD.beatData.ramAddress + (counterInt * 3);
                uint32_t ramAddressHigh = ramAddressLow + 3;

                int32_t dataLow = sdram_read24BitAudio(ramAddressLow);
                int32_t dataHigh = sdram_read24BitAudio(ramAddressHigh);
                int32_t dataDiff = dataHigh - dataLow;
                int32_t data = dataLow + (dataDiff * counterDiff);

                audioLayer = data * lD.beatData.volumeMultiplier;
            }

            // increment counter
            if (lD.beatData.reverse) {
                if (lD.beatData.counter == lD.beatData.counterMax - 1) {
                    system.midi.txTriggerNoteOn[i] = true;
                }
                lD.beatData.counter -= lD.beatData.increment;
                if (lD.beatData.counter < 0) {
                    lD.beatData.active = false;
                    lD.beatData.counter = 0;
                    system.midi.txTriggerNoteOff[i] = true;
                }
            } else {
                if (lD.beatData.counter == 0) {
                    system.midi.txTriggerNoteOn[i] = true;
                }
                lD.beatData.counter += lD.beatData.increment;
                if (lD.beatData.counter > (lD.beatData.counterMax - 1)) {
                    lD.beatData.active = false;
                    lD.beatData.counter = 0;
                    system.midi.txTriggerNoteOff[i] = true;
                }
            }

            audioSong += audioLayer;
        }
    }

    audioSong *= system.volumeFloat;
}

void Controller::interruptAudioLpf() {
    int32_t input = audioSong;

    if (lpf.active) {
        lpf.dataIn[0] = input;
        lpf.dataOut[0] = (lpf.a0 * lpf.dataIn[0]) + (lpf.a1 * lpf.dataIn[1]) + (lpf.a2 * lpf.dataIn[2]) - (lpf.b1 * lpf.dataOut[1]) - (lpf.b2 * lpf.dataOut[2]);

        lpf.dataIn[2] = lpf.dataIn[1];
        lpf.dataIn[1] = lpf.dataIn[0];

        lpf.dataOut[2] = lpf.dataOut[1];
        lpf.dataOut[1] = lpf.dataOut[0];

        audioLpf = (lpf.dataOut[0] * lpf.wet) + (input * lpf.dry);
    } else {
        audioLpf = input;
    }
}

void Controller::interruptAudioEq() {
    int32_t input = audioLpf;
    int32_t output;

    if ((eq.active) || (eq.genTransition.active)) {
        // low shelf filter
        eq.filterLowShelf.dataIn[0] = input;
        if (eq.gainLowShelf != kEqGainZero) {
            eq.filterLowShelf.dataOut[0] = (eq.filterLowShelf.a0 * eq.filterLowShelf.dataIn[0]) + (eq.filterLowShelf.a1 * eq.filterLowShelf.dataIn[1]) + (eq.filterLowShelf.a2 * eq.filterLowShelf.dataIn[2]) - (eq.filterLowShelf.b1 * eq.filterLowShelf.dataOut[1]) - (eq.filterLowShelf.b2 * eq.filterLowShelf.dataOut[2]);
        } else {
            eq.filterLowShelf.dataOut[0] = eq.filterLowShelf.dataIn[0];
        }
        eq.filterLowShelf.dataIn[2] = eq.filterLowShelf.dataIn[1];
        eq.filterLowShelf.dataIn[1] = eq.filterLowShelf.dataIn[0];

        eq.filterLowShelf.dataOut[2] = eq.filterLowShelf.dataOut[1];
        eq.filterLowShelf.dataOut[1] = eq.filterLowShelf.dataOut[0];

        // high shelf filter
        eq.filterHighShelf.dataIn[0] = eq.filterLowShelf.dataOut[0];
        if (eq.gainHighShelf != kEqGainZero) {
            eq.filterHighShelf.dataOut[0] = (eq.filterHighShelf.a0 * eq.filterHighShelf.dataIn[0]) + (eq.filterHighShelf.a1 * eq.filterHighShelf.dataIn[1]) + (eq.filterHighShelf.a2 * eq.filterHighShelf.dataIn[2]) - (eq.filterHighShelf.b1 * eq.filterHighShelf.dataOut[1]) - (eq.filterHighShelf.b2 * eq.filterHighShelf.dataOut[2]);
        } else {
            eq.filterHighShelf.dataOut[0] = eq.filterHighShelf.dataIn[0];
        }
        eq.filterHighShelf.dataIn[2] = eq.filterHighShelf.dataIn[1];
        eq.filterHighShelf.dataIn[1] = eq.filterHighShelf.dataIn[0];

        eq.filterHighShelf.dataOut[2] = eq.filterHighShelf.dataOut[1];
        eq.filterHighShelf.dataOut[1] = eq.filterHighShelf.dataOut[0];

        // peak 1
        eq.filterPeak[0].dataIn[0] = eq.filterHighShelf.dataOut[0];
        if (eq.gainPeak[0] != kEqGainZero) {
            eq.filterPeak[0].dataOut[0] = (eq.filterPeak[0].a0 * eq.filterPeak[0].dataIn[0]) + (eq.filterPeak[0].a1 * eq.filterPeak[0].dataIn[1]) + (eq.filterPeak[0].a2 * eq.filterPeak[0].dataIn[2]) - (eq.filterPeak[0].b1 * eq.filterPeak[0].dataOut[1]) - (eq.filterPeak[0].b2 * eq.filterPeak[0].dataOut[2]);
        } else {
            eq.filterPeak[0].dataOut[0] = eq.filterPeak[0].dataIn[0];
        }
        eq.filterPeak[0].dataIn[2] = eq.filterPeak[0].dataIn[1];
        eq.filterPeak[0].dataIn[1] = eq.filterPeak[0].dataIn[0];

        eq.filterPeak[0].dataOut[2] = eq.filterPeak[0].dataOut[1];
        eq.filterPeak[0].dataOut[1] = eq.filterPeak[0].dataOut[0];

        // peak 2
        eq.filterPeak[1].dataIn[0] = eq.filterPeak[0].dataOut[0];
        if (eq.gainPeak[1] != kEqGainZero) {
            eq.filterPeak[1].dataOut[0] = (eq.filterPeak[1].a0 * eq.filterPeak[1].dataIn[0]) + (eq.filterPeak[1].a1 * eq.filterPeak[1].dataIn[1]) + (eq.filterPeak[1].a2 * eq.filterPeak[1].dataIn[2]) - (eq.filterPeak[1].b1 * eq.filterPeak[1].dataOut[1]) - (eq.filterPeak[1].b2 * eq.filterPeak[1].dataOut[2]);
        } else {
            eq.filterPeak[1].dataOut[0] = eq.filterPeak[1].dataIn[0];
        }
        eq.filterPeak[1].dataIn[2] = eq.filterPeak[1].dataIn[1];
        eq.filterPeak[1].dataIn[1] = eq.filterPeak[1].dataIn[0];

        eq.filterPeak[1].dataOut[2] = eq.filterPeak[1].dataOut[1];
        eq.filterPeak[1].dataOut[1] = eq.filterPeak[1].dataOut[0];

        // peak 3
        eq.filterPeak[2].dataIn[0] = eq.filterPeak[1].dataOut[0];
        if (eq.gainPeak[2] != kEqGainZero) {
            eq.filterPeak[2].dataOut[0] = (eq.filterPeak[2].a0 * eq.filterPeak[2].dataIn[0]) + (eq.filterPeak[2].a1 * eq.filterPeak[2].dataIn[1]) + (eq.filterPeak[2].a2 * eq.filterPeak[2].dataIn[2]) - (eq.filterPeak[2].b1 * eq.filterPeak[2].dataOut[1]) - (eq.filterPeak[2].b2 * eq.filterPeak[2].dataOut[2]);
        } else {
            eq.filterPeak[2].dataOut[0] = eq.filterPeak[2].dataIn[0];
        }
        eq.filterPeak[2].dataIn[2] = eq.filterPeak[2].dataIn[1];
        eq.filterPeak[2].dataIn[1] = eq.filterPeak[2].dataIn[0];

        eq.filterPeak[2].dataOut[2] = eq.filterPeak[2].dataOut[1];
        eq.filterPeak[2].dataOut[1] = eq.filterPeak[2].dataOut[0];

        // peak 4
        eq.filterPeak[3].dataIn[0] = eq.filterPeak[2].dataOut[0];
        if (eq.gainPeak[3] != kEqGainZero) {
            eq.filterPeak[3].dataOut[0] = (eq.filterPeak[3].a0 * eq.filterPeak[3].dataIn[0]) + (eq.filterPeak[3].a1 * eq.filterPeak[3].dataIn[1]) + (eq.filterPeak[3].a2 * eq.filterPeak[3].dataIn[2]) - (eq.filterPeak[3].b1 * eq.filterPeak[3].dataOut[1]) - (eq.filterPeak[3].b2 * eq.filterPeak[3].dataOut[2]);
        } else {
            eq.filterPeak[3].dataOut[0] = eq.filterPeak[3].dataIn[0];
        }
        eq.filterPeak[3].dataIn[2] = eq.filterPeak[3].dataIn[1];
        eq.filterPeak[3].dataIn[1] = eq.filterPeak[3].dataIn[0];

        eq.filterPeak[3].dataOut[2] = eq.filterPeak[3].dataOut[1];
        eq.filterPeak[3].dataOut[1] = eq.filterPeak[3].dataOut[0];

        output = eq.filterPeak[3].dataOut[0];
    } else {
        output = input;
    }

    if (eq.genTransition.active) {
        audioEq = (eq.genTransition.activeWet * output) + (eq.genTransition.activeDry * input);
    } else {
        audioEq = output;
    }
}

void Controller::interruptAudioFilter() {
    int32_t outputA = processAudioFilter(0, audioEq);
    int32_t outputB = processAudioFilter(1, outputA);
    audioFilter = outputB;
}

void Controller::interruptAudioEffect() {
    int32_t outputA = processAudioEffect(0, audioFilter);
    int32_t outputB = processAudioEffect(1, outputA);
    audioEffect = outputB;

    if ((audioEffect >= INT24_MAX) || (audioEffect <= INT24_MIN)) {
        limitAlertShowFlag = true;
        if ((system.limiter) && (!system.volumeTransition.active) && (system.volume > kMinSystemVolume)) {
            system_setVolume(system.volume - 1);
        }
    }
}

void Controller::interruptAudioReverb() {
    int32_t input = (audioEffect << 8);
    int32_t output_L;
    int32_t output_R;

    if ((reverb.active) || (reverb.genTransition.active)) {
        // preDelay

        reverb.preDelayBuffer[reverb.preDelay_recordInterval] = input;
        int32_t reverbInput = reverb.preDelayBuffer[reverb.preDelay_playInterval];

        if (++reverb.preDelay_playInterval > (reverb.kPreDelayBufferSize - 1))
            reverb.preDelay_playInterval = 0;
        if (++reverb.preDelay_recordInterval > (reverb.kPreDelayBufferSize - 1))
            reverb.preDelay_recordInterval = 0;

        // comb filter

        int32_t inputData = 0;
        int32_t combOutput = 0;

        inputData = reverbInput * 0.015;

        reverb.comb1Out = reverb.comb1Buffer[reverb.comb1Index];
        reverb.comb1Filter = (reverb.comb1Out * reverb.combDecay2) + (reverb.comb1Filter * reverb.combDecay1);
        reverb.comb1Buffer[reverb.comb1Index] = inputData + (reverb.comb1Filter * reverb.combFeedback);
        if (++reverb.comb1Index >= reverb.comb1Size)
            reverb.comb1Index = 0;
        combOutput += reverb.comb1Out;

        reverb.comb2Out = reverb.comb2Buffer[reverb.comb2Index];
        reverb.comb2Filter = (reverb.comb2Out * reverb.combDecay2) + (reverb.comb2Filter * reverb.combDecay1);
        reverb.comb2Buffer[reverb.comb2Index] = inputData + (reverb.comb2Filter * reverb.combFeedback);
        if (++reverb.comb2Index >= reverb.comb2Size)
            reverb.comb2Index = 0;
        combOutput += reverb.comb2Out;

        reverb.comb3Out = reverb.comb3Buffer[reverb.comb3Index];
        reverb.comb3Filter = (reverb.comb3Out * reverb.combDecay2) + (reverb.comb3Filter * reverb.combDecay1);
        reverb.comb3Buffer[reverb.comb3Index] = inputData + (reverb.comb3Filter * reverb.combFeedback);
        if (++reverb.comb3Index >= reverb.comb3Size)
            reverb.comb3Index = 0;
        combOutput += reverb.comb3Out;

        reverb.comb4Out = reverb.comb4Buffer[reverb.comb4Index];
        reverb.comb4Filter = (reverb.comb4Out * reverb.combDecay2) + (reverb.comb4Filter * reverb.combDecay1);
        reverb.comb4Buffer[reverb.comb4Index] = inputData + (reverb.comb4Filter * reverb.combFeedback);
        if (++reverb.comb4Index >= reverb.comb4Size)
            reverb.comb4Index = 0;
        combOutput += reverb.comb4Out;

        reverb.comb5Out = reverb.comb5Buffer[reverb.comb5Index];
        reverb.comb5Filter = (reverb.comb5Out * reverb.combDecay2) + (reverb.comb5Filter * reverb.combDecay1);
        reverb.comb5Buffer[reverb.comb5Index] = inputData + (reverb.comb5Filter * reverb.combFeedback);
        if (++reverb.comb5Index >= reverb.comb5Size)
            reverb.comb5Index = 0;
        combOutput += reverb.comb5Out;

        reverb.comb6Out = reverb.comb6Buffer[reverb.comb6Index];
        reverb.comb6Filter = (reverb.comb6Out * reverb.combDecay2) + (reverb.comb6Filter * reverb.combDecay1);
        reverb.comb6Buffer[reverb.comb6Index] = inputData + (reverb.comb6Filter * reverb.combFeedback);
        if (++reverb.comb6Index >= reverb.comb6Size)
            reverb.comb6Index = 0;
        combOutput += reverb.comb6Out;

        reverb.comb7Out = reverb.comb7Buffer[reverb.comb7Index];
        reverb.comb7Filter = (reverb.comb7Out * reverb.combDecay2) + (reverb.comb7Filter * reverb.combDecay1);
        reverb.comb7Buffer[reverb.comb7Index] = inputData + (reverb.comb7Filter * reverb.combFeedback);
        if (++reverb.comb7Index >= reverb.comb7Size)
            reverb.comb7Index = 0;
        combOutput += reverb.comb7Out;

        reverb.comb8Out = reverb.comb8Buffer[reverb.comb8Index];
        reverb.comb8Filter = (reverb.comb8Out * reverb.combDecay2) + (reverb.comb8Filter * reverb.combDecay1);
        reverb.comb8Buffer[reverb.comb8Index] = inputData + (reverb.comb8Filter * reverb.combFeedback);
        if (++reverb.comb8Index >= reverb.comb8Size)
            reverb.comb8Index = 0;
        combOutput += reverb.comb8Out;

        // allpass filter

        reverb.apass1Out = reverb.apass1Buffer[reverb.apass1Index] - combOutput;
        reverb.apass1Buffer[reverb.apass1Index] = combOutput + (reverb.apass1Buffer[reverb.apass1Index] * reverb.apassFeedback);
        if (++reverb.apass1Index >= reverb.apass1Size)
            reverb.apass1Index = 0;

        reverb.apass2Out = reverb.apass2Buffer[reverb.apass2Index] - reverb.apass1Out;
        reverb.apass2Buffer[reverb.apass2Index] = reverb.apass1Out + (reverb.apass2Buffer[reverb.apass2Index] * reverb.apassFeedback);
        if (++reverb.apass2Index >= reverb.apass2Size)
            reverb.apass2Index = 0;

        reverb.apass3Out = reverb.apass3Buffer[reverb.apass3Index] - reverb.apass2Out;
        reverb.apass3Buffer[reverb.apass3Index] = reverb.apass2Out + (reverb.apass3Buffer[reverb.apass3Index] * reverb.apassFeedback);
        if (++reverb.apass3Index >= reverb.apass3Size)
            reverb.apass3Index = 0;

        reverb.apass4Out = reverb.apass4Buffer[reverb.apass4Index] - reverb.apass3Out;
        reverb.apass4Buffer[reverb.apass4Index] = reverb.apass3Out + (reverb.apass4Buffer[reverb.apass4Index] * reverb.apassFeedback);
        if (++reverb.apass4Index >= reverb.apass4Size)
            reverb.apass4Index = 0;

        int32_t data = (reverb.apass4Out * reverb.wetFloat) + (input * reverb.dryFloat);

        int32_t audioReverb;

        if (reverb.genTransition.active) {
            audioReverb = (data * reverb.genTransition.activeWet) + (input * reverb.genTransition.activeDry);
        } else {
            audioReverb = data;
        }

        // surround

        if ((reverb.genTransition.active) && ((reverb.genTransition.mode == REV_MODE_ACTIVE) || (reverb.genTransition.mode == REV_MODE_SURROUND))) {
            reverb.surroundBuffer[reverb.surround_recordInterval] = audioReverb * reverb.genTransition.activeWet;
            output_L = audioReverb;
            output_R = (reverb.surroundBuffer[reverb.surround_playInterval] * reverb.genTransition.activeWet) + (audioReverb * reverb.genTransition.activeDry);
        } else {
            reverb.surroundBuffer[reverb.surround_recordInterval] = audioReverb;
            output_L = audioReverb;
            output_R = reverb.surroundBuffer[reverb.surround_playInterval];
        }

        if (++reverb.surround_playInterval > (reverb.kSurroundBufferSize - 1))
            reverb.surround_playInterval = 0;
        if (++reverb.surround_recordInterval > (reverb.kSurroundBufferSize - 1))
            reverb.surround_recordInterval = 0;
    } else {
        // surround
        output_L = input;
        output_R = input;
    }

    audioReverb_L = output_L;
    audioReverb_R = output_R;
}

void Controller::interruptAudioSend() {
    audioSend_L = (audioReverb_L + audioMetronome) * system.volumeLeftFloat;
    audioSend_R = (audioReverb_R + audioMetronome) * system.volumeRightFloat;

    uint16_t audio_L0 = (uint16_t)((audioSend_L >> 16) & (0xFFFF));
    uint16_t audio_L1 = (uint16_t)(audioSend_L & 0xFFFF);

    uint16_t audio_R0 = (uint16_t)((audioSend_R >> 16) & (0xFFFF));
    uint16_t audio_R1 = (uint16_t)(audioSend_R & 0xFFFF);

    dac.i2s_data[0] = audio_L0;
    dac.i2s_data[1] = audio_L1;

    dac.i2s_data[2] = audio_R0;
    dac.i2s_data[3] = audio_R1;
}

int32_t Controller::processAudioFilter(uint8_t filterNum_, int32_t audio_) {
    Filter &filter_ = filter[filterNum_];
    int32_t input = audio_;
    int32_t output;

    if ((filter_.active) || (filter_.genTransition.active)) {
        filter_.dataIn[0] = input;
        filter_.dataOut[0] = (filter_.a0 * filter_.dataIn[0]) + (filter_.a1 * filter_.dataIn[1]) + (filter_.a2 * filter_.dataIn[2]) - (filter_.b1 * filter_.dataOut[1]) - (filter_.b2 * filter_.dataOut[2]);

        filter_.dataIn[2] = filter_.dataIn[1];
        filter_.dataIn[1] = filter_.dataIn[0];

        filter_.dataOut[2] = filter_.dataOut[1];
        filter_.dataOut[1] = filter_.dataOut[0];

        int32_t data = (filter_.dataOut[0] * filter_.wetFloat) + (input * filter_.dryFloat);

        if (filter_.slope == 0)
            data = (data / 2) + (input / 2);

        if (filter_.genTransition.active) {
            output = (data * filter_.genTransition.activeWet) + (input * filter_.genTransition.activeDry);
        } else {
            output = data;
        }
    } else {
        output = input;
    }

    return output;
}

int32_t Controller::processAudioEffect(uint8_t effectNum_, int32_t audio_) {
    Effect &effect_ = effect[effectNum_];
    int32_t input = audio_;
    int32_t output;

    if ((effect_.active) || (effect_.genTransition.active)) {
        // type selection
        uint8_t type;
        (effect_.genTransition.active) ? type = effect_.genTransition.activeType : type = effect_.type;

        // delay

        if (type == EF_DELAY) {
            Delay &delay_ = effect_.delay;

            volatile int32_t *playPtr = (volatile int32_t *)(effect_.delayAddress + (delay_.playInterval * 4));
            volatile int32_t *recordPtr = (volatile int32_t *)(effect_.delayAddress + (delay_.recordInterval * 4));

            int32_t playData = input + *playPtr;

            *recordPtr = (input * delay_.level) + (*playPtr * delay_.feedback);

            int32_t data = (playData * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }

            if (++delay_.playInterval > (kDelaySize - 1))
                delay_.playInterval = 0;
            if (++delay_.recordInterval > (kDelaySize - 1))
                delay_.recordInterval = 0;
        }

        // chorus

        else if (type == EF_CHORUS) {
            Chorus &chorus_ = effect_.chorus;

            int32_t dataChorus;
            int32_t dataChorusDelay[2];

            for (uint8_t i = 0; i < 2; i++) {
                ChorusDelay &cD = chorus_.chorusDelay[i];

                cD.chorusInterval = cD.playInterval + cD.shiftInterval;

                if (cD.chorusInterval < 0) {
                    cD.chorusInterval += kChorusBufferSize;
                } else if (cD.chorusInterval > kChorusBufferSize) {
                    cD.chorusInterval -= kChorusBufferSize;
                }

                uint16_t interval_Int0 = (uint16_t)cD.chorusInterval;
                uint16_t interval_Int1 = interval_Int0 + 1;
                float remainder = cD.chorusInterval - interval_Int0;

                if (interval_Int1 == kChorusBufferSize)
                    interval_Int1 = 0;

                volatile int32_t *dataPtr0 = (volatile int32_t *)(effect_.chorusAddress + (interval_Int0 * 4));
                volatile int32_t *dataPtr1 = (volatile int32_t *)(effect_.chorusAddress + (interval_Int1 * 4));
                dataChorusDelay[i] = *dataPtr0 + ((*dataPtr1 - *dataPtr0) * remainder);

                cD.shiftInterval += cD.shiftInc;

                if (cD.shiftInterval <= cD.shiftMin) {
                    cD.shiftInc *= -1;
                    cD.shiftInterval = cD.shiftMin;
                }

                if (cD.shiftInterval >= cD.shiftMax) {
                    cD.shiftInc *= -1;
                    cD.shiftInterval = cD.shiftMax;
                }
            }

            dataChorus = (dataChorusDelay[0] * chorus_.chorusDelay[0].mix) + (dataChorusDelay[1] * chorus_.chorusDelay[1].mix);

            volatile int32_t *recordPtr = (volatile int32_t *)(effect_.chorusAddress + (chorus_.recordInterval * 4));

            if (effect_.genTransition.active) {
                *recordPtr = (input + (dataChorus * chorus_.feedback)) * effect_.genTransition.activeRecordWet;
            } else {
                *recordPtr = input + (dataChorus * chorus_.feedback);
            }

            int32_t data = (dataChorus * effect_.wetFloat) + (*recordPtr * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }

            if ((++chorus_.recordInterval) > (kChorusBufferSize - 1))
                chorus_.recordInterval = 0;
            if ((++chorus_.chorusDelay[0].playInterval) > (kChorusBufferSize - 1))
                chorus_.chorusDelay[0].playInterval = 0;
            if ((++chorus_.chorusDelay[1].playInterval) > (kChorusBufferSize - 1))
                chorus_.chorusDelay[1].playInterval = 0;
        }

        // flanger

        else if (type == EF_FLANGER) {
            Flanger &flanger_ = effect_.flanger;

            int32_t dataFlanger;

            flanger_.flangerInterval = flanger_.playInterval + flanger_.shiftInterval;

            if (flanger_.flangerInterval < 0) {
                flanger_.flangerInterval += kFlangerBufferSize;
            } else if (flanger_.flangerInterval > kFlangerBufferSize) {
                flanger_.flangerInterval -= kFlangerBufferSize;
            }

            uint16_t interval_Int0 = (uint16_t)flanger_.flangerInterval;
            uint16_t interval_Int1 = interval_Int0 + 1;
            float remainder = flanger_.flangerInterval - interval_Int0;

            if (interval_Int1 == kFlangerBufferSize)
                interval_Int1 = 0;

            int32_t data0 = effect_.flangerBuffer[interval_Int0];
            int32_t data1 = effect_.flangerBuffer[interval_Int1];
            dataFlanger = data0 + ((data1 - data0) * remainder);

            if (effect_.genTransition.active) {
                effect_.flangerBuffer[flanger_.recordInterval] = (input + (dataFlanger * flanger_.feedback)) * effect_.genTransition.activeRecordWet;
            } else {
                effect_.flangerBuffer[flanger_.recordInterval] = input + (dataFlanger * flanger_.feedback);
            }

            int32_t data = (dataFlanger * effect_.wetFloat) + (effect_.flangerBuffer[flanger_.recordInterval] * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }

            flanger_.shiftInterval += flanger_.shiftInc;

            if (flanger_.shiftInterval <= flanger_.shiftMin) {
                flanger_.shiftInc *= -1;
                flanger_.shiftInterval = flanger_.shiftMin;
            }

            if (flanger_.shiftInterval >= flanger_.shiftMax) {
                flanger_.shiftInc *= -1;
                flanger_.shiftInterval = flanger_.shiftMax;
            }

            if ((++flanger_.recordInterval) > (kFlangerBufferSize - 1))
                flanger_.recordInterval = 0;
            if ((++flanger_.playInterval) > (kFlangerBufferSize - 1))
                flanger_.playInterval = 0;
        }

        // phaser

        else if (type == EF_PHASER) {
            Phaser &phaser_ = effect_.phaser;

            phaser_.lfo = phaser_.depthFreq * sin(2 * M_PI * phaser_.dataY) + phaser_.centerFreq;

            phaser_.dataX += phaser_.Ts;
            phaser_.dataY = phaser_.rate * phaser_.dataX;
            if (phaser_.dataY >= 1.0)
                phaser_.dataX = 0;

            float w0 = 2 * M_PI * phaser_.lfo / kAudioSampleRate;
            float cosw0 = cos(w0);
            float sinw0 = sin(w0);
            float alpha = sinw0 / (2 * phaser_.Q);

            float b0 = 1.0 - alpha;
            float b1 = -2.0 * cosw0;
            float b2 = 1.0 + alpha;

            float a0 = 1.0 + alpha;
            float a1 = -2.0 * cosw0;
            float a2 = 1.0 - alpha;

            int32_t dataPhaser = ((b0 / a0) * input) + ((b1 / a0) * phaser_.ff[0]) + ((b2 / a0) * phaser_.ff[1]) - ((a1 / a0) * phaser_.fb[0]) - ((a2 / a0) * phaser_.fb[1]);

            int32_t data = (dataPhaser * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }

            phaser_.ff[1] = phaser_.ff[0];
            phaser_.ff[0] = input;
            phaser_.fb[1] = phaser_.fb[0];
            phaser_.fb[0] = dataPhaser;
        }

        // compressor

        else if (type == EF_COMPRESSOR) {
            Compressor &compressor_ = effect_.compressor;

            float in_float = (float)(input) / INT24_MAX;
            float in_abs = abs(in_float);
            float in_dB = 20.0 * log10(in_abs / 1.0);

            float gain;
            float gain_dB;
            float gainSmooth;

            if (in_dB < -96.0)
                in_dB = -96.0;
            if (in_dB > compressor_.threshold) {
                gain = compressor_.threshold + (in_dB - compressor_.threshold) / compressor_.rate;
            } else {
                gain = in_dB;
            }
            gain_dB = gain - in_dB;

            if (gain_dB < compressor_.gainSmoothPrev) {
                // attack mode
                gainSmooth = ((1 - compressor_.attackAlpha) * gain_dB) + (compressor_.attackAlpha * compressor_.gainSmoothPrev);
            } else {
                // release mode
                gainSmooth = ((1 - compressor_.releaseAlpha) * gain_dB) + (compressor_.releaseAlpha * compressor_.gainSmoothPrev);
            }
            compressor_.gainSmoothPrev = gainSmooth;

            float amp = pow(10.0, gainSmooth / 20.0);

            int32_t data = (amp * input * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }
        }

        // expander

        else if (type == EF_EXPANDER) {
            Expander &expander_ = effect_.expander;

            float in_float = (float)(input) / INT24_MAX;
            float in_abs = abs(in_float);
            float in_dB = 20.0 * log10(in_abs / 1.0);

            float gain;
            float gain_dB;
            float gainSmooth;

            if (in_dB < -144.0)
                in_dB = -144.0;
            if (in_dB > expander_.threshold) {
                gain = in_dB;
            } else {
                gain = expander_.threshold + (in_dB - expander_.threshold) * expander_.rate;
            }
            gain_dB = gain - in_dB;

            if (gain_dB > expander_.gainSmoothPrev) {
                // attack mode
                gainSmooth = ((1 - expander_.attackAlpha) * gain_dB) + (expander_.attackAlpha * expander_.gainSmoothPrev);
            } else {
                // release mode
                gainSmooth = ((1 - expander_.releaseAlpha) * gain_dB) + (expander_.releaseAlpha * expander_.gainSmoothPrev);
            }
            expander_.gainSmoothPrev = gainSmooth;

            float amp = pow(10.0, gainSmooth / 20.0);

            int32_t data = (amp * input * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }
        }

        // overdrive

        else if (type == EF_OVERDRIVE) {
            Overdrive &overdrive_ = effect_.overdrive;

            double in_double = ((double)(input) / INT24_MAX) * overdrive_.gain;
            double out_double;
            if (in_double < -overdrive_.threshold) {
                out_double = -2 * overdrive_.threshold / 3;
            } else if (in_double > overdrive_.threshold) {
                out_double = 2 * overdrive_.threshold / 3;
            } else {
                out_double = in_double - (in_double * in_double * in_double) / 3;
            }

            if (out_double > 1.0)
                out_double = 1.0;
            if (out_double < -1.0)
                out_double = -1.0;

            int32_t filterInput = out_double * INT24_MAX;
            int32_t filterOutput;

            overdrive_.dataIn[0] = filterInput;
            overdrive_.dataOut[0] = (overdrive_.a0 * overdrive_.dataIn[0]) + (overdrive_.a1 * overdrive_.dataIn[1]) + (overdrive_.a2 * overdrive_.dataIn[2]) - (overdrive_.b1 * overdrive_.dataOut[1]) - (overdrive_.b2 * overdrive_.dataOut[2]);

            overdrive_.dataIn[2] = overdrive_.dataIn[1];
            overdrive_.dataIn[1] = overdrive_.dataIn[0];

            overdrive_.dataOut[2] = overdrive_.dataOut[1];
            overdrive_.dataOut[1] = overdrive_.dataOut[0];

            filterOutput = overdrive_.dataOut[0];

            int32_t data = (filterOutput * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }
        }

        // distortion

        else if (type == EF_DISTORTION) {
            Distortion &distortion_ = effect_.distortion;

            float in_float = ((float)(input) / INT24_MAX) * distortion_.gain;

            // float out_float = (2.0 / M_PI) * atan(in_float) * distortion_.threshold;
            // float out_float = copysign(1.0, in_float) * (1 - exp(-abs(distortion_.gain * in_float))) * distortion_.threshold;
            // float out_float = tanh(distortion_.gain * in_float) * distortion_.threshold;
            // float out_float = (2.0f / distortion_.gain) * tanhf(distortion_.gain * in_float) * distortion_.threshold;
            float out_float = (2.0f / distortion_.gain) * atanf(distortion_.gain * in_float) * distortion_.threshold;

            int32_t filterInput = out_float * INT24_MAX;
            int32_t filterOutput;

            distortion_.dataIn[0] = filterInput;
            distortion_.dataOut[0] = (distortion_.a0 * distortion_.dataIn[0]) + (distortion_.a1 * distortion_.dataIn[1]) + (distortion_.a2 * distortion_.dataIn[2]) - (distortion_.b1 * distortion_.dataOut[1]) - (distortion_.b2 * distortion_.dataOut[2]);

            distortion_.dataIn[2] = distortion_.dataIn[1];
            distortion_.dataIn[1] = distortion_.dataIn[0];

            distortion_.dataOut[2] = distortion_.dataOut[1];
            distortion_.dataOut[1] = distortion_.dataOut[0];

            filterOutput = distortion_.dataOut[0];

            int32_t data = (filterOutput * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }
        }

        // bitcrusher

        else if (type == EF_BITCRUSHER) {
            Bitcrusher &bitcrusher_ = effect_.bitcrusher;

            bitcrusher_.sampleCounter += 1;
            if (bitcrusher_.sampleCounter >= bitcrusher_.sampleCounterMax) {
                bitcrusher_.sampleCounter = 0;

                bitcrusher_.sampleData = input;

                while ((bitcrusher_.sampleData < bitcrusher_.limitNeg) || (bitcrusher_.sampleData > bitcrusher_.limitPos)) {
                    if (bitcrusher_.sampleData > bitcrusher_.limitPos) {
                        switch (bitcrusher_.mode) {
                        case BIT_CLIP:
                            bitcrusher_.sampleData = bitcrusher_.limitPos;
                            break;

                        case BIT_FOLD:
                            bitcrusher_.sampleData = bitcrusher_.limitPos - (bitcrusher_.sampleData - bitcrusher_.limitPos);
                            if (bitcrusher_.sampleData < 0)
                                bitcrusher_.sampleData *= -1;
                            break;
                        }
                    } else if (bitcrusher_.sampleData < bitcrusher_.limitNeg) {
                        switch (bitcrusher_.mode) {
                        case BIT_CLIP:
                            bitcrusher_.sampleData = bitcrusher_.limitNeg;
                            break;

                        case BIT_FOLD:
                            bitcrusher_.sampleData = bitcrusher_.limitNeg - (bitcrusher_.sampleData - bitcrusher_.limitNeg);
                            if (bitcrusher_.sampleData > 0)
                                bitcrusher_.sampleData *= -1;
                            break;
                        }
                    }
                }
            }

            int32_t dataOut = bitcrusher_.sampleData & bitcrusher_.resModifier;

            int32_t data = (dataOut * effect_.wetFloat) + (input * effect_.dryFloat);

            if (effect_.genTransition.active) {
                output = (data * effect_.genTransition.activeWet) + (input * effect_.genTransition.activeDry);
            } else {
                output = data;
            }
        }
    } else {
        output = input;
    }

    return output;
}

void Controller::interruptTransition() {
    // system transition
    if (system.volumeTransition.active) {
        SystemVolumeTransition &vTransition = system.volumeTransition;

        switch (vTransition.actionVolume) {
        case SYS_ACTION_UP:
            if (system.volumeFloat >= vTransition.targetVolume) {
                vTransition.actionVolume = SYS_ACTION_NONE;
            } else {
                system.volumeFloat += 0.0001;
            }
            break;

        case SYS_ACTION_DOWN:
            if (system.volumeFloat <= vTransition.targetVolume) {
                vTransition.actionVolume = SYS_ACTION_NONE;
            } else {
                system.volumeFloat -= 0.0001;
            }
            break;
        }

        if (vTransition.actionVolume == SYS_ACTION_NONE) {
            vTransition.active = false;
        }
    }

    if (system.panTransition.active) {
        SystemPanTransition &pTransition = system.panTransition;

        switch (pTransition.actionVolumeLeft) {
        case SYS_ACTION_UP:
            if (system.volumeLeftFloat >= pTransition.targetVolumeLeft) {
                pTransition.actionVolumeLeft = SYS_ACTION_NONE;
            } else {
                system.volumeLeftFloat += 0.0001;
            }
            break;

        case SYS_ACTION_DOWN:
            if (system.volumeLeftFloat <= pTransition.targetVolumeLeft) {
                pTransition.actionVolumeLeft = SYS_ACTION_NONE;
            } else {
                system.volumeLeftFloat -= 0.0001;
            }
            break;
        }

        switch (pTransition.actionVolumeRight) {
        case SYS_ACTION_UP:
            if (system.volumeRightFloat >= pTransition.targetVolumeRight) {
                pTransition.actionVolumeRight = SYS_ACTION_NONE;
            } else {
                system.volumeRightFloat += 0.0001;
            }
            break;

        case SYS_ACTION_DOWN:
            if (system.volumeRightFloat <= pTransition.targetVolumeRight) {
                pTransition.actionVolumeRight = SYS_ACTION_NONE;
            } else {
                system.volumeRightFloat -= 0.0001;
            }
            break;
        }

        if ((pTransition.actionVolumeLeft == SYS_ACTION_NONE) && (pTransition.actionVolumeRight == SYS_ACTION_NONE)) {
            pTransition.active = false;
        }
    }

    // metronome transition
    if (metronome.volumeTransition.active) {
        MetronomeVolumeTransition &vTransition = metronome.volumeTransition;

        switch (vTransition.actionVolume) {
        case MET_ACTION_UP:
            if (metronome.volumeFloat >= vTransition.targetVolume) {
                vTransition.actionVolume = MET_ACTION_NONE;
            } else {
                metronome.volumeFloat += 0.0001;
            }
            break;

        case MET_ACTION_DOWN:
            if (metronome.volumeFloat <= vTransition.targetVolume) {
                vTransition.actionVolume = MET_ACTION_NONE;
            } else {
                metronome.volumeFloat -= 0.0001;
            }
            break;
        }

        if (vTransition.actionVolume == MET_ACTION_NONE) {
            vTransition.active = false;
        }
    }

    // eq transition
    if (eq.genTransition.active) {
        EqGenTransition &gTransition = eq.genTransition;

        switch (gTransition.actionDry) {
        case EQ_ACTION_UP:
            if (gTransition.activeDry >= gTransition.targetDry) {
                gTransition.actionDry = EQ_ACTION_NONE;
            } else {
                gTransition.activeDry += 0.0001;
            }
            break;

        case EQ_ACTION_DOWN:
            if (gTransition.activeDry <= gTransition.targetDry) {
                gTransition.actionDry = EQ_ACTION_NONE;
            } else {
                gTransition.activeDry -= 0.0001;
            }
            break;
        }

        switch (gTransition.actionWet) {
        case EQ_ACTION_UP:
            if (gTransition.activeWet >= gTransition.targetWet) {
                gTransition.actionWet = EQ_ACTION_NONE;
            } else {
                gTransition.activeWet += 0.0001;
            }
            break;

        case EQ_ACTION_DOWN:
            if (gTransition.activeWet <= gTransition.targetWet) {
                gTransition.actionWet = EQ_ACTION_NONE;
            } else {
                gTransition.activeWet -= 0.0001;
            }
            break;
        }

        if ((gTransition.actionDry == EQ_ACTION_NONE) && (gTransition.actionWet == EQ_ACTION_NONE)) {
            switch (gTransition.mode) {
            case EQ_MODE_NONE:
                gTransition.active = false;
                gTransition.mode = EQ_MODE_NONE;
                gTransition.phase = EQ_PHASE_NONE;
                transitionClearFlag = true;
                break;

            case EQ_MODE_ACTIVE:
                gTransition.active = false;
                gTransition.mode = EQ_MODE_NONE;
                gTransition.phase = EQ_PHASE_NONE;
                transitionClearFlag = true;
                break;
            }
        }
    }

    // filter transition
    for (uint8_t i = 0; i < kFilterLibrarySize; i++) {
        if (filter[i].genTransition.active) {
            FilterGenTransition &gTransition = filter[i].genTransition;

            switch (gTransition.actionDry) {
            case FIL_ACTION_UP:
                if (gTransition.activeDry >= gTransition.targetDry) {
                    gTransition.actionDry = FIL_ACTION_NONE;
                } else {
                    gTransition.activeDry += 0.0001;
                }
                break;

            case FIL_ACTION_DOWN:
                if (gTransition.activeDry <= gTransition.targetDry) {
                    gTransition.actionDry = FIL_ACTION_NONE;
                } else {
                    gTransition.activeDry -= 0.0001;
                }
                break;
            }

            switch (gTransition.actionWet) {
            case FIL_ACTION_UP:
                if (gTransition.activeWet >= gTransition.targetWet) {
                    gTransition.actionWet = FIL_ACTION_NONE;
                } else {
                    gTransition.activeWet += 0.0001;
                }
                break;

            case FIL_ACTION_DOWN:
                if (gTransition.activeWet <= gTransition.targetWet) {
                    gTransition.actionWet = FIL_ACTION_NONE;
                } else {
                    gTransition.activeWet -= 0.0001;
                }
                break;
            }

            if ((gTransition.actionDry == FIL_ACTION_NONE) && (gTransition.actionWet == FIL_ACTION_NONE)) {
                switch (gTransition.mode) {
                case FIL_MODE_NONE:
                    gTransition.active = false;
                    gTransition.mode = FIL_MODE_NONE;
                    gTransition.phase = FIL_PHASE_NONE;
                    transitionClearFlag = true;
                    break;

                case FIL_MODE_ACTIVE:
                    gTransition.active = false;
                    gTransition.mode = FIL_MODE_NONE;
                    gTransition.phase = FIL_PHASE_NONE;
                    transitionClearFlag = true;
                    break;

                case FIL_MODE_TYPE:
                    switch (gTransition.phase) {
                    case EF_PHASE_NONE:
                        gTransition.active = false;
                        gTransition.mode = FIL_MODE_NONE;
                        gTransition.phase = FIL_PHASE_NONE;
                        transitionClearFlag = true;
                        break;

                    case EF_PHASE_A:
                        filter[i].calculateCoef();

                        gTransition.phase = FIL_PHASE_B;

                        gTransition.activeDry = 1.0;
                        gTransition.targetDry = 0.0;

                        gTransition.activeWet = 0.0;
                        gTransition.targetWet = 1.0;

                        filter_calculateGenTransition(i);

                        transitionShowFlag = 2;
                        break;

                    case EF_PHASE_B:
                        gTransition.active = false;
                        gTransition.mode = FIL_MODE_NONE;
                        gTransition.phase = FIL_PHASE_NONE;
                        transitionClearFlag = true;
                        break;
                    }
                    break;
                }
            }
        }

        if (filter[i].mixTransition.active) {
            Filter &filter_ = filter[i];
            FilterMixTransition &mTransition = filter[i].mixTransition;

            switch (mTransition.actionDry) {
            case FIL_ACTION_UP:
                if (filter_.dryFloat >= mTransition.targetDry) {
                    mTransition.actionDry = FIL_ACTION_NONE;
                } else {
                    filter_.dryFloat += 0.0001;
                }
                break;

            case FIL_ACTION_DOWN:
                if (filter_.dryFloat <= mTransition.targetDry) {
                    mTransition.actionDry = FIL_ACTION_NONE;
                } else {
                    filter_.dryFloat -= 0.0001;
                }
                break;
            }

            switch (mTransition.actionWet) {
            case FIL_ACTION_UP:
                if (filter_.wetFloat >= mTransition.targetWet) {
                    mTransition.actionWet = FIL_ACTION_NONE;
                } else {
                    filter_.wetFloat += 0.0001;
                }
                break;

            case FIL_ACTION_DOWN:
                if (filter_.wetFloat <= mTransition.targetWet) {
                    mTransition.actionWet = FIL_ACTION_NONE;
                } else {
                    filter_.wetFloat -= 0.0001;
                }
                break;
            }

            if ((mTransition.actionDry == FIL_ACTION_NONE) && (mTransition.actionWet == FIL_ACTION_NONE)) {
                mTransition.active = false;
            }
        }
    }

    // effect transition
    for (uint8_t i = 0; i < kEffectLibrarySize; i++) {
        if (effect[i].genTransition.active) {
            EffectGenTransition &gTransition = effect[i].genTransition;

            switch (gTransition.actionDry) {
            case EF_ACTION_UP:
                if (gTransition.activeDry >= gTransition.targetDry) {
                    gTransition.actionDry = EF_ACTION_NONE;
                } else {
                    gTransition.activeDry += 0.0001;
                }
                break;

            case EF_ACTION_DOWN:
                if (gTransition.activeDry <= gTransition.targetDry) {
                    gTransition.actionDry = EF_ACTION_NONE;
                } else {
                    gTransition.activeDry -= 0.0001;
                }
                break;
            }

            switch (gTransition.actionWet) {
            case EF_ACTION_UP:
                if (gTransition.activeWet >= gTransition.targetWet) {
                    gTransition.actionWet = EF_ACTION_NONE;
                } else {
                    gTransition.activeWet += 0.0001;
                }
                break;

            case EF_ACTION_DOWN:
                if (gTransition.activeWet <= gTransition.targetWet) {
                    gTransition.actionWet = EF_ACTION_NONE;
                } else {
                    gTransition.activeWet -= 0.0001;
                }
                break;
            }

            switch (gTransition.actionRecordWet) {
            case EF_ACTION_UP:
                if (gTransition.activeRecordWet >= gTransition.targetRecordWet) {
                    gTransition.actionRecordWet = EF_ACTION_NONE;
                } else {
                    gTransition.activeRecordWet += 0.0001;
                }
                break;

            case EF_ACTION_DOWN:
                if (gTransition.activeRecordWet <= gTransition.targetRecordWet) {
                    gTransition.actionRecordWet = EF_ACTION_NONE;
                } else {
                    gTransition.activeRecordWet -= 0.0001;
                }
                break;
            }

            if ((gTransition.actionDry == EF_ACTION_NONE) && (gTransition.actionWet == EF_ACTION_NONE) && (gTransition.actionRecordWet == EF_ACTION_NONE)) {
                switch (gTransition.mode) {
                case EF_MODE_NONE:
                    gTransition.active = false;
                    gTransition.mode = EF_MODE_NONE;
                    gTransition.phase = EF_PHASE_NONE;
                    transitionClearFlag = true;
                    break;

                case EF_MODE_ACTIVE:
                    gTransition.active = false;
                    gTransition.mode = EF_MODE_NONE;
                    gTransition.phase = EF_PHASE_NONE;
                    transitionClearFlag = true;
                    break;

                case EF_MODE_TYPE:
                case EF_MODE_TIME:
                case EF_MODE_FEEDBACK:
                    switch (gTransition.phase) {
                    case EF_PHASE_NONE:
                        gTransition.active = false;
                        gTransition.mode = EF_MODE_NONE;
                        gTransition.phase = EF_PHASE_NONE;
                        transitionClearFlag = true;
                        break;

                    case EF_PHASE_A:
                        switch (gTransition.mode) {
                        case EF_MODE_TYPE:
                            gTransition.activeType = gTransition.targetType;
                            break;

                        case EF_MODE_TIME:
                            switch (effect[i].type) {
                            case EF_DELAY:
                                effect[i].delay.time = kDelayTimeDataLibrary[effect[i].delay.aTime].data;
                                effect[i].delay.update(rhythm.tempo);
                                break;

                            case EF_CHORUS:
                                effect[i].chorus.time = kChorusTimeDataLibrary[effect[i].chorus.aTime].data;
                                effect[i].chorus.update();
                                break;

                            case EF_FLANGER:
                                effect[i].flanger.time = kFlangerTimeDataLibrary[effect[i].flanger.aTime].data;
                                effect[i].flanger.update();
                                break;
                            }
                            break;

                        case EF_MODE_FEEDBACK:
                            switch (effect[i].type) {
                            case EF_CHORUS:
                                effect[i].chorus.feedback = kChorusFeedbackDataLibrary[effect[i].chorus.bFeedback].data;
                                effect[i].chorus.update();
                                break;

                            case EF_FLANGER:
                                effect[i].flanger.feedback = kFlangerFeedbackDataLibrary[effect[i].flanger.bFeedback].data;
                                effect[i].flanger.update();
                                break;
                            }
                            break;
                        }

                        gTransition.phase = EF_PHASE_B;

                        gTransition.activeDry = 1.0;
                        gTransition.targetDry = 0.0;

                        gTransition.activeWet = 0.0;
                        gTransition.targetWet = 1.0;

                        gTransition.activeRecordWet = 0.0;
                        gTransition.targetRecordWet = 1.0;

                        effect_calculateGenTransition(i);

                        transitionShowFlag = 2;
                        break;

                    case EF_PHASE_B:
                        gTransition.active = false;
                        gTransition.mode = EF_MODE_NONE;
                        gTransition.phase = EF_PHASE_NONE;
                        transitionClearFlag = true;
                        break;
                    }
                    break;
                }
            }
        }

        if (effect[i].mixTransition.active) {
            Effect &effect_ = effect[i];
            EffectMixTransition &mTransition = effect[i].mixTransition;

            switch (mTransition.actionDry) {
            case EF_ACTION_UP:
                if (effect_.dryFloat >= mTransition.targetDry) {
                    mTransition.actionDry = EF_ACTION_NONE;
                } else {
                    effect_.dryFloat += 0.0001;
                }
                break;

            case EF_ACTION_DOWN:
                if (effect_.dryFloat <= mTransition.targetDry) {
                    mTransition.actionDry = EF_ACTION_NONE;
                } else {
                    effect_.dryFloat -= 0.0001;
                }
                break;
            }

            switch (mTransition.actionWet) {
            case EF_ACTION_UP:
                if (effect_.wetFloat >= mTransition.targetWet) {
                    mTransition.actionWet = EF_ACTION_NONE;
                } else {
                    effect_.wetFloat += 0.0001;
                }
                break;

            case EF_ACTION_DOWN:
                if (effect_.wetFloat <= mTransition.targetWet) {
                    mTransition.actionWet = EF_ACTION_NONE;
                } else {
                    effect_.wetFloat -= 0.0001;
                }
                break;
            }

            if ((mTransition.actionDry == EF_ACTION_NONE) && (mTransition.actionWet == EF_ACTION_NONE)) {
                mTransition.active = false;
            }
        }
    }

    // reverb transition
    if (reverb.genTransition.active) {
        ReverbGenTransition &gTransition = reverb.genTransition;

        switch (gTransition.actionDry) {
        case REV_ACTION_UP:
            if (gTransition.activeDry >= gTransition.targetDry) {
                gTransition.actionDry = REV_ACTION_NONE;
            } else {
                gTransition.activeDry += 0.0001;
            }
            break;

        case REV_ACTION_DOWN:
            if (gTransition.activeDry <= gTransition.targetDry) {
                gTransition.actionDry = REV_ACTION_NONE;
            } else {
                gTransition.activeDry -= 0.0001;
            }
            break;
        }

        switch (gTransition.actionWet) {
        case REV_ACTION_UP:
            if (gTransition.activeWet >= gTransition.targetWet) {
                gTransition.actionWet = REV_ACTION_NONE;
            } else {
                gTransition.activeWet += 0.0001;
            }
            break;

        case REV_ACTION_DOWN:
            if (gTransition.activeWet <= gTransition.targetWet) {
                gTransition.actionWet = REV_ACTION_NONE;
            } else {
                gTransition.activeWet -= 0.0001;
            }
            break;
        }

        if ((gTransition.actionDry == REV_ACTION_NONE) && (gTransition.actionWet == REV_ACTION_NONE)) {
            switch (gTransition.mode) {
            case REV_MODE_NONE:
                gTransition.active = false;
                gTransition.mode = REV_MODE_NONE;
                gTransition.phase = REV_PHASE_NONE;
                transitionClearFlag = true;
                break;

            case REV_MODE_ACTIVE:
                gTransition.active = false;
                gTransition.mode = REV_MODE_NONE;
                gTransition.phase = REV_PHASE_NONE;
                transitionClearFlag = true;
                break;

            case REV_MODE_PREDELAY:
            case REV_MODE_SURROUND:
                switch (gTransition.phase) {
                case REV_PHASE_NONE:
                    gTransition.active = false;
                    gTransition.mode = REV_MODE_NONE;
                    gTransition.phase = REV_PHASE_NONE;
                    transitionClearFlag = true;
                    break;

                case REV_PHASE_A:
                    switch (gTransition.mode) {
                    case REV_MODE_PREDELAY:
                        reverb.setPreDelay(kReverbPreDelayDataLibrary[reverb.preDelay].data);
                        break;

                    case REV_MODE_SURROUND:
                        reverb.setSurround(kReverbSurroundDataLibrary[reverb.surround].data);
                        break;
                    }
                    gTransition.phase = REV_PHASE_B;

                    gTransition.activeDry = 1.0;
                    gTransition.targetDry = 0.0;

                    gTransition.activeWet = 0.0;
                    gTransition.targetWet = 1.0;

                    reverb_calculateGenTransition();

                    transitionShowFlag = 2;
                    break;

                case REV_PHASE_B:
                    gTransition.active = false;
                    gTransition.mode = REV_MODE_NONE;
                    gTransition.phase = REV_PHASE_NONE;
                    transitionClearFlag = true;
                    break;
                }
                break;
            }
        }
    }

    if (reverb.mixTransition.active) {
        ReverbMixTransition &mTransition = reverb.mixTransition;

        switch (mTransition.actionDry) {
        case REV_ACTION_UP:
            if (reverb.dryFloat >= mTransition.targetDry) {
                mTransition.actionDry = REV_ACTION_NONE;
            } else {
                reverb.dryFloat += 0.0001;
            }
            break;

        case REV_ACTION_DOWN:
            if (reverb.dryFloat <= mTransition.targetDry) {
                mTransition.actionDry = REV_ACTION_NONE;
            } else {
                reverb.dryFloat -= 0.0001;
            }
            break;
        }

        switch (mTransition.actionWet) {
        case REV_ACTION_UP:
            if (reverb.wetFloat >= mTransition.targetWet) {
                mTransition.actionWet = REV_ACTION_NONE;
            } else {
                reverb.wetFloat += 0.0001;
            }
            break;

        case REV_ACTION_DOWN:
            if (reverb.wetFloat <= mTransition.targetWet) {
                mTransition.actionWet = REV_ACTION_NONE;
            } else {
                reverb.wetFloat -= 0.0001;
            }
            break;
        }

        if ((mTransition.actionDry == REV_ACTION_NONE) && (mTransition.actionWet == REV_ACTION_NONE)) {
            mTransition.active = false;
        }
    }
}

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

void Controller::interruptUpDownButtonRead() {
    if (upDownButtonCounter < 2) {
        upDownButtonCounter += 1;
    } else {
        if (!alertFlag) {
            if (upButtonFlag) {
                switch (menu) {
                case FILE_MENU:
                    file_menuUp();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuUp();
                    break;

                case SYSTEM_MENU:
                    if ((menuTab != 3) && (menuTab != 6) && (menuTab != 7)) {
                        system_menuUp();
                    }
                    break;

                case RHYTHM_MENU:
                    rhythm_menuUp();
                    break;

                case METRO_MENU:
                    if ((menuTab != 0) && (menuTab != 1)) {
                        metro_menuUp();
                    }
                    break;

                case EQ_MENU:
                    if (menuTab != 0) {
                        eq_menuUp();
                    }
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    if ((menuTab != 0) && (menuTab != 5)) {
                        filter_menuUp();
                    }
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    if (menuTab != 0) {
                        effect_menuUp();
                    }
                    break;

                case REVERB_MENU:
                    if (menuTab != 0) {
                        reverb_menuUp();
                    }
                    break;

                case LAYER_INST_MENU:
                    if ((menuTab != 0) && (menuTab != 2) && (menuTab != 6) && (menuTab != 7)) {
                        layerInst_menuUp();
                    }
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuUp();
                    break;
                }
            } else if (downButtonFlag) {
                switch (menu) {
                case FILE_MENU:
                    file_menuDown();
                    break;

                case DRUMKIT_MENU:
                    drumkit_menuDown();
                    break;

                case SYSTEM_MENU:
                    if ((menuTab != 3) && (menuTab != 6) && (menuTab != 7)) {
                        system_menuDown();
                    }
                    break;

                case RHYTHM_MENU:
                    rhythm_menuDown();
                    break;

                case METRO_MENU:
                    if ((menuTab != 0) && (menuTab != 1)) {
                        metro_menuDown();
                    }
                    break;

                case EQ_MENU:
                    if (menuTab != 0) {
                        eq_menuDown();
                    }
                    break;

                case FILTER_0_MENU:
                case FILTER_1_MENU:
                    if ((menuTab != 0) && (menuTab != 5)) {
                        filter_menuDown();
                    }
                    break;

                case EFFECT_0_MENU:
                case EFFECT_1_MENU:
                    if (menuTab != 0) {
                        effect_menuDown();
                    }
                    break;

                case REVERB_MENU:
                    if (menuTab != 0) {
                        reverb_menuDown();
                    }
                    break;

                case LAYER_INST_MENU:
                    if ((menuTab != 0) && (menuTab != 2) && (menuTab != 6) && (menuTab != 7)) {
                        layerInst_menuDown();
                    }
                    break;

                case LAYER_SONG_MENU:
                    layerSong_menuDown();
                    break;
                }
            }
        }
    }
}

void Controller::interruptLongButtonRead() {
    // main select
    if (mainMenuButtonFlag) {
        if (keyboard.longButtonCounter < kLongButtonCountLow) {
            keyboard.longButtonCounter += 1;
        } else {
            stopLongButtonTimer();
            keyboard.longButtonCounter = 0;
            mainMenuButtonFlag = false;
            main_select();
        }
    } else {
        if (keyboard.longButtonCounter < kLongButtonCountHigh) {
            keyboard.longButtonCounter += 1;
        } else {
            // layer song beat
            if (layerBeatButtonFlag) {
                keyboard.longButtonCounter = 0;
                switch (layerBeatButtonStage) {
                case 0:
                    layerSong_generateBeat(0);
                    layerBeatButtonStage = 1;
                    break;

                case 1:
                    layerSong_generateBeat(1);
                    layerBeatButtonStage = 2;
                    break;

                case 2:
                    layerSong_generateBeat(2);
                    layerBeatButtonFlag = false;
                    stopLongButtonTimer();
                    keyboard.longButtonCounter = 0;
                    break;

                default:
                    break;
                }
            }

            // layer song clear
            else if (layerClearButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerClearButtonFlag = false;
                layerSong_resetAllBeats(selectedLayerNum, activeBankNum);
            }

            // layer inst copy
            else if (layerInstCopyButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerInstCopyButtonFlag = false;
                copyLayerInstNum = selectedLayerNum;
                textCopyFlag = true;
            }
            // layer inst paste
            else if (layerInstPasteButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerInstPasteButtonFlag = false;
                if ((copyLayerInstNum != -1) && (selectedLayerNum != -1)) {
                    layerInstPasteActionFlag = true;
                    textPasteFlag = true;
                }
            }
            // layer song copy
            else if (layerSongCopyButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerFillCopyButtonFlag = false;
                layerSongCopyButtonFlag = false;
                if (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum != -1) {
                    copyLayerSongNum = selectedLayerNum;
                    copyBankSongNum = activeBankNum;
                    textCopyFlag = true;
                }
            }
            // layer song paste
            else if (layerSongPasteButtonFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                layerFillPasteButtonFlag = false;
                layerSongPasteButtonFlag = false;
                if ((copyLayerSongNum != -1) && (copyBankSongNum != -1) && (layerLibrary[selectedLayerNum].bankLibrary[activeBankNum].lastActiveBeatNum == -1) && (layerLibrary[copyLayerSongNum].bankLibrary[copyBankSongNum].lastActiveBeatNum != -1)) {
                    layerSongPasteActionFlag = true;
                    textPasteFlag = true;
                }
            }

            // rhythm unlock
            else if (rhythmUnlockFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                rhythmUnlockFlag = false;
                rhythm.measureLock = false;
                rhythm.barLock = false;
                rhythm.quantizeLock = false;
                lcd_drawRhythm_MeasureData();
                lcd_drawRhythm_BarData();
                lcd_drawRhythm_QuantizeData();
            }

            // rhythm lock
            else if (rhythmLockFlag) {
                stopLongButtonTimer();
                keyboard.longButtonCounter = 0;
                rhythm.measureLock = true;
                rhythm.barLock = true;
                rhythm.quantizeLock = true;
                lcd_drawRhythm_MeasureData();
                lcd_drawRhythm_BarData();
                lcd_drawRhythm_QuantizeData();
            }
        }
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

void Controller::interruptText() {
    if (textShow) {
        textShow = false;
    } else {
        stopTextTimer();
        textClearFlag = true;
    }
}

void Controller::interruptSd() {
    if ((sd.detect) && (sd_detect() == SD_ERROR)) {
        f_close(&sd.file);
        sd_unmount();
        if (playActive)
            triggerReset();
        sd.detect = false;
        sd.ready = false;
        lcd_drawSdAlert(SD_ERROR_DETECT);
        lcd_drawSdData();
    }
}

void Controller::interruptBeatSync() {
    SYNC_OUT_OFF;
    stopBeatSyncTimer();
}

void Controller::interruptLimitAlert() {
    stopLimitAlertTimer();
    limitAlertClearFlag = true;
}

/*
void Controller::writeWavData() {
    f_open(&sd.file, "Record.wav", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&sd.file, wavIntro, sizeof(wavIntro), &sd.byteswritten);
    f_write(&sd.file, &wavData[0], 176400, &sd.byteswritten);
    f_close(&sd.file);
}
*/
