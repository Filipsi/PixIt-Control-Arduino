#include <LiquidCrystal.h>

byte ProgressBar0[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte ProgressBar1[8] = {
  B00000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B00000
};

byte ProgressBar2[8] = {
  B00000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B00000
};

byte ProgressBar3[8] = {
  B00000,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B00000
};

byte ProgressBar4[8] = {
  B00000,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B00000
};

byte ProgressBar5[8] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000
};

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

LiquidCrystal lcd(2, 3, 5, 6, 7, 8);

String MessageData = "";
char PinOutputNum[6] = { 14, 15, 16, 17, 18, 19 };
int PinReadNum = 9;

void setup() {
  lcd.createChar(0, ProgressBar0);
  lcd.createChar(1, ProgressBar1); 
  lcd.createChar(2, ProgressBar2); 
  lcd.createChar(3, ProgressBar3); 
  lcd.createChar(4, ProgressBar4); 
  lcd.createChar(5, ProgressBar5); 
  lcd.begin(16, 2);
  LcdPrintMessage("LCD ready!", "Loading ...");
  delay(1000);
  
  Serial.begin(115200);
  LcdPrintMessage("Serial ready!", "Loading ...");
  delay(1000);
  
  for (int i = 0; i < sizeof(PinOutputNum) - 1; i++) { pinMode(PinOutputNum[i], OUTPUT); }
  pinMode(PinReadNum, INPUT);
  LcdPrintMessage("Arduino ready!", "");
  delay(1500);
}

void loop() {
  if (Serial.available()) {
    delay(1);
    while(Serial.available()){
      MessageData += (char)Serial.read();
    }
  }
  
  if(MessageData != "") {
    String Operation = MessageData.substring(0, 1);
    if(Operation == "M"){
        String Axis = MessageData.substring(1, 2);
        String Direction = MessageData.substring(2, 3);
        int Repeat = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(')')).toInt();
        
        if(Axis == "X") {
           if(Direction == "L"){
             SendCommand(moveXleft, Repeat, true);
           }else if(Direction == "R") {
             SendCommand(moveXright, Repeat, true);
           }
        }else if(Axis == "Y") {
           if(Direction == "U"){
             SendCommand(moveYup, Repeat, true);
           }else if(Direction == "D") {
             SendCommand(moveYdown, Repeat, true);
           }
        }else if(Axis == "Z") {
           if(Direction == "U"){
             SendCommand(moveZup, Repeat, true);
           }else if(Direction == "D") {
             SendCommand(moveZdown, Repeat, true);
           }
        }     
    }else if(Operation == "S") {
      String Part = MessageData.substring(1, 2);
      String Status = MessageData.substring(2, 3);
      int ValueToSend = 0;
      
      if(Part == "P") {
        if(Status == "U"){
          SendCommandUntil(penUp, false, 10, true);
        }else if(Status == "D") {
          SendCommandUntil(penDown, false, 10, true);
        }
      }else if(Part == "D") {
        if(Status == "L"){
          SendCommand(drillOnLeft, 1, true);
        }else if(Status == "R") {
          SendCommand(drillOnRight, 1, true);
        }else if(Status == "0") {
          SendCommand(drillOff, 1, true);
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
    }else if(Operation == "G") {
      String Part = MessageData.substring(1, 2);
      
      if(Part = "S") {
         int SensorNumber = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(')')).toInt();
         Serial.println(GetSensorState(SensorNumber));
      }
    } else if(Operation == "W") {
      String Part = MessageData.substring(1, 2);
      
      if(Part == "N") {
        int num = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(')')).toInt();   
        WriteBinnaryOut(ConvertToBinnary(num));
      }      
    } else if(Operation == "D") {
      String Part = MessageData.substring(1, 2);
      
      if(Part == "P") {
        int commandsCompleted = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(',')).toInt();  
        int commandsCount = MessageData.substring(MessageData.indexOf(',') + 1, MessageData.indexOf(')')).toInt();  
        LcdPrintProgressBar(commandsCompleted, commandsCount); 
      } else if(Part == "T") {
        String text = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(')'));
        lcd.print(text);
      } else if(Part == "S") {
        int xPos = MessageData.substring(MessageData.indexOf('(') + 1, MessageData.indexOf(',')).toInt();  
        int yPos = MessageData.substring(MessageData.indexOf(',') + 1, MessageData.indexOf(')')).toInt();  
        lcd.setCursor(xPos, yPos);
      } else if(Part == "C") {
        lcd.clear();
      }
    }
    
    MessageData = "";
  }
  
}

//Převede vstupní dekaticnou hodnotu na binární hodnotu
String ConvertToBinnary(int Value) {
  return String(Value, BIN);
}

//Vypíše vstupní binární data na výstupy
void WriteBinnaryOut(String Data) {
  for(int i = 0; i < sizeof(PinOutputNum) - 1; i++) {
    if(i <= Data.length()){
      digitalWrite(PinOutputNum[i], Data.substring(i, i + 1).toInt());
    }else{
      digitalWrite(PinOutputNum[i], LOW);
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

void LcdPrintMessage(String MessageR1, String MessageR2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(MessageR1);
  lcd.setCursor(0, 1);
  lcd.print(MessageR2);  
}

void LcdPrintProgressBar(int Value, int ValueMax) {
  byte procentage = ((float) Value) / ValueMax * 100;
  byte lines = ((float) 80) / 100 * procentage;
  
  lcd.clear();
  for(byte i = 0; i < 80; i += 5) {
    lcd.setCursor(round(i / 5), 0);
    byte segmentValue = lines - (round(i / 5) * 5);
    if(segmentValue > 5) {
      if(segmentValue <= lines) {
        lcd.write(byte(5)); 
      } else {
        lcd.write(byte(0)); 
      }   
    } else {
      lcd.write(byte(segmentValue));  
    }
  }
  
  lcd.setCursor(16 / 2 - ((String(procentage) + "%").length() / 2), 1);
  lcd.print(String(procentage) + "%");
}
