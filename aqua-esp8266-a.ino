#include<ESP8266WiFi.h>
#include<SoftwareSerial.h>
#include<FirebaseArduino.h> // firebase integration
#include<ArduinoJson.h>
#include<ESP8266HTTPClient.h>
SoftwareSerial arduino (1,3); //(Rx,Tx)

//firebase 
#define FIREBASE_HOST "aquarius-866e2-default-rtdb.firebaseio.com"      //add host
#define FIREBASE_AUTH "33KOyVPdsSidscNoZt6RBCjtkJRM0BHZBWj0lr8B"      //add secret key

//wifi
#define WIFI_SSID "ArnoldFIBR"         //wifi ssd
#define WIFI_PASSWORD "0MCtpr@123400"   //wifipassword

//time limit
unsigned long previousMillis=0;
const long interval=10000;


//Variable for parsing data from arduino to nodemcu --//
int8_t indexofA, indexofB, indexofC, indexofD, indexofE;
String data1, data2, data3;
String DataIn;
char c;

String phData, doData, tempData;
String ph, dox, temp;

  

void setup() {
 Serial.begin(9600);
  delay(500);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
      {
    Serial.print(".");
    delay(500);
      }
  Serial.println();
  Serial.print("connected: ");  
  Serial.println(WiFi.localIP());

   Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
   Serial.println("AQUARIUS \n\n");
    delay(1000);

  
   

}

void loop() {

while(arduino.available()>0)
  {
    c = arduino.read();

    if(c == '\n') { break; }
    else {DataIn+=c;}
  }
  if(c== '\n')
  {
    Parse_the_data();
    //print on serial the data that sent by arduino.
    Serial.println("data1 = " + data1);
    Serial.println("data2 = " + data2);
    Serial.println("data3 = " + data3);
    Serial.println("==========================");
    c=0;
    DataIn="";
  }

//send sensor data to firebase
Firebase.pushString("/sensor/temp", data1);
Firebase.pushString("/sensor/pH", data2);
Firebase.pushString("/sensor/DO", data3);

  if(Firebase.failed()){
    Serial.print("pushing /logs failed:");
    Serial.println(Firebase.error());
    return;
    }

}

void Parse_the_data()
{
  indexofA = DataIn.indexOf("A");
  indexofB = DataIn.indexOf("B");
  indexofC = DataIn.indexOf("C");

  data1 = DataIn.substring (0, indexofA);
  data2 = DataIn.substring (indexofA+1, indexofB);
  data3 = DataIn.substring (indexofB+1, indexofC);

}
