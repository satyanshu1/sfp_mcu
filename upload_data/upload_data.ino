char CHIP_ID[9];
const char* SENSOR_TYPE = "pH"; 
const String LORA_CMD = "send_data";
const uint8_t SCOUNT = 11;  // total sample point
const uint32_t idle_time = 1000;  // idle time in ms
const uint32_t lora_receive_timeout = 5000;  // time in ms till which LORA will try to receive
enum DEV_STATES {SEND_LORA_CMD, GET_LORA_RSP, CALC_PH, SEND_TO_SERVER, IDLE};
DEV_STATES dev_state = SEND_LORA_CMD;
uint32_t state_timer = millis();
double PH_BUFF[SCOUNT]; //to store the pH values
String RXPACKET;
char CUR_TIME [50];
int16_t RSSI;
int8_t SNR;


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);    
    sprintf(CHIP_ID, "%x", ESP.getEfuseMac());        // get chip id & store for future
    setup_display();
    setup_wifi();
    setup_lora();
}


void loop() {
    // put your main code here, to run repeatedly:
    // Serial.printf("chip_id: %s, pH: %f\n", CHIP_ID);
    
    static uint8_t count = 0;
    
    switch(dev_state)
    {
        case SEND_LORA_CMD:
            Serial.printf("sending LORA cmd:send_data: to get data from other Lora..\n");
            write_to_display(String("sending LORA cmd:send_data: to get data from other Lora.."));
            if (lora_send(LORA_CMD, 1000))
            {
                dev_state = GET_LORA_RSP;
                
                Serial.printf("LORA send success. Waiting for response from other device..\n");
            }
            else
            {
                Serial.printf("Lora send failed. Retrying..\n");
                write_to_display("Lora send failed. Retrying..");
            }
            break;
        case GET_LORA_RSP:
            if(lora_receive(lora_receive_timeout))
            {
                dev_state = CALC_PH;
                
                Serial.printf("LORA Received:%s\n", RXPACKET.c_str());
                write_to_display_buffered(String("LORA Received:") + RXPACKET + String(" RSSI:") + String(RSSI) + String(" SNR:") + String(SNR));
            }
            else
            {
                dev_state = SEND_LORA_CMD;
                
                Serial.printf("Lora receive failed.\n");
                write_to_display("Lora receive failed.");
            }
            break;
        case CALC_PH:
            {
              uint8_t start_idx = RXPACKET.indexOf("pH:") + 3;
              uint8_t end_idx = RXPACKET.indexOf(",", start_idx) + start_idx;
              double ph = RXPACKET.substring(start_idx, end_idx).toDouble();
              Serial.printf("Sample Count:%d, PH: %lf\n", count, ph);
              insert_value(PH_BUFF, ph, count, SCOUNT);
              ++count;
              if(count >= SCOUNT)
              {
                  dev_state = SEND_TO_SERVER;

                  count = 0;
              }
              else
              {
                  dev_state = SEND_LORA_CMD;
              }
            }
            break;
        case SEND_TO_SERVER:
            dev_state = IDLE;
            {
                double ph = get_median(PH_BUFF, SCOUNT);
                String ph_str = String(ph);
                Serial.printf("PH-min:%lf, max:%lf, median: %lf\n", PH_BUFF[SCOUNT-1], PH_BUFF[0], ph);
                write_to_display_buffered("PH-min:" + String(PH_BUFF[SCOUNT-1]) + " max:" + String(PH_BUFF[0]) + " median:" + ph_str);
                if ((ph >= 0) && (ph <= 14))
                {
                    uint8_t start_idx = RXPACKET.indexOf(":");
                    uint8_t end_idx = RXPACKET.indexOf(",");
                    String serial = RXPACKET.substring(start_idx, end_idx);

                    send_data_over_wifi(serial, ph_str);
                }
                else
                    write_to_display("Invalid PH:" + ph_str + ". Check setup. Not updating to server.");
            }
            write_to_display("Next update after " + String(idle_time/60000) + " mins..");
            break;
        case IDLE:
            if((millis() - state_timer) > idle_time)
            {                
                state_timer = millis();
                dev_state = SEND_LORA_CMD;
            }
            break;
    }
}
