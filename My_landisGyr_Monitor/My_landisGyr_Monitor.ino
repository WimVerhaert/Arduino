/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a distance sensor using HC-SR04 
 * Use this sensor to measure KWH and Watt of your house meeter
 * You need to set the correct pulsefactor of your meeter (blinks per KWH).
 * The sensor starts by fetching current KWH value from gateway.
 * Reports both KWH and Watt back to gateway.
 *
 * Unfortunately millis() won't increment when the Arduino is in 
 * sleepmode. So we cannot make this sensor sleep if we also want 
 * to calculate/report watt-number.
 * http://www.mysensors.org/build/pulse_power
 */

#include <SPI.h>
#include <MySensor.h>  
#define MY_NODE_ID 2

#define DIGITAL_INPUT_SENSOR 3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define ANALOG_INPUT_SENSOR A3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define PULSE_FACTOR 1000       // Nummber of blinks per KWH of your meter
#define SLEEP_MODE false        // Watt-value can only be reported when sleep mode is false.
#define MAX_WATT 10000          // Max watt value to report. This filters outliers.
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define CHILD_ID 1            // Id of the sensor child
#define CHILD_ID_ANALOG 2            // Id of the sensor child
unsigned long SEND_FREQUENCY = 10000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
unsigned long BATTERY_SEND_FREQUENCY = 3600000;  // sleep time between reads (seconds * 1000 milliseconds)
//unsigned long BATTERY_SEND_FREQUENCY = 5000;  // sleep time between reads (seconds * 1000 milliseconds)
MySensor gw;
double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
boolean pcReceived = false;
//boolean pcReceived = true;
volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile unsigned long lastBatteryRead = 0;
volatile unsigned long watt = 0;
unsigned long oldPulseCount = 0;   
unsigned long oldWatt = 0;
double oldKwh;
unsigned long lastSend = 0;
MyMessage wattMsg(CHILD_ID,V_WATT);
MyMessage kwhMsg(CHILD_ID,V_KWH);
MyMessage pcMsg(CHILD_ID,V_VAR1);
bool sendBattery = false;
int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

// Instantiate a Bounce object



void setup()  
{  
 
 // use the 1.1 V internal reference
  analogReference(INTERNAL);
 
  gw.begin(incomingMessage,MY_NODE_ID,false);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Landis&Gyr Energy Meter", "1.1");

  // Register this device as power sensor
  gw.present(CHILD_ID, S_POWER);

  // Fetch last known pulse count value from gw
  gw.request(CHILD_ID, V_VAR1);
  
  attachInterrupt(INTERRUPT, onPulse, RISING);
  lastSend=millis();
  if (sendBattery) {
    gw.sendBatteryLevel(100);
  }
}


void loop()     
{ 
  gw.process();
  unsigned long now = millis();
  // Only send values at a maximum frequency or woken up from sleep
  bool sendTime = now - lastSend > SEND_FREQUENCY;

  if (pcReceived && (SLEEP_MODE || sendTime)) {
    
    // New watt value has been calculated  
    if (!SLEEP_MODE && watt != oldWatt) {
      // Check that we dont get unresonable large watt value. 
      // could hapen when long wraps or false interrupt triggered
      if (watt<((unsigned long)MAX_WATT)) {
        gw.send(wattMsg.set(watt));  // Send watt value to gw 
      }  
      Serial.print("Watt:");
      Serial.println(watt);
      oldWatt = watt;
    }
  
    // Pulse count has changed
    if (pulseCount != oldPulseCount) {
      gw.send(pcMsg.set(pulseCount));  // Send pulse count value to gw 
      double kwh = ((double)pulseCount/((double)PULSE_FACTOR));     
      oldPulseCount = pulseCount;
      if (kwh != oldKwh) {
        gw.send(kwhMsg.set(kwh, 4));  // Send kwh value to gw 
        oldKwh = kwh;
      }
    }    
    lastSend = now;
  } else if (sendTime && !pcReceived) {
    // No count received. Try requesting it again
    gw.request(CHILD_ID, V_VAR1);
    lastSend=now;
  }
  
  if (SLEEP_MODE) {
    gw.sleep(SEND_FREQUENCY);
  }
  
  now = millis();  
  bool intervalBattery = now - lastBatteryRead > BATTERY_SEND_FREQUENCY;
  
  if (intervalBattery && sendBattery) {
  // get the battery Voltage
     int sensorValue = analogRead(BATTERY_SENSE_PIN);
     Serial.println(sensorValue);
     
     // 1M, 470K divider across battery and using internal ADC ref of 1.1V
     // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
     // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
     // 3.44/1023 = Volts per bit = 0.003363075
     float batteryV  = sensorValue * 0.003363075;
     int batteryPcnt = sensorValue / 10;
  
     Serial.print("Battery Voltage: ");
     Serial.print(batteryV);
     Serial.println(" V");
  
     Serial.print("Battery percent: ");
     Serial.print(batteryPcnt);
     Serial.println(" %");
  
     if (oldBatteryPcnt != batteryPcnt) {
       // Power up radio after sleep
       gw.sendBatteryLevel(batteryPcnt);
       oldBatteryPcnt = batteryPcnt;
     }
     lastBatteryRead = now;
  }
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR1) {  
    pulseCount = oldPulseCount = message.getLong();
    Serial.print("Received last pulse count from gw:");
    Serial.println(pulseCount);
    pcReceived = true;
  }
}

void onPulse()     
{ 
  if (!SLEEP_MODE) {
    Serial.print("Pulse received ");
    Serial.println(pulseCount);
    unsigned long newBlink = micros();  
    unsigned long interval = newBlink-lastBlink;
    if (interval<10000L) { // Sometimes we get interrupt on RISING
      Serial.println("not counting");
      return;
    }
    watt = (3600000000.0 /interval) / ppwh;
    lastBlink = newBlink;
  } 
  pulseCount++;
}

