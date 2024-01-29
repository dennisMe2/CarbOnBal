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


#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#include <Arduino.h>
#include "globals.h"
void setInterrupt(bool enabled);
void setupInterruptTimer();
void resetAverages();
float convertToPreferredUnits(int value, int ambient);
float differenceToPreferredUnits(int value);
float convertToMillibar(int value);
float differenceToMillibar(int value);
float convertToCmHg(int value);
float differenceToCmHg(int value);
float convertToInHg(int value);
float differenceToInHg(int value);
const char* unitsAsText();
void createWaitKeyPressChar();
void displayKeyPressPrompt();
settings_t fetchFactoryDefaultSettings();
int buttonPressed();
void waitForAnyKey();
void setInputActiveLow(int i);
void setOutputHigh(int i);
int intExponentialMovingAverage(int shift, int factor,int average, int input);
long longExponentialMovingAverage(int factor, long average, int input);
long mulExponentialMovingAverage( long average, int input);
float floatExponentialMovingAverage(float weight, float average, int input);
int delta(int first, int second);
unsigned int maxVal( unsigned int value[]);
unsigned int minVal( unsigned int value[]);
int freeMemory();
unsigned long getBaud(int index);

#endif
