// JAL Notes:
// This sketch is customized for Heltec or TTGO ESP32 boards with RFM95/sx1276 LoRa and 128x64 ssd1306 OLED on board.
// Converted this sketch to use ESP32 Classic Bluetooth with Android App 'Serial Bluetooth Terminal'.
// LoRa9X_TX and LoRa9x_RX  RFM95 sx1276
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter/receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// Attention: You must change line 54 to the Bluetooth classic name that you will use with your android serial bluetooth device
// You must switch the R/S Receive or Send switch (digital input GPIO35) depending if you are Sending or Receiving

#include <SPI.h>  //see the code line 'SPI.begin(x, x, x, x);' in the 'void setup()' for changing the spi gpio pin assignments
#include <RH_RF95.h>
#include "BluetoothSerial.h"

#define RFM95_CS 18    //Conventional ESP32 boards use GPIO5
#define RFM95_RST 14  //Conventional ESP32 boards use GPIO33
#define RFM95_INT 26   //Conventional ESP32 boards use GPIO4

BluetoothSerial SerialBT;

// Set up buffer String to store incoming data from serial port
String inData;
//set up 'data' buffer up to 30 characters
char data [30];
//This digital input is a '0' or '1' from a switch marked 'S' or 'R' for determining logic to send or receive code
const int buttonPin = 35;  //  gpio35. Don't use gpio32 or 33 on these boards as they are onboard wired to the RFM95 dio1 dio2
int buttonState = 0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define LED 2  //LED 2 is the ESP32 onboard LED.

// Change to 915.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0  //Heltec/ttgo antenna matched for 868.0 MHz

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  SPI.begin(5, 19, 27, 18);  //sck-gpio5, miso-gpio19, mosi-gpio27, nss/cs-gpio18 for Heltec/TTGO boards

  Serial.begin(115200);
  if (!SerialBT.begin("ESP32ttgo1")) { // The bluetooth name you assign. There is a pair 'ESP32ttgo1' and 'ESP32ttgo2'
    Serial.println("An error occurred initializing Bluetooth");
  }
  delay(100);

  Serial.println("Arduino LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

}

void loop()
{

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  delay(1000);
  if (buttonState == HIGH) {
    // Receive data from others and use esp32rfmreceiver loop below:
    if (rf95.available())
    {
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);

      if (rf95.recv(buf, &len))
      {
        digitalWrite(LED, HIGH);
        RH_RF95::printBuffer("Received: ", buf, len); delay(250);
        // SerialBT.print("Got: ");  delay(150);
        SerialBT.println((char*)buf); delay (250);
        //   SerialBT.print("RSSI: "); delay(150);
        //  SerialBT.println(rf95.lastRssi(), DEC); delay(250);

        // Send a reply
        uint8_t data[] = "Put S/R switch to S to reply";
        rf95.send(data, sizeof(data)); delay(250);
        rf95.waitPacketSent();
        //   SerialBT.println("Sent a reply");
        digitalWrite(LED, LOW);
      }
      else
      {
        SerialBT.println("Receive failed");
      }
    }


  } else
  {
    // enable transmitter esp32rfm95transmitter loop:


    if (SerialBT.available()) {
      char received = SerialBT.read();  //read ascii characters one at a time from bluetooth serial into 'char received'
      inData += received;  //add ascii character bytes 'received' to 'String inData'

      // Process message when new line character is received
      if (received == '\n')  // if string received contains character line feed/new line (\n or ascii 010)
      {

        jim();
      }
    }
  }
}

void jim() {
  inData.toCharArray(data, 30);  //converts 'String inData' into a sequence of characters 'data' array
  SerialBT.print(data);

  Serial.print("Sending "); Serial.println(data);
  digitalWrite(LED, HIGH);

  Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)data, 30);

  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  inData = ""; // Clear inData array to be ready for the next bluetooth serial input

  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);

  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      SerialBT.print("Got reply: ");
      Serial.print("Got reply: ");
      SerialBT.println((char*)buf);
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
}
