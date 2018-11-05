#define LIGHTWEIGHT 1


#include <SPI.h>
#include <Ethernet2.h>
#include <aREST.h>
#include <avr/wdt.h>

#define SS     10U    //D10----- SS
#define RST    11U    //D11----- Reset
#define bitrate 19200
// Enter a MAC address for your controller below.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xFE, 0x41 };

EthernetServer server(80);
aREST rest = aREST();

byte ModulesBuffer[12];

// Declare functions to be exposed to the API
int PositionControl(String command);
bool locker = false;

void setup(void)
{
  //Start Serial
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("init");


  // Function to be exposed
  rest.function("SetPosition", PositionControl);
  rest.function("SetBladePosition", BladePositionControl);
  rest.function("Pulse", PulseControl);
  
  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("001");
  rest.set_name("GdP-SBB");

  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  digitalWrite(SS, LOW);
  digitalWrite(RST, HIGH); //Reset
  delay(200);
  digitalWrite(RST, LOW);
  delay(200);
  digitalWrite(RST, HIGH);
  delay(200);              //Wait W5500

  // Start the Ethernet connection and the server
  Serial.println("Start Ethernet");
  if (Ethernet.begin(mac) == 0) {
    //Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac);
  }
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
 
  
  Serial.println("reset.");
  
  // Set transmit mode for RS485
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  sendBreak(1000);
  Serial1.write(0xFF); //Start
  Serial1.write(0xC0); //Set Pos
  Serial1.write(0x00); //Module
  for (int a = 0; a < 12; a++)
  {
    Serial1.write(0x00); //Value
  }

  Serial.println("reset done");
  delay(5000);
  // Start watchdog
  wdt_enable(WDTO_4S);
}
int count = 0;
void loop() {

  // listen for incoming clients
  EthernetClient client = server.available();
  rest.handle(client);
  wdt_reset();

  count++;
  if (count>250)
  {
    count=0;
    sendBreak(50);      
    Serial1.write(0xFF); //Start
    Serial1.write(0xC0);  //Set Pos
    Serial1.write(0x00);  //Débuter au premier module
    for (int i = 0; i < 12; i++) {
      Serial1.write(ModulesBuffer[i]); //Value du module courant
    }    
  }

}

int PositionControl(String command)
{
  if (!locker)
  {
    locker = true;
    byte PositionsModules[25];
        
    //http://de-sbb/SetPosition?cmd={position};{position};... x12
    Serial.println("PositionControl: " + command);
    //sendBreak(80);      
    //Serial1.write(0xFF); //Start
    //Serial1.write(0xC0);  //Set Pos
    //Serial1.write(0x00);  //Débuter au premier module
    for (int i = 0; i < 12; i++) {
      ModulesBuffer[i] = getValue(command, ';', i).toInt(); //Value du module courant
    }    
    locker = false;
    return 1;
  } else
  {
    return -1;
  }
}


int BladePositionControl(String command)
{
  if (!locker)
  {
    locker = true;
        
    //http://de-sbb/SetBladePosition?cmd={position};{position}
    Serial.println("SetBladePositionControl: " + command);
    String strAdr = getValue(command, ';', 0);
    String strPos = getValue(command, ';', 1);

    int addr = strAdr.toInt();
    int pos = strPos.toInt();

    sendBreak(50);
    Serial1.write(0xFF);
    Serial1.write(0xCB);
    Serial1.write(addr);
    Serial1.write(pos);    
        
    locker = false;
    return 1;
  } else
  {
    return -1;
  }
}



int PulseControl(String command)
{
  if (!locker)
  {
    locker = true;  
        
    //http://de-sbb/Pulse?cmd={addr}
    Serial.println("Pulse: " + command);    
    int addr = command.toInt();
    sendBreak(50);      
    Serial1.write(0xFF); //Start
    Serial1.write(0xC6);  // send pulse
    Serial1.write(addr);  //address
      
    locker = false;
    return 1;
  } else
  {
    return -1;
  }
}



String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void sendBreak(unsigned int duration)
{
  Serial1.flush();
  Serial1.end();
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  delay(duration);
  Serial1.begin(bitrate);
}

