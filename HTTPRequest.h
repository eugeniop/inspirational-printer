#ifndef REQUEST_H
#define REQUEST_H

#include <SPI.h>
#include <WiFi101.h>
#include "globals.h"
#include "HTTPResponse.h"

enum CONNECTION_STATUS { CONNECTION_OK, NO_WIFI_CONNECTION, CONNECTION_FAILED };

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

    Response * processResponse(){
      //Resets the response object
      response.Reset();
      //Small delay to allow the library to catchup (it seems)
      delay(2000);
      HTTP_RX_STATE state = HEADERS;
      //Debug(String(client.status()));
      while (client.available()) {
        String line;
        switch(state){
          case HEADERS:
            line  = client.readStringUntil('\n');
            //Serial.println(line);
            ParseHeader(line);
            if(line.length() == 1){ //Headers - Body separator
              state = BODY;
            }
            break;
          case BODY:
            response.data = new String(client.readString());
            break;
        }
      }
      return &response;
    }

    CONNECTION_STATUS ConnectServer(String server, int port){
      if(WiFi.status() != WL_CONNECTED){ 
        Debug(F("HTTP.Connect. No WiFi"));
        return NO_WIFI_CONNECTION; 
      }
      if(!client.connect(server.c_str(), port)){
        Debug(String(F("HTTP. Connection to server failed: ")) + server);
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
      response.Reset();
    }

    int ConnectWiFi(String ssid, String password, int retries){
      int status = WiFi.status();
      if (status == WL_NO_SHIELD) {
        Debug(F("HTTP. No WiFi shield"));
        return status;
      }
      
      while(status != WL_CONNECTED){
        if(password.length() > 0){
          status = WiFi.begin(ssid, password);
        } else {
          status = WiFi.begin(ssid);  //Passwordless WiFi
        }
        if(--retries == 0){
          break;
        }
        // wait 3 seconds for connection:
        delay(5000);
      }
      Debug(String(F("HTTP. WiFi: ")) + String(status));
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
      Serial.print(F("SSID: "));
      Serial.println(WiFi.SSID());
    
      // print your WiFi shield's IP address:
      IPAddress ip = WiFi.localIP();
      Serial.print(F("IP Address: "));
      Serial.println(ip);
    
      // print the received signal strength:
      long rssi = WiFi.RSSI();
      Serial.print(F("RSSI: "));
      Serial.print(rssi);
      Serial.println(" dBm");
    }

    //POSTs a form to server
    Response * PostForm(String server, String route, int port, String access_token, String data){
      if(ConnectServer(server, port) != CONNECTION_OK){
        return NULL;
      }

      client.println("POST " + route + " HTTP/1.1");
      client.println("Host: " + server);
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println(F("Content-Type: application/x-www-form-urlencoded")); 
      client.println("Content-Length: " + String(data.length()));   
      client.println(F("Connection: close"));
      client.println();
      client.print(data);
      return processResponse();
    }

    Response * GetJSON(String server, String route, int port, String access_token){
      if(ConnectServer(server, port) != CONNECTION_OK){
        return NULL;
      }
      client.println("GET " + route + " HTTP/1.1");
      client.println("Host: " + server);
      client.println(F("Connection: close"));
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println();
      Debug("HTTP. GetJSON. Request sent");
      return processResponse();
    }

    Response * PostJSON(String server, String route, int port, String access_token, String data){
      if(ConnectServer(server, port) != CONNECTION_OK){
        return NULL;
      }

      client.println("POST " + route + " HTTP/1.1");
      client.println("Host: " + server);
      if(access_token.length() > 0){
        client.println("Authorization: Bearer " + access_token);
      }
      client.println(F("Content-Type: application/json")); 
      client.println("Content-Length: " + String(data.length()));
      client.println(F("Connection: close"));
      client.println();
      client.print(data);
      return processResponse();
    }
};

#endif
