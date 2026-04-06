#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

uint16_t channels[14] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

const int potPins[6] = {A0, A1, A2, A3, A4, A5}; 
const int switchPin1 = 4;  
const int switchPin2 = 5;  

unsigned long lastSendTime = 0;
bool connectionLedState = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Transmitter - Starting");

  if (!radio.begin()) {
    Serial.println("nRF24L01 initialization FAILED!");
    while (1) {}
  }
  
  Serial.println("nRF24L01 initialized successfully");
  
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(15, 15);
  radio.stopListening();

  for (int i = 0; i < 6; i++) {
    pinMode(potPins[i], INPUT);
  }

  pinMode(switchPin1, INPUT_PULLUP); 
  pinMode(switchPin2, INPUT_PULLUP); 
  
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Transmitter Ready");
}

void loop() {
  readInputs();
  bool success = radio.write(&channels, sizeof(channels));
  
  if (success) {
    if (millis() - lastSendTime > 500) {
      connectionLedState = !connectionLedState;
      digitalWrite(LED_BUILTIN, connectionLedState);
      lastSendTime = millis();
    }
  } else {
    Serial.println("Send failed!");
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 500) {
    Serial.print("Ch1(Roll):"); Serial.print(channels[0]);
    Serial.print(" Ch2(Pitch):"); Serial.print(channels[1]);
    Serial.print(" Ch3(Throttle):"); Serial.print(channels[2]);
    Serial.print(" Ch4(Yaw):"); Serial.print(channels[3]);
    Serial.print(" Ch5(SW1):"); Serial.print(channels[4]);
    Serial.print(" Ch6(SW2):"); Serial.println(channels[5]);
    lastPrintTime = millis();
  }
  
  delay(20); 
}

void readInputs() {
  channels[0] = map(analogRead(potPins[1]), 0, 1023, 1000, 2000); // Roll (A1)
  channels[1] = map(analogRead(potPins[0]), 0, 1023, 1000, 2000); // Pitch (A0)
  channels[2] = map(analogRead(potPins[2]), 0, 1023, 1000, 2000); // Throttle (A2)
  channels[3] = map(analogRead(potPins[3]), 0, 1023, 1000, 2000); // Yaw (A3)

  channels[4] = digitalRead(switchPin1) ? 1000 : 2000; 
  channels[5] = digitalRead(switchPin2) ? 1000 : 2000; 

  for (int i = 6; i < 14; i++) { 
    channels[i] = 1500;
  }
}
