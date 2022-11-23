// format spiffs

#include "SPIFFS.h"
#endif
#include <SPI.h>

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("SPIFFS filesystem");
  // mount SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("FS mounted");
  }
  else
  {
    Serial.println("FS not mounted");
    Serial.println("Formatting SPIFFS: please wait");
    SPIFFS.format();
    Serial.println("format complete");
  }
}

void loop()
{

}
