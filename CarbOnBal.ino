    // This software, known as CarbOnBal is
    // Copyright, 2017 L.L.M. (Dennis) Meulensteen. dennis@meulensteen.nl
    //
    // This file is part of CarbOnBal. A combination of software and hardware.
    // I hope it may be of some help to you in balancing your carburetors and throttle bodies.
    // Always be careful when working on a vehicle or electronic project like this.
    // Your life and health are your sole responsibility, use wisely.
    //
    // CarbOnBal hardware is covered by the Cern Open Hardware License v1.2
    // a copy of the text is incuded with the source code.
    //
    // CarbOnBal is free software: you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation, either version 3 of the License, or
    // (at your option) any later version.
    //
    // CarbOnBal is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.
    //
    // You should have received a copy of the GNU General Public License
    // along with CarbOnBal.  If not, see <http://www.gnu.org/licenses/>.


#include <Arduino.h>
#include <ShiftedLCD.h>
#include "functions.h"
#include "utils.h"
#include "lcdWrapper.h"
#include "menu.h"
#include "menuActions.h"
#include "globals.h"
settings_t settings;

float accumulator[NUM_SENSORS]= {1000.0, 1000.0, 1000.0, 1000.0};
int inputPin[NUM_SENSORS] = {A0, A1, A2, A3};
int timeBase=0;
long sums[NUM_SENSORS]={0,0,0,0};
long sums2[NUM_SENSORS]={0,0,0,0};
int8_t calibration[NUM_SENSORS-1][256];
bool freezeDisplay = false;
unsigned int rpm;
float alpha;
float alphaRpm;
int readingCount[NUM_SENSORS];
uint8_t readIndex[NUM_SENSORS];
int average[NUM_SENSORS];
int total[NUM_SENSORS];
int reading[NUM_SENSORS];

void setup() {
    loadSettings();
    Serial.begin(getBaud(settings.baudRate));

    setInputActiveLow(SELECT);
    setInputActiveLow(LEFT);
    setInputActiveLow(RIGHT);
    setInputActiveLow(CANCEL);

    analogWrite(brightnessPin, settings.brightness);
    analogWrite(contrastPin, settings.contrast);

    lcd_begin(DISPLAY_COLS, DISPLAY_ROWS);
    lcd_print(F("Lets Rock!"));

    doLoadCalibrations();
    alpha = calculateAlpha(settings.damping);
    alphaRpm = calculateAlpha(settings.rpmDamping);
    delay(1000);
}

void loop() {

    switch ( buttonPressed()) {
        case SELECT: actionDisplayMainMenu(); break;
        case LEFT: actionContrast(); break;
        case RIGHT:
        if(settings.button2 ==0){
            actionBrightness();
        } else{
            doRevs();
        }
        break;

        case CANCEL: freezeDisplay = !freezeDisplay; break;
    }

    runningAverage();

    if (!freezeDisplay){
        switch ( settings.graphType) {
            case 0: lcdBarsSmooth(average); break;
            case 1: lcdBarsCenterSmooth(average); break;
        }
    }else{
        byte frozen[8] = {
            0b00000,
            0b00000,
            0b01010,
            0b10101,
            0b01110,
            0b10101,
            0b01010,
            0b00000
        };
        lcd_createChar(7, frozen);
        lcd_setCursor(19, 0);
        lcd_write(byte(0x07));
    }
}

void runningAverage() {
    unsigned long startTime = millis();
    for (int sensor = 0; sensor < settings.cylinders; sensor++) {
        delayMicroseconds(settings.delayTime);

        reading[sensor] = analogRead(inputPin[sensor]);
        if (reading[sensor] <= 1023 - settings.threshold ){
            if (sensor != 0) {  //only apply calibration for non reference sensors
                sums[sensor] += reading[sensor] + calibration[sensor-1][(unsigned int) reading[sensor] >> 2 ];
            } else {
                sums[sensor] += reading[sensor];
            }
            readingCount[sensor]++;

        } else {
            if(readingCount[sensor] > 0){//immediately after the vacuum peak

                average[sensor] = responsiveEMA(alpha, &accumulator[sensor], sums[sensor]/readingCount[sensor]);
            }
            //done the calcs, now reset everything
            sums[sensor]=0;
            readingCount[sensor]=0;
        }
    }
    timeBase = millis() - startTime;
}


void lcdBarsCenterSmooth( int value[]) {
    const uint8_t segmentsInCharacter = 5;

    byte bar[4][8];

    char bars[DISPLAY_COLS + 1];

    int maximumValue = maxVal(value);
    int minimumValue = minVal(value);
    int range;
    int zoomFactor;

    if (maximumValue - value[settings.master-1] >= value[settings.master-1] - minimumValue) {
        range = maximumValue - value[settings.master-1];
    } else {
        range = value[settings.master-1] - minimumValue;
    }

    if (range < 50) {
        range = 50;
    }
    zoomFactor = range / 50;

    for (int sensor = 0 ; sensor < settings.cylinders; sensor++) {
        int delta = value[sensor] - value[settings.master-1];
        int TotalNumberOfLitSegments = delta / zoomFactor;
        int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;
        int numberOfLitSegments = TotalNumberOfLitSegments % segmentsInCharacter;

        if (sensor != settings.master -1) {
            makeCenterBars(bars, numberOfLitBars);
            lcd_setCursor(0, sensor);
            lcd_print(bars);

            makeChar(bar[sensor],numberOfLitSegments);
            lcd_createChar(sensor+2, bar[sensor]);

            if(numberOfLitSegments >0){
                lcd_setCursor(10 + numberOfLitBars, sensor);
            }else{
                lcd_setCursor(9 + numberOfLitBars, sensor);
            }
            if(numberOfLitBars <10) lcd_write(byte(sensor+2));

            if (numberOfLitBars < 0) {
                printLcdSpace(16,sensor,4);
            } else {
                printLcdSpace(0,sensor,4);
            }
            if (!settings.silent) {
                lcd_printInt(delta);
            }
        } else {
            printLcdInteger(value[sensor], 0, sensor, 4);
            printLcdInteger(timeBase, 10, sensor, 4);
            printLcdInteger(range, 16, sensor, 4);
        }
    }
}



void lcdBarsSmooth( int value[]) {
    const uint8_t segmentsInCharacter = 5;

    byte bar[4][8];
    char bars[DISPLAY_COLS + 1];

    for (int sensor = 0 ; sensor < settings.cylinders; sensor++) {
        int TotalNumberOfLitSegments = 100000L/1024 * value[sensor]/1000;
        int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;
        int numberOfLitSegments = TotalNumberOfLitSegments % segmentsInCharacter;

        if(numberOfLitBars > 14){
            makeBars(bars, numberOfLitBars, 4);
        } else{
            makeBars(bars, numberOfLitBars, 0);
        }
        lcd_setCursor(0, sensor);
        lcd_print(bars);

        makeChar(bar[sensor],numberOfLitSegments);
        lcd_createChar(sensor+2, bar[sensor]);
        lcd_setCursor(numberOfLitBars, sensor);
        lcd_write(byte(sensor+2));

        if (numberOfLitBars <= 14) {
            lcd_setCursor(16, sensor);
        } else {
            lcd_setCursor(0, sensor);
        }

        if (!settings.silent) lcd_printInt(value[sensor]);
    }
}

void doZeroCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Wiping..."));
    zeroCalibrations();
    delay(500);
}

void zeroCalibrations() {
    for (uint8_t sensor = 0; sensor < (NUM_SENSORS - 1); sensor++) {
        for (int i = 0; i < numberOfCalibrationValues; i++) {
            calibration[sensor][i] = 0;
        }
    }
}

void doLoadCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Loading..."));
    eeprom_read_block((void*)&calibration, (void*)calibrationOffset, sizeof(calibration));
    delay(500);
}

void doSaveCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Saving..."));
    eeprom_write_block((const void*)&calibration, (void*)calibrationOffset, sizeof(calibration));
    delay(500);
}

void actionSaveSettings() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Saving..."));
    eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
    delay(500);
}

void actionLoadSettings() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Loading..."));
    loadSettings();
    delay(500);
}

void loadSettings(){
    uint8_t compareVersion=0;
    eeprom_read_block((void*)&compareVersion, (void*)0, sizeof(settings.versionID));

    if(compareVersion == versionUID){
        eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
    } else{
        settings.brightness=255;
        settings.contrast=40;
        settings.damping=20;
        settings.delayTime=10;
        settings.graphType=0;
        settings.usePeakAverage=false;
        settings.baudRate = 9;
        settings.silent = false;
        settings.cylinders = 4;
        settings.master = 4;
        settings.threshold  = 100;
        settings.button1 =0;
        settings.button2 =0;
        settings.rpmDamping =20;
        settings.responsiveness = 80;
    }

    doContrast(settings.contrast);
    doBrightness(settings.brightness);
}

void doCalibrate() {
    lcd_clear();
    lcd_setCursor(0, 0);
    lcd_print(F("Calibration busy,"));
    lcd_setCursor(0, 1);
    lcd_print(F("apply vacuum!"));

    bool calibrated[256];
    for(int i =0 ; i<256;i++){
        calibrated[i] = false;
    }

    bool calibrationTimeout = false;
    unsigned long startCalibrationTime = millis();
    while (!calibrationTimeout) {

        uint8_t seconds = 20;
        uint8_t secondCountDown = seconds;
        long lastUpdateTime = startCalibrationTime;

        while (millis() < startCalibrationTime + (long) (1000 * seconds)) {
            delayMicroseconds(settings.delayTime);
            reading[0] = analogRead(inputPin[0]);

            for (uint8_t sensor = 1; sensor < NUM_SENSORS; sensor++) {
                delayMicroseconds(settings.delayTime);
                reading[sensor] = analogRead(inputPin[sensor]);
                int8_t calibrationValue = reading[0] - reading[sensor];

                calibration[sensor-1][reading[sensor] >> 2] = calibrationValue;
                calibrated[reading[0] >> 2] = true;
            }

            if (millis() - lastUpdateTime > 1000L) {

                char bars[DISPLAY_COLS+1];
                makeBars(bars, secondCountDown - 2,0);

                lcd_setCursor(0, 3);
                lcd_print(bars);
                secondCountDown--;
                lastUpdateTime = millis();
            }
        }

        int uncalibrated =0;
        for(int i=0 ;i<256;i++){
            if(!calibrated[i]) uncalibrated++;
        }

        calibrationTimeout = true;
        lcd_clear();
        lcd_setCursor(0, 0);
        lcd_print(F("Calibration Done"));
        lcd_setCursor(0, 1);
        lcd_print("             ");
        printLcdSpace(0, 3, 15);
        lcd_print(F("Uncalibrated: "));
        printLcdInteger(uncalibrated, 15, 3, 4);
        delay(3000);

    }
}

    void doCalibrationDump() {

        lcd_clear();
        lcd_setCursor(0, 1);
        lcd_print(F("Connect Serial monitor"));
        lcd_setCursor(0, 1);
        Serial.begin(getBaud(settings.baudRate));
        if (Serial) {

            Serial.println(F("val\tS02\tS03\tS04\t"));

            for (int i = 0; i < numberOfCalibrationValues; i++) {
                Serial.print(i);
                Serial.print("\t");
                for (uint8_t sensor = 1; sensor < (NUM_SENSORS); sensor++) {
                    Serial.print(calibration[sensor-1][i]);
                    Serial.print("\t");
                }
                Serial.print("\n");
            }
            Serial.print(F("\n*END*\n"));
        }

    }

    void doDataDump() {
        int reading = 0;

        lcd_clear();
        lcd_setCursor(0, 1);
        lcd_print(F("Connect Serial..."));
        Serial.begin(getBaud(settings.baudRate));
        if (Serial) {
            lcd_setCursor(0, 1);
            lcd_print(F("Dumping Sensor Data "));
            Serial.println(F("0\t0\t0\t0\t0"));
            unsigned long startTime = millis();

            while (! (buttonPressed() == CANCEL)) {
                Serial.print(millis() - startTime);
                Serial.print("\t");
                for (uint8_t sensor = 0; sensor < (NUM_SENSORS); sensor++) {
                    delayMicroseconds(settings.delayTime);
                    if (sensor != 0) {  //only apply calibration for non reference sensors
                        reading = analogRead(inputPin[sensor]) + calibration[sensor-1][(int) reading >>  2];
                    } else {
                        reading = analogRead(inputPin[sensor]);
                    }

                    Serial.print(reading);
                    Serial.print("\t");
                }
                Serial.print("\n");
            }
            Serial.print(F("\n*END*\n"));
        }
    }

    void doRevs(){
        bool descending = false;
        bool previousDescending = false;

        unsigned long peak = millis();
        unsigned int delta;
        unsigned long previousPeak = millis();
        unsigned long lastUpdateTime = millis();

        int measurement = 1024;
        int previous = 1024;
        int descentCount = 0;
        int ascentCount = 0;

        float rpmAverage;

        initRpmDisplay();

        while (! (buttonPressed() == CANCEL)) {

            measurement = analogRead(inputPin[0]);

            if(measurement < (previous)){
                descentCount++;
                ascentCount--;
            }
            if(measurement > (previous)){
                ascentCount++;
                descentCount--;
            }
            if(descentCount > 2){
                descending = true;
                ascentCount = 0;
                descentCount =0;
            }

            if (ascentCount > 4){
                descending = false;
                descentCount = 0;
                ascentCount = 0;
            }

            if(previousDescending == true && descending == false){

                peak = millis();
                delta = peak - previousPeak;
                if (delta < 1) {
                    rpm = 0;
                }else{
                    rpm = exponentialMovingAverage(alphaRpm, &rpmAverage, 120000/delta);

                }
                previousPeak = peak;
            }

            previousDescending = descending;
            previous = measurement;

            if (millis() - peak > 2000) rpm = 0;

            if (millis() - lastUpdateTime > 200L) {
                updateRpmDisplay(rpm);
                lastUpdateTime = millis();
            }
        }
    }

    void initRpmDisplay(){
        lcd_clear();
        lcd_setCursor(6, 0);
        lcd_print(F(" RPM"));
        lcd_setCursor(0, 1);
        lcd_print(F("  1 2 3 4 5 6 7 8 9"));
    }

    void updateRpmDisplay(unsigned int rpm){

        char bars[DISPLAY_COLS+1];
        byte bar[8];
        uint8_t TotalNumberOfLitSegments = 1000000L/10000 * rpm/10000;
        uint8_t numberOfLitBars = TotalNumberOfLitSegments / 5;
        uint8_t numberOfLitSegments = TotalNumberOfLitSegments % 5;

        makeBars(bars, numberOfLitBars,0);
        makeChar(bar,numberOfLitSegments);
        lcd_createChar(6, bar);

        printLcdInteger(rpm,0,0,6);

        lcd_setCursor(0, 2);
        lcd_print(bars);
        if(numberOfLitBars < DISPLAY_COLS){
            lcd_setCursor(numberOfLitBars, 2);
            lcd_write(byte(6));
        }
        lcd_setCursor(0, 3);
        lcd_print(bars);
        if(numberOfLitBars < DISPLAY_COLS){
            lcd_setCursor(numberOfLitBars, 3);
            lcd_write(byte(6));
        }

    }


    void makeCenterBars(char *bars, int8_t number) {
        if(number >10) number=10;
        if(number <-10) number=-10;

        if (number < 0) {
            for (int8_t i = 0 ; i < 10 + number; i++) {
                bars[i] = ' ';
            }
            for (int8_t i = 10 + number ; i < 10; i++) {
                bars[i] = 0xff;
            }
            for (int8_t i = 10 ; i < 20; i++) {
                bars[i] = ' ';
            }
        }

        if (number == 0) {
            for (int8_t i = 0 ; i <= 20 ; i++) {
                bars[i] = ' ';
            }
        }

        if (number > 0) {
            for (int8_t i = 0 ; i <= 10 ; i++) {
                bars[i] = ' ';
            }
            for (int8_t i = 10 ; i <= 10 + number ; i++) {
                bars[i] = 0xff;
            }
            for (int8_t i = 10 + number ; i < 20; i++) {
                bars[i] = ' ';
            }
        }

        bars[DISPLAY_COLS] = 0x00;
    }
