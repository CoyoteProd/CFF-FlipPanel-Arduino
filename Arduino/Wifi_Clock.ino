#include "ESP8266.h"
#include <SoftwareSerial.h>
#include "RTClib.h"


#define SSID "xxxx"
#define PASS "xxxxx"

#define CONNECT_ATTEMPTS 15
#define RST 10
#define CHP 11
#define STX 8
#define SRX 9
#define LED 13

#define HOST_NAME   "82.209.243.241"
#define HOST_PORT   (123)

#define H1Addr 0
#define M1Addr 1
#define S1Addr 7
#define C1Addr 6
#define c1 26



SoftwareSerial espSerial(SRX, STX); // RX, TX
ESP8266 wifi(espSerial);
RTC_Millis rtc;
bool showDate = false;
int buttonState = 0; 

void reset();
boolean connectWiFi();

void setup() 
{

    while (!Serial){
    ;
    }
     
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);

    pinMode(6, INPUT);
    
    // put your setup code here, to run once:
    pinMode(RST, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(CHP, OUTPUT);
    reset();
    
    Serial.begin(9600);
    Serial.print("setup begin\r\n");

    Serial.print("FW Version: ");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.joinAP("underground", "permanent")) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");       
        Serial.println(wifi.getLocalIP().c_str());
        ntpupdate();
        
        
    } else {
        Serial.print("Join AP failure\r\n");
    }
}

unsigned long timestart;
unsigned long timeend;
unsigned long delta;


void loop() 
{
   //Serial.print("Time start: ");
   //timestart = millis();
   //Serial.println(timestart ); 
    
    DateTime now = rtc.now();
    Display(now.year(), now.month(), now.day(), now.hour()+2, now.minute(), now.second());
    //delay(100);

    //Serial.print("Time end: ");
    //timeend = millis();
    //Serial.println(timeend ); 

    //Serial.print("Time delta: ");
    //delta = timeend-timestart;
    //Serial.println(delta ); 
}

int cnt=0;

void Display(int Y1,int M1,int D1, int h1, int m1, int s1)
{
  int m2=0; int s2=0;
  int D2=0; int M2=0;

    buttonState = digitalRead(6);
     if (buttonState == HIGH) {
    // turn LED on:
    showDate = false;
  } else {
    // turn LED off:
    showDate = true;
  }
    cnt++;

    if(cnt>5)
    {  // Second
        cnt=0;
        s1++;
        if(s1>59)
        {
            s1 = 0;
            m1++;
            if(m1>59)
            {
                m1 = 0;
                
                h1++;
                if(h1>23)
                {
                    h1=0;                  
                }
            }
        }
        
        //Serial.print(h1);  Serial.print(":"); Serial.print(m1);  Serial.print(":"); Serial.print(s1);  Serial.println();
      }
         

    // Get Date
    if(showDate)
    {
      if(D1 <= 30) D2 = D1+30;    
      else if(D1 > 30) D2 = D1-31;
    
      if(M1 <= 30) M2 = M1+30;
      else if(M1 > 30) M2 = M1-31;
      
      SendPos(H1Addr, c1);
      SendPos(M1Addr, D2);
      SendPos(S1Addr, M2);
      SendPos(C1Addr, Y1);
      
    }else
    {
      // Get time    
      if(m1 <= 30) m2 = m1+30;    
      else if(m1 > 30) m2 = m1-31;
    
      if(s1 <= 30) s2 = s1+30;
      else if(s1 > 30) s2 = s1-31;
                    
      SendPos(H1Addr, h1);
      SendPos(M1Addr, m2);
      SendPos(S1Addr, s2);
      SendPos(C1Addr, c1);
    }

    
    
}


void SendPos(byte address, byte pos)
{
    sendBreak(50);
    Serial1.write(0xFF);
    Serial1.write(0xC0);
    Serial1.write(address);          
    Serial1.write(pos);    
}
void sendBreak(unsigned int duration)
{
  Serial1.flush();
  Serial1.end();
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  delay(duration);
  Serial1.begin(19200);
}

void reset() {
  
  digitalWrite(CHP,HIGH);
  digitalWrite(RST,LOW);
  delay(100);
  digitalWrite(RST,HIGH);
  delay(1000);
  
}


void ntpupdate()
{
  uint8_t buffer[128] = {0};

  if (wifi.registerUDP(HOST_NAME, HOST_PORT)) {
    Serial.print("register udp ok\r\n");
  } else {
    Serial.print("register udp err\r\n");
  }

  static const char PROGMEM
  timeReqA[] = { 227,  0,  6, 236 },timeReqB[] = {  49, 78, 49,  52 };
  // Assemble and issue request packet
  uint8_t       buf[48];
  memset(buf, 0, sizeof(buf));
  memcpy_P( buf    , timeReqA, sizeof(timeReqA));
  memcpy_P(&buf[12], timeReqB, sizeof(timeReqB));
  
  wifi.send((const uint8_t*)buf, 48);

  uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
    Serial.print("UNIX TIME IS:");

    // Serial.print(buffer[42]);
    unsigned long t = (((unsigned long)buffer[40] << 24) |
                       ((unsigned long)buffer[41] << 16) |
                       ((unsigned long)buffer[42] <<  8) |
                       (unsigned long)buffer[43]) - 2208988800UL;
    rtc.begin(DateTime(t));
    Serial.println(t);
  }else
  {
    Serial.println("ntp Error");
  }
  

  
  
  if (wifi.unregisterUDP()) {
    Serial.print("unregister udp ");
    Serial.println(" ok");
  } else {
    Serial.print("unregister udp ");
    Serial.println(" err");
  }

}

