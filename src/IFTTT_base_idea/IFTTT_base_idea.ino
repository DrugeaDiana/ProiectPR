#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"

#define LED_PIN    12       // Digital pin connected to the LED strip
#define LED_COUNT  30      // Number of LEDs in the strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define WLAN_SSID       "Yuki"             // Your SSID
#define WLAN_PASS       "Yuki1234"        // Your password
const char* ntpServer = "pool.ntp.org";
/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com" //Adafruit Server
#define AIO_SERVERPORT  1883                   
#define AIO_USERNAME    "TiredMoonCat"           // Username
#define AIO_KEY         "aio_nyuZ979cQmrrSTO6Zv7chbuHMK43"   // Auth Key

//WIFI CLIENT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/LedBand");
const int PIR_SENSOR_OUTPUT_PIN_1 = 17; 
const int PIR_SENSOR_OUTPUT_PIN_2 = 26; 
int connected = 0;
bool shutdown = false;

struct tm sunset_time;
struct tm sunrise_time;
struct tm start_time;
struct tm end_time;
int current_day, current_month;
void MQTT_connect();

#define timeSeconds 1000
#define timeMinutes 60000

int motion_1 = 0;
int motion_2 = 0;
unsigned long now = millis();
unsigned long lastTrigger_1 = 0;
boolean startTimer_1 = false;
unsigned long lastTrigger_2 = 0;
boolean startTimer_2 = false;
unsigned long last_checked = 0;

boolean daylight = false;
boolean nightlight = false;

void IRAM_ATTR pir_1() {
  if (connected == 1) {
     motion_1 = 1;
     Serial.println("Object Detected_1 things conencted");
  } else {
    Serial.println("Object Detected_1 nothing conencted");
  }
}

void IRAM_ATTR pir_2() {
  if (connected == 1) {
     motion_2 = 1;
     Serial.println("Object Detected_2 things conencted");
  } else {
    Serial.println("Object Detected_2 nothing conencted");
  }
}


void setup() {
  Serial.begin(115200);
  strip.begin();     // Initialize the LED strip
  strip.show();  

  WiFi.mode(WIFI_STA);
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  //Serial.println("IP address: "); 
  //Serial.println(WiFi.localIP());
  mqtt.subscribe(&Light1);
  configTime(7200, 3600, ntpServer);
  //printLocalTime();
  getSunsetSunriseTime();
  start_time = createStartHour();
  end_time = createEndHour();
  current_day = sunset_time.tm_mday;
  current_month = sunset_time.tm_mon;
  pinMode(PIR_SENSOR_OUTPUT_PIN_1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_OUTPUT_PIN_1), pir_1, FALLING);
  pinMode(PIR_SENSOR_OUTPUT_PIN_2, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_OUTPUT_PIN_2), pir_2, FALLING);
  check_light_type();
  check_active_time();
}

void check_light_type() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  time_t current_time = mktime(&timeinfo);
  time_t daylight_end = mktime(&sunset_time);
  time_t daylight_start = mktime(&sunrise_time);

  double diffSecs = difftime(current_time, daylight_start);
  if (diffSecs > 0) {
    diffSecs = difftime(current_time, daylight_end);
    if (diffSecs < 0) {
      daylight = true;
      nightlight = false;
    } else {
      daylight = false;
      nightlight = true;
    }
  } else {
    daylight = false;
    nightlight = true;
  }
}

void check_active_time() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(timeinfo.tm_hour);
  //Serial.println(start_time.tm_hour);
  time_t current_time = mktime(&timeinfo);
  time_t end = mktime(&end_time);
  time_t start = mktime(&start_time);

  double diffSecs = difftime(current_time, start);

  if (diffSecs >= 0) {
    diffSecs = difftime(current_time, end);
    if (diffSecs < 0) {
      if (shutdown == false) {
        connected = 1;
        Serial.println("It's active");
      } else {
        Serial.println("It's turned off. Please activate it manually");
      }
    } else {
      Serial.println("After functioning hours, needs manual activation");
    }
  } else {
      Serial.println("Before functioning hours, needs manual activation");
  }
}


void loop() {
  MQTT_connect();

  now = millis();
  if (now - last_checked > timeMinutes * 10) {
    check_time();
    check_active_time();
    check_light_type();
    Serial.println("Checked change of day & activation hours");
  }
  if (connected == 1 && motion_1 == 1 && startTimer_1 == false) {
      for (int i = 0; i < LED_COUNT/2; i++) {      
        if (daylight == true) {
          strip.setPixelColor(i, strip.Color(0, 0, 255));
        } else if (nightlight == true) {
          strip.setPixelColor(i, strip.Color(255, 165, 0));
        }  // Red color (RGB values)
      }
      strip.show();
      Serial.println("Object Detected_1");
      startTimer_1 = true;
      lastTrigger_1 = millis();
  }
  now = millis();

  if( startTimer_1 && (now - lastTrigger_1 > ( timeMinutes*1)))
  {
    Serial.println("Turning OFF the LED 1" );
    for (int i = 0; i < LED_COUNT/2; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));  // Red color (RGB values)
    }
    strip.show();
    startTimer_1 = false;
    motion_1 = 0;
  }
  if (connected == 1 && motion_2 == 1 && startTimer_2 == false) {
      for (int i = LED_COUNT/2; i < LED_COUNT; i++) {      
        if (daylight == true) {
          strip.setPixelColor(i, strip.Color(0, 0, 255));
        } else if (nightlight == true) {
          strip.setPixelColor(i, strip.Color(255, 165, 0));
        }  // Red color (RGB values)
      } 
      strip.show();
      Serial.println("Object Detected_2");
      startTimer_2 = true;
      lastTrigger_2 = millis();
  }
  now = millis();

  if( startTimer_2 && (now - lastTrigger_2 > ( timeMinutes*1)))
  {
    Serial.println("Turning OFF the LED 2" );
    for (int i = LED_COUNT/2; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));  // Red color (RGB values)
    }
    strip.show();
    startTimer_2 = false;
    motion_2 = 0;
  }
  if (shutdown == 1) {
    strip.clear();
    strip.show();
  }
  Adafruit_MQTT_Subscribe *subscription;
  if (subscription = mqtt.readSubscription(100)) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      connected = Light1_State;
      if (connected == 1) {
        shutdown = false;
        Serial.println("Manual activation received");
      } else {
        shutdown = true;
        Serial.println("Forced shutdown received");
      }
    }
  }
 
}

void updateTimes() {
  getSunsetSunriseTime();
  start_time = createStartHour();
  end_time = createEndHour();
}

void check_time() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  if (current_day != timeinfo.tm_mday) {
    updateTimes();
  }
  last_checked = millis();
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); 
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
  
}

void getSunsetSunriseTime() {
  String serverName = "https://api.sunrise-sunset.org/json?lat=44.4268&lng=26.1025&date=today&formatted=0&tzid=Europe/Bucharest";
  HTTPClient http;
  String serverPath = serverName;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  String payload;

  if (httpResponseCode>0) {
        payload = http.getString();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  const char* time_data1 = doc["results"]["sunset"];
  int year, month, day, hour, minute, second;
  char timezone_sign;
  int timezone_hour, timezone_minute;
  // Parsează șirul de caractere
  sscanf(time_data1, "%d-%d-%dT%d:%d:%d%c%d:%d",
         &year, &month, &day, &hour, &minute, &second,
         &timezone_sign, &timezone_hour, &timezone_minute);

  sunset_time.tm_year = year - 1900; 
  sunset_time.tm_mon = month - 1;     
  sunset_time.tm_mday = day;
  sunset_time.tm_hour = hour;
  sunset_time.tm_min = minute;
  sunset_time.tm_sec = second;
  sunset_time.tm_isdst = -1;

  const char* time_data2 = doc["results"]["sunrise"];
  // Parsează șirul de caractere
  sscanf(time_data2, "%d-%d-%dT%d:%d:%d%c%d:%d",
         &year, &month, &day, &hour, &minute, &second,
         &timezone_sign, &timezone_hour, &timezone_minute);

  sunrise_time.tm_year = year - 1900;  
  sunrise_time.tm_mon = month - 1;     
  sunrise_time.tm_mday = day;
  sunrise_time.tm_hour = hour;
  sunrise_time.tm_min = minute;
  sunrise_time.tm_sec = second;
  sunrise_time.tm_isdst = -1;
}

struct tm createStartHour() {
  struct tm extra_time = sunset_time;
  extra_time.tm_min += 30;
  mktime(&extra_time);
  return extra_time;
}

struct tm createEndHour() {
  struct tm extra_time = sunset_time;
  extra_time.tm_hour += 4;
  mktime(&extra_time);
  return extra_time;
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");

}
