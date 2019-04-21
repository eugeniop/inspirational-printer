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
    HTTPRequest request;
    
    void CloseConnection(){
      request.DisconnectServer();
    };

    int OpenConnection(){
      //request = new HTTPRequest();
      return request.ConnectWiFi(WIFI_SSID, WIFI_PWD, 3);
    };

    void Debug(String s){
      Serial.println(s);
    }

  public:
    DeviceFlow(DeviceFlowOptions& options){
       this->options =  &options;
    }

    ~DeviceFlow(){
      if(request.GetStatus() == WL_CONNECTED){
        request.DisconnectAll();
      }
    };

    Response * StartAuthorization(){
      if( OpenConnection() == WL_CONNECTED ){
        //Kicks-off Device flow auth
        String codeRequest = "{\"client_id\":\"" + options->clientId + "\",\"scope\":\"" + options->scope +"\",\"audience\":\"" + options->audience + "\"}";
        Debug("StartAuthorization. POSTING: " + codeRequest);
        auto * response = request.PostJSON(options->authServer, options->authorizationPath, 443, "", codeRequest);
        CloseConnection();
        if(response->statusCode != 0){
          Debug("StartAuthorization. Request completed");
          response->Debug();
        }
        return response;
      }
      Debug("StartAuthorization. Connection failed");
      return NULL;
    };

    Response * PollAuthorization(String code){
        if(OpenConnection() == WL_CONNECTED){
          String tokenRequest = "{\"grant_type\":\"urn:ietf:params:oauth:grant-type:device_code\",\"client_id\":\"" + options->clientId + "\",\"device_code\":\"" + code + "\"}";
          Debug("PollAuthorization. POSTING: " + tokenRequest);
          auto * response = request.PostJSON(options->authServer, options->tokenPath, 443, "", tokenRequest);
          if(response->statusCode != 0){
            Debug("PollAuthorization. Request completed>");
            response->Debug();
            return response;
          }
        }
        Debug("PollAuthorization. Connection failed");
        return NULL;
    };
//
//    String * userInfo(String access_token){
//        Request * r = new Request(WIFI_SSID, WIFI_PWD);
//        String * response = r->getJSON(options->authServer, options->userInfoPath, 443, access_token);
//        r->disconnect();
//        dele
//      return NULL;
//    };
};

#endif
