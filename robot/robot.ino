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
#define NOTE_LEN 100
#define DRUM_NUM 6

#define HIT 0
#define MOVE 1
#define ACTION_TIME 200

//時間計測器
Chrono timer;

//サーボの宣言
Servo servos[2][2];

//肩の位置の宣言
const int shoulderAngle[2][DRUM_NUM/2] = {{134, 90, 55}, {134, 90, 55}};

//楽譜関連
//  読み込み先
String str_notes[NOTE_LEN];
//  int型の楽譜読込先(ここに音が演奏されるタイミングをすべて書いていく)
int int_notes[NOTE_LEN][2];
//  編集した楽譜
int compiled_note[NOTE_LEN*2][3];
//  読み込み中判定
boolean reading_notes;
//  完成した楽譜を持ってるか判定
boolean has_complete_sheet;
//  楽譜の番号
int note_counter;
//  終了時刻
int global_sheet_time;

//次に打つ位置
int rightNextPos;
int leftNextPos;

void setup() {
  //サーボの割り当て
  servos[RIGHT][SHOULDER].attach(8);  // 8 : 右肩
  servos[RIGHT][ELBOW].attach(9);  // 9 : 右肘
  servos[LEFT][SHOULDER].attach(10);  // 10: 左肩
  servos[LEFT][ELBOW].attach(11);  // 11: 左肘

  //シリアル通信
  Serial.begin(9600);

  //楽譜読み込み状態をfalseに設定
  reading_notes = false;
  has_complete_sheet = false;
}

void loop() {
  if (Serial.available() > 0){
    readSerial();

    //Serial通信を受け取った場合はロボットは動かないようにする
    //   ∵ 譜面が更新されているタイミングで動かすとサーボに変な入力しそう
    return;
  }

  // //T_MOVE ~ 5sec： reset直後、腕を動かす時間
  // if (timer.hasPassed(T_MOVE) && timer.hasPassed(T_MOVE+5)) {
  //   int nextCount = hitCounter + 1;
  //   //ここで楽譜を読んでアーム移動

  //   //次の位置
  //   rightNextPos = sheet[RIGHT][nextCount % SHEET_LEN];
  //   leftNextPos = sheet[LEFT][nextCount % SHEET_LEN];

  //   //次の位置に移動
  //   moveArm(RIGHT, rightNextPos);
  //   //角度が未実装なため左のmoveArmはコメントアウト
  //   moveArm(LEFT , leftNextPos);
  // }

  // //T_IHT ~ 5sec: 腕を動かした後、打つタイミング
  // if (timer.hasPassed(T_HIT) && !timer.hasPassed(T_HIT+5)) {
  //   if (sheet[RIGHT][hitCounter % SHEET_LEN] != -1) {
  //     if (sheet[LEFT][hitCounter % SHEET_LEN] != -1) {
  //       hit(BOTH);
  //     } else {
  //       hit(RIGHT);
  //     }
  //   } else {
  //     if (sheet[LEFT][hitCounter % SHEET_LEN] != -1) {
  //       hit(LEFT);
  //     }
  //   }
  // }


  // //T_RESET: 全ての作業が終わってると見込まれる時間
  // if (timer.hasPassed(T_RESET)) {
  //   //hitCounterが大きくなりすぎたとき用
  //   if (hitCounter < 5001) {
  //     hitCounter++;
  //   } else {
  //     hitCounter = (hitCounter + 1) % SHEET_LEN;
  //   }

  //   //タイマーをリセットする
  //   timer.restart();
  // }

}

void readSerial(){
  //「;」が出てくるまで読み込み続けて、String型に落とし込む
  String input = Serial.readStringUntil(';');

  if(!reading_notes){
    //記録中で無かった場合
    if(input.equals("start")){
      //楽譜の最初の行を受け取ったら
      reading_notes = true;
      has_complete_sheet = false;

      //楽譜の番号も0に初期化
      note_counter = 0;
    }
  } else {
    //記録中の場合
    if(input.indexOf(',') != -1) {
      //最後の場合
      if(input.indexOf("end") != -1){
        global_sheet_time = input.substring(4).toInt();

        //楽譜の最後の行を受け取ったら
        reading_notes = false;
        has_complete_sheet = true;

        //読み込んだString型のArrayを元にint型のArrayを生成する
        makeIntNotes(note_counter);
        compileNotes(note_counter);
        return;
//        setPlayTimer(note_counter);
      } else {
        //形式通りの楽譜ならば
        str_notes[note_counter] = input;
        note_counter++;
      }
    }
  }
}

void makeIntNotes(int len){
  for(int i=0; i<len; i++){
    String tmp = str_notes[i];

    //input1:  一文字目に演奏する番号が必ず入ってくるので一文字目を取得
    int input1 = int(tmp.charAt(0));
    //input2:  カンマ以降（4文字目以降）がタイミングになるのでそれを取得
    int input2 = tmp.substring(4).toInt();

    int_notes[i][0] = input1;
    int_notes[i][1] = input2;
  }
}

void compileNotes(int len){
  int prevNote = -1;
  int counter = 0;

  for(int i=0; i<len; i++){
    //使用する腕を求める
    int currentArm = getArm(int_notes[i]);
    int prevArm = getArm(prevNote);

    //１つ前の楽譜で呼び出された音で場合分けする
    //    ∵それによってdelayのかけ方が変わってくるから
    //    memo: １回の行動にかかる時間はACTION_TIMEmsとする
    if (prevArm == -1){
      //１個前が-1　-> 今回が最初に演奏される音
      if(counter != 0) Serial.println("error");

      //とりあえず腕の位置を動かす
      int tmp = int_notes[i][1] - ACTION_TIME;
      if(int_notes[i][1] < ACTION_TIME){
        tmp = 0;
      }
      compiled_note[counter][0] =  tmp;
      compiled_note[counter][1] =  MOVE;
      compiled_note[counter][2] = int_notes[i][0];
      counter++;

      //そのまま打つ
      compiled_note[counter][0] =  int_notes[i][1];
      compiled_note[counter][1] =  HIT;
      compiled_note[counter][2] = -1;
      counter++;
    } else {
      //２回め以降

      int prevSameArmIndex;
      if(prevArm != currentArm){
        //前回別の腕を動かしていた場合

        //今回使用する腕を前回使用したindexを得る
        for(int j=i-1;;j--){
          if(j < 0){
            Serial.println("error");
          }
          if (getArm(int_notes[j]) == getArm(int_notes[i])){
            prevSameArmIndex = j;
            break;
          }
        }
      } else {
        prevSameArmIndex = i - 1;
      }

      //今回と前回の位置が一緒かを調べる
      if(int_notes[prevSameArmIndex][0] == int_notes[i][0]){
        //一緒の場合

        //前回打ったタイミングと今回打つタイミングの差がACTION_TIMEms以下の場合ずらすかどうかを判定して実行する
        shiftTimeDiffIfOverlapp(i, prevSameArmIndex, len, ACTION_TIME);

        compiled_note[counter][0] =  int_notes[i][1];
        compiled_note[counter][1] =  HIT;
        compiled_note[counter][2] = -1;
        counter++;
      } else {
        //違う場合

        //前回打ったタイミングと今回打つタイミングの差がACTION_TIMEms以下の場合ずらすかどうかを判定して実行する
        shiftTimeDiffIfOverlapp(i, prevSameArmIndex, len, ACTION_TIME*2);

        compiled_note[counter][0] =  int_notes[i][1]-200;
        compiled_note[counter][1] =  MOVE;
        compiled_note[counter][2] = int_notes[i][0];
        counter++;

        compiled_note[counter][0] =  int_notes[i][1];
        compiled_note[counter][1] =  HIT;
        compiled_note[counter][2] = -1;
        counter++;
      }

      prevArm = currentArm;
    }
  }

  Serial.println("done");
}

void shiftTimeDiffIfOverlapp(int index, int prevIndex, int arrLen, int a_time){
  //前回打ったタイミングと今回打つタイミングの差がACTION_TIMEms以下の場合
  int timeDiff = int_notes[index][1] - int_notes[prevIndex][1];
  if(timeDiff < a_time){
    //指定index以降のdiff[ms]だけすべての楽譜をずらす
    for(int i=index; i<arrLen; i++){
      int_notes[i][1] += timeDiff;
    }
  }

  /*
  * memo: a_timeを指定しているのは場合によってずらす時間が変わるから
  *　　　-> 動かさない場合は200ms, 動かす場合は400ms
  */
}

int getArm(int note){
  //演奏する音の番号から腕を返す
  if(note >= 0 && note <=2){
    return RIGHT;
  } else if (note >= 3 && note <= 5){
    return LEFT;
  } else {
    return -1;
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
      delay(100);

      //戻す
      servos[RIGHT][ELBOW].write(165);
      servos[LEFT][ELBOW].write(15);
      delay(100);
      break;

    case RIGHT:
      //右腕
      servos[RIGHT][ELBOW].write(130);
      delay(100);
      servos[RIGHT][ELBOW].write(165);
      delay(100);
      break;

    case LEFT:
      //左腕
      servos[LEFT][ELBOW].write(50);
      delay(100);
      servos[LEFT][ELBOW].write(15);
      delay(100);
      break;
  }
}
