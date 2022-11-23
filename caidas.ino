#include <Wire.h>
#include <MPU6050.h>
const int MPU_addr=0x68;  
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;



boolean fall = false; 
boolean trigger1=false; 
boolean trigger2=false; 
boolean trigger3=false; 

byte trigger1count=0; 
byte trigger2count=0; 
byte trigger3count=0; 
int angleChange=0;


void setup(){
pinMode (8, OUTPUT);

 Wire.begin();
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);  
 Wire.write(0);     
 Wire.endTransmission(true);
 Serial.begin(9600);

 pinMode(11, OUTPUT);
 digitalWrite(11, HIGH);
}
void loop(){

 mpu_read();
 
 
 ax = (AcX-2600)/16384.00;
 ay = (AcY+400)/16384.00;
 az = (AcZ-2000)/16300;


 
 gx = (GyX+1490)/131.07;
 gy = (GyY-380)/131.07;
 gz = (GyZ+1090)/131.07;



 float Raw_AM = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
 int AM = Raw_AM * 10;  
                        
Serial.println(AM);

 
 

 if (trigger3==true){
    trigger3count++;
    
    if (trigger3count>=8){ 
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
       
       if ((angleChange>=0) && (angleChange<=10)){ 
           fall=true; trigger3=false; trigger3count=0;
             }
        else{ 
          trigger3=false; trigger3count=0;
          Serial.println("TRIGGER 3 DESACTIVADO");
       }
     }
  }
 if (fall==true){ 
   Serial.println("HAY UNA CAIDA");
   digitalWrite(11, LOW);
   delay(20);
   digitalWrite(11, HIGH);
   digitalWrite(8, HIGH);
   delay(500);
   digitalWrite(8, LOW);
   fall = false;
   trigger3=false;
   trigger2=false;
   trigger1=false;
   delay(1000);
  
   }
   if (trigger2count>=6){ 
   trigger2=false; trigger2count=0;
   Serial.println("TRIGGER 2 DESACTIVADO");
   }
 if (trigger1count>=12){ 
   trigger1=false; trigger1count=0;
   Serial.println("TRIGGER 1 DESACTIVADO");
   }
 if (trigger2==true){
   trigger2count++;
   
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
   if (angleChange>=30 && angleChange<=400){ 
      Serial.println("MOVIMIENTO CANDIDATO A CAERSE");   
     trigger3=true; trigger2=false; trigger2count=0;
       }
   }
 if (trigger1==true){
   trigger1count++;
   Serial.println(AM);
   if (AM>=15){ 
     trigger2=true;
     trigger1=false; trigger1count=0;
     }
   }
 if (AM>=11 && trigger2==false){ 
   trigger1=true;

   }

 delay(100);
}

void mpu_read(){
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B);  
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,true);  
 AcX=Wire.read()<<8|Wire.read();  
 AcY=Wire.read()<<8|Wire.read();  
 AcZ=Wire.read()<<8|Wire.read();  
 Tmp=Wire.read()<<8|Wire.read();  
 GyX=Wire.read()<<8|Wire.read();  
 GyY=Wire.read()<<8|Wire.read();  
 GyZ=Wire.read()<<8|Wire.read();  
 }