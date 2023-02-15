const uint8_t PORTPIN = 13;  // 7; //13 for old board, 7 for new
const uint8_t PH[] = {4, 10};  // pH values of calibration solutions used
const double PH_ADC[] = {2.08, 1.84};  // ADC reading for corresponding pH values mentioned above
const uint32_t SAMPLE_INTERVAL = 10; // sample reading interval in ms
const double slope = (PH[1]-PH[0])/(PH_ADC[1]- PH_ADC[0]);
const double intercept = PH[0] - (slope*PH_ADC[0]);
enum PH_STATES {READ_ADC, CALC_PH};
uint32_t ph_timer = millis();
PH_STATES ph_state = READ_ADC;


void setup_ph_sensor()
{
    // initialize gpio for reading voltage
    adcAttachPin(PORTPIN);
    analogSetClockDiv(255);  // 1338mS    
}

double get_ph()
{
    double ph = -101;
    static uint8_t count = 0;

    switch(ph_state)
    {
        case READ_ADC:
            // read the voltage and store into the buffer once every SAMPLE_INTERVAL
            if((millis() - ph_timer) > SAMPLE_INTERVAL)
            {
                ph_timer = millis();
                uint16_t reading = analogRead(PORTPIN);
                double adc_val = -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
                insert_value(ADC_BUFF, adc_val, count, SCOUNT);
                ++count;

                Serial.printf("%lf(%d) ", adc_val, reading);

                if(count >= SCOUNT)
                {
                    count = 0;
                    ph_state = CALC_PH;
                    
                    Serial.printf("\nSorted values:");
                    for(int i=0; i<SCOUNT; ++i)
                    {
                        Serial.printf("%lf ", ADC_BUFF[i]);
                    }
                }
            }
            break;
        case CALC_PH:
            ph_state = READ_ADC;
            
            CUR_MEDIAN_ADC_VOLTAGE = get_median(ADC_BUFF, SCOUNT); // read the stable value by the median filtering algorithm
            ph = CUR_MEDIAN_ADC_VOLTAGE * slope + intercept;
            Serial.printf("\nmedian_adc_voltage:%lf, slope:%lf, intercept:%lf, pH:%f", CUR_MEDIAN_ADC_VOLTAGE, slope, intercept, ph);
            break;
    }
    return ph;
}

void insert_value(double arr[], double val, uint8_t cur_size, uint8_t max_size)
{
    if (cur_size < max_size)
    {
        if(0==cur_size)
        {
            arr[0] = val;
        } 
        else 
        {
            int insert_idx = 0;
            // find index where val is greater than equal to value at the index of array but less than value at the next index
            for(;(insert_idx < cur_size) && (val < arr[insert_idx]); ++insert_idx);
            for(int i=cur_size-1; i >= insert_idx; i--)
                arr[i+1] = arr[i];
            arr[insert_idx] = val;
        }
    }
}

double get_median(double arr[], uint8_t size){
    if ((size & 1) > 0)
        return arr[(size - 1) / 2];
    else
        return (arr[size / 2] + arr[size / 2 - 1]) / 2.0;
}
