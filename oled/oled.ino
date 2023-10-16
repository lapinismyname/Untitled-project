#include <Adafruit_GFX.h> 
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>          
#include <TimeLib.h> 
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <esp_now.h>

const char *ssid     = "Mevlut";   
const char *password = "23012003"; 
#define CHANNEL 1
WiFiUDP ntpUDP;
HTTPClient http;
NTPClient timeClient(ntpUDP, "time.nist.gov", 18000, 60000);
char Time[ ] = "00:00:00";
char Date[ ] = "00/00/2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;
unsigned long lastTime = 0;
unsigned long timerDelayWifi = 10000;
unsigned long timerDelay = 5000;
bool wifi_connected;


String tempString;
String humString;
String COppmString;
String waterString;
String motionString;


String serverTemp = "http://192.168.125.61/temp";
String serverHum = "http://192.168.125.61/humidity";
String serverCOppm = "http://192.168.125.61/gas";
String serverWater = "http://192.168.125.61/water";
String serverMotion = "http://192.168.125.61/motion";



#define TFT_CS        5
#define TFT_RST       33
#define TFT_DC        32
#define button        25
uint8_t page;
uint8_t this_page = -1;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void IRAM_ATTR detectsMovement() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (interrupt_time - last_interrupt_time > 200){
    if(page<=5) page+=1;
    else page= 0;
  } 
  last_interrupt_time = interrupt_time;
}

int8_t temp;
uint8_t hum;
float coPPM;
float water; 
uint8_t motion; 
int t1,t2,t3,t4,t5,t6,d1,d2,d3,d4,d5,d6;


typedef struct slave_send {
  uint8_t id;
  int8_t  temp;
  uint8_t humidity;
  uint8_t coPpm;
  uint8_t water;
  uint8_t motion;
  uint8_t istherefire;
  uint8_t firesensoron;
} slave_send;
slave_send info;



void displayPage(){
   this_page = page; 
  
  if(page == 0){
    uint16_t color = 100;
    int i;
    int t;
    for(t = 0 ; t <= 4; t+=1) {
      int x = 0;
      int y = 0;
      int w = tft.width()-2;
      int h = tft.height()-2;
      for(i = 0 ; i <= 16; i+=1) {
        tft.drawRoundRect(x, y, w, h, 5, color);
        x+=2;
        y+=3;
        w-=4;
        h-=6;
        color+=1100;
        delay(10);
      }
      color+=100;
   }
  }



  else if(page == 1){
    if (!wifi_connected){
      tft.setCursor(0, 10);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);  
      tft.setTextSize(2); 
      tft.println("No time and date data");
    } else {
      handleTime();
      if(t6!=Time[7]) tft.fillRect(84, 10, 10, 15, ST7735_BLACK);
      if(t5!=Time[6]) tft.fillRect(72, 10, 10, 15, ST7735_BLACK); 
      if(t4!=Time[4]) tft.fillRect(48, 10, 10, 15, ST7735_BLACK); 
      if(t3!=Time[3]) tft.fillRect(36, 10, 10, 15, ST7735_BLACK); 
      if(t2!=Time[1]) tft.fillRect(12, 10, 10, 15, ST7735_BLACK);
      if(t1!=Time[0]) tft.fillRect(0, 10, 10, 15, ST7735_BLACK);
      if(d6!=Date[9]) tft.fillRect(108, 26, 10, 15, ST7735_BLACK);
      if(d5!=Date[8]) tft.fillRect(96, 26, 10, 15, ST7735_BLACK);
      if(d4!=Date[4]) tft.fillRect(48, 26, 10, 15, ST7735_BLACK);
      if(d3!=Date[3]) tft.fillRect(36, 26, 10, 15, ST7735_BLACK);
      if(d2!=Date[1]) tft.fillRect(12, 26, 10, 15, ST7735_BLACK);
      if(d1!=Date[0]) tft.fillRect(0, 26, 10, 15, ST7735_BLACK);

      tft.setCursor(0, 10);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);  
      tft.setTextSize(2); 
      tft.println(Time);  
      tft.println(Date);

      t6=Time[7];
      t5=Time[6];
      t4=Time[4];
      t3=Time[3];
      t2=Time[1];
      t1=Time[0];
      d1=Date[0];
      d2=Date[1];
      d3=Date[3];
      d4=Date[4];
      d5=Date[8];
      d6=Date[9];
    }
  }
  
  


  else if(page == 2){
    int8_t thisTemp = temp;
    tft.fillCircle(20, 80, 10, ST7735_ORANGE);
    tft.drawRoundRect(18, 20, 6, 63, 4, ST7735_ORANGE);
    uint8_t termometer = map(temp, -40, 60 ,0, 60);
    tft.fillRect(18, 83, 6, -termometer, ST7735_ORANGE);
    tft.setCursor(40, 40);
    tft.setTextColor(ST7735_ORANGE, ST7735_BLACK);  
    tft.setTextSize(3); 
    tft.print(temp);
    tft.setCursor(90, 40);
    tft.print("C") ;
    tft.drawCircle(82, 40, 3, ST7735_ORANGE);
    tft.drawCircle(82, 40, 2, ST7735_ORANGE);
    if (thisTemp!=temp){
      tft.fillRect(40, 40, 32, 21, ST7735_BLACK);
    }    
  }
  

  else if(page == 3){
    int8_t thisHum = hum;
    
    tft.setCursor(40, 40);
    tft.setTextColor(ST7735_CYAN, ST7735_BLACK);  
    tft.setTextSize(3); 
    tft.print(hum);
    tft.setCursor(90, 40);
    tft.print("%") ;

    if (thisHum!=hum){
      tft.fillRect(40, 40, 32, 21, ST7735_BLACK);
    }

  }


  
  else if(page == 4){
    float thiscoPPM = coPPM;
    tft.drawLine(15, 10, 15, 35, 0xB5B5B5);
    tft.drawLine(30, 5, 30, 25, 0xB5B5B5);
    tft.drawLine(45, 10, 45, 35, 0xB5B5B5);
    
    tft.fillTriangle(22, 60, 38, 60, 30, 30, ST7735_RED);
    tft.fillTriangle(10, 80, 22, 60, 10, 40, ST7735_RED);
    tft.fillTriangle(50, 80, 38, 60, 50, 40, ST7735_RED);
    for(uint8_t i=18; i<=20; i++){
      tft.drawCircle(30, 80, i, ST7735_RED);
    }
    for(uint8_t i=10; i<18; i++){
      tft.drawCircle(30, 80, i, ST7735_ORANGE);
    }
    tft.fillCircle(30, 80, 10, ST7735_YELLOW);
    //tft.fillCircle(20, 59, 2, ST7735_BLACK);
    //tft.fillCircle(40, 59, 2, ST7735_BLACK);

    tft.setCursor(73, 20);
    tft.setTextColor(ST7735_ORANGE, ST7735_BLACK);  
    tft.setTextSize(2);
    tft.println("CO:");

    tft.setCursor(70, 47); 
    tft.setTextSize(3);
    tft.println(coPPM);

    tft.setCursor(73, 80); 
    tft.setTextSize(2);
    tft.println("ppm");
    if (thiscoPPM!=coPPM){
      tft.fillRect(70, 47, 50, 21, ST7735_BLACK);
    }
  }




    
  
  else if(page == 5){
    float thiswater = water;
    tft.fillCircle(30, 80, 20, ST7735_CYAN);
    tft.fillTriangle(10, 78, 50, 78, 30, 20, ST7735_CYAN);
    tft.setCursor(73, 20);
    tft.setTextColor(ST7735_BLUE, ST7735_BLACK);  
    tft.setTextSize(2);
    tft.println("Water:");

    tft.setCursor(70, 47); 
    tft.setTextSize(3);
    tft.println(water);

    tft.setCursor(73, 80); 
    tft.setTextSize(2);
    tft.println("Liters");
    if (thiswater!=water){
      tft.fillRect(70, 47, 50, 21, ST7735_BLACK);
    } 

  }
  
  
  
  else if(page == 6){
    tft.fillScreen(ST7735_BLACK);
    tft.drawCircle(40, 40, 15, ST7735_WHITE);
    tft.drawCircle(40, 40, 14, ST7735_WHITE);
    tft.drawCircle(40, 40, 13, ST7735_WHITE);
    tft.fillRect(40, 55, 2, 30, ST7735_WHITE);
    tft.drawLine(40, 60, 20, 70, ST7735_WHITE);
    tft.drawLine(40, 61, 20, 71, ST7735_WHITE);
    tft.drawLine(40, 60, 60, 70, ST7735_WHITE);
    tft.drawLine(40, 61, 60, 71, ST7735_WHITE);

    tft.drawLine(40, 85, 20, 105, ST7735_WHITE);
    tft.drawLine(40, 86, 20, 106, ST7735_WHITE);
    tft.drawLine(40, 85, 60, 105, ST7735_WHITE);
    tft.drawLine(40, 86, 60, 106, ST7735_WHITE);

    if(motion == 0){
      tft.setCursor(90, 20); 
      tft.setTextSize(3);
      tft.println("NO");
      tft.setCursor(80, 46); 
      tft.setTextSize(2);
      tft.println("PERSON");
      tft.setCursor(66, 66); 
      tft.println("DETECTED");
      
    }else if(motion == 1){
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);  
      tft.setCursor(80, 40);
      tft.setTextSize(2);
      tft.println("PERSON");
      tft.setCursor(66, 70); 
      tft.println("DETECTED");
    }
    

    
  }
  if(this_page != page){
    tft.fillScreen(ST7735_BLACK);
  }
  
}

void http_temp();
void http_hum();
void http_COppm();
void http_water();
void http_motion();
void esp_now_setup();
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);

void setup() {
  Serial.begin(115200);
  wifi_connection();
  if(!wifi_connected) esp_now_setup();

  timeClient.begin();
  timeClient.setTimeOffset(10800);
  pinMode(button,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), detectsMovement, FALLING);
  tft_initialize();

}

void loop() {
  if(millis()-lastTime > timerDelay){
    http_temp();
    http_hum();
    http_COppm();
    http_water();
    http_motion();
    lastTime = millis();
  }
  string_to_data();
  displayPage();

}



void handleTime(){
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server
  second_ = second(unix_epoch);
  if (last_second != second_) {
    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);
    Time[7] = second_ % 10 + 48;
    Time[6] = second_ / 10 + 48;
    Time[4] = minute_ % 10 + 48;
    Time[3] = minute_ / 10 + 48;
    Time[1] = hour_   % 10 + 48;
    Time[0] = hour_   / 10 + 48;
    Date[0] = day_   / 10 + 48;
    Date[1] = day_   % 10 + 48;
    Date[3] = month_  / 10 + 48;
    Date[4] = month_  % 10 + 48;
    Date[8] = (year_   / 10) % 10 + 48;
    Date[9] = year_   % 10 % 10 + 48;
    last_second = second_;
  }
}
void tft_initialize(){
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK); 
  tft.setRotation(3);
  tft.setTextWrap(false);  
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
void esp_now_setup(){
  esp_wifi_set_channel(CHANNEL,WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("esp now init.");
  Serial.println(WiFi.macAddress());
}
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len){
  info.id = *(incomingData);
  if(info.id == 1) tempString = String(*(incomingData+1));
  if(info.id == 1) humString = String(*(incomingData+2));
  if(info.id == 1) waterString = String(*(incomingData+4));
  if(info.id == 2) COppmString = String(*(incomingData+3));
  if(info.id == 3) motionString = String(*(incomingData+5));
  Serial.println("data received");
}
void http_temp(){
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){

      http.begin(serverTemp.c_str());
      int httpResponseCode = http.GET();
      Serial.println(httpResponseCode);
      if (httpResponseCode>0) {
        tempString = http.getString();
      }
    }
      http.end();
  }
  lastTime = millis();
}
void http_hum(){
  if(WiFi.status()== WL_CONNECTED){

    http.begin(serverHum.c_str());
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
      humString = http.getString();
    }
  }
    http.end();
}
void http_COppm(){
  if(WiFi.status()== WL_CONNECTED){

    http.begin(serverCOppm.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode>0) {
      COppmString = http.getString();
    }
  }
    http.end();
}
void http_water(){
  if(WiFi.status()== WL_CONNECTED){

    http.begin(serverWater.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode>0) {
      waterString = http.getString();
    }
  }
    http.end();
}
void http_motion(){
  if(WiFi.status()== WL_CONNECTED){

    http.begin(serverMotion.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode>0) {
      motionString = http.getString();
    }
  }
    http.end();
}
void string_to_data(){
  temp = tempString.toInt();
  hum = humString.toInt();
  coPPM = COppmString.toFloat();
  if(motionString.equals(String("Detected!"))) motion = 1;
  else if(motionString.equals(String("NOT detected"))) motion = 0;
  water = waterString.toFloat();
  
}
