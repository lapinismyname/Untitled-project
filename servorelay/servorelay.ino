#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include<ESP32Servo.h>

#define IN1 33
#define IN2 32
#define IN3 25
#define IN4 19
#define servoPin1 16
#define servoPin2 17

Servo servo1;
Servo servo2;

typedef struct master_send {
  uint8_t type;
  uint8_t number;
  uint8_t status;
} master_send;
master_send orders;

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&orders, incomingData, sizeof(orders));
  Serial.print(orders.type);
  Serial.print(orders.number);
  Serial.println(orders.status);
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,HIGH);
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,HIGH);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel("Mevlut");
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  
}
 
void loop() {
  if(orders.type==1)
  {
    if(orders.number==1)
    {
      if(orders.status==1) digitalWrite(IN1,LOW);
      else if(orders.status==0) digitalWrite(IN1,HIGH);
    }
    if(orders.number==2)
    {
      if(orders.status==1) digitalWrite(IN2,LOW);
      else if(orders.status==0) digitalWrite(IN2,HIGH);
    }
    if(orders.number==3)
    {
      if(orders.status==1) digitalWrite(IN3,LOW);
      else if(orders.status==0) digitalWrite(IN3,HIGH);
    }
    if(orders.number==4)
    {
      if(orders.status==1) digitalWrite(IN4,LOW);
      else if(orders.status==0) digitalWrite(IN4,HIGH);
    }
  }
  else if(orders.type == 2)
  {
    if(orders.number == 1)
    {
      if(orders.status == 1) servo1.write(90);
      else if(orders.status == 0) servo1.write(0);
    }
    if(orders.number == 2)
    {
      if(orders.status == 1) servo2.write(90);
      else if(orders.status == 0) servo2.write(0);
    }

  }
}
