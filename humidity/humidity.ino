#include <dht11.h>

// ------ Humidity System ------
dht11 DHT11;
#define DHT11PIN 53
#define humi_fan 51
float temp_dht;
float humid_dht;
boolean hfanState = false;
float humid_thres = 40;
// ------ Water Level ------
#define water_l50 34
#define water_l20 35
#define water_l0 36
int wlevel50State = 0;
int wlevel20State = 0;
int wlevel0State = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("DHT11 AND DS18B20 TEST PROGRAM ");
  Serial.println();
  // ---------- Humisity Setting ----------
  pinMode(humi_fan, OUTPUT);
  // ---------- Water Level Setting ----------
  pinMode(water_l50, INPUT);
  pinMode(water_l20, INPUT);
  pinMode(water_l0, INPUT);
}

void loop()
{
  // ---------- Water level Loop ----------
  Get_Wlevel(wlevel50State, wlevel20State, wlevel0State);
  // ---------- Humisity Loop ----------
  Get_Humidity(temp_dht, humid_dht); // reading humidity sensor
  Get_HumiState(wlevel20State, humid_dht, humid_thres);
  Op_Humi();
  
  Serial.println(humid_dht);
  delay(5000);
}

// ------------------------------------------- FUNCTIONS --------------------------------------------------------
// ------------------------------------------- water level reading -------------------------------------------------
void Get_Wlevel(int & wlevel50State, int & wlevel20State, int & wlevel0State) {
  wlevel50State = digitalRead(water_l50);
  wlevel20State = digitalRead(water_l20);
  wlevel0State = digitalRead(water_l0);
}
// ------------------------------------------- humidity reading -------------------------------------------------
void Get_Humidity(float & temp_dht, float & humid_dht) {
  int chk = DHT11.read(DHT11PIN);
  switch (chk)
  {
    case DHTLIB_OK: Serial.println("OK"); break;
    case DHTLIB_ERROR_CHECKSUM: Serial.println("Checksum error"); break;
    case DHTLIB_ERROR_TIMEOUT: Serial.println("Time out error"); break;
    default: Serial.println("Unknown error"); break;
  }
  temp_dht = DHT11.temperature;
  humid_dht = DHT11.humidity + 10;
}

// ------------------------------------------- humidity state -------------------------------------------------
void Get_HumiState(int wlevel20State, float humid_dht, float humid_thres) {
  if (wlevel20State == HIGH) {
    if (humid_dht < humid_thres) {
      hfanState = true;
    }
    else {
      hfanState = false;
    }
  }
  else {
    hfanState = false;
  }
}

// ------------------------------------------- humidity operation -------------------------------------------------
void Op_Humi() {
  switch (hfanState)
  {
    case true: digitalWrite(humi_fan, HIGH); break;
    case false: digitalWrite(humi_fan, LOW);; break;
    default: Serial.println("Unknown error"); break;
  }
}
