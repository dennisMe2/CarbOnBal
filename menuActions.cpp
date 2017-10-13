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


#include "menuActions.h"
#include "menu.h"
#include "utils.h"

extern settings_t settings;
extern float alpha;
extern float alphaRpm;
extern float stabilityThreshold;

extern const uint8_t brightnessPin;//6
extern const uint8_t contrastPin;//5


void doBaudRate(){
    String actions[] = {F("300"), F("600"), F("1200"), F("2400"), F("4800"), F("9600"), F("14400")
    , F("19200"), F("28800"), F("31250"), F("38400"), F("57600"), F("115200")};
    settings.baudRate = doSettingChooser(F("Select Baud Rate:"), actions, 13, settings.baudRate) ;
}

void actionBrightness() {
    settings.brightness = doSettingChanger(F("Brightness:"), 0, 255, settings.brightness, 10, &doBrightness) ;
}

void doBrightness(int value) {
    analogWrite(brightnessPin, value);
}

void actionContrast() {
    settings.contrast = doSettingChanger(F("Contrast:"), 0, 255, settings.contrast, 10, &doContrast) ;
}

void doContrast(int value) {
    analogWrite(contrastPin, value);
}

void actionSilent() {
    String actions[] = {F("More details"), F("Less details")};
    settings.silent = (bool) doSettingChooser(F("Show Details:"), actions, 2, (int) settings.silent) ;
}

void actionGraphing() {
    String actions[] = {F("Absolute"), F("Centered")};
    settings.graphType = doSettingChooser(F("Select Graph Type:"), actions, 2, settings.graphType) ;
}

void actionDelay() {
    settings.delayTime = doBasicSettingChanger(F("Delay: (us)"), 0, 1000, settings.delayTime, 10) ;
}


void actionResponsiveness() {
    settings.responsiveness = (uint8_t) doBasicSettingChanger(F("Response: (%)"), 0, 100, settings.responsiveness, 5);
    stabilityThreshold = (100 - settings.responsiveness) / 100.00; //more than a certain % difference from the average
}

void actionDamping() {
    settings.damping = (uint8_t) doBasicSettingChanger(F("Damping: (%)"), 0, 100, settings.damping, 5);
    alpha = calculateAlpha(settings.damping);
}


void actionRPMDamping() {
    settings.rpmDamping = (uint8_t) doBasicSettingChanger(F("RPM Damping: (%)"), 0, 100, settings.rpmDamping, 5);
    alphaRpm = calculateAlpha(settings.rpmDamping);
}


void actionThreshold() {
    settings.threshold = (uint8_t) doBasicSettingChanger(F("Threshold:"), 0, 1023, settings.threshold, 10) ;
}


void actionCylinders() {
    settings.cylinders = (uint8_t) doBasicSettingChanger(F("Cylinder count:"), 1, 4, settings.cylinders, 1);
    fixMaster();
}

void actionMaster() {
    settings.master = (uint8_t) doBasicSettingChanger(F("Master Carb:"), 1, 4, settings.master, 1);
    fixMaster();
}


void fixMaster(){
    if (settings.cylinders < settings.master){
        settings.master = settings.cylinders;
    }
}

void actionBrightnessButton(){
    String actions[] = {F("Brightness"), F("RPM Display")};
    settings.button2 = doSettingChooser(F("Button 2 function:"), actions, 2, (int) settings.button2) ;
}
