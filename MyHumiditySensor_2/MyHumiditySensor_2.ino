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
 * This sketch provides an example how to implement a humidity/temperature
 * sensor using DHT11/DHT-22 
 * http://www.mysensors.org/build/humidity
 */
 
#include <SPI.h>
#include <MySensor.h>  

#define MY_NODE_ID 3
#define CHILD_ID_TEMP 0
#define TEMPERATURE_SENSOR_ANALOG_PIN A0
unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)
int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;
int BATTERY_SEND_FREQUENCY = 360;  // Send batterystatus every 6 hours
int currentCount = 358;

MySensor gw;
#define aref_voltage 3.3    
float lastTemp;
float lastHum;
//boolean metric = true; 
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);


void setup()  
{ 
  gw.begin(NULL,MY_NODE_ID,false);
  
  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Temp Living Room", "1.0");

  // Register all sensors to gw (they will be created as child devices)
 
  gw.present(CHILD_ID_TEMP, S_TEMP);
   // If you want to set the aref to something other than 5v
  analogReference(EXTERNAL);

  //metric = gw.getConfig().isMetric;
}

void loop()      
{  


  int reading = analogRead(TEMPERATURE_SENSOR_ANALOG_PIN);  
 //Serial.print("Temp reading = ");
 //Serial.print(reading);     // the raw analog reading
 
 // converting that reading to voltage, for 3.3v arduino use 3.3
 float voltage = reading * aref_voltage;
 voltage /= 1024.0; 
  float temperature = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
 
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  
  
  currentCount++;
  bool intervalBattery = currentCount > BATTERY_SEND_FREQUENCY;
  if (intervalBattery) {
  // get the battery Voltage
     int sensorValue = analogRead(BATTERY_SENSE_PIN);
     //Serial.println(sensorValue);
     
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
     currentCount = 0;
  }
  gw.sleep(SLEEP_TIME); //sleep a bit
}


