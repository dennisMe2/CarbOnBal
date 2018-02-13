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
const char txt2Max[]				PROGMEM = "2.51 = max";
const char txt25NoZoom[]			PROGMEM = "25.69 = no zoom";
const char txt6Max[]				PROGMEM = "6.37 = max";
const char txt65NoZoom[]			PROGMEM = "65.25 = no zoom";
const char txt84Max[]				PROGMEM = "84.96 = max";
const char txt870NoZoom[]			PROGMEM = "870.00 = no zoom";
const char txtAdvanced[]			PROGMEM = "Advanced";
const char txtAdvancedMenu[]		PROGMEM = "Advanced Menu";
const char txtBasic[]				PROGMEM = "Basic";
const char txtAbsoluteGraph[]		PROGMEM = "Absolute";
const char txtBaudRate[]			PROGMEM = "Baud Rate";
const char txtBrightness[]			PROGMEM = "Brightness";
const char txtBrightnessButton[]	PROGMEM = "Brightness Button";
const char txtButton2[]				PROGMEM = "Button 2 function:";
const char txtCalibrateNow[]		PROGMEM = "Calibrate Now";
const char txtCalibration[]  		PROGMEM = "Calibration";
const char txtCalibrationBusy[]		PROGMEM = "Calibrating now,";
const char txtCalibrationBusy2[]	PROGMEM = "apply vacuum!";
const char txtCalibrationDone[]		PROGMEM = "Calibration Done";
const char txtCalibrationDump[]		PROGMEM = "Calibration Dump";
const char txtCenteredGraph[]		PROGMEM = "Centered";
const char txtClearCalibration[]	PROGMEM = "Clear Calibration";
const char txtCmMercury[]			PROGMEM = "cm Hg";
const char txtCmMercuryDesc[]		PROGMEM = "cm Hg Descending";
const char txtConnectSerial[]		PROGMEM = "Connect Serial monitor";
const char txtContrast[]			PROGMEM = "Contrast";
const char txtCylinderCount[]		PROGMEM = "Cylinder count";
const char txtDamping[]				PROGMEM = "Damping";
const char txtDampingPerc[] 		PROGMEM = "Damping: (%)";
const char txtDataTransfer[] 		PROGMEM = "Data Transfer";
const char txtDelayUs[]				PROGMEM = "Delay: (us)";
const char txtDetails[]				PROGMEM = "Details";
const char txtDisplay[]    			PROGMEM = "Display";
const char txtDisplayUnits[]		PROGMEM = "Display Units:";
const char txtDumpingSensorData[] 	PROGMEM = "Dumping Sensor Data ";
const char txtFactoryReset[]		PROGMEM = "'Factory' reset";
const char txtGraphType[]			PROGMEM = "Graph Type";
const char txtHardware[]   			PROGMEM = "Hardware";
const char txtInchMercury[]			PROGMEM = "Inches of mercury";
const char txtInchMercuryDesc[]		PROGMEM = "Inch. Hg desc.";
const char txtKeepSettings[]		PROGMEM = "Keep settings";
const char txtLess[]				PROGMEM = "Less";
const char txtLiveDataDump[]		PROGMEM = "Live Data Dump";
const char txtLoadSettings[] 		PROGMEM = "Load Settings";
const char txtLoading[]				PROGMEM = "Loading...";
const char txtLowestPressure[]		PROGMEM = "Lowest pressure";
const char txtMasterCylinder[]		PROGMEM = "Master Cylinder";
const char txtMaxAdjust[]			PROGMEM = "Max adjustment";
const char txtMaxCalibration[]		PROGMEM = "Max calibration";
const char txtMaxZoomRange[]		PROGMEM = "Max zoom range";
const char txtMillibarHpa[]			PROGMEM = "Millibar / hPa";
const char txtMillibarHpaDesc[]		PROGMEM = "mBar / hPa Desc.";
const char txtMinAdjust[]			PROGMEM = "Min Adjustment";
const char txtMore[]				PROGMEM = "More";
const char txtPressAnyKey[]  		PROGMEM = "Press key when done";
const char txtRawDescending[]		PROGMEM = "Raw, descending";
const char txtRawValues[]			PROGMEM = "Raw values";
const char txtResetSettings[]		PROGMEM = "Reset ALL settings";
const char txtResponsePerc[]		PROGMEM = "Response: (%)";
const char txtResponsiveness[]  	PROGMEM = "Responsiveness";
const char txtRpm[]					PROGMEM = " RPM";
const char txtRpmDamping[]     		PROGMEM = "RPM Damping";
const char txtRpmDampingPerc[]		PROGMEM = "RPM Damping: (%)";
const char txtRpmDisplay[]			PROGMEM = "RPM Display";
const char txtRpmScale[]			PROGMEM = "  1 2 3 4 5 6 7 8 9";
const char txtSampleDelayUs[]		PROGMEM = "Sample Delay (us)";
const char txtSaveSettings[] 		PROGMEM = "Save Settings";
const char txtSaving[]				PROGMEM = "Saving...";
const char txtSelectGraphType[]		PROGMEM = "Select Graph Type";
const char txtSensor2[]				PROGMEM = "Sensor 2";
const char txtSensor3[]				PROGMEM = "Sensor 3";
const char txtSensor4[]				PROGMEM = "Sensor 4";
const char txtSerialFooter[]		PROGMEM = "\n*END*\n";
const char txtSerialHeader[]		PROGMEM = "val\tS02\tS03\tS04\t";
const char txtSetCalibrationMax[] 	PROGMEM	= "Set Calibration Max";
const char txtSettings[]      		PROGMEM = "Settings";
const char txtShowDetails[]			PROGMEM = "Show Details";
const char txtSoftware[]    		PROGMEM = "Software";
const char txtThreshold[]			PROGMEM = "Threshold";
const char txtUncalibrated[] 			PROGMEM = "Uncalibrated: ";
const char txtUnits[]				PROGMEM = "Units";
const char txtWelcome[]     		PROGMEM = " WELCOME ";
const char txtWiping[]				PROGMEM = "Wiping...";
const char txtZoomCmMercury[]		PROGMEM = "Zoom in cm Mercury:";
const char txtZoomInMercury[]		PROGMEM = "Zoom in in Mercury:";
const char txtZoomMillibar[]		PROGMEM = "Zoom millibar:";
const char txtZoomUnits[]			PROGMEM = "Zoom Units:";


// these probably won't need translating but you never know...
const char txt200[]					PROGMEM = "200";
const char txt300[]					PROGMEM = "300";
const char txt600[]					PROGMEM = "600";
const char txt1200[]				PROGMEM = "1200";
const char txt2400[]				PROGMEM = "2400";
const char txt4800[]				PROGMEM = "4800";
const char txt9600[]				PROGMEM = "9600";
const char txt14400[]				PROGMEM = "14400";
const char txt19200[]				PROGMEM = "19200";
const char txt28800[]				PROGMEM = "28800";
const char txt31250[]				PROGMEM = "31250";
const char txt38400[]				PROGMEM = "38400";
const char txt57600[]				PROGMEM = "57600";
const char txt115200[]				PROGMEM = "115200";

const char txt169[]					PROGMEM = "169.92";
const char txt254[]					PROGMEM = "254.88";
const char txt509[]					PROGMEM = "509.77";

const char txt9[]					PROGMEM = "9.56";
const char txt19[]					PROGMEM = "19.12";
const char txt32[]					PROGMEM = "32.63";
const char txt5[]					PROGMEM = "5.02";
const char txt7[]					PROGMEM = "7.53";
const char txt15[]        PROGMEM = "15.05";

#endif
