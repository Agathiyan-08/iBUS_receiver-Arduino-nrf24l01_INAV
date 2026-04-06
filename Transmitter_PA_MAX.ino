#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);
const byte address[6] = "00001";

uint16_t channels[8];

const int potPins[4] = {A0, A1, A2, A3};
const int switchPins[2] = {4, 5};

unsigned long lastSend = 0;
const uint16_t SEND_INTERVAL = 20000; 

void setup() {
  radio.begin();
  radio.openWritingPipe(address);

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);

  radio.setRetries(5, 15); 
  radio.enableDynamicPayloads();
  radio.stopListening();

  pinMode(switchPins[0], INPUT_PULLUP);
  pinMode(switchPins[1], INPUT_PULLUP);
}

void loop() {
  if (micros() - lastSend >= SEND_INTERVAL) {
    lastSend = micros();

    readInputs();

    bool success = radio.write(&channels, sizeof(channels));
    
    if (!success) {
      radio.flush_tx(); 
    }
  }
}

void readInputs() {
  channels[0] = map(analogRead(A1), 0, 1023, 1000, 2000);
  channels[1] = map(analogRead(A0), 0, 1023, 1000, 2000);
  channels[2] = map(analogRead(A2), 0, 1023, 1000, 2000);
  channels[3] = map(analogRead(A3), 0, 1023, 1000, 2000);

  channels[4] = digitalRead(switchPins[0]) ? 1000 : 2000;
  channels[5] = digitalRead(switchPins[1]) ? 1000 : 2000;

  channels[6] = 1500;
  channels[7] = 1500;
}
