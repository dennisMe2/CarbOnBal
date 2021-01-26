// This software, known as CarbOnBal is
// Copyright, 2017-2020 L.L.M. (Dennis) Meulensteen. dennis@meulensteen.nl
//
// This file is part of CarbOnBal. A combination of software and hardware.
// I hope it may be of some help to you in balancing your carburetors and throttle bodies.
// Always be careful when working on a vehicle or electronic project like this.
// Your life and health are your sole responsibility, use wisely.
//
// CarbOnBal hardware is covered by the CERN Open Hardware License v1.2
// a copy of the text is included with the source code.
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

#include "Arduino.h"
#include "EEPROM.h"

#include "functions.h"
#include "globals.h"
#include LANGUAGE
#include "lang_generic.h"
#include "lcdWrapper.h"
#include "menu.h"
#include "menuActions.h"
#include "utils.h"
settings_t settings;
uint8_t settingsOffset = 2;

// The software uses a lot of global variables. This may not be elegant but its one way of writing non-blocking code
int inputPin[NUM_SENSORS] = { A0, A1, A2, A3 }; //used as a means to access the sensor pins using a counter
int timeBase = 0; //allows us to calculate how long it took to measure and update the display (only used for testing)
long sums[NUM_SENSORS] = { 0, 0, 0, 0 }; //tracks totals for calculating a numerical average

bool freezeDisplay = false; //used to tell when a user wants to freeze the display
unsigned int rpm;                                       //stores the current RPM
unsigned int average[NUM_SENSORS]; //used to share the current average for each sensor
int ambientPressure; //stores current ambient pressure for negative pressure display
unsigned long lastUpdate;
int packetRequestCount = 0;
int emaTarget = -1;
unsigned long emaMillis = 0;
volatile longAverages avg[NUM_SENSORS];

uint8_t labelPosition = 0;

unsigned int serialValues[] = { 0, 0, 0, 0 };
unsigned long packetCounter = 0;
unsigned long startTime;
volatile unsigned long lastInterrupt = micros();
volatile unsigned long periodUs = 0;
volatile unsigned long interruptDurationUs = 0;
volatile bool isSerialAllowed = true;
bool dataDumpMode = false;

//this does the initial setup on startup.
void setup() {
	lcd_begin(DISPLAY_COLS, DISPLAY_ROWS);

	settings = loadSettings(settings); //load saved settings into memory from FLASH
	Serial.begin(BAUD_RATE);

	setInputActiveLow(SELECT);          //set the pins connected to the switches
	setInputActiveLow(LEFT);             //switches are wired directly and could
	setInputActiveLow(RIGHT);                //short out the pins if setup wrong
	setInputActiveLow(CANCEL);

	analogWrite(brightnessPin, settings.brightness);  //brightness is PWM driven
	analogWrite(contrastPin, settings.contrast); //contrast is PWM with smoothing (R/C network) to prevent flicker

	ambientPressure = detectAmbient(); //set ambient pressure (important because it varies with weather and altitude)

	//set timer1 interrupt at 1Hz
	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1 = 0; //initialize counter value to 0
	// set compare match register for 1hz increments
	OCR1A = 249;	// = (16*10^6) / (1*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS11 bits for 64 prescaler
	TCCR1B |= (1 << CS11) | (1 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	setInterrupt(true);

	delay(1000); // wait for serial control request after reset from java

	if (!Serial.available() && settings.splashScreen) { //only do demo if no serial data was sent
		doMatrixDemo();

		lcd_setCursor(4, 1);
		lcd_print(txtWelcome);
		delay(1000);

		lcd_clear();
		doAbsoluteDemo();
		doRelativeDemo();
	}

}
int sendDataOnRequest() {
	uint8_t command = 0x00;

	if ((Serial.available() >= 1)) {

		command = Serial.read();
		lcd_clear();
		lcd_setCursor(1, 1);

		if (command == CALIBRATION) {
			doCalibrationDump();
		} else if (command == SETTINGS) {
			doSettingsDump();
		} else if ((uint8_t) command == CARB_VACUUM) {
			//Do nothing here to prevent unwanted recursion
		} else if (command == DIAGNOSTICS) {
			//TODO implement
			//doDiagnosticsDump();
		} else {
			lcd_print(F(" UNKNOWN COMMAND?!?"));
			delay(1500);
		}
	}

	return command;
}

void doSerialReadCommand() {
	uint8_t command = sendDataOnRequest();
	if (isSerialAllowed) {

		if ((uint8_t) command == CARB_VACUUM) {
			doDataDump();
		}
	}
}

void doResetAveraging() {
	emaTarget = settings.damping;
	settings.damping = 0;
	emaMillis = millis();
}

void loop() {
	startTime = micros();

	if (emaTarget >= 0) {
		if ((millis() - emaMillis) >= 125) {
			if (emaTarget > settings.damping) {
				settings.damping++;
			} else {
				emaTarget = -1;
			}
			emaMillis = millis();
		}
	}

	doSerialReadCommand(); //reads commands from serial

	switch (buttonPressed()) {					//test if a button was pressed
	case SELECT:
		actionDisplayMainMenu();
		break;        //the menu is the only function that does not return asap
	case LEFT:
		if (settings.button1 == 0) { // there are three modes for this pin, user settable
			actionContrast();
		} else if (settings.button1 == 2) { //DAMPING
			settings.damping = (int8_t) (doBasicSettingChanger(txtDampingPerc,
					0, 100, (int8_t) settings.damping * 6.25, 6) / 6.25);
			actionSaveSettings();
		} else {
			doResetAveraging();
		}
		break;
	case RIGHT:
		if (settings.button2 == 0) { // there are three modes for this pin, user settable
			actionBrightness();
		} else if (settings.button2 == 2) { // RPMDAMPING
			settings.rpmDamping = (int8_t) (doBasicSettingChanger(
					txtRpmDampingPerc, 0, 100,
					(int8_t) settings.rpmDamping * 6.25, 6) / 6.25);
			actionSaveSettings();
		} else {
			doRevs();
		}
		break;

	case CANCEL:
		if (settings.button3 == 0) { // there are three modes for this pin, user settable
			freezeDisplay = !freezeDisplay;	//toggle the freezeDisplay option
		} else if (settings.button3 == 1) {
			freezeDisplay = false;
			doResetAveraging();
		} else {
			freezeDisplay = false;
			doRevs();
		}
		break;
	}

	if (!freezeDisplay
			&& (settings.graphType == 2 || (millis() - lastUpdate) > 100)) {//only update the display every 100ms or so to prevent flickering

		switch (settings.graphType) { //there are two types of graph. bar and centered around the value of the master carb
		case 0:
			lcdBarsSmooth(average);
			lastUpdate = millis();
			break;          //these functions both draw a graph and return asap
		case 1:
			lcdBarsCenterSmooth(average);
			lastUpdate = millis();
			break;    //
		case 2:
			if ((millis() - lastUpdate) > 100) {
				lcdDiagnosticDisplay(average);
				lastUpdate = millis();
			}
			break;
		}
	} else if (freezeDisplay) {
		//show a little snow flake to indicate the frozen state of the display
		drawSnowFlake();
	}
	timeBase = micros() - startTime; //monitor how long it takes to traverse the main loop
}

// Alternative basic algorithm using only long integer arithmetic
// now the definitive algorithm
void intRunningAverage() {
	unsigned int value;
	int factor = settings.damping;

	for (int sensor = 0; sensor < settings.cylinders; sensor++) { //loop over all sensors
		value = (unsigned int) readSensorCalibrated(sensor);
		avg[sensor].longVal = longExponentialMovingAverage(factor,
				avg[sensor].longVal, value);
		average[sensor] = avg[sensor].intVal[1];
	}
}

// display centered bars, centered on the reference carb's reading, because that's the target we are aiming for
//takes an array of current average values for all sensors as parameter
void lcdBarsCenterSmooth(unsigned int value[]) {
	const uint8_t segmentsInCharacter = 5; //we need this number to make the display smooth

	byte bar[4][8];                         //store one custom character per bar

	char bars[DISPLAY_COLS + 1];          //store for the bar of full characters

	unsigned int maximumValue = maxVal(value); //determine the sensor with the highest average
	unsigned int minimumValue = minVal(value); //determine the lowest sensor average
	int range;          //store the range between the highest and lowest sensors
	int zoomFactor;                              //store the zoom of the display

	const uint8_t hysteresis = 4;

	//the range depends on finding the reading farthest from the master carb reading
	if (maximumValue - value[settings.master - 1]
			>= value[settings.master - 1] - minimumValue) {
		range = maximumValue - value[settings.master - 1];
	} else {
		range = value[settings.master - 1] - minimumValue;
	}

	//sets the minimum range before the display becomes 'pixelated' there are 100 segments available, 50 on either side of the master
	int ranges[] = { 50, 100, 150, 300, 512 };
	if (range < ranges[settings.zoom]) {
		range = ranges[settings.zoom];
	}

	zoomFactor = range / 50;

	for (int sensor = 0; sensor < settings.cylinders; sensor++) { //for each of the sensors the user wants to use
		int delta = value[sensor] - value[settings.master - 1]; //find the difference between it and master
		int TotalNumberOfLitSegments = delta / zoomFactor; //determine the number of lit segments
		int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter; //determine the number of whole characters
		int numberOfLitSegments = TotalNumberOfLitSegments
				% segmentsInCharacter;   //determine the remaining stripes

		if (sensor != settings.master - 1) { //for all sensors except the master carb sensor
			makeCenterBars(bars, numberOfLitBars); //give us the bars of whole characters
			lcd_setCursor(0, sensor);
			lcd_print(bars);                     //place the bars in the display

			makeChar(bar[sensor], numberOfLitSegments); //make a custom char for the remaining stripes
			lcd_createChar(sensor + 2, bar[sensor]);

			if (numberOfLitSegments > 0) {
				lcd_setCursor(10 + numberOfLitBars, sensor); //place it on the right
			} else {
				lcd_setCursor(9 + numberOfLitBars, sensor); //or in the center
			}
			if (numberOfLitBars < 10)
				lcd_write(byte(sensor + 2));

			//hysteresis gives the display more stability and prevents the labels from flipping from side to side constantly.
			if (!settings.silent) {
				if (numberOfLitBars < -hysteresis)
					labelPosition = 15;
				if (numberOfLitBars > hysteresis)
					labelPosition = 0;
				printLcdSpace(labelPosition, sensor, 5);
				lcd_printFormatted(differenceToPreferredUnits(delta)); //display the difference between this sensor and master
			}
		} else {
			float result = convertToPreferredUnits(value[sensor],
					ambientPressure);
			printLcdSpace(0, sensor, 5);
			lcd_printFormatted(result); //print the absolute value of the reference carb
			printLcdInteger(timeBase, 10, sensor, 4); //show how long it took to measure four sensors
			printLcdSpace(15, sensor, 5);
			lcd_printFormatted(differenceToPreferredUnits(range * 2)); //show the zoom range
		}
	}
}

// this is used to display four plain non-zoomed bars with absolute pressure readings
void lcdBarsSmooth(unsigned int value[]) {
	const uint8_t segmentsInCharacter = 5;
	const uint8_t hysteresis = 4;

	byte bar[4][8];
	char bars[DISPLAY_COLS + 1];

	for (int sensor = 0; sensor < settings.cylinders; sensor++) {
		int TotalNumberOfLitSegments = 100000L / 1024 * value[sensor] / 1000; // integer math works faster, so we multiply by 1000 and divide later, powers of two would be even faster
		int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;
		int numberOfLitSegments = TotalNumberOfLitSegments
				% segmentsInCharacter;

		makeBars(bars, numberOfLitBars, 0); //skip function probably no longer needed
		lcd_setCursor(0, sensor);
		lcd_print(bars);

		makeChar(bar[sensor], numberOfLitSegments);
		lcd_createChar(sensor + 2, bar[sensor]);
		lcd_setCursor(numberOfLitBars, sensor);
		lcd_write(byte(sensor + 2));

		if (!settings.silent) {
			if (numberOfLitBars < 10 - hysteresis)
				labelPosition = 14;
			if (numberOfLitBars > 10 + hysteresis)
				labelPosition = 0;
			printLcdSpace(labelPosition, sensor, 5);
			float result = convertToPreferredUnits(value[sensor],
					ambientPressure);
			lcd_printFormatted(result);
		}
	}
}

void lcdDiagnosticDisplay(unsigned int value[]) {
	for (int sensor = 0; sensor < settings.cylinders; sensor++) {
		float result = convertToPreferredUnits(value[sensor], ambientPressure);
		printLcdSpace(0, sensor, 5);
		lcd_printFormatted(result);		//raw value

		printLcdSpace(8, sensor, 5);
		int delta = value[sensor] - value[settings.master - 1];
		lcd_printFormatted(differenceToPreferredUnits(delta));  //delta value
	}
	printLcdInteger(timeBase, 15, 0, 5); //time base
	printLcdInteger(periodUs, 15, 1, 5); //interrupt freq in uS
	printLcdInteger(interruptDurationUs, 15, 2, 5);
	printLcdInteger(packetCounter, 15, 3, 5); //serial packetrs sent

}

//compares freshly loaded settings to the freshly saved verion, if there is a difference the save must have failed
//fail on write is the most common NVRAM failure by far
bool verifySettings() {
	settings_t settingsCopy = settings;
	settings = loadSettings(settingsCopy);
	return memcmp(&settings, &settingsCopy, sizeof(settings));
}

//saves our settings struct
void actionSaveSettings() {
	EEPROM.put(0, versionUID);  //only saves changed bytes!
	EEPROM.put(1, settingsOffset);
	EEPROM.put(settingsOffset, settings);  //only saves changed bytes!
	delay(100);  //eeprom settle time

	//Move our settings up 1 position and retry while memory lasts!
	if (0 != verifySettings()) {
		if (settingsOffset + sizeof(settings) < 255)
			settingsOffset += sizeof(settings);
		EEPROM.put(1, settingsOffset);
		actionSaveSettings();
		lcd_clear();
		lcd_setCursor(0, 1);
		lcd_print(F("SETTINGS WRITE ERROR"));
		lcd_setCursor(0, 2);
		lcd_print(F("SETTINGS RELOCATED"));
		waitForAnyKey();
	}
}

//loads the settings from EEPROM (Flash)
settings_t loadSettings(settings_t settings) {
	uint8_t compareVersion = 0;
	EEPROM.get(0, compareVersion);
	EEPROM.get(1, settingsOffset);

	if (compareVersion == versionUID) { //only load settings if saved by the current version, otherwise reset to 'factory' defaults
		settings = EEPROM.get(settingsOffset, settings);
	} else {
		settingsOffset = 2;
		settings = fetchFactoryDefaultSettings();
	}

	doContrast(settings.contrast);
	doBrightness(settings.brightness);
	return settings;
}

//does the display while clearing the calibration array
void doZeroCalibrations() {
	lcd_clear();
	lcd_setCursor(3, 1);
	lcd_print(txtWiping);
	zeroCalibrations();
	doConfirmation();
}

//determine where the calibration value is stored in EEPROM depending on the sample value
int getCalibrationTableOffsetByValue(int sensor, int value) {
	return calibrationOffset + ((sensor - 1) * numberOfCalibrationValues)
			+ (value >> 2);
}

//determine where in EEPROM the calibration value is stored depending on the position
int getCalibrationTableOffsetByPosition(int sensor, int pos) {
	return calibrationOffset + ((sensor - 1) * numberOfCalibrationValues) + pos;
}

//only write if the value needs writing (saves write cycles)
void eepromWriteIfChanged(int address, int8_t data) {
	if ((uint8_t) data != EEPROM.read(address)) {
		EEPROM.write(address, (uint8_t) data); 		//write the data to EEPROM
	}
}

int readSensorRaw(int sensor) {
	//dummy read did nothing measurable, skip that nonsense
	return (analogRead(inputPin[sensor]));
}
int readSensorCalibrated(int sensor) {
	int value = readSensorRaw(sensor);
	if (sensor > 0) { //only for the calibrated sensors, not the master
		value += (int8_t) EEPROM.read(
				getCalibrationTableOffsetByValue(sensor, value)); //adds this reading adjusted for calibration
	}
	return value;
}

//clear the flash for a single sensor
void doClearCalibration(int sensor) {
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		eepromWriteIfChanged(getCalibrationTableOffsetByPosition(sensor, i), 0); //write the data directly to EEPROM
	}
}

//actually clears the flash for all the sensors
void zeroCalibrations() {
	for (uint8_t sensor = 1; sensor < (NUM_SENSORS); sensor++) {
		doClearCalibration(sensor);
	}
}

void doClearCalibration1() {
	doClearCalibration(1);
	doConfirmation();
}
void doClearCalibration2() {
	doClearCalibration(2);
	doConfirmation();
}
void doClearCalibration3() {
	doClearCalibration(3);
	doConfirmation();
}

void doViewCalibration1() {
	doViewCalibration(1);
}
void doViewCalibration2() {
	doViewCalibration(2);
}
void doViewCalibration3() {
	doViewCalibration(3);
}

void doCalibrate1() {
	doCalibrate(1);
}
void doCalibrate2() {
	doCalibrate(2);
}
void doCalibrate3() {
	doCalibrate(3);
}
void doCalibrate(int sensor) {
	const int shift = 5;
	const int factor = 4;
	int maxValue = -127;
	int minValue = 127;
	int lowestCalibratedValue = 1024;
	int readingStandardPre, readingSensor, readingStandardPost;
	setInterrupt(false);

	lcd_clear();

	lcd_setCursor(0, 0);
	lcd_print(txtCalibrationBusy);

	lcd_setCursor(0, 1);
	lcd_print(txtCalibrationBusy2);

	lcd_setCursor(0, 3);
	lcd_print(txtPressAnyKey);
	displayKeyPressPrompt();

	delay(500); //otherwise key still pressed, probably need a better solution

	//initialize temp values array, note full ints (16 bits) used
	int values[numberOfCalibrationValues];

	//read existing values from EEPROM and pre-shift them
	//shifting an int left by n bits simply gives us n bits of 'virtual' decimal places
	// this is needed for accuracy because EMA calculation works by adding or subtracting relatively small values
	// which would otherwise all be truncated to '0'
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		values[i] = (int8_t) EEPROM.read(
				getCalibrationTableOffsetByPosition(sensor, i));
		values[i] <<= shift;
	}

	while (!buttonPressed()) {
		readingStandardPre = readSensorRaw(0);  //read master
		readingSensor = readSensorRaw(sensor); //read calibration sensor
		readingStandardPost = readSensorRaw(0);  //read master again

		int readingStandard = (readingStandardPre + readingStandardPost) >> 1; //average both to increase accuracy on slopes
		int calibrationValue = readingStandard - readingSensor;

		//record some basic quality statistics
		if (calibrationValue > maxValue)
			maxValue = calibrationValue;
		if (calibrationValue < minValue)
			minValue = calibrationValue;
		if (readingSensor < lowestCalibratedValue)
			lowestCalibratedValue = readingSensor;

		values[(readingSensor >> 2)] = intExponentialMovingAverage(shift,
				factor, values[(readingSensor >> 2)], calibrationValue);
	}

	//post_shift the values in preparation of writing back to EEPROM
	// we don't need to save the 'decimal places' because they are not needed anymore.
	// so we lose them by shifting them out of range to the right
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		values[i] >>= shift;
	}

	//save calibrations
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		eepromWriteIfChanged(getCalibrationTableOffsetByPosition(sensor, i),
				(int8_t) values[i]);
	}

	lcd_clear();

	lcd_setCursor(0, 0);
	lcd_print(txtCalibrationDone);

	lcd_setCursor(0, 1);
	lcd_print(txtLowestPressure);
	printLcdInteger(lowestCalibratedValue, 15, 1, 4);
	lcd_setCursor(0, 2);
	lcd_print(txtMinAdjust);
	printLcdInteger(minValue, 16, 2, 3);
	lcd_setCursor(0, 3);
	lcd_print(txtMaxAdjust);
	printLcdInteger(maxValue, 16, 3, 3);

	waitForAnyKey();
	displayCalibratedValues(values);
	setInterrupt(true);
}

void doViewCalibration(int sensor) {
	setInterrupt(false);
	int values[numberOfCalibrationValues];

	for (int i = 0; i < numberOfCalibrationValues; i++) {
		values[i] = (int8_t) EEPROM.read(
				getCalibrationTableOffsetByPosition(sensor, i));
	}

	displayCalibratedValues(values);
	setInterrupt(true);
}

//display indicator arrows and numeric offsets so we don't get lost in the graph of calibration values.
void displayNavArrowsAndOffsets(int valueOffset,
		bool topLeftArrowPositionAvailable,
		bool topRightArrowPositionAvailable) {
	if (valueOffset == 0) {
		(topLeftArrowPositionAvailable) ?
				lcd_setCursor(0, 0) : lcd_setCursor(0, 3);
		lcd_printChar('[');
		lcd_printInt(valueOffset);
	}
	if (valueOffset > 0) {
		(topLeftArrowPositionAvailable) ?
				lcd_setCursor(0, 0) : lcd_setCursor(0, 3);
		lcd_printChar(char(MENUCARET + 1)); //little arrow to the left
		lcd_printInt(valueOffset);
	}
	if (valueOffset == numberOfCalibrationValues - 20) {
		(topRightArrowPositionAvailable) ?
				lcd_setCursor(16, 0) : lcd_setCursor(16, 3);
		lcd_printInt(valueOffset + 20);
		lcd_printChar(']');
	}
	if (valueOffset < numberOfCalibrationValues - 20) {
		(topRightArrowPositionAvailable) ?
				lcd_setCursor(16, 0) : lcd_setCursor(16, 3);
		if ((valueOffset + 20) < 100)
			lcd_printChar(' ');

		lcd_printInt(valueOffset + 20);
		lcd_printChar(char(MENUCARET)); //little arrow to the right
	}
}

// Show a graph of the computed calibration values so the user can get an idea of the quality of the sensors
// and of the calibration. Repeated calibration increases the accuracy.
// Note: if all sensors are showing the same type of displacement that means that sensor 0
// is off by that much in the opposite direction.
void displayCalibratedValues(int values[]) {
	int valueOffset = 0;
	int numberOfColumns = 20;
	int segmentsPerCharacter = 8;
	int numberOfCharacters = 4;
	int numberOfSegments = segmentsPerCharacter * (numberOfCharacters / 2);
	int valuePerSegment = settings.calibrationMax / numberOfSegments; //128 / 16 = 8
	int pressedButton = 0;
	bool dataChanged = true;
	bool topLeftArrowPositionAvailable, topRightArrowPositionAvailable = true;
	makeCalibrationChars();

	while (pressedButton != CANCEL) {
		if (dataChanged) {
			lcd_clear();
			for (int column = 0; column < numberOfColumns; column++) {
				int valueInSegments = values[valueOffset + column]
						/ valuePerSegment;

				if (valueInSegments <= segmentsPerCharacter
						&& valueInSegments > 0) {
					lcd_setCursor(column, 1);
					lcd_write(byte((byte) 8 - valueInSegments));
				} else if (valueInSegments > 2 * segmentsPerCharacter) {
					lcd_setCursor(column, 0);
					lcd_printChar('|');
					lcd_setCursor(column, 1);
					lcd_printChar('|');
				} else if (valueInSegments > segmentsPerCharacter) {
					lcd_setCursor(column, 0);
					lcd_write(
							byte(
									(byte) 8
											- (valueInSegments
													% segmentsPerCharacter)));
				} else if (valueInSegments < (2 * -segmentsPerCharacter)) {
					lcd_setCursor(column, 2);
					lcd_printChar('|');
					lcd_setCursor(column, 3);
					lcd_printChar('|');
				} else if (valueInSegments < 0
						&& (valueInSegments >= -segmentsPerCharacter)) {
					lcd_setCursor(column, 2);
					lcd_write(byte((byte) (-valueInSegments) - 1));
				} else if (valueInSegments < 0
						&& (valueInSegments < -segmentsPerCharacter)) {
					lcd_setCursor(column, 3);
					lcd_write(
							byte(
									(byte) 8
											- ((valueInSegments + 1)
													% segmentsPerCharacter)));
				}

				if (column == 0)
					topLeftArrowPositionAvailable = (valueInSegments <= 0);
				if (column == 19)
					topRightArrowPositionAvailable = (valueInSegments <= 0);

			}

			//show arrows to indicate scrolling and our location in the calibration array
			displayNavArrowsAndOffsets(valueOffset,
					topLeftArrowPositionAvailable,
					topRightArrowPositionAvailable);
		}

		//allow the user to scroll through the values from left to right and vice versa
		pressedButton = buttonPressed();
		if ((pressedButton == LEFT) && (valueOffset > 20)) {
			valueOffset -= 20;
			dataChanged = true;
		} else if ((pressedButton == LEFT) && (valueOffset <= 20)) {
			valueOffset = 0;
			dataChanged = true;
		} else if ((pressedButton == RIGHT)
				&& (valueOffset < numberOfCalibrationValues - 20 - 20)) {
			valueOffset = (valueOffset + 20);
			dataChanged = true;
		} else if ((pressedButton == RIGHT)
				&& (valueOffset >= numberOfCalibrationValues - 20 - 20)) {
			valueOffset = (numberOfCalibrationValues - 20);
			dataChanged = true;
		} else {
			dataChanged = false;
		}
	}
}

//create special characters in LCD memory these contain the horizontal lines
// we use to generate a graph of our calibration data
// using simple lines instead of full bars means we can use the same characters above and below zero
// because we only have 8 special chars, we would run out if we used bars!
void createSpecialCharacter(int number) {
	byte specialCharacter[8];

	for (int i = 0; i < 8; i++) {
		specialCharacter[i] = 0b00000;
	}
	specialCharacter[number - 1] = 0b11111;
	lcd_createChar(number - 1, specialCharacter);
}

// we need 8 characters to use each line in a 5x8 pixel character cell
void makeCalibrationChars() {
	for (int i = 1; i <= 8; i++) {
		createSpecialCharacter(i);
	}
}

void sendStartSerialData(byte dataType) {
	Serial.write(dataType);
}

void sendEndSerialData(uint8_t counter) {
	Serial.write(counter);
	Serial.write(START_PACKET);
	Serial.write(END_DATA);
}

//dump the calibration array to the serial port for review
void doCalibrationDump() {
	setInterrupt(false);

	for (int i = 0; i < numberOfCalibrationValues; i++) {
		uint8_t counter = 0;
		sendStartSerialData(CALIBRATION);
		Serial.write((int8_t) i);
		counter++;
		for (uint8_t sensor = 1; sensor < (NUM_SENSORS); sensor++) {
			Serial.write(
					(int8_t) EEPROM.read(
							getCalibrationTableOffsetByPosition(sensor, i)));
			counter++;
		}
		sendEndSerialData(counter);
	}

	setInterrupt(true);
}

void doSettingsDump() {
	uint8_t *ptr = (uint8_t*) &settings;
	uint8_t counter = 0;
	setInterrupt(false);
	sendStartSerialData(SETTINGS);
	Serial.write((int8_t) versionUID);
	counter++;
	for (size_t i = 0; i < sizeof(settings); i++) {
		Serial.write(*ptr);
		ptr++;
		counter++;
	}
	sendEndSerialData(counter);
	setInterrupt(true);
}

void serialWriteInteger(intByteUnion value) {
	Serial.write(value.byteVal[1]);
	Serial.write(value.byteVal[0]);
}

//dump calibrated sensor data directly to serial
void doDataDump() {
	dataDumpMode = true;
	doDataDumpBinary();
	dataDumpMode = false;
}

void doDataDumpBinary() {
	intByteUnion intVals;

	lcd_clear();
	lcd_setCursor(0, 1);
	lcd_print(txtConnectSerial);
	//Serial.begin(BAUD_RATE);
	setInterrupt(true);

	if (Serial.availableForWrite()) {
		lcd_setCursor(0, 1);
		lcd_print(txtDumpingSensorData);

		while (!(buttonPressed() == CANCEL)) { //loop while interrupt routine gathers data
			if (isSerialAllowed) {
				sendStartSerialData(CARB_VACUUM);
				int counter = 0;
				for (uint8_t sensor = 0; sensor < (NUM_SENSORS); sensor++) {
					intVals.intVal = average[sensor];
					serialWriteInteger(intVals);
					counter += 2; //2 byte integers!
				}
				sendEndSerialData(counter);
				sendDataOnRequest();
			}
			isSerialAllowed = false;
		}
	}
	setInterrupt(false);
}

void doRevs() {
	setInterrupt(false);
	int hysteresis = 2; //number of descending or ascending measurements needed
	bool descending = false;
	bool previousDescending = false;

	unsigned long peak = millis();
	unsigned int delta;
	unsigned long previousPeak = millis();
	unsigned long lastUpdateTime = millis();

	int measurement = 1024;
	int previous = 1024;
	int descentCount = 0;
	int ascentCount = 0;
	int factor = settings.rpmDamping;
	longAverages rpmAverage;

	initRpmDisplay();

	while (!(buttonPressed() == CANCEL)) {

		measurement = readSensorRaw(0);

		if (measurement < previous)
			descentCount++;
		if (measurement > previous)
			ascentCount++;

		if (descentCount >= hysteresis) {
			descending = true;
			ascentCount = 0;
			descentCount = 0;
		}

		if (ascentCount >= hysteresis) {
			descending = false;
			descentCount = 0;
			ascentCount = 0;
		}

		if (previousDescending == true && descending == false) {

			peak = millis();
			delta = peak - previousPeak;
			if (delta < 1) {
				rpm = 0;
			} else {
				rpmAverage.longVal = longExponentialMovingAverage(factor,
						rpmAverage.longVal, 120000 / delta);
				rpm = rpmAverage.intVal[1];
			}
			previousPeak = peak;
		}

		previousDescending = descending;
		previous = measurement;

		if (millis() - peak > 2000)
			rpm = 0;

		if (millis() - lastUpdateTime > 200L) {
			updateRpmDisplay(rpm);
			lastUpdateTime = millis();
		}
	}
	lcd_clear();
	setInterrupt(true);
}

void initRpmDisplay() {
	lcd_clear();
	lcd_setCursor(6, 0);
	lcd_print(txtRpm);
	lcd_setCursor(0, 1);
	lcd_print(txtRpmScale);
}

void updateRpmDisplay(unsigned int rpm) {

	char bars[DISPLAY_COLS + 1];
	byte bar[8];
	uint8_t TotalNumberOfLitSegments = 1000000L / 10000 * rpm / 10000;
	uint8_t numberOfLitBars = TotalNumberOfLitSegments / 5;
	uint8_t numberOfLitSegments = TotalNumberOfLitSegments % 5;

	makeBars(bars, numberOfLitBars, 0);
	makeChar(bar, numberOfLitSegments);
	lcd_createChar(6, bar);

	printLcdInteger(rpm, 0, 0, 6);

	lcd_setCursor(0, 2);
	lcd_print(bars);
	if (numberOfLitBars < DISPLAY_COLS) {
		lcd_setCursor(numberOfLitBars, 2);
		lcd_write(byte(6));
	}
	lcd_setCursor(0, 3);
	lcd_print(bars);
	if (numberOfLitBars < DISPLAY_COLS) {
		lcd_setCursor(numberOfLitBars, 3);
		lcd_write(byte(6));
	}

}

void makeCenterBars(char *bars, int8_t number) {
	if (number > 10)
		number = 10;
	if (number < -10)
		number = -10;

	if (number < 0) {
		for (int8_t i = 0; i < 10 + number; i++) {
			bars[i] = ' ';
		}
		for (int8_t i = 10 + number; i < 10; i++) {
			bars[i] = 0xff;
		}
		for (int8_t i = 10; i < 20; i++) {
			bars[i] = ' ';
		}
	}

	if (number == 0) {
		for (int8_t i = 0; i <= 20; i++) {
			bars[i] = ' ';
		}
	}

	if (number > 0) {
		for (int8_t i = 0; i <= 10; i++) {
			bars[i] = ' ';
		}
		for (int8_t i = 10; i <= 10 + number; i++) {
			bars[i] = 0xff;
		}
		for (int8_t i = 10 + number; i < 20; i++) {
			bars[i] = ' ';
		}
	}

	bars[DISPLAY_COLS] = 0x00;
}

//used to detect ambient pressure for readings that ascend with vacuum in stead of going down toward absolute 0 pressure
int detectAmbient() {
	unsigned long total = 0;
	uint8_t numberOfSamples = 200;        //set the number of samples to average

	for (int i = 0; i < numberOfSamples; i++) {

		total += readSensorRaw(0);                      //add the reading we got
	}
	return total / numberOfSamples; //divide the result by the number of samples for the resulting average
}

void doMatrixDemo() {
	uint8_t colspeed[20];
	char matrix[4][20];

	while (!buttonPressed()) {
		//set the column speeds
		for (int col = 0; col < 20; col++) {
			colspeed[col] = (rand() % 7) + 1;
		}
		for (int i = 0; i < 256; i++) {
			delay(50);
			if (buttonPressed())
				return;

			for (int col = 0; col < 20; col++) {

				if (i % colspeed[col] == 0) {

					lcd_setCursor(col, 3);
					lcd_printChar(matrix[2][col]);
					matrix[3][col] = matrix[2][col];

					lcd_setCursor(col, 2);
					lcd_printChar(matrix[1][col]);
					matrix[2][col] = matrix[1][col];

					lcd_setCursor(col, 1);
					lcd_printChar(matrix[0][col]);
					matrix[1][col] = matrix[0][col];

					lcd_setCursor(col, 0);
					matrix[0][col] = rand() % 256;
					lcd_printChar(matrix[0][col]);
				}
			}
		}
	}
}

int measureLCDSpeed() {
	unsigned long microseconds = micros();
	lcd_clear();
	for (int i = 0; i < 1000; i++) {
		lcd_setCursor(0, 1);
		lcd_printChar(' ');
	}
	return micros() - microseconds;
}

void doAbsoluteDemo() {
	bool silent = settings.silent;
	settings.silent = true;
	unsigned int values[4];

	for (int i = 0; i <= 1024; i += 30) {
		for (int j = 0; j < NUM_SENSORS; j++) {
			values[j] = i;
		}
		lcdBarsSmooth(values);
		if (buttonPressed())
			return;
	}
	for (unsigned int i = 1024; i >= 30; i -= 30) {
		for (int j = 0; j < NUM_SENSORS; j++) {
			values[j] = i;
		}
		lcdBarsSmooth(values);
		if (buttonPressed())
			return;
	}

	settings.silent = silent;
}
void doRelativeDemo() {
	unsigned int values[4];
	bool silent = settings.silent;
	int masterValue = 500;
	int delta = 100;
	uint8_t master = settings.master;
	settings.silent = true;
	settings.master = 4;
	for (int i = masterValue + delta; i >= masterValue - delta; i -= 4) {
		values[0] = i;
		values[1] = i;
		values[2] = i;
		values[3] = masterValue;
		lcdBarsCenterSmooth(values);
		if (buttonPressed())
			return;
	}
	settings.master = 1;
	for (int i = masterValue - delta; i <= masterValue + delta; i += 4) {
		values[0] = masterValue;
		values[1] = i;
		values[2] = i;
		values[3] = i;
		lcdBarsCenterSmooth(values);
		if (buttonPressed())
			return;
	}
	settings.master = master;
	settings.silent = silent;
}

void doDeviceInfo() {
	setInterrupt(false);
	unsigned long speed = measureLCDSpeed();
	lcd_setCursor(0, 0);
	lcd_print(txtVersion);
	lcd_print(F(SOFTWARE_VERSION));

	lcd_setCursor(0, 1);
	lcd_print(F("Sett: "));
	lcd_printInt((int) sizeof(settings));
	lcd_setCursor(9, 1);
	lcd_print(F("(B) >"));
	lcd_printInt(settingsOffset);

	lcd_setCursor(0, 2);
	lcd_print(txtFreeRam);
	lcd_printInt(freeMemory());

	lcd_setCursor(0, 3);
	lcd_print(txtLcdSpeed);
	lcd_printLong(speed);
	waitForAnyKey();
	setInterrupt(true);
}

ISR(TIMER1_COMPA_vect) {
	unsigned long microSecond = micros();
	periodUs = microSecond - lastInterrupt;
	lastInterrupt = microSecond;

	if (dataDumpMode) {
		for (int sensor = 0; sensor < settings.cylinders; sensor++) { //loop over all sensors
			average[sensor] = readSensorCalibrated(sensor);
		}
	} else {
		intRunningAverage();
	}
	interruptDurationUs = micros() - microSecond;
	isSerialAllowed = true;
}
