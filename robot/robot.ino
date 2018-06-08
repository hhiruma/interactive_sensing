#include <Chrono.h>
#include <LightChrono.h>

Chrono myChrono;
Servo myServo;
Servo myServo2;

//楽譜，０１２３で左から順,4は叩かない
int rightSheet[] = {4,4,4,4};
int leftSheet[] = {4,4,4,4};
int nowtime = 0;
int nowPlace[] = {0,0}; //[0]がR,[1]がL

void setup() {
  // put your setup code here, to run once:
  myServo.attach(9); // サーボを９ピンにつける
  myServo2.attach(8); // サーボを９ピンにつける
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(myChrono.hasPassed(500)){
    //ここで楽譜を読んでアーム移動
    
    if(rightSheet[(nowtime+1)%4] =! 4){
      moveArm(rightSheet[(nowtime+1)%4]-nowPlace[0]);
      nowPlace[0] = rightSheet[(nowtime+1)%4]
    }
    if(leftSheet[(nowtime+1)%4] =! 4){
      moveArm(leftSheet[(nowtime+1)%4]-nowPlace[1]);
      nowPlace[1] = leftSheet[(nowtime+1)%4]
    }
  }
  
  if(myChrono.hasPassed(1000)){
    //ここで叩いてカウント初期化
    hit();

    //nowtimeが大きくなりすぎたとき用
    if(nowtime < 5001){
      nowtime++;
    }else{
      nowtime = (nowtime+1)%4;
    }
    myChrono.restart();
  }
}

void moveArm(int sa){
  if(abs(sa) >1){
    if(abs(sa) == 3){
      //差が３
      
    }else{
      //差が２
      
    }
  }else if(abs(sa) == 1){
    //差が１
    
  }else{
    //この時は差は０なのでそのまま
  }
}

void hit(){
  //ここで両手を叩かせる，今考えているのは角度を分割してfor文で交互に回すことによって同時のように見えさせる
  
}




