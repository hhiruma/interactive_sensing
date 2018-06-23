[名前] （そういや名前つけないと)
====
# ドラム関連
## Arduino側
圧電スピーカーを入力として登録し、それを振動を検知するセンサーとして用いる。  
ループごとに各センサーの値を読み込み、その値が0以上であった場合はSerial通信でProcessing側に自身のセンサーの番号を送信する。  

## Processing側
Arduinoから振動が検知されたスピーカーの番号がSerial通信より送られて来る。それに対応するピッチの値を```pitchLIst```から選ぶ。選ばれたピッチは```MIDIBus```というライブラリを用いてchannel, pitch, velocityの値としてMIDIデータが送信される。  
PC側からマウスおよびキーボードでの操作もできるように画面にドラムを出力するようにした。

## Mac側
MacのVirtual MIDI Busという機能を用いてProcessingから出力されたMIDIデータをGarageBandに送信する。  
仲介役  

## GarageBand側
ドラムキットを起動しておく。Virtual MIDI Busを自動で入力装置として検知するためとくにすることはない。  
MIDIデータが勝手にドラムのトラックに送信されてデータのpitchに対応する音が出力される。

---
# ロボット
# Arduino側
左右の肩と肘、計4箇所にサーボモーターを接続。それぞれ8~11ピンに割り当てた。  
トリガーは3箇所設けた。T_MOVE, T_HIT, T_RESET。

## T_MOVE（0ms ~ 5ms)
肩が動き出すタイミングで、動作の一番最初に当たる。  
次に腕を動かす位置を取得し、```moveArm()```でサーボモーターを稼働させる。

## T_HIT (500ms ~ 505ms)
肩を動かしたあと、打つタイミング。  
楽譜的に打つ腕を判定して```hit()```を実行する。

## T_RESET (1000ms)
全ての動作が終わったと見込まれる時間。

# Processing側
※PCから直接キーボードで操作できるように作成するかも。その場合自動・手動モードを切り替える機能を実装する必要あり。

----
# 入力インターフェース
## Arduino側
Arduino側のコードはinput_interface.inoが担当。  
モード切り替えボタンと、楽譜入力用の音ボタンの入力を受け付ける。  
Chronoで経過時間を測る。  
WAITモードとRECORDモードを切り替えながら使用する。  
モードの表示にArduino基盤上のLEDを使用する（RECORDモードの時のみ点灯）

### WAITモード
モード切り替えボタンが押されたらモードをRECORDに変更。  
RECORDにしたという情報をprocessingに「-1」という値で送る。  
このタイミングでタイマーを0に戻す

### RECORDモード
モード切り替えボタンが押されたらモードをWAITに変更。  
WAITにしたという情報をprocessingに「-2」という値で送る。  

音ボタンが入力されたら、
-押されたボタンの数字（6/24現在未実装）
-押された時の時刻（遅れる値のサイズの関係でmsからsに単位を変更）
連続でprocessingに送る。

## Processing側
Processing側のコードはscore_sender.pdeが担当。  
Arduinoに合わせてモードを切り替えるようにする。それをrecordingというbooleanの値に入れてお
-Arduinoから「-1」が送られたら```recording=true```
-Arduinoから「-2」が送られたら```recording=false```
※ もともとは文字列を送ろうとしたがprocessing側で比較を行うときにうまくいかないので数字にした

### recording = falseのとき
「-1」の値が来るのを待つ

### recording = trueのとき
「-2」の値がきたらモードを切り替える  
Arduino側での説明でも書いた通り音の値と時間の値が2つ連続で送られて来る。  
dataTypeToggleで場合分けをしながら2要素の配列に落とし込んで、Dequeueのインスタンスscoreにappendする。  

### outputScore()メソッド
楽譜が記録されてるDequeueのインスタンスscoreをCSVファイルに出力する。形式は  
-1列目： 音の種類
-2列目： 演奏された時間
ファイル名は```score.csv```。