#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

class Response {

  public:
    int statusCode;
    int length;
    String * data;
    
    ~Response(){
      Reset();
    };

    void Reset(){
      statusCode = 0;
      length = 0;
      if(data){
        delete data;
        data = NULL;
      }
    }

    void Debug(){
      Serial.println(String(F("Status: ")) + String(statusCode));
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

#endif
