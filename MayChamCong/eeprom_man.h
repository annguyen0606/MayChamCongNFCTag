#include <EEPROM.h>

 void EEPROMWrite(int address, int value) 
  {
    
    byte four = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);      // using bitwise operator here for shifting through bits
    byte two = ((value >> 16) & 0xFF);
    byte one = ((value >> 24) & 0xFF);
    
    EEPROM.put(address, four);
    EEPROM.put(address + 1, three);
    EEPROM.put(address + 2, two);
    EEPROM.put(address + 3, one);
    EEPROM.commit();
}
int EEPROMRead(int address) {
    byte four = EEPROM.read(address);       // least LSB
    byte three = EEPROM.read(address + 1);       
    byte two = EEPROM.read(address + 2);
    byte one = EEPROM.read(address + 3);     // most MSB

    return (
          ((four << 0) & 0xFF) +        // retreiving least LSB
          ((three << 8) & 0xFFFF) + 
          ((two << 16) & 0xFFFFFF) + 
          ((one << 24) & 0xFFFFFFFF)    // retreiving most MSB
      );
}
int EEPROMRead_2(int address) {
    EEPROM.begin(32);
    byte four = EEPROM.read(address);       // least LSB
    byte three = EEPROM.read(address + 1);       
    byte two = EEPROM.read(address + 2);
    byte one = EEPROM.read(address + 3);     // most MSB

    return (
          ((four << 0) & 0xFF) +        // retreiving least LSB
          ((three << 8) & 0xFFFF) + 
          ((two << 16) & 0xFFFFFF) + 
          ((one << 24) & 0xFFFFFFFF)    // retreiving most MSB
      );
}
