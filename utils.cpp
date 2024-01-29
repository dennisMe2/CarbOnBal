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

#include "utils.h"
#include <Arduino.h>
#include "globals.h"
#include LANGUAGE
#include "lcdWrapper.h"

float millibarFactor = (P5VSENSOR - P0VSENSOR) / static_cast<float>(analogReadRange); //conversion factor to convert the arduino readings to millibars

byte buttonState[NUM_BUTTONS] = { HIGH, HIGH, HIGH, HIGH }; //array for recording the state of buttons
byte buttonCount[NUM_BUTTONS] = { 0, 0, 0, 0 }; //array for recording the state of buttons
byte lastButtonState[NUM_BUTTONS] = { HIGH, HIGH, HIGH, HIGH }; //array for recording the previous state of buttons
unsigned long lastDebounceTime[NUM_BUTTONS]; //array for recording when the button press was first seen
unsigned long lastEntry = 0;

uint8_t debounceDelay = 200; //allow 200ms for switches to settle before they register

void setInterrupt(bool enabled) {
    if (enabled) {
        TIMSK1 |= (1 << OCIE1A);
    } else {
        TIMSK1 |= (0 << OCIE1A);
    }
}

void setupInterruptTimer() {
    //the computation for the interrunpt timing is as follows:
    //AtMega328p running at 16Mhz native frequency
    //Use a 64 prescaler value to arrive at a timer frequency of 16,000,000/64 = 250,000
    //Set timer (OCR1A setting) to count 0 - 249, then reset to 0, so 250,000/250 = 1000/1Khz
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1 = 0; //initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = 249;	// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS11 bits for 64 prescaler
    TCCR1B |= (1 << CS11) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    setInterrupt(true);
}

float convertToPreferredUnits(int value, int ambient) {
    switch(settings.units) {
    case PressureUnit::RAW:
        return value;
    case PressureUnit::RAW_DESCENDING:
        return ambient - value;
    case PressureUnit::MILLIBAR_HPA:
        return convertToMillibar(value);
    case PressureUnit::MILLIBAR_HPA_DESCENDING:
        return convertToMillibar(ambient) - convertToMillibar(value);
    case PressureUnit::CM_MERCURY:
        return convertToCmHg(value);
    case PressureUnit::CM_MERCURY_DESCENDING:
        return convertToCmHg(ambient) - convertToCmHg(value);
    case PressureUnit::INCH_MERCURY:
        return convertToInHg(value);
    case PressureUnit::INCH_MERCURY_DESCENDING:
        return convertToInHg(ambient) - convertToInHg(value);
    }
    return 0; //error
}

float differenceToPreferredUnits(int value) {
    switch(settings.units) {
    case PressureUnit::RAW:
    case PressureUnit::RAW_DESCENDING:
        return value;
    case PressureUnit::MILLIBAR_HPA:
    case PressureUnit::MILLIBAR_HPA_DESCENDING:
        return differenceToMillibar(value);
    case PressureUnit::CM_MERCURY:
    case PressureUnit::CM_MERCURY_DESCENDING:
        return differenceToCmHg(value);
    case PressureUnit::INCH_MERCURY:
    case PressureUnit::INCH_MERCURY_DESCENDING:
        return differenceToInHg(value);
    }
    return 0; //error
}

const char* unitsAsText() {
    switch(settings.units) {
    case PressureUnit::RAW:
        return txtRawValues;
    case PressureUnit::RAW_DESCENDING:
        return txtRawDescending;
    case PressureUnit::MILLIBAR_HPA:
        return txtMillibarHpa;
    case PressureUnit::MILLIBAR_HPA_DESCENDING:
        return txtMillibarHpaDesc;
    case PressureUnit::CM_MERCURY:
        return txtCmMercury;
    case PressureUnit::CM_MERCURY_DESCENDING:
        return txtCmMercuryDesc;
    case PressureUnit::INCH_MERCURY:
        return txtInchMercury;
    case PressureUnit::INCH_MERCURY_DESCENDING:
        return txtInchMercuryDesc;
    }
    return 0;
}

//convert the arduino reading to millibars for display
float convertToMillibar(int value) {
    return value * millibarFactor + P0VSENSOR; //convert reading and add the sensor's minimum pressure
}
float differenceToMillibar(int value) {
    return value * millibarFactor; //convert reading and add the sensor's minimum pressure
}

//convert the arduino readings to centimeters of mercury
float convertToCmHg(int value) {
    return convertToMillibar(value) * 0.075;
}
float differenceToCmHg(int value) {
    return differenceToMillibar(value) * 0.075;
}

//convert the arduino readings to inches of mercury
float convertToInHg(int value) {
    return convertToMillibar(value) * 0.02953;
}

float differenceToInHg(int value) {
    return differenceToMillibar(value) * 0.02953;
}

//reset to factory defaults
settings_t fetchFactoryDefaultSettings() {
    settings_t settings;

    settings.silent = false;
    settings.advanced = false;
    settings.splashScreen = true;
    settings.cylinders = 4;
    settings.master = 1;
    settings.button1 = Button1Mode::CONTRAST;
    settings.button2 = Button2Mode::BRIGHTNESS;
    settings.button3 = Button3Mode::FREEZE_DISPLAY;
    settings.contrast = 10;
    settings.brightness = 255;
    settings.graphType = GraphType::BARS;
    settings.rpmDamping = 10;
    settings.units = PressureUnit::RAW;
    settings.zoom = 0;
    settings.calibrationMax = 32;
    settings.damping = 8;

    return settings;
}

void doContrast(uint8_t value) {
    analogWrite(contrastPin, value);
}

void doBrightness(uint8_t value) {
    analogWrite(brightnessPin, value);
}

void doHeldButtonAction(int button) {
    switch (button) {

    case CANCEL:
        settings = fetchFactoryDefaultSettings();
        doContrast(settings.contrast);
        doBrightness(settings.brightness);
        break;
    }
}

// tests if a button was pressed and applies debounce logic
// this function assumes all buttons are input_pullup, active LOW, and contiguous pin numbers!
// this function does not use wait loops or other blocking functions which delay processing
int buttonPressed() {
    int pressedButton = 0;
    if (millis() - lastEntry < 50)
        return 0; //checking more often that every 50ms is nonsense, just return
    lastEntry = millis();

    for (uint8_t button = SELECT; button <= CANCEL; button++) {
        uint8_t buttonIndex = button - SELECT;
        buttonState[buttonIndex] = digitalRead(button);

        if ((millis() - lastDebounceTime[buttonIndex]) < debounceDelay)
            return 0; //return if this button hasn't settled yet
        lastDebounceTime[buttonIndex] = millis();

        if (buttonState[buttonIndex] == RELEASED
                && lastButtonState[buttonIndex] == PRESSED) {
            buttonCount[buttonIndex] = 0;
            pressedButton = button;
        } else if (buttonState[buttonIndex] == PRESSED
                   && lastButtonState[buttonIndex] == PRESSED) {
            buttonCount[buttonIndex]++;

            if (button == LEFT)
                pressedButton = LEFT;
            if (button == RIGHT)
                pressedButton = RIGHT;

            if (buttonCount[buttonIndex] > 10) {
                buttonCount[buttonIndex] = 0;
                doHeldButtonAction(button);
            }
        }

        lastButtonState[buttonIndex] = buttonState[buttonIndex];
    }

    return pressedButton; //just don't try to connect a button to pin 0
}

//creates a special character which is stored in the display's memory
void createWaitKeyPressChar() {
    byte customChar[8] = { 0b00100, 0b00100, 0b10101, 0b01110, 0b00100, 0b00000,
                           0b01110, 0b11111
                         };
    lcd_createChar(0, customChar);
}

void displayKeyPressPrompt() {
    createWaitKeyPressChar();
    lcd_setCursor(19, 0);
    lcd_write(byte((byte) 0));
}
void waitForAnyKey() {
    displayKeyPressPrompt();
    while (!buttonPressed()) {
        delay(50);
    }
}

// used by switches which "short" the pin to ground, saves wiring a resistor per switch
void setInputActiveLow(int i) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);  // turn on internal pullups
}

// sets a pin to output, with internal pull-up resistors
void setOutputHigh(int i) {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);  // turn on internal pullups
}

// calculate Extremely Fast Integer Exponentially weighted moving average for smoothing.
// factor is how much weight is given to new values vs the stored average as a power of 2.
//    ie: 0 = 1:1 and 4 = 1/16th
// shift is used to get n bits of accuracy 'below zero' as it were 0 means no smoothing, more is exponentially (1/2^n) more smoothing
// average is a value in which to store the moving average;
//    NOTE that this value is stored shifted 'shift' bits to the left and must be unshifted before use
//    NOTE2 the shift WILL truncate if you overdo it, best used on 8-bit Bytes etc.
int intExponentialMovingAverage(int shift, int factor, int average, int input) {
    average += ((input << shift) - average) >> factor;
    return (average);
}

//slower than the int version but extremely accurate / sensitive
long longExponentialMovingAverage(int factor, long average, int input) {
    longAverages longValue; //this insane union is used to save CPU cycles, instead of shifting bits 16x we just load the upper int in one go
    longValue.intVal[0] = 0;
    longValue.intVal[1] = input;
    average += (longValue.longVal - average) >> factor;
    return (average);
}

long mulExponentialMovingAverage(long average, int input) {
    long weight = 1000;
    average += (((long) input * 1000) - average) / weight;
    return (average);
}

//need a performance benchmark
float floatExponentialMovingAverage(float weight, float average, int input) {
    average += ((float) input - average) / weight;
    return (average);
}

// calculate the absolute difference between two integers
int delta(int first, int second) {
    if (first >= second) {
        return first - second;
    } else {
        return second - first;
    }
}

// return the highest value from a given array
unsigned int maxVal(unsigned int value[]) {
    unsigned int maxValue = 0;
    for (int index = 0; index < NUM_SENSORS; index++) {
        if (value[index] > maxValue) {
            maxValue = value[index];
        }
    }
    return maxValue;
}

// return the lowest value from a given array
unsigned int minVal(unsigned int value[]) {
    unsigned int minValue = 20000;
    for (int index = 0; index < NUM_SENSORS; index++) {
        if (value[index] < minValue) {
            minValue = value[index];
        }
    }
    return minValue;
}

//Free memory routine from the Arduino playground
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
