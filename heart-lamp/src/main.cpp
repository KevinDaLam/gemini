#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#include <CapacitiveSensor.h>


// Pin Initialization
#define PIN_PIR                     5
#define PIN_TOUCH                   1
#define PIN_PIXELS                  14
#define PIN_BASE_PIXELS             10
#define PIN_LED                     2
#define PIN_LED_CHIP                16
#define PIN_CAP                     4

#define BAUD_RATE                   115200

// WiFiManager Object
WiFiManager wifiManager;

#define ACCESS_POINT_NAME           "HeartLamp"

// Firebase Defines
#define FIREBASE_HOST "gemini-lamp.firebaseio.com"
#define FIREBASE_AUTH "y4041WZ3eWpUJeQaFJrIfKtaUc473SHUoQ9SpRjZ"

#define FIREBASE_TARGET             String("kevin")
#define FIREBASE_SELF               String("rithy")
#define TOUCH_TARGET                "/touch"
#define MOTION_TARGET               "/motion"
#define HEART_ACTIVE_TARGET         "/heart/active"
#define HEART_RED_TARGET            "/heart/r"
#define HEART_GREEN_TARGET          "/heart/g"
#define HEART_BLUE_TARGET           "/heart/b"
#define BASE_ACTIVE_TARGET          "/base/active"
#define BASE_RED_TARGET             "/base/r"
#define BASE_GREEN_TARGET           "/base/g"
#define BASE_BLUE_TARGET            "/base/b"

#define FIREBASE_POLL_INTERVAL      5000

// NeoPixel Initialization
#define NUM_PIXELS                  3
#define NUM_BASE_PIXELS             2
#define NP_WHITE                    100,100,100
#define NP_RED                      255,0,0
#define NP_FUSCHIA                  100,0,100
#define NP_GREEN                    0,100,0
#define NP_OFF                      0,0,0
#define NP_RGB_MAX                  255
#define NP_SETUP_COMPLETE_ITER      2
#define NP_SETUP_COMPLETE_DELAY     25
#define NP_TOUCH_RAMP_DELAY         3
#define NP_TOUCH_ON_DELAY           500
#define NP_MOTION_RAMP_DELAY        0
#define NP_MOTION_ON_DELAY          50
#define NP_MOTION_ITER              1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel base_pixels = Adafruit_NeoPixel(NUM_BASE_PIXELS, PIN_BASE_PIXELS, NEO_GRB + NEO_KHZ800);

// State Variables
bool touch_detected = 0;
bool touch_animate = 0;
bool motion_detected = 0;
bool motion_animate = 0;
int fail_count = 0;

unsigned long previous_time = 0;
unsigned long current_time = 0;
int heart_colour[3] = {0,0,0};
int base_colour[3] = {0,0,0};

void set_pixels_chain_colour(Adafruit_NeoPixel *t_pixels, int r, int g, int b);
void touch_animation();
void motion_animation(Adafruit_NeoPixel *t_pixels);
void firebase_state_machine();
void sensor_state_machine();
bool check_firebase_fail(String path, String type);

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
  base_pixels.begin();

  Serial.begin(BAUD_RATE);

  set_pixels_chain_colour(&pixels, NP_WHITE);

  // connect to wifi.
  Serial.println("Starting WiFi Manager");
  wifiManager.autoConnect(ACCESS_POINT_NAME);
  Serial.println("WiFi connected.");
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connecting to Firebase");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase Connected!");

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_CAP, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), motion_interrupt, RISING);
  // attachInterrupt(digitalPinToInterrupt(PIN_CAP), touch_interrupt, FALLING);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED_CHIP, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_LED_CHIP, HIGH);

  for (int i = 0; i < NP_SETUP_COMPLETE_ITER; i++){
    for (int i = 0; i < NP_RGB_MAX; i++){
      set_pixels_chain_colour(&pixels, 0, i, 0);
      pixels.show();
      delay(1);
    }
    delay(NP_SETUP_COMPLETE_DELAY);
    for (int i = NP_RGB_MAX; i >= 0; i--){
      set_pixels_chain_colour(&pixels, 0, i, 0);
      delay(1);
    }
  }

  Serial.println("Setup Complete");

}

void loop() {

  firebase_state_machine(); //Firebase Actions Occur Every FIREBASE_POLL_INTERVAL (Default: 1 Second)
  sensor_state_machine();

}


void firebase_state_machine(){

  current_time = millis();

  if (fail_count >= 10){
    ESP.reset();
    // Serial.println("Re-begin Firebase");
    // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  }

  if (current_time - previous_time > FIREBASE_POLL_INTERVAL){

    previous_time = current_time;

    bool touch_received = Firebase.getBool(FIREBASE_TARGET + TOUCH_TARGET);
    if (check_firebase_fail(FIREBASE_TARGET + TOUCH_TARGET, "Getting")){

    }
    else if (touch_received){
      Firebase.setBool(FIREBASE_TARGET + TOUCH_TARGET, false);
      touch_animation();

      check_firebase_fail(FIREBASE_TARGET + TOUCH_TARGET, "Getting");
    }

    bool motion_received = Firebase.getBool(FIREBASE_TARGET + MOTION_TARGET);
    if (check_firebase_fail(FIREBASE_TARGET + MOTION_TARGET, "Getting")){}
    else if (motion_received){
      Firebase.setBool(FIREBASE_TARGET + MOTION_TARGET, false);
      motion_animation(&pixels);

      set_pixels_chain_colour(&pixels, heart_colour[0], heart_colour[1], heart_colour[2]);

      check_firebase_fail(FIREBASE_TARGET + TOUCH_TARGET, "Setting");
    }

    if (touch_detected){
      Firebase.setBool(FIREBASE_SELF + TOUCH_TARGET, true);
      check_firebase_fail(FIREBASE_SELF + TOUCH_TARGET, "Setting");
    }
    else{
      bool heart_active = Firebase.getBool(FIREBASE_SELF + HEART_ACTIVE_TARGET);
      if (check_firebase_fail(FIREBASE_SELF + HEART_ACTIVE_TARGET, "Getting")){}
      else if (heart_active){
        heart_colour[0] = Firebase.getInt(FIREBASE_SELF + HEART_RED_TARGET);
        heart_colour[1] = Firebase.getInt(FIREBASE_SELF + HEART_GREEN_TARGET);
        heart_colour[2] = Firebase.getInt(FIREBASE_SELF + HEART_BLUE_TARGET);
        set_pixels_chain_colour(&pixels, heart_colour[0], heart_colour[1], heart_colour[2]);
      }
      else{
        set_pixels_chain_colour(&pixels, NP_OFF);
      }

      bool base_active = Firebase.getBool(FIREBASE_SELF + BASE_ACTIVE_TARGET);
      if (check_firebase_fail(FIREBASE_SELF + BASE_ACTIVE_TARGET, "Getting")){}
      else if (base_active){
        base_colour[0] = Firebase.getInt(FIREBASE_SELF + BASE_RED_TARGET);
        base_colour[1] = Firebase.getInt(FIREBASE_SELF + BASE_GREEN_TARGET);
        base_colour[2] = Firebase.getInt(FIREBASE_SELF + BASE_BLUE_TARGET);
        set_pixels_chain_colour(&base_pixels, base_colour[0], base_colour[1], base_colour[2]);
      }
      else{
        set_pixels_chain_colour(&base_pixels, NP_OFF);
      }
    }

    if (motion_detected){
      Firebase.setBool(FIREBASE_SELF + MOTION_TARGET, true);
      motion_detected = 0;
      check_firebase_fail(FIREBASE_SELF + MOTION_TARGET, "Setting");
    }

  }
}

void sensor_state_machine(){

  touch_detected = !digitalRead(PIN_CAP);

  if (motion_animate){

    Serial.println("Motion Animation.");

    motion_animation(&base_pixels);

    set_pixels_chain_colour(&base_pixels, base_colour[0], base_colour[1], base_colour[2]);

    motion_animate = 0;

  }

  if(touch_detected){

    Serial.println("Touch Animation.");

    touch_animation();

    set_pixels_chain_colour(&pixels, heart_colour[0], heart_colour[1], heart_colour[2]);
  }

}

void touch_animation(){

  for (int i = 0; i < NP_RGB_MAX; i++){
    set_pixels_chain_colour(&pixels, i,i,i);
    delay(NP_TOUCH_RAMP_DELAY);
    yield();
  }
  delay(NP_TOUCH_ON_DELAY);
  for (int i = NP_RGB_MAX; i >= 0; i--){
    set_pixels_chain_colour(&pixels, i,i,i);
    delay(NP_TOUCH_RAMP_DELAY);
    yield();
  }
}

void motion_animation(Adafruit_NeoPixel *t_pixels){

  for (int i = 0; i < NP_MOTION_ITER; i++){
    for (int j = 0; j < NP_RGB_MAX; j++){
      set_pixels_chain_colour(t_pixels, 0,j/2,j);
      delay(NP_MOTION_RAMP_DELAY);
    }
    for (int j = 0; j < NP_RGB_MAX; j++){
      set_pixels_chain_colour(t_pixels, j, (NP_RGB_MAX/2)+j/2, NP_RGB_MAX);
      delay(NP_MOTION_RAMP_DELAY);
    }
    delay(NP_MOTION_ON_DELAY);
    for (int j = NP_RGB_MAX; j >= 0; j--){
      set_pixels_chain_colour(t_pixels, j,j,j);
      delay(NP_MOTION_RAMP_DELAY);
    }
  }
}

void set_pixels_chain_colour(Adafruit_NeoPixel *t_pixels, int r, int g, int b){
  for (int i = 0; i < t_pixels->numPixels(); i++){
    t_pixels->setPixelColor(i, pixels.Color(r,g,b));
  }
  t_pixels->show();
}

bool check_firebase_fail(String path, String type){
  if(Firebase.failed()){
    Serial.print(type);
    Serial.print(" ");
    Serial.print(path);
    Serial.print(" failed: ");
    Serial.println(Firebase.error());

    fail_count++;
    return true;
  }
  return false;
}
