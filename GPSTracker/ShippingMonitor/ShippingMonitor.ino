#include <SPI.h>
#include <SD.h>
#include "Adafruit_Si7021.h"
#include "RTClib.h"
#include <Wire.h>
//#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GPS.h>


#define DEBUG false
#define RECORD_INTERVAL 15 //number of seconds between recordings
#define GPSSerial Serial1 //Serial address of GPS module
#define GPSECHO true

const int chipSelect = 4;
Adafruit_Si7021 temp_humid_sensor = Adafruit_Si7021(); //open I2C temperature/humidity sensor
RTC_PCF8523 rtc;
Adafruit_GPS GPS(&GPSSerial);
long past_time = 0;

//Accelerometer parameters
float max_excursion = 0; //maximum magnitude excursion seen over interval
float max_gx = 0; //max X accel
float max_gy = 0; //max Y accel
float max_gz = 0; //max Z accel
const int xInput = A0; //input ADC for X
const int yInput = A1; //input ADC for Y
const int zInput = A2; //input ADC for Z
int xZeroG = 2078; //Zero-G Calibration constant for X
int yZeroG = 2071; //Zero-G Calibration constant for Y
int zZeroG = 2076; //Zero-G Calibration constant for Z
float sens = .0065; //6.5 mV/step
float adc2V = 3./4096; //3V/12-bit yields V/ADC
int sampleSize = 10; //number of accelerometer samples to take and average (noise reduction)
DateTime wakeupTime(2021, 2, 24, 9, 0, 0); //Time to "wake-up" and start logging

void setup() {
  pinMode(8, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  analogReadResolution(12); 

  // Open serial communications and wait for port to open:
  if(DEBUG)
  {
    Serial.begin(115200);
    while (!Serial); // wait for serial port to connect. Needed for native USB port only
  }

  if(DEBUG)
    Serial.println("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    if(DEBUG)
      Serial.println("Card failed, or not present");
    // don't do anything more:
    while(true)
    {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
  }
  if(DEBUG)
    Serial.println("card initialized.");

  if(!rtc.begin())
  {
    if(DEBUG)
      Serial.println("Unable to communicate with RTC");
    while(true)
    {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(500);
    }
  }
  past_time = rtc.now().unixtime();

  if(!temp_humid_sensor.begin())
  {
    if(DEBUG)
      Serial.println("Unable to communicate with Si7021 sensor!");
    while(true)
    {
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
      delay(100);
    }
  }


  GPS.begin(9600);
  
  if (!GPSSerial)
  {
    if(DEBUG)
      Serial.println("Could not establish connection to GPS");
    while (true)
    {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
    }
  }
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_NOANTENNA);
  //GPS.sendCommand(PMTK_STANDBY);
  GPS.standby();
  
  while(rtc.now() < wakeupTime)
  {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(10000);
  }

  //GPS.sendCommand(PMTK_AWAKE);
  GPS.wakeup();
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";
//  String accelString = "";

  DateTime now = rtc.now();
  dataString += now.year();
  dataString += '/';
  dataString += now.month();
  dataString += '/';
  dataString += now.day();
  dataString += ' ';
  dataString += now.hour();
  dataString += ':';
  dataString += now.minute();
  dataString += ':';
  dataString += now.second();
  dataString += "\t";

  //Capture current acceleration from the accelerometer and append it to the data string:
  float xAcc = ConvertToGs(ReadAxis(xInput), xZeroG);
  float yAcc = ConvertToGs(ReadAxis(yInput), yZeroG);
  float zAcc = ConvertToGs(ReadAxis(zInput), zZeroG);
  float mag = Magnitude(xAcc, yAcc, zAcc);

  if(max_excursion < mag)
  {
    max_excursion = mag;
    max_gx = xAcc;
    max_gy = yAcc;
    max_gz = zAcc;
  }

  char c = GPS.read();

  if(now.unixtime() > past_time + RECORD_INTERVAL)
  {  
    past_time = now.unixtime(); //update the time of sensor info save
    
    // read the temperature and the humidity and append to the data string:
    dataString += String(temp_humid_sensor.readTemperature());
    dataString += "\t";
    dataString += String(temp_humid_sensor.readHumidity());
    dataString += "\t";

    //store the g-info
    dataString += String(max_gx);
    dataString += "\t";
    dataString += String(max_gy);
    dataString += "\t";
    dataString += String(max_gz);
    dataString += "\t";
    dataString += String(max_excursion);
    dataString += "\t";

    if(GPS.newNMEAreceived())
    {
      if(GPS.parse(GPS.lastNMEA()))
      {
        if(GPS.fix)
        {
          dataString += String(GPS.latitude,4);
          dataString += "\t";
          dataString += String(GPS.longitude, 4);
          dataString += "\t";
          dataString += String(GPS.speed);
        }
      }
    }

    if(DEBUG)
      Serial.println(dataString);
    else
    {    
      //Open file for writing
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
      // if the file is available, write to it:
      if (dataFile)
      {
        dataFile.println(dataString);
        dataFile.close();
        Serial.println(dataString); // print to the serial port
      }
      else
      {
        Serial.println("error opening datalog.txt"); // if the file isn't open, print error   
      }
    }

    max_excursion = 0;
    max_gx = 0;
    max_gy = 0;
    max_gz = 0;
  }

  delayMicroseconds(2000); //Sample the accelerometer at about 500Hz
}

int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

float ConvertToGs(int adc, int zeroG)
{
  return (adc - zeroG)*adc2V / sens;
}

float Magnitude(float x, float y, float z)
{
  return sqrt(pow(x,2)+pow(y,2)+pow(z,2));
}
