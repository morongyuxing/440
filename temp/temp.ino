#include <DallasTemperature.h>
#include <OneWire.h>
#include <dht11.h>

// ------ Humidity System ------
dht11 DHT11;
#define DHT11PIN 34
float temp_dht;
float humid_dht;
// ------ Temperature System ------
#define ONE_WIRE_BUS 50
#define ledPin 31
#define heater 33
#define cooler 35
#define tempfan 37
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature Tempsensors(&oneWire);
boolean ledState = false;
boolean heaterState = false;
boolean coolerState = false;
boolean hfanState = false;
float temp_threh = 35;
float temp_threl = 10;
float temp_h = 25;
float temp_l = 10;
  
void setup()
{
  Serial.begin(115200);
  Serial.println("DHT11 AND DS18B20 TEST PROGRAM ");
  Serial.println();
  // ---------- Temperature Setting ----------
  Tempsensors.begin();
  pinMode(ledPin,OUTPUT);
  pinMode(heater,OUTPUT);
  pinMode(cooler,OUTPUT);
  pinMode(tempfan,OUTPUT);
}

void loop()
{
  // ---------- Temperature Loop ----------
  Get_Humidity(temp_dht,humid_dht); // reading humidity sensor
  float temp_ds18 = Get_Temperature(); // reading temperature sensor
  float temp_aver = Temp_Average(temp_dht, temp_ds18); // temerature reading averaging
  Get_TempState(temp_threh,temp_h,temp_l,temp_threl,temp_aver); // checking the temperature state
  Op_Temp(); // turn on heater or cooler or LED
  
  Serial.println(temp_aver);
  delay(5000);
}

// ------------------------------------------- FUNCTIONS --------------------------------------------------------
// ------------------------------------------- humidity reading -------------------------------------------------
void Get_Humidity(float & temp_dht, float & humid_dht){
  int chk = DHT11.read(DHT11PIN);
  switch (chk)
  {
    case DHTLIB_OK:Serial.println("OK");break;
    case DHTLIB_ERROR_CHECKSUM:Serial.println("Checksum error");break;
    case DHTLIB_ERROR_TIMEOUT:Serial.println("Time out error");break;
    default:Serial.println("Unknown error");break;
  }
  temp_dht = DHT11.temperature;
  humid_dht = DHT11.humidity + 10;
}

// ------------------------------------------- temperature reading -------------------------------------------------
float Get_Temperature(){
  Tempsensors.requestTemperatures();  
  float temp_ds18 = Tempsensors.getTempCByIndex(0);
  return temp_ds18;
}

// ------------------------------------------- temperature averaging -------------------------------------------------
float Temp_Average(float temp_dht, float temp_ds18){
  float temp_aver = (temp_dht + temp_ds18) / 2;
  return temp_aver;
}

// ------------------------------------------- temperature state -------------------------------------------------
void Get_TempState(float temp_threh,float temp_h,float temp_l,float temp_threl,float temp_aver){
  if (temp_aver > temp_threh){
    ledState = true;
    heaterState = false;
    coolerState = false;
    hfanState = false;
  }
  else if (temp_aver > temp_h){
    ledState = false;
    heaterState = false;
    coolerState = true;
    hfanState = true;
  }
  else if (temp_aver > temp_l){
    ledState = false;
    heaterState = false;
    coolerState = false;
    hfanState = false;
  }
  else if (temp_aver > temp_threl){
    ledState = false;
    heaterState = true;
    coolerState = false;
    hfanState = true;
  }
  else{
    ledState = true;
    heaterState = false;
    coolerState = false;
    hfanState = false;
  }
}

// ------------------------------------------- temperature operation -------------------------------------------------
void Op_Temp(){
  switch (ledState)
  {
    case true:digitalWrite(ledPin,HIGH);break;
    case false:digitalWrite(ledPin,LOW);;break;
    default:Serial.println("Unknown error");break;
  }
  switch (heaterState)
  {
    case true:digitalWrite(heater,HIGH);break;
    case false:digitalWrite(heater,LOW);;break;
    default:Serial.println("Unknown error");break;
  }
  switch (coolerState)
  {
    case true:digitalWrite(cooler,HIGH);break;
    case false:digitalWrite(cooler,LOW);;break;
    default:Serial.println("Unknown error");break;
  }
  switch (hfanState)
  {
    case true:digitalWrite(tempfan,HIGH);break;
    case false:digitalWrite(tempfan,LOW);;break;
    default:Serial.println("Unknown error");break;
  }
}

