#ifndef PRINTER_H
#define PRINTER_H

#include <Adafruit_Thermal.h>
#include "qrcode.h"

//class Printer{
//  protected:
//    QRCode qrcode;
//    uint8_t * qrcodeBytes;
//    void generateQRCode(const char * data){
//      qrcodeBytes = (uint8_t *) malloc(qrcode_getBufferSize(3));
//      qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, data);
//    }
//
//    void freeQRCode(){
//      free(qrcodeBytes);
//      qrcodeBytes = NULL;  
//    }
//    
//  public:
//
//    Printer(){
//      
//    }
//
//    virtual ~Printer(){
//      if(qrcodeBytes){
//        free(qrcodeBytes); 
//      }
//    }
//  
//    virtual void Print(String s) = 0;
//    virtual void Print(char c) = 0;
//    virtual void Justify(char) = 0;
//    virtual void Feed(int) = 0;
//    virtual void PrintQRcode(unsigned char * url) = 0;
//};
//
//class ThermalPrinter : public Printer{
//  Adafruit_Thermal * printer;
//  public:
//    ThermalPrinter(Stream * s){
//      printer = new Adafruit_Thermal(s);
//    }
//    
//    ~ThermalPrinter(){
//      delete printer;
//    }
//  
//    void Print(String s){
//      printer->println(s);
//    }
//
//    void Print(char c){
//      printer->write(c);
//    }
//
//    void Justify(char c){
//      printer->justify(c);
//    }
//
//    void Feed(int lines){
//      printer->feed(lines);
//    }
//    
//    void PrintQRcode(unsigned char * url){
//      
//    }
//};  

//Mock Printer that uses the Serial Port.
class Printer {
    QRCode qrcode;
    uint8_t * qrcodeBytes;
    void generateQRCode(const char * data){
      qrcodeBytes = (uint8_t *) malloc(qrcode_getBufferSize(3));
      qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, data);
    }

    void freeQRCode(){
      free(qrcodeBytes);
      qrcodeBytes = NULL;  
    }
  public:
    Printer(){
    }
  
    void Print(String s){
      Serial.println(s);
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

    void PrintQRcode(unsigned char * url){
      generateQRCode((const char *)url);
      for (uint8 y = 0; y < qrcode.size; y++){
        for (uint8 x = 0; x < qrcode.size; x++){
          if (qrcode_getModule(&qrcode, x, y) ){
            Serial.print("**");
          } else {
            Serial.print("  ");
          }
        }
        Serial.print("\n");
        freeQRCode();
      }
   }
};


#endif
