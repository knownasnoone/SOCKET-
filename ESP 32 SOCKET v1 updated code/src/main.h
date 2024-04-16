#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <ESP32Servo.h>
#include <MFRC522.h>


const int servopin = 13;  // servo to mount the ultrasonic sensor 
Servo servo360;
int angle;
const int servopin2=17;
Servo lockServo; // Declare servo object globally
const int trigpin = 25;
const int echopin = 34;
long duration;
float distance;
unsigned long previousMillis = 0;
const long interval = 10;

const int motor1forward = 22;   // d1m1
const int motor1reverse = 1;
const int mot1pwm = 33;  // white

const int motor2forward = 4;    // d1m2
const int motor2reverse = 2;
const int mot2pwm = 32;   // black

const char *ssid = "Pixel 3";
const char *password = "12345678";
bool forwardbutton = false;
bool reversebutton = false;
bool forwardrightbutton = false;
bool forwardleftbutton = false;
bool reverserightbutton = false;
bool stopbutton = false;
bool automodebutton =false;
bool reverseleftbutton =false;
bool isAutoMode = false;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


#define SS_PIN 5
#define RST_PIN 18
MFRC522 rfid(SS_PIN, RST_PIN);
String str = "";
String accessGranted[] = {"12345678", "87654321"}; // Sample RFID tag IDs
int accessGrantedSize = 2;

bool locked = false;
int unlockPos = 0;
int lockPos = 180;
int lockLEDPin = 3;
int unlockLEDPin = 21;



float calcdistance(){
digitalWrite(trigpin,LOW); // the trigger pin is initially set to low
  delayMicroseconds(2);
  digitalWrite(trigpin,HIGH);
  delayMicroseconds(10); // the trigger pin is set to stay high for 10s , as recommended in the datasheet
  digitalWrite(trigpin, LOW);
  duration = pulseIn(echopin, HIGH); // the pulseIn function is used here to determine the duration in which the echopin level has stayed HIGH.
  distance = duration * 0.0343 / 2;  // the distance is found in cm using the duration measured above microseonds (NOTE THAT THE SPEED OF SOUND USED HERE is in cm/microseconds)
  return distance;
}
int lookLeft(){
  servo360.write(170);
  delay(500);
  int distance = calcdistance();
  delay(100);
  servo360.write(115);
  delay(100);
  return calcdistance();
}
int lookRight(){
servo360.write(50);
  delay(500);
  int distance = calcdistance();
  delay(100);
  servo360.write(115);
  return calcdistance();
}
void servoangle(){
 for (angle = 0; angle <= 180; angle++){
  // a for loop is utilised here therefore the value of the variable angle varies from 0 to 360, which is the angle in which the servo has to move
    servo360.write(angle);
    delay(5);}
 for (angle = 180; angle >= 0; angle--)
  { // a for loop is utilised here aswell, therefore the value of the variable angle varies from 360 to 0 which is the angle in which the servo has to move
    servo360.write(angle);
    delay(5);
  }
}
void motspeed(int mot1, int mot2){
 analogWrite(mot1pwm, mot1);
  analogWrite(mot2pwm, mot2);

}
void stop(){
 digitalWrite(motor1forward, LOW);
  digitalWrite(motor1reverse, LOW);
  digitalWrite(motor2forward, LOW);
  digitalWrite(motor2reverse, LOW);

}
void forward(){
digitalWrite(motor1forward, HIGH);
digitalWrite(motor1reverse, LOW);
digitalWrite(motor2forward, HIGH);
digitalWrite(motor2reverse, LOW);
  motspeed(255, 255);
}
void reverse(){
 digitalWrite(motor1forward, LOW);
  digitalWrite(motor1reverse, HIGH);
  digitalWrite(motor2forward, LOW);
  digitalWrite(motor2reverse, HIGH);
  motspeed(255, 255);
}
void forright(){
digitalWrite(motor1forward, HIGH);
  digitalWrite(motor1reverse, LOW);
  digitalWrite(motor2forward, HIGH);
  digitalWrite(motor2reverse, LOW);
  analogWrite(mot1pwm, 125);
  analogWrite(mot2pwm, 255);
}
void forleft(){
digitalWrite(motor1forward, HIGH);
  digitalWrite(motor1reverse, LOW);
  digitalWrite(motor2forward, HIGH);
  digitalWrite(motor2reverse, LOW);
  analogWrite(mot1pwm, 255);
  analogWrite(mot2pwm, 125);
}
void sendButtonState(){
StaticJsonDocument<200> doc;
  doc["forward"] = forwardbutton;
  doc["reverse"] = reversebutton;
  doc["right"] = forwardrightbutton;
  doc["left"] = forwardleftbutton;
  doc["reverseright"] = reverserightbutton;
  doc["reverseleft"] = reverseleftbutton;
  doc["stop"]=stopbutton;
  doc["automode"]= automodebutton;
  String jsonString;
  serializeJson(doc, jsonString);

  ws.textAll(jsonString); // Assuming you have a function like textAll to send data to all connected clients
}
void resetButtonStates(){
  forwardbutton = false;
  reversebutton = false;
  forwardrightbutton = false;
  forwardleftbutton = false;
  reverserightbutton = false;
  reverseleftbutton = false;
  stopbutton=false;
}
void checkAccess(MFRC522::Uid uid)
{
  bool granted = false;
  String cardID = "";

  // Convert UID to a String
  for (int i = 0; i < uid.size; i++) {
    cardID += String(uid.uidByte[i], HEX);
  }

  Serial.print("Checking access for card ID: ");
  Serial.println(cardID);

  // Check if the card ID matches any granted access
  for (int i = 0; i < accessGrantedSize; i++)
  {
    if (accessGranted[i] == cardID)
    {
      Serial.println("Access Granted");
      granted = true;
      if (locked == true)
      {
        lockServo.write(unlockPos);
        locked = false;
      }
      else if (locked == false)
      {
        lockServo.write(lockPos);
        locked = true;
      }
      digitalWrite(lockLEDPin, LOW);
      digitalWrite(unlockLEDPin, HIGH);
      break; // Exit the loop once access is granted
    }
  }

  if (!granted)
  {
    Serial.println("Access Denied");
    digitalWrite(unlockLEDPin, LOW);
    digitalWrite(lockLEDPin, HIGH);
  }
}

void readCard()
{
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
    {
        // Your code here
    }
    rfid.PICC_HaltA();
}
void lockservo(){
      Servo lockServo; // Declare servo object locally
    int locked = 0;  // Declare locked variable locally
}