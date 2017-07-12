int i;
char out[3] = "0\0";
//              /
//            \__/
void setup(){
  Serial.begin(9600);
  for(i=2;i<7;i++){
    pinMode(i,INPUT);
  } 
}
void loop(){
  out[0] = '0';
  for(i=2;i<7;i++){
    out[0]+=digitalRead(i)*(1<<(i));
  }
  Serial.write(out[0]);
}
