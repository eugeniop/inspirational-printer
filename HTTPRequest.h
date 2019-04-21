#ifndef REQUEST_H
#define REQUEST_H

/*
 * A simple HTTPS helper class
 * Normal use:
 * HTTPRequest * r = new HTTPRequest(SSID,PWD);
 * r->ConnectWiFi(SSID, pwd, retries);
 * if(r->GetStatus() == WL_CONNECTED){
 *  String * response = r->GetJson(...);
 *  doSomething(response);
 * }
 * if(r->GetStatus() == WL_CONNECTED){
 *  String * response = r->PostJson(...);
 *  doSomething(response);
 * }
 * r->DisconnectAll();
 * delete r;
 * 
 * Notice that client of thsi class is respinsible for freeing up the Response object
 * 
 */

#include "globals.h"
#include <SPI.h>
#include <WiFi101.h>

enum CONNECTION_STATUS { CONNECTION_OK, NO_WIFI_CONNECTION, CONNECTION_FAILED };

class Response {

  public:
    int statusCode;
    int length;
    String * data;
//    char data[MAX_HTTP_RESPONSE];
  
    ~Response(){
      if(data){
        delete data;
      }
    };

    void Debug(){
      Serial.println("Status: " + String(statusCode));
      if(length > 0){
        Serial.println(String(F("Content-Length: ")) + String(length));
        Serial.println(F("Data->"));
        Serial.println(*data);
        Serial.println("<-");
      } else {
        Serial.println(F("No content"));
      }
    };
};


class HTTPRequest {
  private:
    WiFiSSLClient client;

    Response response;

    typedef enum { HEADERS, BODY } HTTP_RX_STATE;

    void ParseHeader(String line){
      if(line.startsWith(F("HTTP/1.1"))){
        int i = line.indexOf(' ');
        int j = line.indexOf(' ', i+1);
        auto statusCode = line.substring(i+1, j);
        response.statusCode = atoi(statusCode.c_str());
        return;
      }
      
      if(line.startsWith(F("Content-Length"))){
        int i = line.indexOf(' ');
        auto len = line.substring(i+1);
        response.length = atoi(len.c_str());
        return;
       }
    }

    void ResetResponse(){
      response.statusCode = 0;
      response.length = 0;
      if(response.data){
        delete response.data;
        response.data = NULL;
      }
      Debug("HTTP. Response Reset");
    }

    Response * processResponse(){
      //Small delay to allow the library to catchup (it seems)
      delay(2000);
      HTTP_RX_STATE state = HEADERS;
      while (client.available()) {
        String line;
        switch(state){
          case HEADERS:
            line  = client.readStringUntil('\n');
            ParseHeader(line);
            if(line.length() == 1){ //Headers - Body separator
              state = BODY;
            }
            break;
          case BODY:
            //client.readString().toCharArray(data, MAX_HTTP_RESPONSE);
            response.data = new String(client.readString());
            break;
        }
      }
      return &response;
    }

    CONNECTION_STATUS ConnectServer(String server, int port){
      if(WiFi.status() != WL_CONNECTED){ 
        Debug("HTTPRequest.Connect. No WiFi");
        return NO_WIFI_CONNECTION; 
      }
      if(!client.connect(server.c_str(), port)){
        Debug("HTTPRequest.Connect. Connection to server failed." + server);
        return CONNECTION_FAILED;
      }
      return CONNECTION_OK;
    }

    void Debug(String s){
      Serial.println(s);  
    }
    
  public:
    HTTPRequest(){
      WiFi.setPins(8,7,4,2);      //Set for this board
      ResetResponse();
    }

    int ConnectWiFi(String ssid, String password, int retries){
      int status = WiFi.status();
      if (status == WL_NO_SHIELD) {
        Debug("WiFi shield not present");
        return status;
      }
      
      while(status != WL_CONNECTED){
        if(password.length() > 0){
          Debug("Connecting to WiFi with SSID and Password");
          status = WiFi.begin(ssid, password);
        } else {
          status = WiFi.begin(WIFI_SSID);  //Passwordless WiFi
        }
        if(--retries == 0){
          break;
        }
        // wait 3 seconds for connection:
        delay(5000);
      }
      Debug("HTTP. ConnectWifi" + String(status));
      return status;
    }

    int GetStatus(){
      return WiFi.status();
    }

    void DisconnectServer(){
      client.stop();
    }

    void DisconnectAll(){
      DisconnectServer();
      WiFi.disconnect();
    }

    void PrintWiFiStatus() {
      // print the SSID of the network you're attached to:
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
    
      // print your WiFi shield's IP address:
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);
    
      // print the received signal strength:
      long rssi = WiFi.RSSI();
      Serial.print("signal strength (RSSI):");
      Serial.print(rssi);
      Serial.println(" dBm");
    }

    //POSTs a form to server
    Response * PostForm(String server, String route, int port, String access_token, String data){
      if(ConnectServer(server, port) != CONNECTION_OK){
        Debug("PostForm. Connection to WiFi and Server failed");
        return NULL;
      }

      client.println("POST " + route + " HTTP/1.1");
      client.println("Host: " + server);
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println("Content-Type: application/x-www-form-urlencoded"); 
      client.println("Content-Length: " + String(data.length()));   
      client.println("Connection: close");
      client.println();
      client.print(data);
      return processResponse();
    }

    Response * GetJSON(String server, String route, int port, String access_token){
      ResetResponse();
      if(ConnectServer(server, port) != CONNECTION_OK){
        Debug("GetJSON. Connection to WiFi and Server failed");
        return NULL;
      }
      Serial.println("Connected");
      client.println("GET " + route + " HTTP/1.1");
      client.println("Host: " + server);
      client.println("Connection: close");
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println();
      Debug("HTTP. GetJSON. Request sent");
      return processResponse();
    }

    Response * PostJSON(String server, String route, int port, String access_token, String data){
      if(ConnectServer(server, port) != CONNECTION_OK){
        Debug("PostJSON. Connection to WiFi and Server failed");
        return NULL;
      }

      client.println("POST " + route + " HTTP/1.1");
      client.println("Host: " + server);
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println("Content-Type: application/json"); 
      client.println("Content-Length: " + String(data.length()));
      client.println("Connection: close");
      client.println();
      client.print(data);
      return processResponse();
    }
};

#endif
