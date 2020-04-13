#include <SPI.h>
#include <Scheduler.h>
#include "FS.h"
#include "mngMem.h"
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "conver_hex.h"
#include "oled_0_96.h"
#include "eeprom_man.h"
#include "command_st95.h"
#include "ChuongThongBao.h"
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include "images.h"


ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
HTTPClient http;  
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP8266WebServer server(80);
/*Bien Thoi Gian*/
String timeStamp;
String dayStamp;
String currentDay = "";
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
int setHours = 0;
int setSec = 0;
int setMin = 0;
int setDay = 0;
int setDat = 0;
int setMon = 0;
int setYear = 0;
String Date = "";
String formattedDate = "";
long soLanDoc = 0;
long soThuTuGhi = 0;
long soThuTuFile = 0;
byte kiemtra;
bool checkRead = false;
/*********************************/
String ver = "Ver 1.1";
String macAdd = "";
String textUID = "";
String tempUID = "";
/* CR95HF HEADER command definition ---------------------------------------------- */
uint8_t TagID[8];
uint8_t UID[16];
int Type[1];
/*Bien kiem tra nut bam*/
int checkbutton = 0;
/*======================Dia chi de luu gia tri trong bo nho EEPROM========================*/
const int EEaddress = 0;
const int EEaddress_2 = 8;
const int EEaddress_3 = 12;
const int EEaddress_4 = 16;
/*Bien kiem tra tin hieu mang Wifi*/
long rssi = 0;
int bars = 0;
/*Variable Send API*/
String linkPingInternet = "http://testcodeesp8266.000webhostapp.com/"; //Link kiem tra xem co ket noi Internet hay khong

/************************************/
char * filename[] = {"SaveData.txt","SaveData2.txt","SaveData3.txt","SaveData4.txt","SaveData5.txt","SaveData6.txt","SaveData7.txt","SaveData8.txt","SaveData9.txt","SaveData10.txt","SaveData11.txt","SaveData12.txt","SaveData13.txt","SaveData14.txt","SaveData15.txt",
                     "SaveData16.txt","SaveData17.txt","SaveData18.txt","SaveData19.txt","SaveData20.txt","SaveData21.txt","SaveData22.txt","SaveData23.txt","SaveData24.txt","SaveData25.txt","SaveData26.txt","SaveData27.txt","SaveData28.txt","SaveData29.txt","SaveData30.txt","SaveData31.txt"};
/*================================================================================*/
/*Function de update code qua Mang Internet OTA*/
void UpdateCode()
{
  if ((WiFiMulti.run() == WL_CONNECTED)) 
  {
    Oled_print(60,20,"Updating");  
    ESP.getFreeSketchSpace();
    if(ESPhttpUpdate.update(client,"http://testcodeesp8266.000webhostapp.com/test3.bin") != HTTP_UPDATE_OK){
    }else{
      Oled_print(65,20,"Failure");
    }
  }
}
/*======================Ham gui len clound qua giao thuc HTTP/GET===========================*/
static String ping_wifi(String linkToSend)
{
  String payload = "";
  http.begin(client,linkToSend);  
  int httpCode = http.GET();                                                           
  if (httpCode == 200) { 
    payload = http.getString();  
  }else{
    payload = "Loi mang";
  }
  http.end();   
  return payload;
} 
/*===================Khoi tao module doc de no hoat dong=================*/
void Init_Module_Reader(){
    SPI.begin();
    SPI.setDataMode (SPI_MODE0);
    SPI.setBitOrder (MSBFIRST);
    SPI.setFrequency (4000000);
    pinMode (SS_Pin, OUTPUT);
    digitalWrite (SS_Pin, HIGH);     
    WakeUp_TinZ();
}
/*===================Function de doc ma UID cua Tag NFC========================== */
String ReadTagNFC(){
  uint8_t count;
  String IdTag = "";
  if(CR95HF_ping()){
    if (setprotocol_tagtype5()){
      if (getID_Tag(TagID) == true ){
        TagID[0] = 0x00;
        if (encode8byte_big_edian (TagID, UID) == 0){
          for (count = 0; count < 8 * 2; count++){
            UID[count] += 0x30;
            IdTag += char(UID[count]);
          }
        }
      }
    }
  }
  return IdTag;
}
/*===============Function kiem tra gia tri tra ve Sau khi gui len clound===============*/
int CheckDataResponse(String InputDataResponse){
  if(InputDataResponse.indexOf('1') > 0 && InputDataResponse.length() < 10){
    return 1;
  }else if(InputDataResponse.indexOf('0') > 0 && InputDataResponse.length() < 10){
    return 2;
  }
  else{
    return 3;
  }
}
/*Function nhan chuoi gom SSID, PASS va PORT tu dien thoai thong qua Bluetooth*/
String GetSsidPassWifi(){
  String textData;
  while (1) {
    if(Serial.available() > 0){
      char ch = Serial.read(); //doc 
      if(ch >= 33 && ch <= 126){
        textData += ch; 
        if(ch == ')'){
          break;
        }
      }
      /*Ket thuc cua chuoi la ky tu ')', neu thiet bi nhan duoc ky tu nay thi Break Loop*/
      delay(5); // Thoi gian tre de lay chuoi la 3s
    }
 }  
 return textData; // Tra lai chuoi nhan duoc
}
/*Function gui Du lieu len may tinh thong qua giao thuc UDP*/
static void UDP_send_data(String DataNeedSend, int gatePort)
{
  /*Gui goi tin di*/
  ntpUDP.beginPacket("255.255.255.255",gatePort);
  ntpUDP.write(DataNeedSend.c_str(), DataNeedSend.length());
  //ntpUDP.print(DataNeedSend);
  ntpUDP.endPacket();
}

/*Function Lay Wifi, phan tich SSID, Pass and PORT de truyen du lieu voi may tinh */
void GetWifi(){
  String ssid = "";
  String pass = "";
  int toData = 0;
  int fromData = 0;
  String textWifi;
  String textPORT = "";
  drawImageDemo(34,14,60,36,WiFi_Logo_bits);
  Serial.begin (9600);
  WiFi.disconnect();
  delay(1000);
  Serial.flush();
  /*Lay chuoi SSID, PASS and PORT from Dien thoai thong qua Bluetooth*/
  textWifi = GetSsidPassWifi();
  /*Phan tich chuoi vua nhan xong thanh SSID, PASS and PORT*/
  fromData = textWifi.indexOf(",",toData);
  pass = textWifi.substring(toData,fromData);
  toData = textWifi.indexOf(",",fromData + 1);
  ssid = textWifi.substring(fromData + 1,toData);     
  int a = textWifi.indexOf(")",toData + 1);
  textPORT = textWifi.substring(toData + 1,a);
  /*Start ket noi voi Wifi moi*/
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  /*Sau khi bat dau ket noi voi mot Wifi moi, Thong bao va Restart sau 2s*/
  Oled_print(65,20,"Restart");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  /*Reset thiet bi*/
  ESP.reset();
}

/*Function button*/
void FunctionButton()
{
  int checkbutton = 0;
  if(digitalRead(3) == HIGH){
  }else{
    do{
      checkbutton++;
      digitalWrite(15, LOW);
      delay(100);
      digitalWrite(15, HIGH);  
      delay(400);    
    }while(digitalRead(3) == LOW);        
  }
  if(checkbutton >= 1 && checkbutton < 2){
    checkbutton = 0;
    Oled_print(62,20,WiFi.localIP().toString());
    //Oled_print(65,20,ver);
    delay(1000);
  }  
  if(checkbutton >= 2 && checkbutton <= 5){
    checkbutton = 0;
    GetWifi();
    delay(1000);
  }
  if(checkbutton >= 6){
    checkbutton = 0;
    UpdateCode();
    delay(1000);
  }    
}
void LayThoiGian(){
  while(!timeClient.update()) 
  {
     yield();
     timeClient.forceUpdate();
  }
  setHours = timeClient.getHours();
  setSec = timeClient.getSeconds();
  setMin = timeClient.getMinutes();   
  setDay = timeClient.getDay();
   
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  Date = formattedDate.substring(0, splitT);
  int splitPhay = Date.indexOf("-");
  setYear = Date.substring(0, splitPhay).toInt();
  int splitPhay2 = Date.indexOf("-",splitPhay + 1); 
  setMon = Date.substring(splitPhay + 1, splitPhay2).toInt();
  setDat = Date.substring(splitPhay2 + 1).toInt();  
  
  int abcT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, abcT);
  currentDay = daysOfTheWeek[timeClient.getDay()];
  currentDay += String(" ");
  currentDay += dayStamp;                
  String ipadd = WiFi.localIP().toString();                             
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
 
}

class WebServer : public Task {
public:
    void setup() 
    {
      server.on("/ngay01", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[0]);
        server.send(200, "text/html",getDt);
      });              
      server.on("/ngay02", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[1]);
        server.send(200, "text/html",getDt);
      });        
      server.on("/ngay03", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[2]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay04", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[3]);
        server.send(200, "text/html",getDt);
      });        
      server.on("/ngay05", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[4]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay06", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[5]);
        server.send(200, "text/html",getDt);
      });        
      server.on("/ngay07", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[6]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay08", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[7]);
        server.send(200, "text/html",getDt);
      });        
      server.on("/ngay09", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[8]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay10", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[9]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay11", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[10]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay12", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[11]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay13", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[12]);
        server.send(200, "text/html",getDt);
      });                 
      server.on("/ngay14", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[13]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay15", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[14]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay16", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[15]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay17", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[16]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay18", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[17]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay19", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[18]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay20", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[19]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay21", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[20]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay22", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[21]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay23", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[22]);
        server.send(200, "text/html",getDt);
      });                  
      server.on("/ngay24", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[23]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay25", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[24]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay26", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[25]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay27", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[26]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay28", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[27]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay29", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[28]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay30", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[29]);
        server.send(200, "text/html",getDt);
      });            
      server.on("/ngay31", []{
        ESP.getFreeHeap();
        String getDt = OpenFFS(filename[30]);
        server.send(200, "text/html",getDt);
      });            
/*===================================================================================*/      
      server.on("/clear", []{ 
        clear_mem();
        server.send(200, "text/html","Clear success");
      });        
/*=========================================================*/    
      server.begin();
    }

    void loop() 
    {
      server.handleClient();             
    }   
} web_server;

void clear_mem()
{
        int c = 0;
        for(c = 0; c < 31; c++)
        {
          yield();
          clearFFS(filename[c]);  
        }      
}

/***********************************/
class ReadTag : public Task {
public:
void setup()
{}
void loop() {
  /*Lay trang thai cua nut bam*/
  //Neu bam nut thi Setup lai Wifi va PORT cho thiet bi
  FunctionButton();
  LayThoiGian();
  /*Lay tin hieu song Wifi de hien thi nen man hinh chinh*/
  rssi = WiFi.RSSI();
  bars = getBarsSignal(rssi);
  Main_Screen(Barca_Logo_bits,bars,currentDay,timeStamp);
  /*Doc gia tri Tag*/
  textUID = ReadTagNFC();
  //Co gia tri Tag thi xu ly
  if(textUID != "" && tempUID != textUID){
    tempUID = textUID;
    Oled_print(65,20,"Sending");
    String data = textUID +","+ dayStamp +","+ timeStamp +",";
    switch(setDat){
      case 1:
            writeDataFFS(filename[0],data);
            break;
      case 2:
            writeDataFFS(filename[1],data);
            break;
      case 3:
            writeDataFFS(filename[2],data);
            break;
      case 4:
            writeDataFFS(filename[3],data);
            break;
      case 5:
            writeDataFFS(filename[4],data);
            break;
      case 6:
            writeDataFFS(filename[5],data);
            break;
      case 7:
            writeDataFFS(filename[6],data);
            break;
      case 8:
            writeDataFFS(filename[7],data);
            break;                                          
      case 9:
            writeDataFFS(filename[8],data);
            break;
      case 10:
            writeDataFFS(filename[9],data);
            break;
      case 11:
            writeDataFFS(filename[10],data);
            break;
      case 12:
            writeDataFFS(filename[11],data);
            break;
      case 13:
            writeDataFFS(filename[12],data);
            break;
      case 14:
            writeDataFFS(filename[13],data);
            break;
      case 15:
            writeDataFFS(filename[14],data);
            break;
      case 16:
            writeDataFFS(filename[15],data);
            break;
      case 17:
            writeDataFFS(filename[16],data);
            break;
      case 18:
            writeDataFFS(filename[17],data);
            break;
      case 19:
            writeDataFFS(filename[18],data);
            break;
      case 20:
            writeDataFFS(filename[19],data);
            break;
      case 21:
            writeDataFFS(filename[20],data);
            break;
      case 22:
            writeDataFFS(filename[21],data);
            break;
      case 23:
            writeDataFFS(filename[22],data);
            break;
      case 24:
            writeDataFFS(filename[23],data);
            break;
      case 25:
            writeDataFFS(filename[24],data);
            break;
      case 26:
            writeDataFFS(filename[25],data);
            break;
      case 27:
            writeDataFFS(filename[26],data);
            break;
      case 28:
            writeDataFFS(filename[27],data);
            break;                                                                                                                        
      case 29:
            writeDataFFS(filename[28],data);
            break;
      case 30:
            writeDataFFS(filename[29],data);
            break;
      default:
            writeDataFFS(filename[30],data);
            break;
    }
    UDP_send_data(textUID,6688);
    ChuongBaoThanhCong();                                                                                                                                                                        
    textUID = "";
    delay(1400);
  }
  if(textUID != "" && tempUID == textUID){
    Oled_print(60,20,"Ban da cham the");
    delay(1000);
  }        
}
} read_tag;

void setup()
{
  /*Khoi tao bo nho EEPROM*/
  EEPROM.begin(32);
  /*Khoi tao Coi*/
  pinMode(15,OUTPUT);
  digitalWrite(15, HIGH); 
  /*Khoi tao giao tiep cong COM*/
  Serial.begin (9600);
  /*Khoi tao man hinh*/
  Init_Oled();
  /*Lay gia tri cua PORT*/
  //Neu gia tri < 0 hoac > 9999 thi Toi ham de Setup Wifi va PORT
  //Neu gia tri thoa man > 0 va <= 9999 thi se lay luon
  /*Chao hoi*/
  Oled_print(33,20,"Welcome");
  delay(1000);
  /*Man hinh ket noi Wifi*/
  drawImageDemo(34,14,60,36,WiFi_Logo_bits);
  delay(500);
  /*Ket noi Wifi*/
  //WiFi.mode(WIFI_STA);
  //WiFi.setAutoConnect(true);
  //Serial.println(WiFi.SSID());
  //Serial.println(WiFi.psk());
  WiFi.begin();
  /*Man hinh hien thi toc do ket noi*/
  int counter = 1;
  int goalValue = 0;
  int currentLoad = 0;
  while (WiFi.status() != WL_CONNECTED) {
    goalValue += 8;
    if(goalValue < 85){
      drawProgressBarDemo(currentLoad,counter, goalValue);
    }else{
      GetWifi();
      break;
    }
    delay(1000);
  }
  /*Open Port de truyen du lieu len may tinh*/
  ntpUDP.begin(6688);
  /*Lay dia chi Mac cua thiet bi*/
  macAdd = WiFi.macAddress();
  /*Khoi tao Module de doc Tag*/
  Init_Module_Reader();
  /*Hien thi tin hieu song Wifi*/
  rssi = WiFi.RSSI();
  bars = getBarsSignal(rssi);
  Main_Screen(Barca_Logo_bits,bars,currentDay,timeStamp);
  /*Khoi dong nut bam*/
  pinMode(3,INPUT);
  timeClient.setTimeOffset(3600 * 7);
  timeClient.begin();

  Scheduler.start(&web_server);
    
  Scheduler.start(&read_tag);
    
  Scheduler.begin();  
}
void loop()
{

}
