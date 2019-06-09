#ifndef QUOTES_H
#define QUOTES_H

#include "Arduino.h"
#include "ArduinoJson.h"

#include "globals.h"
#include "HTTPRequest.h"
#include "Authenticator.h"
#include "Printer.h"

class Quotes{

  Authenticator * auth;
  Printer * printer;
  DynamicJsonDocument * quote;
  HTTPRequest * request;

  void Debug(String s){
    Serial.println(s);
  }
  
  public:

    Quotes(HTTPRequest * req, Authenticator * auth, Printer * printer){
      this->auth = auth;
      this->printer = printer;
      this->request = req;
      quote = new DynamicJsonDocument(MAX_QUOTE_DOC);
    }

    ~Quotes(){
      if(quote){
        delete quote;
      }
    }
  
    DynamicJsonDocument * GetQuote(){
      if(request->GetStatus() != WL_CONNECTED){
        Debug(F("Q.Connecting WiFi"));
        if(request->ConnectWiFi(WIFI_SSID, WIFI_PWD, 3) != WL_CONNECTED ){
        Debug(F("Q. WiFi Init failed"));
        return NULL;
        }
      }

      Response * quoteResponse = request->GetJSON(F("ulifeapps.herokuapp.com"), F("/quotes/quote/printer"), 443, auth->GetAccessToken());
      request->DisconnectAll();
     
      if(quoteResponse->statusCode == 403){
        //if 403, then we assumne the access_token is no longer valid
        Debug(F("Q. API failed - 403"));
        auth->InvalidateToken();
        return NULL;
      }
      
      if(quoteResponse->statusCode != 200){
        Debug(String(F("Q. API failed. (")) + String(quoteResponse->statusCode) + ")");
        return NULL;
      }
     
      DeserializationError err = deserializeJson(*quote, quoteResponse->data->c_str());
      if(err){
        Debug(F("Q. JSON failed"));
        Debug(err.c_str());
        return NULL;
      }
      return quote;
    }

    void Print(){
      auto q = GetQuote();
      if(q){
        const char * text = (*q)["quote"];
        const char * author = (*q)["author"];
        printer->Feed(1);
        printer->Justify('L');
        printer->Print('\"');
        printer->Print(text);
        printer->Print('\"');
        printer->Print('\n');
        printer->Print(author);
        printer->Feed(1);
        return;
      }
    }
};

#endif
