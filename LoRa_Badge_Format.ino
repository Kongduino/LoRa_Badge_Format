#include "Format.h"
#include "Commands.h"

void setup() {
  Serial.begin(115200);
  time_t timeout = millis();
  while (!Serial) {
    // on nRF52840, Serial is not available right away.
    // make the MCU wait a little
    if ((millis() - timeout) < 5000) {
      delay(100);
    } else {
      break;
    }
  }
  uint8_t x = 5;
  while (x > 0) {
    Serial.printf(" % d, ", x--);
    delay(500);
  } // Just for show
  Serial.println("0!");
  Serial.println("LoRa Badge Formatter");
  Serial.println("------------------------------------------------------");
  cmdCount = sizeof(cmds) / sizeof(myCommand);
  handleHelp("");
  if (i2ceeprom.begin(EEPROM_ADDR)) {
    // you can put a different I2C address here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  } else {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Testing size!");
  uint8_t test;
  uint32_t max_addr;
  for (max_addr = 0; max_addr < 0xFFFF; max_addr += 256) {
    if (i2ceeprom.read(max_addr) != test) continue;
    if (!i2ceeprom.write(max_addr, (byte)~test)) {
      Serial.print("Failed to write address 0x");
      Serial.println(max_addr, HEX);
    }
    // read address 0x0 again
    uint8_t val0 = i2ceeprom.read(0);
    // re-write the old value
    if (!i2ceeprom.write(max_addr, test)) {
      Serial.print("Failed to re-write address 0x");
      Serial.println(max_addr, HEX);
    }
    // check if addr 0 was changed
    if (val0 == (byte)~test) {
      Serial.println("Found max address");
      break;
    }
  }
  Serial.printf("This EEPROM can store %d bytes\n", max_addr);
}

void loop() {
  if (Serial.available()) {
    char str1[256];
    str1[0] = '>';
    str1[1] = ' ';
    uint8_t ix = 2;
    while (Serial.available()) {
      char c = Serial.read();
      if (c > 31) str1[ix++] = c;
    }
    str1[ix] = 0;
    Serial.println(str1);
    handleCommands(str1 + 2);
  }
}
