#include "Arduino.h"
#include "Globals.h"
#include "Authenticator.h"
#include "Quotes.h"
#include "Printer.h"

//ThermalPrinter printer(&Serial1);
Printer printer;
HTTPRequest request;
Authenticator auth(&request, &printer);
Quotes quotes(&request, &auth, &printer);

void WaitKey(){
  while(Serial.available() <= 0 ){
    delay(200);
  }
  
  while( Serial.available() > 0 ){
    int c = Serial.read();
  }
  return;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait fr USB
  }
}

void loop(){
  Debug(F("Press any key"));
  WaitKey();
//  if(!auth.IsTokenAvailable()){
//    int r = auth.Authenticate();
//    if(r != AUTH_OK){
//      printer.Print("Authentication failed. Please try again!");
//    } else {
//      printer.Print("Your printer is ready!");
//    }
//    return;
//  }

  Debug(F("Getting quote!"));
  quotes.Print();
}