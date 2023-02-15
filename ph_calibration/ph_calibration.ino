const uint8_t SCOUNT = 101; // total sample adc readings
const uint16_t SAMPLE_INTERVAL = 40; // sample reading interval in ms
const uint8_t PORTPIN = 12;    // 7; //13 or 12 for old board, 7 for new
double adc_buff[SCOUNT]; //to store the sample adc readings
String RXPACKET, TXPACKET;
double cur_median_voltage, min_median_voltage, max_median_voltage;
enum STATES {READ_ADC, CALCULATE, LORA_TRANSMIT, IDLE};
STATES state;
uint32_t recorded_time;


void setup() {
    // put your setup code here, to run once:    
    Serial.begin(115200);

    adcAttachPin(PORTPIN);
    analogSetClockDiv(255);    // 1338mS

    setup_lora();
    
    min_median_voltage = 10;
    state = READ_ADC;
    recorded_time = millis();
}

void loop() {
    // put your main code here, to run repeatedly:
    static uint8_t count = 0;

    switch(state)
    {
        case READ_ADC:
            // read the voltage and store into the buffer once every SAMPLE_INTERVAL
            if((millis() - recorded_time) > SAMPLE_INTERVAL)
            {
                recorded_time = millis();
                uint16_t reading = analogRead(PORTPIN);
                double adc_val = -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
                insert_value(adc_buff, adc_val, count, SCOUNT);
                ++count;

                Serial.printf("%lf(%d) ", adc_val, reading);
                TXPACKET += String(adc_val) + "(" + String(reading) + ") ";

                if(count >= SCOUNT)
                {
                    count = 0;
                    state = CALCULATE;
                    
                    Serial.printf("\nSorted values:");
                    TXPACKET += "\nSorted values:";
                    for(int i=0; i<SCOUNT; ++i)
                    {
                        Serial.printf("%lf ", adc_buff[i]);
                        TXPACKET += String(adc_buff[i]) + " ";
                    }
                }
            }
            break;
        case CALCULATE:
            state = LORA_TRANSMIT;

            cur_median_voltage = get_median(adc_buff, SCOUNT); // read the stable value by the median filtering algorithm
            if(cur_median_voltage < min_median_voltage)
                min_median_voltage = cur_median_voltage;
            if(cur_median_voltage > max_median_voltage)
                max_median_voltage = cur_median_voltage;
            
            Serial.printf("\nCURRENT ADC READINGS- min:%lf, max:%lf, median:%lf", adc_buff[SCOUNT-1], adc_buff[0], cur_median_voltage);
            TXPACKET += "\nCURRENT ADC READINGS- min:" + String(adc_buff[SCOUNT-1]) +", max:"+ String(adc_buff[0]) + ", median:"+ String(cur_median_voltage);

            Serial.printf("\nOVERALL MEDIAN ADC READINGS- min:%lf, max:%lf, current:%lf", min_median_voltage, max_median_voltage, cur_median_voltage);
            TXPACKET += "\nOVERALL MEDIAN ADC READINGS- min:" + String(min_median_voltage) +", max:"+ String(max_median_voltage) + ", current:"+ String(cur_median_voltage);
            break;
        case LORA_TRANSMIT:
            state = IDLE;

            lora_send(TXPACKET, 5000);
            recorded_time = millis();
            break;
        case IDLE:
            if((millis() - recorded_time) > 3000) // 3 sec delay
            {
                state = READ_ADC;
                TXPACKET.clear();
            }
            break;

    }
    // String lora_data = String("ADC readings: ");
    // for(int i=0; i < SCOUNT; ++i)
    // {
        // double reading = analogRead(PORTPIN);
        // double adc_val = -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
        // Serial.printf("%d ", adc_val);
        // lora_data += " " + String(adc_val);
        // insert_value(adc_buff, adc_val, i, SCOUNT);
        // delay(SAMPLE_INTERVAL);
    // }
    // lora_send(lora_data, 5000);
    // lora_data.clear();
    // delay(2000);

    // Serial.printf("\nSorted values in array: ");
    // lora_data += "Sorted values in array: ";
    // for(int i=0; i < SCOUNT; i++)
    // {
    //     Serial.printf("%d ", adc_buff[i]);
    //     lora_data += " " + String(adc_buff[i]);
    // }
    // lora_send(lora_data, 5000);    
    // lora_data.clear();
    // delay(2000);

    // double median_voltage = get_median(adc_buff, SCOUNT); // read the stable value by the median filtering algorithm
    // Serial.printf("\nMedian adc reading: %d, voltage: %f\n", median_voltage, median_voltage* (3.3 / 4095.0));
    
    // lora_data += "Median adc reading: "+ String(median_voltage) + ", voltage: " + String(median_voltage* (3.3 / 4095.0));
    // lora_send(lora_data, 5000);

    // test_sorting();
        // cur = median_voltage* (3.3 / 4095.0);
        

    // delay(1000);    // 5 sec delay
}
