#include "Adafruit_nRFCrypto.h"
#include <Wire.h>
#include "Adafruit_EEPROM_I2C.h" // Click here to get the library: http://librarymanager/All#Adafruit_EEPROM_I2C

#define EEPROM_ADDR 0x50 // the default address
#define MAXADD 262143 // max address in byte
Adafruit_EEPROM_I2C i2ceeprom;

char MAGIC[4] = {0xde, 0xca, 0xfb, 0xad};
#define UUIDlen 2
char myUUID[UUIDlen];
char myPlainTextUUID[UUIDlen * 2 + 2];
#define NAMElen 16
char myName[NAMElen + 1];
#define fullSetLen (NAMElen + UUIDlen + 4)
char fullSet[fullSetLen]; // MAGIC = 4 bytes

void hexDump(char *, uint16_t);
void readEEPROM();
void showData();
void initEEPROM(char *);

void showData() {
  memset(myPlainTextUUID, 0, UUIDlen);
  for (uint8_t ix = 0; ix < UUIDlen; ix++) sprintf(myPlainTextUUID + ix * 2, "%02x", fullSet[ix + 4]);
  Serial.printf(" . Name: %s\n . UUID: %s\n", myName, myPlainTextUUID);
}

void initEEPROM(char *userName) {
  Serial.println("* initEEPROM");
  uint16_t addr = 0x0000;
  memset(fullSet, 0x00, fullSetLen);
  memcpy(fullSet, MAGIC, 4);
  nRFCrypto.begin();
  nRFCrypto.Random.generate((uint8_t*)fullSet + 4, UUIDlen);
  nRFCrypto.end();
  uint8_t ln = strlen(userName);
  if (ln == 0) {
    Serial.println("Empty name! Bailing!");
    return;
  }
  if (ln > NAMElen) ln = NAMElen;
  memset(myName, 0, NAMElen + 1);
  memcpy(myName, userName, ln);
  memcpy(fullSet + 4 + UUIDlen, myName, NAMElen);
  hexDump(fullSet, fullSetLen);
  Serial.print(" . Writing... ");
  bool rslt = true;
  rslt = i2ceeprom.write(addr, (uint8_t*)fullSet, fullSetLen);
  //  for (uint8_t ix = 0; ix < fullSetLen; ix++) {
  //    rslt = i2ceeprom.write(addr, fullSet[ix]);
  //    Serial.printf("  - Writing %02x at address %04x: %s\n", fullSet[ix], addr, rslt ? "[ok]" : "[x]");
  //    //delay(100);
  //    addr += 1;
  //  }
  Serial.printf("done: %s\n", rslt ? "[ok]" : "[x]");
  Serial.println(" . Read back...");
  readEEPROM();
  Serial.println(" . Data:");
  showData();
}

void readEEPROM() {
  // Serial.println("* readEEPROM");
  memset(fullSet, 0, fullSetLen);
  uint16_t addr = 0x0000;
  bool rslt = i2ceeprom.read(addr, (uint8_t*)fullSet, fullSetLen);
  // for (uint8_t ix = 0; ix < fullSetLen; ix++) {
  //   fullSet[ix] = i2ceeprom.read(addr);
  //   Serial.printf("  - Read %02x at address %04x\n", fullSet[ix], addr);
  //   addr += 1;
  // }
  Serial.printf(" . Read %d bytes\n", fullSetLen);
  memset(myName, 0, NAMElen + 1);
  memcpy(myName, fullSet + 4 + UUIDlen, NAMElen);
  bool correct = true;
  for (uint8_t ix = 0; ix < 4; ix++) {
    if (fullSet[ix] != MAGIC[ix]) {
      correct = false;
      break;
    }
  }
  if (!correct) {
    Serial.println(" . Incorrect format! Please init EEPROM with \"name xxxxx\"!");
    return;
  } else Serial.println(" . MAGIC checks out.");
  // Serial.println(" . Full Set:");
  // hexDump(fullSet, fullSetLen);
}

void hexDump(char *buf, uint16_t len) {
  char alphabet[17] = "0123456789abcdef";
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\n"));
  for (uint16_t i = 0; i < len; i += 16) {
    if (i % 128 == 0)
      Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
    char s[] = "|                                                | |                |\n";
    uint8_t ix = 1, iy = 52;
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    uint8_t index = i / 16;
    if (i < 256) Serial.write(' ');
    Serial.print(index, HEX); Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
}
