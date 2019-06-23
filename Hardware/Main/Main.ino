#include "SwitchPack.h"
#include <EEPROM.h>

#define btSerial Serial1
#define hit_lim_addr 0
#define day_offset 1
#define sync_year_addr 2
#define sync_month_addr 3
#define sync_day_addr 4
#define hit_start_addr 5
#define btnPin 13
#define ledPin 12
#define btStatePin 11
#define REACTION_TIME 500

DoubleClick key(btnPin, PULLUP, REACTION_TIME);
byte btnPresses = 0;
String btData = "";

void blinkLed(int duration, int n){
    for(int i = 0; i < n; i++){
        digitalWrite(ledPin, HIGH);
        long start = millis();
        while(millis() - start < duration){}
        digitalWrite(ledPin, LOW);
    }
}

void syncData(){
    btData = "";
    while(btSerial.available() > 0){
        btData += btSerial.read();
    }
    Serial.println(btData);
    btData = "";
    btData += EEPROM.read(sync_year_addr);
    btData += ",";
    btData += EEPROM.read(sync_month_addr);
    btData += ",";
    btData += EEPROM.read(sync_day_addr);
    btData += "/";
    for(int i = 0; i < 5; i++){
        btData += EEPROM.read(hit_start_addr + i);
        btData += ",";
    }
    btData.trim();
    Serial.println();
    Serial.println(btData);
    btSerial.print(btData);
    btData = "";
    while(btSerial.available() > 0){
        btData += btSerial.read();
    }
    btSerial.end();
}

void setup(){
    Serial.begin(9600);
    key.begin();
    key.setMaxClicks(3);
    pinMode(ledPin, OUTPUT);
    EEPROM.write(hit_lim_addr, 100);
    EEPROM.write(sync_year_addr, 19);
    EEPROM.write(sync_month_addr, 6);
    EEPROM.write(sync_day_addr, 22);
    EEPROM.write(day_offset, 5);
    for(int i = 0; i < 5; i++){
        EEPROM.write(hit_start_addr + i, i * 10);
    }
}

void loop(){
    //One Day has passed, increment day counter
    if((millis() / 100) % 86400 == 0){
        EEPROM.write(day_offset, EEPROM.read(day_offset) + 1);
    }
    btnPresses = key.clickCount();
    
    if(btnPresses == 1){
        Serial.println(btnPresses);
        if(EEPROM.read(hit_start_addr) + EEPROM.read(day_offset) < EEPROM.read(hit_lim_addr)){
            blinkLed(1000, 1);
            EEPROM.write(EEPROM.read(hit_start_addr) + EEPROM.read(day_offset), EEPROM.read(hit_start_addr) + EEPROM.read(day_offset));
            Serial.println("Hit allowed");
        } else {
            Serial.println("Hit not allowed");
        }
    } else if(btnPresses == 3){
        Serial.println("Pairing Mode");
        blinkLed(3, 500);
        btSerial.begin(9600);
        while(!digitalRead(btStatePin)){
            blinkLed(100, 1);
        }
        Serial.println("Paired");
        Serial.println("Sending");
        syncData();
    }
}