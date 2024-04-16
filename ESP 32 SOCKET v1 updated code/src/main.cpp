#include <Arduino.h>
#include <main.h>

// Use an enum for button states
enum ButtonState {
    BUTTON_FORWARD,
    BUTTON_REVERSE,
    BUTTON_RIGHT,
    BUTTON_LEFT,
    BUTTON_REVERSERIGHT,
    BUTTON_REVERSELEFT,
    BUTTON_STOP,
    BUTTON_AUTOMODE,
    BUTTON_COUNT 
};

// Use an array to store button states
bool buttonStates[BUTTON_COUNT] = {false};

void setup()
{
    SPI.begin();
  rfid.PCD_Init();

   servo360.attach(servopin);

  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);

  pinMode(motor1forward, OUTPUT);
  pinMode(motor1reverse, OUTPUT);
  pinMode(mot1pwm, OUTPUT);

  pinMode(motor2forward, OUTPUT);
  pinMode(motor2reverse, OUTPUT);
  pinMode(mot2pwm, OUTPUT);

  pinMode(motor3forward, OUTPUT);
  pinMode(motor3reverse, OUTPUT);
  pinMode(mot3pwm, OUTPUT);

  pinMode(motor4forward, OUTPUT);
  pinMode(motor4reverse, OUTPUT);
  pinMode(mot4pwm, OUTPUT);

  pinMode(lockLEDPin, OUTPUT);
pinMode(unlockLEDPin, OUTPUT);

  SPIFFS.begin();
  Serial.begin(115200); // Initialize serial communication

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); // to make sure css and js files are uploaded statically
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/css", SPIFFS, "/css");
  server.begin();
  Serial.println("HTTP server started");

ws.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT)
    {
        Serial.println("WebSocket client connected");
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("WebSocket client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT)
        {
            String receivedData = String((char *)data);

            Serial.print("Raw data: ");
            for (size_t i = 0; i < len; i++) {
                Serial.print((char)data[i]);
            }
            Serial.println();

            // Parse JSON
            StaticJsonDocument<200> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, receivedData);
            
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }

            Serial.print("Parsed JSON: ");
            serializeJson(jsonDoc, Serial);
            Serial.println(); // Print new line
            
            String command = "";

            if (jsonDoc["command"]) {
                command = jsonDoc["command"].as<String>();
            }

            if (command == "forward") {
                buttonStates[BUTTON_FORWARD] = true;
            } else if (command == "reverse") {
                buttonStates[BUTTON_REVERSE] = true;
            } else if (command == "right") {
                buttonStates[BUTTON_RIGHT] = true;
            } else if (command == "left") {
                buttonStates[BUTTON_LEFT] = true;
            } else if (command == "reverseright") {
                buttonStates[BUTTON_REVERSERIGHT] = true;
            } else if (command == "reverseleft") {
                buttonStates[BUTTON_REVERSELEFT] = true;
            } else if (command == "stop") {
                buttonStates[BUTTON_STOP] = true;
            } else if (command == "automode_on") {
               isAutoMode = true;
            } else if (command == "automode_off") {
        isAutoMode = false;
    }

        }
        else
        {
            Serial.println("Unsupported WebSocket data type");
        }
    }
    else
    {
        Serial.println("Unsupported WebSocket event type");
    }
});


  server.addHandler(&ws);
}



void loop() {
    String str = "";  // RFID tag ID

if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("Card found");

    MFRC522::Uid uid; // Define an object of type MFRC522::Uid

    if (rfid.uid.uidByte[0] != 0 || rfid.uid.uidByte[1] != 0 || rfid.uid.uidByte[2] != 0 || rfid.uid.uidByte[3] != 0) {
        Serial.print("The card's ID number is : ");
        for (int i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();

        if (rfid.PICC_Select(&rfid.uid) == true) {
            checkAccess(rfid.uid);
        }
    }
    rfid.PICC_HaltA();
}




    if (buttonStates[BUTTON_AUTOMODE]) {
        // Auto mode logic
        if (calcdistance() <= 20) {
            stop();
            delay(300);
            reverse();
            delay(400);
            stop();
            delay(300);
            float distanceRight = lookRight();
            delay(300);
            float distanceLeft = lookLeft();
            delay(300);

            if (calcdistance() >= distanceLeft) {
                forright();
                stop();
            } else {
                forleft();
                stop();
            }
        } else {
            forward();
        }
    } else {
        // Manual mode logic
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            distance = calcdistance();
        }

        if (calcdistance() < 20) {
            stop();
        } else if (buttonStates[BUTTON_STOP]) {
            stop();
            delay(500);
        } else if (buttonStates[BUTTON_FORWARD]) {
            forward();
            servo360.write(0);
        } else if (buttonStates[BUTTON_REVERSE]) {
            reverse();
            servo360.write(180);
        } else if (buttonStates[BUTTON_RIGHT]) {
            forright();
            servo360.write(90);
        } else if (buttonStates[BUTTON_LEFT]) {
            forleft();
            servo360.write(90);
        }
    }

    // Reset button states
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttonStates[i] = false;
    }

    checkAccess(rfid.uid);  // Pass the RFID tag ID to checkAccess function
}