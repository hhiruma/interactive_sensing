const int sensorPins[6] = {
  A0, A1, A2, A3, A4, A5
};

boolean flag = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  int sensorValues[6];
  flag = false;

  for(int i=0; i<1; i++){
    if((sensorValues[i] = analogRead(sensorPins[i])) != 0){
      Serial.write(i);
      flag = true;
    }
  }

  if(flag) {
    delay(100);
  }
}
