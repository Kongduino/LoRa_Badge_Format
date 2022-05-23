void handleHelp(char *);
void evalCmd(char *, char *);
void handleCommands(char *);
void handleName(char *);
void handleRead(char *);
void handleHelp(char *);
void readEEPROM();
void initEEPROM(char *);

int cmdCount = 0;
struct myCommand {
  void (*ptr)(char *); // Function pointer
  char name[12];
  char help[48];
};

myCommand cmds[] = {
  {handleHelp, "help", "Shows this help."},
  {handleName, "name", "Sets the name and initializes the EEPROM."},
  {handleRead, "read", "Reads the information on the EEPROM."},
};

void handleHelp(char *param) {
  Serial.printf("Available commands: %d\n", cmdCount);
  char msg[128];
  for (int i = 0; i < cmdCount; i++) {
    sprintf(msg, " . /%s: %s", cmds[i].name, cmds[i].help);
    Serial.println(msg);
  }
}

void handleName(char *param) {
  char userName[NAMElen + 1] = {0};
  int i = sscanf(param, "/name %s", userName);
  if (i == -1) {
    // no parameters
    readEEPROM();
    showData();
  } else {
    // Set name
    uint8_t ln = strlen(param);
    if (ln == 0) {
      Serial.println("Empty name! Bailing!");
      return;
    }
    if (ln > NAMElen) ln = NAMElen;
    Serial.printf("Initialize EEPROM with name: %s\n", userName);
    initEEPROM(userName);
  }
}

void handleRead(char *param) {
  readEEPROM();
  showData();
}

void evalCmd(char *str, char *fullString) {
  char strq[12];
  for (int i = 0; i < cmdCount; i++) {
    sprintf(strq, "%s?", cmds[i].name);
    if (strcmp(str, cmds[i].name) == 0 || strcmp(strq, str) == 0) {
      cmds[i].ptr(fullString);
      return;
    }
  }
}

void handleCommands(char *str1) {
  char kwd[32];
  int i = sscanf(str1, "/%s", kwd);
  if (i > 0) {
    evalCmd(kwd, str1);
  } else handleHelp("");
}
