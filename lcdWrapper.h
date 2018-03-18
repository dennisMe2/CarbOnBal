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


#ifndef LCDWRAPPER_H_INCLUDED
#define LCDWRAPPER_H_INCLUDED
#include <Arduino.h>

void lcd_begin(int, int);
void lcd_createChar(byte, byte[]);
void lcd_write(byte);

void lcd_setCursor(int a, int i);
void lcd_print(String str);
void lcd_print(const char* str);
void lcd_print(char* str);
void lcd_printInt(int integer);
void lcd_printLong(unsigned long integer);
void lcd_printFormatted(float value);
void lcd_printFloat(float value);

void lcd_printChar(char chr);
void lcd_clear();
void printLcdSpace( uint8_t column, uint8_t row, uint8_t length);
void printLcdInteger(int value, uint8_t column, uint8_t row, uint8_t length);
void printLcdFloat(float value, uint8_t column, uint8_t row, uint8_t length);
void drawSnowFlake(void);
#endif
