#include <Chrono.h>

#define WAIT 0
#define RECORD 1

//システムのモードを入れる
int mode;

//モード切り替えピン
const int mode_toggle_pin = 7;

//楽譜入力ピン
const int note_input_pin = 8;

//RECORD状態表示用LED
const int record_LED_pin = 13;

//楽譜用のタイマー
Chrono timer;

void setup() {
  Serial.begin(9600);
  pinMode(mode_toggle_pin, INPUT);
  pinMode(note_input_pin, INPUT);
  pinMode(record_LED_pin, OUTPUT);

  //初期モードはWAITなのでLOWに
  digitalWrite(record_LED_pin, LOW);

  //初期のモードは待機
  mode = WAIT;
}

void loop() {
  if (mode == WAIT) {
    //待機状態の場合
    //LED表示をOFFに
    digitalWrite(record_LED_pin, LOW);
    //トグル用のボタンのみ読み込む
    int in_toggle = digitalRead(mode_toggle_pin);

    //toggle用のボタンが押されたらモードを切り替えて終了
    if (in_toggle) {
      //processingに文字を送っても比較で失敗するのでSTARTの意味として-1を送る
      Serial.println("-1");
      //モードをRECORDに切り替える
      mode = RECORD;
      delay(200);
      //このタイミングでタイマーをリセットする
      timer.restart();
      return;
    }
  } else {
    //録音状態の場合
    //LED表示をONに
    digitalWrite(record_LED_pin, HIGH);
    //トグル用のボタンと音用のボタンを読み込む
    int in_toggle = digitalRead(mode_toggle_pin);
    int in_note = digitalRead(note_input_pin);

    if (in_toggle) {
      //processingに文字を送っても比較で失敗するのでENDの意味として-2を送る
      Serial.println("-2");
      //モードをWAITに切り替える
      mode = WAIT;
      delay(200);
      return;
    }
    if (in_note) {
      //RECORDモードで音用ボタンが押された場合
      //仮で入力する値を0番目の値とする
      /*
       * ボタンによって入力する値を変えるように実装する予定
       */
      int tmp_input_note = 1;

      //入力値を送る
      Serial.println(tmp_input_note);
      delay(100);

      //入力された時刻を送る
      float time_value = float(timer.elapsed()) / 1000;
      Serial.println(time_value, 5);
      delay(100);
    }
  }
}
