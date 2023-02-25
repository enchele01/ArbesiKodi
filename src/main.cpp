// 100 000   95.6

//float mm = 95.6;
//long steppa = (100000.0 / 95.6) * samm
/*
*/

#include "avr/wdt.h"
#include <Arduino.h>
#include "ArduinoJson.h"
//#include <AccelStepper.h>
//const int v_max = 6500; // shpejtesia
//const int a_max = 500;  // nxitimi

//Motorat
#define X_M A1
#define X_D A0
#define X_S A2
#define Z_L 5
#define Z_P 4
// lirohen 19 21 26 25

#define stepX 6
#define dirX 7
#define enableX 8
#define jS 13
#define jP A5

//AccelStepper stepperX(1, stepX, dirX);

//#define ANALOGPIN 36

// Limit Switch-at
#define X_LM 10
#define X_LD 9
#define Y_LL 11
#define Y_LP 12

// Sharra
#define SR_ON A3  //  SHARRA_ON
#define SR_OFF A4  // SHARRA_OFF

#define HID_ON 3 // HIDRAULIKI_ON per me leviz X-i
#define HID_OFF 2 //27  // HIDRAULIKI_OFF

void ENBL_HID_X(); // lshoje Hidrollin
void DIS_HID_X();
void ENBL_SR(); // Funksioni i cili aktivizon Sharren
void DIS_SR();// Funksioni i cili deaktivizon SHarren
void STARTO_X_D();
void STARTO_X_M();
void STARTO_X_D_A();
void STARTO_X_M_A();

void STOP_X_S();
void lexo();
void fik_krejt();
void yPara();
void yPas();
void HomeFinish();

void passite();

void lexoSX();

void HomeStart();
void softwareReset( uint8_t prescaller);

long sa_steppa(float samm);
long sa_disku(float off);

long diski = 0;


StaticJsonDocument<200>pranimi;// Json fajlli per pranim te te dhenave
StaticJsonDocument<200>dergesa;// JSON fajlli per dergim te te dhenave
String ctr_mes = "";

int kontrolli = 0;
int komanda = 0;
float samm = 0;
int saHere = 0;
float offseti = 0;

// steppa [int] == round(800 000.0 / 951.5[double] * (Xmm)[double/float])

long steppa = 0;

// steppa = round((800000.0/map_mm)*samm)


void setup() {
    Serial.begin(115200);

    //stepperX.setMinPulseWidth(25);
    pinMode(enableX, OUTPUT);
    pinMode(jS, INPUT);
    pinMode(jP, INPUT);

    digitalWrite(enableX, LOW);
    delay(100);
    digitalWrite(enableX, HIGH);

    //Aktuatoret
    pinMode(X_M, OUTPUT);
    pinMode(X_D, OUTPUT);

    pinMode(Z_L, OUTPUT);
    pinMode(Z_P, OUTPUT);

    // Sharra
    pinMode(SR_ON, OUTPUT);
    pinMode(SR_OFF,OUTPUT);

    // Proximity Switch-at
    pinMode(X_LM, INPUT_PULLUP);
    pinMode(X_LD, INPUT_PULLUP);
    pinMode(Y_LL, INPUT_PULLUP);
    pinMode(Y_LP, INPUT_PULLUP);

    // Over Current Detection
    //pinMode(ANALOGPIN, INPUT);

    //Hidraulik Enable X-in
    pinMode(HID_ON,OUTPUT);
    pinMode(HID_OFF,OUTPUT);

    // Stop X-in
    pinMode(X_S,OUTPUT);


    // LOW trig Relays
    digitalWrite(X_M,HIGH);
    digitalWrite(X_D,HIGH);
    digitalWrite(Z_L,HIGH);
    digitalWrite(Z_P,HIGH);
    digitalWrite(X_S,HIGH);
    digitalWrite(SR_OFF,HIGH);
    digitalWrite(SR_ON,HIGH);
    digitalWrite(HID_ON,HIGH);
    digitalWrite(HID_OFF,HIGH);
    pinMode(stepX,OUTPUT);
    pinMode(dirX,OUTPUT);

//    stepperX.setMaxSpeed(v_max); // Jep shpejtesina
//    stepperX.setAcceleration(a_max); // Jep hygjymin
//    stepperX.disableOutputs();

    for(int i=0 ; i<=2 ; i++) {
        dergesa["x"] = -10;
        char JSONmessageBuffer[100];
        serializeJson(dergesa, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
        delay(300);
    }
}
void loop() {

    if (digitalRead(jS)) {

        digitalWrite(dirX, LOW);

        while (digitalRead(jS) && digitalRead(Y_LP)) {
            digitalWrite(stepX, HIGH);
            delayMicroseconds(7);
            digitalWrite(stepX, LOW);
            delayMicroseconds(7);
        }
    }

    else if (digitalRead(jP)) {

        digitalWrite(dirX, HIGH);

        while (digitalRead(jP) && digitalRead(Y_LL)) {
            digitalWrite(stepX, HIGH);
            delayMicroseconds(7);
            digitalWrite(stepX, LOW);
            delayMicroseconds(7);
        }
    }
    else
    {
        lexo();
    }

}

void lexo(){
    // Ne kete bllok pranohet dhe deserializohet JSON-i
    if (Serial.available() > 0) {
        ctr_mes = Serial.readStringUntil(' ');
    }
    DeserializationError error = deserializeJson(pranimi, ctr_mes);
    if(error){
        //Serial.println("Deshtim ne Deserializim");
    }else {

        kontrolli = pranimi["k"];
        komanda = pranimi ["c"];
        samm    = pranimi["m"];
        saHere  = pranimi["h"];
        offseti = pranimi["o"];

        steppa = sa_steppa(samm);
        diski = sa_disku(offseti);
    }
    if(kontrolli == 0){

        switch (komanda) {
            case 101: softwareReset(WDTO_30MS);
                break;
            case 8: digitalWrite(Z_L,LOW);
                break;
            case 2: digitalWrite(Z_P,LOW);
                break;
            case 1: digitalRead(Y_LL)?yPara():passite();
                break;
            case 9: digitalRead(Y_LP)?yPas():passite();
                break;
            case 6: digitalRead(X_LD)?STARTO_X_D():passite();
                break;
            case 4: digitalRead(X_LM)?STARTO_X_M():passite();
                break;
            case 32: digitalWrite(HID_OFF, LOW);
                break;
            case 33: digitalWrite(HID_ON, LOW);
                break;
            case 55: digitalWrite(SR_ON, LOW);
                break;
            case 66: digitalWrite(SR_OFF, LOW);
                break;
            case 77: digitalWrite(X_S, LOW);
                break;
            case 99: HomeStart();
                break;
            default: fik_krejt();
        }
    }
    else if(kontrolli == 1){
        steppa = sa_steppa(samm);
        diski = sa_disku(offseti);
        int hapi = 0;
        ENBL_HID_X();
        ENBL_SR();

        while(hapi<=saHere){

            delay(500);
            STARTO_X_M();
            delay(500);
            STARTO_X_D();
            delay(500);

            digitalWrite(dirX, HIGH);

            for(int i=0 ; (i<=steppa+diski && digitalRead(Y_LL)) ; i++) {

                if (i > 50000) {
                    digitalWrite(stepX, HIGH);
                    delayMicroseconds(50);
                    digitalWrite(stepX, LOW);
                    delayMicroseconds(50);
                } else {
                    digitalWrite(stepX, HIGH);
                    delayMicroseconds(12);
                    digitalWrite(stepX, LOW);
                    delayMicroseconds(12);
                }
            }

            hapi +=1;
        }

        delay(300);
        DIS_SR();
        delay(300);
        DIS_HID_X();
        delay(300);

        softwareReset( WDTO_30MS);
    }

}

void HomeStart() {
    STARTO_X_D();
    delay(500);
    digitalWrite(dirX, LOW);

    while (digitalRead(Y_LP)){
        digitalWrite(stepX, HIGH);
        delayMicroseconds(7);
        digitalWrite(stepX, LOW);
        delayMicroseconds(7);

    }

    delay(300);

    softwareReset( WDTO_30MS);
}


void HomeFinish() {
    STARTO_X_D();

    delay(500);

    digitalWrite(dirX, HIGH);


    while (digitalRead(Y_LL)){
        digitalWrite(stepX, HIGH);
        delayMicroseconds(20);
        digitalWrite(stepX, LOW);
        delayMicroseconds(20);
    }

    delay(300);


    softwareReset( WDTO_30MS);
}



void passite() {

}

void STARTO_X_D(){

    digitalWrite(X_D, LOW);
    delay(300);
    digitalWrite(X_D, HIGH);

    while(digitalRead(X_LD)){
        lexoSX();
        if(kontrolli == 2 || komanda == 77){
            STOP_X_S();
            delay(300);
            softwareReset( WDTO_30MS);
        }
    }
    STOP_X_S();



}

void STARTO_X_D_A(){

    digitalWrite(X_D, LOW);
    delay(700);
    digitalWrite(X_D, HIGH);


    while(digitalRead(X_LD)){

    }
    STOP_X_S();

}



void STARTO_X_M(){
    digitalWrite(X_M, LOW);
    delay(300);
    digitalWrite(X_M, HIGH);

    while(digitalRead(X_LM)){
        lexoSX();
        if(kontrolli == 2 || komanda == 77){
            STOP_X_S();
            delay(300);
            softwareReset( WDTO_30MS);
        }
    }
    STOP_X_S();
}

void STARTO_X_M_A(){
    digitalWrite(X_M, LOW);
    delay(700);
    digitalWrite(X_M, HIGH);

    while(digitalRead(X_LM)){

    }
    STOP_X_S();
}



void STOP_X_S(){
    // puls HIDRAULIKA ENBL X
    digitalWrite(X_S,LOW);
    delay(200);
    digitalWrite(X_S,HIGH);
    //delay(200);
    //DIS_HID_X();
}

void ENBL_HID_X(){
    // puls HIDRAULIKA ENBL X
    digitalWrite(HID_ON,LOW);
    delay(700);
    digitalWrite(HID_ON,HIGH);
}
void DIS_HID_X(){
    // puls HIDRAULIKA DISBL X
    digitalWrite(HID_OFF,LOW);
    delay(700);
    digitalWrite(HID_OFF,HIGH);
}

void fik_krejt(){

    digitalWrite(Z_L,HIGH);
    digitalWrite(Z_P,HIGH);
    digitalWrite(X_S, HIGH);
    digitalWrite(HID_ON,HIGH);
    digitalWrite(HID_OFF, HIGH);
    digitalWrite(SR_ON,HIGH);
    digitalWrite(SR_OFF,HIGH);
    digitalWrite(X_M, HIGH);
    digitalWrite(X_D, HIGH);

}

void ENBL_SR(){
    // puls per sharren ON
    digitalWrite(SR_ON, LOW);
    delay(700);
    digitalWrite(SR_ON,HIGH);

}

void DIS_SR(){
    // puls per sharren OFF
    digitalWrite(SR_OFF, LOW);
    delay(700);
    digitalWrite(SR_OFF,HIGH);
}

void yPara(){

    steppa = sa_steppa(samm);
    //steppa = samm;
    digitalWrite(dirX, HIGH);

    for(int i=0 ; i<=2 ; i++) {
        dergesa["x"] = 1;
        char JSONmessageBuffer[100];
        serializeJson(dergesa, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
        delay(100);
    }
    delay(300);

    for(int i=0 ; (i<=steppa && digitalRead(Y_LL)) ; i++){

        if(i>50000){
            digitalWrite(stepX, HIGH);
            delayMicroseconds(50);
            digitalWrite(stepX, LOW);
            delayMicroseconds(50);
        }
        else {
            digitalWrite(stepX, HIGH);
            delayMicroseconds(12);
            digitalWrite(stepX, LOW);
            delayMicroseconds(12);
        }
    }

    for(int i=0 ; i<=2 ; i++) {
        dergesa["x"] = 0;
        char JSONmessageBuffer[100];
        serializeJson(dergesa, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
        delay(100);
    }

}




void yPas(){
    digitalWrite(dirX, LOW);
    steppa = sa_steppa(samm);
    //steppa = samm;

    for(int i=0 ; i<=2 ; i++) {
        dergesa["x"] = 1;
        char JSONmessageBuffer[100];
        serializeJson(dergesa, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
        delay(100);
    }
    delay(300);

    for(int i=0 ; i<=steppa && digitalRead(Y_LP) ; i++){

        if(i>50000){
            digitalWrite(stepX, HIGH);
            delayMicroseconds(50);
            digitalWrite(stepX, LOW);
            delayMicroseconds(50);
        }
        else {
            digitalWrite(stepX, HIGH);
            delayMicroseconds(12);
            digitalWrite(stepX, LOW);
            delayMicroseconds(12);
        }
    }


    for(int i=0 ; i<=2 ; i++) {
        dergesa["x"] = 0;
        char JSONmessageBuffer[100];
        serializeJson(dergesa, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
        delay(100);
    }
}

void lexoSX() {

    // Ne kete bllok pranohet dhe deserializohet JSON-i
    if (Serial.available() > 0) {
        ctr_mes = Serial.readStringUntil(' ');
    }
    DeserializationError error = deserializeJson(pranimi, ctr_mes);
    if(error){
        //Serial.println("Deshtim ne Deserializim");
    }else {

        kontrolli = pranimi["k"];
        komanda = pranimi ["c"];
        samm    = pranimi["m"];
        saHere  = pranimi["h"];
        offseti = pranimi["o"];

    }

}


void softwareReset( uint8_t prescaller) {
    // start watchdog with the provided prescaller
    wdt_enable( prescaller);
    // wait for the prescaller time to expire
    // without sending the reset signal by using
    // the wdt_reset() method
    while(1) {}
}

long sa_steppa(float samm){

    return round((1200000.0 / 1143.1) * samm);
}

long sa_disku(float off){

    return round((1200000.0 / 1143.1) * off);

}


//######################################################///


//// 800 000 steppa  = = = =  951.5 mm
//#include "avr/wdt.h"
//// steppa [int] == round(800 000.0 / 951.5[double] * (Xmm)[double/float])
//#include <Arduino.h>
//#include "ArduinoJson.h"
//#include <AccelStepper.h>
//
//const int v_max = 5500; // shpejtesia
//const int a_max = 850;  // nxitimi
//
////Motorat
//#define X_M A2
//#define X_D A1
//#define X_S A0
//#define Z_L 5
//#define Z_P 4
//// lirohen 19 21 26 25
//
//#define stepX 6
//#define dirX 7
//#define enableX 8
//#define jS A5
//#define jP 13
//
//AccelStepper stepperX(1, stepX, dirX);
//
//#define ANALOGPIN 36
//
//// Limit Switch-at
//#define X_LM 10
//#define X_LD 9
//#define Y_LL 11
//#define Y_LP 12
//
//// Sharra
//#define SR_ON A4  //  SHARRA_ON
//#define SR_OFF A3  // SHARRA_OFF
//
//#define HID_ON 2 // HIDRAULIKI_ON per me leviz X-i
//#define HID_OFF 3 //27  // HIDRAULIKI_OFF
//
//void ENBL_HID_X(); // lshoje Hidrollin
//void DIS_HID_X();
//void ENBL_SR(); // Funksioni i cili aktivizon Sharren
//void DIS_SR();// Funksioni i cili deaktivizon SHarren
//void STARTO_X_D();
//void STARTO_X_M();
//void STARTO_X_D_A();
//void STARTO_X_M_A();
//void STOP_X_S();
//void lexo();
//void fik_krejt();
//void yPara();
//void yPas();
//
//void passite();
//
//void lexoSX();
//
//void HomeStart();
//void softwareReset( uint8_t prescaller);
//int sa_more = 0;
//
//StaticJsonDocument<200>pranimi;// Json fajlli per pranim te te dhenave
//StaticJsonDocument<200>dergesa;// JSON fajlli per dergim te te dhenave
//String ctr_mes = "";
//
//int kontrolli = 0;
//int komanda = 0;
//float samm = 0;
//int saHere = 0;
//int offseti = 0;
//
//// steppa [int] == round(800 000.0 / 951.5[double] * (Xmm)[double/float])
//
//int steppa = 0;
//float map_mm = 951.5;
//
//// steppa = round((800000.0/map_mm)*samm)
//
//
//void setup() {
//    Serial.begin(115200);
//
//    stepperX.setMinPulseWidth(25);
//    pinMode(enableX, OUTPUT);
//    pinMode(jS, INPUT);
//    pinMode(jP, INPUT);
//
//    digitalWrite(enableX, LOW);
//
//    //Aktuatoret
//    pinMode(X_M, OUTPUT);
//    pinMode(X_D, OUTPUT);
//
//    pinMode(Z_L, OUTPUT);
//    pinMode(Z_P, OUTPUT);
//
//    // Sharra
//    pinMode(SR_ON, OUTPUT);
//    pinMode(SR_OFF,OUTPUT);
//
//    // Proximity Switch-at
//    pinMode(X_LM, INPUT);
//    pinMode(X_LD, INPUT);
//    pinMode(Y_LL, INPUT);
//    pinMode(Y_LP, INPUT);
//
//    // Over Current Detection
//    pinMode(ANALOGPIN, INPUT);
//
//    //Hidraulik Enable X-in
//    pinMode(HID_ON,OUTPUT);
//    pinMode(HID_OFF,OUTPUT);
//
//    // Stop X-in
//    pinMode(X_S,OUTPUT);
//
//
//    // LOW trig Relays
//    digitalWrite(X_M,HIGH);
//    digitalWrite(X_D,HIGH);
//    digitalWrite(Z_L,HIGH);
//    digitalWrite(Z_P,HIGH);
//    digitalWrite(X_S,HIGH);
//    digitalWrite(SR_OFF,HIGH);
//    digitalWrite(SR_ON,HIGH);
//    digitalWrite(HID_ON,HIGH);
//    digitalWrite(HID_OFF,HIGH);
//
//
//    stepperX.setMaxSpeed(v_max); // Jep shpejtesina
//    stepperX.setAcceleration(a_max); // Jep hygjymin
//    stepperX.disableOutputs();
//
//    for(int i=0 ; i<=2 ; i++) {
//        dergesa["x"] = -10;
//        char JSONmessageBuffer[100];
//        serializeJson(dergesa, JSONmessageBuffer);
//        Serial.println(JSONmessageBuffer);
//        delay(1000);
//    }
//}
//int nx = 1000;
//void loop() {
//    nx = 1000;
//    lexo();
//
//    if (digitalRead(jS)) {
//        digitalWrite(enableX, LOW);
//        digitalWrite(dirX, HIGH);
//
//        while (digitalRead(jS) && digitalRead(Y_LP)) {
//            nx = nx - 10;
//            digitalWrite(stepX, HIGH);
//            delayMicroseconds(nx);
//            digitalWrite(stepX, LOW);
//            delayMicroseconds(nx);
//        }
//        digitalWrite(enableX, HIGH);
//        nx = 1000;
//
//    }
//
//
//    if (digitalRead(jP)) {
//        digitalWrite(enableX, LOW);
//        digitalWrite(dirX, LOW);
//
//        while (digitalRead(jS) && digitalRead(Y_LP)) {
//            nx = nx - 10;
//            digitalWrite(stepX, HIGH);
//            delayMicroseconds(nx);
//            digitalWrite(stepX, LOW);
//            delayMicroseconds(nx);
//        }
//        nx = 1000;
//        digitalWrite(enableX, HIGH);
//
//    }
//
//}
//void lexo(){
//    // Ne kete bllok pranohet dhe deserializohet JSON-i
//    if (Serial.available() > 0) {
//        ctr_mes = Serial.readStringUntil(' ');
//    }
//    DeserializationError error = deserializeJson(pranimi, ctr_mes);
//    if(error){
//        //Serial.println("Deshtim ne Deserializim");
//    }else {
//
//        kontrolli = pranimi["k"];
//        komanda = pranimi ["c"];
//        samm    = pranimi["m"];
//        saHere  = pranimi["h"];
//        offseti = pranimi["o"];
//
//        steppa = round((800000.0/map_mm)*samm);
//    }
//    if(kontrolli == 0){
//        switch (komanda) {
//            case 101: softwareReset(WDTO_30MS);
//                break;
//            case 8: digitalWrite(Z_L,LOW);
//                break;
//            case 2: digitalWrite(Z_P,LOW);
//                break;
//            case 9: digitalRead(Y_LL)?yPara():passite();
//                break;
//            case 1: digitalRead(Y_LP)?yPas():passite();
//                break;
//            case 6: digitalRead(X_LD)?STARTO_X_D():passite();
//                break;
//            case 4: digitalRead(X_LM)?STARTO_X_M():passite();
//                break;
//            case 32: DIS_HID_X();
//                break;
//            case 33: ENBL_HID_X();
//                break;
//            case 55: ENBL_SR();
//                break;
//            case 66: DIS_SR();
//                break;
//            case 77: STOP_X_S();
//                break;
//            case 99: HomeStart();
//                break;
//            default: fik_krejt();
//        }
//    }
//    else if(kontrolli == 1){
//
//        steppa = round((800000.0/map_mm)*samm);
//
//        int hapi = 0;
//        ENBL_HID_X();
//        ENBL_SR();
//
//        while(hapi<=saHere){
//            if(hapi == 0){
//
//                steppa = round((800000.0/map_mm)*(samm+offseti+1));
//
//            }
//            else{
//
//                steppa = round((800000.0/map_mm)*(samm+offseti));
//            }
//
//            delay(1000);
//            STARTO_X_M();
//            delay(1000);
//            STARTO_X_D();
//            delay(1000);
//
//
//
//            stepperX.setAcceleration(a_max);
//            stepperX.setMaxSpeed(v_max);
//            stepperX.move((-1) * steppa);
//
//            digitalWrite(enableX, LOW);
//            while (abs(stepperX.currentPosition()) < steppa && digitalRead(Y_LL)) {
//                stepperX.enableOutputs();
//                stepperX.run();
//            }
//
//            stepperX.disableOutputs();
//            stepperX.setCurrentPosition(0);
//            digitalWrite(enableX, HIGH);
//
//
//            hapi +=1;
//        }
//        delay(300);
//        DIS_SR();
//        delay(300);
//        DIS_HID_X();
//        delay(300);
//
//        softwareReset( WDTO_30MS);
//
//    }
//
//}
//
//void HomeStart() {
//    STARTO_X_D();
//    delay(500);
//    digitalWrite(enableX, LOW);
//    stepperX.setAcceleration(a_max);
//    stepperX.setMaxSpeed(v_max);
//    sa_more= 999999999;
//    stepperX.move(sa_more);
//
//    while (digitalRead(Y_LP)){
//        stepperX.enableOutputs();
//        stepperX.run();
//    }
//    stepperX.disableOutputs();
//    stepperX.setCurrentPosition(0);
//    delay(300);
//
//    digitalWrite(enableX, HIGH);
//
//    softwareReset( WDTO_30MS);
//}
//
//void passite() {
//
//}
//
//void STARTO_X_D(){
//
//    digitalWrite(X_D, LOW);
//    delay(500);
//    digitalWrite(X_D, HIGH);
//
//
//    while(digitalRead(X_LD)){
//        lexoSX();
//        if(kontrolli == 2 || komanda == 77){
//            STOP_X_S();
//            delay(300);
//            softwareReset( WDTO_30MS);
//        }
//    }
//    STOP_X_S();
//
//}
//
//void STARTO_X_D_A(){
//
//    digitalWrite(X_D, LOW);
//    delay(700);
//    digitalWrite(X_D, HIGH);
//
//
//    while(digitalRead(X_LD)){
//
//    }
//    STOP_X_S();
//
//}
//
//
//
//void STARTO_X_M(){
//    digitalWrite(X_M, LOW);
//    delay(700);
//    digitalWrite(X_M, HIGH);
//
//
//    while(digitalRead(X_LM)){
//        lexoSX();
//        if(kontrolli == 2 || komanda == 77){
//            STOP_X_S();
//            delay(100);
//            softwareReset( WDTO_30MS);
//        }
//    }
//    STOP_X_S();
//}
//
//void STARTO_X_M_A(){
//    digitalWrite(X_M, LOW);
//    delay(700);
//    digitalWrite(X_M, HIGH);
//
//    while(digitalRead(X_LM)){
//
//    }
//    STOP_X_S();
//}
//
//
//
//void STOP_X_S(){
//    // puls HIDRAULIKA ENBL X
//    digitalWrite(X_S,LOW);
//    delay(200);
//    digitalWrite(X_S,HIGH);
//    //delay(200);
//    //DIS_HID_X();
//}
//
//void ENBL_HID_X(){
//    // puls HIDRAULIKA ENBL X
//    digitalWrite(HID_ON,LOW);
//    delay(300);
//    digitalWrite(HID_ON,HIGH);
//    delay(300);
//}
//void DIS_HID_X(){
//    // puls HIDRAULIKA DISBL X
//    digitalWrite(HID_OFF,LOW);
//    delay(100);
//    digitalWrite(HID_OFF,HIGH);
//}
//
//void fik_krejt(){
//
//    digitalWrite(Z_L,HIGH);
//    digitalWrite(Z_P,HIGH);
//
//
//}
//
//void ENBL_SR(){
//    // puls per sharren ON
//    digitalWrite(SR_ON, LOW);
//    delay(200);
//    digitalWrite(SR_ON,HIGH);
//
//}
//
//void DIS_SR(){
//    // puls per sharren OFF
//    digitalWrite(SR_OFF, LOW);
//    delay(300);
//    digitalWrite(SR_OFF,HIGH);
//}
//
//void yPara(){
//    digitalWrite(enableX, LOW);
//    for(int i=0 ; i<=2 ; i++) {
//        dergesa["x"] = 1;
//        char JSONmessageBuffer[100];
//        serializeJson(dergesa, JSONmessageBuffer);
//        Serial.println(JSONmessageBuffer);
//        delay(100);
//    }
//    delay(300);
//
//    stepperX.setAcceleration(a_max);
//    stepperX.setMaxSpeed(v_max);
//    stepperX.move(steppa);
//
//    while (abs(stepperX.currentPosition()) < steppa  && digitalRead(Y_LP) ){
//        stepperX.enableOutputs();
//        stepperX.run();
//    }
//
//    stepperX.disableOutputs();
//    stepperX.setCurrentPosition(0);
//
//    delay(300);
//    for(int i=0 ; i<=2 ; i++) {
//        dergesa["x"] = 0;
//        char JSONmessageBuffer[100];
//        serializeJson(dergesa, JSONmessageBuffer);
//        Serial.println(JSONmessageBuffer);
//        delay(100);
//    }
//}
//
//
//void yPas(){
//    digitalWrite(enableX, LOW);
//    for(int i=0 ; i<=2 ; i++) {
//        dergesa["x"] = 1;
//        char JSONmessageBuffer[100];
//        serializeJson(dergesa, JSONmessageBuffer);
//        Serial.println(JSONmessageBuffer);
//        delay(100);
//    }
//
//    stepperX.setAcceleration(a_max);
//    stepperX.setMaxSpeed(v_max);
//    stepperX.move((-1)*steppa);
//
//    while (abs(stepperX.currentPosition()) < steppa && digitalRead(Y_LL)){
//        stepperX.enableOutputs();
//        stepperX.run();
//    }
//
//
//    stepperX.disableOutputs();
//    stepperX.setCurrentPosition(0);
//
//    for(int i=0 ; i<=2 ; i++) {
//        dergesa["x"] = 0;
//        char JSONmessageBuffer[100];
//        serializeJson(dergesa, JSONmessageBuffer);
//        Serial.println(JSONmessageBuffer);
//
//        delay(100);
//    }
//
//}
//
//void lexoSX() {
//
//    // Ne kete bllok pranohet dhe deserializohet JSON-i
//    if (Serial.available() > 0) {
//        ctr_mes = Serial.readStringUntil(' ');
//    }
//    DeserializationError error = deserializeJson(pranimi, ctr_mes);
//    if(error){
//        //Serial.println("Deshtim ne Deserializim");
//    }else {
//
//        kontrolli = pranimi["k"];
//        komanda = pranimi ["c"];
//        samm    = pranimi["m"];
//        saHere  = pranimi["h"];
//        offseti = pranimi["o"];
//
//    }
//
//}
//
//
//void softwareReset( uint8_t prescaller) {
//    // start watchdog with the provided prescaller
//    wdt_enable( prescaller);
//    // wait for the prescaller time to expire
//    // without sending the reset signal by using
//    // the wdt_reset() method
//    while(1) {}
//}

