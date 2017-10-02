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

#define NUM_SENSORS 4
#define NUM_BUTTONS 4
#define SELECT 0
#define LEFT 1
#define RIGHT 2
#define CANCEL 3

byte buttonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH};
byte lastButtonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastDebounceTime[NUM_BUTTONS];

uint8_t debounceDelay = 200;

int buttonPressed() {

    for (uint8_t button = SELECT; button <= CANCEL; button++) {
        buttonState[button] = digitalRead(button);

        if ( (millis() - lastDebounceTime[button]) > debounceDelay) {
            if ((buttonState[button] != lastButtonState[button] ) || ((millis() - lastDebounceTime[button]) > debounceDelay * 2) ) {
                if (buttonState[button] == LOW) {
                    lastDebounceTime[button] = millis();
                    lastButtonState[button] = buttonState[button];
                    return button;
                }
            }
            lastButtonState[button] = buttonState[button];
        }
    }
    return -1;
}

void setInputActiveLow(int i) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);  // turn on internal pullups
}

void setOutputHigh(int i) {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);  // turn on internal pullups
}

float calculateAlpha(int input){
    int reverse = 100-input;
    return pow((float) reverse / 100.0, 5);
}

float exponentialMovingAverage(float alpha, float *accumulator, float new_value) {
    *accumulator += alpha * (new_value - *accumulator);
    return(*accumulator);
}

int delta(int first, int second){
    if( first >= second){
        return first-second;
    }else{
        return second-first;
    }
}

int maxVal( int value[]) {
    int maxValue = 0;
    for (int index = 0 ; index < NUM_SENSORS; index++) {
        if (value[index] > maxValue) {
            maxValue = value[index];
        }
    }
    return maxValue;
}

int minVal( int value[]) {
    int minValue = 20000;
    for (int index = 0 ; index < NUM_SENSORS; index++) {
        if (value[index] < minValue) {
            minValue = value[index];
        }
    }
    return minValue;
}

unsigned long getBaud(int index){
    const unsigned long baud[] = {
        300,
        600,
        1200,
        2400,
        4800,
        9600,
        14400,
        19200,
        28800,
        31250,
        38400,
        57600,
        115200
    };
    return baud[index];
}
