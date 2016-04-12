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
 * Version 1.0 - Wim Verhaert
 * 
 * DESCRIPTION
 * Use this sensor to measure KWH and Watt of your house meter
 * The sensor starts by fetching current KWH value from gateway.
 * Reports both KWH and Watt back to gateway.
 *
 * Use this sensor to measure flow and volume  of your water meter
 * The sensor starts by fetching current volume value from gateway.
 * Reports both flow and volume back to gateway.
 * 
 * http://www.mysensors.org/build/pulse_power
 */

#include <SPI.h>
#include <PinChangeInt.h>

#define MY_NODE_ID 2
#define CHILD_ID_E 1            // Id of the sensor child L&G meter
#define CHILD_ID_H 2            // Id of the sensor child Watermeter

#define DIGITAL_INPUT_SENSOR 3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define WATERMETER_INPUT_SENSOR 6  // the pin we are interested in

#define SLEEP_MODE false        // Watt-value can only be reported when sleep mode is false.
unsigned long SEND_FREQUENCY = 10000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.

#define PULSE_FACTOR_E 1000       // Nummber of blinks per KWH of your meter
#define PULSE_FACTOR_H 5       // Nummber of blinks per liter of your meter
#define MAX_WATT 40000          // Max watt value to report. This filters outliers.
#define MAX_FLOW 100          // Max watt value to report. This filters outliers.

//Emeter pulses per kWh
double ppwh = ((double)PULSE_FACTOR_E)/1000; // Pulses per watt hour
//Watermeter pulses
double ppLiter = ((double)PULSE_FACTOR_H); //  1 pulse/ 5 liter

//variables for Watermeter
volatile unsigned long pulseCount_H = 0;   
volatile unsigned long lastBlink_H = 0;
volatile unsigned long flow = 0;   //liter per minute
unsigned long oldPulseCount_H = 0;   
unsigned long oldflow = 0;
double volume;
double oldvolume = 0;
unsigned long lastSend_H = 0;
unsigned long lastSendFlow = 0;

//variables for Emeter
volatile unsigned long pulseCount_E = 0;   
volatile unsigned long lastBlink_E = 0;
volatile unsigned long lastBatteryRead = 0;
volatile unsigned long watt = 0;
unsigned long oldPulseCount_E = 0;   
unsigned long oldWatt = 0;
double oldKwh;
unsigned long lastSend_E = 0;

boolean pcReceived_H = false;
boolean pcReceived_E = false;



void setup()  
{  
 
  
  attachInterrupt(INTERRUPT, onPulse_E, RISING); //attach the interrupt for the Emeter
  
  pinMode(WATERMETER_INPUT_SENSOR, INPUT);     //set the pin to input
  digitalWrite(WATERMETER_INPUT_SENSOR, HIGH); //use the internal pullup resistor
  PCintPort::attachInterrupt(WATERMETER_INPUT_SENSOR, onPulse_H,RISING); // attach a PinChange Interrupt to our pin on the rising edge: for the Watermeter
  
  lastSend_E = millis();
  lastSend_H = millis();
}

void loop()     
{ 
  unsigned long now = millis();
  // Only send values at a maximum frequency or woken up from sleep
  bool sendTime_E = now - lastSend_E > SEND_FREQUENCY;
  bool sendTime_H = now - lastSend_H > (SEND_FREQUENCY);
  bool sendTimeFlow = now - lastSendFlow > (SEND_FREQUENCY*30);
  
  if (pcReceived_E &&  sendTime_E) {
    // New watt value has been calculated  
    if (watt != oldWatt) {
      // Check that we dont get unresonable large watt value. 
      // could hapen when long wraps or false interrupt triggered
      if (watt<((unsigned long)MAX_WATT)) {
        Serial.print("watt:");  // Send watt value to gw 
        Serial.println(watt);  // Send watt value to gw 
      }  
      oldWatt = watt;
      watt = 0;
    }
    // Pulse count has changed
    if (pulseCount_E != oldPulseCount_E) {
        Serial.print("pulseCount_E:");  // Send watt value to gw 
        Serial.println(pulseCount_E);  // Send watt value to gw 
        
      double kwh = ((double)pulseCount_E/((double)PULSE_FACTOR_E));     
      oldPulseCount_E = pulseCount_E;
      if (kwh != oldKwh) {
         Serial.print("kwh:");  // Send watt value to gw 
        Serial.println(kwh);  // Send watt value to gw 
        
        oldKwh = kwh;
      }
    }    
    lastSend_E = now;
  } else if (sendTime_E && !pcReceived_E) {
    // No count received. Try requesting it again
   
    lastSend_E=now;
  }
  
  
  if (pcReceived_H && sendTime_H) {
    
    // New flow value has been calculated  
    if (flow != oldflow) {
      // Check that we dont get unresonable large flow value. 
      // could hapen when long wraps or false interrupt triggered
      if (flow<((unsigned long)MAX_FLOW)) {
         Serial.print("flow:");  // Send watt value to gw 
        Serial.println(flow);  // Send watt value to gw 
      }  

      oldflow = flow;
      flow=0;
    }
    
  if (sendTimeFlow && (pulseCount_H == oldPulseCount_H)) { //nothing was flowing, we send this as 0 l/min
     Serial.print("flow:");  // Send watt value to gw 
        Serial.println(0);  // Send watt value to gw 
    lastSendFlow=now;
  }
  
    // Pulse count has changed
    if (pulseCount_H != oldPulseCount_H) {
       Serial.print("pulseCount_H:");  // Send watt value to gw 
        Serial.println(pulseCount_H);  // Send watt value to gw 
      double volume = ((double)pulseCount_H*((double)PULSE_FACTOR_H));     
      oldPulseCount_H = pulseCount_H;
      if (oldvolume != volume) {
            Serial.print("volume:");  // Send watt value to gw 
        Serial.println(volume);  // Send watt value to gw  oldvolume = volume;
      }
    }    
  
    lastSend_H = now;
  } else if (sendTime_H && !pcReceived_H) {
    // No count received. Try requesting it again
    
    lastSend_H=now;
  }
  
  if (SLEEP_MODE) {
    
  }
 
}



void onPulse_E()     
{ 
  
  Serial.print("Pulse received on E-meter");
  Serial.println(pulseCount_E);
  unsigned long newBlink = micros();  
  unsigned long interval = newBlink-lastBlink_E;
  if (interval<10000L) { // Sometimes we get interrupt on RISING
    Serial.println("not counting Emeter pulse: pulse too soon");
    return;
  }
  watt = (3600000000.0 /interval) / ppwh;
  lastBlink_E = newBlink;
  pulseCount_E++;
}

void onPulse_H()     
{ 
 
  Serial.print("Pulse received on WaterMeter ");
  Serial.println(pulseCount_H);
  unsigned long newBlink = micros();  
  unsigned long interval = newBlink-lastBlink_H;
  if (interval<10000L) { // Sometimes we get interrupt on RISING
    Serial.println("not counting H-meter pulse: pulse too soon");
    return;
  }
  flow = flow + ppLiter;
  lastBlink_H = newBlink;
  pulseCount_H++;
}


