#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define LED_G 2
#define LED_R 12
#define SERVO 10
#define BUZZER 11

char senha[] = "4213";

// A-55hz to A-1760hz 
int notes[] = {55,58,62,65,69,73,78,82,87,92,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760, 0};
// Songs
int tones[2][6] = {
  {27,34,41,36,38,0}, // Fail
  {22,28,31,32,33,0}  // Success
};
int duration[2][6] = {
  {300,300,300,300,500,0}, // Fail
  {300,300,150,300,300,0}  // Success
};
int tcursor=0;// escolhe a nota/duracao
int scursor=0;// escolhe a musica
bool isplay=0;// se esta tocando

// keypad
const byte rows = 4; 
const byte cols = 3;
byte rowPins[rows] = {3, 4, 5, 6};
byte colPins[cols] = {7, 8, 9};
char keys[rows][cols] = { 
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'} 
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);
int lcdcurstate;
int lcdnewstate;
bool updlcd;

// Servo
Servo s;

// Str digitada
char strbuff[200];
bool strdone=0;
int strwr = 0;

int i;

// Tempos
unsigned long t;
unsigned long tuneTime;
unsigned long openTime;
unsigned long msgTime;

// Tranca
bool lock=1;
bool lockpos=1;

void setup() {  
  pinMode(LED_G,OUTPUT);
  pinMode(LED_R,OUTPUT);
  s.attach(SERVO);
  s.write(0); // Inicia motor na posição zero 
  
  lcd.begin();
  lcd.backlight();

  updlcd=1;
  lcdcurstate=0;
  lcdnewstate=1;
  msgTime=millis()+2000;

  Serial.begin(9600);
}

void loop() {
  t=millis();

  // Keypad Logic
  char key = keypad.getKey();
  if(key!=0){
    if(key=='#'){ //enter
      strdone=1;
    }else if(key == '*'){// cancel
      clearStr();
      updlcd=1;
    }else{
      strbuff[strwr++] = key;
      updlcd=1;lcdnewstate=2;msgTime=t-1;
      Serial.println("pressed a key");
    }
    if(strwr>=200){// too many numbers
      clearStr();
    }
  }

  // Tune Logic
  if(isplay && tuneTime<t){
    tone(BUZZER,notes[tones[tcursor][scursor]]);
    tuneTime = t+duration[tcursor][scursor];
    
    if(tones[tcursor][scursor]==0){
      noTone(BUZZER);
    }
    if(duration[tcursor][scursor]==0){
      scursor=0;
      isplay=0;
      noTone(BUZZER);
    }else{
      scursor++;
    }
  }

  // Lock Logic
  if(strdone){// string is done
    if(!strcmp(senha, strbuff)){// senha certa
      lock = 0;
      startSong(1);
      lcdnewstate=4;
    }else{ // senha errada
      lock = 1;
      startSong(0);
      lcdnewstate=3;
    }
    msgTime=t-1;
    clearStr();
    strdone=0;
  }
  if(lock==0 && lockpos==1){// time to open
    s.write(180);
    openTime=t;
    lockpos=0;
  }
  if(lock==1 && lockpos==0){// time to close
    s.write(0);
    lockpos=1;
  }
  if(lock==0 && t>openTime+5000){// open for too long
    lock=1;
  }

  // Led Logic
  if(lock){
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW); 
  }else{
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
  }
  
  // LCD Logic
  if(t>msgTime&&lcdnewstate!=lcdcurstate){
    lcdcurstate=lcdnewstate;
    updlcd=1;
  }
  if(updlcd){
    switch(lcdcurstate){
      case 0:
        lcd.setCursor(0,0);
        lcd.print("   Controle de  ");
        lcd.setCursor(0,1);
        lcd.print("   Acesso  :)   ");
        break;
      case 1:
        lcd.setCursor(0,1);
        lcd.print("   Acesso  ;)   ");
        lcdnewstate = 0;msgTime=t+500;
        break;
      case 2:
        lcd.setCursor(0,0);
        lcd.print("Digite a Senha: ");
        lcd.setCursor(0,1);
        for(i=0;i<16;i++){
          if(i<strlen(strbuff))
            lcd.print("*");
          else
            lcd.print(" ");
        }
        break;
      case 3:
        lcd.setCursor(0, 0);
        lcd.print("Senha Incorreta ");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcdnewstate = 2;msgTime=t+5000;
        break;
      case 4:
        lcd.setCursor(0, 0);
        lcd.print("Senha Correta   ");
        lcd.setCursor(0, 1);
        lcd.print("Seja bem vindo! ");
        lcdnewstate = 2;msgTime=t+5000;
        break;
    }
    updlcd=0;
  }
}
void startSong(int s){
  isplay=1;
  tcursor=s;
  scursor=0;
}
void clearStr(){
  for(i=0;i<200;i++) strbuff[i]=0;
  strwr=0;
}
