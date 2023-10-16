#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "HX711.h"
#include "DHT.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <HTTPClient.h>

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN 25
#define LOADCELL_SCK_PIN 26

#define CHANNEL 1
#define DHTPIN 32
#define DHTTYPE DHT11

HX711 scale;
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address and the number of columns and rows

TaskHandle_t Task1;
//SERVER STUFF
const char* ssid = "Mevlut";
const char* password = "23012003";
const char* serverWater = "http://192.168.125.61/water";
const char* serverTemp = "http://192.168.125.61/temp";
const char* serverHum = "http://192.168.125.61/humidity";

String waterPost;
String tempPost;
String humPost;
int httpResponseCode;

uint8_t broadcastAddress[] = { 0xB4, 0x8A, 0x0A, 0x5E, 0x09, 0x3C };  //MASTER//
unsigned long lastTime = 0;
unsigned long timerDelay = 500;
unsigned long timerDelayWifi = 5000;
bool wifi_connected;
unsigned long previousMillis = 0;

WiFiClient client;
HTTPClient http;
esp_now_peer_info_t peerInfo;

float temperature;
float humidity;
float water;
typedef struct slave_send {
  uint8_t id = 1;
  int8_t temp;
  uint8_t humidity;
  uint8_t coPpm;
  uint8_t water;
  uint8_t motion;
  uint8_t istherefire;
  uint8_t firealarm;
} slave_send;
slave_send info;

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void wifi_connection() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  unsigned long startingTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    lastTime = millis();
    if (lastTime - startingTime > timerDelayWifi) break;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  } else wifi_connected = false;
}
void http_post_request() {
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (wifi_connected) {
      http.begin(client, serverTemp);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      tempPost = "temp=" + String(temperature);
      http.POST(tempPost);
      http.end();

      http.begin(client, serverWater);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      waterPost = "water=" + String(water);
      http.POST(waterPost);
      http.end();

      http.begin(client, serverHum);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      humPost = "humidity=" + String(humidity);
      http.POST(humPost);
      http.end();
    }
    lastTime = millis();
  }
}

void esp_now_setup() {
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void Task1code(void* parameter) {
  vTaskDelay(10);
  for (;;) {
    water = scale.get_units(5);
    if (water < 0) water = 0;
    else water /= 1000;
    Serial.println(water, 2);

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    Serial.println(temperature);
    Serial.println(humidity);

    lcd.setCursor(0, 1);  // set the cursor to column 0, line 1
    lcd.print(String(water) + "L water left");
    lcd.setCursor(0, 0);  // set the cursor to column 0, line 1
    lcd.print(String(temperature) + " C  " + String(humidity) + "%");
  }
}

void setup() {
  Serial.begin(115200);

  wifi_connection();
  esp_now_setup();

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_offset(4294419263);
  scale.set_scale(-112);  // **********BURASI KALİBRE ŞEYSİ ***********
  //scale.set_scale(-471.497);   // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();
  lcd.begin();
  lcd.backlight();
  dht.begin();

  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
}

void loop() {
  if (!wifi_connected) {
    info.water = water;
    info.temp = temperature;
    info.humidity = humidity;
    esp_now_send(broadcastAddress, (uint8_t*)&info, sizeof(info));
  } else http_post_request();

  delay(500);
}
