#include <LiquidCrystal.h>
#include <GPRS_Shield_Arduino.h>
LiquidCrystal lcd(8, 6, 5, 4, 3, 2);
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = A0, TXPin = A1;
static const uint32_t GPSBaud = 9600;

char messages[160];
char Lat[12];
char Lng[12];

TinyGPSPlus gps;
GPRS gprs(TXPin,RXPin,GPSBaud);
#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char phone[16];
char datetime[24];
char buff_lat[20];
char buff_lng[20];
bool is_active;
char in_number[15];
char my_number[15]="+6281376918656";
char in_aktif[10]="aktif";
char in_nonaktif[10]="nonaktif";
char in_mati[10]="matikan";
char in_cek[10]="cek";
char in_buzzer[6]="alarm";
char out_aktif[40]="Kemanan Telah Di aktifkan";
char out_nonaktif[40]="Kemanan Telah Di non aktifkan";
char out_cek[60]="Kunci kontak terbuka silahkan cek motor anda";

int hidup_alarm=0;
#define buzzer 7
#define klakson 9
#define kontak 10
int stt_motor=0;
#define cek_kontak A5
int keamanan = 0;

unsigned long previousMillis = 0;
const long interval = 120; 


void aktif_telfon(){
  if (
    in_number[0]==my_number[0] && 
    in_number[1]==my_number[1] && 
    in_number[2]==my_number[2] && 
    in_number[3]==my_number[3] && 
    in_number[4]==my_number[4] && 
    in_number[5]==my_number[5] && 
    in_number[6]==my_number[6] && 
    in_number[7]==my_number[7] && 
    in_number[8]==my_number[8] && 
    in_number[9]==my_number[9] && 
    in_number[10]==my_number[10] && 
    in_number[11]==my_number[11] && 
    in_number[12]==my_number[12] && 
    keamanan==0)
    {
      keamanan=1;
      buzzer_on();
      delay(1000);
      gprs.hangup();
      gprs.sendSMS(my_number,out_aktif);
    }
}

void nonaktif_telfon(){
  if (
    in_number[0]==my_number[0] && 
    in_number[1]==my_number[1] && 
    in_number[2]==my_number[2] && 
    in_number[3]==my_number[3] && 
    in_number[4]==my_number[4] && 
    in_number[5]==my_number[5] && 
    in_number[6]==my_number[6] && 
    in_number[7]==my_number[7] && 
    in_number[8]==my_number[8] && 
    in_number[9]==my_number[9] && 
    in_number[10]==my_number[10] && 
    in_number[11]==my_number[11] && 
    in_number[12]==my_number[12] && 
    keamanan==1)
    {
      keamanan=0;
      buzzer_off();
      delay(1000);
      gprs.hangup();
      gprs.sendSMS(my_number,out_nonaktif);
    }
}

void sendlocation(){
    if(!sim900_check_with_cmd(F("AT+CMGF=1\r\n"), "OK\r\n", CMD)) { 
        return false;
    }
    delay(500);
    sim900_flush_serial();
    sim900_send_cmd(F("AT+CMGS=\""));
    sim900_send_cmd(my_number);
    if(!sim900_check_with_cmd(F("\"\r\n"),">",CMD)) {return false;}
    delay(1000);
    sim900_send_cmd(messages);
    sim900_send_End_Mark();
    return sim900_wait_for_resp("OK\r\n", CMD, 5, 5000);
}

void buzzer_on(){
  digitalWrite(klakson,HIGH);
  delay(200);
  digitalWrite(klakson,LOW);
}

void buzzer_off(){
  digitalWrite(klakson,HIGH);
  delay(200);
  digitalWrite(klakson,LOW);
  delay(200);
  digitalWrite(klakson,HIGH);
  delay(200);
  digitalWrite(klakson,LOW);
}

void setup()
{

  lcd.begin(16, 2);
  pinMode(buzzer,OUTPUT);
  pinMode(klakson,OUTPUT);
  pinMode(kontak,OUTPUT);
  Serial.begin(9600);
  lcd.setCursor(0,0);
  lcd.print("Cek Modem       ");
  for (int t=0;t<16;t++){
    lcd.setCursor(t,1);
    lcd.write(0xff);
    delay(2000);
  }
  gprs.checkPowerUp();
  lcd.setCursor(0,0);
  lcd.print("Modem Ok        ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  Cek Jaringan  ");
  while(!gprs.isNetworkRegistered()){
    digitalWrite(buzzer,HIGH);
    delay(50);
    digitalWrite(buzzer,LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Mencari Sinyal ");
    for (int t=0;t<16;t++){
      lcd.setCursor(t,1);
      lcd.write(0xff);
      delay(20);
    }
  }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    Sinyal OK   ");
    delay(1000);
    lcd.clear();
}

void loop()
{

 while (keamanan==0){
 digitalWrite(kontak,LOW);
 lcd.setCursor(0,1);
 lcd.print(" Mode Aman OFF  ");
 digitalWrite(klakson,LOW);
 messageIndex = gprs.isSMSunread();
 if (messageIndex > 0){
    gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime); 
    gprs.deleteSMS(messageIndex);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(phone);
    lcd.setCursor(0,1);
    lcd.print(message);
    delay(2000);
    lcd.clear();
    if_sms();
 } 

 
 if(gprs.isCallActive(in_number)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Call in");
    lcd.setCursor(0,1);
    lcd.print(in_number);
    delay(2000);
    lcd.clear();
    aktif_telfon();
  }

 while (Serial.available() > 0)
 if (gps.encode(Serial.read()))
 if (gps.location.isValid()){ 
    lcd.setCursor(0,0);
    lcd.print(" Location Avail ");
    strcpy(messages, "http://www.google.com/maps/place/");
    dtostrf(gps.location.lat(), 1, 6, Lat);
    strcat(messages,Lat);
    strcat(messages,"+");
    dtostrf(gps.location.lng(), 1, 6, Lng);
    strcat(messages,Lng);
    strcat(messages,"");
  }
 else{
    lcd.setCursor(0,0);
    lcd.print(" Mencari Lokasi ");
    strcpy(messages, "location not available");
    for (int t=0;t<16;t++){
      lcd.setCursor(t,1);
      lcd.write(0xff);
      delay(15);
    }
  }
 if (millis() > 5000 && gps.charsProcessed() < 10){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("   Cek Wiring   ");
  lcd.setCursor(0,1);
  lcd.print("       GPS      ");
  while(true);
 }

}






 while (keamanan==1){
   lcd.setCursor(0,1);
   lcd.print("  Mode Aman ON  ");
   if(gprs.isCallActive(in_number)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Call in");
      lcd.setCursor(0,1);
      lcd.print(in_number);
      delay(2000);
      lcd.clear();
      nonaktif_telfon();
    }

 messageIndex = gprs.isSMSunread();
 if (messageIndex > 0){
    gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime); 
    gprs.deleteSMS(messageIndex);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(phone);
    lcd.setCursor(0,1);
    lcd.print(message);
    delay(2000);
    lcd.clear();
    if_sms();
 } 
 
   if (hidup_alarm==1){
    digitalWrite(klakson,HIGH);
    delay(500);
    digitalWrite(klakson,LOW);
    delay(100);
  }
   else if (hidup_alarm==0){
    digitalWrite(klakson,LOW);
   }

while (Serial.available() > 0)
 if (gps.encode(Serial.read()))
 if (gps.location.isValid()){ 
    lcd.setCursor(0,0);
    lcd.print(" Location Avail ");
    strcpy(messages, "http://www.google.com/maps/place/");
    dtostrf(gps.location.lat(), 1, 6, Lat);
    strcat(messages,Lat);
    strcat(messages,"+");
    dtostrf(gps.location.lng(), 1, 6, Lng);
    strcat(messages,Lng);
    strcat(messages,"");
  }
 else{
    strcpy(messages, "location not available");
    lcd.setCursor(0,0);
    lcd.print(" Mencari Lokasi ");
  }
  
  float volt_kontak = analogRead(cek_kontak);
  volt_kontak = volt_kontak*0.004887;
  if (volt_kontak>4 && stt_motor == 0){
    lcd.setCursor(0,0);
    lcd.print("  Kunci aktif   ");
    gprs.sendSMS(my_number,out_cek);
    sendlocation();
    stt_motor=1;
    gprs.callUp(my_number); 
  }


 unsigned long currentMillis = millis()/1000;
 if (currentMillis - previousMillis >= interval) {
  previousMillis = currentMillis;
  stt_motor=0;
  }
 } 
}











void if_sms(){
  if (message[0] == 'Y'){
    keamanan=1;
    buzzer_on();
    gprs.sendSMS(my_number,out_aktif);
    digitalWrite(kontak,LOW);
    hidup_alarm=0;
  }
  else if (message[0] == 'N'){
    keamanan=0;
    buzzer_off();
    gprs.sendSMS(my_number,out_nonaktif);
    digitalWrite(kontak,LOW);
    hidup_alarm=0;
  }
  else if (message[0] == 'M'){digitalWrite(kontak,HIGH);}
  else if (message[0] == 'H'){digitalWrite(kontak,LOW);}
  else if (message[0] == 'C'){    
    strcpy(messages, "http://www.google.com/maps/place/");
    dtostrf(gps.location.lat(), 1, 6, Lat);
    strcat(messages,Lat);
    strcat(messages,"+");
    dtostrf(gps.location.lng(), 1, 6, Lng);
    strcat(messages,Lng);
    strcat(messages,"");
    sendlocation();
    }
  else if (message[0] == '1'){hidup_alarm=1;}
  else if (message[0] == '0'){hidup_alarm=0;}
  
}















/*
void aktif_sms(){
  if (
    phone[0]==my_number[0] && 
    phone[1]==my_number[1] && 
    phone[2]==my_number[2] && 
    phone[3]==my_number[3] && 
    phone[4]==my_number[4] && 
    phone[5]==my_number[5] && 
    phone[6]==my_number[6] && 
    phone[7]==my_number[7] && 
    phone[8]==my_number[8] && 
    phone[9]==my_number[9] && 
    phone[10]==my_number[10] && 
    phone[11]==my_number[11] && 
    phone[12]==my_number[12] &&
    message[0]==in_aktif[0] &&
    message[1]==in_aktif[1] &&
    message[2]==in_aktif[2] &&
    message[3]==in_aktif[3] &&
    message[4]==in_aktif[4] &&
    message[5]==in_aktif[5] &&
    message[6]==in_aktif[6] &&
    message[7]==in_aktif[7] &&
    message[8]==in_aktif[8] &&
    message[9]==in_aktif[9] &&
    message[10]==in_aktif[10] &&
    keamanan==0)
    {
      keamanan=1;
      buzzer_on();
      delay(1000);
      gprs.sendSMS(my_number,out_aktif);
    }
}


void nonaktif_sms(){
  if (
    phone[0]==my_number[0] && 
    phone[1]==my_number[1] && 
    phone[2]==my_number[2] && 
    phone[3]==my_number[3] && 
    phone[4]==my_number[4] && 
    phone[5]==my_number[5] && 
    phone[6]==my_number[6] && 
    phone[7]==my_number[7] && 
    phone[8]==my_number[8] && 
    phone[9]==my_number[9] && 
    phone[10]==my_number[10] && 
    phone[11]==my_number[11] && 
    phone[12]==my_number[12] &&
    message[0]==in_aktif[0] &&
    message[1]==in_aktif[1] &&
    message[2]==in_aktif[2] &&
    message[3]==in_aktif[3] &&
    message[4]==in_aktif[4] &&
    message[5]==in_aktif[5] &&
    message[6]==in_aktif[6] &&
    message[7]==in_aktif[7] &&
    message[8]==in_aktif[8] &&
    message[9]==in_aktif[9] &&
    message[10]==in_aktif[10] &&
    keamanan==1)
    {
      keamanan=0;
      buzzer_on();
      delay(1000);
      gprs.sendSMS(my_number,out_nonaktif);
    }
}




void matikan(){
  if (
    message[0]==in_mati[0] &&
    message[1]==in_mati[1] &&
    message[2]==in_mati[2] &&
    message[3]==in_mati[3] &&
    message[4]==in_mati[4] &&
    message[5]==in_mati[5] &&
    message[6]==in_mati[6] &&
    message[7]==in_mati[7])
    {
     digitalWrite(kontak,HIGH);
    }
}

void cek_lokasi(){
  if (
    message[0]==in_cek[0] &&
    message[1]==in_cek[1] &&
    message[2]==in_cek[2] &&
    message[3]==in_cek[3] &&
    message[4]==in_cek[4] &&
    message[5]==in_cek[5] &&
    message[6]==in_cek[6] &&
    message[7]==in_cek[7])
    {
     sendlocation();
    }
}

void alarm(){
  if (
    message[0]==in_buzzer[0] &&
    message[1]==in_buzzer[1] &&
    message[2]==in_buzzer[2] &&
    message[3]==in_buzzer[3] &&
    message[4]==in_buzzer[4] &&
    message[5]==in_buzzer[5] &&
    message[6]==in_buzzer[6])
    {
     hidup_alarm=1;
    }
}
*/



