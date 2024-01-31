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

#include <LiquidCrystal.h>

#include "lcdWrapper.h"


#include "utils.h"
#include "globals.h"

//  Order of parameters: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(13,12,10,9,8,7);

void lcd_setCursor(int a, int i) {
    lcd.setCursor(a, i);
}

void lcd_print(String str) {
    lcd.print(str);
}

//print strings stored in PROGMEM to LCD without using a buffer
void lcd_print(const char* str) {
    for (int i = 0; (unsigned) i < strlen_P(str); i++) {
        lcd.print((char) pgm_read_byte_near(str + i));
    }
}

void lcd_print(char* str) {
    lcd.print(str);
}

void lcd_printInt(int integer) {
    lcd.print(integer);
}

void lcd_printLong(unsigned long integer) {
    lcd.print(String(integer));
}

void lcd_printFloat(float value) {
    lcd.print(String(value));
}

void lcd_printFormatted(float value) {
    float fractionalPart = abs(value) - (long)abs(value);

    //if(value < 0.0) lcd.print('-');
    switch(settings.units) {
    case PressureUnit::RAW:
    case PressureUnit::RAW_DESCENDING:
        lcd.print(String((int)value));
        break;
    case PressureUnit::MILLIBAR_HPA:
    case PressureUnit::MILLIBAR_HPA_DESCENDING:
    case PressureUnit::CM_MERCURY:
    case PressureUnit::CM_MERCURY_DESCENDING:
        lcd.print(String((int)value));
        lcd.print('.');
        lcd.print((int)(fractionalPart * 10.0));
        break;
    default:
        lcd.print(String((int)value));
    }
}

void lcd_printChar(char chr) {
    lcd.print(chr);
}

void lcd_begin(int cols, int rows) {
    lcd.begin(cols, rows);
}

void lcd_createChar(byte byt, byte array[]) {
    lcd.createChar(byt, array);
}

void lcd_write(byte byt) {
    lcd.write(byt);
}

void lcd_clear() {
    lcd.clear();
}

void printLcdSpace( uint8_t column, uint8_t row, uint8_t length) {
    char space[length+1];
    for (uint8_t i=0 ; i<length; i++) {
        space[i] = ' ';
    }
    space[length] = 0x00;

    lcd_setCursor(column, row);
    lcd_print(space);
    lcd_setCursor(column, row);
}

void printLcdInteger(int value, uint8_t column, uint8_t row, uint8_t length) {
    printLcdSpace(column,row,length);
    lcd_print(String(value));
}
void printLcdFloat(float value, uint8_t column, uint8_t row, uint8_t length) {
    printLcdSpace(column,row,length);
    lcd_print(String(value));
}

void drawSnowFlake() {
    //make a little snow flake to indicate the frozen state of the display
    byte frozen[8] = {
        0b00000,
        0b00000,
        0b01010,
        0b10101,
        0b01110,
        0b10101,
        0b01010,
        0b00000
    };
    lcd_createChar(7, frozen); //tells LCD to store our snow flake in slot 7
    lcd_setCursor(19, 0); // place the cursor on the LCD
    lcd_write(byte(0x07)); //display the stored snow flake
}

