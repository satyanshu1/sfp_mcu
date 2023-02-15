char CHIP_ID[9];
const char* SENSOR_TYPE = "pH"; 
const String LORA_CMD = "send_data";
String RXPACKET;
char CUR_TIME [50];
int16_t RSSI;
int8_t SNR;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sprintf(CHIP_ID, "%x", ESP.getEfuseMac());    // get chip id & store for future
  setup_time();
  setup_wifi();
  setup_lora();
  setup_display();
}


void loop() {
  // put your main code here, to run repeatedly:
  // Serial.printf("chip_id: %s, pH: %f\n", CHIP_ID);

  bool is_success = false;  // flag to configure retry time. Retry after configured delay if success else retry immediately. 
  write_to_display(String("sending LORA cmd:send_data: to get data from other Lora.."));
  Serial.printf("sending LORA cmd:send_data: to get data from other Lora..\n");
  delay(2000);  // delay to retain display message
  if (lora_send(LORA_CMD, 1000))
  {
    write_to_display(String("LORA send success. Waiting for response from other device.."));
    Serial.printf("LORA send success. Waiting for response from other device..\n");
    if(lora_receive(5000)){  // try to receive within 5 sec
      delay(2000);  // to retain previous message
      write_to_display_buffered(String("LORA Received:") + RXPACKET + String(" RSSI:") + String(RSSI) + String(" SNR:") + String(SNR));
      Serial.printf("LORA Received:%s\n", RXPACKET.c_str());
      update_time();
      send_data_over_wifi();
      delay(2000);  // to retain previous message
      is_success = true;
    }
    else{
      write_to_display(String("LORA receive failed. Retrying.."));
      Serial.printf("LORA receive failed.\n");
      delay(2000);
    }
  }
  else{
      write_to_display("Lora send failed. Retrying..");
      Serial.printf("Lora send failed. Retrying..\n");
      delay(2000);
  }
  if(is_success)
    write_to_display("Next update after 5 mins..");
    delay(1*2*1000);  // delay in ms
}
