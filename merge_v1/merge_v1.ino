#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <dht11.h>

// ------ UI System ------
LiquidCrystal lcd(40, 42, 22, 24, 26, 28);
#define UImode 12
#define RbuttonPin 2
#define LbuttonPin 3
#define SbuttonPin 4
#define CbuttonPin 5
#define MbuttonPin 6
// Variables will change:
int UImodeState = 0;
int lastUImodeState = 0;
int buttonPushCounter = 0; // counter for the number of button presses
int levelCounter = 0;
int RbuttonState = 0;         // current state of the button
int lastRbuttonState = 0;     // previous state of the button
int LbuttonState = 0;         // current state of the button
int lastLbuttonState = 0;     // previous state of the button
int SbuttonState = 0;         // current state of the button
int lastSbuttonState = 0;     // previous state of the button
int printPosition = 0;
int CbuttonState = 0;         // current state of the button
int lastCbuttonState = 0;     // previous state of the button
int MbuttonState = 0;         // current state of the button
int lastMbuttonState = 0;     // previous state of the button
const char level[4][17] = {
  {'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', '\0'},
  {'I', 'i', 'J', 'j', 'K', 'k', 'L', 'l', 'M', 'm', 'N', 'n', 'O', 'o', 'P', 'p', '\0'},
  {'Q', 'q', 'R', 'r', 'S', 's', 'T', 't', 'U', 'u', 'V', 'v', 'W', 'w', 'X', 'x', '\0'},
  {'Y', 'y', 'Z', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'}
};
String senddata;
// ------ Humidity System ------
dht11 DHT11;
#define DHT11PIN 53
#define humi_fan 51
float temp_dht;
float humid_dht;
boolean hfanState = false;
float humid_thres = 40;
// ------ Water Level ------
#define water_l50 45
#define water_l20 47
#define water_l0 49
int wlevel50State = 0;
int wlevel20State = 0;
int wlevel0State = 0;
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
boolean thfanState = false;
float temp_threh = 35;
float temp_threl = 10;
float temp_h = 25;
float temp_l = 10;

#define plantName "Weed"


void setup() {
  // ---------- UI Setting ----------
  lcd.begin(16, 2);
  lcd.blink();
  pinMode(UImode, INPUT);
  pinMode(RbuttonPin, INPUT);
  pinMode(LbuttonPin, INPUT);
  pinMode(SbuttonPin, INPUT);
  pinMode(CbuttonPin, INPUT);
  pinMode(MbuttonPin, INPUT);
  // ---------- Humisity Setting ----------
  pinMode(humi_fan, OUTPUT);
  // ---------- Water Level Setting ----------
  pinMode(water_l50, INPUT);
  pinMode(water_l20, INPUT);
  pinMode(water_l0, INPUT);
  // ---------- Temperature Setting ----------
  Tempsensors.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(cooler, OUTPUT);
  pinMode(tempfan, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  UImodeState = digitalRead(UImode);
  if (UImodeState == HIGH) {
    if (UImodeState != lastUImodeState) {
      lcdReset();
    }
    rightM(buttonPushCounter, levelCounter);
    leftM(buttonPushCounter, levelCounter);
    selectM(printPosition, buttonPushCounter, levelCounter);
    cancelM(printPosition, buttonPushCounter, levelCounter);
    sendM();
  }
  else {
    // ---------- Sensor reading Loop ----------
    Get_Wlevel(wlevel50State, wlevel20State, wlevel0State);
    Get_Humidity(temp_dht, humid_dht); // reading humidity sensor
    float temp_ds18 = Get_Temperature(); // reading temperature sensor
    float temp_aver = Temp_Average(temp_dht, temp_ds18); // temerature reading averaging

    // ---------- Humisity Loop ----------
    Get_HumiState(wlevel20State, humid_dht, humid_thres);
    Op_Humi();
    // ---------- Temperature Loop ----------
    Get_TempState(temp_threh, temp_h, temp_l, temp_threl, temp_aver); // checking the temperature state
    Op_Temp(); // turn on heater or cooler or LED

    if (UImodeState != lastUImodeState) {
      lcd.clear();
    }
    lcdDisplayTH(temp_aver, humid_dht);
    delay(1000);
  }
  delay(50);
  lastUImodeState = UImodeState;

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

// ------------------------------------------- temperature reading -------------------------------------------------
float Get_Temperature() {
  Tempsensors.requestTemperatures();
  float temp_ds18 = Tempsensors.getTempCByIndex(0);
  return temp_ds18;
}

// ------------------------------------------- temperature averaging -------------------------------------------------
float Temp_Average(float temp_dht, float temp_ds18) {
  float temp_aver = (temp_dht + temp_ds18) / 2;
  return temp_aver;
}

void lcdDisplay (int levelCounter) {
  switch (levelCounter)
  {
    case 0: lcd.print(level[0]); break;
    case 1: lcd.print(level[1]); break;
    case 2: lcd.print(level[2]); break;
    case 3: lcd.print(level[3]); lcd.setCursor(14, 1); lcd.print("  "); break;
    default: Serial.println("Unknown error"); break;
  }
}

// ------------------------------------------- temperature state -------------------------------------------------
void Get_TempState(float temp_threh,float temp_h,float temp_l,float temp_threl,float temp_aver){
  if (temp_aver > temp_threh){
    ledState = true;
    heaterState = false;
    coolerState = false;
    thfanState = false;
  }
  else if (temp_aver > temp_h){
    ledState = false;
    heaterState = false;
    coolerState = true;
    thfanState = true;
  }
  else if (temp_aver > temp_l){
    ledState = false;
    heaterState = false;
    coolerState = false;
    thfanState = false;
  }
  else if (temp_aver > temp_threl){
    ledState = false;
    heaterState = true;
    coolerState = false;
    thfanState = true;
  }
  else{
    ledState = true;
    heaterState = false;
    coolerState = false;
    thfanState = false;
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
  switch (thfanState)
  {
    case true:digitalWrite(tempfan,HIGH);break;
    case false:digitalWrite(tempfan,LOW);;break;
    default:Serial.println("Unknown error");break;
  }
}

void rightM(int & buttonPushCounter, int & levelCounter) {
  RbuttonState = digitalRead(RbuttonPin);
  if (RbuttonState != lastRbuttonState) {
    if (RbuttonState == HIGH) {
      buttonPushCounter ++;
      if (buttonPushCounter > 15) {
        if (levelCounter == 3) {
          buttonPushCounter = 15;
        } else {
          levelCounter ++;
          lcd.setCursor(0, 1);
          lcdDisplay(levelCounter);
          lcd.setCursor(0, 1);
          buttonPushCounter = 0;
        }
      } else {
        lcd.setCursor(buttonPushCounter, 1);
      }
    }
    delay(50);
  }
  lastRbuttonState = RbuttonState;
}

void leftM(int & buttonPushCounter, int & levelCounter) {
  LbuttonState = digitalRead(LbuttonPin);
  if (LbuttonState != lastLbuttonState) {
    if (LbuttonState == HIGH) {
      buttonPushCounter --;
      if (buttonPushCounter < 0) {
        if (levelCounter == 0) {
          buttonPushCounter = 0;
        } else {
          levelCounter --;
          lcd.setCursor(0, 1);
          lcdDisplay(levelCounter);
          lcd.setCursor(15, 1);
          buttonPushCounter = 15;
        }
      } else {
        lcd.setCursor(buttonPushCounter, 1);
      }
    }
    delay(50);
  }
  lastLbuttonState = LbuttonState;
}

void selectM(int & printPosition, int & buttonPushCounter, int & levelCounter) {
  SbuttonState = digitalRead(SbuttonPin);
  if (SbuttonState != lastSbuttonState) {
    if (SbuttonState == LOW) {
      if (printPosition > 15) {
      }
      else {
        lcd.setCursor(0, 0);
        senddata = senddata + String(level[levelCounter][buttonPushCounter]);
        lcd.print(senddata);
        printPosition ++;
        lcd.setCursor(buttonPushCounter, 1);
      }
    }
    delay(50);
  }
  lastSbuttonState = SbuttonState;
}

void cancelM(int & printPosition, int & buttonPushCounter, int & levelCounter) {
  CbuttonState = digitalRead(CbuttonPin);
  if (CbuttonState != lastCbuttonState) {
    if (CbuttonState == HIGH) {
      if (printPosition < 1) {
      }
      else {
        lcd.setCursor(0, 0);
        printPosition --;
        senddata = senddata.substring(0, printPosition);
        lcd.print(senddata);
        lcd.print(' ');
        lcd.setCursor(buttonPushCounter, 1);
      }
    }
    delay(50);
  }
  lastCbuttonState = CbuttonState;
}

void sendM() {
  MbuttonState = digitalRead(MbuttonPin);
  if (MbuttonState != lastMbuttonState) {
    if (MbuttonState == HIGH) {
      Serial.println(senddata);
    }
    delay(50);
  }
  lastMbuttonState = MbuttonState;
}

void lcdReset() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcdDisplay(0);
  lcd.setCursor(0, 1);
  senddata = "";
  buttonPushCounter = 0; // counter for the number of button presses
  levelCounter = 0;
  RbuttonState = 0;         // current state of the button
  lastRbuttonState = 0;     // previous state of the button
  LbuttonState = 0;         // current state of the button
  lastLbuttonState = 0;     // previous state of the button
  SbuttonState = 0;         // current state of the button
  lastSbuttonState = 0;     // previous state of the button
  printPosition = 0;
  CbuttonState = 0;         // current state of the button
  lastCbuttonState = 0;     // previous state of the button
  MbuttonState = 0;         // current state of the button
  lastMbuttonState = 0;     // previous state of the button
}

void lcdDisplayTH(float temp_aver, float humid_dht) {
  lcd.setCursor(0, 0);
  lcd.print("Name:"); lcd.println(plantName);
  lcd.setCursor(strlen(plantName) + 5, 0); lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print("T:"); lcd.print(temp_aver);
  lcd.setCursor(9, 1);
  lcd.print("H:"); lcd.print(humid_dht);
}
