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
 * DESCRIPTION
 *
 * Interrupt driven binary switch example with dual interrupts
 * Original Author: Patrick 'Anticimex' Fallberg
 * Author: Wim Verhaert
 * Connect one button or door/window reed switch between
 * digitial I/O pin 3 (BUTTON_PIN below) and GND and the other
 * one in similar fashion on digital I/O pin 2.
 *
 */


#include <MySensor.h>
#include <SPI.h>

#define SKETCH_NAME "Cat Door"
#define SKETCH_MAJOR_VER "2"
#define SKETCH_MINOR_VER "0"

#define PRIMARY_CHILD_ID 3
#define SECONDARY_CHILD_ID 4

#define PRIMARY_BUTTON_PIN 2   // Arduino Digital I/O pin for button switch
#define SECONDARY_BUTTON_PIN 3 // Arduino Digital I/O pin for button switch
#define BATTERY_SENSE_PIN A0  // select the input pin for the battery sense point

#if (PRIMARY_BUTTON_PIN < 2 || PRIMARY_BUTTON_PIN > 3)
#error PRIMARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (SECONDARY_BUTTON_PIN < 2 || SECONDARY_BUTTON_PIN > 3)
#error SECONDARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (PRIMARY_BUTTON_PIN == SECONDARY_BUTTON_PIN)
#error PRIMARY_BUTTON_PIN and BUTTON_PIN2 cannot be the same
#endif
#if (PRIMARY_CHILD_ID == SECONDARY_CHILD_ID)
#error PRIMARY_CHILD_ID and SECONDARY_CHILD_ID cannot be the same
#endif

boolean SLEEP_MODE = true;      // Watt-value can only be reported when sleep mode is false.
MySensor sensor_node;

// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msgOut(PRIMARY_CHILD_ID, V_TRIPPED);
MyMessage msgIn(SECONDARY_CHILD_ID, V_TRIPPED);

boolean CatOutTrigger = false;
boolean CatInTrigger = false;
unsigned long previousReq = 0;
#define EntryTime 5000 //we allow the cat to move in/out for 5000 ms, so no extra trigger will be used in the mean time 
#define maxBattery 4800 // battery level at full charge 4xAA batteries = 4 * 1.2V

void setup()
{
  sensor_node.begin(incomingMessage);

  // Setup the buttons
  pinMode(PRIMARY_BUTTON_PIN, INPUT);
  pinMode(SECONDARY_BUTTON_PIN, INPUT);

  // Activate internal pull-ups
  digitalWrite(PRIMARY_BUTTON_PIN, HIGH);
  digitalWrite(SECONDARY_BUTTON_PIN, HIGH);

  // Send the sketch version information to the gateway and Controller
  sensor_node.sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register binary input sensor to sensor_node (they will be created as child devices)
  // We use S_DOOR
    sensor_node.wait(500);
  sensor_node.present(PRIMARY_CHILD_ID, S_DOOR);
    sensor_node.wait(500);
  sensor_node.request(PRIMARY_CHILD_ID, V_VAR1);
    sensor_node.wait(500);
  sensor_node.present(SECONDARY_CHILD_ID, S_DOOR);
    sensor_node.wait(500);

}

void loop()
{

  bool ret;
  int8_t loopCount;
  if (CatOutTrigger)
  {
    loopCount = 0;
    do
    {
      ret = sensor_node.send(msgOut.set(1), true); // Send volume value to gw
      Serial.print("ret sending:");
      Serial.println(ret);
      loopCount++;
      sensor_node.wait(500);
    }
    while ((loopCount < 10) && !ret);
  }

  if (CatInTrigger)
  {
    loopCount = 0;
    do
    {
      ret = sensor_node.send(msgIn.set(1), true); // Send volume value to gw
      Serial.print("ret sending:");
      Serial.println(ret);
      loopCount++;
      sensor_node.wait(500);
    }
    while ((loopCount < 10) && !ret);
  }

  sensor_node.wait(EntryTime);
  loopCount = 0;
  if (CatInTrigger || CatOutTrigger)
  {
    do
    {
      ret = sensor_node.send(msgOut.set(0), true); // Send volume value to gw
      loopCount++;
      sensor_node.wait(500);
    }
    while ((loopCount < 10) && !ret);

    loopCount = 0;
    do
    {
      ret = sensor_node.send(msgIn.set(0), true); // Send volume value to gw
      loopCount++;
      sensor_node.wait(500);
    }
    while ((loopCount < 10) && !ret);
    
    CatInTrigger = false;
    CatOutTrigger = false;
  }
  //unsigned long batteryPcnt = (readVcc() * 100) / maxBattery;
  //Serial.print("Battery:");
  //Serial.print(batteryPcnt);
  //Serial.print("---");
  //Serial.println(readVcc());
  sensor_node.sendBatteryLevel(readVcc());
  sensor_node.wait(500);
  
  sensor_node.request(PRIMARY_CHILD_ID, V_VAR1);
  sensor_node.wait(2500);
    
  // Sleep until something happens with the sensor
  if (SLEEP_MODE){ 
    
    int8_t retValue = sensor_node.sleep(PRIMARY_BUTTON_PIN - 2, FALLING, SECONDARY_BUTTON_PIN - 2, FALLING, 0);
    Serial.print("interrupt:");
    Serial.print(retValue);
    if (retValue == 0)
    {
      CatOutTrigger = true;
      Serial.println("Pulse received Cat going out");
  
    }
    if (retValue == 1)
    {
      CatInTrigger = true;
      Serial.println("Pulse received Cat going in");
    }
  } else {
      unsigned long now = millis();
      bool req= now - previousReq > 10000;
      if (req) {
        sensor_node.request(PRIMARY_CHILD_ID, V_VAR1);
        previousReq = now;
      }
   
  }
}

void incomingMessage(const MyMessage & message)
{
  if (message.type == V_VAR1)
  {
    SLEEP_MODE = message.getBool();
    Serial.print("Received SLEEPMODE:");
    Serial.println(SLEEP_MODE);
  }
}

long readVcc()
{
    // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
  // analogReference(INTERNAL1V1);
#else
   //analogReference(INTERNAL);
#endif

 int sensorValue = analogRead(BATTERY_SENSE_PIN);
// 1M, 470K divider across battery and using internal ADC ref of 1.1V
   // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
   // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
   // 3.44/1023 = Volts per bit = 0.003363075
   float batteryV  = sensorValue * 0.0047212121212121;//0.003363075;
   int batteryPcnt = (batteryV / 4.8)*100;
    Serial.print("sensorValue:");
    Serial.println(sensorValue);
        Serial.print("batteryV:");
    Serial.println(batteryV);
    Serial.print("batteryPcnt:");
    Serial.println(batteryPcnt);
  return batteryPcnt; // Vcc in millivolts
}
