/*********************************FFT********************************/
#include "arduinoFFT.h"
 
#define SAMPLES 128             //Must be a power of 2
#define SAMPLING_FREQUENCY 100 //Hz, must be less than 10000 due to ADC
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];
/******************************WIFI**************************************************/
#include "WiFiEsp.h"
/*#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6,7);
#endif*/

char ssid[]="Galaxy A12B183";//Galaxy A12B183
char pass[]="ipri1935";//ipri1935
int status =WL_IDLE_STATUS;

char server[] = "dweet.io";
unsigned long lastConnectionTime=0;
const unsigned long postingInterval=10L*1000;
char * thingName = "Ric_Fra_FFT";

WiFiEspClient client;


void httpRequest(double vReal[])
{
  client.stop();

  if(client.connect(server,80))
  {
    Serial.println("connecting..");

    String request = "GET /dweet/for/";
    request+= String(thingName);
    request+="?";
    for(int i=0;i<(SAMPLES/2);i++)
    {
      if(i<((SAMPLES/2)-1))
        request+= "a[]=" + String(vReal[i])+ "&";
      else
        request+= "a[]=" + String(vReal[i]);
    }
    //request+= "?value=" +String(sensorValue);
    

    client.println(request);
    client.println("Host: dweet.io");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
  }
  else
  {
    Serial.println("connection failed");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
 
void setup() {
    Serial.begin(9600);
    /*****************WIFI******************/
    Serial1.begin(9600);
    WiFi.init(&Serial1);
  
    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      // don't continue
      while (true);
    }
    // attempt to connect to WiFi network
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
    }
  
    // you're connected now, so print out the data
    Serial.println("You're connected to the network");
    
    //printWifiStatus();
  
    Serial.println();
    /**********************FFT***************************************/
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(0);
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.DCRemoval(vReal,SAMPLES);
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/
    Serial.print("Major peak=");
    Serial.println(peak);     //Print out what frequency is the most dominant.
    
    for(int i=0; i<(SAMPLES/2); i++)
    {
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        //Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        //Serial.print(" ");
        Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
    }
 
    /*******************************WIFI**********************************/
    while(client.available())
    {
    char c=client.read();
    Serial.write(c);
    }
    /*if(millis()-lastConnectionTime > postingInterval)
    {*/
      Serial.println("Entro");
      httpRequest(vReal);
      Serial.println("Esco");
    /*}*/
    //while(1);
}
