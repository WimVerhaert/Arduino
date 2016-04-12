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
 * Version 1.0 - Henrik Ekblad
 * 
 * DESCRIPTION
 * Motion Sensor example using HC-SR501 
 * http://www.mysensors.org/build/motion
 *
 */

#include <MySensor.h>  
#include <DHT.h> 
#include <SPI.h>

//unsigned long SLEEP_TIME = 120000; // Sleep time between reports (in milliseconds)
unsigned long SLEEP_TIME = 10000; // Sleep time between reports (in milliseconds)
#define DIGITAL_INPUT_SENSOR 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define DHT_SENSOR_DIGITAL_PIN 4
#define CHILD_ID_MOTION 1   // Id of the sensor child
#define CHILD_ID_HUM 2
#define CHILD_ID_TEMP 3
#define DHTTYPE DHT22   // DHT 22  (AM2302)

MySensor gw;
// Initialize motion message
MyMessage msg(CHILD_ID_MOTION, V_TRIPPED);

// Initialize Humidity and Temperature sensor
DHT dht(DHT_SENSOR_DIGITAL_PIN, DHTTYPE);
float lastTemp;
float lastHum;
boolean metric = true; 
unsigned long lastSendTemp = 0;
unsigned long lastSendHum = 0;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

void setup()  
{  
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Motion/Temp/Humidity Sensor", "1.0");

  pinMode(DIGITAL_INPUT_SENSOR, INPUT);      // sets the motion sensor digital pin as input
  
  attachInterrupt(INTERRUPT, readPIR, CHANGE);
  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_MOTION, S_MOTION);  
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  dht.begin();
  metric = gw.getConfig().isMetric;
}

void loop()     
{    
  // read the motion sensor
  
  // read the humidity and temperature
  readDHT22(); 
  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR) == HIGH; 
  Serial.print("Motion Sensor State: ");      
  Serial.println(tripped);
  
  gw.send(msg.set(tripped?"1":"0"));  // Send tripped value to gw 
  
  // Sleep until interrupt comes in on motion sensor. Send update every two minute. 
  gw.sleep(SLEEP_TIME);
}

void readPIR() {
  // Read digital motion value
  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR) == HIGH; 
  Serial.print("Motion Sensor State: ");      
  Serial.println(tripped);
  
  gw.send(msg.set(tripped?"1":"0"));  // Send tripped value to gw 
  
}

void readDHT22() {
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float hum = dht.readHumidity();
  // Read temperature as Celsius
  float temp = dht.readTemperature();
   // Check if any reads failed and exit early (to try again).
  if (isnan(hum) && isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  //process temperature

  if (isnan(temp)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temp != lastTemp){
    lastTemp = temp;
    gw.send(msgTemp.set(temp, 1));
  }
  
  //process humidity
   if (isnan(hum)) {
      Serial.println("Failed reading Humidity from DHT");
  } else if (hum != lastHum){
    lastHum = hum;
    gw.send(msgHum.set(hum, 1));
  }
  
  Serial.print("Humidity: "); 
  Serial.print(hum);
  Serial.println(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(temp);
  Serial.println(" *C ");
}

