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
#define NOTE_LEN 50
#define DRUM_NUM 6

#define HIT 0
#define MOVE 1
#define ACTION_TIME 200

//時間計測器
Chrono timer;

//サーボの宣言
Servo servos[2][2];

//肩の位置の宣言 {RIGHT, LEFT}
const int shoulderAngle[2][DRUM_NUM/2] = {{55, 90, 134}, {55, 90, 134}};

//楽譜関連
//  int型の楽譜読込先(ここに音が演奏されるタイミングをすべて書いていく)
int int_notes[NOTE_LEN][2];
//  編集した楽譜
//   {タイミング, MOVE/HIT, 音}　
int compiled_note[NOTE_LEN*2][3];
//  読み込み中判定
boolean reading_notes;
//  完成した楽譜を持ってるか判定
boolean has_complete_sheet;
//  楽譜の番号
int note_counter;
//  終了時刻
int global_sheet_time;
//  演奏済みの番号
int note_position;
//　コンパイルされた楽譜の長さを格納
int compiled_notes_len;

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

  if (!has_complete_sheet){
    return;
  }

  if(note_position >= compiled_notes_len){
    if(timer.hasPassed(global_sheet_time)){
      note_position = 0;
      timer.restart();
    }
  } else if(timer.hasPassed(compiled_note[note_position][0])){
    //ちょうど通過したら
    //腕を確認
    int arm = getArm(compiled_note[note_position][2]);

    //HIT / MOVEを確認
    switch(compiled_note[note_position][1]){
      case HIT:
        Serial.print("HIT: time=");
        Serial.print(compiled_note[note_position][0]);
        Serial.print(" note=");
        Serial.println(compiled_note[note_position][2]);
        hit(arm);
        break;
      case MOVE:
        Serial.print("MOVE: time=");
        Serial.print(compiled_note[note_position][0]);
        Serial.print(" note=");
        Serial.println(compiled_note[note_position][2]);
        moveArm(arm, compiled_note[note_position][2]);
    }

    note_position++;
  }
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
        compileNotes(note_counter);
        return;

        //読み込み終わった瞬間にタイマーをリスタートする
        timer.restart();
      } else {
        //形式通りの楽譜ならば
        convertToIntNote(note_counter, input);
        note_counter++;
      }
    }
  }
}

void convertToIntNote(int counter, String input){
  int input_note = ((String)input.charAt(0)).toInt();
  int input_time = input.substring(4).toInt();

  int_notes[counter][0] = input_note;
  int_notes[counter][1] = input_time;
}

void compileNotes(int len){
  boolean firstNote = true;
  int counter = 0;
  int currentArm;
  int prevArm;

  for(int i=0; i<len; i++){
    //１つ前の楽譜で呼び出された音で場合分けする
    //    ∵それによってdelayのかけ方が変わってくるから
    //    memo: １回の行動にかかる時間はACTION_TIMEmsとする
    if (firstNote){
      //今回が最初に演奏される音だった
      firstNote = false;

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
      compiled_note[counter][2] =  int_notes[i][0];
      counter++;
    } else {
      //２回め以降
      currentArm = getArm(int_notes[i]);
      prevArm = getArm(int_notes[i-1]);

      int prevSameArmIndex;
      if(prevArm != currentArm){
        //前回別の腕を動かしていた場合

        //今回使用する腕を前回使用したindexを得る
        for(int j=i-1;;j--){
          if(j < 0){
            Serial.println("no same arm used previously");
            /*
             * 前に同じ腕を使用したことがない場合の対処法未実装
             */
            prevSameArmIndex = -1;
            break;
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
        compiled_note[counter][2] =  int_notes[i][0];
        counter++;
      } else {
        //違う場合

        //前回打ったタイミングと今回打つタイミングの差がACTION_TIMEms以下の場合ずらすかどうかを判定して実行する
        shiftTimeDiffIfOverlapp(i, prevSameArmIndex, len, ACTION_TIME*2);

        compiled_note[counter][0] =  int_notes[i][1]-ACTION_TIME;
        compiled_note[counter][1] =  MOVE;
        compiled_note[counter][2] = int_notes[i][0];
        counter++;

        compiled_note[counter][0] =  int_notes[i][1];
        compiled_note[counter][1] =  HIT;
        compiled_note[counter][2] =  int_notes[i][0];
        counter++;
      }

      prevArm = currentArm;
    }
  }
  //コンパイルされた楽譜の長さを格納する
  compiled_notes_len = counter;
}


void shiftTimeDiffIfOverlapp(int index, int prevIndex, int arrLen, int aTime){
  //前回打ったタイミングと今回打つタイミングの差がACTION_TIMEms以下の場合
  int timeDiff = int_notes[index][1] - int_notes[prevIndex][1];

  if(timeDiff < aTime){
    int incrementTime;

    if(timeDiff >= 0){
      incrementTime = aTime - timeDiff;
    } else {
      incrementTime = aTime + timeDiff;
    }

    //指定index以降のdiff[ms]だけすべての楽譜をずらす
    for(int i=index; i<arrLen; i++){
      int_notes[i][1] += incrementTime;
    }
  }

  /*
  * memo: a_timeを指定しているのは場合によってずらす時間が変わるから
  *　　　-> 動かさない場合は200ms, 動かす場合は400ms
  */
}

int getArm(int note){
  //演奏する音の番号から腕を返す
  if(note >= 1 && note <=3){
    return LEFT;
  } else if (note >= 4 && note <= 6){
    return RIGHT;
  } else {
    return -1;
  }
}

void moveArm(int arm, int nextPos) {
  if(nextPos == -1) return;

  int dest;

  switch(nextPos){
    case 1:
      dest = shoulderAngle[LEFT][0]; break;
    case 2:
      dest = shoulderAngle[LEFT][1]; break;
    case 3:
      dest = shoulderAngle[LEFT][2]; break;
    case 4:
      dest = shoulderAngle[RIGHT][0]; break;
    case 5:
      dest = shoulderAngle[RIGHT][1]; break;
    case 6:
      dest = shoulderAngle[RIGHT][2]; break;
  }

  if (arm == RIGHT) {
    servos[RIGHT][SHOULDER].write(dest);
  } else {
      servos[LEFT][SHOULDER].write(dest);
  }
}

void hit(int arm) {
  int makeDelay = 100;

  switch (arm) {
    case BOTH:
      //打つ
      servos[RIGHT][ELBOW].write(130);
      servos[LEFT][ELBOW].write(50);
      delay(makeDelay);

      //戻す
      servos[RIGHT][ELBOW].write(165);
      servos[LEFT][ELBOW].write(15);
      delay(makeDelay);
      break;

    case RIGHT:
      //右腕
      servos[RIGHT][ELBOW].write(130);
      delay(makeDelay);
      servos[RIGHT][ELBOW].write(165);
      delay(makeDelay);
      break;

    case LEFT:
      //左腕
      servos[LEFT][ELBOW].write(50);
      delay(makeDelay);
      servos[LEFT][ELBOW].write(15);
      delay(makeDelay);
      break;
  }
}
