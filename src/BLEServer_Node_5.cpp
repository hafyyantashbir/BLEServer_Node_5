#include <Arduino.h>

//library RTC
// #include <Wire.h>
// #include "RTClib.h"

//library Json
#include <ArduinoJson.h>

//library NRF24L01
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "printf.h"

//library BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertisedDevice.h>

//konfigurasi BLE
// RTC_DS3231 rtc;
// char days[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//konfigurasi BLE
#define SERVICE_UUID "0268a775-7fcf-4ff8-8c71-0f5294a48f83"
#define CHARACTERISTIC_UUID "4b0f99fa-0c13-4b24-a19b-fbb87d3f8c89"

//konfigurasi NRF24L01
RF24 radio(4, 5); //(pin CE, pin CSN)
RF24Network network(radio);      // Network uses that radio
uint8_t dataBuffer[MAX_PAYLOAD_SIZE];
#define LED_BUILTIN 2

//alamat node
const uint16_t this_node = 00;   // alamat node ini (NODE_5) dalam format Octal (master harus 00)
const uint16_t NODE_1 = 01;  // Alamat NODE_1 dalam format Octal
const uint16_t NODE_2 = 02; // Alamat NODE_2 dalam format Octal
const uint16_t NODE_3 = 03; // Alamat NODE_3 dalam format Octal
const uint16_t NODE_4 = 04; // Alamat NODE_4 dalam format Octal

//variabel DATA
int node_asal = 5;
int pesan = 1;
String datakirim;
String dataterima;
int count = 0;
int jumlahnode[5];

//variabel millis
unsigned long previousTime = 0; // Waktu sebelumnya
unsigned long intervalmillis = 10000; // Interval waktu (dalam milidetik)

//variabel RSSI node
int NODE_1_RSSI = 0;
int NODE_2_RSSI = 0;
int NODE_3_RSSI = 0;
int NODE_4_RSSI = 0;

//variabel case
int pilihan = 1;

//variabel BLE
int scanTime = 1; //In seconds

//Fungsi untuk 2 loop
//TaskHandle_t Task1;

//konfigurasi fungsi scan RSSI BLE
BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.getName() == "NODE_1")
      {
        NODE_1_RSSI = advertisedDevice.getRSSI();
      }
      if (advertisedDevice.getName() == "NODE_2")
      {
        NODE_2_RSSI = advertisedDevice.getRSSI();
      }
      if (advertisedDevice.getName() == "NODE_3")
      {
        NODE_3_RSSI = advertisedDevice.getRSSI();
      }
      if (advertisedDevice.getName() == "NODE_4")
      {
        NODE_4_RSSI = advertisedDevice.getRSSI();
      }
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

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
  StaticJsonDocument<512> doc; // buat document Json

  JsonArray jsonarray = doc.to<JsonArray>();
  JsonObject jsonobject = jsonarray.createNestedObject();
  jsonobject["NodeID"] = node_asal;
  jsonobject["Pesan"] = pesan;
  serializeJson(doc, datakirim);

  //Fungsi untuk 2 loop
  //  xTaskCreatePinnedToCore(
  //    loop2,
  //    "BLE_SCANNING",
  //    1000,
  //    NULL,
  //    1,
  //    &Task1,
  //    0);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // if (! rtc.begin()) {
  //   Serial.println("Could not find RTC! Check circuit.");
  //   while (1);
  // }
  // DateTime now = rtc.now();
  
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  Serial.println(F("RF24Network/examples/helloworld_rx/"));
  printf_begin();  // needed for RF24* libs' internal printf() calls

  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {
      // hold in infinite loop
    }
  }
  radio.setChannel(90);
  network.begin(/*node address*/ this_node);
  radio.printDetails();

  //fungsi setup BLE
  BLEDevice::init("NODE_5");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer ->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello this is NODE 5");
  pService->start();
  BLEDevice::startAdvertising();
  Serial.println("BLE READY!!!");

  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  //scan BLE
  Serial.println("SCANNING......");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("RSSI NODE 1 : " + String(NODE_1_RSSI));
  Serial.print(" || RSSI NODE 2 : " + String(NODE_2_RSSI));
  Serial.print(" || RSSI NODE 3 : " + String(NODE_3_RSSI));
  Serial.println(" || RSSI NODE 4 : " + String(NODE_4_RSSI));

  
  Serial.println("Tunggu 5 detik");
  Serial.println(datakirim);
  delay(5000);
  char kirim_setup[datakirim.length() + 1];
  datakirim.toCharArray(kirim_setup,sizeof(kirim_setup));
  
    if (NODE_1_RSSI <= NODE_2_RSSI && NODE_1_RSSI <= NODE_3_RSSI && NODE_1_RSSI <= NODE_4_RSSI) {
      Serial.println("Jalan");
      RF24NetworkHeader header(/*to node*/ NODE_1);
      bool NODE_1 = network.write(header, &kirim_setup, sizeof(kirim_setup));
      Serial.println(NODE_1 ? F("DATA TERKIRIM KE NODE 1") : F("GAGAL  TERKIRIM KE NODE 1"));
      if(!NODE_1){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }else{
        // //Serial.println("Unixtime : "+String(now.unixtime()));
      }
      digitalWrite(LED_BUILTIN, LOW);
    }else if (NODE_2_RSSI <= NODE_1_RSSI && NODE_2_RSSI <= NODE_3_RSSI && NODE_2_RSSI <= NODE_4_RSSI) {
      RF24NetworkHeader header(/*to node*/ NODE_2);
      bool NODE_2 = network.write(header, &kirim_setup, sizeof(kirim_setup));
      Serial.println(NODE_2 ? F("DATA TERKIRIM KE NODE 2") : F("GAGAL  TERKIRIM KE NODE 2"));
      if(!NODE_2){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }else{
        // //Serial.println("Unixtime : "+String(now.unixtime()));
      }
      digitalWrite(LED_BUILTIN, LOW);
    }else if (NODE_3_RSSI <= NODE_1_RSSI && NODE_3_RSSI <= NODE_2_RSSI && NODE_3_RSSI <= NODE_4_RSSI) {
      RF24NetworkHeader header(/*to node*/ NODE_3);
      bool NODE_3 = network.write(header, &kirim_setup, sizeof(kirim_setup));
      Serial.println(NODE_3 ? F("DATA TERKIRIM KE NODE 3") : F("GAGAL  TERKIRIM KE NODE 3"));
      if(!NODE_3){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }else{
        // //Serial.println("Unixtime : "+String(now.unixtime()));
      }
      digitalWrite(LED_BUILTIN, LOW);
    }else if (NODE_4_RSSI <= NODE_1_RSSI && NODE_4_RSSI <= NODE_2_RSSI && NODE_4_RSSI <= NODE_3_RSSI) {
      RF24NetworkHeader header(/*to node*/ NODE_4);
      bool NODE_4 = network.write(header, &kirim_setup, sizeof(kirim_setup));
      Serial.println(NODE_4 ? F("DATA TERKIRIM KE NODE 4") : F("GAGAL TERKIRIM KE NODE 4"));
      if(!NODE_4){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
      }else{
        // //Serial.println("Unixtime : "+String(now.unixtime()));
      }
      digitalWrite(LED_BUILTIN, LOW);
    }


}

void loop() {
  network.update();  // Check the network regularly
  // DateTime now = rtc.now();
  StaticJsonDocument<512> doc;

  //scan ble
  unsigned long currentTime = millis(); // Waktu saat ini

  // if (currentTime - previousTime >= intervalmillis) {
  //   previousTime = currentTime; // Perbarui waktu sebelumnya
  //   Serial.println("MODE : SCANNING......");
  //   BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  //   Serial.print("RSSI NODE 1 : " + String(NODE_1_RSSI));
  //   Serial.print(" || RSSI NODE 2 : " + String(NODE_2_RSSI));
  //   Serial.print(" || RSSI NODE 3 : " + String(NODE_3_RSSI));
  //   Serial.println(" || RSSI NODE 4 : " + String(NODE_4_RSSI));
  // }

  while (network.available()) {
    RF24NetworkHeader header;  // If so, grab it and print it out
    uint16_t payloadSize = network.peek(header);
    char terima_loop[payloadSize];
    network.read(header, &terima_loop, payloadSize);
    dataterima = "";
    for (uint32_t i = 0; i < payloadSize; i++){
      dataterima += terima_loop[i];
    }
    DeserializationError error = deserializeJson(doc, dataterima);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    for (int i = 0; i <= 4; i++) {
      jumlahnode[i] = NULL;
      count = 0;
    }
    for (int i = 0; i <= 4; i++) {
      JsonObject parsing = doc[i];
      int NodeID = parsing["NodeID"];
      if (NodeID != 0) {
        count++;
        jumlahnode[i] += NodeID;
      }
    }

    //doc Array index 0
    JsonObject parsing_0 = doc[0];
    int NodeID_0 = parsing_0["NodeID"];
    if (NodeID_0 == 1) {
      int Suhu_0 = parsing_0["Suhu"];
      int Kelembapan_0 = parsing_0["Kelembapan"];
      uint32_t Unixtime1_0 = parsing_0["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_0) + " Suhu : " + String(Suhu_0) + " Kelembapan : " + String(Kelembapan_0) + " Timestamp : " + String(Unixtime1_0));
    } else if (NodeID_0 == 2) {
      int Berat_0 = parsing_0["Berat"];
      uint32_t Unixtime2_0 = parsing_0["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_0) + " Berat : " + String(Berat_0) + " Timestamp : " + String(Unixtime2_0));
    } else if (NodeID_0 == 3) {
      int usX_0 = parsing_0["usX"];
      int usY_0 = parsing_0["usY"];
      int usZ_0 = parsing_0["usZ"];
      uint32_t Unixtime3_0 = parsing_0["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_0) + " usX : " + String(usX_0) + " usY : " + String(usY_0) + " usZ : " + String(usZ_0) + " Timestamp : " + String(Unixtime3_0));
    } else if (NodeID_0 == 4) {
      int TofX_0 = parsing_0["TofX"];
      int TofY_0 = parsing_0["TofY"];
      int TofZ_0 = parsing_0["TofZ"];
      uint32_t Unixtime4_0 = parsing_0["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_0) + " ToF X : " + String(TofX_0) + " ToF Y :" + String(TofY_0) + " ToF Z : " + String(TofZ_0) + " Timestamp : " + String(Unixtime4_0));
    }
    //doc Array index 1
    JsonObject parsing_1 = doc[1];
    int NodeID_1 = parsing_1["NodeID"];
    if (NodeID_1 == 1) {
      int Suhu_1 = parsing_1["Suhu"];
      int Kelembapan_1 = parsing_1["Kelembapan"];
      uint32_t Unixtime1_1 = parsing_0["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_1) + " Suhu : " + String(Suhu_1) + " Kelembapan : " + String(Kelembapan_1) + " Timestamp : " + String(Unixtime1_1));
    } else if (NodeID_1 == 2) {
      int Berat_1 = parsing_1["Berat"];
      uint32_t Unixtime2_1 = parsing_1["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_1) + " Berat : " + String(Berat_1) + " Timestamp : " + String(Unixtime2_1));
    } else if (NodeID_1 == 3) {
      int usX_1 = parsing_1["usX"];
      int usY_1 = parsing_1["usY"];
      int usZ_1 = parsing_1["usZ"];
      uint32_t Unixtime3_1 = parsing_1["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_1) + " usX : " + String(usX_1) + " usY : " + String(usY_1) + " usZ : " + String(usZ_1) + " Timestamp : " + String(Unixtime3_1));
    } else if (NodeID_1 == 4) {
      int TofX_1 = parsing_1["TofX"];
      int TofY_1 = parsing_1["TofY"];
      int TofZ_1 = parsing_1["TofZ"];
      uint32_t Unixtime4_1 = parsing_1["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_1) + " ToF X : " + String(TofX_1) + " ToF Y :" + String(TofY_1) + " ToF Z : " + String(TofZ_1)) + " Timestamp : " + String(Unixtime4_1);
    }
    //doc Array index 2
    JsonObject parsing_2 = doc[2];
    int NodeID_2 = parsing_2["NodeID"];
    if (NodeID_2 == 1) {
      int Suhu_2 = parsing_2["Suhu"];
      int Kelembapan_2 = parsing_2["Kelembapan"];
      uint32_t Unixtime1_2 = parsing_2["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_2) + " Suhu : " + String(Suhu_2) + " Kelembapan : " + String(Kelembapan_2) + " Timestamp : " + String(Unixtime1_2));
    } else if (NodeID_2 == 2) {
      int Berat_2 = parsing_2["Berat"];
      uint32_t Unixtime2_2= parsing_2["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_2) + " Berat : " + String(Berat_2) + " Timestamp : " + String(Unixtime2_2));
    } else if (NodeID_2 == 3) {
      int usX_2 = parsing_2["usX"];
      int usY_2 = parsing_2["usY"];
      int usZ_2 = parsing_2["usZ"];
      uint32_t Unixtime3_2 = parsing_2["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_2) + " usX : " + String(usX_2) + " usY : " + String(usY_2) + " usZ : " + String(usZ_2) + " Timestamp : " + String(Unixtime3_2));
    } else if (NodeID_2 == 4) {
      int TofX_2 = parsing_2["TofX"];
      int TofY_2 = parsing_2["TofY"];
      int TofZ_2 = parsing_2["TofZ"];
      uint32_t Unixtime4_2 = parsing_2["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_2) + " ToF X : " + String(TofX_2) + " ToF Y :" + String(TofY_2) + " ToF Z : " + String(TofZ_2) + " Timestamp : " + String(Unixtime4_2));
    }
    //doc Array index 3
    JsonObject parsing_3 = doc[3];
    int NodeID_3 = parsing_3["NodeID"];
    if (NodeID_3 == 1) {
      int Suhu_3 = parsing_3["Suhu"];
      int Kelembapan_3 = parsing_3["Kelembapan"];
      uint32_t Unixtime1_3 = parsing_3["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_3) + " Suhu : " + String(Suhu_3) + " Kelembapan : " + String(Kelembapan_3) + " Timestamp : " + String(Unixtime1_3));
    } else if (NodeID_3 == 2) {
      int Berat_3 = parsing_3["Berat"];
      uint32_t Unixtime2_3 = parsing_3["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_3) + " Berat : " + String(Berat_3) + " Timestamp : " + String(Unixtime2_3));
    } else if (NodeID_3 == 3) {
      int usX_3 = parsing_3["usX"];
      int usY_3 = parsing_3["usY"];
      int usZ_3 = parsing_3["usZ"];
      uint32_t Unixtime3_3 = parsing_3["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_3) + " usX : " + String(usX_3) + " usY : " + String(usY_3) + " usZ : " + String(usZ_3) + " Timestamp : " + String(Unixtime3_3));
    } else if (NodeID_3 == 4) {
      int TofX_3 = parsing_3["TofX"];
      int TofY_3 = parsing_3["TofY"];
      int TofZ_3 = parsing_3["TofZ"];
      uint32_t Unixtime4_3 = parsing_3["Unixtime"];
      Serial.println("Node ID : " + String(NodeID_3) + " ToF X : " + String(TofX_3) + " ToF Y :" + String(TofY_3) + " ToF Z : " + String(TofZ_3) + " Timestamp : " + String(Unixtime4_3));
    }
    Serial.print("Jalur Pengiriman Data : ");
    for (int i = 0; i <= 4; i++) {
      Serial.print(jumlahnode[i]);
      Serial.print(",");
    }
    Serial.println("");
//==================================================MODE PENGIRIMAN DATA==================================================
    delay(1000);
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    char kirim_loop[datakirim.length() + 1];
    datakirim.toCharArray(kirim_loop,sizeof(kirim_loop));
    if (NodeID_3 == 4) {
      if (NODE_1_RSSI <= NODE_2_RSSI && NODE_1_RSSI <= NODE_3_RSSI && NODE_1_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_1);
        bool NODE_1 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_1 ? F("DATA TERKIRIM KE NODE 1") : F("GAGAL TERKIRIM KE NODE 1"));
        if(!NODE_1){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_2_RSSI <= NODE_1_RSSI && NODE_2_RSSI <= NODE_3_RSSI && NODE_2_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_2);
        bool NODE_2 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_2 ? F("DATA TERKIRIM KE NODE 2") : F("GAGAL TERKIRIM KE NODE 2"));
        if(!NODE_2){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_3_RSSI <= NODE_1_RSSI && NODE_3_RSSI <= NODE_2_RSSI && NODE_3_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_3);
        bool NODE_3 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_3 ? F("DATA TERKIRIM KE NODE 3") : F("GAGAL TERKIRIM KE NODE 3"));
        if(!NODE_3){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_4_RSSI <= NODE_1_RSSI && NODE_4_RSSI <= NODE_2_RSSI && NODE_4_RSSI <= NODE_3_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_4);
        bool NODE_4 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_4 ? F("DATA TERKIRIM KE NODE 4") : F("GAGAL TERKIRIM KE NODE 4"));
        if(!NODE_4){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    if (NodeID_3 == 3) {
      if (NODE_1_RSSI <= NODE_2_RSSI && NODE_1_RSSI <= NODE_3_RSSI && NODE_1_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_1);
        bool NODE_1 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_1 ? F("DATA TERKIRIM KE NODE 1") : F("GAGAL TERKIRIM KE NODE 1"));
        if(!NODE_1){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_2_RSSI <= NODE_1_RSSI && NODE_2_RSSI <= NODE_3_RSSI && NODE_2_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_2);
        bool NODE_2 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_2 ? F("DATA TERKIRIM KE NODE 2") : F("GAGAL TERKIRIM KE NODE 2"));
        if(!NODE_2){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_3_RSSI <= NODE_1_RSSI && NODE_3_RSSI <= NODE_2_RSSI && NODE_3_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_3);
        bool NODE_3 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_3 ? F("DATA TERKIRIM KE NODE 3") : F("GAGAL TERKIRIM KE NODE 3"));
        if(!NODE_3){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_4_RSSI <= NODE_1_RSSI && NODE_4_RSSI <= NODE_2_RSSI && NODE_4_RSSI <= NODE_3_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_4);
        bool NODE_4 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_4 ? F("DATA TERKIRIM KE NODE 4") : F("GAGAL TERKIRIM KE NODE 4"));
        if(!NODE_4){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    if (NodeID_3 == 2) {
      if (NODE_1_RSSI <= NODE_2_RSSI && NODE_1_RSSI <= NODE_3_RSSI && NODE_1_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_1);
        bool NODE_1 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_1 ? F("DATA TERKIRIM KE NODE 1") : F("GAGAL TERKIRIM KE NODE 1"));
        if(!NODE_1){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_2_RSSI <= NODE_1_RSSI && NODE_2_RSSI <= NODE_3_RSSI && NODE_2_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_2);
        bool NODE_2 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_2 ? F("DATA TERKIRIM KE NODE 2") : F("GAGAL TERKIRIM KE NODE 2"));
        if(!NODE_2){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_3_RSSI <= NODE_1_RSSI && NODE_3_RSSI <= NODE_2_RSSI && NODE_3_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_3);
        bool NODE_3 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_3 ? F("DATA TERKIRIM KE NODE 3") : F("GAGAL TERKIRIM KE NODE 3"));
        if(!NODE_3){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_4_RSSI <= NODE_1_RSSI && NODE_4_RSSI <= NODE_2_RSSI && NODE_4_RSSI <= NODE_3_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_4);
        bool NODE_4 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_4 ? F("DATA TERKIRIM KE NODE 4") : F("GAGAL TERKIRIM KE NODE 4"));
        if(!NODE_4){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    if (NodeID_3 == 1) {
      if (NODE_1_RSSI <= NODE_2_RSSI && NODE_1_RSSI <= NODE_3_RSSI && NODE_1_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_1);
        bool NODE_1 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_1 ? F("DATA TERKIRIM KE NODE 1") : F("GAGAL TERKIRIM KE NODE 1"));
        if(!NODE_1){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_2_RSSI <= NODE_1_RSSI && NODE_2_RSSI <= NODE_3_RSSI && NODE_2_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_2);
        bool NODE_2 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_2 ? F("DATA TERKIRIM KE NODE 2") : F("GAGAL TERKIRIM KE NODE 2"));
        if(!NODE_2){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_3_RSSI <= NODE_1_RSSI && NODE_3_RSSI <= NODE_2_RSSI && NODE_3_RSSI <= NODE_4_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_3);
        bool NODE_3 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_3 ? F("DATA TERKIRIM KE NODE 3") : F("GAGAL TERKIRIM KE NODE 3"));
        if(!NODE_3){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }else if (NODE_4_RSSI <= NODE_1_RSSI && NODE_4_RSSI <= NODE_2_RSSI && NODE_4_RSSI <= NODE_3_RSSI) {
        network.update();
        RF24NetworkHeader header(/*to node*/ NODE_4);
        bool NODE_4 = network.write(header, &kirim_loop, sizeof(kirim_loop));
        Serial.println(NODE_4 ? F("DATA TERKIRIM KE NODE 4") : F("GAGAL TERKIRIM KE NODE 4"));
        if(!NODE_4){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        }else{
          //Serial.println("Unixtime : "+String(now.unixtime()));
        }
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }
  //pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}
