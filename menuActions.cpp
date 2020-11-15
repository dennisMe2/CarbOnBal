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

#include LANGUAGE
#include "lang_generic.h"
#include "menu.h"
#include "lcdWrapper.h"
#include "utils.h"

extern settings_t settings;
extern bool quitMenu;
extern const uint8_t brightnessPin;//6
extern const uint8_t contrastPin;//11

void doConfirmation(){
	lcd_clear();
	lcd_setCursor(7,1);
	lcd_print(txtDone);
	waitForAnyKey();
}


void actionBrightness() {
    settings.brightness = doSettingChanger(txtBrightness, 0, 255, settings.brightness, 10, &doBrightness) ;
    actionSaveSettings();
    lcd_clear();
}



void doEmaFactor(int value){
	settings.damping = value/6;
}

void actionContrast() {
    settings.contrast = doSettingChanger(txtContrast, 0, 127, settings.contrast, 1, &doContrast) ;
    actionSaveSettings();
    lcd_clear();
}



void actionSilent() {
	const char* actions[] = {txtEnabled,txtDisabled};
    settings.silent = (bool) doSettingChooser(txtShowDetails, actions, 2, (int) settings.silent) ;
    actionSaveSettings();
}

void doSplashScreen(){
	const char* actions[] = {txtDisabled, txtEnabled};
	    settings.splashScreen = (bool) doSettingChooser(txtSplashScreen, actions, 2, (int) settings.splashScreen) ;
	    actionSaveSettings();
}

void doAdvanced() {
  bool oldAdvanced = settings.advanced;
	const char* actions[] = {txtDisabled, txtEnabled};
    settings.advanced = (bool) doSettingChooser(txtAdvancedMenu, actions, 2, (int) settings.advanced) ;
    actionSaveSettings();
    if(oldAdvanced != settings.advanced){
      quitMenu = true;
    }
}


void actionGraphing() {
	const char* actions[] = {txtAbsoluteGraph, txtCenteredGraph, txtDiagnostic};
    settings.graphType = doSettingChooser(txtSelectGraphType, actions, 3, settings.graphType) ;
    actionSaveSettings();
}

void actionReset() {
	const char* actions[] ={ txtKeepSettings, txtResetSettings};
    bool reset = doSettingChooser(txtFactoryReset, actions, 2, 0);
    if(reset){
        resetToFactoryDefaultSettings();
        actionSaveSettings();     //store the data
        asm volatile ("  jmp 0");                                 //soft reset by jumping into the arduino boot loader directly
    }
}


void actionEmaFactor() {
    settings.damping = (uint8_t) (doBasicSettingChanger(txtDampingPerc, 0, 100, (uint8_t) settings.damping*6.25, 6) / 6.25);
    actionSaveSettings();
}

void actionRpmEmaFactor() {
    settings.rpmDamping = (uint8_t) (doBasicSettingChanger(txtDampingPerc, 0, 100, (uint8_t) settings.rpmDamping*6.25, 6) / 6.25);
    actionSaveSettings();
}

void actionCylinders() {
    settings.cylinders = (uint8_t) doBasicSettingChanger(txtCylinderCount, 1, 4, settings.cylinders, 1);
    fixMaster();
    actionSaveSettings();
}

void actionMaster() {
    settings.master = (uint8_t) doBasicSettingChanger(txtMasterCylinder, 1, 4, settings.master, 1);
    fixMaster();
    actionSaveSettings();
}


void fixMaster(){
    if (settings.cylinders < settings.master){
        settings.master = settings.cylinders;
    }
}

void actionBrightnessButton(){
	const char* actions[] = {txtBrightness, txtRpmDisplay, txtRpmDamping};
    settings.button2 = doSettingChooser(txtBrightnessButton, actions, 3, (int) settings.button2) ;
    //if(settings.button2 == 2) settings.button1 = 2;

    actionSaveSettings();
}

void actionContrastButton(){
	const char* actions[] = {txtContrast, txtResetMeasurements, txtDamping};
    settings.button1 = doSettingChooser(txtContrastButton, actions, 3, (int) settings.button1) ;
    //if(settings.button1 == 2) settings.button2 = 2;

    actionSaveSettings();
}

void actionCancelButton(){
	const char* actions[] = {txtFreezeDisplay, txtResetMeasurements, txtRpmDisplay};
    settings.button3 = doSettingChooser(txtCancelButton, actions, 3, (int) settings.button3) ;

    actionSaveSettings();
}

void doUnits(){
    const char* actions[] = {txtRawValues,txtRawDescending, txtMillibarHpa, txtMillibarHpaDesc,
                        txtCmMercury, txtCmMercuryDesc, txtInchMercury, txtInchMercuryDesc};
    settings.units = doSettingChooser(txtDisplayUnits, actions, 8, (int) settings.units) ;
    actionSaveSettings();
}

void actionCalibrationMax() {
    settings.calibrationMax = doBasicSettingChanger(txtMaxCalibration, 16, 127, settings.calibrationMax, 16);
    actionSaveSettings();
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

    actionSaveSettings();
}


