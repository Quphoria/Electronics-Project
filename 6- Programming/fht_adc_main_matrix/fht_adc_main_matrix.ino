/*
fht_adc.pde
guest openmusiclabs.com 8.18.12
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.  there is a pure data patch for
visualizing the data.
*/

//'maniacbug' Mighty 1284p 16MHz using Optiboot

//Put 3.3k resistor between A0 and GND

#define LOG_OUT 1 // use the log output function
#define FHT_N 128 // set to 256 point fht

int averlist[FHT_N];
byte pattern = 0;

#define AV_LIST_SIZE 32

byte avlist[AV_LIST_SIZE];
byte avlistplace = 0;
  
#include <FHT.h> // include the library

#include <FastLED.h>
#define DATA_PIN 6
#define DISPLAY_WIDTH 32
#define DISPLAY_HEIGHT 16
#define NUM_LEDS DISPLAY_WIDTH*DISPLAY_HEIGHT

CRGB leds[NUM_LEDS];
byte image[NUM_LEDS*3];

byte brightness_scale;

void setup() {
  Serial.begin(115200); // use the serial port
  Serial.println("SetupLoadingLEDS");
  brightness_scale = 255;
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++){
    image[i*3] = 100;
    image[i*3+1] = 0;
    image[i*3+2] = 255;
  }
  FastLED.setBrightness(brightness_scale);
  
  
  Serial.println("SetupFinishing");
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  delay(5000);
  
}

void setBrightness(byte LED_brightness) {
  brightness_scale = constrain(LED_brightness, 0 ,255);
  FastLED.setBrightness(brightness_scale);
}

int PosToIndex(int POS_x, int POS_y){
  int PIndex = 0;
  //bottom left led 0,0

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
void boolLED(int LED_x, int LED_y, bool LED_State){
  //Serial.println("BoolLED");
  int PIX_Index = PosToIndex(LED_x,LED_y);
  if (LED_State) {
    leds[PIX_Index].r = image[PIX_Index*3];
    leds[PIX_Index].g = image[PIX_Index*3+1];
    leds[PIX_Index].b = image[PIX_Index*3+2];
  }
  else {
    leds[PIX_Index].r = 0;
    leds[PIX_Index].g = 0;
    leds[PIX_Index].b = 0;
  }
}

void drawBar(int X_Bar, int Bar_Val) {
  //Serial.println("DrawBAR");
  Bar_Val = constrain(Bar_Val, 0, 16);
  for (byte ycoord = 0; ycoord < 16; ycoord++){
    boolLED(X_Bar,ycoord,ycoord < Bar_Val);
  }
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    float aver = 0;
    for (int i = 0 ; i < FHT_N ; i ++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      averlist[i] = k;
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into even bins
    }
    for (int i = 0 ; i < FHT_N ; i ++) {
      int k = averlist[i];
      k = constrain(k * 1.2, 0, 255);
      aver = max(constrain(map(k, 0, 1023, 0, 255),0,255),aver);
    }
    //Serial.println("FHTrecorded");
    aver = min(aver,255);
    avlist[avlistplace] = aver;
    avlistplace++;
    if (avlistplace > AV_LIST_SIZE - 1) {
      avlistplace = 0;
    }

    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    
    sei();
    byte outputvars[FHT_N/4];
    //Serial.println("FHTfinished");
    float gaver = 0;
    for (byte j = 0; j < AV_LIST_SIZE; j++) {
      gaver += avlist[j];
    }
    gaver = gaver / (AV_LIST_SIZE / 4);
    gaver = gaver / FHT_N;

    float z = fht_log_out[1] * gaver;
    z = min(z,255);
    outputvars[0] = map(z, 0, 255, 0, 16);
    for (byte i = 2; i < FHT_N/2; i+=2) {
      Serial.print("A");
      float y = fht_log_out[i] + fht_log_out[i+1];
      float z = y * gaver;
      //float z = y;
      z = min(z,255);
      outputvars[i/2] = map(z, 0, 255, 0, 16);
    }
    Serial.println("B");
    for (byte xcoord = 0; xcoord < 32; xcoord++) {
      drawBar(xcoord, outputvars[xcoord]);
    }
    Serial.println("C");
    FastLED.show();
    Serial.println("D");
  }
}
