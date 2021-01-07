/*
  Hi stranger, this is main code file for this project
  I'm not a 100% programmer, but i can make electronics work,
  so i will be grateful if you add any features

  it is fully opensource project, so anyone can build stuff based on this code 

  have a great time reading this badly written working code (^_^) 

*/

#include <Arduino.h>      // why not...
#include <Wire.h>
#include <SPI.h>
// i have to make all modules work, so i will use some libraries to make life easier
//1) Display.      im using 0.96 oled from china, it is not standart at dimentions, bt i like how it looks in final designs :)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // Adafruit librari works 50/50, it depends on display driver (yes, they can hava same names, bt diffrent drivers)

//2) RGB Led panel.       LEDs 2812 (8-bit panel) 
#include <FastLED.h>

//3) NRF24L01+ 
#include <nRF24L01.h>
#include <RF24.h>

//4)APDC9960 usefull sensor
#include "Adafruit_APDS9960.h"

//5) LoRa radio sx1278
#include <LoRa.h>


//6) MPU6050 gyro + acsel
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

//7) MP3 module
#include <DFPlayer_Mini_Mp3.h>

// first switches connection
int8_t first_sw[8] = { A14, A13, A12, A11, A10, A9, A8, A7 };

// second switches connection
int8_t second_sw[8] = { 38, 37, 36, 35, 34, 33, 32, A15 };

// buttons connection
int8_t buttons[4] = { A3, A1, A0, A2 };

#define LED1 10
#define LED2 11

#define JOY_X A6
#define JOY_Y A5

#define POT A4

#define LORA_D0 42
#define LORA_NSS 43
#define LORA_RST 44

#define NRF_CSN 40
#define NRF_CE 41

#define IR_LED 7
#define R_LED 4
#define G_LED 5
#define B_LED 6

#define WS_LED 45



#define LED_PIN     45
#define NUM_LEDS    8
#define BRIGHTNESS  20
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100 

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

RF24 radio(NRF_CE, NRF_CSN);

Adafruit_MPU6050 mpu;

Adafruit_SSD1306 display(128, 32, &Wire, -1);

Adafruit_APDS9960 apds;

volatile bool irqMPU;
volatile bool irqAPDC;

struct allData
{
  volatile boolean irqMPU;
  volatile boolean irqAPDC;

  bool stable;
  int8_t x_acs;
  int8_t y_acs;
  int8_t z_acs;

  uint8_t mode;
  uint8_t channel;

  uint16_t button;
  
  uint16_t potData;
  uint16_t joyX;
  uint16_t joyY;

  uint8_t led1Mode;
  uint8_t led2Mode;

  uint8_t redLedMode;
  uint8_t blueLedMode;
  uint8_t greenLedMode;

  uint8_t wsLedMode;

  

}mainData;

struct radioData
{
  bool stable;
  int8_t x_acs;
  int8_t y_acs;
  int8_t z_acs;

  uint8_t mode;
  uint8_t channel;

  uint16_t button;
  
  uint16_t potData;
  uint16_t joyX;
  uint16_t joyY;

} telemetriData;

void readMode();
void readCh();
void readAcs();
void readJoy();
void readPot();
void readButtons();
void sendNRF();
void sendBL();
void sendLoRa();   // will reliase it soon
void displayInfo();
void FillLEDsFromPaletteColors( uint8_t colorIndex);
void ChangePalettePeriodically();
void SetupTotallyRandomPalette();
void SetupBlackAndWhiteStripedPalette();
void SetupPurpleAndGreenPalette();
// at all it is possible to create up to 256 diffrent modes,
// but if you need more - connect mode counter with channel counter (maybe partly)
void n1Mode();
void n2Mode();
void n3Mode();
void n4Mode();
void n5Mode();
void n6Mode();
void n7Mode();
void n8Mode();
void n9Mode();
void n10Mode();
void n11Mode();
void n12Mode();



void acsel()
{
  mainData.irqMPU=true;
}
void gesture()
{
  mainData.irqAPDC=true;

}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};




void setup() 
{
  for(int i=0;i<8;i++)
    pinMode(first_sw[i], INPUT_PULLUP);
  for(int i=0;i<8;i++)
    pinMode(second_sw[i], INPUT_PULLUP);
  for(int i=0;i<4;i++)
    pinMode(buttons[i], INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  analogWrite(LED1, 10);
  analogWrite(LED2, 100);
  
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  pinMode(POT, INPUT_PULLUP);
  
  pinMode(LORA_D0, OUTPUT);
  pinMode(LORA_NSS, OUTPUT);
  pinMode(LORA_RST, OUTPUT);
  
  pinMode(NRF_CSN, OUTPUT);
  pinMode(NRF_CE, OUTPUT);
  
  pinMode(IR_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  
  pinMode(WS_LED, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(9600);
  
    if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
  }
  mp3_set_serial(Serial2);
  mp3_set_volume(25);
  mp3_play (1);
  if (!mpu.begin())
    Serial.println("Sensor init failed");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  display.clearDisplay();
  
  display.display();
  if(!apds.begin())
    Serial.println("failed to initialize device! Please check your wiring.");
  apds.enableProximity(true);
  apds.enableGesture(true);
  if(!radio.begin())
      Serial.println("nrf err");
  radio.setChannel(100);                               
  radio.setDataRate     (RF24_1MBPS);                   
  radio.setPALevel      (RF24_PA_HIGH);                 
  radio.openWritingPipe (0x1234567899LL);               
  radio.setAutoAck(false);

  attachInterrupt(0, acsel, RISING);
  attachInterrupt(1, gesture, RISING);

  Serial1.begin(9600);         // bluetooth module connected to Serial1 
  delay(2000);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
 // mp3_stop ();
  
  
}

void loop()
{
 readMode();
 readCh();
 readAcs();
 readJoy();
 readPot();
 readButtons();

 
 
 displayInfo();

  switch (mainData.mode)
  {
  case 0:
    n1Mode();
    break;
  case 2:
    n2Mode();
    break;
  case 3:
    n3Mode();
    break;
  case 4:
    n4Mode();
    break;
  
  }
  ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}


void readAcs()      // reading acseleration values from sensor directly to main struct
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  mainData.x_acs = a.acceleration.x;
  mainData.y_acs = a.acceleration.y;
  mainData.z_acs = a.acceleration.z;
  return;
}

void readJoy()     // i am filering analog values for better perfomance 
{
  mainData.joyX = (analogRead(JOY_X)+analogRead(JOY_X)+analogRead(JOY_X)+analogRead(JOY_X))/4;
  mainData.joyY = (analogRead(JOY_Y)+analogRead(JOY_Y)+analogRead(JOY_Y)+analogRead(JOY_Y))/4;
  return;
}

void readPot()
{
  mainData.potData = analogRead(POT);
  return;
}

void readButtons()   // buttons : 1) 1; 2)0; 3)1; 4)1;   and mainData.button == 1011 
{
  mainData.button = !digitalRead(A1)*1000+!digitalRead(A2)*100+!digitalRead(A3)*10+!digitalRead(A0);
  return;
}

void sendNRF()
{
  // i am writing telemetri struct only when sending data
  // in this case i can track how relevant telemetri data is

  telemetriData.stable = mainData.stable;
  telemetriData.x_acs = mainData.x_acs;
  telemetriData.y_acs = mainData.y_acs;
  telemetriData.z_acs = mainData.z_acs;

  telemetriData.mode = mainData.mode;
  telemetriData.channel = mainData.channel;

  telemetriData.button = mainData.button;
  
  telemetriData.potData = mainData.potData;
  telemetriData.joyX = mainData.joyX;
  telemetriData.joyY = mainData.joyY;
  radio.write(&telemetriData, sizeof(telemetriData));
}

void sendBL(String inp)
{
  Serial1.print(inp);
  return;
}


// void sendLoRa();

void displayInfo()
{
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);       
  display.setCursor(0, 0);            
  display.print(mainData.channel);    
  display.print("  ");          
  display.print(mainData.mode);
  display.print("  ");
  display.println(mainData.z_acs);      
  display.print(mainData.button);
  display.print("  ");
  display.print(mainData.joyX);
  display.print("  ");
  display.print(mainData.joyX);
  display.print("  ");
  display.println(mainData.potData);
  display.display();
}


void readMode()
{
  bitWrite(mainData.mode, 0, (!digitalRead(A14)));
  bitWrite(mainData.mode, 1, (!digitalRead(A13)));
  bitWrite(mainData.mode, 2, (!digitalRead(A12)));
  bitWrite(mainData.mode, 3, (!digitalRead(A11)));
  bitWrite(mainData.mode, 4, (!digitalRead(A10)));
  bitWrite(mainData.mode, 5, (!digitalRead(A9)));
  bitWrite(mainData.mode, 6, (!digitalRead(A8)));
  bitWrite(mainData.mode, 7, (!digitalRead(A7)));
  return;
}

void readCh()
{
  bitWrite(mainData.channel, 0, !(digitalRead(second_sw[0])));
  bitWrite(mainData.channel, 1, !(digitalRead(second_sw[1])));
  bitWrite(mainData.channel, 2, !(digitalRead(second_sw[2])));
  bitWrite(mainData.channel, 3, !(digitalRead(second_sw[3])));

  bitWrite(mainData.channel, 4, !(digitalRead(second_sw[4])));
  bitWrite(mainData.channel, 5, !(digitalRead(second_sw[5])));
  bitWrite(mainData.channel, 6, !(digitalRead(second_sw[6])));
  bitWrite(mainData.channel, 7, !(digitalRead(second_sw[7])));
  return;
}


void n1Mode()
{
  sendNRF();
  digitalWrite(LED1, !digitalRead(LED1)); // just blink to understand, that it is working
}
void n2Mode()
{

}
void n3Mode()
{

}
void n4Mode()
{

}
void n5Mode()
{

}
void n6Mode()
{

}
void n7Mode()
{

}
void n8Mode()
{

}
void n9Mode()
{

}
void n10Mode()
{

}
void n11Mode()
{

}
void n12Mode()
{

}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}