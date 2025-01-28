// Morse_code.ino
// Author: Alex Chang, 2020
// Modified by: EA5JTT, 2025
// Version: 1.0
// - Adapted to Lilygo ESP32 LoRA T3_V1.6.1
// Limitaciones
/* Según la documentación RH_RF95.h, https://github.com/PaulStoffregen/RadioHead/blob/master/RH_RF95.h
la longitud máxima del mensaje es de 251 octetos, 
esto es uint8_t, pero el programa  usa int 16 para almacenar las matrices (de esta manera se pueden  obtener más de 255 ms de tiempo), 
lo que significa que obtenemos 124 números para enviar nuestro mensaje. De los 124, 
la mitad se usa para comunicar el tiempo de inactividad y la otra mitad es el tiempo de mantener presionado el botón, es decir, 
necesitamos 2 números por pitido, por lo tanto, podemos enviar un máximo de 62 pitidos por mensaje. 
En la práctica son unos 15s de transmisión lo que da para unos 7 caradcteres morse */

// Tonos para sonido
#include"pitches.h"
// Bus SPI (NEcesario para SX12xx)
#include <SPI.h>
//RH_RF95 Works with Semtech SX1276/77/78/79 que son los que monta Lilygo
//https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
#include <RH_RF95.h>

// Adafruit-Feather-32u4-RFM-LoRa-PCB
// #define  RFM95_CS 8
// #define  RFM95_RST 4
// IRQ (Interrupt ReQuest)
// #define  RFM95_INT 7

// SX1278 433
// SX1276 868/915
// CS Chip RST Reset INT interruption
//
// SPI LoRa Radio  LiLYGO Lora T3_v1.4.1
// GPIO18 - SX1278 SCK  -> 18
// GPIO19 - SX1278 MISO -> 19
// GPIO23 - SX1278 MOSI -> 23
// GPIO5  - SX1278 CS   -> 5 ACTIVACION
// GPIO4  - SX1278 RST  -> 4 RESET O PUESTA A CERO
// GPIO26 - SX1276 DIO  -> 26 INTERRUPCION REQUEST IRQ
//
// SPI LoRa Radio  LiLYGO Lora T3_V1.6.1
// GPIO5  - SX1276 SCK CLK -> 5
// GPIO19 - SX1276 MISO -> 19
// GPIO27 - SX1276 MOSI -> 27
// GPIO18 - SX1276 CS NSS -> 18 ACTIVACION
// GPIO14 - SX1276 RST -> 23 RESET O PUESTA A CERO
// GPIO26 - SX1276 DI0  -> 26 


// LILYGO T3 V1.6.1
// https://lilygo.cc/products/lora3
#define  RFM95_CS  18
#define  RFM95_RST 23
#define  RFM95_INT 26
#define  RFM95_MOSI 27
#define  RFM95_MISO 19
#define  RFM95_CLK  5

// Frecuencia de TX y RX LoRa
// Banda Radioaficionados
// #define  RF95_FREQ 915.0
// Banda LoRa Europa
#define RF95_FREQ 886.0

// https://github.com/PaulStoffregen/RadioHead/blob/master/RH_RF95.h 
// https://www.arduino.cc/reference/cs/language/functions/external-interrupts/attachinterrupt/

RH_RF95  rf95(RFM95_CS, RFM95_INT);


// PINOUT placa
const int  buttonPin = 21;    //  Manipulador LiLygo 21
const int  tonepin = 22;      //  Altavoz LiLygo 22

int buttonState = 0;     // current state of the button
int lastButtonState  = 0; // previous state of the button
int startPressed = 0;    // the moment the  button was pressed
int endPressed = 0;      // the moment the button was released

int holdTime = 0;        // how long the button was hold
int idleTime = 0;        //  how long the button was idle

int idleArray[100];     // array to record button  idle times
int holdArray[100];     // array to record button hold times
int i; 
int j;
int k;
int sendArray[200];   // array to store idle/hold times  to be sent
uint8_t buf[251];   //Lora message buffer
int recvArray[124];   
int  pressed = 0;


void setup() {
 //PAra hacer trazas quitar este comentario y los de serial.println
 //Serial.begin(9600); 
 // while (!Serial) {
 //   delay(1);
 // }


// Serial.println(" Inicio SETUP ");
// Serial.println(" RFM95_CS: ");  Serial.println(RFM95_CS);
// Serial.println(" RFM95_RST: "); Serial.println(RFM95_RST);
// Serial.println(" RFM95_INT: "); Serial.println(RFM95_INT);

//  Serial.println(" Inicio SPI ");
  SPI.begin(RFM95_CLK, RFM95_MISO, RFM95_MOSI, RFM95_CS );
  // Inicializacion RFM_95
  // RESET  H (3,3V) a L (0V)
  //Serial.println(" Reset LoRa ");
  pinMode(RFM95_RST, OUTPUT);
  //digitalWrite(RFM95_RST,  HIGH);
  //delay(100);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  // https://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#aa7bee6fb7ad0e0d0ae5e04ed27cfd79d
  // rf95.init() devuelve TRUE si ha ido bien la inicializacion

  while (!rf95.init()) {

    //Serial.println("  Error inicializacion LoRa  ");
    //Serial.println("  Inicio registro ...   ");
    //rf95.printRegisters();
    //Serial.println("  ... fin registro   ");
    // SOS si falla inicializacion
    SoS();
    while (1);
  }
 // Serial.println(" inicializacion LoRa OK  ");

  if (!rf95.setFrequency(RF95_FREQ))  {
    //Serial.println("Error ajustando frecuencia TX");
    SoS();
    while (1);
  }
  //Serial.println(" Ajuste freecuencia OK  ");
  //Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  //Serial.println("FIN INICIALIZACION");
  
  // initialize the button pin as a input
  pinMode(buttonPin, INPUT); 
  // ajuste potencia
  rf95.setTxPower(23,  false);
}

void loop() {
  buttonState = digitalRead(buttonPin);  // read the button input
  updateState();

  /******************************Transmit******************************/
  //Only transmit if it's been 5 seconds (idleTime) since  last button press
  if(idleTime > 5000 && pressed == 1){ 
  pressed = 0; //reset button press
  k = 0;
  idleArray[0]  = 0; //set first delay to zero so the message doesn't start with delay
  for(i=0;i<124;){
      sendArray[i]=idleArray[k]; //Put idle times in even index of sendArray
      i = i + 1; //indexing sendArray
      sendArray[i]=holdArray[k]; //Put  hold times in odd index of sendArray
      i = i + 1;
      k = k + 1; //indexing  idle and hold arrays     
  }
    memcpy(buf,sendArray,sizeof(sendArray));  //copy send array into uint8_t buffer
    //for(i=0;i<200;i++){ //For debug
    //  Serial.println((int)x[i]);
    //}
  

    rf95.send(buf, sizeof(buf));  // Send buffer
    rf95.waitPacketSent(); 
    startPressed = millis(); //reset  counter
    memset(idleArray,0,sizeof(idleArray)); //clear arrays
    memset(holdArray,0,sizeof(holdArray));
    i = 0; 
    j = 0;
    //play tone to indicate  message sent
    tone(tonepin,NOTE_C6); 
    delay(100);
    noTone(tonepin);
    // K es invitación a transmitir, cambio 
    // tone(tonepin,NOTE_C5);
    // delay(300);
    // noTone(tonepin);
    // delay(100);
    // tone(tonepin,NOTE_C5);
    // delay(100);
    // noTone(tonepin);
    // delay(100);
    // tone(tonepin,NOTE_C5);
    // delay(300);
    // noTone(tonepin);
    // delay(100);
  }
  delay(2);
  if(lastButtonState != buttonState && buttonState == 1){
    idleArray[i] =  idleTime; //Record button idletime into array
    //Serial.print(idleArray[i]);
    //Serial.print(" ");
    i++;
  }
 
  if(lastButtonState != buttonState  && buttonState == 0){
    holdArray[j] = holdTime;
    //Serial.println(holdArray[j]);

    j++;
  }
  lastButtonState = buttonState;

  /****************************** Recieve ******************************/
   if (rf95.available())
  {
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf,  &len))
    {
      // Mensaje recibido
      /*
      RH_RF95::printBuffer("Received: ",buf, len);
      Serial.println(*(int*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(),  DEC);
      */

      
      memcpy(recvArray,buf,sizeof(buf)); //copy recieved buffer  into our reciving array
      /*
      for(i = 0; i < 100; i++){
        Serial.println(x[i]);
        Serial.print(idleIn[i]);
        Serial.print(" ");
        Serial.println(holdIn[i]);
      }*/
     for(i = 0; i < 124; i++){ //play the message
        noTone(tonepin);
        delay(recvArray[i]);
        tone(tonepin,NOTE_C5);
        i = i  + 1;
        delay(recvArray[i]);
        noTone(tonepin);
      }
      i = 0;
    }
  }
  
}

void updateState() {
  if (buttonState  == HIGH) {
      startPressed = millis();
      holdTime = startPressed -  endPressed; //time button was held down
      tone(tonepin,NOTE_C5);
      if(idleTime  > 5000){
        idleTime = 0; //if button was pressed after more than 5 seconds  idle, restart 5 second timer
      }
      pressed = 1; //button was pressed
  } 
  if(buttonState == LOW){
    noTone(tonepin);
      endPressed =  millis();
      idleTime = endPressed - startPressed; //time button was idle
    }
}

void SoS() { 
//MENSAJE SOS EN MORSE
//S
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(300);
//O
tone(tonepin,NOTE_C5);
delay(100);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(100);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(100);
noTone(tonepin);
delay(300);
//S
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(100);
tone(tonepin,NOTE_C5);
delay(300);
noTone(tonepin);
delay(700);
}