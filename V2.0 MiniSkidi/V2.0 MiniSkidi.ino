#include <ESP32Servo.h>
#include <DabbleESP32.h>


#define INCLUDE_GAMEPAD_MODULE


int dirPin2 = 33;
int dirPin1 = 32;

int dirPin4 = 25;
int dirPin3 = 26;

int armDirPin = 19;
int armDirPin1 = 21;

int auxDirPin = 5;
int auxDirPin1 = 18;

int x = 175;
int y = 175;
Servo myservo;
Servo myservo2;

void setup() {


  myservo.attach(23);
  myservo.write(x);
  myservo2.attach(22);
  myservo2.write(y);


  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(dirPin3, OUTPUT);
  pinMode(dirPin4, OUTPUT);

  pinMode(armDirPin, OUTPUT);
  pinMode(armDirPin1, OUTPUT);
  pinMode(auxDirPin, OUTPUT);
  pinMode(auxDirPin1, OUTPUT);
  
  Serial.begin(250000);    // Set your Serial Monitor is set at 250000
  Dabble.begin("ProfBoots MiniSkidi");      // This is the baude rate of the HM-10
}

void loop() {
  Dabble.processInput(); { // This line is crucial in grabbing our data

    if (GamePad.isUpPressed())
    {
      Serial.println("Forward");
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      digitalWrite(dirPin3, HIGH);
      digitalWrite(dirPin4, LOW);
    }

    else if (GamePad.isDownPressed())
    {
      Serial.println("Backward");
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, HIGH);
      digitalWrite(dirPin3, LOW);
      digitalWrite(dirPin4, HIGH);
    }
    else if (GamePad.isLeftPressed())
    {
      Serial.println("Left");
      digitalWrite(dirPin3, LOW);
      digitalWrite(dirPin4, HIGH);
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
    }
    else if (GamePad.isRightPressed())
    {
      Serial.println("Right");
      digitalWrite(dirPin3, HIGH);
      digitalWrite(dirPin4, LOW);
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, HIGH);
    }
    else if (GamePad.isStartPressed())
    {
      if(y<178)
      {
      y++;
      myservo2.write(y);
      delay(10);
      }
    }
    else if (GamePad.isSelectPressed())
    {
      if(y>10)
      {
      y--;
      myservo2.write(y);
      delay(10);
      }
    }
    else if (GamePad.isTrianglePressed())
    {
      digitalWrite(armDirPin, HIGH);
      digitalWrite(armDirPin1, LOW);
      }
    else if (GamePad.isCrossPressed())
    {
      digitalWrite(armDirPin, LOW);
      digitalWrite(armDirPin1, HIGH);
      }
    else if (GamePad.isSquarePressed())
    {
      if(x<179)
      {
      x++;
      myservo.write(x);
      delay(10);
      }
    }
    else if (GamePad.isCirclePressed())
    {
      if(x>1)
      {
      x--;
      myservo.write(x);
      delay(10);
      }
    }
    else
    {
      Serial.println("Stopped");
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, LOW);
      digitalWrite(dirPin3, LOW);
      digitalWrite(dirPin4, LOW);
      digitalWrite(armDirPin, LOW);
      digitalWrite(armDirPin1, LOW);
      digitalWrite(auxDirPin, LOW);
      digitalWrite(auxDirPin1, LOW);
      
    }
  }
}
