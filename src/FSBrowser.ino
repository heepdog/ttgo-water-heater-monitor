


/*
  FSWebServer - Example WebServer with FS backend for esp8266/esp32
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP32 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp32fs.local/edit; done

  access the sample web page at http://esp32fs.local
  edit the page by going to http://esp32fs.local/edit
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "time.h"
#include <LinkedList.h>
#include <ArduinoOTA.h>

#define FILESYSTEM SPIFFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM false
//#define DBG_OUTPUT_PORT Serial
#define DBG_OUTPUT_PORT tft

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif

#include <TFT_eSPI.h>
#include <Button2.h>
#include "config.h"
#include "serverfunctions.h"

const char* ssid = "heppners-2";
const char* password = "Cannotcrackit";
const char* host = "esp32fs";

//File fsUploadFile;
//WebServer server(80);

volatile int interruptCounter = 1;

 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void setup(void) {
//  DBG_OUTPUT_PORT.begin(115200);
//  DBG_OUTPUT_PORT.print("\n");
//  DBG_OUTPUT_PORT.setDebugOutput(true);
  Serial.begin(115200);

  setupTFT();
  
  if (FORMAT_FILESYSTEM) FILESYSTEM.format();
  FILESYSTEM.begin();
  {
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while (file) {
      String fileName = file.name();
      size_t fileSize = file.size();
      //DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      file = root.openNextFile();
    }
    DBG_OUTPUT_PORT.printf("\n");
  }


  //WIFI INIT
  WiFi.disconnect(false,true);
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
//  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
//  }
  int connectTries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
    delay(500);
    DBG_OUTPUT_PORT.print(".");
    connectTries+=1;
//    if (connectTries>10)
    if (WiFi.status() == WL_CONNECT_FAILED)
      ESP.restart();
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! \nIP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

//  MDNS.begin(host);
//    if (!MDNS.begin(host)) {
//        Serial.println("Error setting up MDNS responder!");
//        while(1) {
//            delay(1000);
//        }
//    }

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000*60, true);
  timerAlarmEnable(timer);

  //set time from ntp server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.setHostname(host);
  ArduinoOTA.begin();

  //int btnClick = false;

  serverSetup();

  setupButtons();
  
  DBG_OUTPUT_PORT.println("HTTP server started");

}


void loop(void) {

  average(analogRead(33), &temp1Average, &temp1Total);
  average(analogRead(32), &temp2Average, &temp2Total);

  ArduinoOTA.handle();
  server.handleClient();
  btn1.loop();
  btn2.loop();

  if (interruptCounter > 0) {
 
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;

    struct sample testsample;

    
    getLocalTime(&testsample.timeinfo);
    
    testsample.value = temp1Average;

    if (temp1buffer.size() > (60*24)){
      Serial.print("removing oldest sample");
      temp1buffer.remove(0);
    }

    temp1buffer.add(testsample);
 
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
 
  }

}
