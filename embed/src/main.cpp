#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <HTTPClient.h>
#include <Firebase_ESP_Client.h>
#include <ESP_Google_Sheet_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// ########## CONFIGURATION : MODIFY HERE ##########
char ssid[] = "po1";      // เครือข่าย Wi-Fi ที่ต้องการเชื่อมต่อ
char pass[] = "299792ps"; // รหัสผ่านเครือข่าย

// Firebase
#define API_KEY "AIzaSyAld8mAMkx-3bGBbOAS0-eqTn4QX7PyGn0"
#define DATABASE_URL "https://es-project-f6e5f-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData firebaseData;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// Google Sheet
#define PROJECT_ID "es-project-441904"
#define CLIENT_EMAIL "ineedyourwater@es-project-441904.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDJxK1maOpwiZ6f\npKKEChbSSFBbY2RlkKlinvWvLkZGiY7Z7CEzVCTrTwv53EmJg2nkApwdYd2OLp9q\nhe2v1lTku4K6EycHfZaC1Gtkqbr6Qb2WLOvRn+1bYAGeh1GCxNnI6m3tGqWRP3oe\noxWJn55ujFLVMEbp0Omu8XC18/rcBMXOl7yoSzMNTraq2tXyFreLrNUxPD1naIJu\nPYdiJdkP4yH7IUSkPUqKDJA4GFFDHZk0auKtwXLE5PuZ8PsFhbP0W/JKGwityEV9\nGgNQzVbdsw3oKQ1LFh1fLENi7yJPFj966+ZmvlWX7/WjKhvzMHKZDss9gXGdWrEE\no9+ZlYJLAgMBAAECggEAIhTErW2/W+8DCjhG80K5BCAO4sDxLm63Dd+H5GVOnY+S\nv6CxIvhtnAmoHjZlfYLjGg6OzC/BusiJSPcCZRcxE8nQlRmzGALQr5yt3z0ERk1g\ndsNzud7nuu2GmH6G+C3EE7n1g31ixhwQxqgJdUuu+UFepIAZGlmdf11tCm8ByrP+\nxwTDN5zXosMjrMFUnEyuK32A8DBoYMZbYxTPhMdfrGBX9Up+MntN8wwKIg6tMhJr\nJV2YICfJIxvH1DL1e3I4vBrL9vBuXCI9Ej+fapli4NEX1nSMCVDBx444wYmWh43V\ndPlcdvCEAf0sqx5k7RIl+D2WXUBP+5eEhErhvACS4QKBgQDwa7bo05KXXFboKKcx\nYvQMFQscC6daYpc2xXUYmuj3cn4heQTfRTaINZGuGfyniyC8ZYqK1IwloB7iZKlo\n7yx0eyz8WGnfBavAjsxSIOurApNmFq5wAzwH12Hbjbn0avEP5XJ/8Fw9NDGiCf91\nzlmv9mlDsR+/iv28rDgeEeTjiQKBgQDW18P2GCdlogLN6Ob5HV/BtM0BHuPqtyUZ\nTgidjDNjrcRdNDatr3f4zkJt+pGHMrNvDyX2fuG97fHsQKOeJkQPjLZWD2BJJAEc\nWIa6P0O8XOetMaBMNKOq3jvmDshNdE03SgULnkZ2IQWhbqMh1PA4eWadLKwhobT3\nxbnrdZk+MwKBgFHHEcxYSwCZNCZ7zYMK+aAyeDjt7drctjq3J4FbD+s7UpPoKjQX\nwtjJ/+eMh1gmBtiOYXWfnpwdvuObAdTjpedpDoA3QqvNrgItkGpPMFpW4fRTurn3\nH7A7HtojdyEgTybtEOlbxMQ8wLUxWWek7PThiCSyVu48RNqsadowk/6hAoGALArh\nbkK3WNtkjcewCeOerLwexpsJiCVxYoWfuIR0SHvn5F9+0g5FnujRr3ANg1S+egb1\n9lk+d8weqxr5L8OSckmbW3sddwD9hU0UmXu8rBGzoMqqnNSxtKaISvo+z7oG1k3D\nSQVyVmcgm15t7NKVSE6C3be5tc9LpPuQyJc8CgcCgYEA3akQsLbk8oVjAtCuDBKc\nePEq9y/aFxBBvCJmZvVOqTYUDeZMM6SEHH5ifncUNXa3THviYP+lVz1yGD675uuB\nG6xflAgG8CzezipJfcXQ+gwBfUQ7pGqMoZiINKXBes3d1CcTyWIk916dIa/SD53l\nJJLSdA9sGOW4vMW6JoPS4Ec=\n-----END PRIVATE KEY-----\n";
const char spreadsheetId[] = "1nrptxrGPTv4CGdiIWHMLfivn4ZOGgbJkRBv9PzrSqZ8";

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
unsigned long epochTime; 

// DHT22
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

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

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

  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.setPrerefreshSeconds(10 * 60);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop()
{
  bool ready = GSheet.ready();

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

    // send Line message notification
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

    // Firebase
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

    // Google Sheet
    FirebaseJson response;
    FirebaseJson sheet;
    epochTime = getTime();

    sheet.add("majorDimension", "COLUMNS");
    sheet.set("values/[0]/[0]", epochTime);
    sheet.set("values/[1]/[0]", airHumid);
    sheet.set("values/[2]/[0]", light);
    sheet.set("values/[3]/[0]", temperature); 
    sheet.set("values/[4]/[0]", soilHumid); 

    // Append values to the spreadsheet
    bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &sheet /* data range to append */);
    if (success){
      response.toString(Serial, true);
      sheet.clear();
    }
    else{
      Serial.println(GSheet.errorReason());
    }

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
