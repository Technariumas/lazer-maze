#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>//get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>      //comes with Arduino IDE (www.arduino.cc)
#include <Tone.h>

#include <SerialCommand.h>

SerialCommand SCmd;   // The demo SerialCommand object

//*********************************************************************************************
//************ IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODEID        254    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
//#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//*********************************************************************************************

#define SERIAL_BAUD   115200

#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

Tone player;

static union {
	char buff[10];
	struct {
		uint8_t nodeId;
		uint8_t messageId;
		uint16_t batteryVoltage;
		uint8_t calibration;
		uint8_t laserTripped;
	} s;
} packet;

char buff[1];
#define CMD_LASER_ON 1
#define CMD_LASER_OFF 2
#define CMD_CALIBRATE 3
#define CMD_GET_STATUS 4

#define BUTTON_LIGHT 6
#define BUTTON A3
#define LEDS 5
#define SIREN 4

#define ALL 255

void sendCommand(uint8_t to, uint8_t cmd) {
  buff[0] = cmd;
  radio.send(to, buff, 1);
}

void laserControll(uint8_t cmd) {
  char *arg;  
  uint8_t address = ALL;

  arg = SCmd.next();
  
  if (arg != NULL) {
    address=atoi(arg);
  }

  sendCommand(address, cmd);
}

void laserOn() {
  laserControll(CMD_LASER_ON);
}

void laserOff() {
  laserControll(CMD_LASER_OFF);
}

void laserStatus() {
  laserControll(CMD_GET_STATUS);
}

void laserCalibrate() {
  laserControll(CMD_CALIBRATE);
}

void initDetectors() {
  sendCommand(ALL, CMD_LASER_OFF);
  delay(1000);
  sendCommand(ALL, CMD_CALIBRATE);  
  delay(1500);
  sendCommand(ALL, CMD_LASER_ON);
  delay(100);
  sendCommand(ALL, CMD_GET_STATUS);
}

void discover() {
  sendCommand(ALL, CMD_LASER_OFF);
  delay(1000);
  if(radio.receiveDone()){
  }
  
  for(uint8_t i = 1; i <= 20; i++) {
    sendCommand(i, CMD_LASER_ON);
    Serial.print(i, DEC);
    delay(100);
    uint32_t ts = millis();
    uint8_t responseReceived = 0;
    sendCommand(ALL, CMD_LASER_OFF);
    while(millis() - ts < 50 && !responseReceived) {
      if(radio.receiveDone()){
        for (byte i = 0; i < radio.DATALEN; i++) {
          packet.buff[i] = (char)radio.DATA[i];
        }
        responseReceived = 1;
        Serial.print(" sviecia i ");
        Serial.println(packet.s.nodeId, DEC);
      }
    }
    if(!responseReceived) {
      Serial.println(" niekur");
    }
  }
}

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_LIGHT, OUTPUT);
  digitalWrite(BUTTON_LIGHT, HIGH);
  pinMode(LEDS, OUTPUT);
  pinMode(SIREN, OUTPUT);
  
  Serial.begin(SERIAL_BAUD);
  delay(10);

  SCmd.addCommand("ON",laserOn);
  SCmd.addCommand("OFF",laserOff);
  SCmd.addCommand("init",initDetectors);
  SCmd.addCommand("discover", discover);
  SCmd.addCommand("status", laserStatus);
  SCmd.addCommand("calibrate", laserCalibrate);
  
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(0);
  radio.promiscuous(promiscuousMode);
  //radio.setFrequency(919000000); //set frequency to some custom frequency
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
    
#ifdef ENABLE_ATC
  Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
#endif
  player.begin(9);
  player.play(4000);
  delay(200);
  player.stop();

//  sendCommand(ALL, CMD_LASER_ON);
//  delay(100);
  
  initDetectors();
}

byte ackCount=0;
uint32_t packetCount = 0;
uint8_t mode = 1;

#define GAME_TIMEOUT_MS 500000
uint32_t gameStartTs;

void switchMode() {
  if(mode) {
    sendCommand(ALL, CMD_LASER_ON);
    delay(650);
    sendCommand(128, CMD_LASER_OFF);
    gameStartTs = millis();
  } else {
    sendCommand(ALL, CMD_LASER_OFF);
    digitalWrite(SIREN, HIGH);
    delay(50);
    digitalWrite(SIREN, LOW);
    delay(550);
    digitalWrite(SIREN, HIGH);
    delay(50);
    digitalWrite(SIREN, LOW);
    radio.receiveDone();
    sendCommand(128, CMD_LASER_ON);
  }
}


uint8_t isGameTimeout() {
  return (millis() - gameStartTs) > GAME_TIMEOUT_MS; 
}



void loop() {
  SCmd.readSerial();
  if (radio.receiveDone()){
    
    Serial.print("#[");
    Serial.print(++packetCount);
    Serial.print(']');
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    
    for (byte i = 0; i < radio.DATALEN; i++) {
      packet.buff[i] = (char)radio.DATA[i];
    }

    if(mode && packet.s.laserTripped) {
      player.play(4000);
      digitalWrite(SIREN, HIGH);
      delay(100);
      digitalWrite(SIREN, LOW);
      player.stop();
      gameStartTs = millis();      
    }
    
    if(128 == packet.s.nodeId) {
      mode = !mode;
      switchMode();      
    }
        
    Serial.print(packet.s.nodeId);
    Serial.print(" ");

    Serial.print(packet.s.messageId);
    Serial.print(" ");

    Serial.print(packet.s.batteryVoltage);
    Serial.print(" ");

    Serial.print(packet.s.calibration);
    Serial.print(" ");

    Serial.print(packet.s.laserTripped);
        
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    Serial.println();
  }

  if(LOW == digitalRead(BUTTON)) {
    digitalWrite(LEDS, LOW);
    digitalWrite(SIREN, HIGH);
    gameStartTs = millis();      
  } else {
    digitalWrite(SIREN, LOW);
    digitalWrite(LEDS, HIGH);
  }
  
  if(mode && isGameTimeout()){
    mode = 0;
    switchMode();
  }

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
