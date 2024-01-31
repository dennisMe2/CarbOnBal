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


#ifndef MENUACTIONS_H_INCLUDED
#define MENUACTIONS_H_INCLUDED

#include <Arduino.h>
#include "globals.h"

void doConfirmation();
void doBaudRate();
void actionBrightness() ;
void actionContrast();
void actionContrastButton();
void actionSilent();
void doAdvanced();
void doSplashScreen();
void actionAveragingMethod();
void actionGraphing();
void doArduinoMode() ;
void actionReset();
void actionEmaShift();
void actionEmaDamping();
void doEmaFactor(int value);
void actionEmaCorrection();
void actionEmaRpmSensitivity();
void doDelayTime(int value);
void actionResponsiveness();
void actionDamping();
void actionRpmEmaDamping();
void doDamping(int value);
void actionCylinders();
void doCylinders(int value) ;
void actionMaster() ;
void doMaster(int value);
void fixMaster();
void actionBrightnessButton();
void actionCancelButton();
void actionCalibrationMax();
void doUnits();
void doMaxZoom();
#endif
