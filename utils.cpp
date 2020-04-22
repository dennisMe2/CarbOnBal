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

extern settings_t settings;

float millibarFactor =  (P5VSENSOR - P0VSENSOR) / 1024.00;           //conversion factor to convert the arduino readings to millibars

byte buttonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH}; //array for recording the state of buttons
byte lastButtonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH};//array for recording the previous state of buttons
unsigned long lastDebounceTime[NUM_BUTTONS]; //array for recording when the button press was first seen
unsigned long lastEntry = 0 ;

uint8_t debounceDelay = 200; //allow 200ms for switches to settle before they register

void setInterrupt(bool enabled){
	if(enabled) {
		TIMSK1 |= (1 << OCIE1A);
	}else{
		TIMSK1 |= (0 << OCIE1A);
	}
}


float convertToPreferredUnits(int value, int ambient){
  if (0 == settings.units) return value;
  if (1 == settings.units) return ambient - value;
  if (2 == settings.units) return convertToMillibar(value);
  if (3 == settings.units) return convertToMillibar(ambient) - convertToMillibar(value);
  if (4 == settings.units) return convertToCmHg(value);
  if (5 == settings.units) return convertToCmHg(ambient) - convertToCmHg(value);
  if (6 == settings.units) return convertToInHg(value);
  if (7 == settings.units) return convertToInHg(ambient) - convertToInHg(value);
  return 0; //error
}
float differenceToPreferredUnits(int value){
  if (0 == settings.units) return value;
  if (1 == settings.units) return value;
  if (2 == settings.units) return differenceToMillibar(value);
  if (3 == settings.units) return differenceToMillibar(value);
  if (4 == settings.units) return differenceToCmHg(value);
  if (5 == settings.units) return differenceToCmHg(value);
  if (6 == settings.units) return differenceToInHg(value);
  if (7 == settings.units) return differenceToInHg(value);
  return 0; //error
}

const char* unitsAsText(){
	 if (0 == settings.units) return txtRawValues;
	 if (1 == settings.units) return txtRawDescending;
	 if (2 == settings.units) return txtMillibarHpa;
	 if (3 == settings.units) return txtMillibarHpaDesc;
	 if (4 == settings.units) return txtCmMercury;
	 if (5 == settings.units) return txtCmMercuryDesc;
	 if (6 == settings.units) return txtInchMercury;
	 if (7 == settings.units) return txtInchMercuryDesc;
	 return 0;
}

//convert the arduino reading to millibars for display
float convertToMillibar(int value){
  return value * millibarFactor + P0VSENSOR;                      //convert reading and add the sensor's minimum pressure
}
float differenceToMillibar(int value){
  return value * millibarFactor;                      //convert reading and add the sensor's minimum pressure
}

//convert the arduino readings to centimeters of mercury
float convertToCmHg(int value){
    return convertToMillibar(value) * 0.075;
}
float differenceToCmHg(int value){
    return differenceToMillibar(value) * 0.075;
}

//convert the arduino readings to inches of mercury
float convertToInHg(int value){
    return convertToMillibar(value) * 0.02953;
}

float differenceToInHg(int value){
    return differenceToMillibar(value) * 0.02953;
}


//reset to factory defaults
void resetToFactoryDefaultSettings(){
    settings.brightness = 255;
    settings.contrast = 10;
    settings.damping = 85;//obsolete
    settings.delayTime = 0;
    settings.graphType = 0;
    settings.usePeakAverage = false;
    settings.baudRate = 9;
    settings.silent = false;
    settings.cylinders = 4;
    settings.master = 4;
    settings.threshold  = 100;
    settings.button1 = 0;
    settings.button2 = 0;
    settings.rpmDamping = 35;
    settings.responsiveness = 75;
    settings.units = 0;
    settings.zoom = 0;
    settings.calibrationMax = 32;
    settings.advanced = false;
    settings.splashScreen = true;
    settings.arduinoCompatible = true;
    settings.averagingMethod = 0;
    settings.emaShift = 12;
    settings.emaFactor = 8;
    settings.emaCorrection = 6;
    settings.emaRpmSensitivity = 2;
}

// tests if a button was pressed and applies debounce logic
// this function assumes all buttons are input_pullup, active LOW, and contiguous pin numbers!
// this function does not use wait loops or other blocking functions which delay processing
int buttonPressed() {
	if( millis() - lastEntry < 50) return 0;//checking more often that every 50ms is nonsense, just return
	lastEntry = millis();

	for (uint8_t button = SELECT; button <= CANCEL; button++) {
		buttonState[button-SELECT] = digitalRead(button);

		if ( (millis() - lastDebounceTime[button-SELECT]) > debounceDelay) {
			if ((buttonState[button-SELECT] != lastButtonState[button-SELECT] ) || ((millis() - lastDebounceTime[button-SELECT]) > debounceDelay * 2) ) {
				if (buttonState[button-SELECT] == LOW) {
					lastDebounceTime[button-SELECT] = millis();
					lastButtonState[button-SELECT] = buttonState[button-SELECT];
					return button;
				}
			}
			lastButtonState[button-SELECT] = buttonState[button-SELECT];
		}
	}


    return 0;//just don't try to connect a button to pin 0
}

//creates a special character which is stored in the display's memory
void createWaitKeyPressChar(){
		byte customChar[8] = {
			0b00100,
			0b00100,
			0b10101,
			0b01110,
			0b00100,
			0b00000,
			0b01110,
			0b11111
		};
		lcd_createChar(0, customChar);
}

void displayKeyPressPrompt(){
	createWaitKeyPressChar();
			lcd_setCursor(19,0);
			lcd_write(byte((byte) 0));
}
void waitForAnyKey(){
	displayKeyPressPrompt();
      while(!buttonPressed()){
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
    average += ((input<<shift) - average)>>factor;
    return(average);
}

//slower than the int version but extremely accurate / sensitive
long longExponentialMovingAverage( int factor,  long average,  int input) {
    longAverages longValue; //this insane union is used to save CPU cycles, instead of shifting bits 16x we just load the upper int in one go
    longValue.intVal[0] = 0;
    longValue.intVal[1] = input;
    average += (longValue.longVal - average) >> factor;
    return(average);
}

long mulExponentialMovingAverage( long average, int input) {
	long weight = 1000;
	average += (((long)input * 1000) - average) / weight;
    return(average);
}

//need a performance benchmark
float floatExponentialMovingAverage(float weight, float average, int input){
	average += ((float)input - average) / weight;
	return (average);
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
unsigned int maxVal( unsigned int value[]) {
    unsigned int maxValue = 0;
    for (int index = 0 ; index < NUM_SENSORS; index++) {
        if (value[index] > maxValue) {
            maxValue = value[index];
        }
    }
    return maxValue;
}

// return the lowest value from a given array
unsigned int minVal( unsigned int value[]) {
    unsigned int minValue = 20000;
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
        115200,
		230400
    };
    return baud[index];
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
