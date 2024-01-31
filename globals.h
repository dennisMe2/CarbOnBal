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


#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED
#include <Arduino.h>

#define SOFTWARE_VERSION "2.0.b2"

#define SELECT 2
#define LEFT 3
#define RIGHT 4
#define CANCEL 5

#define P0VSENSOR 150                                             //minimum pressure reported by MAP sensor at 0V in millibar/hPa (=10x KPa value)
#define P5VSENSOR 1020												//maximum pressure reported by sensor at 5V in millibar
#define NUM_BUTTONS 4
#define NUM_SENSORS 4
#define DISPLAY_COLS 20
#define DISPLAY_ROWS 4

enum class Command : byte {
    NO_COMMAND = 0x0,
    START_PACKET = 0xfe,
    REQUEST_PACKET = 0xfd,
    CARB_VACUUM = 0xe0,
    CALIBRATION = 0xe1,
    SETTINGS = 0xe2,
    DIAGNOSTICS = 0xe3,
    END_DATA = 0xe4
};

#define BAUD_RATE 230400

#define RELEASED 0x1
#define PRESSED  0x0

#define LANGUAGE "lang_en_gb.h"			//select a different language file to change translations
//#define LANGUAGE "lang_nl_nl.h"
static const uint16_t analogReadRange=1024;// The Nano has a 10-bit ADC
static const uint8_t numReadings=20;
static const uint8_t brightnessPin=6;
static const uint8_t contrastPin=11;
static const uint16_t numberOfCalibrationValues=256;
static const uint16_t calibrationOffset=256; //eeprom base address for calibration data

static const uint8_t versionUID = 31; //update when settings_t changes!

enum class PressureUnit : uint8_t {
    RAW, RAW_DESCENDING,
    MILLIBAR_HPA, MILLIBAR_HPA_DESCENDING,
    CM_MERCURY, CM_MERCURY_DESCENDING,
    INCH_MERCURY, INCH_MERCURY_DESCENDING,
};

enum class GraphType : uint8_t {
    BARS, BARS_SMOOTH, DIAGNOSTIC,
};

/* button1 has three user-settable modes */
enum class Button1Mode : uint8_t {
    CONTRAST,
    DAMPING,
    RESET_AVERAGING,
};

/* button2 has three user-settable modes */
enum class Button2Mode : uint8_t {
    BRIGHTNESS,
    RPM_DISPLAY,
    RPM_DAMPING,
};

/* button3 has three user-settable modes */
enum class Button3Mode : uint8_t {
    FREEZE_DISPLAY,
    RESET_MEASUREMENTS,
    RPM_DISPLAY /* NOTE:overlap with button2 */
};

//this struct is used to store settings in NVRAM
//does not use bit fields because these cause more writes to the same NVRAM locations
//if a setting is not writable then the settings are moved to the next available location in NVRAM
//this allows more than enough writes in total for endless futzing around
struct settings_t {
    bool silent;
    bool advanced;
    bool splashScreen;//bool is 8 bits on Arduino
    uint8_t cylinders;
    uint8_t master;
    enum Button1Mode button1;
    enum Button2Mode button2;
    enum Button3Mode button3;
    uint8_t contrast;
    uint8_t brightness;
    enum GraphType graphType;
    uint8_t rpmDamping;
    enum PressureUnit units;
    uint8_t zoom;
    uint8_t calibrationMax;
    uint8_t damping;
};

union longAverages {
    int32_t longVal;
    int16_t intVal[2];
};
union intByteUnion {
    uint16_t intVal;
    uint8_t byteVal[2];
};

extern int readingCount[NUM_SENSORS];
extern unsigned int average[NUM_SENSORS];
extern int total[NUM_SENSORS];
extern int reading[NUM_SENSORS];
extern unsigned int rpm;
extern settings_t settings;
extern int ambientPressure;

#endif
