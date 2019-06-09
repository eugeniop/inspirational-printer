#include "Arduino.h"
#include "Authenticator.h"
#include "Quotes.h"
#include "Printer.h"
#include "HTTPRequest.h"
#include "activate.h"

Printer printer(&Serial1);
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
  Serial1.begin(19200);
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait fr USB
  }

  while(!Serial1){
    ; //Wait on Printer
  }

  printer.Init();
}

void loop(){

//  printer.Feed(3);
//  printer.SetSize('S');
//  printer.Justify('L');
//  printer.PrintLn("Code: 123456");
//  printer.SetSize('L');
//  printer.PrintLn("123456");
//  printer.Feed(2);
//  printer.PrintBitmap(activate_width, activate_height, activate_data);
//
//  while(1);
  
  if(!auth.IsTokenAvailable()){
    int r = auth.Authenticate();
    printer.Justify('L');
    printer.SetSize('S');
    if(r != AUTH_OK){
      printer.Print("Authentication failed. Please try again!");
    } else {
      printer.Print("Your printer is ready!");
    }
    printer.Feed(2);
    return;
  }
  
  quotes.Print();
  delay(20000);
}
