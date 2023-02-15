#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
static RadioEvents_t RadioEvents;
enum LORA_STATES {L_IDLE, L_BUSY, L_TX_DONE, L_TX_TIMEOUT, L_RX_DONE, L_RX_TIMEOUT, L_RX_ERROR};
const char* LORA_STATE_NAMES[] = {"L_IDLE", "L_BUSY", "L_TX_DONE", "L_TX_TIMEOUT", "L_RX_DONE", "L_RX_TIMEOUT", "L_RX_ERROR"};
volatile LORA_STATES LORA_STATE;


void setup_lora() {
  Mcu.begin();
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODINGRATE, LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}

bool lora_receive(uint32_t timeout) {
  Serial.printf("\nLORA waiting to receive data..\n");
  LORA_STATE = L_BUSY;
  uint32_t start_time = millis();
  Radio.Rx(0);
  while((L_BUSY == LORA_STATE) && (!is_timed_out(start_time, timeout))){
    Radio.IrqProcess();
  }
  if(L_RX_DONE != LORA_STATE)
  {
      Serial.printf("\nLORA Receive failed. State:%s\n", LORA_STATE_NAMES[LORA_STATE]);
      return false;
  }
  return true;
}

bool lora_send(const String &txpacket, uint32_t timeout) {
  Serial.printf("\nsending packet:%s:, length:%d\n",txpacket.c_str(), txpacket.length());
  LORA_STATE = L_BUSY;
  uint32_t start_time = millis();
  Radio.Send( (uint8_t *)txpacket.c_str(), txpacket.length() ); //send the package out	
  while((L_BUSY == LORA_STATE) && (!is_timed_out(start_time, timeout))){
    Radio.IrqProcess();
  }
  if(L_TX_DONE != LORA_STATE)
  {
    Serial.printf("\nLORA transmit failed. State:%s\n", LORA_STATE_NAMES[LORA_STATE]);
    return false;
  }
  return true;
}

void OnTxDone( void ) {
	Serial.println("\nTX done......");
  LORA_STATE = L_TX_DONE;
}

void OnTxTimeout( void ) {
  Radio.Sleep( );
  Serial.println("\nTX Timeout......");
  LORA_STATE = L_TX_TIMEOUT;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  RXPACKET = String(payload, size);
  Radio.Sleep( );
  Serial.printf("\nreceived packet:%s: with rssi:%d\n", RXPACKET.c_str(), rssi);
  LORA_STATE = L_RX_DONE;
}

void OnRxTimeout() {
  Radio.Sleep( );
  Serial.println("\nRX Timeout......");
  LORA_STATE = L_RX_TIMEOUT;
}

void OnRxError() {
  Radio.Sleep( );
  Serial.println("\nRX Error......");
  LORA_STATE = L_RX_ERROR;  
}

bool is_timed_out(uint32_t start_time, uint32_t timeout){
  return (millis() - start_time) >= timeout;
}