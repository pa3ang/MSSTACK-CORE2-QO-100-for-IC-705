// Copyright (c) PA3ANG. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define VERSION "2.0"
#define AUTHOR "PA3ANG"
#define NAME "Core2-QO-100"

#include <M5Unified.h>
#include <BluetoothSerial.h>
#include <M5StackUpdater.h>

#define BT 1
#define IC_MODEL 705                            
#define CI_V_ADDRESS 0xA4                      

// Bluetooth connector
BluetoothSerial CAT;

// Global Variables
uint8_t TXOld;
long long frequencyFrom, frequencyCurrent, dupOffsetOld, dupOffset;
int modeFrom, modeCurrent;
int btnA, btnB, btnC;

long long QO_SHIFT          = 500000-700;   // 500kHz shift on transponder and 700 Hz TCXO (aging) deviation on SG-Lab transvertor
long long LNB_CALIBRATE     = 40000;        // expected LNB TXCO deviation 
long long LNB_OFFSET        = 10057000000;  // specified LNB offset
long long Beacon_frequency  = 10489500400;  // 700Hz CW pitch and assume beacon on 500.4 gives best result 
long long CW_frequency      = 10489525000;  // mid CW band
long long PH_frequency      = 10489680000;  // dutch channel

boolean calibrationMode     = false;
boolean squelchOn           = false;
boolean btConnected         = false;

String frequencyOld, frequencyNew;