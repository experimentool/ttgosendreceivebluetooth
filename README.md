# ttgosendreceivebluetooth
This project involves using the ttgoesp32 development board from Banggood that also has the rfm95 LoRa and spi oled 128x64 onboard.  These boards have been supplied as a pair, with each board programmed differently. One board is programmed as a transmitter and the other as a receiver.
The code supplied by Banggood is very basic and does not have enough comments to make it very useful for development.
I have modified the code and added bluetooth serial to the project so that cell phones with an appropriate serial bluetooth app can be used as the keyboard and display.  A digital button has been added to each device for the transmit and receive functions.  In this way, both development boards use the same code with one caveat.  The only change to each code is a separate bluetooth address for pairing to each cell phone.
In essence, these development boards are now being used as a long range bluetooth text project.
Due to the short packet size of the rfm95 being used, there is a limit of 30 characters for the text.
