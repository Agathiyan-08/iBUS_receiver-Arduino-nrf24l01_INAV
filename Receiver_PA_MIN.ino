#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// Software Serial for IBUS (connect to flight controller's RX)
SoftwareSerial ibusSerial(3, 4); 

uint16_t channels[14] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

unsigned long lastReceiveTime = 0;
const unsigned long connectionTimeout = 1000;
bool isConnected = false;
const int statusLedPin = 2;

uint8_t ibusPacket[32];

void setup() {
  Serial.begin(115200);
  Serial.println("IBUS Receiver - Starting");
 
  if (!radio.begin()) {
    Serial.println("nRF24L01 initialization FAILED!");
    while (1) {}
  }
  
  Serial.println("nRF24L01 initialized successfully");
  
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(15, 15);
  radio.startListening();

  ibusSerial.begin(115200);

  pinMode(statusLedPin, OUTPUT);
  digitalWrite(statusLedPin, LOW);

  initializeIbusPacket();
  
  Serial.println("IBUS Receiver Ready - Waiting for transmitter");
}

void loop() {
  if (radio.available()) {
    uint16_t receivedChannels[14];
    radio.read(&receivedChannels, sizeof(receivedChannels));
    
    for (int i = 0; i < 14; i++) {
      channels[i] = receivedChannels[i];
    }
    
    lastReceiveTime = millis();
    
    if (!isConnected) {
      isConnected = true;
      Serial.println("Transmitter CONNECTED!");
      digitalWrite(statusLedPin, HIGH);
    }
  }
  
  sendIbusToFC();
  checkConnection();
  
  delay(10);
}

void initializeIbusPacket() {
  ibusPacket[0] = 0x20; 
  ibusPacket[1] = 0x40; 
}

void sendIbusToFC() {
  for (int i = 0; i < 14; i++) {
    ibusPacket[2 + i * 2] = channels[i] & 0xFF;       
    ibusPacket[3 + i * 2] = (channels[i] >> 8) & 0xFF; 
  }

  uint16_t checksum = 0xFFFF;
  for (int i = 0; i < 30; i++) {
    checksum -= ibusPacket[i];
  }
  ibusPacket[30] = checksum & 0xFF;        
  ibusPacket[31] = (checksum >> 8) & 0xFF; 

  ibusSerial.write(ibusPacket, 32);
}

void checkConnection() {
  if (millis() - lastReceiveTime > connectionTimeout) {
    if (isConnected) {
      isConnected = false;
      Serial.println("Transmitter DISCONNECTED!");
      digitalWrite(statusLedPin, LOW);
      
      for (int i = 0; i < 14; i++) {
        channels[i] = 1000; 
      }
    }
  
    static unsigned long lastBlinkTime = 0;
    if (millis() - lastBlinkTime > 500) {
      digitalWrite(statusLedPin, !digitalRead(statusLedPin));
      lastBlinkTime = millis();
    }
  }
}
