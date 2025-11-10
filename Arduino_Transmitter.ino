#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

uint16_t channels[14] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

const int potPins[6] = {A0, A1, A2, A3, A4, A5}; // inputs 

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

  pinMode(2, INPUT_PULLUP); // Switch 1
  pinMode(3, INPUT_PULLUP); // Switch 2
  pinMode(4, INPUT_PULLUP); // Switch 3
  
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
    Serial.print(" Ch4(Yaw):"); Serial.println(channels[3]);
    lastPrintTime = millis();
  }
  
  delay(20); 
}

void readInputs() {
  channels[0] = map(analogRead(potPins[1]), 0, 1023, 1000, 2000); // Roll (A0)
  channels[1] = map(analogRead(potPins[0]), 0, 1023, 1000, 2000); // Pitch (A1)
  channels[2] = map(analogRead(potPins[2]), 0, 1023, 1000, 2000); // Throttle (A2)
  channels[3] = map(analogRead(potPins[3]), 0, 1023, 1000, 2000); // Yaw (A3)

  channels[4] = digitalRead(2) ? 1000 : 2000; // Switch 1
  channels[5] = digitalRead(3) ? 1000 : 2000; // Switch 2
  channels[6] = digitalRead(4) ? 1000 : 2000; // Switch 3
  
  for (int i = 7; i < 14; i++) {
    channels[i] = 1500;
  }
}
