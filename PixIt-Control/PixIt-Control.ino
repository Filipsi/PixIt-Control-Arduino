const int moveXleft = 128 + 1;
const int moveXright = 128 + 2;
const int moveYup = 128 + 3;
const int moveYdown = 128 + 4;
const int moveZup = 128 + 5;
const int moveZdown = 128 + 6;
const int penUp = 128 + 7;
const int penDown = 128 + 8;
const int drillOnRight = 128 + 9;
const int drillOnLeft = 128 + 10;
const int drillOff = 128 + 11;

String ReadData = "";
int PinReadNum = 12;

void setup() {                
  Serial.begin(115200);
  for(int i = 2; i <= 9; i++){pinMode(i, OUTPUT);}
  pinMode(PinReadNum, INPUT);
}

void loop() {
  if (Serial.available()) {
    delay(1);
    while(Serial.available()){
      ReadData += (char)Serial.read();
    }
  }
  
  if(ReadData != "") {
    String Operation = ReadData.substring(0, 1);
    if(Operation == "M"){
        String Axis = ReadData.substring(1, 2);
        String Direction = ReadData.substring(2, 3);
        int Repeat = ReadData.substring(ReadData.indexOf('(') + 1, ReadData.indexOf(')')).toInt();
        int ValueToSend = 0;
        
        if(Axis == "X") {
           if(Direction == "L"){
             ValueToSend = moveXleft;
           }else if(Direction == "R") {
             ValueToSend = moveXright;
           }
        }else if(Axis == "Y") {
           if(Direction == "U"){
             ValueToSend = moveYup;
           }else if(Direction == "D") {
             ValueToSend = moveYdown;
           }
        }else if(Axis == "Z") {
           if(Direction == "U"){
             ValueToSend = moveZup;
           }else if(Direction == "D") {
             ValueToSend = moveZdown;
           }
        }

        if(ValueToSend != 0) {
          SendCommand(ValueToSend, Repeat, true);
        }
        
    }else if(Operation == "S") {
      String Part = ReadData.substring(1, 2);
      String Status = ReadData.substring(2, 3);
      int ValueToSend = 0;
      
      if(Part == "P") {
        if(Status == "U"){
          SendCommandUntil(penUp, false, 10, true);
        }else if(Status == "D") {
          SendCommandUntil(penDown, false, 10, true);
        }
      }else if(Part == "D") {
        if(Status == "L"){
          ValueToSend = drillOnLeft;
        }else if(Status == "R") {
          ValueToSend = drillOnRight;
        }else if(Status == "0") {
          ValueToSend = drillOff;
        }else if(Status == "U") {
          SendCommandUntil(moveZup, true, 2000, true);
        }else if(Status == "D"){
          SendCommand(drillOnRight, 1, false);
          SendCommand(moveZdown, 35, false);
          SendCommandUntil(moveZdown, false, 100, false);
          SendCommand(drillOff, 1, false);
          SendCommand(moveZup, 40, true);
        }
      }else if(Part == "S") {
        if(Status == "X"){
          SendCommandUntil(moveXleft, true, 2000, true);
        }else if(Status == "Y") {
          SendCommandUntil(moveYdown, true, 2000, true);
        }
      }else if(Part == "E") {
        if(Status == "X"){
          SendCommandUntil(moveXright, true, 2000, true);
        }else if(Status == "Y") {
          SendCommandUntil(moveYup, true, 2000, true);
        }
      }
      
      if(ValueToSend != 0){
        SendCommand(ValueToSend, 1, true);
      }
      
    }else if(Operation == "G") {
      String Part = ReadData.substring(1, 2);
      
      if(Part = "S") {
         int SensorValue = ReadData.substring(ReadData.indexOf('(') + 1, ReadData.indexOf(')')).toInt();
         boolean SensorState = GetSensorState(SensorValue);
         Serial.println(SensorState);
      }
    } else if(Operation == "W") {
      String Part = ReadData.substring(1, 2);
      
      if(Part == "N") {
        int num = ReadData.substring(ReadData.indexOf('(') + 1, ReadData.indexOf(')')).toInt();
        
        WriteBinnaryOut(ConvertToBinnary(num));
        Serial.println(num);
        Serial.println(ConvertToBinnary(num));
      }      
    }
    
    ReadData = "";
  }
  
}

//Převede vstupní dekaticnou hodnotu na binární hodnotu
String ConvertToBinnary(int Value) {
  return String(Value, BIN);
}

//Vypíše vstupní binární data na výstupy
void WriteBinnaryOut(String Data) {
  for(int i = 0; i < 8; i++) {
    if(i <= Data.length()){
      digitalWrite(9 - i, Data.substring(i, i+1).toInt());
    }else{
      digitalWrite(9 - i, LOW);
    }
  }
}

//Vrátí stav senzoru
boolean GetSensorState(int Value){
  WriteBinnaryOut(ConvertToBinnary(Value));
  delayMicroseconds(10);
  WriteBinnaryOut(ConvertToBinnary(Value + 32));
  delayMicroseconds(900);
  boolean state = digitalRead(PinReadNum);
  delayMicroseconds(10);
  WriteBinnaryOut(ConvertToBinnary(Value));
  return state;
}

//Odešle příkaz tiskárně
void SendCommand(int ValueToSend, int Loops, boolean ConfirmDone) {
  if(GetSensorState(ValueToSend) == false) {
    for(int i = 0; i < Loops; i++){
      delayMicroseconds(2000);
      WriteBinnaryOut(ConvertToBinnary(ValueToSend));
      delayMicroseconds(2000);
      WriteBinnaryOut(ConvertToBinnary(ValueToSend + 16));
    }
    WriteBinnaryOut(ConvertToBinnary(ValueToSend));
    delayMicroseconds(500);
  }
  if(ConfirmDone){ Serial.println("A"); }
}

//Odesílá příkazy a kontrolu stav zenzoru tak dlouho dokud není kontrolovaný senzor sepnut
void SendCommandUntil(int Val, boolean UseMicroseconds, int Time, boolean ConfirmDone) { 
  WriteBinnaryOut(ConvertToBinnary(Val));
  delayMicroseconds(10);
  WriteBinnaryOut(ConvertToBinnary(Val + 32));
  delayMicroseconds(10);
  
  while(digitalRead(PinReadNum) != HIGH){
    if(UseMicroseconds){ delayMicroseconds(Time); }else { delay(Time); }
    WriteBinnaryOut(ConvertToBinnary(Val));
    if(UseMicroseconds){ delayMicroseconds(Time); }else { delay(Time); }
    WriteBinnaryOut(ConvertToBinnary(Val + 32 + 16)); 
  }
  WriteBinnaryOut(ConvertToBinnary(Val));
  if(ConfirmDone){ Serial.println("A"); }
  delayMicroseconds(500);
}


