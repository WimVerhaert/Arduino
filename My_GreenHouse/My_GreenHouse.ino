/**
 * Using the MySensors Arduino library 
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 * REVISION HISTORY
 * Version 1.0 - Wim Verhaert
 * 
 * DESCRIPTION
 * Greenhouse setup to monitor couple variable:
 * - humidity and temp => DHT22
 * - moisture of soil ( 4 inputs) => coming from extention module via i2c
 * - temperature of soil => TMP36 (36gz)
 * - check light intensity
 *
 * If humidity or temperature are too high I turn on a computer fan through a TIP120 transistor 
 * All measured data is send back to MyController.
 * Some default values are retrieved/controlled from MyController.
 * - Fan ON duration
 * - maximum tempeerature
 * - maximum humidity
 * - Fan on can be triggered
 *
 */

#include <MySensor.h>  
#include <Wire.h> 
#include <DHT.h> 
#include <SPI.h>
#include <LiquidCrystal_I2C.h>


//identify which sensors to use by enabling them
boolean dht_enabled = true; //temp/hum sensor
boolean fan_enabled = true; //Fan is present and can be switched on/off
boolean soilmoister_enabled=false; //soilmoister module is present on remote i2c sensor-extention
boolean ldr_enabled= true; // lightLevel can be measured
boolean tempSoil_enabled= false; // temperature of soil can be read

//some program-wide used variables
uint8_t retryLoopCounter = 0;
uint8_t maxRetryCount = 10; // how many time will we attempt to send a message to the GW befire giving up
boolean ret;
unsigned long SLEEP_TIME_RFI = 120000;//; // Sleep time between measurements and reports (in milliseconds)
unsigned long SLEEP_TIME_LCD = 10000;//; // Sleep time between measurements and reports (in milliseconds)
unsigned long SLEEP_TIME = SLEEP_TIME_RFI; //default sleep time

//define the pins used
#define USE_NRF24_DIGITAL_PIN 4 //pin for turning the fan on/off
#define FAN_ENABLE_DIGITAL_PIN A2 //pin for turning the fan on/off
#define DHT_SENSOR_DIGITAL_PIN 8  //pin for the sensor child DHT22
#define LIGHT_SENSOR_ANALOG_PIN A0 // pin for the light level sensor
#define SOILTEMP_SENSOR_ANALOG_PIN A1 //pin for the sensor for temp of soil

//I2C variables
#define I2C_REQ_DELAY_MS         2  // used for IO reads - from node's memory (fast)
#define I2C_REQ_LONG_DELAY_MS    5  //used for configuration etc.
enum { 
  I2C_CMD_GET_ANALOGS = 1,
  I2C_CMD_TEST = 2
};
#define I2C_TIMEOUT 500
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//Initialize de gw sensor
MySensor gw;
#define CHILD_ID_SOIL_TEMP 1   // Id of the sensor child TMP36
#define CHILD_ID_HUMTEMP 2 //id of the sensor child for Hum and temp
#define CHILD_ID_LIGHT 4 //id of the sensor child for light level
#define CHILD_ID_FAN 10 //id of the sensor child for turnig fan on/off
#define CHILD_ID_SOILMOISTURE_0 100 //id of the sensor child for moisture of soil
#define CHILD_ID_SOILMOISTURE_1 101 //id of the sensor child for moisture of soil
#define CHILD_ID_SOILMOISTURE_2 102 //id of the sensor child for moisture of soil
#define CHILD_ID_SOILMOISTURE_3 103 //id of the sensor child for moisture of soil

// Initialize soil moisture message
int i2cSlaveAddr_SOILMOISTURE = 10;
int soilVals[3];
MyMessage msgSoilMoisture_0(CHILD_ID_SOILMOISTURE_0, V_LEVEL);
MyMessage msgSoilMoisture_1(CHILD_ID_SOILMOISTURE_1, V_LEVEL);
MyMessage msgSoilMoisture_2(CHILD_ID_SOILMOISTURE_2, V_LEVEL);
MyMessage msgSoilMoisture_3(CHILD_ID_SOILMOISTURE_3, V_LEVEL);

// Initialize LDR
MyMessage msgLDR(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
float lastLightLevel;

// Initialize Humidity and Temperature sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHT_SENSOR_DIGITAL_PIN, DHTTYPE);
float lastTemp;
float lastHum;
boolean pcMaxHumReceived  = false;
boolean pcMaxTempReceived  = false;
boolean metric = true; 
unsigned long lastSendTemp = 0;
unsigned long lastSendHum = 0;
MyMessage msgHum(CHILD_ID_HUMTEMP, V_HUM);
MyMessage msgTemp(CHILD_ID_HUMTEMP, V_TEMP);

// Initialize the FAN 
int maxHum = 80;
int maxTemp = 40;
boolean pcDurationReceived  = false;
unsigned long FanOn_Duration = 900000; //default value for FAN to be on is 15 minutes / 15*60*1000ms
boolean fanON = false; //status to indicate that the fan is ON/OFF
MyMessage msgFANON_State(CHILD_ID_FAN, V_TRIPPED);
unsigned long fanStartTime;

// Initialize soil  Temperature sensor
float lastSoilTemp;
unsigned long lastSendSoilTemp = 0;
MyMessage msgSoilTemp(CHILD_ID_SOIL_TEMP, V_TEMP);

boolean use_NRF24 = true;
void setup()  
{  
  pinMode(USE_NRF24_DIGITAL_PIN,INPUT_PULLUP);
  use_NRF24 = digitalRead(USE_NRF24_DIGITAL_PIN);
  if (use_NRF24) {
    gw.begin(incomingMessage,105,false);
    // Send the sketch version information to the gateway and Controller
    gw.sendSketchInfo("GreenHouse Sensors", "1.0");
    gw.wait(500);
    // Register all sensors to gw (they will be created as child devices)
    gw.present(CHILD_ID_SOIL_TEMP, S_TEMP);  
    gw.wait(500);
    gw.present(CHILD_ID_HUMTEMP, S_HUM);
    gw.wait(500);
    gw.present(CHILD_ID_HUMTEMP, S_TEMP);
    gw.wait(500);
    gw.request(CHILD_ID_HUMTEMP, V_VAR3);
    gw.wait(500);
    gw.request(CHILD_ID_HUMTEMP, V_VAR4);
    gw.wait(500);
    gw.present(CHILD_ID_FAN, S_DOOR);
    gw.wait(500);
    gw.request(CHILD_ID_FAN, V_VAR5); //get the Fan On duration from Gateway
    gw.wait(500);
    gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
    gw.wait(500);
    gw.present(CHILD_ID_SOILMOISTURE_0, S_MOISTURE);
    gw.wait(500);
    gw.present(CHILD_ID_SOILMOISTURE_1, S_MOISTURE);
    gw.wait(500);
    gw.present(CHILD_ID_SOILMOISTURE_2, S_MOISTURE);
    gw.wait(500);
    gw.present(CHILD_ID_SOILMOISTURE_3, S_MOISTURE);  
    gw.wait(500);
    metric = gw.getConfig().isMetric;
  }
  else {
     Serial.begin(115200);
      lcd.init();
    Serial.println("We will be using the I2C display");
     lcd.setCursor(0, 0);
      lcd.print ("My GreenHouse");
      lcd.setCursor(0, 1); 
      lcd.print("********************  "); 
     if (!use_NRF24) {
        lcd.noBacklight();
     }
  }
 
 delay(2000);
  
  dht.begin();
  
  Wire.begin(); 
}

void loop()     
{    
  use_NRF24 = digitalRead(USE_NRF24_DIGITAL_PIN);
 if (!use_NRF24) {
   lcd.init();
    lcd.noBacklight();
    writeLCD("My GreenHouse","By Wim Verhaert");
    delay(1000);
    SLEEP_TIME = SLEEP_TIME_LCD;
    }
 else {
    writeLCD("","");
  lcd.noBacklight();
  lcd.noDisplay();
   SLEEP_TIME = SLEEP_TIME_RFI;
 }
  
    
	if (dht_enabled) {
     if (use_NRF24) {
    		//retrieve maximum temp and Humidity from controller
    		if (!pcMaxHumReceived){
    		  gw.request(CHILD_ID_HUMTEMP, V_VAR4);
    		}
    		if (!pcMaxTempReceived){
    		  gw.request(CHILD_ID_HUMTEMP, V_VAR3);
    		}
     }
		  // read the humidity and temperature
		  readDHT22(); 
	  
		if (!use_NRF24) {
		  writeLCD("Humidity: "+String(lastHum),"Temp: "+String(lastTemp)+"C");
      delay(1000);
		}

	    
	}
	if (ldr_enabled){
	  // read light intensity 
		readLDR();

		
			if (!use_NRF24) {
			  writeLCD("Light Level: "+ String(lastLightLevel),"");
        delay(1000);

			}
			
	 
	}
	
	if (tempSoil_enabled) {
		readSoilTemp();
	
			if (!use_NRF24) {
			  writeLCD("Soil Temp:" + String(lastSoilTemp)+"C","");
        delay(1000);
			}
	  
	}
	
	if (soilmoister_enabled) {
	  // start to read moisture of ground on Slave Node
	  startSoilMoisture();
	  delay(I2C_REQ_DELAY_MS);
	  // read soil temperature over I2C
	  
		readSoilMoisture();
		
			if (!use_NRF24) {
       writeLCD("Moisture 1:" + String(soilVals[0]),"Moisture 2:" + String(soilVals[1]));
       delay(3000);
       writeLCD("Moisture 3:" + String(soilVals[2]),"Moisture 4:" + String(soilVals[3]));
       delay(1000);
			}			
	 
	}  

	if (fan_enabled) {
     if (use_NRF24) {
		    if (!pcDurationReceived){
		      gw.request(CHILD_ID_FAN, V_VAR5); //get the Fan On duration from Gateway      
		    }
     }
		
		if (fanON){		//do we need to turn on the fan?
			 /************************************************
			 * turn the fan pin off based on the time it was on
			 ************************************************/
			unsigned long now = millis();
			boolean fanRunTimeExpired = now - fanStartTime > FanOn_Duration;
			if(fanRunTimeExpired){ 
				digitalWrite(FAN_ENABLE_DIGITAL_PIN,LOW); //Turn FAN off
				Serial.println("Turning FAN OFF.");
				fanON = false;
        if (use_NRF24) {
    				retryLoopCounter = 0;
    				do
    				{
    				  ret =  gw.send(msgFANON_State.set(0), false); 
    				  retryLoopCounter++;
    				  gw.wait(500);
    				}
    				while ((retryLoopCounter < maxRetryCount) && !ret);
        }
			}		
		}
	}	
	 if (use_NRF24) {
	    gw.wait(SLEEP_TIME);
	 }
   else {
      delay(SLEEP_TIME);
   }
}

void readDHT22() {
	 // Reading temperature or humidity takes about 250 milliseconds!
	 
	double hum = dht.readHumidity();
	double temp = dht.readTemperature();
	 
	 // Check if any reads failed and exit early (to try again).
	if (isnan(hum) && isnan(temp)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}
	else {
		//  Serial.print("Humidity: "); 
		//  Serial.print(hum);
		//  Serial.println(" %\t");
		//  Serial.print("Temperature: "); 
		//  Serial.print(temp);
		//  Serial.println(" *C ");
	 
		//process temperature
		if (isnan(temp)) {
		//	Serial.println("Failed reading temperature from DHT");
		} else if (temp != lastTemp){
			lastTemp = temp;
      if (use_NRF24) {
  			retryLoopCounter = 0;
  			do
  				{
  				ret =  gw.send(msgTemp.set(temp, 1),true); 
  				retryLoopCounter++;
  				gw.wait(500);
  				}
  			while ((retryLoopCounter < maxRetryCount) && !ret);
      }
		}
		
		//process humidity
		if (isnan(hum)) {
		//	Serial.println("Failed reading Humidity from DHT");
		} else if (hum != lastHum){
			lastHum = hum;
      if (use_NRF24) {
    			retryLoopCounter = 0;
    			do
    				{
    				ret =  gw.send(msgHum.set(hum, 1),true); 
    				retryLoopCounter++;
    				gw.wait(500);
    				}
    			while ((retryLoopCounter < maxRetryCount) && !ret);		  
      }
		}
	  
		if (((temp>maxTemp) || (hum>maxHum))&& !fanON){
      if (use_NRF24) {
    			retryLoopCounter = 0;
    			do
    				{
    				ret =  gw.send(msgFANON_State.set(1), true); 
    				retryLoopCounter++;
    				gw.wait(500);
    				}
    			while ((retryLoopCounter < maxRetryCount) && !ret);
      }
			fanON=true;
			fanStartTime = millis();
			Serial.println("Turning ON FAN for some time.");
			digitalWrite(FAN_ENABLE_DIGITAL_PIN,HIGH);
		}
		else if (!fanON){
			fanON=false;
			fanStartTime = 0;
		//	Serial.println("Turning OFF FAN.");
			digitalWrite(FAN_ENABLE_DIGITAL_PIN,LOW);
		}
	}
}

void readSoilTemp() {
   // Reading temperature !
	float sample;     
	float tempGround = 0; 
	// Array to hold 8 samples for Average temp calculation
	for(int i = 0;i<=7;i++){                                           // gets 8 samples of temperature
    //Serial.print("Soil temp:");
    //Serial.println(analogRead(SOILTEMP_SENSOR_ANALOG_PIN));
		sample = ((analogRead(SOILTEMP_SENSOR_ANALOG_PIN) * 0.004882814))*100.0;    // conversion math of tmp36GZ sample to readable temperature and stores result to samples array. 
    //Serial.print("Soil temp After:");
    //Serial.println(sample);
		if (isnan(sample)) {
	//		Serial.println("Failed reading temperature from sensor");
			i--;
		} else {
			tempGround = tempGround + sample;	
		}
    delay(200);
	}
  
	tempGround = tempGround/8 + 4.3;
//	Serial.print("Average Ground Temperature: "); 
//	Serial.print(tempGround);
//	Serial.println(" *C ");
  
	//process temperature
	if (tempGround != lastTemp){
		lastTemp = tempGround;
    if (use_NRF24) {
  		retryLoopCounter = 0;
  		do
  			{
  			ret =  gw.send(msgSoilTemp.set(tempGround, 1),true);
  			retryLoopCounter++;
  			gw.wait(500);
  			}
  		while ((retryLoopCounter < maxRetryCount) && !ret);
    }
	}
}

void readLDR() {
  
	float sample;     
	float tempLDR = 0; 
	for(int i = 0;i<=7;i++){
     
		sample = analogRead(LIGHT_SENSOR_ANALOG_PIN);
       
		if (isnan(sample)) {
			//Serial.println("Failed reading LDR from sensor");
			i--;
		} else {
			tempLDR = tempLDR + sample;	
		}
    delay(200);
	}	
	tempLDR = tempLDR/8;
 
//	Serial.print("Average LightLevel: "); 
//	Serial.println(tempLDR);
  //Serial.print("Average Lux LightLevel: "); 
  //Serial.println(lux);
	
	if (tempLDR != lastLightLevel) {
		lastLightLevel = tempLDR;
    if (use_NRF24) {
    		retryLoopCounter = 0;
    		do
    			{
    			ret =  gw.send(msgLDR.set(tempLDR, 1),true);
    			retryLoopCounter++;
    			gw.wait(500);
    			}
    		while ((retryLoopCounter < maxRetryCount) && !ret);
    }
	}
} 

void startSoilMoisture() {
	 //requesting analogs read: 
//   Serial.println("Start Reading I2C Soil Moisture."); 
  	Wire.beginTransmission(i2cSlaveAddr_SOILMOISTURE); 
  	//Wire.write((uint8_t)I2C_CMD_GET_ANALOGS);  
   
	Wire.write((uint8_t)I2C_CMD_TEST);  
  	Wire.endTransmission();  
	
}

void readSoilMoisture(){
	// master knows slave should return 3 bytes to the I2C_CMD_GET_ANALOGS command
  int respVals[3];
//  Serial.println("Request Readings I2C Soil Moisture."); 
  Wire.requestFrom(i2cSlaveAddr_SOILMOISTURE, 3);

  uint8_t respIoIndex = 0;

  if(Wire.available())
    for (byte r = 0; r < 3; r++)
      if(Wire.available()){ 
        respVals[respIoIndex] = (uint8_t)Wire.read();
        soilVals[respIoIndex]=respVals[respIoIndex];
        respIoIndex++;      
      }
      else{
        // log or handle error: "missing read"; if you are not going to do so use r index instead of respIoIndex and delete respoIoIndex from this for loop
        break;
      }
  // now the respVals array should contain analog values for each sensor input in the same order as defined in slave (respVals[0] - A0, respVals[1] - A1 ...)
  //do something with the data
 
//    Serial.print("Received value:");
//    Serial.println(respVals[i]);
    if (use_NRF24) {
        retryLoopCounter = 0;
        do
          {
          ret =  gw.send(msgSoilMoisture_0.set(respVals[0], 1),true); 
          retryLoopCounter++;
          gw.wait(500);
          }
        while ((retryLoopCounter < maxRetryCount) && !ret);
        retryLoopCounter = 0;
        do
          {
          ret =  gw.send(msgSoilMoisture_1.set(respVals[1], 1),true); 
          retryLoopCounter++;
          gw.wait(500);
          }
        while ((retryLoopCounter < maxRetryCount) && !ret);
        retryLoopCounter = 0;
        do
          {
          ret =  gw.send(msgSoilMoisture_2.set(respVals[2], 1),true); 
          retryLoopCounter++;
          gw.wait(500);
          }
        while ((retryLoopCounter < maxRetryCount) && !ret);
    }

  //gw.send(msgSoilMoisture.set(30, 1)); 
}

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VAR3) {  
    maxHum = message.getLong();
    Serial.print("Received maximum Humidity:");
   Serial.println(maxHum);
    pcMaxHumReceived = true;
  }
  if (message.type==V_VAR4) {  
    maxTemp = message.getLong();
    Serial.print("Received maximum Temperature:");
    Serial.println(maxTemp);
    pcMaxTempReceived = true;
  }
  if (message.type==V_VAR5) {  
    FanOn_Duration = message.getLong()*1000;
    Serial.print("Received Duration the Fan should be on:");
    Serial.println(FanOn_Duration);
    pcDurationReceived = true;
  }
  if (message.type==V_TRIPPED) {  
    fanON = message.getBool();
	
	if (fanON){
		Serial.println("Received message to turn ON the Fan");
		if (fanON) {
		    digitalWrite(FAN_ENABLE_DIGITAL_PIN,HIGH);
		  }
	}
     else {
	 	Serial.println("Received message to turn OFF the Fan");
	    digitalWrite(FAN_ENABLE_DIGITAL_PIN,LOW);
		  }
    
  }
}

void writeLCD( String Line1, String Line2){
   // set the cursor to column 0, line 0
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(Line1);
  lcd.setCursor(0,1);
  lcd.print(Line2);
}
