const int sensorPins[6] = {
  A0, A1, A2, A3, A4, A5
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  int sensorValues[6];
  for(int i=0; i<4; i++){
    if((sensorValues[i] = analogRead(sensorPins[i])) != 0){
      Serial.write(i);
      delay(5);
    }
  }
}
