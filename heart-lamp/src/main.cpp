#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_MPR121.h"

#include <CapacitiveSensor.h>


// Pin Initialization
#define PIN_PIR                     5
#define PIN_MIC                     A0
#define PIN_TOUCH                   1
#define PIN_PIXELS                  4
#define PIN_LED                     2
#define PIN_LED_CHIP                16
#define PIN_CAP                     14

#define BAUD_RATE                   115200

// Firebase Defines
#define FIREBASE_HOST "gemini-lamp.firebaseio.com"
#define FIREBASE_AUTH "y4041WZ3eWpUJeQaFJrIfKtaUc473SHUoQ9SpRjZ"

#define FIREBASE_TARGET             String("rithy")
#define FIREBASE_SELF               String("kevin")
#define TOUCH_TARGET                "/touch"
#define MOTION_TARGET               "/motion"

#define FIREBASE_POLL_INTERVAL      1000

// NeoPixel Initialization
#define NUM_PIXELS                  1
#define NP_WHITE                    255,255,255
#define NP_RED                      255,0,0
#define NP_GREEN                    0,255,0
#define NP_OFF                      0,0,0
#define NP_RGB_MAX                  255
#define NP_SETUP_COMPLETE_ITER      5
#define NP_SETUP_COMPLETE_DELAY     100
#define NP_TOUCH_RAMP_DELAY         5
#define NP_TOUCH_ON_DELAY           1000
#define NP_MOTION_RAMP_DELAY        1
#define NP_MOTION_ON_DELAY          200
#define NP_MOTION_ITER              3

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);

// Capacitive Sensor Initialization
#define CAP_ADDR                    0x5A

bool cap_connected = 0;

// Adafruit_MPR121 cap = Adafruit_MPR121();
CapacitiveSensor cap = CapacitiveSensor(2, 14);


// State Variables
bool touch_detected = 0;
bool touch_animate = 0;
bool motion_detected = 0;
bool motion_animate = 0;

unsigned long previous_time = 0;
unsigned long current_time = 0;

void set_pixels_chain_colour(int r, int g, int b);
void touch_animation();
void motion_animation();
void firebase_state_machine();
void sensor_state_machine();

void motion_interrupt(){
  motion_detected = 1;
  motion_animate = 1;
}

void touch_interrupt(){
  touch_detected = 1;
  touch_animate = 1;
}

void setup() {

  pixels.begin();

  Serial.begin(BAUD_RATE);

  set_pixels_chain_colour(NP_RED);
  pixels.show();

  // connect to wifi.
  Serial.println("Starting WiFi Manager");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("WiFi connected.");
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to Firebase");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase Connected!");

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_CAP, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), motion_interrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_CAP), touch_interrupt, FALLING);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED_CHIP, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_LED_CHIP, HIGH);

  for (int i = 0; i < NP_SETUP_COMPLETE_ITER; i++){
    for (int i = 0; i < NP_RGB_MAX; i++){
      set_pixels_chain_colour(0, i, 0);
      pixels.show();
      delay(2);
    }
    delay(NP_SETUP_COMPLETE_DELAY);
    for (int i = NP_RGB_MAX; i >= 0; i--){
      set_pixels_chain_colour(0, i, 0);
      pixels.show();
      delay(2);
    }
    delay(NP_SETUP_COMPLETE_DELAY);
  }

  Serial.println("Setup Complete");

}


void loop() {

  firebase_state_machine(); //Firebase Actions Occur Every FIREBASE_POLL_INTERVAL (Default: 1 Second)
  sensor_state_machine();

  Serial.println(digitalRead(PIN_CAP));

  delay(10);

}


void firebase_state_machine(){

  current_time = millis();

  if (current_time - previous_time > FIREBASE_POLL_INTERVAL){

    bool touch_received = Firebase.getBool(FIREBASE_TARGET + TOUCH_TARGET);
    if (Firebase.failed()) {
        Serial.print("Getting /touch failed:");
        Serial.println(Firebase.error());
        return;
    }
    if (touch_received){
      Firebase.setBool(FIREBASE_TARGET + TOUCH_TARGET, false);
      touch_animation();

      if (Firebase.failed()) {
          Serial.print("Setting /touch failed:");
          Serial.println(Firebase.error());
          return;
      }
    }

    bool motion_received = Firebase.getBool(FIREBASE_TARGET + MOTION_TARGET);
    if (Firebase.failed()) {
        Serial.print("Getting /motion failed:");
        Serial.println(Firebase.error());
        return;
    }

    if (motion_received){
      Firebase.setBool(FIREBASE_TARGET + MOTION_TARGET, false);
      motion_animation();

      if (Firebase.failed()) {
        Serial.print("Setting /motion failed:");
        Serial.println(Firebase.error());
        return;
      }
    }

    if (touch_detected){
      Firebase.setBool(FIREBASE_SELF + TOUCH_TARGET, true);
      touch_detected = 0;
    }

    if (motion_detected){
      Firebase.setBool(FIREBASE_SELF + MOTION_TARGET, true);
      motion_detected = 0;
    }

  }
}

void sensor_state_machine(){

  if (motion_animate){

    Serial.println("Motion Animation.");

    motion_animation();

    motion_animate = 0;

  }

  if (touch_animate){

    Serial.println("Touch Animation.");

    touch_animation();

    touch_animate = 0;

  }

}

void touch_animation(){

  for (int i = 0; i < NP_RGB_MAX; i++){
    set_pixels_chain_colour(i,i,i);
    pixels.show();
    delay(NP_TOUCH_RAMP_DELAY);
    yield();
  }
  delay(NP_TOUCH_ON_DELAY);
  for (int i = NP_RGB_MAX; i >= 0; i--){
    set_pixels_chain_colour(i,i,i);
    pixels.show();
    delay(NP_TOUCH_RAMP_DELAY);
    yield();
  }
}

void motion_animation(){

  for (int i = 0; i < NP_MOTION_ITER; i++){
    for (int j = 0; j < NP_RGB_MAX; j++){
      set_pixels_chain_colour(0,j/2,j);
      pixels.show();
      delay(NP_MOTION_RAMP_DELAY);
      yield();
    }
    for (int j = 0; j < NP_RGB_MAX; j++){
      set_pixels_chain_colour(j, 127+j/2, NP_RGB_MAX);
      pixels.show();
      delay(NP_MOTION_RAMP_DELAY);
      yield();
    }
    delay(NP_MOTION_ON_DELAY);
    for (int j = NP_RGB_MAX; j >= 0; j--){
      set_pixels_chain_colour(j,j,j);
      pixels.show();
      delay(NP_MOTION_RAMP_DELAY);
      yield();
    }
  }
}

void set_pixels_chain_colour(int r, int g, int b){
  for (int i = 0; i < NUM_PIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(r,g,b));
  }
}
