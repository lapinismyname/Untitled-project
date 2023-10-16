#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "time.h"         

const char* ssid = "Mevlut";
const char* password = "23012003";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
    <link rel="stylesheet" href="styles.css">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <link href="https://db.onlinewebfonts.com/c/060c3b5d0ce760f5b7ab3008d536cb4e?family=ReloadAlt-Regular" rel="stylesheet">
    <script src="https://kit.fontawesome.com/3030eb36f4.js" crossorigin="anonymous"></script>
</head>
<body>
<img src="logo.jpg" alt="metu" title="METU" id="logo">
<h2>UNTITLED PROJECT</h2>
<hr class="page-title-border">
<div class="datas"><p>
  <i class="fa-solid fa-temperature-half fa-xl"></i>
  <span class="sensor-labels">Temperature</span> 
  <span id="temp">%TEMP%</span>
  <sup class="units">°C</sup>
  
  <i class="fa-solid fa-droplet fa-xl" style="color: #24d3ff;"></i>
  <span class="sensor-labels">Humidity</span> 
  <span id="humidity">%HUMIDITY%</span>
  <sup class="units">&#37</sup>
  <br>
  <br>

  <i class="fa-solid fa-triangle-exclamation fa-xl" style="color: #ff5900;"></i>
  <span class="sensor-labels">CarbonMonoxide</span> 
  <span id="gas">%CARBONMO%</span>
  <sup class="units">ppm</sup>
  
  <i class="fa-solid fa-fire-flame-simple fa-xl" style="color: #babdb6;"></i> 
  <span id="firealarm">%FIREALARM%</span>
  
  <i class="fa-solid fa-power-off fa-xl" style="color: #f01414;"></i>
  <span id="firesensor">%FIRESENSOR%</span>
  <br>
  <br>

  <i class="fa-solid fa-bottle-water fa-xl" style="color: #0f67ff;"></i>
  <span class="sensor-labels">Water left</span> 
  <span id="water">%WATER%</span>
  <sup class="units">L</sup>
  <br>
  <br>

  <i class="fa-solid fa-user-secret fa-xl" style="color: #000000;"></i>
  <span class="sensor-labels">Motion</span> 
  <span id="motion">%MOTION%</span>
</p></div>
<div class="vl"></div>
<div class="alert-list"><span id="alerts">%ALERTS%</span></div>
<div class="footer">
  <p>&copy; 2023. Project by Ayşe Betül Yıldırım, Mevlüt Metin, and Emre Öğütlü.</p>
  <a href="https://github.com/lapinismyname" target=_blank title="Github"><i class="fa-brands fa-github fa-2xl" id="github"></i></a>
</div>
</body>

<script>

setInterval(function () { <!-- TEMP SENSOR -->
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var temp = Math.round(Number(this.responseText));
      document.getElementById("temp").innerHTML = temp;
      
      var thermo = document.getElementsByClassName("fa-solid")[0];

      if (temp < 15 && !thermo.classList.contains("fa-temprature-empty")) {
        thermo.style.color = "#64c8fa";
        thermo.setAttribute("class", "fa-solid fa-temperature-empty fa-xl");
      }
      else if (temp < 28 && !thermo.classList.contains("fa-temprature-half")) {
        thermo.style.color = "#ffd000";
        thermo.setAttribute("class", "fa-solid fa-temperature-half fa-xl");
      }
      else if (temp >= 28 && !thermo.classList.contains("fa-temprature-full")) {
        thermo.style.color = "#eb6600";
        thermo.setAttribute("class", "fa-solid fa-temperature-full fa-xl");
      }
    }
  };
  xhttp.open("GET", "/temp", true);
  xhttp.send();
}, 100) ;

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var hmdty = Math.round(Number(this.responseText));
            document.getElementById("humidity").innerHTML = hmdty;

            var drop = document.getElementsByClassName("fa-solid")[1];

            if (hmdty >= 65 && !drop.classList.contains("fa-cloud-showers-heavy")) {
                drop.style.color = "#969696";
                drop.setAttribute("class", "fa-solid fa-cloud-showers-heavy fa-xl");
            }
            else if (hmdty < 65 && !drop.classList.contains("fa-droplet")) {
                drop.style.color = "#24d3ff";
                drop.setAttribute("class", "fa-solid fa-droplet fa-xl");
            }
        }
    };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
}, 100);

setInterval(function () { <!-- GAS SENSOR -->
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("gas").innerHTML = this.responseText;

      var warn = document.getElementsByClassName("fa-triangle-exclamation")[0];
      var co = Number(this.responseText);

      if (co >= 20 && !warn.classList.contains("fa-fade")) warn.classList.add("fa-fade");
      else if (co < 20 && warn.classList.contains("fa-fade")) warn.classList.remove("fa-fade");
    }
  };
  xhttp.open("GET", "/gas", true);
  xhttp.send();
}, 100) ;

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("firealarm").innerHTML = this.responseText;

      var fire = document.getElementsByClassName("fa-solid")[3];
      var isFire = this.responseText;
      
      if (isFire == "FIRE!" && !fire.classList.contains("fa-fade")) {
        fire.style.color = "#ff0000";
        fire.setAttribute("class", "fa-solid fa-fire fa-xl fa-fade");
      }
      else if (isFire == "No Fire" && fire.classList.contains("fa-fade")) {
        fire.style.color = "#babdb6";
        fire.setAttribute("class", "fa-solid fa-fire-flame-simple fa-xl");
      }
    }
  };
  xhttp.open("GET", "/firealarm", true);
  xhttp.send();
}, 100) ;

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("firesensor").innerHTML = this.responseText;

      var sensor = document.getElementsByClassName("fa-solid")[4];
      var _on = this.responseText;

      if (_on == "System OFF" && sensor.style.color != "#f01414") {
        sensor.style.color = "#f01414";
        if (sensor.classList.contains("fa-moon")) sensor.setAttribute("class", "fa-solid fa-power-off fa-xl");
      }
      else if (_on == "System ON" && sensor.style.color != "#00fa32") {
        sensor.style.color = "#00fa32";
        if (sensor.classList.contains("fa-moon")) sensor.setAttribute("class", "fa-solid fa-power-off fa-xl");
      }
      else if (_on == "System is at sleep" && sensor.style.color != "#0038ff") {
        sensor.style.color = "#0038ff";
        sensor.setAttribute("class", "fa-solid fa-moon fa-xl");
      }
    }
  };
  xhttp.open("GET", "/firesensor", true);
  xhttp.send();
}, 100) ;

setInterval(function () { <!-- WATER SENSOR -->
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("water").innerHTML = this.responseText;
      
      var bottle = document.getElementsByClassName("fa-bottle-water")[0];
      var water = Number(this.responseText);

      if (water <= 0.5 && !(bottle.classList.contains("fa-bounce"))) bottle.classList.add("fa-bounce");
      else if (water > 0.5 && bottle.classList.contains("fa-bounce")) bottle.classList.remove("fa-bounce");
    }
  };
  xhttp.open("GET", "/water", true);
  xhttp.send();
}, 100);

setInterval(function () { <!-- MOTION SENSOR -->
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motion").innerHTML = this.responseText;

      var burglar = document.getElementsByClassName("fa-user-secret")[0];
      var motion = this.responseText;

      if (motion == "Detected!" && !burglar.classList.contains("fa-fade")) burglar.classList.add("fa-fade");
      else if (motion == "NOT detected" && burglar.classList.contains("fa-fade")) burglar.classList.remove("fa-fade");
    }
  };
  xhttp.open("GET", "/motion", true);
  xhttp.send();
}, 100) ;

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var check = document.getElementById("alerts");
      var arrivalText = this.responseText;

      if (check.innerHTML == "%ALERTS%") {
        check.innerHTML = "";
        var ul = document.createElement("ul");
        ul.id = "alertsUl";
        check.appendChild(ul);
        if (!arrivalText) {
          var li = document.createElement("li");
          li.innerHTML = "No available records found.";;
          ul.appendChild(li);
        }
        else {
          var arrivalList = arrivalText.split("-").reverse();
          var len = arrivalList.length;
          var j = 1;
          for (;j<len;j++) {
            var li = document.createElement("li");
            var i = document.createElement("i");
            var text = arrivalList[j];
            if (text[0] == 'F') {
              i.setAttribute("class", "fa-solid fa-fire fa-xl");
              i.style.color = "#ff0000";
            }
            else {
              i.setAttribute("class", "fa-solid fa-user-secret fa-xl");
              i.style.color = "#000";
            }
            li.appendChild(i); 
            li.innerHTML += arrivalList[j];
            ul.appendChild(li);
          }
        }
      }
      else {
        var arrivalList = arrivalText.split("-").reverse();
        var newLen = arrivalList.length;
        var alertsUl = document.getElementById("alertsUl");
        var oldLen = alertsUl.children.length;

        if (newLen - 1 > oldLen) {
          var j = 1;
          var diff = newLen - oldLen;
          for (;j<diff;j++) {
            var li = document.createElement("li");
            var i = document.createElement("i");
            var text = arrivalList[j];
            if (text[0] == 'F') {
              i.setAttribute("class", "fa-solid fa-fire fa-fade fa-xl");
              i.style.color = "#ff0000";
            }
            else {
              i.setAttribute("class", "fa-solid fa-user-secret fa-fade fa-xl");
              i.style.color = "#000";
            }
            li.appendChild(i); 
            li.innerHTML += arrivalList[j];
            li.style.fontWeight = "bold";
            alertsUl.insertBefore(li, alertsUl.firstChild);
    }}}}
  };
    xhttp.open("GET", "/alerts", true);
    xhttp.send();
}, 100);

</script>
</html>
)rawliteral";

#define CHANNEL 1
TaskHandle_t Task1;

typedef struct slave_send {
  uint8_t id;
  uint8_t temp;
  uint8_t humidity;
  uint8_t coPpm;
  uint8_t water;
  uint8_t motion;
  uint8_t istherefire;
  uint8_t firesensoron;
} slave_send;
slave_send info;

/*type 1: relay
type 2: servo
type 3:
type 4: PIR */
typedef struct master_send{
  uint8_t type;
  uint8_t number;
  uint8_t status;
}master_send;
master_send orders;


String tempString;
String humidityString;
String COString;
String waterString;
String motionString;
String istherefireString;
String fireSensorOnString;

String typeString;
String numberString;
String statusString;

uint8_t wasFire = 0;
uint8_t wasVisitors = 0;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;
const int   daylightOffset_sec = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void handle_orders(){
  orders.type = typeString.toInt();
  orders.number = numberString.toInt();
  orders.status = statusString.toInt();
}

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len){
  info.id = *(incomingData);
  if(info.id == 1) tempString = String(*(incomingData+1));
  if(info.id == 1) humidityString = String(*(incomingData+2));
  if(info.id == 1) waterString = String(*(incomingData+4));
  if(info.id == 2) COString = String(*(incomingData+3));
  if(info.id == 2) istherefireString = String(*(incomingData+6));
  if(info.id == 2) fireSensorOnString = String(*(incomingData+7));
  if(info.id == 3) motionString = String(*(incomingData+5));

  //Serial.print(info.id);
  //Serial.println(info.motion);
}
void serverVoiceOrders();
void serverTempHum();
void serverCO();
void serverWater();
void serverMotion();
void serverAlerts();
void Task1code( void * orders_ptr );
/*void handle_structure_values(){
tempString = String(info.temp);
humidityString = String(info.humidity);
COString = String(info.coPpm);
waterString = String(info.water);
motionString = String(info.motion);
istherefireString = String(info.istherefire);
fireSensorOnString = String(info.firesensoron);
}*/
void esp_now_setup(){
  esp_wifi_set_channel(CHANNEL,WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}
void wifi_setup(){
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
  Serial.println(WiFi.localIP());
}

String readAlert();

String processor(const String& var){
  if(var == "TEMP") return tempString;
  if(var == "HUMIDITY") return humidityString;
  
  if(var == "CARBONMO") return COString;

  if(var == "FIREALARM"){
    if(istherefireString.toInt()) return "FIRE!";
    else return "No Fire";
  }
  
  if(var == "FIRESENSOR"){
    if(!fireSensorOnString.toInt()) return "System OFF";
    else if(fireSensorOnString.toInt()==1) return "System is at sleep";
    else if(fireSensorOnString.toInt()==2) return "System ON";
  }

  if(var == "WATER") return waterString;

  if(var == "MOTION"){
    if(motionString.toInt()) return "Detected!";
    else return "NOT detected";
  }

  if (var == "ALERTS") return readAlert();
    
  return String();  
}

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }
  wifi_setup();
  esp_now_setup();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css", "text/css");
  });
  server.on("/logo.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.jpg", "image/jpg");
  });

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, (void*)&orders, 1, &Task1, 0);
  serverVoiceOrders();
  serverTempHum();
  serverCO();
  serverWater();
  serverMotion();
  serverAlerts();
  
  server.begin();
}

void loop() {
/////////////
}

String readAlert() {
  File file2 = SPIFFS.open("/alerts.txt");
  String alerts = "";
  if(!file2){
    Serial.println("Failed to open file for reading");
    return "Unable to display previous alerts";
  }
    
    while(file2.available()) alerts.concat((char)file2.read());

    //Serial.print(alerts);
    file2.close();
    return alerts;
}

void saveAlert(int type) {
  File file = SPIFFS.open("/alerts.txt", FILE_APPEND);
  struct tm timeinfo;
  
  if(!file){
     Serial.println("There was an error opening the file for writing");
     return;
  }
  if (type) file.print("Fire detected in the house on: ");
  else file.print("Motion detected in the house on: ");
  getLocalTime(&timeinfo);
  file.print(&timeinfo, "%A, %B %d %Y %H:%M:%S-");
  
  file.close();
}

void serverVoiceOrders() {
  server.on("/voice", HTTP_POST, [](AsyncWebServerRequest *request){
        
        if (request->hasParam("value1", true)) {
            typeString = request->getParam("value1", true)->value();
            numberString = request->getParam("value2", true)->value();
            statusString = request->getParam("value3", true)->value();
            handle_orders();
        } 
        request->send(200, "text/plain", "Sent successfully");
    });

  server.on("/voice", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "voice message: " + typeString+numberString+statusString);
  });
}

void serverTempHum() {
  server.on("/temp", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("temp", true)){
            tempString = request->getParam("temp", true)->value();
        } 
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", tempString.c_str());
    });

  //////////////////////////////////////////////////////////////////////////

  server.on("/humidity", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("humidity", true)){
            humidityString = request->getParam("humidity", true)->value();
        }
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", humidityString.c_str());
    });
}

void serverCO() {
  server.on("/gas", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("gas", true)){
            COString = request->getParam("gas", true)->value();
        } 
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/gas", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", COString.c_str());
    });
 
  ///////////////////////////////////////////////////////////////////////////////////////////

  server.on("/firealarm", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("fire", true)){
            istherefireString = request->getParam("fire", true)->value();
        } 
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/firealarm", HTTP_GET, [](AsyncWebServerRequest *request){
    if(istherefireString.toInt()){
      request->send_P(200, "text/plain", "FIRE!");
      if (!wasFire) saveAlert(1);
      wasFire = 1;
    }
      
    else {
      wasFire = 0;
      request->send_P(200, "text/plain", "No Fire");
    }
    });

  /////////////////////////////////////////////////////////////////////////////////////////////

  server.on("/firesensor", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("on", true)){
            fireSensorOnString = request->getParam("on", true)->value();
        } 
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/firesensor", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!fireSensorOnString.toInt()) request->send_P(200, "text/plain", "System OFF");
    else if(fireSensorOnString.toInt()==1) request->send_P(200, "text/plain", "System is at sleep");
    else if(fireSensorOnString.toInt()==2) request->send_P(200, "text/plain", "System ON");
    }); 
}

void serverWater() {
  server.on("/water", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("water", true)){
            waterString = request->getParam("water", true)->value();
        }
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/water", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", waterString.c_str());
    });
}

void serverMotion() {
  server.on("/motion", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("motion", true)){
            motionString = request->getParam("motion", true)->value();
        }
        request->send(200, "text/plain", "Sent successfully");
    });
  server.on("/motion", HTTP_GET, [](AsyncWebServerRequest *request){
        if(motionString.toInt()) {
          request->send_P(200, "text/plain", "Detected!");
          if (!wasVisitors) saveAlert(0);
          wasVisitors = 1;
        }
        else if (!motionString.toInt()) {
          wasVisitors = 0;
          request->send_P(200, "text/plain", "NOT detected");
        }
    });
}

void serverAlerts() {
  server.on("/alerts", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", readAlert().c_str());
  });
}

void Task1code( void * orders_ptr ){

typedef struct voice_orders
{   uint8_t taskType ;
    uint8_t taskNumber;
    uint8_t taskStatus;
}voice_orders;
voice_orders task_orders;
uint8_t PIRmac[] = {0xA8,0x42,0xE3,0x56,0xF3,0x1C};
uint8_t RelayServomac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;
peerInfo.channel = 0;  
peerInfo.encrypt = false;
memcpy(peerInfo.peer_addr, PIRmac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }
memcpy(peerInfo.peer_addr, RelayServomac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }
for(;;){
task_orders.taskType = *((uint8_t*)orders_ptr);
task_orders.taskNumber =*(((uint8_t*)orders_ptr)+1);
task_orders.taskStatus = *(((uint8_t*)orders_ptr)+2);

esp_err_t result = esp_now_send(0, (uint8_t *) &task_orders, sizeof(task_orders));
if (result == ESP_OK) {
    //Serial.println("Sent with success");
  }
  else {
    //Serial.println("Error sending the data");
  }
delay(500);
}}
