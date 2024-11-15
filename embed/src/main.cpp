#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// ########## CONFIGURATION : MODIFY HERE ##########
char ssid[] = "po1";      // เครือข่าย Wi-Fi ที่ต้องการเชื่อมต่อ
char pass[] = "299792ps"; // รหัสผ่านเครือข่าย

#define API_KEY "AIzaSyAld8mAMkx-3bGBbOAS0-eqTn4QX7PyGn0"
#define DATABASE_URL "https://es-project-f6e5f-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData firebaseData;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LDRPIN 35

// ข้อความ ที่จะแสดงใน Line
String txt1 = "ความชื้นเท่ากับ "; // ข้อความ 1 ที่จะแสดงใน Line
String txt2 = " รดน้ำได้แล้ว !"; // ข้อความ 2 ที่จะแสดงใน Line

// กำหนดขาที่เชื่อมต่อกับเซ็นเซอร์
int sensorPin = 34;

int maxHumidity = 0;
int minHumidity = 9999;

const String channelAccessToken = "ZVz4QailNs7w5K0ZKVBSCSktM5lyn6NLvoCKfXIej5NZ1HE87yRGWFKYxyEVClw1j2vidfC6+BMvuf2HG/bKieKTZAV52PcB+NFsQcCWgTmGUzQWx4nw4UcO8HNF3IDdT/dGwwbWufcvR22ogtUKFwdB04t89/1O/w1cDnyilFU=";
const String userId = "U9a8cae192b1f8130f069637f58e0f275";

void sendLineMessage(int moistureInside, int moistureOutside, float temperature, int light, String notifyText)
{
  // Check if WiFi is connected
  if (WiFi.status() == WL_CONNECTED)
  {
    const String channelAccessToken = "ZVz4QailNs7w5K0ZKVBSCSktM5lyn6NLvoCKfXIej5NZ1HE87yRGWFKYxyEVClw1j2vidfC6+BMvuf2HG/bKieKTZAV52PcB+NFsQcCWgTmGUzQWx4nw4UcO8HNF3IDdT/dGwwbWufcvR22ogtUKFwdB04t89/1O/w1cDnyilFU=";
    const String userId = "U9a8cae192b1f8130f069637f58e0f275";

    HTTPClient http;

    // Define the URL of the LINE Messaging API
    const String url = "https://api.line.me/v2/bot/message/push";

    // Start HTTP connection
    http.begin(url);

    // Set the required headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + channelAccessToken); // Add Authorization header

    String data = "{\"to\":\"" + userId + "\",\"messages\":["
                                          "{\"type\":\"text\",\"text\":\"Moisture (Inside): " +
                  String(moistureInside) + "%\"},"
                                          "{\"type\":\"text\",\"text\":\"Moisture (Outside): " +
                  String(moistureOutside) + "%\"},"
                                            "{\"type\":\"text\",\"text\":\"Temperature: " +
                  String(temperature) + "°C\"},"
                                        "{\"type\":\"text\",\"text\":\"Light: " +
                  String(light) + " lux\"},"
                                  "{\"type\":\"text\",\"text\":\"" +
                  notifyText + "\"}]}";

    // Send the POST request with the payload
    int httpCode = http.POST(data);

    // Check the response
    if (httpCode > 0)
    {
      String response = http.getString(); // Get the response as a string
      Serial.println("Message sent successfully");
      // Serial.println("Response: " + response);
    }
    else
    {
      Serial.println("Error sending message");
    }

    // End HTTP request
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

void setup()
{

  pinMode(sensorPin, INPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  Serial.print("WiFi Connecting");
  Serial.println(F("DHTxx test!"));

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  dht.begin();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    int light = analogRead(LDRPIN);
    int soilHumid = analogRead(sensorPin);

    delay(2000);
    float airHumid = dht.readHumidity();
    float temperature = dht.readTemperature();
    float f = dht.readTemperature(true);
    float hif = dht.computeHeatIndex(f, airHumid);
    float hic = dht.computeHeatIndex(temperature, airHumid, false);

    if (isnan(airHumid) || isnan(temperature) || isnan(f))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    if (soilHumid < 2500)
    {
      Serial.print("Moisture Sensor Value:");
      Serial.println(soilHumid);
      sendLineMessage(soilHumid, airHumid, temperature, 1, txt1 + String(soilHumid) + txt2);
    }
    else if (soilHumid >= 2500)
    {
      Serial.print("Soil Moisture Sensor Value:");
      Serial.println(soilHumid);
    }

    Serial.println("Attempting to set Firebase value...");

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/soilHumid", soilHumid))
    {
      Serial.println();
      Serial.print(soilHumid);
      Serial.print("-successfully saved");
      Serial.println(fbdo.dataPath());
    }

    else
    {
      Serial.println("FAILED");
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/temperature", temperature))
    {
      Serial.print("-successfully saved");
      Serial.println(fbdo.dataPath());
    }
    else
    {
      Serial.println("FAILED");
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/airHumid", airHumid))
    {
      Serial.println();
      Serial.print(airHumid);
      Serial.print("-successfully saved");
      Serial.println(fbdo.dataPath());
    }

    else
    {
      Serial.println("FAILED");
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/light", soilHumid))
    {
      Serial.println();
      Serial.print(light);
      Serial.print("-successfully saved");
      Serial.println(fbdo.dataPath());
    }

    else
    {
      Serial.println("FAILED");
    }

    Serial.println("Finished attempting to set Firebase value.");
    // Serial.print("Moisture Sensor Value:");
    // Serial.println(soilHumid);
    // if (maxHumidity < Humidity)
    // {
    //   maxHumidity = Humidity;
    // }
    // if (minHumidity > Humidity)
    // {
    //   minHumidity = Humidity;
    // }
    // Serial.print("Max Humidity: ");
    // Serial.println(maxHumidity);
    // Serial.print("Min Humidity: ");
    // Serial.println(minHumidity);

    // Serial.print(F("Humidity: "));
    // Serial.print(h);
    // Serial.print(F("%  Temperature: "));
    // Serial.print(t);
    // Serial.print(F("°C "));
    // Serial.print(f);
    // Serial.print(F("°F  Heat index: "));
    // Serial.print(hic);
    // Serial.print(F("°C "));
    // Serial.print(hif);
    // Serial.println(F("°F"));

    // delay(500);
  }
}
