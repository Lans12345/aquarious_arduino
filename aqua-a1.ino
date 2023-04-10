  //---- Start: Including of all libraries ----//
#include <stdlib.h>
#include <Arduino.h>
#include "SoftwareSerial.h" //communication between arduino and nodemcu
#include <OneWire.h> // for temp 
#include <DallasTemperature.h> // for temp 
#include <DS3231.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#define rxPin 11  //for GSM SIM800
#define txPin 10  //for GSM Sim 800

SoftwareSerial serialSIM800(10, 11);  //for GSM 
SoftwareSerial nodemcu(6, 7); // serial communication from esp
String data1, data2, data3; // variables to parse data to nodemcu
int water_pump = 13;             //arduino pin for waterpump_relay
int solenoid = 10;               //arduino pin for solenoid_relay
int aerator = 9;                //arduino pin for aerator_relay


//--normal values for sensors--//
float pH_normal = 7; float  DO_normal = 5; float temp_normal = 25;
float pH_float=data1.toFloat(); float temp_float=data2.toFloat(); float DO_float=data3.toFloat();


DS3231  rtc(SDA, SCL);    
Time t;

//water pump
int OnSec_wp = 5;          //turn on after x seconds
int OffSec_wp = 10;        // turn off after x seconds

//solenoid
int OnSec_sol=30;         //turn on after x seconds
int OffSec_sol=35;        //turn off after x seconds

//-- Start: PH SENSOR CODE--//
  float calibration_value = 21.34 - 0.7;
  int phval = 0;        //pin PH
  unsigned long int avgval; 
  int buffer_arr[10],temp;
  float ph_act;
  
//-- End: PH SENSOR CODE --//
  
//-- Start: TEMP SENSOR CODE --//
  #define ONE_WIRE_BUS 4      //pin sa temp
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature temps(&oneWire);
  float Celcius = 0;
  float Fahrenheit = 0;
//-- End: TEMP SENSOR CODE --//

//-- Start: DO SENSOR CODE --//
  #define DO_PIN A2   //DO pin
  #define VREF 5000    //VREF (mv)
  #define ADC_RES 1024 //ADC Resolution
  #define CAL1_V (2084) //mv
  #define CAL1_T (29)   //℃ 
  #define READ_TEMP (29)

  const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 0
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}
//-- End: DO SENSOR CODE --//

//--- End: Defining of variables ----//




void setup() {
  
  Serial.begin(9600); //start serial
  nodemcu.begin(9600); //start connection between arduino to esp

  //GSM
  serialSIM800.begin(9600);
  delay(1000);
  Serial.println("SIM800L software serial initialize");
  serialSIM800.println("AT");
  pinMode(aerator, OUTPUT);
  digitalWrite(aerator, LOW);



   //relay and rtc
  rtc.begin();
  rtc.setDOW(FRIDAY);     
  rtc.setTime(11, 0, 0);      //hours,minutes,second
  rtc.setDate(30,10,2023);    //day,month,year

  //Water pump 
  pinMode(water_pump, OUTPUT);
  digitalWrite(water_pump, LOW);


  //solenoid 
  pinMode(solenoid,OUTPUT);
  digitalWrite(solenoid,LOW);
 
  //aerator
  pinMode(aerator,OUTPUT);
  digitalWrite(solenoid,LOW);
 

}
void loop() {

 //-- Start: PH Sensor code --//
  for(int i=0;i<10;i++) 
 { 
 buffer_arr[i]=analogRead(A0);
 }
 delay(30);
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/1024/6; 
  ph_act = -5.70 * volt + calibration_value;
  data1 = ph_act;
  
  //-- End: PH Sensor code --//


   //-- Start: TEMP Sensor code --//
  temps.requestTemperatures();
  Celcius = temps.getTempCByIndex(0);
  Fahrenheit = temps.toFahrenheit(Celcius);
  data2 = Celcius;
  //-- End: TEMP Sensor code --//


   //-- Start: DO Sensor code --//
  Temperaturet = (uint8_t)READ_TEMP;
  ADC_Raw = analogRead(DO_PIN);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
  data3 = (readDO(ADC_Voltage, Temperaturet)/1000);
  //-- End: DO Sensor code --//


  //Send data to nodemcu
  nodemcu.print(data1); nodemcu.print("A");
  nodemcu.print(data2); nodemcu.print("B");
  nodemcu.print(data3); nodemcu.print("C");
  nodemcu.print("\n");

  //monitor data
  Serial.print("pH:    "); Serial.println(data1);
  Serial.print("temp:  "); Serial.println(data2);
  Serial.print("DO:    "); Serial.println(data3);
 //--GSM--//

 
  if (pH_float < pH_normal || DO_float < DO_normal || temp_float< temp_normal) {
    digitalWrite(aerator, HIGH);
    //--Send Message Alert--//
     serialSIM800.write("AT+CMFG=1\r\n");
     delay(1000);
     serialSIM800.write("AT+CMGS=\"+639124656787\"\r\n");
     delay(1000);
    serialSIM800.write("ALERT");
    delay(1000);
    serialSIM800.write((char)26);
    delay(1000);
    Serial.println("Sent");
    delay(1000);}
   else 
  {
    digitalWrite(aerator, LOW);
    delay(1000);
    //--SendMessageNormal()--//
    serialSIM800.write("AT+CMFG=1\r\n");
    delay(1000);
    serialSIM800.write("AT+CMGS=\"+639124656787\"\r\n");
    delay(1000);
    serialSIM800.write("NORMAL");
    delay(1000);
    serialSIM800.write((char)26);
    delay(1000);
    Serial.println("Sent");    
    }
  
 }
