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
const char txt1024NoZoom[]			PROGMEM = "1024 = geen zoom";
const char txt25NoZoom[]			PROGMEM = "25.69 = geen zoom";
const char txt2Max[]				PROGMEM = "2.51 = max";
const char txt65NoZoom[]			PROGMEM = "65.25 = geen zoom";
const char txt6Max[]				PROGMEM = "6.37 = max";
const char txt84Max[]				PROGMEM = "84.96 = max";
const char txt870NoZoom[]			PROGMEM = "870.00 = geen zoom";
const char txtAbsoluteGraph[]		PROGMEM = "Absoluut";
const char txtAdvancedMenu[]		PROGMEM = "Geavanceerd Menu";
const char txtArduinoMode[]			PROGMEM = "Arduino Modus";
const char txtAveragingMethod[]		PROGMEM = "Middelingsmethode";
const char txtBaudRate[]			PROGMEM = "Baud Snelheid";
const char txtBrightness[]			PROGMEM = "Helderheid";
const char txtBrightnessButton[]	PROGMEM = "Neer / Rechts Knop";
const char txtButton2[]				PROGMEM = "Knop 2 functie:";
const char txtCalibrateNow[]		PROGMEM = "Sensoren Calibreren";
const char txtCalibrateSensor2[]	PROGMEM = "Calibreer Sensor 2";
const char txtCalibrateSensor3[]	PROGMEM = "Calibreer Sensor 3";
const char txtCalibrateSensor4[]	PROGMEM = "Calibreer Sensor 4";
const char txtCalibration[]  		PROGMEM = "Calibratie";
const char txtCalibrationBusy[]		PROGMEM = "Calibreert nu,";
const char txtCalibrationBusy2[]	PROGMEM = "Vacuum aansluiten!";
const char txtCalibrationDone[]		PROGMEM = "Calibratie Gereed";
const char txtCalibrationDump[]		PROGMEM = "Calibratie Zenden";
const char txtCenteredGraph[]		PROGMEM = "Gecentreerd";
const char txtClearAllSensors[]		PROGMEM = "Wis ALLE Sensoren";
const char txtClearCalibration[]  	PROGMEM = "Calibraries Wissen";
const char txtClearSensor2[]		PROGMEM = "Wis Sensor 2";
const char txtClearSensor3[]		PROGMEM = "Wis Sensor 3";
const char txtClearSensor4[]		PROGMEM = "Wis Sensor 4";
const char txtCmMercury[]			PROGMEM = "cm Hg";
const char txtCmMercuryDesc[]		PROGMEM = "cm Hg Dalend";
const char txtConnectSerial[]		PROGMEM = "Serieeel aansluiten";
const char txtContrast[]			PROGMEM = "Kontrast";
const char txtContrastButton[]		PROGMEM = "Op / Links Knop";
const char txtCylinderCount[]		PROGMEM = "Cylinder aantal";
const char txtDamping[]				PROGMEM = "Demping";
const char txtDampingPerc[] 		PROGMEM = "Demping: (%)";
const char txtDataTransfer[] 		PROGMEM = "Data Overzenden";
const char txtDelayUs[]				PROGMEM = "Vertraging: (us)";
const char txtDetails[]				PROGMEM = "Details";
const char txtDeviceInfo[]			PROGMEM = "Apparaat Info";
const char txtDisabled[]   			PROGMEM = "Niet actief";
const char txtDisplay[]    			PROGMEM = "Weergave";
const char txtDisplayUnits[]		PROGMEM = "Weergave Eenheden:";
const char txtDone[]				PROGMEM = "Gereed!";
const char txtDumpingSensorData[] 	PROGMEM = "Sensor Data Zenden ";
const char txtEnabled[]			   	PROGMEM = "Actief";
const char txtExtraMenu[]    		PROGMEM = "Extra";
const char txtFactoryReset[]		PROGMEM = "'Fabrieks' reset";
const char txtFreeRam[]    			PROGMEM = "Vrij RAM (B): ";
const char txtGraphType[]			PROGMEM = "Type Grafiek";
const char txtDiagnostic[]			PROGMEM = "Diagnostisch";
const char txtHardware[]   			PROGMEM = "Hardware";
const char txtInchMercury[]			PROGMEM = "Inches kwik";
const char txtInchMercuryDesc[]		PROGMEM = "Inch. Hg dalend.";
const char txtKeepSettings[]		PROGMEM = "Instellingen houden";
const char txtLcdSpeed[]    		PROGMEM = "LCD uS: ";
const char txtLiveDataDump[]		PROGMEM = "Live Data Zenden";
const char txtLowestPressure[]		PROGMEM = "Laagste druk";
const char txtMasterCylinder[]		PROGMEM = "Hoofd Cylinder";
const char txtMaxAdjust[]			PROGMEM = "Max bijstelling";
const char txtMaxCalibration[]		PROGMEM = "Max calibratie";
const char txtMaxZoomRange[]		PROGMEM = "Max zoom bereik";
const char txtMillibarHpa[]			PROGMEM = "Millibar / hPa";
const char txtMillibarHpaDesc[]		PROGMEM = "mBar / hPa Dalend.";
const char txtMinAdjust[]			PROGMEM = "Min Bijstelling";
const char txtPressAnyKey[]  		PROGMEM = "Gereed? Druk toets";
const char txtRawDescending[]		PROGMEM = "Ruw, dalend";
const char txtRawValues[]			PROGMEM = "Ruwe waarden";
const char txtResetSettings[]		PROGMEM = "Reset ALLE instell.";
const char txtResetAverages[]		PROGMEM = "Reset Meting";
const char txtResponsePerc[]		PROGMEM = "Reacte: (%)";
const char txtResponsiveness[]  	PROGMEM = "Reactievermogen";
const char txtRpm[]					PROGMEM = " TPM";
const char txtRpmDamping[]     		PROGMEM = "TPM Demping";
const char txtRpmDampingPerc[]		PROGMEM = "TPM Demping: (%)";
const char txtRpmDisplay[]			PROGMEM = "TPM Weergave";
const char txtRpmScale[]			PROGMEM = "  1 2 3 4 5 6 7 8 9";
const char txtSampleDelayUs[]		PROGMEM = "Sample Vertrag. (us)";
const char txtSelectGraphType[]		PROGMEM = "Kies Type Grafiek";
const char txtSerialFooter[]		PROGMEM = "\n*END*\n";
const char txtSerialHeader[]		PROGMEM = "val\tS02\tS03\tS04\t";
const char txtSetCalibrationMax[] 	PROGMEM	= "Calibratie Bereik";
const char txtSettings[]      		PROGMEM = "Instellingen";
const char txtSettingsBytes[]   	PROGMEM = "Instell. Bytes: ";
const char txtShowDetails[]			PROGMEM = "Toon Details";
const char txtSoftware[]    		PROGMEM = "Software";
const char txtSplashScreen[]   		PROGMEM = "Intro Scherm";
const char txtThreshold[]			PROGMEM = "Drempel";
const char txtUncalibrated[] 		PROGMEM = "Ongecalibreerd: ";
const char txtUnits[]				PROGMEM = "Eenheden";
const char txtVersion[]        		PROGMEM = "Versie Nr: ";
const char txtViewCalibrations[]	PROGMEM = "Calibraties inzien";
const char txtViewSensor2[]			PROGMEM = "Bekijk Sensor 2";
const char txtViewSensor3[]			PROGMEM = "Bekijk Sensor 3";
const char txtViewSensor4[]			PROGMEM = "Bekijk Sensor 4";
const char txtWelcome[]     		PROGMEM = " WELKOM ";
const char txtWiping[]				PROGMEM = "Wissen...";
const char txtZoomCmMercury[]		PROGMEM = "Zoom in cm Kwik:";
const char txtZoomInMercury[]		PROGMEM = "Zoom in inch Kwik:";
const char txtZoomMillibar[]		PROGMEM = "Zoom millibar:";
const char txtZoomUnits[]			PROGMEM = "Zoom Eenheden:";


const char txtPeakAverage[]			PROGMEM = "Piek Gem. (std)";
const char txtThresholdAverage[]	PROGMEM = "Drempel Gem.";
const char txtIIRAverage[]			PROGMEM = "IIR Gem.";
const char txtRunningAverage[]		PROGMEM = "Lopend Gem.";
const char txtIntRunningAverage[]	PROGMEM = "Int Lopend Gem.";


#endif
