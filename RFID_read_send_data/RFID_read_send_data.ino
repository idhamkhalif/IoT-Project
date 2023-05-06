#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

String formattedDate;
String dayStamp;
String timeStamp;
String uid;

const char* ssid = "Galaxy M33";
const char* password = "123456789";
const char* Server = "https://api.siiha.id/api/absensi-siswa";

void setup(void) {
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  pinMode(2, OUTPUT);
  Serial.printf("connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  Serial.println("NDEF Reader");
  nfc.begin();
}


void loop(void) {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  if (nfc.tagPresent())
  {

    NfcTag tag = nfc.read();
    //tag.print();
    uid = tag.getUidString();
    Serial.println(uid);
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
    delay(2000);
    digitalWrite(4, LOW);
    if (WiFi.status() == WL_CONNECTED)
    {
      // Extract date
      formattedDate = timeClient.getFormattedDate();
      int splitT = formattedDate.indexOf("T");
      dayStamp = formattedDate.substring(0, splitT);
      timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);

      HTTPClient http;
      http.begin(Server);
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<200> jsonBuffer;
      JsonObject payload = jsonBuffer.to<JsonObject>();
      String payloadData;
      payload["uid_kartu"] = uid;
      payload["tanggal"] = dayStamp;
      payload["waktu"] = timeStamp;

      serializeJson(payload, payloadData);
      int httpResponseCode = http.POST(payloadData);

      if ((httpResponseCode > 0))
      {
        String response = http.getString();
        Serial.println("Response Code : " + String(httpResponseCode));
        serializeJsonPretty(payload, Serial);
        // Serial.println(response);
      }
      else
      {
        Serial.print("Error on sending POST: ");
        Serial.println("Respon Code : " + String(httpResponseCode));
      }
      http.end();

    }
  }
  else
  {
    digitalWrite(2, HIGH);
  }


}
