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
 * http://www.mysensors.org/build/distance
 */

#include <SPI.h>
#include <MySensor.h>  
#include <NewPing.h>

#define between(x, a, b)  (((a) <= (x)) && ((x) <= (b)))
#define CHILD_ID 1
#define VAR_ID 2
#define TRIGGER_PIN  6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
unsigned long SLEEP_TIME = 2000; // Sleep time between reads (in milliseconds)

MySensor gw;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
MyMessage msg(CHILD_ID, V_DISTANCE);

int lastDist;
boolean metric = true; 
int catHeight = 0;
int mendelHeight = 0;
int eliasHeight = 0;
int martineHeight = 0;
int wimHeight = 0;

void setup()  
{ 
  gw.begin(incomingMessage);
  gw.wait(1000);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Distance Sensor", "1.0");
  gw.wait(1000);

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID, S_DISTANCE);
  gw.wait(1000);
  gw.request(VAR_ID, V_VAR1);
  gw.wait(1000);
  gw.request(VAR_ID, V_VAR2);
  gw.wait(1000);
  gw.request(VAR_ID, V_VAR3);
  gw.wait(1000);
  gw.request(VAR_ID, V_VAR4);
  gw.wait(1000);
  gw.request(VAR_ID, V_VAR5);
  gw.wait(1000);
  metric = gw.getConfig().isMetric;
  gw.wait(1000);
}

void loop()      
{     
  if ((catHeight)==0) {
    Serial.println("Request cat height:");
    gw.request(VAR_ID, V_VAR1);  
    gw.wait(2000);
  }
  if ((mendelHeight)==0) {
    Serial.println("Request mendel height:");
    gw.request(VAR_ID, V_VAR2);  
    gw.wait(2000);
  }
  if ((eliasHeight)==0) {
    Serial.println("Request eliasHeight:");
    gw.request(VAR_ID, V_VAR3);  
    gw.wait(2000);
  }
  if ((martineHeight)==0) {
    Serial.println("Request martineHeight:");
    gw.request(VAR_ID, V_VAR4); 
    gw.wait(2000); 
  }
  if ((wimHeight)==0) {
    Serial.println("Request wimHeight:");
    gw.request(VAR_ID, V_VAR5);  
    gw.wait(2000);
  }    
  
  int dist = metric?sonar.ping_cm():sonar.ping_in();
  Serial.print("Ping: ");
  Serial.print(dist); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.println(metric?" cm":" in");

  if (dist != lastDist) {
      if (between(dist,10,catHeight+10)) {
        gw.send(msg.set(1));
      }
      if (between(dist,mendelHeight-10,mendelHeight+10)) {
        gw.send(msg.set(2));
      }
      if (between(dist,eliasHeight-10,eliasHeight+10)) {
        gw.send(msg.set(3));
      }
      if (between(dist,martineHeight-10,martineHeight+10)) {
        gw.send(msg.set(4));
      }
      if (between(dist,wimHeight-10,wimHeight+1)) {
        gw.send(msg.set(5));
      }
      if (between(dist,wimHeight+10,210)) {
        gw.send(msg.set(6));
      }
      if (between(dist,210,500)) {
        gw.send(msg.set(0));
      }
     
      lastDist = dist;
  }

  gw.sleep(SLEEP_TIME);
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR1) {  
    catHeight = message.getLong();
    Serial.print("Received cat height:");
    Serial.println(catHeight);
  }
  if (message.type==V_VAR2) {  
    mendelHeight = message.getLong();
    Serial.print("Received mendelHeight:");
    Serial.println(mendelHeight);
  }
  if (message.type==V_VAR3) {  
    eliasHeight = message.getLong();
    Serial.print("Received eliasHeight:");
    Serial.println(eliasHeight);
  }
  if (message.type==V_VAR4) {  
    martineHeight = message.getLong();
    Serial.print("Received martineHeight:");
    Serial.println(martineHeight);
  }
  if (message.type==V_VAR5) {  
    wimHeight = message.getLong();
    Serial.print("Received wimHeight:");
    Serial.println(wimHeight);
  }
}

