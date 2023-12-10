// Copyright (c) PA3ANG. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Send CI-V Command by Bluetooth
void sendCommand(char *request, size_t n, char *buffer, uint8_t limit, boolean sendOnly = false)
{
  uint8_t byte1, byte2, byte3;
  uint8_t counter = 0;

  while (counter != limit)
  {
    for (uint8_t i = 0; i < n; i++)  CAT.write(request[i]);
    vTaskDelay(200);
    if(sendOnly == true) return;

    while (CAT.available())
    {
      byte1 = CAT.read();
      byte2 = CAT.read();

      if (byte1 == 0xFE && byte2 == 0xFE)
      {
        counter = 0;
        byte3 = CAT.read();
        while (byte3 != 0xFD)
        {
          buffer[counter] = byte3;
          byte3 = CAT.read();
          counter++;
        }
      }
    }
  }
}

// Get Frequency
void getFrequency()
{
  String frequency, frequencyNew;

  static char buffer[8];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x03, 0xFD};
  double freq; // Current frequency in Hz
  const uint32_t decMulti[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};

  uint8_t lenght = 0;
  size_t n = sizeof(request) / sizeof(request[0]);
  sendCommand(request, n, buffer, 8);

  freq = 0;
  for (uint8_t i = 2; i < 7; i++)
  {
    freq += (buffer[9 - i] >> 4) * decMulti[(i - 2) * 2];
    freq += (buffer[9 - i] & 0x0F) * decMulti[(i - 2) * 2 + 1];
  }

  freq += double(LNB_OFFSET - LNB_CALIBRATE);
  frequencyCurrent = freq;
  frequency = String(freq);
  lenght = frequency.length();

  if(frequency != "0")
  {
    int8_t i;
    for(i = lenght - 6; i >= 0; i -= 3)  frequencyNew = "." + frequency.substring(i, i + 3) + frequencyNew;
    if(i == -3) 
      frequencyNew = frequencyNew.substring(1, frequencyNew.length());
    else 
      frequencyNew = frequency.substring(0, i + 3) + frequencyNew;
  }
  else 
    frequencyNew = "-";

  frequency = frequencyNew;

  // do not write every time to the display only when the frequency changes
  if (frequency != frequencyOld)
  {
    frequencyOld = frequency;
    M5.Lcd.setFont(&UniversCondensed20pt7b);
    M5.Lcd.setTextPadding(200);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    if (frequency <     "10.489.500.000")
      M5.Lcd.drawString("  OUT OF BAND  ", 162, 90);
    else
      M5.Lcd.drawString(frequency.substring(0, 13), 162, 90);
  }
}

// Set Operating Frequency
void setFrequency(int frequency)
{
  // calculate RX frequency based on QO_frequency  LNB_OFFSET - LNB_CALIBRATE
  int RX_frequency = (frequency - LNB_OFFSET + LNB_CALIBRATE);
  
  //const uint32_t decMulti[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
  int a =  RX_frequency/1000000000;
  int b = (RX_frequency%1000000000) / 100000000;
  int c = (RX_frequency%100000000) / 10000000;
  int d = (RX_frequency%10000000) / 1000000;
  int e = (RX_frequency%1000000) / 100000;
  int f = (RX_frequency%100000) / 10000;
  int g = (RX_frequency%10000) / 1000;
  int h = (RX_frequency%1000) / 100;
  int i = (RX_frequency%100) / 10;
  int j = (RX_frequency%10) / 1;

  // now b = 5, c = 3, d = 4, e = 5
  int k = 0x10*a + b;
  int l = 0x10*c + d;
  int m = 0x10*e + f;
  int n = 0x10*g + h;
  int o = 0x10*i + j;


  // write RX frequency in selected vfo command 05
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x05, (char)o, (char)n, (char)m, (char)l, (char)k, 0xFD};
  size_t q = sizeof(request) / sizeof(request[0]);
  sendCommand(request, q, buffer, 6, true);
}

// Set Duplex Offset
void setDupOffset()
{
  if(dupOffset == dupOffsetOld)
    return;

  //set Dup On
  static char buffer[6];
  char request1[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x0F, 0x11, 0xFD};
  size_t n = sizeof(request1) / sizeof(request1[0]);
  sendCommand(request1, n, buffer, 6, true);

  // enter the Dup Offset. This is 100 Hz resolution. No need to make it on 10 Hz resolution demonstrated experiments
  dupOffsetOld = dupOffset;
  int Dup_Offset = (QO_SHIFT + dupOffset);
  int b =  Dup_Offset/100000;
  int c = (Dup_Offset%100000) / 10000;
  int d = (Dup_Offset%10000) / 1000;
  int e = (Dup_Offset%1000) / 100;
    
  char request2[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x0D, (char)(0x10*d + e), (char)(0x10*b + c), 0x00, 0xFD};      
  // write DUP_offset max 999.9 kHz
  size_t m = sizeof(request2) / sizeof(request2[0]);
  sendCommand(request2, m, buffer, 6, true);

  char CAL[5];
  sprintf(CAL,"%lld",dupOffset/10);
  M5.Lcd.setFont(&tahoma8pt7b);
  M5.Lcd.setTextPadding(24);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE);
  
  M5.Lcd.fillRoundRect(204, 150, 94, 18, 2, TFT_BLACK);
  M5.Lcd.drawString("CAL:", 231, 160);
  M5.Lcd.drawString(CAL, 271, 160);

  M5.Speaker.tone(3000, 50);
}

// Get TX
uint8_t getTX()
{
  uint8_t value;
  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x1C, 0x00, 0xFD};
  size_t n = sizeof(request) / sizeof(request[0]);
  sendCommand(request, n, buffer, 5);

  if (buffer[4] <= 1) 
    value = buffer[4];
  else 
    value = 0;

  // do not write every time the status on the display only when it changes
  if (value != TXOld)
  {
    TXOld = value;
    M5.Lcd.setFont(&UniversCondensed20pt7b);
    M5.Lcd.setTextPadding(24);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_RED);
    if (value == 1) 
      M5.Lcd.drawString("TX", 102, 162);
    else 
      M5.Lcd.fillRoundRect(84, 140, 50, 40, 2, TFT_BLACK);
  }
  return value;
}

// Set RF Power
void setPower(int Mode)
{
  static char buffer[8];
  if (Mode == 1) 
  {
    // 40% by USB
    char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x0A, 0x01, 0x03, 0xFD}; 
    size_t o = sizeof(request) / sizeof(request[0]);
    sendCommand(request, o, buffer, 6, true);
  }
  else 
  {
    // 5% by CW 
    char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x0A, 0x00, 0x13, 0xFD};
    size_t o = sizeof(request) / sizeof(request[0]);
    sendCommand(request, o, buffer, 6, true); 
  }
}

// Set Squelch
void setSquelch(boolean Sq)
{
  static char buffer[8];
  if (Sq) 
  {
    char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x03, 0x01, 0x66, 0xFD}; 
    size_t o = sizeof(request) / sizeof(request[0]);
    sendCommand(request, o, buffer, 6, true);
    squelchOn = true;
  }
  else 
  {
    char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x14, 0x03, 0x00, 0x00, 0xFD}; 
    size_t o = sizeof(request) / sizeof(request[0]);
    sendCommand(request, o, buffer, 6, true);
    squelchOn = false;
  }
}

// Set Mode
void setMode(int Mode)
{
  static char buffer[6];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x06, (char)Mode, 0xFD};      
  size_t o = sizeof(request) / sizeof(request[0]);
  sendCommand(request, o, buffer, 6, true);
}

// Get Mode
uint8_t getMode()
{
  String value;
  static char buffer[5];
  char request[] = {0xFE, 0xFE, CI_V_ADDRESS, 0xE0, 0x04, 0xFD};
  size_t n = sizeof(request) / sizeof(request[0]);
  sendCommand(request, n, buffer, 5);

  if(uint8_t(buffer[4]) >= 0 && uint8_t(buffer[4]) <= 23)
  {
  modeCurrent = (uint8_t)buffer[3];
  }
  return uint8_t(buffer[3]);
}