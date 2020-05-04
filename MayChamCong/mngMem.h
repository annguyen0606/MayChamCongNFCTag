#include <FS.h>
/*
bool clearData(const char* filename)
{
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename, "w");
  if(!f)
  {
    f.close();
    return false;  
  }  
  else
  {
    f.close();
    return true;  
  }
}

bool saveData(const char* filename, const char* content, uint16_t len)
{
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename,"a");
  if(!f)
  {
    f.close();
    return false;  
  }  
  else
  {
    f.write((const uint8_t*)content, len);
    f.close();
    return true;  
  }
}

String readData(const char* filename)
{
  SPIFFS.begin();
  File f = SPIFFS.open(String("/") + filename, "r");
  String ret = f.readString();
  f.close();
  return ret;  
}*/
/*=========================Clear array======================*/
void clear_arr(uint8_t *data, uint8_t len)
{
  uint8_t count = 0;
  for(count = 0; count < len; count++)
  {
    data[count] = 0;  
  }  
}
/*==========Functions do with File System=============*/
void writeDataFFS(String file_name, String textDt)
{
  SPIFFS.begin();
  File writeData = SPIFFS.open(file_name,"a");
  writeData.print(textDt);     
  writeData.close();    
}
/*===================================================*/
String OpenFFS(String file_name)
{
  SPIFFS.begin();
  File  readData2 = SPIFFS.open(file_name,"r");
  return readData2.readString();
  readData2.close();     
}
/*===================================================*/
void clearFFS(String file_name)
{
  SPIFFS.begin();
  File clearData = SPIFFS.open(file_name,"w");
  clearData.close();    
}
/*==========================Clear Memory===================*/
/*
void clear_mem()
{
        int c = 0;
        for(c = 0; c < 29; c++)
        {
          clearFFS(filename[c]);  
        }      
}*/
