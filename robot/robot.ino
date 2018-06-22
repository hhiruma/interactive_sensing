#include <Chrono.h>
#include <LightChrono.h>
#include <Servo.h>

#define RIGHT 0
#define LEFT 1
#define BOTH 2
#define SHOULDER 0
#define ELBOW 1

#define T_MOVE 0
#define T_HIT 500
#define T_RESET 1000

#define SHEET_LEN 4
#define DRUM_NUM 6

//時間計測器
Chrono timer;

//サーボの宣言
Servo servos[2][2];

//肩の位置の宣言
/*
 * 左肩の角度に関してはまだ未実装なので修正する必要あり
 */
const int shoulderAngle[2][DRUM_NUM/2] = {{165, 90, 15}, {15, 15, 15}};

//楽譜の長さ設定
const int sheetLen = 4;

//楽譜，０１2で左から順,-1は叩かない
int sheet[2][SHEET_LEN];
//sheet[0]: 右譜面
//sheet[1]: 左譜面

//打った回数を入れるカウンタ
int hitCounter = 0;

//次に打つ位置
int rightNextPos;
int leftNextPos;

void setup() {
  //サーボの割り当て
  servos[RIGHT][SHOULDER].attach(8);  // 8 : 右肩
  servos[RIGHT][ELBOW].attach(9);  // 9 : 右肘
  servos[LEFT][SHOULDER].attach(10);  // 10: 左肩
  servos[LEFT][ELBOW].attach(11);  // 11: 左肘

  //楽譜の初期化
  for (int i = 0; i < sheetLen; i++) {
    sheet[RIGHT][i] = -1;
    sheet[LEFT][i] = -1;
  }
}

void loop() {
  //T_MOVE ~ 5sec： reset直後、腕を動かす時間
  if (timer.hasPassed(T_MOVE) && timer.hasPassed(T_MOVE+5)) {
    int nextCount = hitCounter + 1;
    //ここで楽譜を読んでアーム移動

    //次の位置
    rightNextPos = sheet[RIGHT][nextCount % SHEET_LEN];
    leftNextPos = sheet[LEFT][nextCount % SHEET_LEN];

    //次の位置に移動
    moveArm(RIGHT, rightNextPos);
    //角度が未実装なため左のmoveArmはコメントアウト
//    moveArm(LEFT , leftNextPos);
  }



  //T_IHT ~ 5sec: 腕を動かした後、打つタイミング
  if (timer.hasPassed(T_HIT) && !timer.hasPassed(T_HIT+5)) {
    if (sheet[RIGHT][hitCounter % SHEET_LEN] != -1) {
      if (sheet[LEFT][hitCounter % SHEET_LEN] != -1) {
        hit(BOTH);
      } else {
        hit(RIGHT);
      }
    } else {
      if (sheet[LEFT][hitCounter % SHEET_LEN] != -1) {
        hit(LEFT);
      }
    }
  }


  //T_RESET: 全ての作業が終わってると見込まれる時間
  if (timer.hasPassed(T_RESET)) {
    //hitCounterが大きくなりすぎたとき用
    if (hitCounter < 5001) {
      hitCounter++;
    } else {
      hitCounter = (hitCounter + 1) % SHEET_LEN;
    }

    //タイマーをリセットする
    timer.restart();
  }

}

void moveArm(int arm, int nextPos) {
  if(nextPos == -1) return;
  
  if (arm == RIGHT) {
    servos[RIGHT][SHOULDER].write(shoulderAngle[RIGHT][nextPos]);
  } else {
    //左腕ようの何かしらの計算が必要...?

    /*
      int new_nextPos = nextPos * ...
    */
    servos[LEFT][SHOULDER].write(nextPos);
    delay(100);
  }
}

void hit(int arm) {
  switch (arm) {
    case BOTH:
      //右腕
      servos[RIGHT][ELBOW].write(110);
      servos[LEFT][ELBOW].write(70);
      delay(200);

      //左腕
      servos[RIGHT][ELBOW].write(165);
      servos[LEFT][ELBOW].write(15);
      delay(200);
      break;

    case RIGHT:
      //右腕
      servos[RIGHT][ELBOW].write(110);
      delay(200);
      servos[RIGHT][ELBOW].write(165);
      delay(200);
      break;

    case LEFT:
      //左腕
      servos[LEFT][ELBOW].write(70);
      delay(200);
      servos[LEFT][ELBOW].write(15);
      delay(200);
      break;
  }
}
