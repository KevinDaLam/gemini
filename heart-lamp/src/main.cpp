#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <Adafruit_NeoPixel.h>

#define PIR_PIN 0
#define MIC_PIN A0
#define TOUCH_PIN 1

#define NEOPIXEL_PIN 4
#define NUM_PIXELS 1

#define LED_PIN 2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    // put your setup code here, to run once:
    pixels.begin();
    Serial.begin(9600);

}

void loop() {
    // put your main code here, to run repeatedly:
    pixels.setPixelColor(0, pixels.Color(255,255,255));
    pixels.show();
    delay(500);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.show();
    delay(500);
}
