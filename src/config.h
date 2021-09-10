#include <TFT_eSPI.h>
#include <Button2.h>
#include "LinkedList.h"


#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL              4   // Display backlight control pin
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            0
#define BUTTON_2            35

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
  
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);


char buff[512];
int vref = 1100;
int btnClick2 = false;
int btnClick1 = false;

const char* ntpServer = "0.pool.ntp.org";
const long  gmtOffset_sec = -6*60*60;
const int   daylightOffset_sec = 3600;

struct sample{
  struct tm timeinfo;
  int value;
};

LinkedList<sample> temp1buffer;

void setupButtons()
{

  btn2.setPressedHandler([](Button2 & b) {
    btnClick2 = true;
    Serial.println("btn 2 pressed");
  });

  btn2.setReleasedHandler([](Button2 &b){
    btnClick2 = false;
    Serial.println("button 2 was released");
  });

  btn1.setPressedHandler([](Button2 & b) {
    btnClick1 = true;
    Serial.println("btn 1 pressed");
  });

  btn1.setReleasedHandler([](Button2 &b){
    btnClick1 = false;
    Serial.println("button1 was released");
  });

  

  
};

void setupTFT()
{
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    
    if (TFT_BL > 0) {                           // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
        pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }

  tft.setTextWrap(true ,true);
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);

}
