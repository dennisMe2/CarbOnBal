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
#include "globals.h"
#include "utils.h"

#define NUM_SENSORS 4
#define NUM_BUTTONS 4
#define SELECT 0
#define LEFT 1
#define RIGHT 2
#define CANCEL 3
                         
#define P0VSENSOR 150                                             //minimum pressure reported by MAP sensor at 0V in millibar/hPa (=10x KPa value)
#define P5VSENSOR 1020                                            //maximum pressure reported by sensor at 5V in millibar

extern settings_t settings;

float millibarFactor =  (P5VSENSOR - P0VSENSOR) / 1024.00;           //conversion factor to convert the arduino readings to millibars

byte buttonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH}; //array for recording the state of buttons
byte lastButtonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH};//array for recording the previous state of buttons
unsigned long lastDebounceTime[NUM_BUTTONS]; //array for recording when the buttonpress was first seen

uint8_t debounceDelay = 200; //allow 200ms for switches to settle before they register

float convertToPreferredUnits(int value){
  if (0 == settings.units) return value;
  if (1 == settings.units) return convertToMillibar(value);
  if (2 == settings.units) return convertToCmHg(value);
  if (3 == settings.units) return convertToInHg(value);
  return 0; //error
}


//convert the arduino reading to millibars for display
float convertToMillibar(int value){
  return value * millibarFactor + P0VSENSOR;                      //convert reading and add the sensor's minimum pressure
}

//convert the arduino readings to centimeters of mercury
float convertToCmHg(int value){
    return convertToMillibar(value) * 0.075;
}

//convert the arduino readings to inches of mercury
float convertToInHg(int value){
    return convertToMillibar(value) * 0.02953;
}



//reset to factory defaults
void resetToFactoryDefaultSettings(){
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
    settings.units = 0;
}

// tests if a button was pressed and applies debounce logic
// this function assumes all buttons are input_pullup, active LOW, and contiguous pin numbers!
// this function does not use wait loops or other blocking functions which delay processing
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

// calculates the smoothing factor 'alpha' used by the Exponential moving average
// this takes a percentage as input and uses an exponential function to make the settings more sensitive
float calculateAlpha(int input){
    int reverse = 100-input;
    return pow((float) reverse / 100.0, 5);
}

// calculate Exponentially weighted moving average for smoothing
// alpha is how much weight is given to new values vs the stored average: 0 = 0%  - 1 = 100%
// accumulator is a pointer to a global value in which to store the average
// new value is the new data measurement
float exponentialMovingAverage(float alpha, float *accumulator, float new_value) {
    *accumulator += alpha * (new_value - *accumulator);
    return(*accumulator);
}

// version of the Exponential Moving Average that deliberately stops smoothing
// if the value changes more than a certain "percentage", the global 'stabilityThreshold' which is precalculated to save an expensive FP division.
// this is needed to get a stable readout, yet respond quickly when the gas is tweaked.
// not technically needed but convinces the user the system is working as expected!
float responsiveEMA(float alpha, float *accumulator, float new_value) {

    if(new_value > (*accumulator * (1 + stabilityThreshold)) || new_value < (*accumulator * (1-stabilityThreshold))){
      *accumulator = new_value;//short out the EMA if our value is changing fast
      }

    *accumulator += alpha * (new_value - *accumulator);
    return(*accumulator);
}

// calculate the absolute difference between two integers
int delta(int first, int second){
    if( first >= second){
        return first-second;
    }else{
        return second-first;
    }
}

// return the highest value from a given array
int maxVal( int value[]) {
    int maxValue = 0;
    for (int index = 0 ; index < NUM_SENSORS; index++) {
        if (value[index] > maxValue) {
            maxValue = value[index];
        }
    }
    return maxValue;
}

// return the lowest value from a given array
int minVal( int value[]) {
    int minValue = 20000;
    for (int index = 0 ; index < NUM_SENSORS; index++) {
        if (value[index] < minValue) {
            minValue = value[index];
        }
    }
    return minValue;
}

// convert an integer index into a baud rate for the arduino
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
