#include <Arduino.h>

//library RTC
#include <Wire.h>
#include "RTClib.h"

//library Json
#include <ArduinoJson.h>

//library NRF24L01
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include "printf.h"

//konfigurasi us-100
//sumbu X
#define TRIGGER1 33
#define ECHO1    25
//sumbu Y
#define TRIGGER2 32
#define ECHO2 35
//sumbu Z
#define TRIGGER3 27
#define ECHO3    26

//konfigurasi stack size
SET_LOOP_TASK_STACK_SIZE(64*1024); // 64KB

//konfigurasi RTC
RTC_DS3231 rtc;
char days[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//konfigurasi NRF24L01
RF24 radio(4, 5); //(pin CE, pin CSN)
RF24Network network(radio);      // Network uses that radio
RF24Mesh mesh(radio, network);
uint8_t dataBuffer[MAX_PAYLOAD_SIZE];  //MAX_PAYLOAD_SIZE is defined in RF24Network_config.h

//alamat node
#define this_node 5

//variabel DATA
int node_asal = 5;
unsigned long usX = 2; // data us sumbu X
unsigned long usY = 4; // data us sumbu Y
unsigned long usZ = 6; // data us sumbu Z
String datakirim;

//variabel millis
unsigned long currentMillis = 0;

//Fungsi untuk 2 loop
//TaskHandle_t Task1;

//program loop 2
//void loop2( void * parameter) {
//  for (;;) {
//    unsigned long currentTime = millis(); // Waktu saat ini
//
//    if (currentTime - previousTime >= intervalmillis) {
//      previousTime = currentTime; // Perbarui waktu sebelumnya
//      Serial.println("MODE : SCANNING......");
//      BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//      Serial.print("RSSI NODE 1 : " + String(NODE_1_RSSI));
//      Serial.println(" || RSSI NODE 2 : " + String(NODE_2_RSSI));
//    }
//  }
//}

void setup() {
  Serial.begin(115200);

  pinMode(TRIGGER1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIGGER2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(TRIGGER3, OUTPUT);
  pinMode(ECHO3, INPUT);

  //Fungsi untuk 2 loop
  //  xTaskCreatePinnedToCore(
  //    loop2,
  //    "BLE_SCANNING",
  //    1000,
  //    NULL,
  //    1,
  //    &Task1,
  //    0);

  if (! rtc.begin()) {
    Serial.println("Tidak dapat menemukan RTC! Periksa sirkuit.");
    while (1);
  }

  while (!Serial) {
    // some boards need this because of native USB capability
  }
  mesh.setNodeID(this_node);
  Serial.println(F("Menghubungkan ke jaringan..."));

  if (!mesh.begin()){
    if (radio.isChipConnected()){
      do {
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
        Serial.println(F("Gagal terhubung ke jaringan.\nMenghubungkan ke jaringan..."));
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println(F("NRF24L01 tidak merespon."));
      while (1) {
        // hold in an infinite loop
      }
    }
  }
  printf_begin();
  radio.printDetails();  // print detail konfigurasi NRF24L01

  // print memori stack keseluruhan
  Serial.printf("ESP32 Stack was set to %d bytes", getArduinoLoopTaskStackSize());
  // print sisa memori stack pada void setup
  Serial.printf("\nSetup() - Free Stack Space: %d", uxTaskGetStackHighWaterMark(NULL));
}

void loop() {
  // print sisa memori stack pada void loop
  Serial.printf("\nLoop() - Free Stack Space: %d", uxTaskGetStackHighWaterMark(NULL));
  
  mesh.update();
  DateTime now = rtc.now();
  StaticJsonDocument<128> doc;

  // Mengirim data ke master
  if (millis() - currentMillis >= 1000) {
    currentMillis = millis();
    
    //sumbu X
    long duration1;
    long distance1;
    digitalWrite(TRIGGER1, LOW);
    delayMicroseconds(2); 
    digitalWrite(TRIGGER1, HIGH);
    delayMicroseconds(10); 
    digitalWrite(TRIGGER1, LOW);
    duration1 = pulseIn(ECHO1, HIGH);
    distance1 = duration1 * 0.34 / 2;
    usX = distance1;

    //sumbu Y
    long duration2;
    long distance2;
    digitalWrite(TRIGGER2, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER2, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER2, LOW);
    duration2 = pulseIn(ECHO2, HIGH);
    distance2 = duration2 * 0.34 / 2;
    usY = distance2;

    //sumbu Z
    long duration3;
    long distance3;
    digitalWrite(TRIGGER3, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER3, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER3, LOW);
    duration3 = pulseIn(ECHO3, HIGH);
    distance3 = duration3 * 0.34 / 2;
    usZ = distance3;

    doc["NodeID"] = String(node_asal);
    doc["usX"] = String(usX);
    doc["usY"] = String(usY);
    doc["usZ"] = String(usZ);
    doc["Unixtime"] = String(now.unixtime());
    datakirim = "";
    serializeJson(doc, datakirim);
    char kirim_loop[datakirim.length() + 1];
    datakirim.toCharArray(kirim_loop, sizeof(kirim_loop));

    if (!mesh.write(&kirim_loop, '5', sizeof(kirim_loop))) {
      if (!mesh.checkConnection()) {
        Serial.println("Memperbaharui Alamat");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          mesh.begin();
        }
      } else {
        Serial.println("Gagal Mengirim ke Master, Tes jaringan OK");
      }
    } else {
      Serial.print("Berhasil Mengirim ke Master : ");
      Serial.println(datakirim);
    }
  }
}
