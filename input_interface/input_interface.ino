using namespace std;

#include <Chrono.h>

#define WAIT 0
#define RECORD 1

//システムのモードを入れる
int mode;

//モード切り替えピン
const int mode_toggle_pin = 7;

//楽譜入力ピン
const int note_input_pin = 8;

//楽譜用のタイマー
Chrono timer;

void setup() {
  Serial.begin(9600);
  pinMode(mode_toggle_pin, INPUT);
  pinMode(note_input_pin, INPUT);

  //初期のモードは待機
  mode = WAIT;
}

void loop() {
  if (mode == WAIT) {
    //待機状態の場合
    int in_toggle = digitalRead(mode_toggle_pin);

    //toggle用のボタンが押されたらモードを切り替えて終了
    if (in_toggle) {
      mode = RECORD;
      Serial.println("RECORD START");
      delay(500);
      timer.restart();
      return;
    }
  } else {
    //録音状態の場合
    int in_toggle = digitalRead(mode_toggle_pin);
    int in_note = digitalRead(note_input_pin);

    if (in_toggle) {
      mode = WAIT;
      Serial.println("RECORD END");
      delay(500);
      return;
    }
    if (in_note) {
      //仮で入力する値を0番目の値とする
      int tmp_input_note = 0;
      
      /*
       * 実装予定：
       *   楽譜をprocessingに送って配列に入れる
       */
       
      Serial.print("note = ");
      Serial.print(tmp_input_note);
      Serial.print(", time =");
      Serial.println(timer.elapsed());
      delay(200);
    }
  }
}
