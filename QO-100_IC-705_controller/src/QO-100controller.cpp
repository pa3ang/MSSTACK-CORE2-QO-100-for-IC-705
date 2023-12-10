// Copyright (c) PA3ANG. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <M5Unified.h>
#include "QO-100controller.h"
#include "fonts.h"
#include "IC705commands.h"
#include "M5StackCORE2functions.h"


// Setup
void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(TFT_BLACK);

  CAT.register_callback(callbackBT);
  CAT.begin(NAME);

  viewGUI();
  
  // Multitasking task for retreive button
  xTaskCreatePinnedToCore(
      button,   // Function to implement the task
      "button", // Name of the task
      8192,     // Stack size in words
      NULL,     // Task input parameter
      4,        // Priority of the task
      NULL,     // Task handle
      1);       // Core where the task should run
}

// Main loop
void loop()
{
  if(checkConnection() and !getTX()) 
  {
    getFrequency();
    getMode();
  }
}