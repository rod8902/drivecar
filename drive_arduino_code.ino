#include <Servo.h>

#define servoL_pin 12
#define servoR_pin 13


Servo Left, Right;
void setup() {
  // put your setup code here, to run once:
  Left.attach(servoL_pin);
  Right.attach(servoR_pin);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int lv=1500, rv=1500;
  //int x=0, y=0;
  
  int a1,a2,a3,a4,b1,b2,b3,b4;
  while(!Serial.available()){}
  a1=Serial.read();  
  while(!Serial.available()){}
  a2=Serial.read();  
  while(!Serial.available()){}
  a3=Serial.read();  
  while(!Serial.available()){}
  a4=Serial.read();  
  lv=(a4<<24)+(a3<<16)+(a2<<8)+a1;
  
  while(!Serial.available()){}
  b1=Serial.read();
  while(!Serial.available()){}
  b2=Serial.read();   
  while(!Serial.available()){}
  b3=Serial.read();  
  while(!Serial.available()){}
  b4=Serial.read();  
  rv=(b4<<24)+(b3<<16)+(b2<<8)+b1;
/*
  Serial.print(a1);
  Serial.print(a2);
  Serial.print(a3);
  Serial.println(a4);
  */
//  Serial.print(Serial.readString());
  //rv=Serial.readString();
  Serial.print("lv: ");
  Serial.print(lv);
  Serial.print(", rv: ");
  Serial.println(rv);

  Left.writeMicroseconds(lv);
  Right.writeMicroseconds(rv);   
  Serial.end();
  Serial.begin(9600);
}
