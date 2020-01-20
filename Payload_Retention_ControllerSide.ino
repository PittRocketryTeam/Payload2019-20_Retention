#include <SPI.h>
#include <RH_RF95.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

// LoRa PIN Mapping and Settings
// Set Vin to connect to +5V pin
#define RFM95_G0    2 //must be an interrupt pin. Default is 2. On Arduino Nano pins D2 and D3 are interrupts.
#define RFM95_SCK   13
#define RFM95_MISO  12
#define RFM95_MOSI  11
#define RFM95_CS    10 //also known as the SS or Slave Select pin
#define RFM95_RST   3
#define RFM95_FREQ  433.0 //in MHz
#define RFM95_TP    23 //Transmit Power in dB; 23 is maximum
#define PACKET_SIZE 86

RH_RF95 rf95(RFM95_CS, RFM95_G0);// radio driver
const String header = "TRJY";
String packet = "";

#define button_pin 6
int buttonState = 0; 

void send_message(String message)
{
  String out;
  packet = "";
  packet.concat(header);
  packet.concat(message);

  // cast pointer to unsigned character and send
  rf95.send((uint8_t*)packet.c_str(), PACKET_SIZE);

  // wait for send to finish
  rf95.waitPacketSent();

}


void setup()
{
  // start serial
  Serial.begin(9600);
  delay(100);
  
  // Start LoRa
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH); //turning on LoRa
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(100);
  
  // init and check radio systems
  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    delay(100);
  }

  Serial.println("LoRa radio init OK!");

  //set transmit frequency
  if (!rf95.setFrequency(RFM95_FREQ))    // Make this a while loop -Rachel
  {
    Serial.println("setFrequency failed");
    delay(100);
  }
  Serial.print("Set Freq to: ");
  Serial.println(RFM95_FREQ);

  // Set transmit power
  rf95.setTxPower(RFM95_TP, false); // how high can this go? -Patrick


  //END OF LORA

  //set button pin to input
  pinMode(button_pin, INPUT);
}

void loop()
{
  String message = "";
  buttonState = digitalRead(button_pin);
  if(buttonState == HIGH) {
    send_message(message);
  }
  delay(10); // breathing room
}
