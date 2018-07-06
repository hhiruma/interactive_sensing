#include <Chrono.h>
#include <LightChrono.h>
#include <Servo.h>
#include <TimedAction.h> //読み込めるかはまだチェックしてない

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

//時間計測器
Chrono timer;

//サーボの宣言
Servo servos[2][2];

//肩の位置の宣言
const int shoulderAngle[2][DRUM_NUM/2] = {{134, 90, 55}, {134, 90, 55}};

//楽譜関連
//  読み込み先
String str_notes[NOTE_LEN];
//  int型の楽譜読込先 (直接使用することは無いが、デバッグ用に作っておく)
int int_notes[NOTE_LEN][2];
//  TimedAction型の楽譜読込先
TimedAction* playThreads;
//  TimedAction型の音再生タイミングアラーム読込先
TimedAction* playTimeSetter;
//  読み込み中判定
boolean reading_notes;
//  完成した楽譜を持ってるか判定
boolean has_complete_sheet;
//  楽譜の番号
int note_counter;
//  TimedActionにのコールバックに渡す時に使うためのグローバル変数
int global_selected_note;
//  終了時刻
int global_sheet_time;
//  初回ループ判定
boolean is_setup_loop;

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
  is_setup_loop = true;
}

void loop() {
  if (Serial.available() > 0){
    readSerial();

    //Serial通信を受け取った場合はロボットは動かないようにする
    //   ∵ 譜面が更新されているタイミングで動かすとサーボに変な入力しそう
    return;
  }

  if(!has_complete_sheet) {
    //最初に完璧な楽譜を持っていない状態では何もしない
    return;
  }
  
//  //セットアップ用の初回周期では
//  if(is_setup_loop){
//    //int_notesの各音に対して該当する値があるか確認
////    Serial.println(String(timer.elapsed()));
//    for(int i=0; i<note_counter; i++){
//      if(timer.hasPassed(int_notes[i][1]) && !timer.hasPassed(int(int_notes[i][1]) + 200)){
//        //タイミングが合ったら、スレッドを有効化する
//        playThreads[i].reset();
//        playThreads[i].enable();
//        String tmp = "enable : " + i;
//        Serial.println("enable");
//
//        if(i == note_counter-1){
//          //最後の音のセットアップが完了したら
//          is_setup_loop = false;
//          Serial.println("setup done");
//        }
//      }
//    }
//    return;
//  } else {
    for(int i=0; i<note_counter; i++){
      playThreads[i].check();
    }
//  }
}

void readSerial(){
  //「;」が出てくるまで読み込み続けて、String型に落とし込む
  String input = Serial.readStringUntil(';');

  if(!reading_notes){
    //記録中で無い場合
    if(input.equals("start")){
      //楽譜の最初の行を受け取ったら
      reading_notes = true;
      is_setup_loop = true;
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
        makeTimedActionNotes(note_counter);
        setPlayTimer(note_counter);
      } else {
        //形式通りの楽譜ならば
        str_notes[note_counter] = input;
        note_counter++;
      }
    }
  }
}

int global_next_note_num = 0;
void setPlayTimer(int len){
  Serial.println("setPlayTimer");
//  playTimeSetter = malloc(len);
  for(int i=0; i<len; i++){
    global_next_note_num = i;
    playTimeSetter[i] = TimedAction(int_notes[i][1], enablePlayer);
  }
}

void enablePlayer(){
  Serial.println(String(global_next_note_num));
  playThreads[global_next_note_num].enable();
}

void makeIntNotes(int len){
  Serial.println("makeIntNotes");
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

void makeTimedActionNotes(int len){
  Serial.println("makeTimedActionNotes");
//  playThreads = malloc(len);
  //バグ発生の可能性: makeIntNotesが全部実行し終わるまで待つのか...?
  for(int i=0; i<len; i++){
    global_selected_note = int_notes[i][0];
    playThreads[i] = TimedAction(global_sheet_time, playNote);
    //作った段階ではまだ無効化しておく
    playThreads[i].disable();
  }
}

void playNote(){
  /***/
  Serial.println("playNote executed");
  /***/
  
  //TimeActionはコールバックに引数を渡せないためグローバルに一旦入れておいたので、それを持ってくる
  int note = global_selected_note;
  //エラー検知用に-1に戻しておく
  global_selected_note = -1;

  //腕を選択する
  int chosenArm = getArm(note);
  //腕を動かす
  moveArm(chosenArm, note);
  //打つ
  hit(chosenArm);
}

int getArm(int note){
  //演奏する音の番号から腕を返す

  //一時的にRIGHTを返すようにしておく
  return RIGHT;
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
