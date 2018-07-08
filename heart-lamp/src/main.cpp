#include <Arduino.h>

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>

#include <Adafruit_NeoPixel.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "Adafruit_MPR121.h"

// Pin Initialization
#define PIN_PIR                     0
#define PIN_MIC                     A0
#define PIN_TOUCH                   1
#define PIN_PIXELS                  4
#define PIN_LED                     2

#define BAUD_RATE                   9600

//Client Initialization
WiFiClient espClient;
PubSubClient client(espClient);

// aREST Initialization
#define LISTEN_PORT                 80

aREST rest = aREST(client);

char* device_id = "gemini";

// WiFiServer server(LISTEN_PORT);

// NeoPixel Initialization
#define NUM_PIXELS                  1
#define NP_WHITE                    255,255,255
#define NP_RED                      255,0,0
#define NP_OFF                      0,0,0

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800);

// Capacitive Sensor Initialization
#define CAP_ADDR                    0x5A

bool cap_connected = 0;

Adafruit_MPR121 cap = Adafruit_MPR121();

// REST Variables
bool received_touch = 0;
bool received_motion = 0;

bool send_touch = 0;
bool send_motion = 0;


void callback(char* topic, byte* payload, unsigned int length);
int touch_animation(String command);
int motion_animation(String command);
void idle_state_machine();

void setup() {

  Serial.begin(BAUD_RATE);
  client.setCallback(callback);

  Serial.println("Initializing RESTful Environment");

  rest.function("touch", touch_animation);
  rest.function("motion", motion_animation);

  rest.set_id(device_id);
  rest.set_name("heart-lamp-rithy");

  Serial.println("Starting WiFi Manager");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("WiFi connected.");

  // Start the server
  // server.begin();
  // Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  //Peripherals Setup
  pixels.begin();

  // if (cap.begin(0x5A)) {
  //   Serial.println("Cap Connected.");
  //   cap_connected = 1;
  // }

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED, OUTPUT);

  Serial.println("Setup Complete");

  char* out_topic = rest.get_topic();

}

void loop() {

    // Handle REST calls
    // WiFiClient client = server.available();
    // if (!client) {
    //   Serial.println("Error with WiFiClient, WiFi-less Functionality");
    //   return;
    // }
    // while(!client.available()){
    //   delay(1);
    // }
    rest.handle(client);

}

int touch_animation(String command){
  for (int i = 0; i < NUM_PIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(NP_WHITE));
  }
  pixels.show();
  delay(1000);
  for (int i = 0; i < NUM_PIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(NP_OFF));
  }
  pixels.show();

  return 1;
}

int motion_animation(String command){
  return 1;
}

void idle_state_machine(){
  Serial.println("IDLE");
}

void callback(char* topic, byte* payload, unsigned int length) {

  rest.handle_callback(client, topic, payload, length);

}
