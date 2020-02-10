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
#define PACKET_SIZE 86

RH_RF95 rf95(RFM95_CS, RFM95_G0);// radio driver
const String header = "TRJY";
String packet = "";

//Step Motor Driver Pin Mappings and Settings
#define STEP_MOTOR_1_STEP 4
#define STEP_MOTOR_1_DIR  5
#define STEP_MOTOR_1_EN   8
#define STEP_MOTOR_2_STEP 6
#define STEP_MOTOR_2_DIR  7
#define STEP_MOTOR_2_EN   9
#define STEPS_PER_REV     200
#define REVS_PER_RECIEVE  10

String cmd = "";
String str = "";

String recieve()
{
  String out = "";
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
      if(out.substring(0,header.length()) == header)
      {
        out = out.substring(header.length());
      }
      else
      {
        out = "ERR_INVALID_HEAD";
      }
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

// clockwise means looking at axle it turns clockwise; would unscrew a nail if true
void turnRevs(int stepPin, int dirPin, bool clockwise, int revolutions)
{
  // Set the spinning direction
  if(clockwise)
  {
    digitalWrite(dirPin, HIGH);
  }
  else
  {
    digitalWrite(dirPin, LOW);
  }
  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < STEPS_PER_REV*revolutions; i++)
  {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(2000); // from tests this is the most accurate time to use
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);
  }
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
  if (!rf95.setFrequency(RFM95_FREQ))
  {
    Serial.println("setFrequency failed");
    delay(100);
  }
  Serial.print("Set Freq to: ");
  Serial.println(RFM95_FREQ);

  // Set transmit power
  rf95.setTxPower(RFM95_TP, false);

  Serial.println("LoRa Successfully Initialized!");
  Serial.println("Initializing Motors...");

  //sets pins for motors
  pinMode(STEP_MOTOR_1_STEP, OUTPUT);
  pinMode(STEP_MOTOR_1_DIR, OUTPUT);
  pinMode(STEP_MOTOR_1_EN, OUTPUT);
  pinMode(STEP_MOTOR_2_STEP, OUTPUT);
  pinMode(STEP_MOTOR_2_DIR, OUTPUT);
  pinMode(STEP_MOTOR_2_EN, OUTPUT);
  
  digitalWrite(STEP_MOTOR_1_EN, HIGH); // disables the step motor drivers until the first command is recieved (active low)
  digitalWrite(STEP_MOTOR_2_EN, HIGH);
  Serial.println("Step Motors Disabled");

  Serial.println("Motors Successfully Initialized!");

  Serial.println("Initialization Completed Successfully!");

}

void loop()
{
  cmd = recieve();
  Serial.print("Recieved message: ");
  Serial.println((char*)cmd.c_str());
  if((cmd == "ERR_NO_REPLY") || (cmd == "ERR_INVALID_HEAD"))
  {
    //unless instructed to change, retain and execute the previous command
  }
  else if(cmd == str)
  {
    str = ""; //send same message twice to stop
  }
  else
  {
    str = cmd; //upon initially receiving a command or receiving a command which is different from the current command being executed, update the command to that which is received
    digitalWrite(STEP_MOTOR_1_EN, LOW); // if the first command, will enable the step motor drivers; harmless for further execution
    digitalWrite(STEP_MOTOR_2_EN, LOW);
    Serial.println("Step Motors Enabled");
  }
  Serial.print("Executing command: ");
  Serial.println((char*)str.c_str());
  if(str == "FORWARD_1")
  {
    turnRevs(STEP_MOTOR_1_STEP, STEP_MOTOR_1_DIR, true, REVS_PER_RECIEVE);
    //delay(4000);
  }
  if(str == "BACK_1")
  {
    turnRevs(STEP_MOTOR_1_STEP, STEP_MOTOR_1_DIR, false, REVS_PER_RECIEVE);
    //delay(4000);
  }
  if(str == "FORWARD_2")
  {
    turnRevs(STEP_MOTOR_2_STEP, STEP_MOTOR_2_DIR, true, REVS_PER_RECIEVE);
    //delay(4000);
  }
  if(str == "BACK_2")
  {
    turnRevs(STEP_MOTOR_2_STEP, STEP_MOTOR_2_DIR, false, REVS_PER_RECIEVE);
    //delay(4000);
  }
  
  delay(1000); // breathing room
}
