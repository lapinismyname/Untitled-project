#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "soc/rtc_wdt.h"
#include <ESP_Mail_Client.h>


uint8_t broadcastAddress[] = {0xB4, 0x8A, 0x0A, 0x5E, 0x09, 0x3C};      //MASTER//

bool HttpCheck=true;
TaskHandle_t Task1;
uint8_t flag = 1;
uint8_t system_open = 1;
unsigned long lastTime = 0;
unsigned long timerDelayWifi = 5000;
unsigned long timerDelay = 1000;
unsigned long timerDelayMail = 10000;
bool motionZero2One = false;
String thereisfire = "Motion detected in the house!!!!";

const char* serverName = "http://192.168.125.61/motion";
const char* ssid = "Mevlut";
const char* password = "23012003";
String httpRequestData;
int httpResponseCode;
bool wifi_connected;
unsigned long previousMillis = 0;
bool psswrd_correct = 0;

WiFiClient client;
HTTPClient http;
esp_now_peer_info_t peerInfo;
Session_Config config;
SMTP_Message message;
SMTPSession smtp;

typedef struct master_send {
  int type;
  int number;
  int status;
} master_send;
master_send orders;           

typedef struct slave_send {
  uint8_t id = 3;
  uint8_t temp;
  uint8_t humidity;
  uint8_t coPpm;
  uint8_t weight;
  uint8_t motion;
  uint8_t istherefire;
  uint8_t firealarm;
} slave_send;
slave_send info;




#define button      32
#define buzzer      25
#define red         16
#define green       17
#define PIR         33
#define Eb 156
#define CHANNEL 1
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "esp32iothome@gmail.com"
#define AUTHOR_PASSWORD "bbtmpqbqgwowmlfn"
#define RECIPIENT_EMAIL "mevmetin03@gmail.com"

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

void mail_client_setup(){
MailClient.networkReconnect(true);
smtp.debug(1);
smtp.callback(smtpCallback);

config.server.host_name = SMTP_HOST;
config.server.port = SMTP_PORT;
config.login.email = AUTHOR_EMAIL;
config.login.password = AUTHOR_PASSWORD;
config.login.user_domain = "";

config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
config.time.gmt_offset = 3;
config.time.day_light_offset = 0;


message.sender.name = F("ESP");
message.sender.email = AUTHOR_EMAIL;
message.subject = F("ESP PIR");
message.addRecipient(F("Mevlüt"), RECIPIENT_EMAIL);

message.text.content = thereisfire.c_str();
message.text.charSet = "us-ascii";
message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


}

void mail_send(){
if (!smtp.connect(&config)){
 
  ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

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

if (!MailClient.sendMail(&smtp, &message))
  ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}






void esp_now_setup(){
  //esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
 // Serial.print("\r\nLast Packet Send Status:\t");
 // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
void OnDataRecv(const uint8_t* mac,const uint8_t* incomingData, int len){
  memcpy(&orders,incomingData,sizeof(orders));
  if(orders.type==4) psswrd_correct = true;
  }

void tone(byte pin, int freq) {
  ledcSetup(0, 2000, 8); 
  ledcAttachPin(pin, 0); 
  ledcWriteTone(0, freq);
}
void noTone(int playing) {
  tone(playing, 0);
}
void IRAM_ATTR detectsMovement() {
  info.motion = 1; 
}
void IRAM_ATTR close_alarm() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 200){
    psswrd_correct = 1;
  }
  last_interrupt_time = interrupt_time;
}
void wifi_connection(){
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  unsigned long startingTime = millis();
  while(WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
      lastTime = millis();
      if(lastTime-startingTime > timerDelayWifi) break;
  }
  if(WiFi.status()==WL_CONNECTED){
      wifi_connected = true;
      Serial.println("");
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
    }else wifi_connected = false;

}
void http_post_request(){
 
    //Check WiFi connection status
    Serial.println("Sending Request");
    if(wifi_connected){
      //////////////////////////////////////////////////////////////////////////////
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "motion="+String(info.motion);           
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      ///////////////////////////////////////////////////////////////////////////////
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    delay(500);
    Serial.println("Done");
  
}


void setup() {
  Serial.begin(115200);
  wifi_connection();
  info.motion = 0;

  esp_now_setup();
  
  mail_client_setup();

  pinMode(button,INPUT_PULLUP); 
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(PIR, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIR), detectsMovement, RISING);
  attachInterrupt(digitalPinToInterrupt(button), close_alarm, RISING);


  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    (void*)&info,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */    


}


void loop(){

if(HttpCheck){
http_post_request();
delay(500);
HttpCheck = false;
}
digitalWrite(red,LOW);

while(info.motion)
{
if(!HttpCheck){
    if(!wifi_connected) esp_now_send(broadcastAddress, (uint8_t *) &info, sizeof(info));
      else http_post_request();
      delay(500);
      mail_send();
      HttpCheck = true;
      }
digitalWrite(red,HIGH);
tone(buzzer,Eb);
delay(1000);
digitalWrite(red,LOW);
noTone(buzzer);
delay(1000);
}
}

void Task1code( void * infoPtr ){


for(;;){
vTaskDelay(10);
while (info.motion){

  vTaskDelay(10);
  if(psswrd_correct){
    info.motion=0;
    psswrd_correct=0;
    digitalWrite(green,HIGH);
    delay(4000);
    digitalWrite(green,LOW);
    }
}
}
}
