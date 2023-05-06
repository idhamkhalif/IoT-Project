#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define LDR 12
#define SDA1 21
#define SCL1 22
#define PIR  4
#define SNDD 35

#define LED1 26

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int statusPIR;
int statusLDR;
float temp;
float hum;
float voice;

String formattedDate;
String dayStamp;
String timeStamp;

Adafruit_BME280 bme;
TwoWire I2Cone = TwoWire(1);
uint32_t n = 400000;

/*
  const char* ssid = "Galaxy M33";
  const char* password = "123456789";
*/
const char* ssid = "HotSpot - ITB (NEW)";
const char* password = "123456789";

const char* Server = "http://206.189.154.62/api/send-datasensor";
String openWeatherMapApiKey = "fd7f3c85b7f4d92e4c2597c6e9c74723";

String city = "Kukusan";
String countryCode = "ID";
String jsonBuffer1, jsonBuffer2;
float temp1;
float suhuLuar;

HTTPClient http;

String httpGETRequest(const char* serverName) {
  WiFiClient client;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}


void setup()
{
  Serial.begin(115200);
  pinMode(LDR, INPUT_PULLUP);
  pinMode(PIR, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);

  Serial.println("init I2C");
  I2Cone.begin(SDA1, SCL1, n); // SDA pin 21, SCL pin 22
  delay(100);
  bool status = bme.begin(0x76, &I2Cone);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  Serial.printf("connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    digitalWrite(LED1, HIGH);
    delay(500);
    digitalWrite(LED1, LOW);

    if (WiFi.status() == WL_CONNECTED)
    {
      digitalWrite(LED1, 1);
    }
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  timeClient.begin();
  timeClient.setTimeOffset(25200);
}

void loop()
{
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    // Extract date
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
    String serverName = "http://192.168.137.69/data.json";
    http.begin(serverPath.c_str());
    http.begin(serverName.c_str());

    const size_t size_j = JSON_ARRAY_SIZE(2400);
    DynamicJsonDocument doc(size_j);
    DynamicJsonDocument doc1(size_j);

    hum = bme.readHumidity();
    temp = bme.readTemperature();
    statusPIR = digitalRead(PIR);
    statusLDR = digitalRead(LDR);
    voice = analogRead(SNDD);
    jsonBuffer1 = httpGETRequest(serverPath.c_str());
    jsonBuffer2 = httpGETRequest(serverName.c_str());

    DeserializationError error = deserializeJson(doc, jsonBuffer1);
    temp1 = doc["main"]["temp"].as<float>();
    temp1 = temp1 / 10;

    DeserializationError error1 = deserializeJson(doc1, jsonBuffer2);
    suhuLuar = doc1["temperature"].as<float>();

    HTTPClient http;
    http.begin(Server);
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> jsonBuffer;
    JsonObject payload = jsonBuffer.to<JsonObject>();
    /*
      payload["id"]=1;
      payload["suhu"]=25;
      payload["humid"]=25;
    */
    //payload["id"]=1;
    String payloadData;
   //String dataJson = "{\"date\":\"" + dayStamp + "\",\"time\": \"" + timeStamp + "\",\"humidity\": " + hum + ",\"ldr\": " + statusLDR + ",\"pir\": " + statusPIR + ",\"temperature\": " + temp + ",\"voice\": " + voice + "}";

    payload["date"] = dayStamp;
    payload["time"] = timeStamp;
    payload["humidity"] = hum;
    payload["ldr"] = statusLDR;
    payload["pir"] = statusPIR;
    payload["temperature"] = temp;
    payload["voice"] = voice;
    payload["device_id"] = "device2";
    payload["openweather"] = temp1;
    payload["suhu_luar_ruangan"] = suhuLuar;

    serializeJsonPretty(payload, Serial);
    serializeJson(payload, payloadData);
    // Serial.println(dataJson);
    Serial.println(" ");
    int httpResponseCode = http.POST(payloadData);
    if ((httpResponseCode > 0))
    {
      String response = http.getString();
      Serial.println("Response Code : " + String(httpResponseCode));
      // Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println("Respon Code : " + String(httpResponseCode));
    }
    http.end();
  }
  else Serial.println("Error in WiFi connection");
  delay(60000);
}
