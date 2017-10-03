#include <FastLED.h>
#define DATA_PIN 6
#define DISPLAY_WIDTH 32
#define DISPLAY_HEIGHT 16
#define NUM_LEDS DISPLAY_WIDTH*DISPLAY_HEIGHT

CRGB leds[NUM_LEDS];
//CRGB image[NUM_LEDS];

float brightness_scale;
int step_delay;

int PosToIndex(int POS_x, int POS_y){
  int PIndex = 0;
  //PIndex = ((POS_y - 1) * DISPLAY_WIDTH) + (POS_x - 1); //bottom left led 1,1
  //PIndex = (POS_y * DISPLAY_WIDTH) + POS_x; //bottom left led 0,0
  
//  //HS-BL Bottom left led 0,0 =IF(ISEVEN(Y),31-X,X)+32*Y
//  if (POS_y % 2 == 0) {
//    PIndex = 31 - POS_x;
//  }
//  else {
//    PIndex = POS_x;
//  }
//  PIndex += 32 * POS_y;

  //HS-BL Bottom left led 0,0 =IF(ISEVEN(X),IF(Y>7,23-Y,7-Y),Y)+8*(31-X)+IF(Y>7,248,0)
  bool isupperpanel = POS_y > 7;
  if (POS_x % 2 == 0){
    if (isupperpanel) {
      PIndex = 23 - POS_y;
    }
    else {
      PIndex = 7 - POS_y;
    }
  }
  else {
    PIndex = POS_y;
  }
  PIndex += 8*(31-POS_x);
  if (isupperpanel) {
    PIndex += 248;
  }
  return PIndex;
}

void setLED(int LED_x, int LED_y, float LED_r, float LED_g, float LED_b){
  leds[PosToIndex(LED_x,LED_y)].setRGB( float(LED_r) * (brightness_scale/100), float(LED_g) * (brightness_scale/100), float(LED_b) * (brightness_scale/100));
}

void setup() {
  brightness_scale = 255;
  step_delay = 1;
  // put your setup code here, to run once:
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
  for (int yi = 0; yi < 16; yi++ ){
    for (int xi = 0; xi < 32; xi++ ){
      setLED(xi,yi,255,0,0);
      FastLED.show();
      delay(step_delay);
    }
  }
  for (int yi = 0; yi < 16; yi++ ){
    for (int xi = 0; xi < 32; xi++ ){
      setLED(xi,yi,0,255,0);
      FastLED.show();
      delay(step_delay);
    }
  }
  for (int yi = 0; yi < 16; yi++ ){
    for (int xi = 0; xi < 32; xi++ ){
      setLED(xi,yi,0,0,255);
      FastLED.show();
      delay(step_delay);
    }
  }
  for (int yi = 0; yi < 16; yi++ ){
    for (int xi = 0; xi < 32; xi++ ){
      setLED(xi,yi,255,255,255);
      FastLED.show();
      delay(step_delay);
    }
  }
  for (int yi = 0; yi < 16; yi++ ){
    for (int xi = 0; xi < 32; xi++ ){
      setLED(xi,yi,0,0,0);
    }
  }
  FastLED.show();
}
