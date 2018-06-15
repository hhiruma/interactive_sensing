import processing.serial.*;
import themidibus.*;
import java.util.Queue;
import java.util.ArrayDeque;

final int HNUM = 4;
final int VNUM = 2;

int[] pitchList = { 36, 38, 42, 44, 45, 50, 46, 51 };
StringDict nameList = new StringDict();
Drum[] drumList = new Drum[HNUM * VNUM];
Queue<Integer> playList = new ArrayDeque<Integer>();
int global_play_tmp;

// Create a MidiBus object
MidiBus mb;

Serial port;

void setup() {
  size(1000, 400);
  background(0, 0, 0);
  
  // List available MIDI inputs and outputs
  MidiBus.list();
  // Instantiate the MidiBus
  mb = new MidiBus(this, -1, "Virtual MIDI Bus");

  //draw setup
  fill(255);
  ellipseMode(CENTER);
  textMode(CENTER);

  //set names
  nameList.set("36", "kick");
  nameList.set("38", "snare");
  nameList.set("42", "high hat");
  nameList.set("44", "high hat closed");
  nameList.set("45", "tom1");
  nameList.set("46", "crash");
  nameList.set("50", "tom2");
  nameList.set("51", "ride");

  //instantiate drums
  for (int i=0; i<VNUM; i++) {
    for (int j=0; j<HNUM; j++) {
      int r = 100;
      int x = width/(HNUM+1) * (j+1);
      int y = height/(VNUM+1) * (i+1);
      int n = i*HNUM + j;
      int pitch = pitchList[n];
      drumList[i*HNUM + j] = new Drum(x, y, r, n+1, pitch, nameList.get(str(pitch)));
    }
  }
  
  //show all drums
  for (int i=0; i<drumList.length; i++) {
    drumList[i].show();
  }
  
  //select serial port for arduino
  String arduinoPort = Serial.list()[3];
  
  //instatiate serial object
  port = new Serial(this, arduinoPort, 9600);
}


void draw() {
  //play all notes in playlist
  if (playList.size() > 0) {
    global_play_tmp = playList.remove();
    thread("playNote");
  }
}


void playNote() {
  // Play a note on our 1st (and only) channel
  int channel = 1;
  int velocity = 127;
  int pitch = global_play_tmp;

  //print("Play: ");
  //println(pitch);

  mb.sendNoteOn(channel, pitch, velocity); 
  delay(500);
  mb.sendNoteOff(channel, pitch, velocity);
  delay(500);
}

void mouseClicked(){
  for(int i=0; i<drumList.length; i++){
    if (dist(mouseX, mouseY, drumList[i].x, drumList[i].y) < drumList[i].r/2){
      playList.add(drumList[i].pitch);
      return;
    }
  }
}

void keyTyped() {
  //for(int i=0; i<drumList.length; i++){
  if(int(key) >= 49 && int(key) <= 56){
    playList.add(drumList[int(key)-48-1].pitch);
  }
}

int counter = 0;

void serialEvent(Serial p){
  int input = p.read();
  if(input >= 0 && input <= 8){
    playList.add(drumList[input].pitch);
    counter++;
  } else {
    println("***input is invalid***");
  }
  println("received note: " + input + ", counter = " + counter);
}

/*
pitch list
 43: super tyo low tom G1
 44: high hat funda G1#
 45: tyo low tom A2
 46: high hat open A2#
 47: low tom B2
 48: tom C2
 49: crash3 C2#
 50: kick* D2
 51: ride3 D2#
 52: crash2 E2
 53: ride2 F2
 54: bell F2#
 57: crash A3
 59: ride  B3
 70: shaker
 75: stick D4#
 
 
 selected
 
 c1 kick : 36
 d1 snare : 38
 F1# high hat : 42
 G1# high hat closed : 44
 A2 tom  : 45
 A2# crash : 46
 D2 kick : 50
 D2# ride : 51
 */