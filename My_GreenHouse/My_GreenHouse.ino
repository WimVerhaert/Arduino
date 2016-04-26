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
 * Greenhouse setup to monitor couple variable:
 * - humidity and temp => DHT22
 * - moisture of soil ( 2 inputs)
 * - temperature of soil => TMP36 (36gz)
 * - check light intensity
 *
 * if humidity or temperature are too high I turn on a 4-wire computer fan => 25kHz PWM
 * Everything is send back to MyController
 *
 */

#include <MySensor.h>  
#include <Wire.h> 
#include <DHT.h> 
#include <SPI.h>

boolean dht_enabled = true;
boolean fan_enabled = true;
boolean soilmoister_enabled=false;
boolean ldr_enabled= false;
boolean tempSoil_enabled= false;
int retryloopCount = 0;
bool ret;
//unsigned long SLEEP_TIME = 120000; // Sleep time between measurements and reports (in milliseconds)

unsigned long SLEEP_TIME = 1000; // Sleep time between measurements and reports (in milliseconds)
#define DHT_SENSOR_DIGITAL_PIN 8

#define SOILTEMP_SENSOR_ANALOG_PIN A1

#define CHILD_ID_SOIL_TEMP 1   // Id of the sensor child TMP36
#define CHILD_ID_HUMTEMP 2
//#define CHILD_ID_TEMP 3
#define CHILD_ID_LIGHT 4
#define CHILD_ID_FAN 10
#define CHILD_ID_SOILMOISTURE_0 100
#define CHILD_ID_SOILMOISTURE_1 101
#define CHILD_ID_SOILMOISTURE_2 102
#define CHILD_ID_SOILMOISTURE_3 103



#define DHTTYPE DHT22   // DHT 22  (AM2302)

//I2C variables
#define I2C_REQ_DELAY_MS         2  // used for IO reads - from node's memory (fast)
#define I2C_REQ_LONG_DELAY_MS    5  //used for configuration etc.
enum { 
  I2C_CMD_GET_ANALOGS = 1
};

//Initialize de gw sensor
MySensor gw;

// Initialize soil moisture message
int maxHum = 60;
int maxTemp = 21;
int i2cSlaveAddr_SOILMOISTURE = 10;
MyMessage msgSoilMoisture_0(CHILD_ID_SOILMOISTURE_0, V_LEVEL);
MyMessage msgSoilMoisture_1(CHILD_ID_SOILMOISTURE_1, V_LEVEL);
MyMessage msgSoilMoisture_2(CHILD_ID_SOILMOISTURE_2, V_LEVEL);
MyMessage msgSoilMoisture_3(CHILD_ID_SOILMOISTURE_3, V_LEVEL);

// Initialize LDR
#define LIGHT_SENSOR_ANALOG_PIN A0
MyMessage msgLDR(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
int lastLightLevel;

// Initialize Humidity and Temperature sensor
DHT dht(DHT_SENSOR_DIGITAL_PIN, DHTTYPE);
float lastTemp;
float lastHum;
boolean metric = true; 
unsigned long lastSendTemp = 0;
unsigned long lastSendHum = 0;
MyMessage msgHum(CHILD_ID_HUMTEMP, V_HUM);
MyMessage msgTemp(CHILD_ID_HUMTEMP, V_TEMP);

// Initialize the FAN 
#define FAN_ENABLE_DIGITAL_PIN 7
unsigned long FanOn_Duration = 900000; //default value for FAN to be on is 15 minutes 
boolean fanON = false; //do we turn on the FAN?
MyMessage msgFANON_State(CHILD_ID_FAN, V_TRIPPED);
//MyMessage msgFANON_Duration(CHILD_ID_FAN, V_VAR5); //retrieve how many seconds we turn the fan on
unsigned long fanStartTime;

// Initialize soil  Temperature sensor
#define SOILTEMP_SENSOR_ANALOG_PIN A1
float lastSoilTemp;
unsigned long lastSendSoilTemp = 0;
MyMessage msgSoilTemp(CHILD_ID_SOIL_TEMP, V_TEMP);



void setup()  
{  
  gw.begin(incomingMessage);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("GreenHouse Sensors", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_SOIL_TEMP, S_TEMP);  
  gw.present(CHILD_ID_HUMTEMP, S_HUM);
  gw.present(CHILD_ID_HUMTEMP, S_TEMP);
  gw.request(CHILD_ID_HUMTEMP, V_VAR3);
  gw.request(CHILD_ID_HUMTEMP, V_VAR4);
  
  gw.present(CHILD_ID_FAN, S_DOOR);
  gw.request(CHILD_ID_FAN, V_VAR5); //get the Fan On duration from Gateway
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  gw.present(CHILD_ID_SOILMOISTURE_0, S_MOISTURE);
  gw.present(CHILD_ID_SOILMOISTURE_1, S_MOISTURE);
  gw.present(CHILD_ID_SOILMOISTURE_2, S_MOISTURE);
  gw.present(CHILD_ID_SOILMOISTURE_3, S_MOISTURE);  
  
  dht.begin();
  metric = gw.getConfig().isMetric;
  Serial.print("Default Duration the Fan should be on:");
  Serial.println(FanOn_Duration);
  Wire.begin(); 
}

void loop()     
{    
	gw.process();
	if (dht_enabled) {
	  // read the humidity and temperature
	  readDHT22(); 
	}
	if (ldr_enabled){
	  // read light intensity 
	  readLDR();
	}
	if (tempSoil_enabled) {
	readSoilTemp();
	}
	if (soilmoister_enabled) {
	  // start to read moisture of ground on Slave Node
	  startSoilMoisture();
	  delay(I2C_REQ_DELAY_MS);
	  // read soil temperature over I2C
	  readSoilTemp();
	}  

	if (fan_enabled) {
	//do we need to turn on the fan?
	  if (fanON){		
		 /************************************************
		 * turn the fan pin on/off based on temp
		 ************************************************/
		  unsigned long now = millis();
		  if(now - fanStartTime>FanOn_Duration)
		  { 
			digitalWrite(FAN_ENABLE_DIGITAL_PIN,LOW);
			Serial.println("Turning FAN OFF. ");
			fanON = false;
			retryloopCount = 0;
			do
			{
			  ret =  gw.send(msgFANON_State.set(0), true); 
			  retryloopCount++;
			  gw.wait(500);
			}
			while ((retryloopCount < 10) && !ret);
		  }		
	  }
	}
	//gw.sleep(SLEEP_TIME);
 gw.delay(2000);
}


void readDHT22() {
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  delay(2000);
  float hum = dht.readHumidity();
  // Read temperature as Celsius
  float temp = dht.readTemperature();
   // Check if any reads failed and exit early (to try again).
  if (isnan(hum) && isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: "); 
  Serial.print(hum);
  Serial.println(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(temp);
  Serial.println(" *C ");
  
  //process temperature
  if (isnan(temp)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temp != lastTemp){
    lastTemp = temp;
    retryloopCount = 0;
	do
		{
		ret =  gw.send(msgTemp.set(temp, 1),true); 
		retryloopCount++;
		gw.wait(500);
		}
	while ((retryloopCount < 10) && !ret);
  }
  
  //process humidity
   if (isnan(hum)) {
      Serial.println("Failed reading Humidity from DHT");
  } else if (hum != lastHum){
    lastHum = hum;
    gw.send(msgHum.set(hum, 1));
  }
  if (((temp>maxTemp) || (hum>maxHum))&& !fanON){
	retryloopCount = 0;
	do
		{
		ret =  gw.send(msgFANON_State.set(1), true); 
		retryloopCount++;
		gw.wait(500);
		}
	while ((retryloopCount < 10) && !ret);
  	fanON=true;
    fanStartTime = millis();
	Serial.println("Turning ON FAN for some time.");
  	digitalWrite(FAN_ENABLE_DIGITAL_PIN,HIGH);
  }
  else if (!fanON){
  	fanON=false;
    fanStartTime = 0;
	Serial.println("Turning OFF FAN.");
  	digitalWrite(FAN_ENABLE_DIGITAL_PIN,LOW);
  }
  
}

void readSoilTemp() {
   // Reading temperature !
  float samples[8];     
  float tempGround = 0; 
  // Array to hold 8 samples for Average temp calculation
  for(int i = 0;i<=7;i++){                                           // gets 8 samples of temperature
 	samples[i] = ( 4.4 * analogRead(SOILTEMP_SENSOR_ANALOG_PIN) * 26) / 1024.0;    // conversion math of tmp36GZ sample to readable temperature and stores result to samples array. 
		if (isnan(samples[i])) {
      		Serial.println("Failed reading temperature from DHT");
      		i--;
		} else {
			tempGround = tempGround + samples[i];	
		}
  }
  
  tempGround = tempGround/8	;
  
  Serial.print("Temperature: "); 
  Serial.print(tempGround);
  Serial.println(" *C ");
  
  //process temperature
  if (tempGround != lastTemp){
    lastTemp = tempGround;
    gw.send(msgSoilTemp.set(tempGround, 1));
  }
}

void readLDR() {
 int lightLevel = (1023-analogRead(LIGHT_SENSOR_ANALOG_PIN))/10.23; 
  Serial.println(lightLevel);
  if (lightLevel != lastLightLevel) {
      gw.send(msgLDR.set(lightLevel));
      lastLightLevel = lightLevel;
  }
  
  gw.send(msgLDR.set(lightLevel, 1));
} 

void startSoilMoisture() {
	 //requesting analogs read: 
  	Wire.beginTransmission(i2cSlaveAddr_SOILMOISTURE); 
  	Wire.write((uint8_t)I2C_CMD_GET_ANALOGS);  
  	Wire.endTransmission();  
	
}

void readSoilMoisture(){
	// master knows slave should return 4 bytes to the I2C_CMD_GET_ANALOGS command
  int respVals[4];

  Wire.requestFrom(i2cSlaveAddr_SOILMOISTURE, 4);

  uint8_t respIoIndex = 0;

  if(Wire.available())
    for (byte r = 0; r < 4; r++)
      if(Wire.available()){ 
        respVals[respIoIndex] = (uint8_t)Wire.read();
        respIoIndex++;      
      }
      else{
        // log or handle error: "missing read"; if you are not going to do so use r index instead of respIoIndex and delete respoIoIndex from this for loop
        break;
      }
  // now the respVals array should contain analog values for each sensor input in the same order as defined in slave (respVals[0] - A0, respVals[1] - A1 ...)
  //do something with the data
  //gw.send(msgSoilMoisture.set(30, 1)); 
}
void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR3) {  
    maxHum = message.getLong();
    Serial.print("Received maximum Humidity:");
    Serial.println(maxHum);
  }
  if (message.type==V_VAR4) {  
    maxTemp = message.getLong();
    Serial.print("Received maximum Temperature:");
    Serial.println(maxTemp);
  }
  if (message.type==V_VAR5) {  
    FanOn_Duration = message.getLong();
    Serial.print("Received Duration the Fan should be on:");
    Serial.println(FanOn_Duration);
  }
  if (message.type==V_TRIPPED) {  
    fanON = message.getBool();
    Serial.print("Received message to turn ");
	Serial.print(fanON) ;
	Serial.println(" the Fan!");
  }
}
