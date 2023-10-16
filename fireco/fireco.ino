#include "MQ7.h"
#include "WiFi.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP_Mail_Client.h>


const char* ssid = "Mevlut";
const char* password = "23012003";
const char* serverName = "http://192.168.125.61/gas";
const char* serverName2 = "http://192.168.125.61/firealarm";
const char* serverName3= "http://192.168.125.61/firesensor";
String httpRequestData;
int httpResponseCode;
SMTPSession smtp;

uint8_t broadcastAddress[] = {0xA8, 0x42, 0xE3, 0x48, 0x21, 0x38};     //MASTER//
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
unsigned long timerDelayPost = 1000;
bool wifi_connected;
unsigned long previousMillis = 0;
uint8_t on = 2;
/* on = 0 stands for closed state
   on = 1 stands for waiting state (when the button is pressed whole system shuts down for five minutes, only the green led blinks)
   on = 2 stands for opened state (when the CO ppm exceeds 50, the red led blinks and  the buzzer starts buzzing)
*/



#define Eb 156
#define red 17
#define green 16
#define interButton 25
#define closeButton 35
#define buzzer 32
#define CHANNEL 1
#define A_PIN 33
#define VOLTAGE 5

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "esp32iothome@gmail.com"
#define AUTHOR_PASSWORD "bbtmpqbqgwowmlfn"
#define RECIPIENT_EMAIL "mevmetin03@gmail.com"

TaskHandle_t Task;
WiFiClient client;
HTTPClient http;
SemaphoreHandle_t coSemaphore;
esp_now_peer_info_t peerInfo;    

          
float co;
typedef struct slave_send {
  uint8_t id = 2;
  uint8_t temp;
  uint8_t humidity;
  uint8_t coPpm;
  uint8_t weight;
  uint8_t motion;
  uint8_t istherefire;
  uint8_t firealarm;
} slave_send;
slave_send info;

void createSemaphore(){
    coSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive( ( coSemaphore) );
}
void lockVariable(){
    xSemaphoreTake(coSemaphore, portMAX_DELAY);
}
void unlockVariable(){
    xSemaphoreGive(coSemaphore);
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }

void wifi_connection(){
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  unsigned long startingTime = millis();
  while(WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
      lastTime = millis();
      if(lastTime-startingTime > timerDelay) break;
  }
  if(WiFi.status()==WL_CONNECTED){
      wifi_connected = true;
      Serial.println("");
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
    }else wifi_connected = false;

}
void http_post_request(){
  if ((millis() - lastTime) > timerDelayPost) {
    //Check WiFi connection status
    if(wifi_connected){
      //////////////////////////////////////////////////////////////////////////////
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "gas="+String(co);           
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      ///////////////////////////////////////////////////////////////////////////////
      http.begin(client, serverName2);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpRequestData ="&fire="+String(info.istherefire);           
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      ///////////////////////////////////////////////////////////////////////////////
      http.begin(client, serverName3);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      httpRequestData = "&on="+String(on);           
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      ///////////////////////////////////////////////////////////////////////////////
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }



}




MQ7 mq7(A_PIN, VOLTAGE);


int playing = 0;
void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); 
  ledcAttachPin(pin, 0); 
  ledcWriteTone(0, freq);
  playing = pin;
}
void noTone(int playing) {
  tone(playing, 0);
}
void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
  if (status.success()){

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      
      SMTP_Result result = smtp.sendingResult.getItem(i);

      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}

void IRAM_ATTR interrupt() {
  if (on) {
    on = 1;
    digitalWrite(green,LOW);
  }
}

void esp_now_setup(){
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void mail_client_setup(){
MailClient.networkReconnect(true);
smtp.debug(1);
smtp.callback(smtpCallback);
}
void setup() {
	Serial.begin(115200);
  wifi_connection();
  esp_now_setup();
  createSemaphore();
  xTaskCreatePinnedToCore(Task1,"Task1",10000,NULL,1,&Task,0);

	pinMode(red,OUTPUT); //RED LED
  pinMode(buzzer,OUTPUT); //BUZZER
  pinMode(green,OUTPUT); //GREEN LED
  pinMode(interButton,INPUT_PULLUP); //BUTTON
  pinMode(closeButton,INPUT); //PERMA CLOSE BUTTON
  
  mq7.calibrate();

  attachInterrupt(interButton, interrupt, FALLING);
 
}
 
void loop() {
  
	if (digitalRead(closeButton)==HIGH) {
    delay(1000);
    }
    if (digitalRead(closeButton)) {
      digitalWrite(green, LOW);
    if (on) on = 0;
    else on = 2;
    }

  while (on == 2) {
    co= mq7.readPpm();
    info.coPpm = co;

    digitalWrite(green,HIGH);
      if (info.coPpm>20.0) {
        info.istherefire = 1;
        int i;
        for (i=0; i<3; i++) {
          digitalWrite(red,HIGH);
          tone(buzzer, Eb);
          delay(250);
          digitalWrite(red,LOW);
          noTone(buzzer);
          delay(250);
        }}
      else {
        info.istherefire = 0;
        digitalWrite(red,LOW);
        noTone(buzzer);
      }
      if(!wifi_connected) esp_now_send(broadcastAddress, (uint8_t *) &info, sizeof(info));
      else http_post_request();

      if (digitalRead(closeButton)==HIGH) {
      delay(1000);
      }
        if (digitalRead(closeButton)) {
        digitalWrite(green, LOW);
        on = 0;
        }
    }
  if (on == 1) {
    for (uint8_t i=0; i<5; i++) {
      digitalWrite(green,LOW);
      delay(1000);
      digitalWrite(green,HIGH);
      delay(1000);
      if(!wifi_connected) esp_now_send(broadcastAddress, (uint8_t *) &info, sizeof(info));
      else http_post_request();
    }
    on = 2;
  }
  digitalWrite(green,LOW);

  if(!wifi_connected) esp_now_send(broadcastAddress, (uint8_t *) &info, sizeof(info));
  else http_post_request();
  

  }
void Task1(void* parameter)
{

Session_Config config;
SMTP_Message message;

String thereisfire = "Fire detected in the house!!!!";

unsigned long interval = 10000;
config.server.host_name = SMTP_HOST;
config.server.port = SMTP_PORT;
config.login.email = AUTHOR_EMAIL;
config.login.password = AUTHOR_PASSWORD;
config.login.user_domain = "";

/*
Set the NTP config time
For times east of the Prime Meridian use 0-12
For times west of the Prime Meridian add 12 to the offset.
Ex. American/Denver GMT would be -6. 6 + 12 = 18
See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
*/
config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
config.time.gmt_offset = 3;
config.time.day_light_offset = 0;


message.sender.name = F("ESP");
message.sender.email = AUTHOR_EMAIL;
message.subject = F("ESP Fire");
message.addRecipient(F("Mevlüt"), RECIPIENT_EMAIL);

message.text.content = thereisfire.c_str();
message.text.charSet = "us-ascii";
message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


for(;;)
{mail_client_setup();
  break;
}

if(wifi_connected) Serial.println("Mail System Online");

for(;;){
vTaskDelay(10);
if(co >= 20){
if (!smtp.connect(&config)){
  Serial.println("11/1");
  ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  Serial.println("11/2");
}
vTaskDelay(10);
if (!smtp.isLoggedIn()){
  Serial.println("\nNot yet logged in.");
}
else{
  if (smtp.isAuthenticated())
    Serial.println("\nSuccessfully logged in.");
  else
    Serial.println("\nConnected with no Auth.");
}

/* Start sending Email and close the session */
if (!MailClient.sendMail(&smtp, &message))
  ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
unsigned long startingTime = millis();
while(1){
vTaskDelay(10);
unsigned long endingTime = millis();
if(endingTime - startingTime > interval) break;
}
}
}
}