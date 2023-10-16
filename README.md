# Untitled Project

**[Totally Safe Link for the Project Video](https://drive.google.com/drive/folders/1kgSl9buzDY80vLcEEej2cLBxeEtfbaw_?usp=sharing)**

This project has been implemented by the collaboration of Ayşe Betül Yıldırım, Emre Öğütlü, and Mevlüt Metin.

In this project, which is a groundwork of a smart home system, we used ESP32 WROOM32D microcontrollers because of their features including Wi-Fi connectivity, ESP-NOW
communication, dual-core processor, Bluetooth module, and its low price.
The project contains:
- a temperature-humidity sensor,
- a fire alarm connected to a carbon monoxide detector,
- a load cell and an amplifier as a carboy scale,
- a security system with a motion sensor,
- relays to control the units in the house (e.g. lights, coffeemaker, toaster, etc.)
- a window adjustment system,
- a voice assistant,
- a web-based user interface,
- user-interface-oriented peripheral screens,
- a remote notification system.

## Implementation
### Server

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/95d4c3aa-8385-43eb-b936-a3cd6423b106" height="375px"/>
</p>

The server is a web-based user interface that displays a variety of data including the ambient temperature and humidity, the density of carbon monoxide, whether there is a fire (based on the the density of carbon monoxide), the carbon monoxide system’s state (whether it is on, off, or at sleep), the amount of water on the carboy scale, whether the motion sensor detected motion or not, and the previous alert records.

### Fire Alarm System

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/bc4009e0-ed60-4aa5-96ff-fe6dd0f77d2d" height="450px" />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/fcdd680f-aa8f-4f16-94b5-a54217dd93d2" height="450px"/>
</p>

The fire alarm system consists of 6 items besides ESP32. These are,
- MQ-7 Carbon Monoxide Sensor
- two buttons
- two LEDs
- a buzzer

MQ-7 provides the CO value in ppm’s with two significant figures. As ESP32 reads the
sensor readings, it sends the data to the server via a GET request. If, somehow, ESP32 cannot connect to an internet router, then it transmits the data to other boards via ESP-NOW.

If the CO value is higher than 50 ppm, which is the maximum acceptable value for human health,
the red LED starts blinking, and synchronously buzzer starts beeping. Meanwhile, the second core
of ESP32 sends an e-mail with the quote “Fire detected in the house!”. Utilizing dual-core
feature, the system can communicate with the server and send warning e-mail simultaneously.

One of the buttons sets the system asleep on click, preventing the alarm system from functioning for 5 minutes. The other one, on being pressed for 2 seconds, opens the system if it is closed or vice versa. 

### Person Detection System

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/e22c982b-f70d-4c6c-a977-ec7a2f7131b6" height="450px" />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/12c335d0-f049-4365-bb1e-94a2b4c77cf7" height="450px"/>
</p>

The person detection system consists of 5 items besides ESP32. These are,
- PIR sensor
- a button
- a red LED and a blue LED
- a buzzer

In this system, the PIR sensor (passive infrared sensor) gives logical 1 signal if it detects any motion,
otherwise gives logical 0. As ESP32 reads the sensor readings, it sends the data to the server via
HTTP GET request. If, somehow, ESP32 cannot connect to an internet router, then it transmits
the data to other boards via ESP-NOW. 

If the PIR sensor detects any motion, an e-mail with
the quote “Motion detected in the house!” is sent to a specific mail address, relevant information is
transferred to the website, the red LED starts blinking, and synchronously buzzer starts beeping. In
case of correct password provision, the blue LED blinks, and the alarm system stops beeping. 

At the very beginning of the Person Detection System’s implementation, a quadruple-switch existed to
provide a hard user interface to get the password input. Yet, due to the lack of the number of available pins on ESP32, it was removed. Thus, in the final implementation, there only stood a
button and a voice assistant which can be used to turn off the alarm.

### Water Scale & Temperature-Humidity System

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/c93e2241-71b6-4c4a-bbb5-fdea18791924" width="625px" />
  <p></p>
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/d0ddc1ac-d564-4a35-bb78-7c166b88cc62" width="625px"/>
</p>

The water scale & temperature-humidity system consists of 4 items besides ESP32. These are,

- a load cell
- HX711
- DHT11
- TFT Screen

HX711 converts the analog signal to digital which comes from the load cell and amplifies it
whereas DHT11 senses the ambient temperature and humidity.
As ESP32 reads the sensor readings, it sends the data to the server via HTTP-GET request. If
ESP32 could not connect to an internet router, then it transmits the data to other boards via ESPNOW. The accumulated data is displayed on the TFT screen as well.

### Relay & Servo Motor System

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/0520bd4d-dcd5-47f1-85e4-1da65560c7ab" height="450px" />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/b4a41aa2-a8b4-4b9c-be43-a457fd2844d8" height="450px"/>
</p>

The relay & servo motor system consists of 3 items besides ESP32. These are,

- 2 servo motors
- 1 relay module with 4 relays on it

This system is basically a voice-controlled house environment. It operates only in the presence
of an internet connection. The servo motors simulate the windows while the relay is used for
electrical devices in the house such as lamps, coffee machines, or air conditioners.
Python-based voice assistant collects user commands (like window movement or turning on the
lights) and compares the command (in string form) with pre-defined commands inside the
Python lists. If it finds a match for the command, it sends the related “Command Code” to the
server via HTTP-POST request. The ESP receives the command code via HTTP-GET request
and executes the commands.

### TFT Display

<p float="left">
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/2c3d947a-252c-490b-b513-b89291e3817c" height="300px"/><p></p>
  <img src="https://github.com/lapinismyname/Untitled-project/assets/92723777/c1441ee7-ea31-4d18-8088-a5d89ef1e3fd" width="900px"/>
</p>

The TFT display UI consists of 2 items besides ESP32. These are,
- a 1.8-inch TFT display
- a button

ESP gets its data from the server via HTTP-GET request in the presence of an internet connection.
If the connection is broken it receives the data delivered by other systems via ESP-NOW.

There are 6 different pages on the TFT screen which can be changed by clicking the button.
These pages are,
- A Screensaver
- Temperature
- Humidity
- Water level
- Person detection status
- Carbon monoxide density

### Voice Assistant 

The Python/Speech Recognition-based voice assistant has two functions, it sends the
commands for the Relay & Servo system and checks the PIR password whether it is true or not.
For the commands, we constituted a 3-digit “Command Code System”.

The first digit stands for type information,
- 0 means no message,
- 1 means the target of the command is servo motors,
- 2 means the target of the command is the relay module,
- 4 means the PIR password is correct regardless of other digits.

The second digit stands for the ID number in order to decide between the same types,
- 1 means the first element among the same types,
- 2 means the second element among the same types,
and so on.

The third and final digit stands for status change information,
- 0 means the target device must be off
- 1 means the target device must be on

For instance, assume the user said, “Turn on the second LED”. The algorithm generates
“121” code. Like that, every command code has its own command words pre-defined inside a
Python list with other variations of them. Since all of them are in lists, it is rather easy to add
new variations as well.
