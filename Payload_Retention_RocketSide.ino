#include <SPI.h>
#include <RH_RF95.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

// LoRa PIN Mapping and Settings
// Set Vin to connect to +5V pin
#define RFM95_G0   2 //must be an interrupt pin. Default is 2. On Arduino Nano pins D2 and D3 are interrupts.
#define RFM95_SCK  13
#define RFM95_MISO 12
#define RFM95_MOSI 11
#define RFM95_CS   10 //also known as the SS or Slave Select pin
#define RFM95_RST  3
#define RFM95_FREQ 433.0 //in MHz
#define RFM95_TP   23 //Transmit Power in dB; 23 is maximum

RH_RF95 rf95(RFM95_CS, RFM95_G0);// radio driver

#define PACKET_SIZE 86

String packet;
const String header = "TRJY";

String send_and_listen(String message)
{
  String out;
  packet = "";
  packet.concat(header);
  packet.concat(message);

  // cast pointer to unsigned character and send
  rf95.send((uint8_t*)packet.c_str(), PACKET_SIZE);

  // wait for send to finish
  rf95.waitPacketSent();

  // check for a response
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = RH_RF95_MAX_MESSAGE_LEN;
  Serial.println("Waiting for reply...");
  delay(10);
  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now. If something in the buffer, proceed
    if (rf95.recv(buf, &len)) // Don't pass by reference, just use RH_RF95_MAX_MESSAGE_LEN -Rachel
    {
      Serial.println("Received something, checking header...");
      out = String((char*)buf);
    }
    else
    {
      out = "ERR_NO_REPLY";
    }
  }
  else
  {
    out = "ERR_NO_REPLY";
  }

  return out;
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

}

void loop()
{
  String message = "nipples";
  String str = send_and_listen(message);
  Serial.println((char*)str.c_str());
  delay(1000); // breathing room
}
