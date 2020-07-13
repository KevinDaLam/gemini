// #include <Arduino.h>
//
// #include <Wire.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <PubSubClient.h>
// #include <aREST.h>
//
// #include <Adafruit_NeoPixel.h>
// #include <DNSServer.h>
// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>
//
// #include "Adafruit_MPR121.h"
//
// // Pin Initialization
// #define PIN_PIR                     5
// #define PIN_MIC                     A0
// #define PIN_TOUCH                   1
// #define PIN_PIXELS                  4
// #define PIN_LED                     2
//
// #define BAUD_RATE                   115200
//
// //Client Initialization
// WiFiClient espClient;
// // PubSubClient client(espClient);
// // HTTPClient http;
//
// String host = "cloud.arest.io";
// String motion_url = "/gemini/motion";
//
// // aREST Initialization
// #define LISTEN_PORT                 80
//
// aREST rest = aREST();
//
// char* device_id = "gemini";
//
// // WiFiServer server(LISTEN_PORT);
//
// // NeoPixel Initialization
// #define NUM_PIXELS                  1
// #define NP_WHITE                    255,255,255
// #define NP_RED                      255,0,0
// #define NP_OFF                      0,0,0
// #define NP_TOUCH_RAMP_DELAY         10
// #define NP_TOUCH_ON_DELAY           1500
// #define NP_MOTION_RAMP_DELAY        1
// #define NP_MOTION_ON_DELAY          200
// #define NP_MOTION_ITER              3
//
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);
//
// // Capacitive Sensor Initialization
// #define CAP_ADDR                    0x5A
//
// bool cap_connected = 0;
//
// Adafruit_MPR121 cap = Adafruit_MPR121();
//
// // State Variables
// bool touch_detected = 0;
// bool motion_detected = 0;
//
//
// void callback(char* topic, byte* payload, unsigned int length);
// void set_pixels_chain_colour(int r, int g, int b);
// int touch_animation(String command);
// int motion_animation(String command);
// void idle_state_machine();
//
// void motion_interrupt(){
//   motion_detected = 1;
// }
//
// void touch_interrupt(){
//   touch_detected = 1;
// }
//
// void setup() {
//   // client.setCallback(callback);
//
//   Serial.begin(BAUD_RATE);
//
//   Serial.println("Initializing RESTful Environment");
//
//   rest.function("touch", touch_animation);
//   rest.function("motion", motion_animation);
//
//   rest.set_id(device_id);
//   rest.set_name("heart-lamp-rithy");
//
//   Serial.println("Starting WiFi Manager");
//   WiFiManager wifiManager;
//   wifiManager.autoConnect("AutoConnectAP");
//   Serial.println("WiFi connected.");
//
//   // Start the server
//   // server.begin();
//   // Serial.println("Server started");
//
//   // Print the IP address
//   Serial.println(WiFi.localIP());
//
//   //Peripherals Setup
//   pixels.begin();
//
//   // if (cap.begin(0x5A)) {
//   //   Serial.println("MPR121 Connected.");
//   //   cap_connected = 1;
//   // }
//   // else{
//   //   Serial.println("MPR121 Not Found.");
//   // }
//
//   pinMode(PIN_PIR, INPUT);
//   attachInterrupt(digitalPinToInterrupt(PIN_PIR), motion_interrupt, RISING);
//
//   pinMode(PIN_LED, OUTPUT);
//   digitalWrite(PIN_LED, 0);
//
//   Serial.println("Setup Complete");
//
//   // char* out_topic = rest.get_topic();
//
// }
//
// void loop() {
//
//   if(motion_detected){
//     Serial.println("Motion Detected.");
//     String empty_command = "";
//     // motion_animation(empty_command);
//     motion_detected = 0;
//
//     HTTPClient http;
//
//     http.begin("url")
//
//     http.GET()
//
//     reads
//
//
//     exit
//
//
//     espClient.println("GET /gemini/touch HTTP/1.1");
//     espClient.println("Host: cloud.arest.io");
//     espClient.println();
//
//     long interval = 2000;
// unsigned long currentMillis = millis(), previousMillis = millis();
//
// while(!espClient.available()){
//
//   if( (currentMillis - previousMillis) > interval ){
//
//     Serial.println("Timeout");
//     espClient.stop();
//     return;
//   }
//   currentMillis = millis();
// }
//
// while (espClient.connected())
// {
//   if ( espClient.available() )
//   {
//     char str=espClient.read();
//    Serial.println(str);
//   }
// }
//
//   }
//   if(touch_detected){
//     Serial.println("Touch Detected.");
//     String empty_command = "";
//     touch_animation(empty_command);
//     touch_detected = 0;
//   }
//
//   // rest.handle(client);
//
// }
//
// void set_pixels_chain_colour(int r, int g, int b){
//   for (int i = 0; i < NUM_PIXELS; i++){
//     pixels.setPixelColor(i, pixels.Color(r,g,b));
//   }
// }
//
// int touch_animation(String command){
//
//   for (int i = 0; i < 255; i++){
//     set_pixels_chain_colour(i,i,i);
//     pixels.show();
//     delay(NP_TOUCH_RAMP_DELAY);
//     yield();
//   }
//   delay(NP_TOUCH_ON_DELAY);
//   for (int i = 255; i >= 0; i--){
//     set_pixels_chain_colour(i,i,i);
//     pixels.show();
//     delay(NP_TOUCH_RAMP_DELAY);
//     yield();
//   }
//
//   return 1;
// }
//
// int motion_animation(String command){
//
//   for (int i = 0; i < NP_MOTION_ITER; i++){
//     for (int j = 0; j < 255; j++){
//       set_pixels_chain_colour(0,j/2,j);
//       pixels.show();
//       delay(NP_MOTION_RAMP_DELAY);
//       yield();
//     }
//     for (int j = 0; j < 255; j++){
//       set_pixels_chain_colour(j, 127+j/2, 255);
//       pixels.show();
//       delay(NP_MOTION_RAMP_DELAY);
//       yield();
//     }
//     delay(NP_MOTION_ON_DELAY);
//     for (int j = 255; j >= 0; j--){
//       set_pixels_chain_colour(j,j,j);
//       pixels.show();
//       delay(NP_MOTION_RAMP_DELAY);
//       yield();
//     }
//   }
//
//   return 1;
// }
//
// void callback(char* topic, byte* payload, unsigned int length) {
//
//   // rest.handle_callback(client, topic, payload, length);
//
// }
