#ifndef AUTH_H
#define AUTH_H

#include "Arduino.h"
#include "DeviceFlow.h"
#include "ArduinoJson.h"
#include "Printer.h"
#include "HTTPRequest.h"

enum AUTHENTICATION_STATUS { AUTH_OK, AUTH_START_FAILED, AUTH_TOKEN_FAILED };

class Authenticator {

  Printer * printer;
  HTTPRequest * request;

  DeviceFlowOptions options = {
    AUTHZ_SERVER, //authServer
    AUTHZ_PATH, //authorizationPath
    CLIENT_ID, //clientId
    SCOPE, //scope
    TOKEN_PATH, //tokenPath
    AUDIENCE //audience
  };

  String * accessToken;

  DynamicJsonDocument * ParseJSON(String * input){
    auto JSON = new DynamicJsonDocument(MAX_AUTHZ_DOC);
    DeserializationError err = deserializeJson(*JSON, input->c_str());
    if(err){
      Debug(F("A.ParseJSON. Error:"));
      Debug(err.c_str());
      delete JSON;
      return NULL;
    }
    return JSON;
  };

  void Debug(String s){
    Serial.println(s);
  }

  int isSlowDown(const char * error){
    return 0 == strcmp(error, "access_denied");
  }

  int isAuthorizationPending(const char * error){
    return 0 == strcmp(error, "authorization_pending");
  }
  
 public:

  Authenticator(HTTPRequest * req, Printer * printer){
    this->request = req;
    this->printer = printer;
  }

  int IsTokenAvailable(){
    return (accessToken && accessToken->length());
  }

  const char * GetAccessToken(){
    if(accessToken){
      return accessToken->c_str();
    }
    return NULL;
  }

  void InvalidateToken(){
    if(accessToken){ 
      delete accessToken;
    }
    accessToken = NULL;
  }
  
  AUTHENTICATION_STATUS Authenticate(){
    DeviceFlow df(&options, request);
    auto res = df.StartAuthorization();
    if(!res){
      Debug(F("Auth. Start Failed"));
      return AUTH_START_FAILED;
    }
  
    if(200 != res->statusCode){
      Debug(String(F("Auth. Start failed with code: ")) + String(res->statusCode));
      return AUTH_START_FAILED;
    }

    auto * authzJSON = ParseJSON(res->data);
    if(!authzJSON){
      return AUTH_START_FAILED;
    }
  
    const char * verification_url_complete = (*authzJSON)["verification_uri_complete"];
    const char * user_code = (*authzJSON)["user_code"];
  
    printer->Print("Please visit this URL: " + String(verification_url_complete));
    printer->Print("If prompted, please enter this code when prompted:");
    printer->Print(String(user_code));
    printer->PrintQRcode((unsigned char *)verification_url_complete);
    
    char device_code[MAX_DEVICE_CODE];
    strcpy(device_code, (*authzJSON)["device_code"]);
    
    auto interval = (*authzJSON)["interval"].as<int>() * 1000;  //convert to ms
    delete authzJSON;

    int hard_retries = 5;

    while(hard_retries){
      delay(interval ? interval : 5000);
      res = df.PollAuthorization(device_code);

      if(!res){
        Debug(F("Auth. Poll failed"));
        hard_retries--;
        if(hard_retries==0){
          return AUTH_TOKEN_FAILED;
        }
      } else {
        //A 200 means "success" and authentication is complete.
        if(200 == res->statusCode){
          //User authentication completed. Extract access_token
          auto * authJSON = ParseJSON(res->data);
          if(!authJSON){
            return AUTH_TOKEN_FAILED;
          }
  
          //Extract access_token
          this->accessToken = new String((const char *)(*authJSON)["access_token"]);
          delete authJSON;
          return AUTH_OK;
        }
        
        //Anything else from 200 or 403 is a failure. Return with error.
        if(res->statusCode != 403){
          //Anything other than a 403 is a failure
          Debug(String(F("Auth. Failed: ")) + String(res->statusCode));
          return AUTH_TOKEN_FAILED;
        }
  
        /* 403 means many things. response.data.error provides more info:
         *  authorization_pending: continue polling.
         *  slow_down: polling is happening too fast.
         *  access_denied: user cancelled.
         *  expired_token: the flow is expired. Try again the whole thing.
         *  invalid_grant: code is invalid
         */
         auto * authJSON = ParseJSON(res->data);
         const char * error = (*authJSON)["error"];
         delete authJSON;
         
         if(isAuthorizationPending(error)){
          //Just wait
          Debug(F("Authenticate. Authorization Pending"));
         } else {
          if(isSlowDown(error)){
            Debug(F("Authenticate. Polling too fast"));
            interval += 2000; //Add 2 seconds to polling
          } else {
            //Any other error is final
            Debug("Authenticate. Error:" + String(error));
            return AUTH_TOKEN_FAILED;        
          }
        }
      }
    }   
    return AUTH_TOKEN_FAILED;
  }
};

#endif
