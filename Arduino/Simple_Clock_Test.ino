
// Module Address
int H1Addr = 0;
int M1Addr = 1;
int S1Addr = 7;
int C1Addr = 6;

// Starting hh:mm:ss
int h1=23;
int m1=59;
int s1=50;
int c1=26;

// Starting dd.mm.yy
int D1=4;
int M1=11;
int Y1=18;

// Temp variable to convert minute to pos
int m2=0;
int s2=0;
int D2=0;
int M2=0;

void setup() {
   while (!Serial){
   ;
  }
  Serial.begin(19200); 
  
  // DE and RE pins
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);

  // Button for date/Hour switch
  pinMode(6, INPUT);

}

int cnt = 0;
unsigned long timestart;
unsigned long timeend;
unsigned long delta;
bool showDate = false;
int buttonState = 0; 

void loop() {
    
	
    buttonState = digitalRead(6);
    if (buttonState == HIGH) 
	{    
		showDate = false;
	} else {
    
		showDate = true;
	}
    cnt++;

    if(cnt>6)
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
        
        Serial.print(h1); 
        Serial.print(":");
        Serial.print(m2); 
        Serial.print(":");
        Serial.print(s2); 
        Serial.println();
      }
         

    // Get Date
    if(showDate)
    {
      if(D1 <= 30) D2 = D1+30;    
      else if(D1 > 30) D2 = D1-31;
    
      if(M1 <= 30) M2 = M1+30;
      else if(M1 > 30) M2 = M1-31;
      SendPos(H1Addr, 26);
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

