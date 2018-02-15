// This software, known as CarbOnBal is
// Copyright, 2017 L.L.M. (Dennis) Meulensteen. dennis@meulensteen.nl
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


#include <Arduino.h>
#include <EEPROM.h>

#include "functions.h"
#include "globals.h"
#include "lang_en_gb.h"   //include British English texts
#include "lcdWrapper.h"
#include "menu.h"
#include "menuActions.h"
#include "utils.h"
settings_t settings;

// The software uses a lot of global variables. This may not be elegant but its one way of writing non-blocking code
float accumulator[NUM_SENSORS] = {1000.0, 1000.0, 1000.0, 1000.0}; //used to track the average values per sensor
int inputPin[NUM_SENSORS] = {A0, A1, A2, A3};               //used as a means to access the sensor pins using a counter
int timeBase = 0;                                           //allows us to calculate how long it took to measure and update the display (only used for testing)
long sums[NUM_SENSORS] = {0, 0, 0, 0};                      //tracks totals for calculating a numerical average

bool freezeDisplay = false;                                 //used to tell when a user wants to freeze the display
unsigned int rpm;                                           //stores the current RPM
float alpha;                                                //alpha is the math term for the weighting factor used to calculate
//EMA Exponentially weighted moving average. Using this we save a lot of precious memory
float alphaRpm;                                             //alpha factor used to smooth the RPM display
float stabilityThreshold;                                   //the factor used to detect when the throttle is being yanked and a response is required

int readingCount[NUM_SENSORS];                              //used to store the number of captured readings for calculating a numerical average
int average[NUM_SENSORS];                                   //used to share the current average for each sensor
int total[NUM_SENSORS];                                     //
int reading[NUM_SENSORS];                                   //the latest reading for each sensor

int ambientPressure;                                        //stores current ambient pressure for negative pressure display
unsigned long lastUpdate;

//this does the initial setup on startup.
void setup() {
	lcd_begin(DISPLAY_COLS, DISPLAY_ROWS);

	loadSettings();                                         //load saved settings into memory from FLASH
	Serial.begin(getBaud(settings.baudRate));

	setInputActiveLow(SELECT);                              //set the pins connected to the switches
	setInputActiveLow(LEFT);                                //switches are wired directly and could
	setInputActiveLow(RIGHT);                               //short out the pins if setup wrong
	setInputActiveLow(CANCEL);

	analogWrite(brightnessPin, settings.brightness);        //brightness is PWM driven
	analogWrite(contrastPin, settings.contrast);            //contrast is PWM with smoothing (R/C network) to prevent flicker

	ambientPressure = detectAmbient();                      //set ambient pressure (important because it varies with weather and altitude)
	alpha = calculateAlpha(settings.damping);               //prime the alpha from the settings
	alphaRpm = calculateAlpha(settings.rpmDamping);
	stabilityThreshold = (100 - settings.responsiveness) / 100; //responsiveness is how quickly the system responds to rapid RPM changes as opposed to smoothing the display

	if(settings.splashScreen){
		demo();

		lcd_setCursor(4,1);
		lcd_print(txtWelcome);
		delay(2000);

		lcd_clear();
		demo2();
		demo3();
	}

}

void loop() {

	switch ( buttonPressed()) {							//test if a button was pressed
	case SELECT: actionDisplayMainMenu(); break;        //the menu is the only function that does not return asap
	case LEFT: actionContrast(); break;
	case RIGHT:
		if (settings.button2 == 0) {                        // there are two modes for this pin, user settable
			actionBrightness();
		} else {
			doRevs();
		}
		break;

	case CANCEL: freezeDisplay = !freezeDisplay; break; //toggle the freezeDisplay option
	}

	runningAverage(); //calculate the running averages and return asap

	if (!freezeDisplay && ((millis() - lastUpdate) > 100)) {//only update the display every 100ms or so to prevent flickering
		lastUpdate=millis();
		switch ( settings.graphType) {                      //there are two types of graph. bar and centered around the value of the master carb
		case 0:  lcdBarsSmooth(average); break;          //these functions both draw a graph and return asap
		case 1:  lcdBarsCenterSmooth(average); break;    //
		}
	} else if(freezeDisplay){
		//make a little snow flake to indicate the frozen state of the display
		drawSnowFlake();
	}
}

//the main measurement function which calculates a true average of all samples while the signal drops below the threshold.
//all the sensors are sampled and values stored then the loop returns asap so the screen can be updated in real time
void runningAverage() {
	unsigned long startTime = millis();
	for (int sensor = 0; sensor < settings.cylinders; sensor++) {       //loop over all sensors
		readSensorRaw(sensor);
		if (reading[sensor] <= 1023 - settings.threshold ) {            //1023 is basically normal airpressure at sea level. it can be lower, especially in the mountains
			if (sensor != 0) { //only apply calibration for non reference sensors

				sums[sensor] += reading[sensor] + (int8_t) EEPROM.read(getCalibrationTableOffsetByValue(sensor, reading[sensor])); //adds this reading adjusted for calibration
			} else {
				sums[sensor] += reading[sensor];                        //sensor0 is the reference sensor, no calibration needed
			}
			readingCount[sensor]++;                                     //keeps count of the number of readings collected

		} else {                                                        //above the measurement threshold
			if (readingCount[sensor] > 0) {                             //immediately after the vacuum peak
				//calculate the average now we've measured a whole "vacuum signal trough"
				average[sensor] = responsiveEMA(alpha, &accumulator[sensor], sums[sensor] / readingCount[sensor]);
			}
			//done the calcs, now reset everything ready to start over when the signal drops below threshold
			sums[sensor] = 0;
			readingCount[sensor] = 0;
		}
	}
	timeBase = millis() - startTime;                                    //monitor how long it takes to measure 4 sensors
}

// display centered bars, centered on the reference carb's reading, because that's the target we are aiming for
//takes an array of current average values for all sensors as parameter
void lcdBarsCenterSmooth( int value[]) {
	const uint8_t segmentsInCharacter = 5;                              //we need this number to make the display smooth

	byte bar[4][8];                                                     //store one custom character per bar

	char bars[DISPLAY_COLS + 1];                                        //store for the bar of full characters

	int maximumValue = maxVal(value);                                   //determine the sensor with the highest average
	int minimumValue = minVal(value);                                   //determine the lowest sensor average
	int range;                                                          //store the range between the highest and lowest sensors
	int zoomFactor;                                                     //store the zoom of the display

	//the range depends on finding the reading furthest from the master carb reading
	if (maximumValue - value[settings.master - 1] >= value[settings.master - 1] - minimumValue) {
		range = maximumValue - value[settings.master - 1];
	} else {
		range = value[settings.master - 1] - minimumValue;
	}

	//sets the minimum range before the display becomes 'pixelated' there are 100 segments available, 50 on either side of the master
	int ranges[] = {50, 100, 150, 300, 512};
	if (range < ranges[settings.zoom]) {
		range = ranges[settings.zoom];
	}

	zoomFactor = range / 50;

	for (int sensor = 0 ; sensor < settings.cylinders; sensor++) { //for each of the sensors the user wants to use
		int delta = value[sensor] - value[settings.master - 1];     //find the difference between it and master
		int TotalNumberOfLitSegments = delta / zoomFactor;          //determine the number of lit segments
		int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;   //determine the number of whole characters
		int numberOfLitSegments = TotalNumberOfLitSegments % segmentsInCharacter;   //determine the remaining stripes

		if (sensor != settings.master - 1) {            //for all sensors except the master carb sensor
			makeCenterBars(bars, numberOfLitBars);      //give us the bars of whole characters
			lcd_setCursor(0, sensor);
			lcd_print(bars);                            //place the bars in the display

			makeChar(bar[sensor], numberOfLitSegments); //make a custom char for the remaining stripes
			lcd_createChar(sensor + 2, bar[sensor]);

			if (numberOfLitSegments > 0) {
				lcd_setCursor(10 + numberOfLitBars, sensor); //place it on the right
			} else {
				lcd_setCursor(9 + numberOfLitBars, sensor); //or in the center
			}
			if (numberOfLitBars < 10) lcd_write(byte(sensor + 2));

			if (!settings.silent) {
				if (numberOfLitBars < 0) {
					printLcdSpace(15, sensor, 5);           //clear the display in preparation of printing the readings
				} else {
					printLcdSpace(0, sensor, 5);
				}
				lcd_printFloat(differenceToPreferredUnits(delta) );                    //display the difference between this sensor and master
			}
		} else {
			float result = convertToPreferredUnits(value[sensor], ambientPressure);
			printLcdFloat(result, 0, sensor, 5);   //print the absolute value of the reference carb
			printLcdInteger(timeBase, 10, sensor, 4);       //show how long it took to measure four sensors
			printLcdFloat(differenceToPreferredUnits(range * 2), 15, sensor, 5);        //show the zoom range
		}
	}
}


// this is used to display four plain non-zoomed bars with absolute pressure readings
void lcdBarsSmooth( int value[]) {
	const uint8_t segmentsInCharacter = 5;

	byte bar[4][8];
	char bars[DISPLAY_COLS + 1];

	for (int sensor = 0 ; sensor < settings.cylinders; sensor++) {
		int TotalNumberOfLitSegments = 100000L / 1024 * value[sensor] / 1000; // integer math works faster, so we multiply by 1000 and divide later, powers of two would be even faster
		int numberOfLitBars = TotalNumberOfLitSegments / segmentsInCharacter;
		int numberOfLitSegments = TotalNumberOfLitSegments % segmentsInCharacter;

		//if (numberOfLitBars > 14) {
		//	makeBars(bars, numberOfLitBars, 4);
		//} else {
			makeBars(bars, numberOfLitBars, 0);  //skip function probably no longer needed
		//}
		lcd_setCursor(0, sensor);
		lcd_print(bars);

		makeChar(bar[sensor], numberOfLitSegments);
		lcd_createChar(sensor + 2, bar[sensor]);
		lcd_setCursor(numberOfLitBars, sensor);
		lcd_write(byte(sensor + 2));


		if (!settings.silent) {
			//set the cursor so the pressure readings don't interfere with the bars
			if (numberOfLitBars <= 12) {
				lcd_setCursor(14, sensor);
			} else {
				lcd_setCursor(0, sensor);
			}
			float result = convertToPreferredUnits(value[sensor], ambientPressure);
			lcd_printFloat(result);
		}
	}
}


//saves our settings struct
void actionSaveSettings() {
	lcd_clear();
	lcd_setCursor(3, 1);
	lcd_print(txtSaving);
	eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
	delay(500);
}

//handles the display for loading settings
void actionLoadSettings() {
	lcd_clear();
	lcd_setCursor(3, 1);
	lcd_print(txtLoading);
	loadSettings();
	delay(500);
}

//loads the settings from EEPROM (Flash)
void loadSettings() {
	uint8_t compareVersion = 0;
	eeprom_read_block((void*)&compareVersion, (void*)0, sizeof(settings.versionID));

	//only load settings if saved by the current version, otherwise reset to 'factory' defaults
	if (compareVersion == versionUID) {
		eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
	} else {
		resetToFactoryDefaultSettings();
	}

	doContrast(settings.contrast);
	doBrightness(settings.brightness);
}

//does the display while clearing the calibration array
void doZeroCalibrations() {
	lcd_clear();
	lcd_setCursor(3, 1);
	lcd_print(txtWiping);
	zeroCalibrations();
	delay(500);
}

//determine where the calibration value is stored in EEPROM depending on the sample value
int getCalibrationTableOffsetByValue(int sensor, int value) {
	return calibrationOffset + ((sensor - 1) * numberOfCalibrationValues) + (value >> 2);
}

//determine where in EEPROM the calibration value is stored depending on the position
int getCalibrationTableOffsetByPosition(int sensor, int pos) {
	return calibrationOffset + ((sensor - 1) * numberOfCalibrationValues) + pos;
}

//only write if the value needs writing (saves write cycles)
void eepromWriteIfChanged(int address, int data) {
	if (data != (int) EEPROM.read(address)) {
		EEPROM.write(address, data); 				//write the data to EEPROM
	}
}

//actually clears the flash
void zeroCalibrations() {
	for (uint8_t sensor = 1; sensor < (NUM_SENSORS ); sensor++) {
		for (int i = 0; i < numberOfCalibrationValues; i++) {
			eepromWriteIfChanged(getCalibrationTableOffsetByPosition(sensor, i), 0); //write the data directly to EEPROM
		}
	}
}

//read raw data from the sensor by an effective method
int readSensorRaw(int sensor) {
	if (!settings.delayTime) { //if delaytime is zero, do a pre-read to prime the ADC
		analogRead(inputPin[sensor]);
	} else {
		delayMicroseconds(settings.delayTime);
	}
	reading[sensor] = analogRead(inputPin[sensor]);
	return (reading[sensor]);
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
	const int shift = 8;
	const int factor = 3;
	int maxValue = 0;
	int minValue = 0;
	int lowestCalibratedValue = 1024;

	lcd_clear();

	createWaitKeyPressChar();
	lcd_setCursor(19,0);
	lcd_write(byte((byte) 0));

	lcd_setCursor(0, 0);
	lcd_print(txtCalibrationBusy);

	lcd_setCursor(0, 1);
	lcd_print(txtCalibrationBusy2);

	lcd_setCursor(0, 2);
	lcd_print(txtPressAnyKey);

	delay(500);//otherwise key still pressed, probably need a better solution

	//initialize temp values array, note full ints (16 bits) used
	int values[numberOfCalibrationValues];

	//read existing values from EEPROM and pre-shift them
	//shifting an int left by n bits simply gives us n bits of 'virtual' decimal places
	// this is needed for accuracy because EMA calculation works by adding or subtracting relatively small values
	// which would otherwise all be truncated to '0'
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		values[i] = ((int) EEPROM.read(getCalibrationTableOffsetByPosition(sensor, i))) << shift;
	}

	while (!buttonPressed()) {
		readSensorRaw(0);  //read master
		readSensorRaw(sensor); //read calibration sensor

		int calibrationValue = reading[0] - reading[sensor];

		//record some basic quality statistics
		if (calibrationValue > maxValue) maxValue = calibrationValue;
		if (calibrationValue < minValue) minValue = calibrationValue;
		if (reading[sensor] < lowestCalibratedValue) lowestCalibratedValue = reading[sensor];

		values[(reading[sensor] >> 2)] = intExponentialMovingAverage(shift, factor, values[(reading[sensor] >> 2)], calibrationValue);
	}

	//post_shift the values in preparation of writing back to EEPROM
	// we don't need to save the 'decimal places' because they are not needed anymore.
	// so we lose them by shifting them out of range to the right
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		values[i] = (values[i] >> shift);
	}

	//save calibrations
	for (int i = 0; i < numberOfCalibrationValues; i++) {
		eepromWriteIfChanged(getCalibrationTableOffsetByPosition(sensor, i) , values[i]);
	}

	lcd_clear();
	lcd_setCursor(19,0);
	lcd_write(byte((byte) 0));

	lcd_setCursor(0, 0);
	lcd_print(txtCalibrationDone);

	lcd_setCursor(0, 1);
	lcd_print(txtLowestPressure);
	printLcdInteger( lowestCalibratedValue, 14, 1, 5);
	lcd_setCursor(0, 2);
	lcd_print(txtMinAdjust);
	printLcdInteger( minValue, 14, 2, 5);
	lcd_setCursor(0, 3);
	lcd_print(txtMaxAdjust);
	printLcdInteger( maxValue, 14, 3, 5);

	lcd_setCursor(19,0);
	lcd_write(byte((byte) 0));

	waitForAnyKey();
	displayCalibratedValues(values);
	waitForAnyKey();
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
void displayCalibratedValues(int values[]){
	int valueOffset=0;
	int numberOfColumns = 20;
	int segmentsPerCharacter = 8;
	int numberOfCharacters = 4;
	int numberOfSegments = segmentsPerCharacter * (numberOfCharacters / 2);
	int valuePerSegment = settings.calibrationMax / numberOfSegments; //128 / 16 = 8
	int pressedButton = 0;
	bool dataChanged = true;
	bool topLeftArrowPositionAvailable, topRightArrowPositionAvailable = true;
	makeCalibrationChars();

	while(pressedButton != CANCEL){
		if(dataChanged){
			lcd_clear();
			for(int column = 0; column < numberOfColumns; column++){
				int valueInSegments = values[valueOffset + column] / valuePerSegment;

				if(valueInSegments <=  segmentsPerCharacter && valueInSegments > 0){
					lcd_setCursor(column,1);
					lcd_write(byte((byte) 8-valueInSegments));
				}else if(valueInSegments >  2 * segmentsPerCharacter){
					lcd_setCursor(column,0);
					lcd_printChar('|');
					lcd_setCursor(column,1);
					lcd_printChar('|');
				}else if(valueInSegments >  segmentsPerCharacter){
					lcd_setCursor(column,0);
					lcd_write(byte((byte) 8-(valueInSegments % segmentsPerCharacter)));
				}else if (valueInSegments < (2 * -segmentsPerCharacter)){
					lcd_setCursor(column,2);
					lcd_printChar('|');
					lcd_setCursor(column,3);
					lcd_printChar('|');
				}else if(valueInSegments < 0 && (valueInSegments >= -segmentsPerCharacter)){
					lcd_setCursor(column,2);
					lcd_write(byte((byte) (-valueInSegments)-1));
				}else if (valueInSegments < 0 && (valueInSegments < -segmentsPerCharacter)){
					lcd_setCursor(column,3);
					lcd_write(byte((byte) 8-((valueInSegments+1) % segmentsPerCharacter)));
				}

				if(column == 0) topLeftArrowPositionAvailable = (valueInSegments <= 0);
				if(column == 19) topRightArrowPositionAvailable = (valueInSegments <= 0);

			}

			//show arrows to indicate scrolling and our location in the calibration array
			displayNavArrowsAndOffsets(valueOffset,
					topLeftArrowPositionAvailable,
					topRightArrowPositionAvailable);
		}

		//allow the user to scroll through the values from left to right and vice versa
		pressedButton = buttonPressed();
		if((pressedButton == LEFT) && (valueOffset > 20)) {
			valueOffset -= 20;
			dataChanged = true;
		}else if((pressedButton == LEFT) && (valueOffset <= 20)){
			valueOffset = 0;
			dataChanged = true;
		}else if((pressedButton == RIGHT) && (valueOffset < numberOfCalibrationValues-20-20)) {
			valueOffset = (valueOffset + 20);
			dataChanged = true;
		}else if((pressedButton == RIGHT) && (valueOffset >= numberOfCalibrationValues-20-20)) {
			valueOffset = (numberOfCalibrationValues - 20);
			dataChanged = true;
		} else{
			dataChanged =false;
		}
	}
}



//create special characters in LCD memory these contain the horizontal lines
// we use to generate a graph of our calibration data
// using simple lines instead of full bars means we can use the same characters above and below zero
// because we only have 8 special chars, we would run out if we used bars!
void createSpecialCharacter(int number){
	byte specialCharacter[8];

	for(int i=0; i < 8; i++){
		specialCharacter[i] = 0b00000;
	}
	specialCharacter[number-1] = 0b11111;
	lcd_createChar(number-1, specialCharacter);
}


// we need 8 characters to use each line in a 5x8 pixel character cell
void makeCalibrationChars(){
	for(int i = 1; i<=8; i++){
		createSpecialCharacter(i);
	}
}


//dump the calibration array to the serial port for review
void doCalibrationDump() {
	lcd_clear();
	lcd_setCursor(0, 1);
	lcd_print(txtConnectSerial);
	lcd_setCursor(0, 1);
	Serial.begin(getBaud(settings.baudRate));
	if (Serial) {

		Serial.println(txtSerialHeader);

		for (int i = 0; i < numberOfCalibrationValues; i++) {
			Serial.print(i);
			Serial.print("  \t");
			for (uint8_t sensor = 1; sensor < (NUM_SENSORS); sensor++) {
				Serial.print((int) EEPROM.read(getCalibrationTableOffsetByPosition(sensor, i)));
				Serial.print("  \t");
			}
			Serial.print("\n");
		}
		Serial.print(txtSerialFooter);
	}
}

//dump calibrated sensor data directly to serial
void doDataDump() {
	int reading = 0;

	lcd_clear();
	lcd_setCursor(0, 1);
	lcd_print(txtConnectSerial);
	Serial.begin(getBaud(settings.baudRate));
	if (Serial) {
		lcd_setCursor(0, 1);
		lcd_print(txtDumpingSensorData);
		Serial.println(F("0\t0\t0\t0\t0"));
		unsigned long startTime = millis();

		while (! (buttonPressed() == CANCEL)) {
			Serial.print(millis() - startTime);
			Serial.print("\t");
			for (uint8_t sensor = 0; sensor < (NUM_SENSORS); sensor++) {

				if (sensor != 0) {  //only apply calibration for non reference sensors
					reading = readSensorRaw(sensor) + (int) EEPROM.read(getCalibrationTableOffsetByValue(sensor, reading));
				} else {
					reading = readSensorRaw(sensor);
				}

				Serial.print(reading);
				Serial.print("\t");
			}
			Serial.print("\n");
		}
		Serial.print(txtSerialFooter);
	}
}

void doRevs() {
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

	float rpmAverage;

	initRpmDisplay();

	while (! (buttonPressed() == CANCEL)) {

		measurement = readSensorRaw(0);

		if (measurement < (previous)) {
			descentCount++;
			ascentCount--;
		}
		if (measurement > (previous)) {
			ascentCount++;
			descentCount--;
		}
		if (descentCount > 2) {
			descending = true;
			ascentCount = 0;
			descentCount = 0;
		}

		if (ascentCount > 4) {
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
				rpm = exponentialMovingAverage(alphaRpm, &rpmAverage, 120000 / delta);

			}
			previousPeak = peak;
		}

		previousDescending = descending;
		previous = measurement;

		if (millis() - peak > 2000) rpm = 0;

		if (millis() - lastUpdateTime > 200L) {
			updateRpmDisplay(rpm);
			lastUpdateTime = millis();
		}
	}
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
	if (number > 10) number = 10;
	if (number < -10) number = -10;

	if (number < 0) {
		for (int8_t i = 0 ; i < 10 + number; i++) {
			bars[i] = ' ';
		}
		for (int8_t i = 10 + number ; i < 10; i++) {
			bars[i] = 0xff;
		}
		for (int8_t i = 10 ; i < 20; i++) {
			bars[i] = ' ';
		}
	}

	if (number == 0) {
		for (int8_t i = 0 ; i <= 20 ; i++) {
			bars[i] = ' ';
		}
	}

	if (number > 0) {
		for (int8_t i = 0 ; i <= 10 ; i++) {
			bars[i] = ' ';
		}
		for (int8_t i = 10 ; i <= 10 + number ; i++) {
			bars[i] = 0xff;
		}
		for (int8_t i = 10 + number ; i < 20; i++) {
			bars[i] = ' ';
		}
	}

	bars[DISPLAY_COLS] = 0x00;
}

//used to detect ambient pressure for readings that ascend with vacuum in stead of going down toward absolute 0 pressure
int detectAmbient() {
	unsigned long total = 0;
	uint8_t numberOfSamples = 200;                                //set the number of samples to average

	for (int i = 0; i < numberOfSamples; i++) {

		total += readSensorRaw(0);                          //add the reading we got
	}
	return total / numberOfSamples;                         //divide the result by the number of samples for the resulting average
}

void demo(){
	uint8_t colspeed[20];
	char matrix[4][20];

	while(!buttonPressed()){
		//set the column speeds
		for(int col=0;col<20;col++){
			colspeed[col] = (rand() % 7)+1;
		}
		for(int i =0; i<256;i++){
			delay(50);
			if(buttonPressed()) return;

			for(int col=0;col<20;col++){

				if(i % colspeed[col] == 0){

					lcd_setCursor(col,3);
					lcd_printChar(matrix[2][col]);
					matrix[3][col] = matrix[2][col];

					lcd_setCursor(col,2);
					lcd_printChar(matrix[1][col]);
					matrix[2][col] = matrix[1][col];

					lcd_setCursor(col,1);
					lcd_printChar(matrix[0][col]);
					matrix[1][col] = matrix[0][col];

					lcd_setCursor(col,0);
					matrix[0][col] = rand() % 256;
					lcd_printChar(matrix[0][col]);
				}
			}
		}
	}
}

int measureLCDSpeed(){
	unsigned long microseconds = micros();
	lcd_clear();
	return micros() - microseconds;
}

void demo2(){
	bool silent = settings.silent;
	settings.silent = true;
	int values[4];

	for(int i=0;i<=1024; i+=30){
		for(int j = 0; j < NUM_SENSORS; j++){
			values[j] = i;
		}
		lcdBarsSmooth(values);
	}
	for(unsigned int i=1024;i>=30;i-=30){
		for(int j = 0; j < NUM_SENSORS; j++){
			values[j] = i;
		}
		lcdBarsSmooth(values);
	}

	settings.silent = silent;
}
void demo3(){
	int values[4];
	bool silent = settings.silent;
	uint8_t master = settings.master;
	settings.silent = true;

	for(int i=100;i>=0;i-=4){
		values[0] = i;
		values[1] = i;
		values[2] = i;
		values[3] = 128-i;
		lcdBarsCenterSmooth(values);
	}
	settings.master=0;
	for(int i=0;i<=100;i+=4){
			values[3] = i;
			values[1] = i;
			values[2] = i;
			values[1] = 128-i;
			lcdBarsCenterSmooth(values);
		}
	settings.master = master;
	settings.silent = silent;
}

void doDeviceInfo(){
	unsigned long speed = measureLCDSpeed();
	lcd_setCursor(0,0);
	lcd_print(F("Settings Size B: "));
	lcd_printInt(sizeof(settings));
	lcd_setCursor(0,1);
	lcd_print(F("Free SRAM B: "));
	lcd_printInt(freeMemory());
	lcd_setCursor(0,2);
	lcd_print(F("LCD uS: "));
	lcd_printLong(speed);
	waitForAnyKey();
}
