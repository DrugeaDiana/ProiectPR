#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "time.h"

#define LED_PIN    12       // Digital pin connected to the LED strip
#define LED_COUNT  30      // Number of LEDs in the strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// WiFi credentials
const char *ssid = "Yuki";             // Replace with your WiFi name
const char *password = "Yuki1234";   // Replace with your WiFi password

// MQTT Broker settings
const char *mqtt_broker = "192.168.43.212";
const char *mqtt_topic = "config/system";
const char *mqtt_send_topic = "info/activity";
const char *mqtt_username = "proiect";
const char *mqtt_password = "1234";
const int mqtt_port = 8883;

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

// Root CA Certificate
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIID4zCCAsugAwIBAgIUDJUPYkFY4QP6rxuYjPJpuSn+lzswDQYJKoZIhvcNAQEL
BQAwgYAxCzAJBgNVBAYTAlJPMRAwDgYDVQQIDAdSb21hbmlhMRIwEAYDVQQHDAlC
dWN1cmVzdGkxDDAKBgNVBAoMA2NhdDEXMBUGA1UEAwwOMTkyLjE2OC40My4yMTIx
JDAiBgkqhkiG9w0BCQEWFWRydWdlYWRpYW5hQGdtYWlsLmNvbTAeFw0yNTAxMTMx
NTI3MTFaFw0zNTAxMTExNTI3MTFaMIGAMQswCQYDVQQGEwJSTzEQMA4GA1UECAwH
Um9tYW5pYTESMBAGA1UEBwwJQnVjdXJlc3RpMQwwCgYDVQQKDANjYXQxFzAVBgNV
BAMMDjE5Mi4xNjguNDMuMjEyMSQwIgYJKoZIhvcNAQkBFhVkcnVnZWFkaWFuYUBn
bWFpbC5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCn484LdKt9
ikPhQXm89AXBX55x1oxOMbCIPLXcKJsb4KPD0MFkIskeQYoR8w2diCSr3mUgoTxE
0BcvDiatst+z+EMT14eUVjCqOrWYAODq0o3phLGreZcdaouJLZaJN5bs5E+BNmHQ
rgZL+fZoNEPdslldXyPLdB+t6ouwMzGec10g3NGQVfasjcufgSswbW+DWSP+F5YE
6NLtUlBEcmT8ueL2OSk3Y1i35DPeBf2TgeTYVF9Z2ZM8HZRv3vuD7bd8kwgFkWM2
sHUXb2iytHVHEoshF04/2pamB4qY/SphW3nf5nFD+PInm7wEYhCprF9jI+uwaEgu
yCGpfF7rTMKrAgMBAAGjUzBRMB0GA1UdDgQWBBQjTsWaIxzzKKPmnLIRAj5EFDJE
JzAfBgNVHSMEGDAWgBQjTsWaIxzzKKPmnLIRAj5EFDJEJzAPBgNVHRMBAf8EBTAD
AQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBzhUr1EeHNcruL8I2gVJqPfDRdtbiW85TR
Po2FcOEhv28fqE6L9s/HkO+3EJC8BBfTgGbm3wzJCoaGqwJa7bCthmfoz9ls1+KZ
LNCLZYhNM3IrHCyO0WdYKDvTCqnK+Wpgf3WTghWZ2o42r4zZDa7YqCueDQe4Jc8u
VQ6fqTX4yOiag6u62UR3qlcdFlehGKWlZXzCaG0Tr3NvJ1oWrgAFbOGCXd/r6Gyx
fKg1hiilijOTQ78dkyMxxX1pfF8P3I0ulshbfGBYCDBO9uLTtd/5VAf86tUoeP1+
acMgzrSzyKUxaNJn9u7UzmznL3xhk35gBFoowX0qnIhDxsADaKJT
-----END CERTIFICATE-----
)EOF";

const int PIR_SENSOR_OUTPUT_PIN_1 = 17; 
const int PIR_SENSOR_OUTPUT_PIN_2 = 26; 
const char* ntpServer = "pool.ntp.org";

int motion_1 = 0;
int motion_2 = 0;
unsigned long now = millis();
unsigned long lastTrigger_1 = 0;
boolean startTimer_1 = false;
unsigned long lastTrigger_2 = 0;
boolean startTimer_2 = false;
unsigned long last_checked = 0;
int connected = 0;
int config_message = 0;

struct tm start_time;
struct tm end_time;
int config = 0;
#define timeSeconds 1000
#define timeMinutes 60000
int senzor_1_message = 0;
int senzor_2_message = 0;

void IRAM_ATTR pir_1() {
  if (connected == 1) {
     motion_1 = 1;
     Serial.println("Object Detected_1 things connected");
  } else {
    Serial.println("Object Detected_1 nothing connected");
  }
  senzor_1_message = 1;
}

void IRAM_ATTR pir_2() {
  if (connected == 1) {
     motion_2 = 1;
     Serial.println("Object Detected_2 things conencted");
  } else {
    Serial.println("Object Detected_2 nothing conencted");
  }
  senzor_2_message = 1;
}

// Function Declarations
void connectToWiFi();

void connectToMQTT();

void mqttCallback(char *topic, byte *payload, unsigned int length);


void setup() {
    Serial.begin(115200);
    connectToWiFi();
    strip.begin();     // Initialize the LED strip
    strip.show();  

    configTime(7200, 3600, ntpServer);

    // Set Root CA certificate
    esp_client.setCACert(ca_cert);
    pinMode(PIR_SENSOR_OUTPUT_PIN_1, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_OUTPUT_PIN_1), pir_1, FALLING);
    pinMode(PIR_SENSOR_OUTPUT_PIN_2, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_OUTPUT_PIN_2), pir_2, FALLING);
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTT();
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void connectToMQTT() {
    while (!mqtt_client.connected()) {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic);
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" Retrying in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    char messageBuffer[300];
    memcpy(messageBuffer, payload, length);   //copy the payload into the array
    messageBuffer[length] = '\0';
    Serial.printf("%s\n", messageBuffer);

    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, messageBuffer);
    const char* activation = doc["Activation"];
    const char* deactivation = doc["Deactivation"];
    Serial.printf("Deactivation: %s\n", activation);
    Serial.printf("Activation: %s\n", deactivation);

    parse_time(activation, deactivation);
    Serial.print("Ora activare: "); Serial.println(start_time.tm_hour);
    Serial.print("Minute activare: "); Serial.println(start_time.tm_min);
    Serial.println("\n-----------------------");
    config = 1;
}

void parse_time(const char *activation, const char *deactivation) {
    sscanf(activation, "%4d-%2d-%2dT%2d:%2d:%2d", 
         &start_time.tm_year, 
         &start_time.tm_mon, 
         &start_time.tm_mday, 
         &start_time.tm_hour, 
         &start_time.tm_min, 
         &start_time.tm_sec);
    start_time.tm_year -= 1900;
    start_time.tm_mon -= 1;     
    start_time.tm_isdst = -1;

    sscanf(deactivation, "%4d-%2d-%2dT%2d:%2d:%2d", 
         &end_time.tm_year, 
         &end_time.tm_mon, 
         &end_time.tm_mday, 
         &end_time.tm_hour, 
         &end_time.tm_min, 
         &end_time.tm_sec);
    end_time.tm_year -= 1900;
    end_time.tm_mon -= 1;    
    end_time.tm_isdst = -1;
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
        connected = 1;
        Serial.println("It's active");
    } else {
      Serial.println("After functioning hours, needs manual activation");
    }
  }
  else {
      Serial.println("Before functioning hours, needs manual activation");
  }
}

void send_message(int senzor) {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    char formattedTime[20];
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    Serial.print("Timpul formatat: ");
    Serial.println(formattedTime);
    char message[100];
    if (senzor == 1) {
      const char* senzor_name = "\"Curte\"";
      sprintf(message, "{\"Time\": \"%s\", \"Senzor\": %s}", formattedTime, senzor_name);
    }
    if (senzor == 2) {
      const char* senzor_name = "\"Poarta\"";
      sprintf(message, "{\"Time\": \"%s\", \"Senzor\": %s}", formattedTime, senzor_name);
    }
    Serial.println(message);
    mqtt_client.publish(mqtt_send_topic, message);

}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTT();
    }
    mqtt_client.loop();
    now = millis();
    if (config == 1) {
      if (config_message == 1) {
        check_active_time();
        Serial.println("Checked activation hours");
        last_checked = millis();
        config_message = 0;
      } else { 
        if (now - last_checked > timeMinutes * 10) {
          check_active_time();
          Serial.println("Checked activation hours");
          last_checked = millis();
        }
      }
      if (connected == 1 && motion_1 == 1 && startTimer_1 == false) {
          for (int i = 0; i < LED_COUNT/2; i++) {      
              strip.setPixelColor(i, strip.Color(0, 0, 255));
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
          strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        strip.show();
        startTimer_1 = false;
        motion_1 = 0;
      }
      if (connected == 1 && motion_2 == 1 && startTimer_2 == false) {
          for (int i = LED_COUNT/2; i < LED_COUNT; i++) {      
              strip.setPixelColor(i, strip.Color(255, 165, 0));
              // Red color (RGB values)
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
    } else {
      if (config_message == 0) {
        Serial.println("Not configured, please send config data");
        config_message = 1;
      }
    }
    if (senzor_1_message == 1) {
      send_message(1);
      senzor_1_message = 0;
    }
    if (senzor_2_message == 1) {
      send_message(2);
      senzor_2_message = 0;
    }
}