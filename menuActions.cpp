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

#include "lang_en_gb.h"   //include British English texts
#include "menu.h"
#include "utils.h"

extern settings_t settings;
extern float alpha;
extern float alphaRpm;
extern float stabilityThreshold;

extern const uint8_t brightnessPin;//6
extern const uint8_t contrastPin;//11


void doBaudRate(){
    String actions[] = {F("300"), F("600"), F("1200"), F("2400"), F("4800"), F("9600"), F("14400")
    , F("19200"), F("28800"), F("31250"), F("38400"), F("57600"), F("115200")};
    settings.baudRate = doSettingChooser(F(TXT_BAUD_RATE), actions, 13, settings.baudRate) ;
}

void actionBrightness() {
    settings.brightness = doSettingChanger(F(TXT_BRIGHTNESS), 0, 255, settings.brightness, 10, &doBrightness) ;
}

void doBrightness(int value) {
    analogWrite(brightnessPin, value);
}

void actionContrast() {
    settings.contrast = doSettingChanger(F(TXT_CONTRAST), 0, 127, settings.contrast, 1, &doContrast) ;
}

void doContrast(int value) {
    analogWrite(contrastPin, value);
}

void actionSilent() {
    String actions[] = {F(TXT_MORE), F(TXT_LESS)};
    settings.silent = (bool) doSettingChooser(F(TXT_SHOW_DETAILS), actions, 2, (int) settings.silent) ;
}

void actionGraphing() {
    String actions[] = {F(TXT_ABSOLUTE_GRAPH), F(TXT_CENTERED_GRAPH)};
    settings.graphType = doSettingChooser(F(TXT_SELECT_GRAPH_TYPE), actions, 2, settings.graphType) ;
}

void actionReset() {
    String actions[] ={ F(TXT_KEEP_SETTINGS), F(TXT_RESET_SETTINGS)};
    bool reset = doSettingChooser(F(TXT_FACTORY_RESET), actions, 2, 0);
    if(reset){
        resetToFactoryDefaultSettings();
        eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));     //store the data
        asm volatile ("  jmp 0");                                 //soft reset by jumping into the arduino boot loader directly
    }
}

void actionDelay() {
    settings.delayTime = doBasicSettingChanger(F(TXT_DELAY_US), 0, 1000, settings.delayTime, 10) ;
}


void actionResponsiveness() {
    settings.responsiveness = (uint8_t) doBasicSettingChanger(F(TXT_RESPONSE_PERC), 0, 100, settings.responsiveness, 5);
    stabilityThreshold = (100 - settings.responsiveness) / 100.00; //more than a certain % difference from the average
}

void actionDamping() {
    settings.damping = (uint8_t) doBasicSettingChanger(F(TXT_DAMPING_PERC), 0, 100, settings.damping, 5);
    alpha = calculateAlpha(settings.damping);
}


void actionRPMDamping() {
    settings.rpmDamping = (uint8_t) doBasicSettingChanger(F(TXT_RPM_DAMPING_PERC), 0, 100, settings.rpmDamping, 5);
    alphaRpm = calculateAlpha(settings.rpmDamping);
}


void actionThreshold() {
    settings.threshold = (uint8_t) doBasicSettingChanger(F(TXT_THRESHOLD), 0, 1023, settings.threshold, 10) ;
}



void actionCylinders() {
    settings.cylinders = (uint8_t) doBasicSettingChanger(F(TXT_CYLINDER_COUNT), 1, 4, settings.cylinders, 1);
    fixMaster();
}

void actionMaster() {
    settings.master = (uint8_t) doBasicSettingChanger(F(TXT_MASTER_CYLINDER), 1, 4, settings.master, 1);
    fixMaster();
}


void fixMaster(){
    if (settings.cylinders < settings.master){
        settings.master = settings.cylinders;
    }
}

void actionBrightnessButton(){
    String actions[] = {F(TXT_BRIGHTNESS), F(TXT_RPM_DISPLAY)};
    settings.button2 = doSettingChooser(F(TXT_BUTTON_2), actions, 2, (int) settings.button2) ;
}

void doUnits(){
    String actions[] = {F(TXT_RAW_VALUES),F(TXT_RAW_DESCENDING), F(TXT_MILLIBAR_HPA), F(TXT_MILLIBAR_HPA_DESC),
                        F(TXT_CM_MERCURY), F(TXT_CM_MERCURY_DESC), F(TXT_INCH_MERCURY), F(TXT_INCH_MERCURY_DESC)};
    settings.units = doSettingChooser(F(TXT_DISPLAY_UNITS), actions, 8, (int) settings.units) ;
}

void actionCalibrationMax() {
    settings.calibrationMax = doBasicSettingChanger(F(TXT_MAX_CALIBRATION), 16, 127, settings.calibrationMax, 16);
}

void doMaxZoom(){
    
    static uint8_t count = 5;
    
    if ((0 == settings.units)||(1 == settings.units)){
      String actions[] = { F(TXT_100_MAX), F("200"), F("300"), F("600"), F(TXT_1024_NO_ZOOM)  };
      settings.zoom = doSettingChooser(F(TXT_ZOOM_UNITS), actions, count, (int) settings.zoom) ;
    }
    
    if ((2 == settings.units) || (3 == settings.units)){
     String actions[] = { F(TXT_84_MAX), F("169.92"), F("254.88"), F("509.77"), F(TXT_870_NO_ZOOM)  };
      settings.zoom = doSettingChooser(F(TXT_ZOOM_MILLIBAR), actions, count, (int) settings.zoom) ;
    }
    
    if ((4 == settings.units) || (5 == settings.units)){
      String actions[] = { F(TXT_6_MAX), F("9.56"), F("19.12"), F("32.63"), F(TXT_65_NO_ZOOM)  };
      settings.zoom = doSettingChooser(F(TXT_ZOOM_CM_MERCURY), actions, count, (int) settings.zoom) ;
    }
    
    if ((6 == settings.units) || (7 == settings.units)){
      String actions[] = { F(TXT_2_MAX), F("5.02"), F("7.53"), F("15.05"), F(TXT_25_NO_ZOOM)  };
      settings.zoom = doSettingChooser(F(TXT_ZOOM_IN_MERCURY), actions, count, (int) settings.zoom) ;
    }

}

