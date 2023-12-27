


int speedR = 255;  //You can adjust the speed of the wheel. (IO12, IO13)
int speedL = 255;  //You can adjust the speed of the wheel. (IO14, IO15)
float decelerate = 0.4;   // value = 0-1

void servo_rotate(int channel, int angle) {
    int val = 7864-angle*34.59; 
    if (val > 7864)
       val = 7864;
    else if (val < 1638)
      val = 1638; 
    ledcWrite(channel, val);
}

void move_front(void){
  ledcWrite(5,speedR);
  ledcWrite(6,0);
  ledcWrite(7,speedL);
  ledcWrite(8,0);    
}

void move_left(void){
  ledcWrite(5,speedR*decelerate);
  ledcWrite(6,0);
  ledcWrite(7,0);
  ledcWrite(8,speedL*decelerate);       
}

void move_right(void){
  ledcWrite(5,0);
  ledcWrite(6,speedR*decelerate);
  ledcWrite(7,speedL*decelerate);
  ledcWrite(8,0);   
}

void move_stop(void){
   ledcWrite(5,0);
   ledcWrite(6,0);
   ledcWrite(7,0);
   ledcWrite(8,0);  
}

void move_back(void){
   ledcWrite(5,0);
   ledcWrite(6,speedR);
   ledcWrite(7,0);
   ledcWrite(8,speedL); 
}


void move_frontleft(void){   
    ledcWrite(5,speedR);
    ledcWrite(6,0);
    ledcWrite(7,speedL*decelerate);
    ledcWrite(8,0);   
}
void move_frontright(void){ 
     ledcWrite(5,speedR*decelerate);
     ledcWrite(6,0);
     ledcWrite(7,speedL);
     ledcWrite(8,0);   
}  
void move_leftafter(void){
     ledcWrite(5,0);
     ledcWrite(6,speedR);
     ledcWrite(7,0);
     ledcWrite(8,speedL*decelerate);
} 
void move_rightafter(void){
              
      ledcWrite(5,0);
      ledcWrite(6,speedR*decelerate);
      ledcWrite(7,0);
      ledcWrite(8,speedL);
}  
