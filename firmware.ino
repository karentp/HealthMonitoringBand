#define BLYNK_PRINT Serial

/* Template ID (Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID   "TMPLEj9Uy1rd"

#include <SPI.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>


char auth[] = "YourAuthToken";

char ssid[] = "Redmi 9";
char pass[] = "internet";

void setup()
{
  
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  
}

void loop()
{
  Blynk.run();
}
