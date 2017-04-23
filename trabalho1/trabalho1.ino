
/* Piscar o led a cada 1s
  bt1 : acelerar o pisca-pisca a cada pressionamento (low->high)
  bt2 : desacelerar o pisca-pisca a cada pressionamento (low->high)
  b1+2: parar (em menos de 500ms)*/

#define LED_PIN 2
#define BT1_PIN 4
#define BT2_PIN 8

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BT1_PIN, INPUT);
  pinMode(BT2_PIN, INPUT);
  Serial.begin(9600);

}
long int timeStop=10000;
long unsigned int tempo, bt1press = 0, bt2press = 0, tempoLed=0, timeBt1=1000,timeBt2 ;
int bt1, bt2;
int statusLed = 0, statusBt1 = 0, statusBt2 = 0;
int led = 0;

int v = 1000;
void loop() {
  tempo = millis();

  bt1 = digitalRead(BT1_PIN);
  bt2 = digitalRead(BT2_PIN);
  digitalWrite(LED_PIN, statusLed);

  if (tempo > tempoLed) {
    if (statusLed) { //high
      tempoLed = tempo + v;
    }
    else { //low
      tempoLed = tempo + 1000;
    }
    statusLed = !statusLed;

  }
  Serial.print(bt1);
  
  Serial.print(", ");
  
  Serial.print(statusBt1);
  
  Serial.print(", ");
  
  Serial.println(bt1press);
  //acelerar
  if (!bt1 && statusBt1&&(tempo > bt1press)&&!bt2) {
      v -= 200;
      bt1press = tempo + 100;
  }
  statusBt1 = bt1;

  //desacelerar
  if (!bt2 && statusBt2&&(tempo > bt2press)&&!bt1) {
      v += 200;
      bt2press = tempo + 100;
  }
  statusBt2 = bt2;

  if(bt1)timeBt1 = tempo;
  if(bt2)timeBt2 = tempo;
  timeStop = timeBt1-timeBt2;
  timeStop = abs(timeStop);
  Serial.println(timeStop);
  
  //parar
  if((timeStop<500)){
   while(1){digitalWrite(LED_PIN, LOW);}    
  }
}




