/*
 *
 * Copyright (c) [2026] by InvenSense, Inc.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
 
#include "ICM45687.h"

// Instantiate an ICM456XX with LSB address set to 0
ICM456xx IMU(Wire,0);

volatile bool wake_up = false;
bool vvd_update = false;
bool vvd_status = false;
int sample_cnt = 0;
uint64_t last_vvd_time_us = 0;

void irq_handler(void) {
  wake_up = true;
}

void setup() {
  int ret;
  Serial.begin(115200);
  while(!Serial) {}

  // Initializing the ICM456XX
  ret = IMU.begin();
  if (ret != 0) {
    Serial.print("ICM456xx initialization failed: ");
    Serial.println(ret);
    while(1);
  }

  IMU.SetVocalVibDet_thresh(1000); // default 1920
  IMU.SetVocalVibDet_nb_samples(62); //default 31
  vvd_update = true;
  // APEX VVD enabled, irq on pin 2
  IMU.startVocalVibDet(2,irq_handler);
  sample_cnt = IMU.GetVocalVibDet_nb_samples();
}

void loop() {
  if(wake_up)
  {
    if(!vvd_status)
    {
      Serial.println("Detect the Voice! (start)");
      vvd_status = true;
    } else {
      Serial.println("Detect the Voice! (continue)");
    }
    last_vvd_time_us = micros();
    wake_up = false;
  }

  if(vvd_update)
  {
    // called when changing the VVD thresh(SetVocalVibDet_Dynthresh) and init
    int ret = IMU.CheckVocalVibDet_thresh();
    if(ret)
    {
      Serial.println("Updated the VVD thresh");
      vvd_update = false;
    }
  }

  if(vvd_status)
  {
    unsigned long currentTime = micros();
    if(currentTime - last_vvd_time_us >
      ((uint64_t)sample_cnt * 1375 /* 800 Hz + 10% */))
      {
        Serial.println("End the Voice! (end)");
        vvd_status = false;
      }
  }
}
