#include <Wire.h>
#define addr 0x1E


//motor azimuth
int pinA1 = 2; //pin motorA positif
int pinA2 = 4; //pin motorA negatif
int speedA = 3; 
//motor elevasi
int pinB1 = 6; //pin motorB positif
int pinB2 = 7; //pin motorB negatif
int speedB = 5;
//compassing
int x,y,z;
//positioning dari gps
int posGetX; 
int posGetZ;
float posSat;
int posMax; //nilai paling tinggi di variabel gps
float kodeSat = -0.505; //kode posisi satelit
int kodeelevasi = -35; //ieu asa teu dipake
//signal
int sensBuff;
int sens;
//trigger
int cari;
int c; //counter

void setup() {
  Serial.begin(9600);
  pinMode(pinA1,OUTPUT);
  pinMode(pinA2,OUTPUT);
  pinMode(speedA,OUTPUT);
  pinMode(pinB1,OUTPUT);
  pinMode(pinB2,OUTPUT);
  pinMode(speedB,OUTPUT);
  Wire.begin();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x02); // Set the Register
  Wire.write(0x00); // Tell the HMC5883 to Continuously Measure
  Wire.endTransmission();
}

//ieu jang baca arah antena realtime
void  cekArah(){
  Wire.beginTransmission(addr);
  Wire.write(0x03); //start with register 3.
  Wire.endTransmission();
  Wire.requestFrom(addr, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //MSB  x 
    x |= Wire.read(); //LSB  x
    z = Wire.read()<<8; //MSB  z
    z |= Wire.read(); //LSB z
    y = Wire.read()<<8; //MSB y
    y |= Wire.read(); //LSB y
    posGetX = x;
    posGetZ = z;
  }  
}

//ieu jang berhenti semua motor 
void motorStop(){
     analogWrite(speedA, 0);
     digitalWrite(pinA1, LOW);
     digitalWrite(pinA2, LOW);     
     analogWrite(speedB, 0);
     digitalWrite(pinB1, LOW);
     digitalWrite(pinB2, LOW);    
}
//antena gerak ke kanan
void aziKanan(){
    analogWrite(speedA, 70);
    digitalWrite(pinA1, HIGH);
    digitalWrite(pinA2, LOW);
    Serial.print("AZIMUTH KANAN : ");
    cekArah();
    Serial.print(posGetX);
}
//antena gerak ke kiri
void aziKiri(){
    analogWrite(speedA, 66);
    digitalWrite(pinA1, LOW);
    digitalWrite(pinA2, HIGH);
    Serial.print("AZIMUTH KIRI : ");
    cekArah();
    Serial.print(posGetX);
}
//elevasi antena naik
void elevNaik(){
    analogWrite(speedB, 240);
    digitalWrite(pinB1, HIGH);
    digitalWrite(pinB2, LOW);
    cekArah();
    Serial.print("ELEVASI NAIK ");
    Serial.println(posGetZ);
}
//elevasi antena turun
void elevTurun(){
    analogWrite(speedB, 240);
    digitalWrite(pinB1, LOW);
    digitalWrite(pinB2, HIGH);
    cekArah();
    Serial.print("ELEVASI TURUN ");
    Serial.println(posGetZ);
}
//baca tegangan dari satfinder
void sensGet(){
    sensBuff=0;
	//nilai satfinder diperhalus dirataratakeun
    for (c=0;c<25;c++){
        sensBuff=sensBuff+analogRead(A0);
    }
    sensBuff=sensBuff/25;
    Serial.print(" ");
    Serial.println(sensBuff);
}
//tracking elevasi sedikit, atau disebut FINE TUNING
void cekNaik(){
      elevNaik();
      delay(1300);
      motorStop();
      sensGet();
      Serial.println(sensBuff);
      if(sensBuff>sens){
        sens=sensBuff;
        Serial.print("DAPET LEBIH GEDE ");
        Serial.println(sensBuff);
      } else if (sensBuff<sens){
        elevTurun();
        delay(1300);
        motorStop();
        Serial.print("DAPET LEBIH KECIL ");
        Serial.println(sensBuff);
      }
}
//ieu program utama, standby nunggu tombol dipencet
void loop() {

  if (digitalRead(8) == HIGH) {
    aziKanan();
    sensGet();
  } else if (digitalRead(9) == HIGH) {
    aziKiri();
    sensGet();
  } else if (digitalRead(10) == HIGH) {
    elevNaik();
    sensGet();
  } else if (digitalRead(11) == HIGH) {
    elevTurun();
    sensGet();
  } else if (digitalRead(12) == HIGH) {
	//ieu fungsi pointing
    cari = 1;
    if (cari == 1){
      cekArah();
      c=0;
	  //pertama aya kalibrasi nilai gps heula
	  //antena diputerkeun selama sekitar 6 detik.
      for (c=0;c<270;c++){
        aziKanan();
        cekArah();
        if (posGetX>posMax){
          posMax=posGetX;
        }
      Serial.println(c);  
      }
      motorStop();
      Serial.println("POSISI MAKSIMAL : ");
      Serial.println(posMax);
      Serial.println("KALIBRASI OK");
      delay(1300);
    //kalibrasi beres trus pointing ke kode satelit nu nggeus disimpen
      cekArah();
      Serial.println(kodeSat);
      posSat=posMax*kodeSat;
      Serial.println(posSat);
      while (posGetX>posSat){
        aziKiri();
        cekArah();
        Serial.println();
      }
    sensGet();
      
    motorStop();
    Serial.print("AZIMUTH POINTING OK ON : ");
    Serial.println(posGetX);
    delay(750);
    cekArah();

    
      //Cek Elevasi halus mulai
      if (sensBuff < 18){
        elevTurun();
        delay(1000);
        motorStop();
        delay(500);
        sensGet();
          if(sensBuff>sens){
            sens=sensBuff;
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          } else if (sensBuff<sens){
            elevNaik();
            delay(2000);
            motorStop();
            Serial.println("DAPET LEBIH KECIL");
            delay(500);
          }else if (sensBuff<sens){
            elevTurun();
            delay(1000);
            motorStop();
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          }
      }
      //Cek Elevasi halus berhenti
    Serial.println("tracking kasar selesai");
    delay(1000);

    }
     cari = 0;
  } else if (digitalRead(13) == HIGH) {
    Serial.println("SOFT SEARCH");
    cari = 1;
    sensGet();
    sens = sensBuff;

    if (cari == 1){

    //tracking azimut halus mulai
	//tracking cuma berfungsi ketika
	//tegangan didapat dari satfinder < dari 18
      if (sensBuff < 18){
      aziKanan();
      delay(100);
      motorStop();
      delay(500);
      sensGet();
        if(sensBuff>sens){
          sens=sensBuff;
          Serial.println("DAPET LEBIH GEDE");
        } else if (sensBuff<sens){
          aziKiri();
          delay(200);
          motorStop();
          Serial.println("DAPET LEBIH KECIL");
          delay(500);
        } else if (sensBuff<sens){
          aziKanan();
          delay(100);
          motorStop();
          Serial.println("DAPET LEBIH GEDE");
          delay(500);
        }
        if (sensBuff < 18){
          //CEK 2
        aziKanan();
        delay(350);
        motorStop();
        delay(500);
        sensGet();
          if(sensBuff>sens){
            sens=sensBuff;
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          } else if (sensBuff<sens){
            aziKiri();
            delay(700);
            motorStop();
            Serial.println("DAPET LEBIH KECIL");
            delay(500);
          }else if (sensBuff<sens){
            aziKiri();
            delay(350);
            motorStop();
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          }
        }
      }
      ////tracking azimut halus selesai
      
      //Cek Elevasi halus mulai
      if (sensBuff < 17){
        elevTurun();
        delay(1000);
        motorStop();
        delay(500);
        sensGet();
          if(sensBuff>sens){
            sens=sensBuff;
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          } else if (sensBuff<sens){
            elevNaik();
            delay(2000);
            motorStop();
            Serial.println("DAPET LEBIH KECIL");
            delay(500);
          }else if (sensBuff<sens){
            elevTurun();
            delay(1000);
            motorStop();
            Serial.println("DAPET LEBIH GEDE");
            delay(500);
          }
      }
      //Cek Elevasi halus berhenti
    }
    cari = 0;
  } else {
    cekArah();
    Serial.print("Pos: ");
    Serial.print(x);
    Serial.print(" ");
    Serial.print(y);
    Serial.print(" ");
    Serial.print(z);
    Serial.print(" | Volt: ");
    sensGet();
    motorStop();
  }
}
