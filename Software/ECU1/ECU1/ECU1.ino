/*

 Project Name Reaction Lights Training Module Project.
 Developed by : projectswithalex.
 Email :projectswithalex@gmail.com
 Github : https://github.com/projectswithalex/Reaction-Lights-Training-Module
 Social Media and stuff : https://linktr.ee/projectswithalex
 

Disclaimer : Code is opensource and can be modified by everyone. If you can improve the code and add new functionality to it i would be happy to get
              in touch with you and update the repository. 
              Also for everything HW related you have a full responsability for your build. I am not a hardware developer and every decision that i have made
              when i build this project was my own understanding over the modules that i have used. Always double check and if you have any questions let me know.
              
              
 Thanks for being here and wish you a good training session with this modules.
*/


#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <VL6180X_WE.h>



/******************************** TRAINING MODE SELECTION ******************************/
#define MasterECU
#define SELBUTTON 14
enum tipeOfTraining_en {
  TRAINING_START = 0,
  TRAINING_SIMPLE = 1,           //GREEN
  TRAINING_ALLONALLOFF,          //BLUE
  TRAINING_RETURNTOMASTER,       //YELLOW
  TRAINING_PARTNERMODE,          //AQUA
  TRAINING_PARTNERMODERACE = 5,  //MAGENTA
  TRAINING_COUNTERMODE = 6,      //WHITE
  TRAINING_END = 6
};
bool training_startTraining_Flag = false;

enum TRAINING_SIMPLE_en {
  TRAINING_SIMPLE_NrOfColorSelection,
  TRAINING_SIMPLE_counterSetSelection
};
TRAINING_SIMPLE_en TRAINING_SIMPLE_selection;
void trainingSimpleMain(void);


enum TRAINING_ALLONALLOFF_en {
  TRAINING_ALLONALLOFF_NrOfColorSelection,
  TRAINING_ALLONALLOFF_counterSetSelection,
  TRAINING_ALLONALLOFF_stopTimeDuration,
};
TRAINING_ALLONALLOFF_en TRAINING_ALLONALLOFF_selection;
void trainingAllOnAllOffMain(void);
uint8_t TRAINING_ALLONALLOFF_sendData = 0;
uint8_t trainingAllOnAllOfActiveColorIndex = 2;  //Blue

enum TRAINING_RETURNTOMASTER_en {
  TRAINING_RETURNTOMASTER_NrOfColorSelection,
  TRAINING_RETURNTOMASTER_counterSetSelection,
  TRAINING_RETURNTOMASTER_stopTimeDuration,
};
TRAINING_RETURNTOMASTER_en TRAINING_RETURNTOMASTER_selection;
void trainingReturnToMasterMain(void);

enum TRAINING_PARTNERMODE_en {
  TRAINING_PARTNERMODE_SelectColor_Player1,
  TRAINING_PARTNERMODE_SelectColor_Player2,
  TRAINING_PARTNERMODE_counterSetSelection,
};
TRAINING_PARTNERMODE_en TRAINING_PARTNERMODE_selection;
void trainingPartnerModeMain(void);

enum TRAINING_PARTNERMODERACE_en {
  TRAINING_PARTNERMODERACE_SelectColor_Player1,
  TRAINING_PARTNERMODERACE_SelectColor_Player2,
  TRAINING_PARTNERMODERACE_counterSetSelection,
  TRAINING_PARTNERMODERACE_stopTimeDuration,
};
TRAINING_PARTNERMODERACE_en TRAINING_PARTNERMODERACE_selection;
void trainingPartnerModeRaceMain(void);


enum TRAINING_COUNTERMODE_en {
  TRAINING_COUNTERMODE_counterSetSelection,
};
TRAINING_COUNTERMODE_en TRAINING_COUNTERMODE_selection;
void trainingCounterModeMain(void);

bool training_tipeOfTrainingFlag = false;
tipeOfTraining_en training_trainingType;

bool training_nrOfColorsFlag = false;
uint8_t training_nrOfColors = 0;

bool training_counterValStopFlag = false;
uint8_t training_counterValStop = 0;

bool training_stopTimeDurationFlag = false;
uint16_t training_stopTimeDuration = 0;

bool training_partnerMode_P1ColorFlag = false;
uint8_t training_partnerMode_P1Color = 0;

bool training_partnerMode_P2ColorFlag = false;
uint8_t training_partnerMode_P2Color = 0;

bool traininig_partnerMode_stopTimeIntervalFlag = false;
uint8_t traininig_partnerMode_stopTimeInterval = 0;

unsigned long debounceDelay = 200;
volatile unsigned long lastDebounce = 0;
volatile uint8_t interruptModeSelection = 0;
volatile bool isbuttonpressed = 0;

ICACHE_RAM_ATTR void handleInterruptSEL() {
  lastDebounce = millis();
  isbuttonpressed = true;
}

bool buttonPushValid(void) {
  //unsigned long start = millis();
  if (isbuttonpressed) {
    if (millis() - lastDebounce > debounceDelay) {
      interruptModeSelection++;
      isbuttonpressed = false;
      lastDebounce = millis();
      Serial.print("interruptModeSelection:");
      Serial.println(interruptModeSelection);
      return true;
    }
  }
  return false;
}

bool TRAINING_counterSetSelectionFunction(void);
bool TRAINING_nrOfColorsFunction(void);
bool TRAINING_stopTimeDurationFunction(void);
bool TRAINING_selectColor_P1(void);
bool TRAINING_selectColor_P2(void);

uint16_t TRAINING_stopTimeDurationFunctionHelper(void) {
  return interruptModeSelection * 500;
}

uint8_t TRAINING_counterSetSelectionFunctionHelper(void) {
  return interruptModeSelection * 5;
}


uint8_t counterExercise = 0;
/******************************** TRAINING MODE SELECTION ******************************/

/********************************ESP NOW COMMUNICATION CODE ******************************/
#define MY_ROLE ESP_NOW_ROLE_COMBO        // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE ESP_NOW_ROLE_COMBO  // set the role of the receiver
/*replaceValueHere*/ #define MY_ECU 1     //ECU number
#define WIFI_CHANNEL 1
#define MACADDRESSSIZE 6                       //Mac address size
#define NO_ECU 0                               //No ecu with the define MY_ECU 0
#define RGBCLEARDELAY 100                      //delay to be used with RGB clear ?TBD
  /*replaceValueHere*/ #define AVAILABLEECU 3  //Nr of ECUs to be used
#define MAXAVAILABLEECU 10                     // I think ESPNOW supports up to 10 devices




  //Receivers ECUS addreses.Add all of them here.

  /*replaceValueHere*/                                                                       //uint8_t receiverAddress1[] = {0xAC, 0x0B, 0xFB, 0xCF, 0xC1, 0x0F};    //  this ECU MAC address ,only for example purposes
  /*replaceValueHere*/ uint8_t receiverAddress2[] = { 0xAC, 0x0B, 0xFB, 0xCF, 0xD8, 0xB1 };  //  ECU 2
/*replaceValueHere*/ uint8_t receiverAddress3[] = { 0xF4, 0xCF, 0xA2, 0x79, 0x23, 0x84 };    //  ECU 3
///*replaceValueHere*/ uint8_t receiverAddress4[] = { 0x4C, 0xEB, 0xD6, 0x62, 0x09, 0x54 };    //  ECU 4

uint8_t receiverECU_Address[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //Placeholder for the receiver address

uint8_t receiverArray[MAXAVAILABLEECU][MACADDRESSSIZE];

struct __attribute__((packed)) dataPacketAlone {
  uint8_t LED_Token;  // Token for activating ECUs
  uint8_t counterExerciseData;
};

struct __attribute__((packed)) dataPacketPartner {
  uint8_t LED_Token_P1;  // Token for activating ECUs
  uint8_t LED_Token_P2;  // Token for activating ECUs
  uint8_t counterExerciseP1;
  uint8_t counterExerciseP2;
};

struct __attribute__((packed)) dataPacketSettings {
  uint8_t training_trainingType;
  uint8_t training_nrOfColors;  
  uint8_t training_counterValStop;
  uint16_t training_stopTimeDuration;
  uint8_t training_partnerMode_P1Color;
  uint8_t training_partnerMode_P2Color;
};


//state in which the ECU can be found
enum transmissionState_en {
  DATARECEIVED_en,
  SENDDATA_en,
  SENDINGDATA_en,
  TRANSMISIONSUCCESFULL_en,
  ONLYRECEIVE_en
};

/*replaceValueHere*/ dataPacketAlone packetAlone = { 1, 0 };  //Package of data to be sent !if not ECU1 set to 0!
transmissionState_en TransmisionStatus = DATARECEIVED_en;                    //Transmision Status

dataPacketPartner packetPartner;
dataPacketSettings packetSettings;


/*replaceValueHere*/ void initReceiverAddress(void) {

  // memcpy(&receiverArray[0], NOECU, 6); //no ECU is allowed to be on 0 position
  // memcpy(&receiverArray[0], receiverAddress1, 6);  //This is my ECU position doesn't need to be filed.
  memcpy(&receiverArray[2], receiverAddress2, 6);
  memcpy(&receiverArray[3], receiverAddress3, 6);
  //memcpy(&receiverArray[4], receiverAddress4, 6);
  //.......
  //and so on until MAXAVAILABLEECU
}


void initESPNOWcomm(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);  // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);          // this function will get called whenever we receive data

  /*replaceValueHere*/  //add peers here or modify the reciverAddress to the right ECUS
  esp_now_add_peer(receiverAddress2, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(receiverAddress3, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);
  // esp_now_add_peer(receiverAddress4, RECEIVER_ROLE, WIFI_CHANNEL, NULL, 0);

  initReceiverAddress();
}
/********************************ESP NOW COMMUNICATION CODE ******************************/







/******************************** RGB LED CODE  ******************************/
//RGB Defines
#define RGBDATAPIN 2
#define RGBLEDNUM 16
#define NUMBEROFCOLORS 7
Adafruit_NeoPixel pixels(RGBLEDNUM, RGBDATAPIN, NEO_GRB + NEO_KHZ800);



//                                            RED   Green  Blue
const uint8_t colors[NUMBEROFCOLORS][3] = { { 150, 0, 0 },        //RED
                                            { 0, 150, 0 },        //GREEN
                                            { 0, 0, 150 },        //BLUE
                                            { 150, 150, 0 },      //YELLOW
                                            { 0, 150, 150 },      //AQUA
                                            { 150, 0, 150 },      //MAGENTA
                                            { 150, 150, 150 } };  //WHITE
uint8_t selectColor = 0;

//Select color next ON cycle
void selectColorNextCycle(void) {
  selectColor++;
  if (selectColor > NUMBEROFCOLORS - 1) {
    selectColor = 0;
  }
}

uint8_t generateRandomColor(void) {
  randomSeed(millis());
  uint8_t returnValue = 0;
  uint8_t randomColor = 0;
  while (returnValue == 0) {
    randomColor = random(0, training_nrOfColors);

    if (training_trainingType == TRAINING_ALLONALLOFF) {
      if (randomColor != trainingAllOnAllOfActiveColorIndex) {  //BLUE is the active LIGHT for ALLONALLOFF
        returnValue = randomColor;
      }
    } else {
      returnValue = randomColor;
    }
  }
  return returnValue;
}

void setRGBcolors(uint8_t colorIndex) {

  for (int i = 0; i < RGBLEDNUM; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]));
    pixels.show();  // Send the updated pixel colors to the hardware.
  }
}

void setRGBColorsNumber(uint8_t number) {
  for (int i = 0; i < number; i++) {  // For each pixel...
    pixels.setPixelColor(i, pixels.Color(colors[training_trainingType][0], colors[training_trainingType][1], colors[training_trainingType][2]));
    pixels.show();  // Send the updated pixel colors to the hardware.
  }
}

void clearRGBcolors() {
  //Serial.println("Clear RGB");
  for (int i = 0; i < RGBLEDNUM; i++) {              // For each pixel...
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));  //clear the RGBs
    pixels.show();                                   // Send the updated pixel colors to the hardware.
  }
}

/******************************** RGB LED CODE  ******************************/


/******************************** BATTERY CHECK CODE  ******************************/
//Battery Check Defines
#define BATMEAS A0       // Analog input pin
int analogVal = 0;       // Analog Output of Sensor
int bat_percentage = 0;  // Battery in percentage

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

void readBatValue(void) {
  analogVal = analogRead(BATMEAS);
  /*replaceValueHere*/ float voltage = (((analogVal * 3.3) / 1024) * 1.54);  //1.54 is the constant for me , check out with the multimeter and set the right value for you (trial&error) until correct
  bat_percentage = mapfloat(voltage, 3, 4.2, 0, 100);                        //Real Value

  if (bat_percentage >= 100) {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0) {
    bat_percentage = 1;
  }
};

void showBatteryPercentage(void) {
  if ((bat_percentage <= 100) && (bat_percentage >= 90))  //Battery Levels between 100%-90% = 16LEDS green
  {
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 90) && (bat_percentage >= 60))  //Battery Levels between 89%-60% = 12LEDS green
  {
    for (int i = 0; i < 12; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 60) && (bat_percentage >= 40))  //Battery Levels between 59%-40% = 8LEDS green
  {
    for (int i = 0; i < 8; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 40) && (bat_percentage >= 20))  //Battery Levels between 39%-20% = 4LEDS green
  {
    for (int i = 0; i < 4; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
  }

  if ((bat_percentage < 20) && (bat_percentage >= 5))  //Battery Levels between 19%-10% = 4LEDS flashing RED leds
  {

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i = i + 3;
    }
    delay(200);
    pixels.clear();
    pixels.show();
    delay(200);
    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(20, 0, 0));
      pixels.show();
      i = i + 3;
    }
  }
  // Lower then 10 percent nothing is displayed. As a safety measure since i don't know how much trust i have in the "BMS" that is going to
  // protect the battery of over-discharge.
};

void initBatteryCheck(void) {
  readBatValue();
  showBatteryPercentage();
}
/******************************** BATTERY CHECK CODE  ******************************/

/******************************** TOF SENSOR CODE  ******************************/
#define VL6180X_ADDRESS 0x29
#define TOF_INT 12
#define SCL_PIN 5
#define SDA_PIN 4

VL6180xIdentification identification;
VL6180x TOFsensor(VL6180X_ADDRESS);

volatile bool intrerruptTOF = false;
volatile int flag = 0;

ICACHE_RAM_ATTR void handleInterruptTOF() {

  intrerruptTOF = true;
  if (counterExercise >= training_counterValStop) {
    flag++;
  }
}


void initTOFSensor(void) {
  pinMode(TOF_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOF_INT), handleInterruptTOF, FALLING);

  delay(500);  //do i really need this here
  while (TOFsensor.VL6180xInit() == VL6180x_FAILURE_RESET) {
    Serial.println("FAILED TO INITALIZE");  //Initialize device and check for errors
  }
  TOFsensor.VL6180xDefautSettings();                         //Load default settings to get started.
  delay(500);                                                //do i really need this here
  /*replaceValueHere*/ TOFsensor.VL6180xSetDistInt(20, 20);  //it detects a movement when it lower than 2cm. With the current initialization should work for values up until 20cm .
  TOFsensor.getDistanceContinously();
  TOFsensor.VL6180xClearInterrupt();
  intrerruptTOF = false;
  delay(500);  //do i really need this here
}
/******************************** TOF SENSOR CODE  ******************************/

//Callback function after a transmission has been sent
void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    TransmisionStatus = TRANSMISIONSUCCESFULL_en;
  } else {
    TransmisionStatus = SENDDATA_en;
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}
//Callback function after a transmission has been received
void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("Received data from: ");
  Serial.println(macStr);

  memcpy(&packetAlone, data, sizeof(packetAlone));


  Serial.print("LED_TOKEN: ");
  Serial.println(data[0]);

  counterExercise = data[1];
  Serial.print("counterExercise: ");
  Serial.println(data[1]);
  training_counterValStopFlag = data[2];
  Serial.print("stopExercise: ");
  Serial.println(data[2]);

  TransmisionStatus = DATARECEIVED_en;
}
/******************************** Logic CODE  ******************************/

uint8_t randomECUSelection = 0;

uint8_t randomECUselect(void) {

  randomSeed(millis());
  uint8_t returnValue = 0;
  uint8_t randomNumber = 0;
  while (returnValue == 0) {
    randomNumber = random(1, AVAILABLEECU + 1);

    if ((randomNumber != MY_ECU) && (randomNumber != NO_ECU)) {
      returnValue = randomNumber;
    }
  }

  return returnValue;
}


void selectECU_number(uint8_t ECU) {
  memcpy(&receiverECU_Address, receiverArray[ECU], MACADDRESSSIZE);
  packetAlone.LED_Token = ECU;
  TransmisionStatus = SENDDATA_en;
}

void endOfTrainingLight(void) {
  for (int i = 0; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(50, 0, 0));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);
  for (int i = 1; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 50, 0));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);
  for (int i = 2; i < RGBLEDNUM; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 50));
    pixels.show();
    i = i + 3;
  }
  delay(200);
  pixels.clear();
  pixels.show();
  delay(200);

  if (intrerruptTOF) {
    Serial.print("Flag:");
    Serial.println(flag);
    intrerruptTOF = false;
    TOFsensor.VL6180xClearInterrupt();
  }
}

void startOfNewTraining(void) {

  unsigned long start = millis();
  while (millis() - start < 3000) {

    Serial.print("Broadcast");
    packetAlone.LED_Token = MY_ECU;
    packetAlone.counterExerciseData = 0;
    esp_now_send(NULL, (uint8_t *)&packetAlone, sizeof(packetAlone));

    pixels.clear();
    pixels.show();

    for (int i = 0; i < RGBLEDNUM; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 50, 0));
      pixels.show();
      delay(50);
    }
    intrerruptTOF = false;
    TOFsensor.VL6180xClearInterrupt();

    TransmisionStatus = DATARECEIVED_en;
    packetAlone.LED_Token = MY_ECU;
    counterExercise = 0;
    flag = 0;
  }
}

void startOptionSelection(void) {
  tipeOfTraining_en lModeSelect;
  uint8_t setCounter = 0;
  uint8_t setColor = 0;
  if (training_startTraining_Flag == false) {
    if (training_tipeOfTrainingFlag == false) {
      buttonPushValid();
      lModeSelect = (tipeOfTraining_en)interruptModeSelection;
      if (lModeSelect > TRAINING_END) {
        lModeSelect = TRAINING_SIMPLE;
        interruptModeSelection = 1;
      }
      setRGBcolors(lModeSelect);
      if (intrerruptTOF == true && lModeSelect != 0) {
        training_trainingType = lModeSelect;
        training_tipeOfTrainingFlag = true;
        interruptModeSelection = 0;
        Serial.print("modeSelect:");
        Serial.println(lModeSelect);
        clearRGBcolors();
      }
    }

    switch (training_trainingType) {
      case TRAINING_SIMPLE:

        switch (TRAINING_SIMPLE_selection) {
          case TRAINING_SIMPLE_NrOfColorSelection:
            if (TRAINING_nrOfColorsFunction()) {
              TRAINING_SIMPLE_selection = TRAINING_SIMPLE_counterSetSelection;
            }
            break;

          case TRAINING_SIMPLE_counterSetSelection:
            if (TRAINING_counterSetSelectionFunction()) {
              training_startTraining_Flag = true;
            }
            break;
        }
        break;  //TRAINING_SIMPLE

      case TRAINING_ALLONALLOFF:
        switch (TRAINING_ALLONALLOFF_selection) {
          case TRAINING_ALLONALLOFF_NrOfColorSelection:
            if (TRAINING_nrOfColorsFunction()) {
              TRAINING_ALLONALLOFF_selection = TRAINING_ALLONALLOFF_counterSetSelection;
            }
            break;

          case TRAINING_ALLONALLOFF_counterSetSelection:
            if (TRAINING_counterSetSelectionFunction()) {
              TRAINING_ALLONALLOFF_selection = TRAINING_ALLONALLOFF_stopTimeDuration;
            }
            break;

          case TRAINING_ALLONALLOFF_stopTimeDuration:
            if (TRAINING_stopTimeDurationFunction()) {
              training_startTraining_Flag = true;
            }
            break;
        }

        break;  //TRAINING_ALLONALLOFF

      case TRAINING_RETURNTOMASTER:
        switch (TRAINING_RETURNTOMASTER_selection) {
          case TRAINING_RETURNTOMASTER_NrOfColorSelection:
            if (TRAINING_nrOfColorsFunction()) {
              TRAINING_RETURNTOMASTER_selection = TRAINING_RETURNTOMASTER_counterSetSelection;
            }
            break;

          case TRAINING_RETURNTOMASTER_counterSetSelection:
            if (TRAINING_counterSetSelectionFunction()) {
              TRAINING_RETURNTOMASTER_selection = TRAINING_RETURNTOMASTER_stopTimeDuration;
            }
            break;

          case TRAINING_RETURNTOMASTER_stopTimeDuration:
            if (TRAINING_stopTimeDurationFunction()) {
              training_startTraining_Flag = true;
            }
            break;
        }
        break;  //TRAINING_RETURNTOMASTER

      case TRAINING_PARTNERMODE:
        switch (TRAINING_PARTNERMODE_selection) {
          case TRAINING_PARTNERMODE_SelectColor_Player1:
            if (TRAINING_selectColor_P1()) {
              TRAINING_PARTNERMODE_selection = TRAINING_PARTNERMODE_SelectColor_Player2;
            }
            break;

          case TRAINING_PARTNERMODE_SelectColor_Player2:
            if (TRAINING_selectColor_P2()) {
              TRAINING_PARTNERMODE_selection = TRAINING_PARTNERMODE_counterSetSelection;
            }
            break;

          case TRAINING_PARTNERMODE_counterSetSelection:
            if (TRAINING_counterSetSelectionFunction()) {
              training_startTraining_Flag = true;
            }
            break;
        }
        break;  //TRAINING_PARTNERMODE

      case TRAINING_PARTNERMODERACE:
        switch (TRAINING_PARTNERMODERACE_selection) {
          case TRAINING_PARTNERMODERACE_SelectColor_Player1:
            if (TRAINING_selectColor_P1()) {
              TRAINING_PARTNERMODERACE_selection = TRAINING_PARTNERMODERACE_SelectColor_Player2;
            }
            break;

          case TRAINING_PARTNERMODERACE_SelectColor_Player2:
            if (TRAINING_selectColor_P2()) {
              TRAINING_PARTNERMODERACE_selection = TRAINING_PARTNERMODERACE_counterSetSelection;
            }
            break;

          case TRAINING_PARTNERMODERACE_counterSetSelection:
            if (TRAINING_counterSetSelectionFunction()) {
              training_startTraining_Flag = TRAINING_PARTNERMODERACE_stopTimeDuration;
            }
            break;

          case TRAINING_PARTNERMODERACE_stopTimeDuration:
            if (TRAINING_stopTimeDurationFunction()) {
              training_startTraining_Flag = true;
            }
            break;
        }
        break;  //TRAINING_PARTNERMODERACE

      case TRAINING_COUNTERMODE:

        break;  //TRAINING_COUNTERMODE
    }
  }
}


/******************************** Logic CODE  ******************************/

void setup() {
  Serial.begin(115200);          // initialize serial port
  Wire.begin(SDA_PIN, SCL_PIN);  //Initialize I2C for VL6180x (TOF Sensor)
  pinMode(BATMEAS, INPUT);       //measure Battery Pin

  pinMode(SELBUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(SELBUTTON), handleInterruptSEL, FALLING);


  Serial.println("pixels");
  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();  // Set all pixel colors to 'off'
  pixels.show();
  Serial.println("initESPNOWcomm");
  initESPNOWcomm();

  Serial.println("initTOFSensor");
  initTOFSensor();
  Serial.println("initBatteryCheck");
  initBatteryCheck();
  Serial.println("almost Ready");
  delay(1000);
  clearRGBcolors();

  Serial.println("Ready");
}

unsigned long timeFlag = 0;
unsigned long countTime = 0;
void loop() {
  if (training_startTraining_Flag == false) {
    startOptionSelection();
  } else {

    switch (training_trainingType) {
      case TRAINING_SIMPLE:
        trainingSimpleMain();
        break;

      case TRAINING_ALLONALLOFF:
        trainingAllOnAllOffMain();
        break;

      case TRAINING_RETURNTOMASTER:
        trainingReturnToMasterMain();
        break;

      case TRAINING_PARTNERMODE:
        trainingPartnerModeMain();
        break;

      case TRAINING_PARTNERMODERACE:
        trainingPartnerModeRaceMain();
        break;
    }
  }
}


void trainingSimpleMain(void) {
  if (counterExercise < training_counterValStopFlag) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(selectColor);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        selectColor = generateRandomColor();
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();

        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
        Serial.print("counter:");
        Serial.println(counterExercise);

      } else {
        //do nothing but wait
      }
    } else {
      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(receiverECU_Address, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {
            intrerruptTOF = false;
            clearRGBcolors();
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
    if (flag < 5) {
      endOfTrainingLight();
    } else {
      startOfNewTraining();
      Serial.print("Reset");
    }
  }
}

void trainingAllOnAllOffMain(void) {
  if (TransmisionStatus == DATARECEIVED_en || TRAINING_ALLONALLOFF_sendData >= (AVAILABLEECU - 1)) {
    countTime = millis();
    while (millis() - countTime < training_stopTimeDuration) {
      clearRGBcolors();
    }
    TRAINING_ALLONALLOFF_sendData = 0;
    TransmisionStatus = ONLYRECEIVE_en;
  }

  if (counterExercise < training_counterValStopFlag) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      selectColor = generateRandomColor();
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(trainingAllOnAllOfActiveColorIndex);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
      } else {
        //do nothing but wait
      }
    } else {
      setRGBcolors(selectColor);
      if (TransmisionStatus == SENDDATA_en) {

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(NULL, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;

      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {

            intrerruptTOF = false;
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
    if (flag < 5) {
      endOfTrainingLight();
    } else {
      startOfNewTraining();
      Serial.print("Reset");
    }
  }
}

void trainingReturnToMasterMain(void) {

  if (counterExercise < training_counterValStopFlag) {
    if (millis() - timeFlag > 500) {
      randomECUSelection = randomECUselect();
      timeFlag = millis();
    }

    if (packetAlone.LED_Token == MY_ECU) {
      setRGBcolors(selectColor);
      //Is the sensor active and the ECU is valid ?
      if (intrerruptTOF) {
        selectColor = generateRandomColor();
        Serial.println("Intrerupt received");
        intrerruptTOF = false;
        selectECU_number(randomECUSelection);
        //delay(RGBCLEARDELAY);  //why did i used this ???
        clearRGBcolors();
        TOFsensor.VL6180xClearInterrupt();
        counterExercise++;
        packetAlone.counterExerciseData = counterExercise;
      } else {
        //do nothing but wait
      }
    } else {
      if (TransmisionStatus == SENDDATA_en) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", receiverECU_Address[0], receiverECU_Address[1], receiverECU_Address[2], receiverECU_Address[3], receiverECU_Address[4], receiverECU_Address[5]);
        Serial.print("sending to:");
        Serial.println(macStr);
        esp_now_send(receiverECU_Address, (uint8_t *)&packetAlone, sizeof(packetAlone));
        TransmisionStatus = SENDINGDATA_en;
      } else {
        if (TransmisionStatus == TRANSMISIONSUCCESFULL_en) {

          Serial.println("Transmision succesful");
          TransmisionStatus = ONLYRECEIVE_en;
          intrerruptTOF = false;
          TOFsensor.VL6180xClearInterrupt();
        } else {
          if (TransmisionStatus == ONLYRECEIVE_en) {
            intrerruptTOF = false;
            clearRGBcolors();
            TOFsensor.VL6180xClearInterrupt();
          }
        }
      }
    }

  } else {
    if (flag < 5) {
      endOfTrainingLight();
    } else {
      startOfNewTraining();
      Serial.print("Reset");
    }
  }
}

void trainingPartnerModeMain(void) {
  //add control Code
}

void trainingPartnerModeRaceMain(void) {
  //add control Code
}

bool TRAINING_counterSetSelectionFunction(void) {
  bool lreturn = false;
  uint8_t setCounter;
  if (training_counterValStopFlag == false) {
    buttonPushValid();
    setCounter = TRAINING_counterSetSelectionFunctionHelper();
    if (setCounter != 0) {
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
    }
    setRGBColorsNumber(interruptModeSelection);
    if (intrerruptTOF == true && setCounter != 0) {
      training_counterValStop = setCounter;
      training_counterValStopFlag = true;
      interruptModeSelection = 0;
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
      Serial.print("training_counterValStop:");
      Serial.println(training_counterValStop);
      clearRGBcolors();
      lreturn = true;
    }
  }
  return lreturn;
}

bool TRAINING_nrOfColorsFunction(void) {
  bool lreturn = false;
  uint8_t setColor;
  if (training_nrOfColorsFlag == false) {
    buttonPushValid();
    setColor = interruptModeSelection;
    if (setColor != 0) {
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
    }
    setRGBColorsNumber(interruptModeSelection);
    if (intrerruptTOF == true && setColor != 0) {
      training_nrOfColors = setColor;
      training_nrOfColorsFlag = true;
      interruptModeSelection = 0;
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
      Serial.print("training_nrOfColors:");
      Serial.println(training_nrOfColors);
      clearRGBcolors();
      lreturn = true;
    }
  }
  return lreturn;
}

bool TRAINING_stopTimeDurationFunction(void) {
  bool lreturn = false;
  uint16_t stopDuration;
  if (training_stopTimeDurationFlag == false) {
    buttonPushValid();
    stopDuration = TRAINING_stopTimeDurationFunctionHelper();
    if (stopDuration != 0) {
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
    }
    setRGBColorsNumber(interruptModeSelection);
    if (intrerruptTOF == true && stopDuration != 0) {
      training_stopTimeDuration = stopDuration;
      training_counterValStopFlag = true;
      interruptModeSelection = 0;
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
      Serial.print("training_counterValStop:");
      Serial.println(training_counterValStop);
      clearRGBcolors();
      lreturn = true;
    }
  }
  return lreturn;
}

bool TRAINING_selectColor_P1(void) {
  bool lreturn = false;
  uint8_t setColor;
  if (training_partnerMode_P1ColorFlag == false) {
    buttonPushValid();
    setColor = interruptModeSelection;
    if (setColor != 0) {
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
    }
    if (setColor > NUMBEROFCOLORS - 1) {
      interruptModeSelection = 1;
    }
    setRGBcolors(interruptModeSelection);
    if (intrerruptTOF == true && setColor != 0) {
      training_partnerMode_P1Color = setColor;
      training_partnerMode_P1ColorFlag = true;
      interruptModeSelection = 0;
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
      Serial.print("training_nrOfColors:");
      Serial.println(training_nrOfColors);
      clearRGBcolors();
      lreturn = true;
    }
  }
  return lreturn;
}

bool TRAINING_selectColor_P2(void) {
  bool lreturn = false;
  uint8_t setColor;
  if (training_partnerMode_P2ColorFlag == false) {
    buttonPushValid();
    setColor = interruptModeSelection;
    if (setColor != 0) {
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
    }
    if (setColor > NUMBEROFCOLORS - 1) {
      interruptModeSelection = 1;
    }
    setRGBcolors(interruptModeSelection);
    if (intrerruptTOF == true && setColor != 0) {
      training_partnerMode_P2Color = setColor;
      training_partnerMode_P2ColorFlag = true;
      interruptModeSelection = 0;
      intrerruptTOF = false;
      TOFsensor.VL6180xClearInterrupt();
      Serial.print("training_nrOfColors:");
      Serial.println(training_nrOfColors);
      clearRGBcolors();
      lreturn = true;
    }
  }
  return lreturn;
}