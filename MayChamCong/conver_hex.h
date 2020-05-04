int8_t encode4byte(uint16_t number, uint8_t * bcd)
{
  if(number < 10000U) {
    uint8_t d[4], temp;
    d[0] = number       & 0xF;
    d[1] = (number>>4)  & 0xF;
    d[2] = (number>>8)  & 0xF;
    d[3] = (number>>12) & 0xF;
    d[0] = 6*(d[3] + d[2] + d[1]) + d[0];
    temp = d[0] / 10;
    d[0] = d[0] % 10;
    d[1] = temp + 9*d[3] + 5*d[2] + d[1];
    temp = d[1] / 10;
    d[1] = d[1] % 10;
    d[2] = temp + 2*d[2];
    temp = d[2] / 10;
    d[2] = d[2] % 10;
    d[3] = temp + 4*d[3];
    temp = d[3] / 10;
    d[3] = d[3] % 10;
    bcd[0]= d[3];
    bcd[1]= d[2];
    bcd[2]= d[1];
    bcd[3]= d[0];
    return 0;
  } else {
    return 1;
  }
}
int8_t encode8byte_big_edian(uint8_t *number,uint8_t *bcd)
{
  if (number[1] < 0x23 && number[0]==0) {
    uint32_t temp;
    uint32_t d[4];
    int8_t i;
    d[3] = (number[0] * 256 + number[1]) & 0xFFFF;
    d[2] = (number[2] * 256 + number[3]) & 0xFFFF;
    d[1] = (number[4] * 256 + number[5]) & 0xFFFF;
    d[0] = (number[6] * 256 + number[7]) & 0xFFFF;
    d[0] = 656 * d[3] + 7296 * d[2] + 5536 * d[1] + d[0];
    temp = d[0] / 10000;
    d[0] = d[0] % 10000;
    d[1] = temp + 7671 * d[3] + 9496 * d[2] + 6 * d[1];
    temp = d[1] / 10000;
    d[1] = d[1] % 10000;
    d[2] = temp + 4749 * d[3] + 42 * d[2];
    temp = d[2] / 10000;
    d[2] = d[2] % 10000;
    d[3] = temp + 281 * d[3];
    temp = d[3] / 10000;
    d[3] = d[3] % 10000;
    
    for (i =3; i>=0; i--) {
      encode4byte(d[i],(bcd+12-4*i));
    }
    
    return 0;
  } else {
    return 1;
  }
}
uint16_t crcx25 (const uint8_t* data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;
    if (len)
        do
            {
                crc ^= *data++;
                
                for (i = 0; i < 8; i++)
                    {
                        if (crc & 1)
                            crc = (crc >> 1) ^ 0x8408;
                        else
                            crc >>= 1;
                    }
            }
        while (--len);
        
    return (~crc);
}
