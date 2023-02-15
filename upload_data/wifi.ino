#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

const String SSID = "";
const String PASSWORD = "";
const String WIFI_HOSTNAME = "LORA_WAN_MASTER";
const String REST_API = "https://sahoo6924.pythonanywhere.com/rest/";


void setup_wifi(){
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(WIFI_HOSTNAME.c_str());  // set wifi host name
  connect_to_wifi();  // connect to WiFi
}

int connect_to_wifi(){
  WiFi.mode(WIFI_STA);  // set to station mode
  WiFi.disconnect();

  // scan networks
  int n = WiFi.scanNetworks();
  Serial.printf("wifi scan done & %d networks found\n", n);
  for (int i = 0; i < n; ++i) {
    //if network found, try to connect
    if (WiFi.SSID(i).compareTo(SSID) == 0)
    {
      Serial.printf("WiFi %s found with RSSI: %d\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      WiFi.begin(SSID.c_str(), PASSWORD.c_str());  // connect to wifi
      Serial.print("Connecting to WiFi.");
      while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);
      }
      Serial.printf("\nConnected to WiFi newtork: %s\n", SSID.c_str());
      return 0;
    }
  }
  Serial.printf("WiFi %s not found in scanned networks\n", SSID.c_str());
  write_to_display("WiFi: " + SSID + " not found in scanned networks");
  return 1;
}

void send_data_over_wifi(String &sensor_ser, String &sensor_val){
    if(WiFi.status() == WL_CONNECTED)
    {
        update_time();
        
        // rest api communication
        HTTPClient client;
        int http_code;
        client.begin(REST_API);

        // get rest api
        // http_code = client.GET();
        // Serial.printf("GET status: %d, payload: %s \n", http_code, client.getString().c_str());

        //put rest api
        client.addHeader("Content-Type", "application/json");
        StaticJsonDocument<300> doc;
        doc["sensor_ser"] = sensor_ser;
        doc["sensor_type"] = SENSOR_TYPE;
        doc["time_stamp"] = CUR_TIME;  // "2023-01-28T06:29:28.337973Z";
        doc["sensor_value"] = sensor_val;
        String json_out;
        serializeJson(doc, json_out);
        http_code = client.PUT(json_out);
        String rest_api_pl = client.getString();
        String msg = String("Server Upload Status:") + String(http_code) + String(", Payload:") + rest_api_pl;
        write_to_display(msg);
        Serial.printf("%s \n", msg.c_str());    
    } 
    else
    {
        write_to_display("Can't send to server as network: " + SSID + " not available");
        connect_to_wifi();
    }
}