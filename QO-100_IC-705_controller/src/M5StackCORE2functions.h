// Copyright (c) PA3ANG. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Bluetooth callback
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    btConnected = true;
  }
  if (event == ESP_SPP_CLOSE_EVT)
  {
    btConnected = false;
  }
}

// Get button
void getButton()
{
  M5.update();
  btnA = M5.BtnA.wasClicked();
  btnB = M5.BtnB.wasClicked();
  btnC = M5.BtnC.wasClicked();
}

// View GUI
void viewGUI()
{
  // Clear
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_BLACK);

  M5.Lcd.setFont(&tahoma8pt7b);
  M5.Lcd.setTextPadding(24);
  M5.Lcd.setTextDatum(CC_DATUM);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawString("QO-100 Controller IC-705 (BT)", 163, 20);
  
  // view menu buttons
  M5.Lcd.setTextColor(TFT_RED);
 
  M5.Lcd.fillRoundRect(38, 208, 34, 22, 2, TFT_BLACK);
  M5.Lcd.drawRoundRect(38, 208, 34, 22, 2, TFT_RED);
  M5.Lcd.drawString("CW", 55, 220);

  M5.Lcd.fillRoundRect(142, 208, 38, 22, 2, TFT_BLACK);
  M5.Lcd.drawRoundRect(142, 208, 38, 22, 2, TFT_RED);
  M5.Lcd.drawString("CAL", 162, 220);

  M5.Lcd.fillRoundRect(230, 208, 68, 22, 2, TFT_BLACK);
  M5.Lcd.drawRoundRect(230, 208, 68, 22, 2, TFT_RED);
  M5.Lcd.drawString("Squelch", 265, 220);
}

// Check connection
boolean checkConnection()
{
  if (btConnected == false)
  {
    // no BT connection show NEED PAIRING message
    M5.Lcd.setFont(&UniversCondensed20pt7b);
    M5.Lcd.setTextPadding(200);
    M5.Lcd.setTextDatum(CC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.drawString(" NEED PAIRING ", 162, 90);
    vTaskDelay(750);
    M5.Lcd.drawString("", 162, 90);
    vTaskDelay(250);
    return false;
  }
  return true;
}

// Get Button
void button(void *pvParameters)
{
  for (;;)
  {
    // Get button
    getButton();
    // first beep to aknowledge  
    if(btnA || btnB || btnC ) 
    {
      if(btnA || btnC )
      {
        // play beep sound 1000Hz 100msec (background task)
        M5.Speaker.tone(1000, 50);
      }
      else if(btnB)
      {
        // play beep sound 2000Hz 100msec (background task)
        M5.Speaker.tone(2000, 50);
      }
      // wait done
      while (M5.Speaker.isPlaying()) { vTaskDelay(1); }
    }
    // second run the Function under the button
    if(calibrationMode == false)
    {
      // Menu buttons
      if(btnA) {
        String buttonText;
        if (modeCurrent == 1)
        {
          setFrequency(CW_frequency);
          setMode(3);
          setPower(3);
          buttonText = "PH";
        }
        else
        {
          setFrequency(PH_frequency);
          setMode(1);
          setPower(1);
          buttonText = "CW";
        }
        M5.Lcd.setFont(&tahoma8pt7b);
        M5.Lcd.setTextPadding(24);
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.fillRoundRect(38, 208, 34, 22, 2, TFT_BLACK);
        M5.Lcd.drawRoundRect(38, 208, 34, 22, 2, TFT_RED);
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.drawString(buttonText, 55, 220);
      }
      else if(btnB) { 
        calibrationMode = true;

        // store existing mode and frequency
        frequencyFrom = frequencyCurrent;
        modeFrom = modeCurrent;

        // set beacon frequency
        setFrequency(Beacon_frequency);
        setMode(3);

        // highlight menu button CAL
        M5.Lcd.setFont(&tahoma8pt7b);
        M5.Lcd.setTextPadding(24);
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.fillRoundRect(142, 208, 38, 22, 2, TFT_RED);
        M5.Lcd.drawRoundRect(142, 208, 38, 22, 2, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString("CAL", 162, 220);
        vTaskDelay(pdMS_TO_TICKS(150));
      }
      else if(btnC) {
        // set SQ on or off level about S5
        if (squelchOn) {
          setSquelch(false);
          M5.Lcd.setFont(&tahoma8pt7b);
          M5.Lcd.setTextPadding(24);
          M5.Lcd.setTextDatum(CC_DATUM);
          M5.Lcd.fillRoundRect(230, 208, 70, 22, 2, TFT_BLACK);
          M5.Lcd.drawRoundRect(230, 208, 70, 22, 2, TFT_RED);
          M5.Lcd.setTextColor(TFT_RED);
          M5.Lcd.drawString("Squelch", 265, 220);
        }
        else {
          setSquelch(true);
          M5.Lcd.setFont(&tahoma8pt7b);
          M5.Lcd.setTextPadding(24);
          M5.Lcd.setTextDatum(CC_DATUM);
          M5.Lcd.fillRoundRect(230, 208, 70, 22, 2, TFT_RED);
          M5.Lcd.drawRoundRect(230, 208, 70, 22, 2, TFT_WHITE);
          M5.Lcd.setTextColor(TFT_WHITE);
          M5.Lcd.drawString("Squelch", 265, 220);
        }
      }
    }
    else if(calibrationMode == true)
    {
      if(btnB) {
        calibrationMode = false;
        // take existing frequency and calculate new lnb offset 
        long long lnb = frequencyCurrent - Beacon_frequency;
        LNB_CALIBRATE = (LNB_CALIBRATE + lnb);
        dupOffset = LNB_CALIBRATE;
        setDupOffset();

        // restore mode and frequency
        setFrequency(frequencyFrom-lnb);
        setMode(modeFrom);
        
        // de-highlight menu button CAL
        M5.Lcd.setFont(&tahoma8pt7b);
        M5.Lcd.setTextPadding(24);
        M5.Lcd.setTextDatum(CC_DATUM);
        M5.Lcd.fillRoundRect(142, 208, 38, 22, 2, TFT_BLACK);
        M5.Lcd.drawRoundRect(142, 208, 38, 22, 2, TFT_RED);
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.drawString("CAL", 162, 220);
        vTaskDelay(pdMS_TO_TICKS(150));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));  
  }    
}