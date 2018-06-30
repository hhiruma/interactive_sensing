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
const int shoulderAngle[2][DRUM_NUM/2] = {{134, 90, 55}, {134, 90, 55}};

//楽譜の長さ設定
const int sheetLen = 4;

//楽譜関連
//  読み込み先
String str_notes[100];
//  読み込み中判定
boolean reading_notes;
//  楽譜の番号
int note_counter;

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
//  for (int i = 0; i < sheetLen; i++) {
//    sheet[RIGHT][i] = -1;
//    sheet[LEFT][i] = -1;
//  }

  //シリアル通信
  Serial.begin(9600);

  //楽譜読み込み状態をfalseに設定
  reading_notes = false;
}

void loop() {
  if (Serial.available() > 0){
    readSerial();
  }

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
    moveArm(LEFT , leftNextPos);
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

void readSerial(){
  //「;」が出てくるまで読み込み続けて、String型に落とし込む
  String input = Serial.readStringUntil(';');

  if(!reading_notes){
    //記録中で無い場合
    if(input.equals("start")){
      //楽譜の最初の行を受け取ったら
      reading_notes = true;

      //楽譜の番号も0に初期化
      note_counter = 0;
    }
  } else {
    //記録中の場合
    if (input.equals("end")){
      //楽譜の最後の行を受け取ったら
      reading_notes = false;
    } else if(input.indexOf(',') != -1) {
      //形式通りの楽譜ならば
      str_notes[note_counter] = input;
      note_counter++;
    }
  }
}

void moveArm(int arm, int nextPos) {
  if(nextPos == -1) return;

  if (arm == RIGHT) {
    servos[RIGHT][SHOULDER].write(shoulderAngle[RIGHT][nextPos]);
  } else {
    servos[LEFT][SHOULDER].write(shoulderAngle[LEFT][nextPos]);
  }
}

void hit(int arm) {
  switch (arm) {
    case BOTH:
      //打つ
      servos[RIGHT][ELBOW].write(130);
      servos[LEFT][ELBOW].write(50);
      delay(200);

      //戻す
      servos[RIGHT][ELBOW].write(165);
      servos[LEFT][ELBOW].write(15);
      delay(200);
      break;

    case RIGHT:
      //右腕
      servos[RIGHT][ELBOW].write(130);
      delay(200);
      servos[RIGHT][ELBOW].write(165);
      delay(200);
      break;

    case LEFT:
      //左腕
      servos[LEFT][ELBOW].write(50);
      delay(200);
      servos[LEFT][ELBOW].write(15);
      delay(200);
      break;
  }
}
