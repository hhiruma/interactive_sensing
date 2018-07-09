import java.util.Queue;
import java.util.ArrayDeque;

int situation;
int baseTime;
int time;
Queue<float[]> score = new ArrayDeque<float[]>();

void setup(){
  situation = 0;
  size(700,400);
  colorMode(RGB,256);
  rectMode(CENTER);
  baseTime = 0;
}

void draw(){
  if(score.peek()==null){  //until hit button, it stop
  baseTime = millis();
  }
  if(situation == 1){  //record-mode
    time = millis();
    stroke(255);
    line(0,200,(time-baseTime)/50,200);
    
    
  }else if(situation == 2){   //record-finish,showing previou
    fill(0);
    rect(350, 105, 200, 40);
    stroke(255);
    fill(255);
    text("Press SPACE to Re-record", 350, 110);
    
  }else{
    background( 0 );
    textAlign(CENTER);
    text("Press SPACE to Start!", 350, 200);
  }
}

void keyPressed(){
  if( key == ' ' ){   //SPACE KEY for Start
    background(0);
    situation =  1;
    stroke(255);
    fill(255);
    text("Press ENTER to End", 350, 100);
    text("You can use sdfjkl to Record note", 350, 120);
    while(score.size() > 0){  // when hit SPACE, all data is clear
    score.remove();
    }
    
  }else if(key == 's'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,140,10,10);
    float[] tmp_data_array = {1, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == 'd'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,160,10,10);
    float[] tmp_data_array = {2, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == 'f'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,180,10,10);
    float[] tmp_data_array = {3, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == 'j'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,220,10,10);
    float[] tmp_data_array = {4, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == 'k'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,240,10,10);
    float[] tmp_data_array = {5, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == 'l'){
    stroke(255, 0, 0);
    fill(255,0,0);
    ellipse((time-baseTime)/50+10,260,10,10);
    float[] tmp_data_array = {6, (time-baseTime)};
    score.add(tmp_data_array);
    
  }else if(key == ENTER){  //ENTER to stop
    outputScore();
    situation =2;
  }
}

void outputScore(){
  //output score as csv file
  PrintWriter output = createWriter("score.csv");
  float[] tmp;
  output.println( "start;");

  while(score.size() > 0){
    tmp = score.remove();
    println("note=" + tmp[0] + ", time=" + tmp[1]);
    
    output.println(tmp[0] + "," + int(tmp[1]) + ";");
  }
  output.println( "end"+ "," + (time-baseTime) + ";");
  output.flush();
  output.close();
}