#ifndef DEVICEFLOW_H
#define DEVICEFLOW_H

#include "globals.h"
#include "HTTPRequest.h"

class DeviceFlowOptions {
  public:
    String authServer;
    String authorizationPath;
    String clientId;
    String scope;
    String tokenPath;
    String audience;
};

class DeviceFlow {
  private:
    DeviceFlowOptions * options;
    HTTPRequest * request;
    
    void CloseConnection(){
      request->DisconnectServer();
    };

    int OpenConnection(){
      return request->ConnectWiFi(WIFI_SSID, WIFI_PWD, 3);
    };

    void Debug(String s){
      Serial.println(s);
    }

  public:
    DeviceFlow(DeviceFlowOptions * options, HTTPRequest * request){
       this->options =  options;
       this->request = request;
    }

    ~DeviceFlow(){
      if(request->GetStatus() == WL_CONNECTED){
        request->DisconnectAll();
      }
    };

    Response * StartAuthorization(){
      if( OpenConnection() == WL_CONNECTED ){
        //Kicks-off Device flow auth
        //String codeRequest = "{\"client_id\":\"" + options->clientId + "\",\"scope\":\"" + options->scope +"\",\"audience\":\"" + options->audience + "\"}";
        String codeRequest = "client_id=" + options->clientId + "&scope=" + options->scope +"&audience=" + options->audience;
        //Debug("StartAuthorization. POSTING: " + codeRequest);
        auto * response = request->PostForm(options->authServer, options->authorizationPath, 443, "", codeRequest);
        CloseConnection();
        
        if(response == NULL){
          Debug(F("DF.Start Post failed"));
          return NULL;
        }
        
        if(response->statusCode != 0){
          //Debug(F("StartAuthZ. Request completed"));
          response->Debug();
        }
        
        return response;
      }
      Debug(F("DF.Start Connection fail"));
      return NULL;
    };

    Response * PollAuthorization(String code){
        if(OpenConnection() == WL_CONNECTED){
          String tokenRequest = "{\"grant_type\":\"urn:ietf:params:oauth:grant-type:device_code\",\"client_id\":\"" + options->clientId + "\",\"device_code\":\"" + code + "\"}";
          Debug(F("DF. Poll. POSTING:"));
          Debug(tokenRequest);
          auto * response = request->PostJSON(options->authServer, options->tokenPath, 443, "", tokenRequest);
                  
          if(response == NULL){
            Debug(F("DF.Poll Post failed"));
            return NULL;
          }
        
          if(response->statusCode != 0){
            response->Debug();
            return response;
          }
        }
        Debug(F("DF.Poll Connection fail"));
        return NULL;
    };
};

#endif
