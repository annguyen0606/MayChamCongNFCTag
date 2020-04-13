#define CR95HF_COMMAND_SEND                         0x00
#define CR95HF_COMMAND_RESET                        0x01
#define CR95HF_COMMAND_RECEIVE                      0x02
#define CR95HF_COMMAND_POLLING                      0x03

/* CR95HF command definition */
#define IDN                                         0x01
#define PROTOCOL_SELECT                             0x02
#define POLL_FIELD                                  0x03
#define SEND_RECEIVE                                0x04
#define LISTEN                                      0x05
#define SEND                                        0x06
#define IDLE                                        0x07
#define READ_REGISTER                               0x08
#define WRITE_REGISTER                              0x09
#define BAUD_RATE                                   0x0A
#define SUB_FREQ_RES                                0x0B
#define AC_FILTER                                   0x0D
#define TEST_MODE                                   0x0E
#define SLEEP_MODE                                  0x0F
#define ECHO                                        0x55
// customs command

#define CR95HF_15693ANTICOL                         0xA0
#define CR95HF_INVENTORY16SLOTS                     0xA1
#define CR95HF_ISWAKEUP                             0xA2
#define CR95HF_GOTOTAGDETECTINGSTATE                0xA3
#define CR95HF_CALIBRATETHETAGDETECTION             0xA4

#define CR95HF_READCUSTOMTAGMEMORY                  0xB0
#define CR95HF_READMCUBUFFER                        0xB1
#define CR95HF_GETHARDWAREVERSION                   0xB2
#define CR95HF_DOWNLOADSTM32MEM                     0xB5
#define CR95HF_READWHOLEMEMORY                      0xB6
#define CR95HF_TRANSPARENT                          0xB7
#define CR95HF_PULSE_POOLINGREADING                 0xB8
#define CR95HF_PULSE_SPINSS                         0xB9
#define CR95HF_GETINTERFACEPINSTATE                 0xBA
#define CR95HF_SETUSBDISCONNECTSTATE                0xBB
#define CR95HF_GETMCUVERSION                        0xBC
#define CR95HF_RESETSEQUENCE                        0xBD
#define CR95HF_PULSE_IRQIN                          0xBE
#define BLOCK_SIZE     4U

const int SS_Pin = 16;   // Slave Select pin
const int IRQPin = 2;   // Sends wake-up pulse
/*=========================================================*/
bool Send_CMD (uint8_t* cmd,  uint8_t cmd_len)
{
    uint8_t counter;
    digitalWrite (SS_Pin, LOW);
    SPI.transfer (CR95HF_COMMAND_SEND);
    for (counter = 0; counter < cmd_len; counter++)
        {
            SPI.transfer (cmd[counter]);
        }
        
    digitalWrite (SS_Pin, HIGH);
    delay (10);
    return true;
}
/*==============================================================================*/
bool Polling_Data (void)
{
    uint8_t counter = 0;
    digitalWrite (SS_Pin, LOW);
    uint8_t temp = 0;
    while (temp != 8 && counter < 20)
        {
            temp = SPI.transfer (CR95HF_COMMAND_POLLING); // Write CR95HF_COMMAND_POLLING
            temp = temp & 0x08;  // Bit 3 is set
            counter ++;
        }
        
    digitalWrite (SS_Pin, HIGH);
    delay(10);
    if (counter >= 20)
    {
        return false;
    }
    else
    {
        return true;
    }
}
/*=====================================================*/

/*===============================================================================*/
bool Read_Data (uint8_t* data)
{
    uint8_t counter;
    digitalWrite (SS_Pin, LOW);
    SPI.transfer (CR95HF_COMMAND_RECEIVE);            // SPI control byte for read
    data[0] = SPI.transfer (0); // Response code
    data[1] = SPI.transfer (0); // Length of data
    if ( (data[0] == 0x80) && (data[1] > 0))
        {
            for (counter = 0; counter < data[1]; counter ++)
                {
                    data[2 + counter] = SPI.transfer (0);
                }
        } 
    digitalWrite (SS_Pin, HIGH);
    return true;
    delay (10);
}


bool setprotocol_tagtype5()
{
    uint8_t set_cmd[] = {PROTOCOL_SELECT, 0x02, 0x01, 0x0D};
    uint8_t response[] = {0xFF, 0xFF};
    Send_CMD (set_cmd, sizeof (set_cmd));
    Polling_Data();
    Read_Data (response);
    delay(10);
    if ( (response[0] == 0x00) && (response[1] == 0x00))
        return true;
    else
        return false;
}

/*Set protocol tag type 2 - 3 - 4*/
bool Protocol_Selection_type2()
{
    uint8_t set_cmd[] = {PROTOCOL_SELECT, 0x02, 0x02, 0x00};
    uint8_t response[] = {0xFF, 0xFF};
    Send_CMD (set_cmd, sizeof (set_cmd));
    Polling_Data();
    Read_Data (response);
    delay(10);
    if ( (response[0] == 0x00) && (response[1] == 0x00)){
        return true;
    }
    else
        return false;
}

/*Get ID Tag type 2-3-4*/
bool Type2_Request_A(byte *UID, int *Type)
{
  byte response[16];
  int i = 0;
  byte message1[4] = {0x04,0x02,0x26,0x07};
  byte message2[5] = {0x04,0x03,0x93,0x20,0x08};
  byte message3[10] = {0x04,0x08,0x93,0x70,0x00,0x00,0x00,0x00,0x00,0x28};
  byte message4[5] = {0x04,0x03,0x95,0x20,0x08};
  byte message5[10] = {0x04,0x08,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0xA8};
  Type[0] = 0;
  Send_CMD (message1, sizeof (message1));
  if(Polling_Data()){
    Read_Data (response);
    if((response[2] == 0x44))Type[0]=2;
    if((response[2] == 0x42))Type[0]=4;
    if((response[2] == 0x04))Type[0]=7;  
    if((response[2] == 0x00))
    {
      Type[0]=1;
      {
        Send_CMD (message5, sizeof (message5));
        Polling_Data();
        Read_Data (response);
        if((response[0] == 0x80)&&(response[1]==0x0B))
        {
          UID[0] = response[4];
          UID[1] = response[5];
          UID[2] = response[6];
          UID[3] = response[7];
          return true;
        }
      }
    }  
  }

    if((response[0] == 0x80)&&(response[1]==0x05))
    {
      Send_CMD (message2, sizeof (message2));
      Polling_Data();
      Read_Data (response);
     if((response[0] == 0x80)&&(response[1]==0x08))
      {
        if(Type[0]==7)
        {
          UID[0] = response[2];
          UID[1] = response[3];
          UID[2] = response[4];
          UID[3] = response[5];
          return true;
        }
        UID[0] = response[3];
        UID[1] = response[4];
        UID[2] = response[5];
        
        message3[4] = response[2];
        message3[5] = response[3];
        message3[6] = response[4];
        message3[7] = response[5];
        message3[8] = response[6];
        Send_CMD (message3, sizeof (message3));
        Polling_Data();
        Read_Data (response);
        if((response[0] == 0x80)&&(response[1]==0x06))
        {
          Send_CMD (message4, sizeof (message4));
          Polling_Data();
          Read_Data (response);
          if((response[0] == 0x80)&&(response[1]==0x08))
          {
            UID[3] = response[2];
            UID[4] = response[3];
            UID[5] = response[4];
            UID[6] = response[5];
            message3[4] = response[2];
            message3[5] = response[3];
            message3[6] = response[4];
            message3[7] = response[5];
            message3[8] = response[6];
            Send_CMD (message3, sizeof (message3));
            return true;
          }
        }
      }
     }
     return false;
}
/*==========================Function get UID Tag*/
bool getID_Tag (uint8_t* tagid)
{
    uint8_t get_cmd[] = {SEND_RECEIVE, 0x03, 0x26, 0x01, 0x00};
    uint8_t response[0x1F] = {0xFF, 0xFF};
    uint16_t crc = 0;
    uint8_t counter;
    Send_CMD (get_cmd, sizeof (get_cmd));
    if(Polling_Data())
    {
      if(Read_Data (response))
      {
        if (response[1] == 0x0D)
            {
                crc = crcx25 (response + 2, response[1] - 3);
                
                if (crc == response[response[1]]*0x100 + response[response[1]-1])
                    {
                        for (counter = 0; counter < response[1] - 5; counter++)
                            {
                                tagid[counter] = response[response[1] - 2 - counter];
                            }
                            return true;
                    }
                    else
                    {
                      return false;
                    } 
            }
        else
        return false;
        delay(10);
      }
    }
}
/*=========================Function Wake Up ST95HF======================*/
void WakeUp_TinZ (void)
{
    pinMode (IRQPin, OUTPUT);
    digitalWrite (IRQPin, HIGH); // Wake up pulse
    delay (150);
    digitalWrite (IRQPin, LOW);     // Pulse to put the
    delay (150);        // CR95HF Easy into SPI
    digitalWrite (IRQPin, HIGH);    // Mode
    delay (100);
}
/*=====================Function Ping CR95HF============================*/
bool CR95HF_ping (void)
{
    uint8_t ping_cmd[] = {ECHO};
    uint8_t response[] = {0x00, 0x00};
    Send_CMD (ping_cmd, sizeof (ping_cmd));
    Polling_Data();
    Read_Data (response);
    if (response[0] == ECHO)
    {
        return true ;
    }
    else
    {
        return false;
    }
}
/*=======================Function read single Block=====================*/
bool read_single_block(uint8_t block, uint8_t data[BLOCK_SIZE])
{
    uint8_t command[] = {0x04, 0x03, 0x02, 0x20, 0x00};
    command[4] = block; 
    uint8_t response[0x1F] = {0xFF, 0xFF};
    uint16_t crc = 0;
    uint8_t counter;
    bool check = false;
    Send_CMD (command, 5);
    Polling_Data();
    Read_Data (response); 
    if (response[0] == 0x80)
    {
          if(response[1] > 3)
          {
            crc = crcx25 (response + 2, 5);
              if (crc == response[8]*0x100 + response[7])
                  {
                      for (counter = 3; counter < 7; counter++)
                          {
                              data[counter - 3] = response[counter];
                           
                          }
                          return true;
                  }
                  else
                  {
                    return false;
                  } 
          }
          else
          {
              return false;            
          }
     }
     else
     {
        return false; 
     }  
}

void PrintHex8(uint8_t *data, uint8_t length)
{
  char tmp[16];
  for(int i = 0; i < length; i++)
  {
    sprintf(tmp,"0x%2X",data[i]);
    Serial.print(tmp);
    Serial.print(" ");  
  }
}
