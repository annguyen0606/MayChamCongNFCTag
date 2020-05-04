#include <Wire.h>
#include "SSD1306Wire.h"


SSD1306Wire display(0x3c, SDA, SCL);


void drawProgressBarDemo(int &currentLoad,int &counter, int &goalValue) {
  //int counter = 1;
  for(int i = currentLoad; i < goalValue; i++){
    display.clear();
    int progress = (counter / 5) % 100;
    // draw the progress bar
    display.drawProgressBar(0, 32, 120, 10, progress);
  
    // draw the percentage as String
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(progress) + "%");
    display.display();  
    counter++;
  }
  
}

void drawImageDemo(int x0, int y0, int Logo_Width, int Logo_Heigh, const uint8_t *img) {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.clear();
    
    //display.drawFastImage(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display.drawXbm(x0, y0, Logo_Width, Logo_Heigh, img);
    display.display();
}

void Oled_print(int x, int y, String str){
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(x, y, str);
  display.display();
}

void Init_Oled(){
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

void Oled_Draw_Rec(void) {
  display.clear();
  display.drawRect(0, 0, display.getWidth(), display.getHeight());
  display.display();
}

int getBarsSignal(long rssi){
  // 5. High quality: 90% ~= -55db
  // 4. Good quality: 75% ~= -65db
  // 3. Medium quality: 50% ~= -75db
  // 2. Low quality: 30% ~= -85db
  // 1. Unusable quality: 8% ~= -96db
  // 0. No signal
  int bars;
  
  if (rssi > -55) { 
    bars = 5;
  } else if (rssi < -55 & rssi > -65) {
    bars = 4;
  } else if (rssi < -65 & rssi > -75) {
    bars = 3;
  } else if (rssi < -75 & rssi > -85) {
    bars = 2;
  } else if (rssi < -85 & rssi > -96) {
    bars = 1;
  } else {
    bars = 0;
  }
  return bars;
}

void Oled_Show_Wifi_Signal(int bars){
  display.clear();
  for (int b=0; b <= bars; b++) {
    display.fillRect(110 + (b*3),13 - (b*3),2,b*3); 
  }  
  display.display();
}

void Main_Screen(const uint8_t *img, int bars, String ngay, String thoiGian){
    display.clear();
    //display.drawFastImage(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display.drawXbm(0, 14, 128, 34, img);  
    for (int b=0; b <= bars; b++) {
      display.fillRect(110 + (b*3),14 - (b*3),2,b*3); 
    }      
    display.setFont(ArialMT_Plain_10);
    display.drawString(70, 3, ngay);
    display.drawString(70, 45, thoiGian);
    display.display();
}
