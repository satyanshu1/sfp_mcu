const String EXP_CMD = "send_data"; 
enum DEV_STATES {WAIT_FOR_LORA_CMD, GET_PH, RSP_LORA};
const uint8_t SCOUNT = 101; // sum of sample point
const uint32_t TX_DELAY = 1000;
double ADC_BUFF[SCOUNT]; //to store the sample adc readings
char CHIP_ID[9];
String RXPACKET, TXPACKET;
double CUR_MEDIAN_ADC_VOLTAGE;
DEV_STATES dev_state = WAIT_FOR_LORA_CMD;
uint32_t tx_timer = 0;


void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    sprintf(CHIP_ID, "%x", ESP.getEfuseMac());    // get chip id & store for future
    setup_lora();
    setup_ph_sensor();
}

void loop() {
    // put your main code here, to run repeatedly:
    // PH test code
    // Serial.printf("chip_id: %s, pH: %f\n", CHIP_ID, get_ph());
    // delay(5000);
    switch(dev_state)
    {
        case WAIT_FOR_LORA_CMD:
            Serial.printf("waiting to receive command over lora\n");
            if(lora_receive(5*60*1000))  // try to receive within 5 mins
            {
                Serial.printf("Recived Cmd:%s\n", RXPACKET.c_str());
                // TODO: add command validation
                dev_state = GET_PH;
            } 
            else 
            {
                Serial.printf("Did not receive command in 5 mins\n");
            }
            break;
        case GET_PH:
            {
                double cur_ph = get_ph();
                if(cur_ph > -100)  // flag to ensure pH computation is complete
                {
                    dev_state = RSP_LORA;
                    char buff[200];
                    sprintf(buff, "CHIP_ID:%s, pH:%lf, ADC Volt-median:%lf, min:%lf, max:%lf", CHIP_ID, cur_ph, CUR_MEDIAN_ADC_VOLTAGE, ADC_BUFF[SCOUNT-1], ADC_BUFF[0]);
                    TXPACKET = String(buff);
                    tx_timer = millis();
                }
            }
            break;
        case RSP_LORA:
            if(millis() - tx_timer >= TX_DELAY)
            {
                dev_state = WAIT_FOR_LORA_CMD;
                
                Serial.printf("Sending data %s over lora\n", TXPACKET.c_str());
                if(lora_send(TXPACKET, 1000))
                {
                    Serial.printf("send success\n");
                } 
                else 
                {
                    Serial.printf("send failed\n");
                }
            }
            break;
    }
}