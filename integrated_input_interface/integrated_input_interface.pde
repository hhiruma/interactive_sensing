import interfascia.*;
import processing.serial.*;
import themidibus.*;
import java.util.Queue;
import java.util.ArrayDeque;

//arduino sender
Serial port;

//garage band sender
int[] pitchList = { 36, 38, 42, 44, 45, 50, 46, 51 };
StringDict nameList = new StringDict();
Queue<Integer> playList = new ArrayDeque<Integer>();
int global_play_tmp;
MidiBus mb;

//mode select buttons
GUIController c_menuButton, c_saveInterface;
IFButton noteModeButton, playModeButton, saveButton, uploadNowButton;
boolean noteMode, playMode;
IFLookAndFeel defaultMenuButtonView, chosenMenuButtonView;
IFTextField fileNameField;

//note mode: status
int recordingStatus;
int baseTime;
int endTime;
ArrayList<Float[]> score = new ArrayList<Float[]>();
char[] noteSelection = {'s', 'd', 'f', 'j', 'k', 'l'};
String csvFileName;


void setup(){
    size(700, 400);
    colorMode(RGB, 256);
    rectMode(CORNER);
    smooth();

    //set mode init vals
    noteMode = true;
    playMode = false;
    recordingStatus = 0;
    baseTime = 0;
    csvFileName = "score";

    //setup serial
    // String arduinoPort = "/dev/cu.usbmodem144231";
    // port = new Serial(this, arduinoPort, 9600);

    //setup drums and garage band
    MidiBus.list();
    mb = new MidiBus(this, -1, "Virtual MIDI Bus");
    nameList.set("36", "kick");
    nameList.set("38", "snare");
    nameList.set("42", "high hat");
    nameList.set("44", "high hat closed");
    nameList.set("45", "tom1");
    nameList.set("46", "crash");
    nameList.set("50", "tom2");
    nameList.set("51", "ride");


    initDraw();
    saveInterfaceDraw();
}

void draw(){
    if(noteMode){
        drawNoteMode();
    } else {
        drawPlayMode();
    }

    if(playList.size() > 0){
        global_play_tmp = playList.remove();
        thread("playNote");
    }
}

void playNote(){
    int channel = 1;
    int velocity = 127;
    int pitch = global_play_tmp;

    mb.sendNoteOn(channel, pitch, velocity);
    delay(500);
    mb.sendNoteOff(channel, pitch, velocity);
    delay(500);
}

void drawNoteMode(){
    pushMatrix();
    translate(150, 80);

    fill(25);
    noStroke();
    rect(0, 0, 530, 220);

    //title
    String title = "Note Node";
    noFill();
    stroke(255);
    textSize(20);
    rect(12, 14, textWidth(title)+20, 30);

    noStroke();
    fill(255);
    text(title, 20, 37);

    //score lines
    stroke(100);
    //vertical line
    line(20, 80, 20, 180);
    line(520, 80, 520, 180);
    for(int i=0; i<6; i++){
        //horizontal line
        line(20, 80+20*i, 520, 80+20*i);
    }

    if(score.size() == 0){
        baseTime = millis();
    }

    fill(255);
    textSize(16);
     switch(recordingStatus){
         case 0:
             text("Status: Ready to record. Press key to begin.", 20, 215);
             break;
         case 1:
             line((millis() - baseTime)/30, 80, (millis() - baseTime)/30, 180);
             break;
         case 2:
             text("Status: Recorded notes. Press SPACE to re-record.", 20, 215);
             break;
         default:
             break;
     }

    //draw scores
    for(int i=0; i<score.size(); i++){
        stroke(255, 0, 0);
        fill(255, 0, 0);
        Float[] tmp = score.get(i);
        ellipse(tmp[1]/30+20, 80+20*tmp[0], 10, 10);
        // println(str(tmp[1]/50) + ", " + str(180 + 40 * tmp[0]));
    }


    popMatrix();
}

void drawPlayMode(){
    pushMatrix();
    translate(150, 80);

    fill(25);
    noStroke();
    rect(0, 0, 530, 200);

    String title = "Play Node";
    noFill();
    stroke(255);
    rect(12, 14, textWidth(title)+20, 30);

    noStroke();
    fill(255);
    textSize(20);
    text(title, 20, 37);

    popMatrix();
}

void initDraw(){
    background(0);

    //make title
    fill(255);
    textSize(24);
    text("Integrated Input Interface", 25,  50);

    //make menu buttons
    c_menuButton = new GUIController(this);
    noteModeButton = new IFButton("Note Mode", 30, 100, 100, 20);
    playModeButton = new IFButton("Play Mode", 30, 150, 100, 20);

    noteModeButton.addActionListener(this);
    playModeButton.addActionListener(this);

    //default view of menu buttons
    defaultMenuButtonView = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
    defaultMenuButtonView.baseColor = color(0, 0, 120);
    defaultMenuButtonView.highlightColor = color(0, 0, 200);
    defaultMenuButtonView.textColor = color(255);
    defaultMenuButtonView.activeColor = color(0, 0, 255);
    defaultMenuButtonView.borderColor = color(0);

    //chosen version of view for menu buttons
    chosenMenuButtonView = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
    chosenMenuButtonView.baseColor = color(0, 0, 255);
    chosenMenuButtonView.highlightColor = color(0, 0, 200);
    chosenMenuButtonView.textColor = color(255);
    chosenMenuButtonView.activeColor = color(0, 0, 255);
    chosenMenuButtonView.borderColor = color(0);

    c_menuButton.setLookAndFeel(defaultMenuButtonView);

    c_menuButton.add(noteModeButton);
    c_menuButton.add(playModeButton);

    noteModeButton.setLookAndFeel(chosenMenuButtonView);
}

void saveInterfaceDraw(){
    //somehow translate doesn't seem to work on these
    c_saveInterface = new GUIController(this);

    fileNameField = new IFTextField("fileName", 20+150, 240+80, 100, "score");
    textSize(15);
    text(".csv", 125+150, 255+80);
    saveButton = new IFButton("Save As", 180+150, 240+80, 50, 20);
    uploadNowButton = new IFButton("Upload Now", 240+150, 240+80, 70, 20);

    fileNameField.addActionListener(this);
    saveButton.addActionListener(this);
    uploadNowButton.addActionListener(this);

    c_saveInterface.add(fileNameField);
    c_saveInterface.add(saveButton);
    c_saveInterface.add(uploadNowButton);

    saveButton.setLookAndFeel(defaultMenuButtonView);
    uploadNowButton.setLookAndFeel(defaultMenuButtonView);
}

void actionPerformed(GUIEvent e){
    if(e.getSource() == noteModeButton){
        noteMode = true;
        playMode = false;
        noteModeButton.setLookAndFeel(chosenMenuButtonView);
        playModeButton.setLookAndFeel(defaultMenuButtonView);
    } else if (e.getSource() == playModeButton){
        noteMode = false;
        playMode = true;
        noteModeButton.setLookAndFeel(defaultMenuButtonView);
        playModeButton.setLookAndFeel(chosenMenuButtonView);
    } else if (e.getSource() == saveButton){
        csvFileName = fileNameField.getValue();
        outputScore();
        saveButton.setLookAndFeel(chosenMenuButtonView);
    } else if (e.getSource() == uploadNowButton){
        ArrayList<String> output = new ArrayList<String>();
        Float[] tmp;
        output.add("start;");

        for(int i=0; i<score.size(); i++){
            tmp = score.get(i);
            output.add(tmp[0] + "," + tmp[1] + ";");
            println(tmp[0] + "," + tmp[1] + ";");
        }
        output.add("end"+ "," + (endTime - baseTime) + ";");
        println( "end"+ "," + (endTime - baseTime) + ";");

        // sendToArduino(output);
    }
}

void sendToArduino(ArrayList<String> output){
    for(int i=0; i<output.size(); i++){
        port.write(output.get(i));
    }
}

void outputScore(){
    PrintWriter output = createWriter(csvFileName+".csv");
    Float[] tmp;
    output.println("start;");

    for(int i=0; i<score.size(); i++){
        tmp = score.get(i);
        output.println(tmp[0] + "," + tmp[1] + ";");
        println(tmp[0] + "," + tmp[1] + ";");
    }
    output.println( "end"+ "," + (endTime - baseTime) + ";");
    println( "end"+ "," + (endTime - baseTime) + ";");
    output.flush();
    output.close();
}

void keyPressed(){
    if(noteMode){
        if(key == ' '){
            //when hit space, clear all score
            score = new ArrayList<Float[]>();
            recordingStatus = 0;
            return;
        }
        if(recordingStatus == 2){
          return;
        }
        for(int i=0; i<noteSelection.length; i++){
            if(key == noteSelection[i]){
                Float[] tmp_data_array = {(float)i, (float)(millis()-baseTime)};
                score.add(tmp_data_array);
                playList.add(pitchList[i]);
                return;
            }
        }
        if(key == ENTER){
            endTime = millis();
            outputScore();
            recordingStatus = 2;
            return;
        }
    } else {

    }
}