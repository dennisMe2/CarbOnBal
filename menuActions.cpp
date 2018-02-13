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
	 const char* actions[] = {txt300, txt600, txt1200, txt2400, txt4800, txt9600, txt14400, txt19200, txt28800, txt31250, txt38400, txt57600, txt115200};
    settings.baudRate = doSettingChooser(txtBaudRate, actions, 13, settings.baudRate) ;
}

void actionBrightness() {
    settings.brightness = doSettingChanger(txtBrightness, 0, 255, settings.brightness, 10, &doBrightness) ;
}

void doBrightness(int value) {
    analogWrite(brightnessPin, value);
}

void actionContrast() {
    settings.contrast = doSettingChanger(txtContrast, 0, 127, settings.contrast, 1, &doContrast) ;
}

void doContrast(int value) {
    analogWrite(contrastPin, value);
}

void actionSilent() {
	const char* actions[] = {txtMore, txtLess};
    settings.silent = (bool) doSettingChooser(txtShowDetails, actions, 2, (int) settings.silent) ;
}

void doAdvanced() {
	const char* actions[] = {txtBasic, txtAdvanced};
    settings.advanced = (bool) doSettingChooser(txtAdvancedMenu, actions, 2, (int) settings.advanced) ;
}

void actionGraphing() {
	const char* actions[] = {txtAbsoluteGraph, txtCenteredGraph};
    settings.graphType = doSettingChooser(txtSelectGraphType, actions, 2, settings.graphType) ;
}

void actionReset() {
	const char* actions[] ={ txtKeepSettings, txtResetSettings};
    bool reset = doSettingChooser(txtFactoryReset, actions, 2, 0);
    if(reset){
        resetToFactoryDefaultSettings();
        eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));     //store the data
        asm volatile ("  jmp 0");                                 //soft reset by jumping into the arduino boot loader directly
    }
}

void actionDelay() {
    settings.delayTime = doBasicSettingChanger(txtDelayUs, 0, 1000, settings.delayTime, 10) ;
}


void actionResponsiveness() {
    settings.responsiveness = (uint8_t) doBasicSettingChanger(txtResponsePerc, 0, 100, settings.responsiveness, 5);
    stabilityThreshold = (100 - settings.responsiveness) / 100.00; //more than a certain % difference from the average
}

void actionDamping() {
    settings.damping = (uint8_t) doBasicSettingChanger(txtDampingPerc, 0, 100, settings.damping, 5);
    alpha = calculateAlpha(settings.damping);
}


void actionRPMDamping() {
    settings.rpmDamping = (uint8_t) doBasicSettingChanger(txtRpmDampingPerc, 0, 100, settings.rpmDamping, 5);
    alphaRpm = calculateAlpha(settings.rpmDamping);
}


void actionThreshold() {
    settings.threshold = (uint8_t) doBasicSettingChanger(txtThreshold, 0, 1023, settings.threshold, 10) ;
}



void actionCylinders() {
    settings.cylinders = (uint8_t) doBasicSettingChanger(txtCylinderCount, 1, 4, settings.cylinders, 1);
    fixMaster();
}

void actionMaster() {
    settings.master = (uint8_t) doBasicSettingChanger(txtMasterCylinder, 1, 4, settings.master, 1);
    fixMaster();
}


void fixMaster(){
    if (settings.cylinders < settings.master){
        settings.master = settings.cylinders;
    }
}

void actionBrightnessButton(){
	const char* actions[] = {txtBrightness, txtRpmDisplay};
    settings.button2 = doSettingChooser(txtButton2, actions, 2, (int) settings.button2) ;
}

void doUnits(){
    const char* actions[] = {txtRawValues,txtRawDescending, txtMillibarHpa, txtMillibarHpaDesc,
                        txtCmMercury, txtCmMercuryDesc, txtInchMercury, txtInchMercuryDesc};
    settings.units = doSettingChooser(txtDisplayUnits, actions, 8, (int) settings.units) ;
}

void actionCalibrationMax() {
    settings.calibrationMax = doBasicSettingChanger(txtMaxCalibration, 16, 127, settings.calibrationMax, 16);
}

void doMaxZoom(){
    
    static uint8_t count = 5;
    
    if ((0 == settings.units)||(1 == settings.units)){
    	const char* actions[] = { txt100Max, txt200, txt300, txt600, txt1024NoZoom  };
      settings.zoom = doSettingChooser(txtZoomUnits, actions, count, (int) settings.zoom) ;
    }
    
    if ((2 == settings.units) || (3 == settings.units)){
    	const char* actions[] = { txt84Max, txt169, txt254, txt509, txt870NoZoom  };
      settings.zoom = doSettingChooser(txtZoomMillibar, actions, count, (int) settings.zoom) ;
    }
    
    if ((4 == settings.units) || (5 == settings.units)){
    	const char* actions[] = { txt6Max, txt9, txt19, txt32, txt65NoZoom  };
      settings.zoom = doSettingChooser(txtZoomCmMercury, actions, count, (int) settings.zoom) ;
    }
    
    if ((6 == settings.units) || (7 == settings.units)){
    const char*  actions[] = { txt2Max, txt5, txt7, txt15, txt25NoZoom  };
      settings.zoom = doSettingChooser(txtZoomInMercury, actions, count, (int) settings.zoom) ;
    }

}

