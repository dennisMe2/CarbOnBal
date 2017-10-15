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

// The software uses a lot of global variables. This may not be elegant but its one way of writing non-blocking code
float accumulator[NUM_SENSORS]= {1000.0, 1000.0, 1000.0, 1000.0}; //used to track the average values per sensor
int inputPin[NUM_SENSORS] = {A0, A1, A2, A3};               //used as a means to access the sensor pins using a counter
int timeBase=0;                                             //allows us to calculate how long it took to measure and update the display (only used for testing)
long sums[NUM_SENSORS]={0,0,0,0};                           //tracks totals for calculating a numerical average
long sums2[NUM_SENSORS]={0,0,0,0};                          //totals for numerical averages
// this array is 1/4 the size of the possible values bacause it would eat up all the ram
// (there are theoretically four arrays of 1024 values required) by only sampling 1/4 we get a close approxmation
int8_t calibration[NUM_SENSORS-1][256];                     //store the offsets of the sensors relative to the sensor 0 reading, used to get a calibrated reading
bool freezeDisplay = false;                                 //used to tell when a user wants to freeze the display
unsigned int rpm;                                           //stores the current RPM
float alpha;                                                //alpha is the math term for the weighting factor used to calculate
                                                            //EMA Exponentially weighted moving average. Using this we save a lot of precious memory
float alphaRpm;                                             //alpha factor used to smoothe the RPM display
float stabilityThreshold;                                   //the factor used to detect when the throttle is being janked and a response is required

int readingCount[NUM_SENSORS];                              //used to store the number of captured readings for calculating a numerical average
uint8_t readIndex[NUM_SENSORS];                             //
int average[NUM_SENSORS];                                   //used to share the current average for each sensor
int total[NUM_SENSORS];                                     //
int reading[NUM_SENSORS];                                   //the current reading for each sensor

int ambientPressure;                                        //stores current ambient pressure for negative pressure display

//this does the initial setup on startup.
void setup() {
    loadSettings();                                         //load saved settings into memory from FLASH
    Serial.begin(getBaud(settings.baudRate));

    setInputActiveLow(SELECT);                              //set the pins connected to the switches
    setInputActiveLow(LEFT);                                //switches are wired directly and could
    setInputActiveLow(RIGHT);                               //short out the pins if setup wrong
    setInputActiveLow(CANCEL);

    analogWrite(brightnessPin, settings.brightness);        //brightness is PWM driven
    analogWrite(contrastPin, settings.contrast);            //contrast is PWM with smoothing (R/C network) to prevent flicker

    lcd_begin(DISPLAY_COLS, DISPLAY_ROWS);                  //instantiate the ShiftedLCD library

    doLoadCalibrations();                                   //load calibration data into RAM
    ambientPressure = detectAmbient();                      //set ambient pressure (important because it varies with weather and altitude)
    alpha = calculateAlpha(settings.damping);               //prime the alpha from the settings
    alphaRpm = calculateAlpha(settings.rpmDamping);
    stabilityThreshold = (100 - settings.responsiveness) / 100; //responsiveness is how quickly the system responds to rapid RPM changes as opposed to smoothing the display
    delay(1000);
}

void loop() {

    //test if a button was pressed
    switch ( buttonPressed()) {
        case SELECT: actionDisplayMainMenu(); break;        //the menu is the only function that does not return asap
        case LEFT: actionContrast(); break;
        case RIGHT:
        if(settings.button2 ==0){                           // there are two modes fot this pin, user settable
            actionBrightness();
        } else{
            doRevs();
        }
        break;

        case CANCEL: freezeDisplay = !freezeDisplay; break; //toggle the freezeDisplay option
    }

    runningAverage(); //calculate the running averages and return asap

    if (!freezeDisplay){
        switch ( settings.graphType) {                      //there are twotypes of graph. bar and centered around the value of the master carb
            case 0: lcdBarsSmooth(average); break;          //these functions both draw a graph and return asap
            case 1: lcdBarsCenterSmooth(average); break;    //
        }
    }else{
        //make a little snowflake to indicate the frozen state of the display
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
        lcd_createChar(7, frozen);                          //tells LCD to store our snowflake in slot 7
        lcd_setCursor(19, 0);                               // place the cursor on the LCD
        lcd_write(byte(0x07));                              //display the stored snowflake
    }
}

//the main measurement function which calculates a true average of all samples while the signal drops below the threshold.
//all the sensors are sampled and values stored then the loop returns asap so the screen can be updated in real time
void runningAverage() {
    unsigned long startTime = millis();
    for (int sensor = 0; sensor < settings.cylinders; sensor++) {       //loop over all sensors
        delayMicroseconds(settings.delayTime);                          //arduino's analog read needs a delay or the results suffer

        reading[sensor] = analogRead(inputPin[sensor]);
        if (reading[sensor] <= 1023 - settings.threshold ){             //1023 is basically normal airpressure at sea level. it can be lower, especially in the mountains
            if (sensor != 0) {                                          //only apply calibration for non reference sensors
                sums[sensor] += reading[sensor] + calibration[sensor-1][(unsigned int) reading[sensor] >> 2 ]; //adds this reading adjusted for calibration
            } else {
                sums[sensor] += reading[sensor];                        //sensor0 is the reference sensor, no calibration needed
            }
            readingCount[sensor]++;                                     //keeps count of the number of readings collected

        } else {                                                        //above the measurement threshold
            if(readingCount[sensor] > 0){                               //immediately after the vacuum peak
                //calculate the average now we've measured a whole "vacuum signal trough"
                average[sensor] = responsiveEMA(alpha, &accumulator[sensor], sums[sensor]/readingCount[sensor]);
            }
            //done the calcs, now reset everything ready to start over when the signal drops below threshold
            sums[sensor]=0;
            readingCount[sensor]=0;
        }
    }
    timeBase = millis() - startTime;                                    //monitor how long it takes to measure 4 sensors
}

// display centered bars, centered on the reference carb's reading, because that's the target we are aiming for
//takes an array of current average values for all sensors as parameter
void lcdBarsCenterSmooth( int value[]) {
    const uint8_t segmentsInCharacter = 5;                              //we need this number to make the display smooth

    byte bar[4][8];                                                     //store one custom character per bar

    char bars[DISPLAY_COLS + 1];                                        //store for the bar of full characters

    int maximumValue = maxVal(value);                                   //determine the sensor with the highest average
    int minimumValue = minVal(value);                                   //determine the lowest sensor average
    int range;                                                          //store the range between the highest and lowest sensors
    int zoomFactor;                                                     //store the zoom of the display

    //the range depends on finding the reading furthest from the master carb reading
    if (maximumValue - value[settings.master-1] >= value[settings.master-1] - minimumValue) {
        range = maximumValue - value[settings.master-1];
    } else {
        range = value[settings.master-1] - minimumValue;
    }

    //sets the minimum range before the display becomes 'pixellated' there are 100 segments available, 50 on either side of the master
    int ranges[]={50, 100, 150,300, 512};
    if (range < ranges[settings.zoom]) {
        range = ranges[settings.zoom];
    }
    
    zoomFactor = range / 50;

    for (int sensor = 0 ; sensor < settings.cylinders; sensor++) { //for each of the sensors the user wants to use
        int delta = value[sensor] - value[settings.master-1];       //find the difference between it and master
        int TotalNumberOfLitSegments = delta / zoomFactor;          //determine the number of lit segments
        int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;   //determine the number of whole characters
        int numberOfLitSegments = TotalNumberOfLitSegments % segmentsInCharacter;   //determine the remaining stripes

        if (sensor != settings.master -1) {             //for all sensors except the master carb sensor
            makeCenterBars(bars, numberOfLitBars);      //give us the bars of whole charachters
            lcd_setCursor(0, sensor);
            lcd_print(bars);                            //place the bars in the display

            makeChar(bar[sensor],numberOfLitSegments);  //make a custom char for the remaining stripes
            lcd_createChar(sensor+2, bar[sensor]);

            if(numberOfLitSegments >0){
                lcd_setCursor(10 + numberOfLitBars, sensor); //place it on the right
            }else{
                lcd_setCursor(9 + numberOfLitBars, sensor); //or in the center
            }
            if(numberOfLitBars <10) lcd_write(byte(sensor+2));

            if (numberOfLitBars < 0) {
                printLcdSpace(15,sensor,5);             //clear the display in preparation of printing the readings
            } else {
                printLcdSpace(0,sensor,5);
            }
            if (!settings.silent) {
                lcd_printFloat(differenceToPreferredUnits(delta) );                    //display the difference between this sensor and master
            }
        } else {
            float result = convertToPreferredUnits(value[sensor],ambientPressure);
            printLcdFloat(result, 0, sensor, 5);   //print the absolute value of the reference carb
            printLcdInteger(timeBase, 10, sensor, 4);       //show how long it took to measure four sensors
            printLcdFloat(differenceToPreferredUnits(range*2), 15, sensor, 5);          //show the zoom range
        }
    }
}


// this is used to display four plain unzoomed bars with absolute pressure readings
void lcdBarsSmooth( int value[]) {
    const uint8_t segmentsInCharacter = 5;

    byte bar[4][8];
    char bars[DISPLAY_COLS + 1];

    for (int sensor = 0 ; sensor < settings.cylinders; sensor++) {
        int TotalNumberOfLitSegments = 100000L/1024 * value[sensor]/1000;// integer math works faster, so we multiply by 1000 and divide later, powers of two would be even faster
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

        //set the cursor so the pressure readings don't interfere with the bars
        if (numberOfLitBars <= 12) {
            lcd_setCursor(14, sensor);
        } else {
            lcd_setCursor(0, sensor);
        }

        if (!settings.silent) {
          float result = convertToPreferredUnits(value[sensor], ambientPressure);
          lcd_printFloat(result);
        }
    }
}
//does the display while clearing the calibration array
void doZeroCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Wiping..."));
    zeroCalibrations();
    delay(500);
}
//actually clears the array
void zeroCalibrations() {
    for (uint8_t sensor = 0; sensor < (NUM_SENSORS - 1); sensor++) {
        for (int i = 0; i < numberOfCalibrationValues; i++) {
            calibration[sensor][i] = 0;
        }
    }
}

//loads calibrations from FLASH
void doLoadCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Loading..."));
    eeprom_read_block((void*)&calibration, (void*)calibrationOffset, sizeof(calibration));
    delay(500);
}
//saves calibrations to flash
void doSaveCalibrations() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Saving..."));
    eeprom_write_block((const void*)&calibration, (void*)calibrationOffset, sizeof(calibration));
    delay(500);
}

//saves our settings struct
void actionSaveSettings() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Saving..."));
    eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
    delay(500);
}

//handles the display for loading settings
void actionLoadSettings() {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print(F("Loading..."));
    loadSettings();
    delay(500);
}

//loads the settings from EEPROM (Flash)
void loadSettings(){
    uint8_t compareVersion=0; //compare version MUST be altered each time the settings struct is altered with a new setting or a different type
    eeprom_read_block((void*)&compareVersion, (void*)0, sizeof(settings.versionID));

    //only load settings if saved by the current version, otherwise reset to 'factory' defaults
    if(compareVersion == versionUID){
        eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
    } else{
        resetToFactoryDefaultSettings();
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

        int uncalibrated = 0;
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

    //used to detect ambient pressure for readings that ascend with vacuum in stead of going down toward absolute 0 pressure
    int detectAmbient(){
      unsigned long total=0;
      uint8_t numberOfSamples = 200;                                //set the number of samples to average
      
      for (int i = 0; i < numberOfSamples; i++){
        delayMicroseconds(settings.delayTime);
        total += analogRead(inputPin[0]);                    //add the reading we got
      }
      return total / numberOfSamples;                         //divide the result by the number of samples for the resulting average
    }

