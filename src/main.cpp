#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define eepromSize 512

void eepromClear(uint16_t);
bool eepromWriteJSON(JsonObject&);
JsonObject& eepromReadJSON();

void setup()
{
  DynamicJsonBuffer buf(eepromSize);

  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);

  /* In order to Write JSON to EEPROM */ 
  JsonObject& root1 = buf.createObject();

  root1["username"] = "Saeed";
  root1["password"] = "Hosseini";

  eepromWriteJSON(root1);

  // eepromClear(eepromSize);

  /* In order to Read JSON from EEPROM */
  JsonObject& root2 = eepromReadJSON();

  if(root2.measureLength() > 2)
    root2.prettyPrintTo(Serial);
  else
    Serial.println("EEPROM is Empty!");
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

void eepromClear(uint16_t numberOfBytes)
{
  EEPROM.begin(numberOfBytes);

  for (uint16_t i = 0; i < numberOfBytes; i++)
    EEPROM.write(i, 0);

  EEPROM.end();
}

bool eepromWriteJSON(JsonObject& root)
{
  String jsonStr;
  uint8_t rem;
  uint16_t eepromIndex = 0;
  uint16_t num = root.measureLength();

  if(num >= eepromSize)
   return false;
  else
    root.printTo(jsonStr);

  EEPROM.begin(eepromSize);

  // Write Number of Bytes at the begining
  while (num != 0)
  {
    rem = num % 10;
    EEPROM.write(eepromIndex++, rem);
    num /= 10;
  } 
  
  // Writing JSON right after Number of Bytes
  for(uint16_t index = eepromIndex; index  < root.measureLength() + eepromIndex; index++)
    EEPROM.write(index, jsonStr[index - eepromIndex]);

  EEPROM.end();
  return true;
}

JsonObject& eepromReadJSON()
{
  uint8_t lengthNo[5], lengthNoIndex = 0;
  uint16_t length = 0;
  char jsonStr[eepromSize];

  EEPROM.begin(eepromSize);

  if(EEPROM.read(0) != '\0') // Check if EEPROM has any data
  {
    while(((char)EEPROM.read(lengthNoIndex)) != '{') // Separate the Number of Bytes from the JSON
    {
      lengthNo[lengthNoIndex] = EEPROM.read(lengthNoIndex);
      lengthNoIndex++;
    }
  }

  int index; // Making this global so it is possible to end the string (in the second loop) with \0

  for(index = lengthNoIndex - 1; index >= 0; index--) // Calculating NO of Bytes
    length = lengthNo[index] + length * 10; 

  for(index = lengthNoIndex; index < length + lengthNoIndex; index++) // Extracting the JSON to jsonStr
    jsonStr[index - lengthNoIndex] = (char)EEPROM.read(index);

  jsonStr[index - lengthNoIndex] = '\0'; // ending the string with \0
  EEPROM.end();

  DynamicJsonBuffer buf(eepromSize);
  JsonObject& root = buf.parseObject(jsonStr);
  return root;
}