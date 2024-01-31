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

#ifndef LANG_H_INCLUDED
#define LANG_H_INCLUDED

//keep these alphabetical,helps to identify duplicates and find strings in a long list


const char txt100Max[]				PROGMEM = "100 = max";
const char txt1024NoZoom[]			PROGMEM = "1024 = no zoom";
const char txt25NoZoom[]			PROGMEM = "25.69 = no zoom";
const char txt2Max[]				PROGMEM = "2.51 = max";
const char txt65NoZoom[]			PROGMEM = "65.25 = no zoom";
const char txt6Max[]				PROGMEM = "6.37 = max";
const char txt84Max[]				PROGMEM = "84.96 = max";
const char txt870NoZoom[]			PROGMEM = "870.00 = no zoom";
const char txtAbsoluteGraph[]		PROGMEM = "Absolute";
const char txtAdvancedMenu[]		PROGMEM = "Advanced Menu";
const char txtBrightness[]			PROGMEM = "Brightness";
const char txtBrightnessButton[]	PROGMEM = "Down / Right button";
const char txtButton2[]				PROGMEM = "Button 2 function:";
const char txtButtons[]				PROGMEM = "Buttons";
const char txtCancelButton[]		PROGMEM = "Cancel Button";
const char txtCalibrateNow[]		PROGMEM = "Calibrate Sensors";
const char txtCalibrateSensor2[]	PROGMEM = "Calibrate Sensor 2";
const char txtCalibrateSensor3[]	PROGMEM = "Calibrate Sensor 3";
const char txtCalibrateSensor4[]	PROGMEM = "Calibrate Sensor 4";
const char txtCalibration[]  		PROGMEM = "Calibration";
const char txtCalibrationBusy[]		PROGMEM = "Calibrating now,";
const char txtContrastButton[]		PROGMEM = "Up / Left Button";
const char txtCalibrationBusy2[]	PROGMEM = "apply vacuum!";
const char txtCalibrationDone[]		PROGMEM = "Calibration Done";
const char txtCalibrationDump[]		PROGMEM = "Calibration Dump";
const char txtCenteredGraph[]		PROGMEM = "Centered";
const char txtDiagnostic[]			PROGMEM = "Diagnostic";
const char txtClearAllSensors[]		PROGMEM = "Clear All Sensors";
const char txtClearCalibration[]  	PROGMEM = "Clear Calibrations";
const char txtClearSensor2[]		PROGMEM = "Clear Sensor 2";
const char txtClearSensor3[]		PROGMEM = "Clear Sensor 3";
const char txtClearSensor4[]		PROGMEM = "Clear Sensor 4";
const char txtCmMercury[]			PROGMEM = "cm Hg";
const char txtCmMercuryDesc[]		PROGMEM = "cm Hg Descending";
const char txtConnectSerial[]		PROGMEM = "Connect Terminal";
const char txtContrast[]			PROGMEM = "Contrast";
const char txtCylinderCount[]		PROGMEM = "Cylinder count";
const char txtDamping[]				PROGMEM = "Damping";
const char txtDampingPerc[] 		PROGMEM = "Damping: (0..16)";
const char txtDataTransfer[] 		PROGMEM = "Data Transfer";
const char txtDelayUs[]				PROGMEM = "Delay: (us)";
const char txtDetails[]				PROGMEM = "Details";
const char txtDeviceInfo[]			PROGMEM = "Device Info";
const char txtDisabled[]   			PROGMEM = "Disabled";
const char txtDisplay[]    			PROGMEM = "Display";
const char txtDisplayUnits[]		PROGMEM = "Display Units:";
const char txtDone[]				PROGMEM = "Done!";
const char txtDumpingSensorData[] 	PROGMEM = "Dumping Sensor Data ";
const char txtEnabled[]				PROGMEM = "Enabled";
const char txtExtraMenu[]    		PROGMEM = "Extra";
const char txtFactoryReset[]		PROGMEM = "'Factory' reset";
const char txtFreeRam[]    			PROGMEM = "Free RAM (B): ";
const char txtFreezeDisplay[]		PROGMEM = "Freeze display ";
const char txtGraphType[]			PROGMEM = "Graph Type";
const char txtHardware[]   			PROGMEM = "Hardware";
const char txtInchMercury[]			PROGMEM = "Inches of mercury";
const char txtInchMercuryDesc[]		PROGMEM = "Inch. Hg desc.";
const char txtKeepSettings[]		PROGMEM = "Keep settings";
const char txtLcdSpeed[]    		PROGMEM = "LCD uS: ";
const char txtLiveDataDump[]		PROGMEM = "Live Data Dump";
const char txtLowestPressure[]		PROGMEM = "Lowest pressure";
const char txtMasterCylinder[]		PROGMEM = "Master Cylinder";
const char txtMaxAdjust[]			PROGMEM = "Max adjustment";
const char txtMaxCalibration[]		PROGMEM = "Max calibration";
const char txtMaxZoomRange[]		PROGMEM = "Max zoom range";
const char txtMillibarHpa[]			PROGMEM = "Millibar / hPa";
const char txtMillibarHpaDesc[]		PROGMEM = "mBar / hPa Desc.";
const char txtMinAdjust[]			PROGMEM = "Min Adjustment";
const char txtPressAnyKey[]  		PROGMEM = "Press key when done";
const char txtRawDescending[]		PROGMEM = "Raw, descending";
const char txtRawValues[]			PROGMEM = "Raw values";
const char txtResetSettings[]		PROGMEM = "Reset ALL settings";
const char txtResetMeasurements[]	PROGMEM = "Reset Measurement";
const char txtRpm[]					PROGMEM = " RPM";
const char txtRpmDamping[]     		PROGMEM = "RPM Damping";
const char txtRpmDampingPerc[]		PROGMEM = "RPM Damping: (0..16)";
const char txtRpmDisplay[]			PROGMEM = "RPM Display";
const char txtRpmScale[]			PROGMEM = "  1 2 3 4 5 6 7 8 9";
const char txtSelectGraphType[]		PROGMEM = "Select Graph Type";
const char txtSetCalibrationMax[] 	PROGMEM	= "Calibration Range";
const char txtSettings[]      		PROGMEM = "Settings";
const char txtSettingsBytes[]   	PROGMEM = "Settings Bytes: ";
const char txtShowDetails[]			PROGMEM = "Show Details";
const char txtSoftware[]    		PROGMEM = "Software";
const char txtSplashScreen[]   		PROGMEM = "Splash Screen";
const char txtUncalibrated[] 		PROGMEM = "Uncalibrated: ";
const char txtUnits[]				PROGMEM = "Units";
const char txtVersion[]        		PROGMEM = "Version Nr: ";
const char txtViewCalibrations[]	PROGMEM = "View Calibrations";
const char txtViewSensor2[]			PROGMEM = "View Sensor 2";
const char txtViewSensor3[]			PROGMEM = "View Sensor 3";
const char txtViewSensor4[]			PROGMEM = "View Sensor 4";
const char txtWelcome[]     		PROGMEM = " WELCOME ";
const char txtWiping[]				PROGMEM = "Wiping...";
const char txtZoomCmMercury[]		PROGMEM = "Zoom in cm Mercury:";
const char txtZoomInMercury[]		PROGMEM = "Zoom in in Mercury:";
const char txtZoomMillibar[]		PROGMEM = "Zoom millibar:";
const char txtZoomUnits[]			PROGMEM = "Zoom Units:";
const char txtIntRunningAverage[]	PROGMEM = "Int Running Avg.";


#endif
