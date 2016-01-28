#define servoL_pin 12
#define servoR_pin 13

#include <Servo.h>
#include <String.h>

Servo servoL;
Servo servoR;

String getSign;
float lv=1500,rv=1500; 

void setup() {
  servoR.attach(servoR_pin);
  servoL.attach(servoL_pin);
  Serial.begin(9600);  
}

void loop() {
  int int_a=0,int_b=0,int_r=0;  
  int a,a1,a2,a3,a4,b,b1,b2,b3,b4;
  while(!Serial.available()){}
  a1=Serial.read();  
  while(!Serial.available()){}
  a2=Serial.read();  
  while(!Serial.available()){}
  a3=Serial.read();  
  while(!Serial.available()){}
  a4=Serial.read();  
  a=(a4<<24)+(a3<<16)+(a2<<8)+a1;
  
  while(!Serial.available()){}
  b1=Serial.read();
  while(!Serial.available()){}
  b2=Serial.read();   
  while(!Serial.available()){}
  b3=Serial.read();  
  while(!Serial.available()){}
  b4=Serial.read();  
  b=(b4<<24)+(b3<<16)+(b2<<8)+b1;
  
  int_a=a; 
  int_b=b;    
}

void drive(int accel, int brake, int angle){
  int d;
  float velocity;
  float diff_max_rv = rv-1400; //현재 우측모터속도와 최대모터속도와의 차이
  float diff_max_lv = 1600-lv; //현재 좌측모터속도와 최대모터속도와의 차이
  float diff_min_rv = 1500-rv; //현재 우측모터속도와 모터정지값과의 차이  
  float diff_min_lv = lv-1500; //현재 좌측모터속도와 모터정지값과의 차이
  
 /* 
  Serial.print("lv : ");
  Serial.println(lv); 
  Serial.print("rv : ");
  Serial.println(rv);
  Serial.print("accel : ");
  Serial.println(accel);
  Serial.print("brake : ");
  Serial.println(brake);
  Serial.print("angle : ");
  Serial.println(angle);
*/

  //엑셀값이 브레이크값보다 크므로 전진
  if(accel > brake){
    d = accel-brake;
    velocity = (float)(d/2);

    //직진 셋팅
    if( angle == 90 ){
      if(diff_max_lv != diff_max_rv){
        lv = 1600;
        rv = 1400;
      }
      else if(diff_max_lv == diff_max_rv){
        if( lv < 1600 ){
          if(diff_max_lv < velocity){        
            lv = 1600;          
          }
          else if(diff_max_lv >= velocity){
            lv += velocity;
          }
        }
        if( rv > 1400 ){
          if(diff_max_rv < velocity){
           rv = 1400;        
         }
          else if(diff_max_rv >= velocity){
           rv -= velocity;
          }        
        }
      }
    }

    //좌회전 셋팅
    else if( angle >= 0 && angle < 90 ){
      lv = 1510+angle;
      rv = 1400;      
    }

    //우회전 셋팅
    else if( angle > 90 && angle <= 180 ){      
      lv = 1600;
      rv = 1410+(angle-90);
    }       
  }  

  //엑셀값이 브레이크값보다 작으므로 브레이킹
  else if(accel < brake){
    d = brake - accel;
    velocity = (float)(d/2);
    if(rv < 1500){
      if(diff_min_rv < velocity){
        rv = 1500;
      }
      else if(diff_min_rv >= velocity){
        rv += velocity;
      }
    }
    if( lv > 1500 ){
      if(diff_min_lv < velocity){
        lv = 1500;
      }
      else if(diff_min_lv >= velocity){
        lv -= velocity;
      }
    }
  }

  //엑셀값과 브레이크값이 같으므로 서서히 감속 (실체 차량 페달 뗀것과 동일)
  else if(accel == brake){
    if((lv != 1500) && (rv != 1500)){
      lv--;
      rv++;
    }
    else if((lv == 1500) && (rv != 1500)){
      rv++;
    }
    else if((lv != 1500) && (rv == 1500)){
      lv--;
    }
  }

  servoL.writeMicroseconds(lv);
  servoR.writeMicroseconds(rv);
}

