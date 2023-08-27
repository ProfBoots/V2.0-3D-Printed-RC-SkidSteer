//make sure to upload with ESP32 Dev Module selected as the board under tools>Board>ESP32 Arduino

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h> //by dvarrel
#elif defined(ESP8266)
#include <ESPAsyncTCP.h> //by dvarrel
#endif
#include <ESPAsyncWebSrv.h> //by dvarrel

#include <ESP32Servo.h> //by Kevin Harrington
#include <iostream>
#include <sstream>

const char* ssid     = "ProfBoots MiniSkidi";

#define bucketServoPin  23

#define auxServoPin 22

Servo bucketServo;
Servo auxServo;
struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {32, 33},  //RIGHT_MOTOR Pins (IN1, IN2)
  {26, 25},  //LEFT_MOTOR  Pins
  {19, 21}, //ARM_MOTOR pins 
};

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define ARMUP 5
#define ARMDOWN 6
#define STOP 0


#define RIGHT_MOTOR 1
#define LEFT_MOTOR 0
#define ARM_MOTOR 2

#define FORWARD 1
#define BACKWARD -1

bool horizontalScreen;//When screen orientation is locked vertically this rotates the D-Pad controls so that forward would now be left.
bool removeArmMomentum = false;



AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=.9, maximum-scale=1, user-scalable=yes">
    <style>
    .arrows {
      font-size:50px;
      color:grey;
    }
    td.button {
      background-color:black;
      border-radius:20%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 20px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    </style>
  
  </head>
  <body class="noselect" align="center" style="background-color:white">
     
          <div class="slidecontainer">
    <label for="powerSwitch" style="font-size: 20px;">HorizontalScreen:</label>
    <input type="checkbox" id="powerSwitch" class="power-switch" onchange='sendButtonInput("Switch", 0)'>
    </div> 
    <h1 style="color: black;text-align:center;">MINISKIDI</h1>
    
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","1")'onmousedown='sendButtonInput("MoveCar","1")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","3")'onmousedown='sendButtonInput("MoveCar","3")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <td class="button"></td>    
        <td class="button" ontouchstart='sendButtonInput("MoveCar","4")'onmousedown='sendButtonInput("MoveCar","4")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","2")'onmousedown='sendButtonInput("MoveCar","2")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
        <td></td>
      </tr>
      <tr/>
      <tr/>
      <tr/><tr/>
      <tr>
        <td class="button" ontouchstart='sendButtonInput("MoveCar","6")'onmousedown='sendButtonInput("MoveCar","6")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <td class="button"></td>    
        <td class="button" ontouchstart='sendButtonInput("MoveCar","5")'onmousedown='sendButtonInput("MoveCar","5")'onmouseup='sendButtonInput("MoveCar","0")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr/>
      <tr/>
      <tr/><tr/>
      <tr>
        <td style="text-align:left;font-size:25px"><b>Bucket:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="10" max="180" value="90" class="slider" id="Bucket" oninput='sendButtonInput("Bucket",value)'>
          </div>
        </td>
      </tr>  
            <tr/>
      <tr/>
      <tr/><tr/> 
      <tr>
        <td style="text-align:left;font-size:25px"><b>AUX:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="10" max="180" value="90" class="slider" id="AUX" oninput='sendButtonInput("AUX",value)'>
          </div>
        </td>
      </tr> 

    </table>
  
    <script>
      var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";      
      var websocketCarInput;
      const auxSlider = document.getElementById('AUX');
      const bucketSlider = document.getElementById('Bucket');
      
      function initCarInputWebSocket() 
      {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onclose   = function(event){setTimeout(initCarInputWebSocket, 2000);};
        websocketCarInput.onmessage = function(event){};        
      }
      
      function sendButtonInput(key, value) 
      {
       var data = key + "," + value;
       websocketCarInput.send(data);
      }
      function handleKeyDown(event) {
        if (event.keyCode === 38) {
            sendButtonInput("MoveCar", "1");
        }
        if (event.keyCode === 40)
        {
          sendButtonInput("MoveCar", "2");
        }
        if (event.keyCode ===37)
        {
          sendButtonInput("MoveCar", "3");
        }
        if (event.keyCode ===39)
        {
          sendButtonInput("MoveCar", "4");
        }
        if (event.keyCode === 87)
        {
          sendButtonInput("MoveCar", "5");
        }
        if (event.keyCode === 83)
        {
          sendButtonInput("MoveCar", "6");
        }
        if(event.keyCode === 69)
        {
          auxSlider.value = parseInt(auxSlider.value) + 5; // You can adjust the increment value as needed
          sendButtonInput("AUX",auxSlider.value);
      // Trigger the 'input' event on the slider to update its value
          auxSlider.dispatchEvent(new Event('input'));
        }
        if(event.keyCode === 68)
        {
          auxSlider.value = parseInt(auxSlider.value) - 5; // You can adjust the increment value as needed
          sendButtonInput("AUX",auxSlider.value);
      // Trigger the 'input' event on the slider to update its value
          auxSlider.dispatchEvent(new Event('input'));
        }
        if(event.keyCode === 81)
        {
          bucketSlider.value = parseInt(bucketSlider.value) + 5; // You can adjust the increment value as needed
          sendButtonInput("Bucket",bucketSlider.value);
      // Trigger the 'input' event on the slider to update its value
          bucketSlider.dispatchEvent(new Event('input'));
        }
        if(event.keyCode === 65)
        {
          bucketSlider.value = parseInt(bucketSlider.value) - 5; // You can adjust the increment value as needed
          sendButtonInput("Bucket",bucketSlider.value);
      // Trigger the 'input' event on the slider to update its value
          bucketSlider.dispatchEvent(new Event('input'));
        }
        }
      function handleKeyUp(event) {
        if (event.keyCode === 37 || event.keyCode === 38 || event.keyCode === 39 || event.keyCode === 40 || event.keyCode === 87 || event.keyCode === 83) {
            sendButtonInput("MoveCar", "0");
        }
    }  
      
  
      window.onload = initCarInputWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });
      document.addEventListener('keydown', handleKeyDown);
      document.addEventListener('keyup', handleKeyUp); 
           
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";


void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    if(removeArmMomentum)
    {
    digitalWrite(motorPins[ARM_MOTOR].pinIN1, HIGH);
    digitalWrite(motorPins[ARM_MOTOR].pinIN2, LOW); 
    delay(10);
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
    delay(5);
    digitalWrite(motorPins[ARM_MOTOR].pinIN1, HIGH);
    digitalWrite(motorPins[ARM_MOTOR].pinIN2, LOW);
    delay(10);  
    removeArmMomentum = false;
    }
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue); 
  if(!(horizontalScreen))
  { 
  switch(inputValue)
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                  
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD); 
      break;
 
    case STOP:
      rotateMotor(ARM_MOTOR, STOP); 
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;

    case ARMUP:
      rotateMotor(ARM_MOTOR, FORWARD);
      break;
      
    case ARMDOWN:
      rotateMotor(ARM_MOTOR, BACKWARD);
      removeArmMomentum = true;
      break; 
      
    default:
      rotateMotor(ARM_MOTOR, STOP);    
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP); 
      break;
  }
  }else {
      switch(inputValue)
  {
     case UP:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                  
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD); 
      break;
 
    case STOP:
      rotateMotor(ARM_MOTOR, STOP); 
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;

    case ARMUP:
      rotateMotor(ARM_MOTOR, FORWARD); 
      break;
      
    case ARMDOWN:
      rotateMotor(ARM_MOTOR, BACKWARD);
      removeArmMomentum = true;
      break; 
      
    default:
      rotateMotor(ARM_MOTOR, STOP);    
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP); 
      break;
  }
  }
}


void bucketTilt(int bucketServoValue)
{
  bucketServo.write(bucketServoValue); 
}
void auxControl(int auxServoValue)
{
  auxServo.write(auxServoValue); 
}


void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(STOP);
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str());     
        if (key == "MoveCar")
        {
          moveCar(valueInt);        
        }
        else if (key == "AUX")
        {
          auxControl(valueInt);
        }
        else if (key == "Bucket")
        {
          bucketTilt(valueInt);        
        }  
        else if (key =="Switch")
        {
          if(!(horizontalScreen))
          {
            horizontalScreen = true;   
          }
          else{
            horizontalScreen = false;
          }
        }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setUpPinModes()
{
      
  for (int i = 0; i < motorPins.size(); i++)
  {   
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  
  }
  moveCar(STOP);
  bucketServo.attach(bucketServoPin);
  auxServo.attach(auxServoPin);
  auxControl(150);
  bucketTilt(140);
}


void setup(void) 
{
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid );
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
      
  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("HTTP server started");

}

void loop() 
{
  wsCarInput.cleanupClients(); 
}
