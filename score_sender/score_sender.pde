import processing.serial.*;
import java.util.Queue;
import java.util.ArrayDeque;

Serial port;

//check data type from serial
//  0: note
//  1: time
int dataTypeToggle;

float tmp_data_store1;
float tmp_data_store2;

Queue<float[]> score = new ArrayDeque<float[]>();

boolean recording;

final int RECORD_START = -1;
final int RECORD_END = -2;

void setup(){
  size(100,100);
  
  //select serial port for arduino
  String arduinoPort = Serial.list()[3];
  
  port = new Serial(this, arduinoPort, 9600);
  
  //initialize as note type
  dataTypeToggle = 0;
  
  recording = false;
}

void draw(){
}

void serialEvent(Serial p){
  //get value as String and then convert that to float
  //get message till line break (ASCII > 13)
  String message = port.readStringUntil(13);
  if(message != null){
    float value = float(message);
    
    if (!recording){
      if(value == RECORD_START){
        recording = true;
        println("RECORD START");
        return;
      }
    } else {
      if(value == RECORD_END){
        recording = false;
        println("RECORD END");
        outputScore();
        return;
      }
      if(dataTypeToggle == 0){
          tmp_data_store1 = value;
          dataTypeToggle = 1;
      } else {
          tmp_data_store2 = value;
          
          println("new score: note = " + tmp_data_store1 + ", time = " + tmp_data_store2);
          float[] tmp_data_array = {tmp_data_store1, tmp_data_store2};
          score.add(tmp_data_array);
          
          dataTypeToggle = 0;
      }
    }
  }
}

void outputScore(){
  //output score as csv file
  PrintWriter output = createWriter("../csv_files/score.csv");
  float[] tmp;

  while(score.size() > 0){
    tmp = score.remove();
    println("note=" + tmp[0] + ", time=" + tmp[1]);
    
    output.println(tmp[0] + "," + tmp[1] + ";");
  }
  
  output.flush();
  output.close();
}