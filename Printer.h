#ifndef PRINTER_H
#define PRINTER_H

#include "Arduino.h"
#include <Adafruit_Thermal.h>

#ifndef MOCK_PRINTER
class Printer {
  Adafruit_Thermal * printer;
  public:
    Printer(Stream * stream){
      printer = new Adafruit_Thermal(stream);      
    }

    ~Printer(){
     //delete printer;
    }

    void Init(){
      printer->begin();
    }
    
    void Print(String s){
      printer->print(s);
    }

    void Print(char c){
      printer->write(c);
    }

    void PrintLn(String s){
      printer->println(s);
    }

    void SetSize(char s){
      printer->setSize(s);
    }

    void Justify(char c){
      printer->justify(c);
    }

    void Feed(int lines){
      printer->feed(lines);
    }
    
    void PrintBitmap(int width, int height, const unsigned char * qrcode){
      printer->printBitmap(width, height, qrcode);
    }
};  

#else

//Mock Printer that uses the Serial Port.
class Printer {
  public:
    Printer(){
    }
  
    void Print(String s){
      Serial.print(s);
    }

    void Print(char c){
      Serial.write(c);
    }

    void Justify(char c){
      //NoOp
    }

    void Feed(int lines){
      for(int i=0; i<lines; i++){
        Serial.write('\n');  
      }
    }

    void PrintBitmap(int w, int h, const unsigned char * bmp){
      Serial.println("Width: " + w);
      Serial.println("Height: " + h);
   }
};
#endif

#endif
