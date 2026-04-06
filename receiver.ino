#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>

RF24 radio(9, 10);
const byte address[6] = "00001";

SoftwareSerial ibusSerial(3, 4);

uint16_t rfChannels[8];
uint16_t channels[14];

unsigned long lastReceiveTime = 0;
const unsigned long failsafeTimeout = 500;

uint8_t ibusPacket[32];

unsigned long lastIbusSend = 0;
const uint16_t IBUS_INTERVAL = 7000;

const int statusLedPin = 2;
unsigned long lastBlinkTime = 0;
bool ledState = false;

void setup() {
  radio.begin();
  radio.openReadingPipe(0, address);

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(5, 15);
  radio.enableDynamicPayloads();
  radio.startListening();

  ibusSerial.begin(115200);

  pinMode(statusLedPin, OUTPUT);

  initializeIbusPacket();

  for (int i = 0; i < 14; i++) channels[i] = 1500;
}

void loop() {
  receiveData();
  failsafe();
  updateLED();

  if (micros() - lastIbusSend >= IBUS_INTERVAL) {
    lastIbusSend = micros();
    sendIbus();
  }
}

// ---------------- RF RECEIVE ----------------
void receiveData() {
  if (radio.available()) {
    radio.read(&rfChannels, sizeof(rfChannels));

    if (rfChannels[2] >= 900 && rfChannels[2] <= 2100) {
      for (int i = 0; i < 8; i++) {
        channels[i] = rfChannels[i];
      }

      for (int i = 8; i < 14; i++) {
        channels[i] = 1500;
      }

      lastReceiveTime = millis();
    }
  }
}

// ---------------- FAILSAFE ----------------
void failsafe() {
  if (millis() - lastReceiveTime > failsafeTimeout) {
    if (channels[2] > 1000) channels[2] -= 8;

    channels[0] = 1500;
    channels[1] = 1500;
    channels[3] = 1500;
  }
}

// ---------------- LED STATUS ----------------
void updateLED() {
  unsigned long now = millis();

  if (lastReceiveTime == 0) {
    // Never connected → fast blink
    if (now - lastBlinkTime > 800) {
      ledState = !ledState;
      digitalWrite(statusLedPin, ledState);
      lastBlinkTime = now;
    }
  }
  else if (now - lastReceiveTime < failsafeTimeout) {
    // Connected → solid ON
    digitalWrite(statusLedPin, HIGH);
  }
  else {
    // Lost signal → slow blink
    if (now - lastBlinkTime > 500) {
      ledState = !ledState;
      digitalWrite(statusLedPin, ledState);
      lastBlinkTime = now;
    }
  }
}

// ---------------- IBUS ----------------
void initializeIbusPacket() {
  ibusPacket[0] = 0x20;
  ibusPacket[1] = 0x40;
}

void sendIbus() {
  for (int i = 0; i < 14; i++) {
    ibusPacket[2 + i * 2] = channels[i] & 0xFF;
    ibusPacket[3 + i * 2] = channels[i] >> 8;
  }

  uint16_t checksum = 0xFFFF;
  for (int i = 0; i < 30; i++) checksum -= ibusPacket[i];

  ibusPacket[30] = checksum & 0xFF;
  ibusPacket[31] = checksum >> 8;

  ibusSerial.write(ibusPacket, 32);
}