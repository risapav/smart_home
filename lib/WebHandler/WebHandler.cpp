#include <App.hpp>
#include <DefaultAppConfig.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <WifiHandler.hpp>


#include "WebHandler.hpp"
//#include "WebHandlerPrivate.hpp"

WebHandler webHandler;

static AsyncWebServer server(80);

const int capacity=JSON_OBJECT_SIZE(12);
StaticJsonBuffer<256> jsonBuffer;


char json[] = "{"
  "\"time\":\"1351824120\","
  "\"c_garaz\":\"1\","
  "\"c_svetlo\":\"0\","
  "\"c_den_noc\":\"1\","
  "\"c_rezerva\":\"0\","
  "\"f_garaz\":\"1\","
  "\"f_svetlo\":\"0\","
  "\"f_den_noc\":\"1\","
  "\"f_rezerva\":\"0\","
  "\"f_temp1\":\"1.0\","
  "\"f_temp2\":\"2.0\""
  "}";

JsonObject& object = jsonBuffer.parseObject(json);
 // Parse the root object
 //JsonObject &object = jsonBuffer.createObject();

//object.set<long>("time", 1351824120);
/*
object.set(<char *>"time",<long> 1351824120);
object.set<bool>("c_garaz", true);
object.set<bool>("c_svetlo", true);
object.set<bool>("c_den_noc", true);
object.set<bool>("c_rezerva", true);
object.set<bool>("c_rezerva", true);
object.set<bool>("f_garaz", true);
object.set<bool>("f_svetlo", true);
object.set<bool>("f_den_noc", true);
object.set<bool>("f_rezerva", true);
object.set<float>("f_temp1", 2.0);
object.set<float>("f_temp2", 1.0);
*/

static String doCommand() {
  String message;
  // Declare a buffer to hold the result
  // Produce a minified JSON document
  object.printTo(message);
  object.printTo(Serial);

  return message;
}

static String doFeedback() {
  String message;
  // Declare a buffer to hold the result
  // Produce a minified JSON document
  object.printTo(message);
  object.printTo(Serial);

  return message;
}

WebHandler::WebHandler()
{
  initialized = false;
}

void WebHandler::setup()
{
  LOG0("HTTP server setup...\n");

  if (!SPIFFS.begin()) {
    Serial.println("*******************cannot mount SPIFFS************");
    return;
  } else {
    Serial.println("*******************SPIFFS mounted************");
    Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        Serial.println("*****");
        Serial.print(dir.fileName());
        File f = dir.openFile("r");
        Serial.println(f.size());
      }
  }

  //Initialize Webserver
  // Serve files in directory "/www/" when request url starts with "/"
  // Request to the root or none existing files will try to server the defualt
  // file name "index.htm" if exists
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

  server.on("/cmd", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", doCommand() );
    response->addHeader( "Access-Control-Allow-Origin", "*" );
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
  });
  server.on("/fbk", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", doFeedback() );
    response->addHeader( "Access-Control-Allow-Origin", "*" );
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
  });

  server.on("/json", HTTP_ANY, [](AsyncWebServerRequest * request) {

    AsyncJsonResponse * response = new AsyncJsonResponse();
    JsonObject& root = response->getRoot();
    root["key1"] = "key number one";
    JsonObject& nested = root.createNestedObject("nested");
    nested["key1"] = "key number one";

    response->setLength();
    request->send(response);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  //setupRestApi(server);
  server.on("/rand", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(200, "text/plain", String(random(1000)));
   });
  server.begin();

  LOG0("HTTP server started\n");
  initialized = true;
}

void WebHandler::handle()
{
  if( !initialized )
  {
    setup();
  }
}
