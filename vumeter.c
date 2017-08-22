#define DEBUG

#define F_CPU 1000000L // CKDIV8 unprogrammed

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifdef DEBUG
  #define SERIAL_DEBUG_DDR DDRB
  #define SERIAL_DEBUG_PORT PORTB
  #define SERIAL_DEBUG_PIN PB1
  #include "dbginclude.c"
#endif

#define PIN_I_ENABLE PB0
//#define PIN_O_DATA   PB1
#define PIN_I_AUDIO  PB3
//#define PIN_O_SHIFT  PB2
//#define PIN_O_STORE  PB4

//#define ADC_READ_COUNT 4
#define ADC_DROP 4


static volatile uint16_t g_adc_min = 0xFFFF;
static volatile uint16_t g_adc_max = 0 ;
static volatile uint8_t g_adc_run ;


ISR(ADC_vect) {
  static uint16_t l_adc = 0 ;
  static uint8_t l_adc_drop = ADC_DROP ;

  l_adc = ADCL ;
  l_adc += (ADCH << 8) ;
  if (l_adc_drop) l_adc_drop-- ;
    else if (l_adc < g_adc_min) g_adc_min = l_adc ;
      else if (l_adc > g_adc_max) g_adc_max = l_adc ;
  if (g_adc_run) ADCSRA |= _BV(ADSC) ; // New acquisition
}

/*
//uint8_t Audio_Read() {
uint16_t Audio_Read() {
  uint16_t l_sum ;
  uint8_t l_count ;

  l_sum = 0 ;
  // Read ADC several times
  for (l_count=0 ; l_count<ADC_READ_COUNT ; l_count++) {
    ADCSRA |= _BV(ADSC) ; // Start conversion
    while (ADCSRA & _BV(ADSC)) ; // Wait conversion to complete
//    l_sum += ADCH ;
    l_sum += ADCL ;
    l_sum += (ADCH << 8) ;
  }
//  return (uint8_t)(l_sum / ADC_READ_COUNT) ;
  return (l_sum / ADC_READ_COUNT) ;
}
*/

int main(void) {
  uint16_t l_adc_min, l_adc_max ;

//  DDRB = _BV(PIN_O_DATE) | _BV(PIN_O_SHIFT) | _BV(PIN_O_STORE) ; //Output
//  DDRB = 0 ;
//  PORTB = _BV(PIN_I_AUDIO) | _BV(PIN_I_ENABLE) ;
//   PORTB |= _BV(PIN_I_ENABLE) ;

  // ADC init
  ADMUX = _BV(MUX1) | _BV(MUX0) ;
  ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) ;

#ifdef DEBUG
//  OSCCAL = 0x56;
  Serial_Debug_Init() ;
#endif


  for(;;) {
    sei() ;
      g_adc_run = 1 ;
      ADCSRA |= _BV(ADSC) ; // Start acquisition
      _delay_ms(5000) ;
      g_adc_run = 0 ;
    cli() ;
    l_adc_min = g_adc_min ;
    l_adc_max = g_adc_max ;
#ifdef DEBUG
    Serial_Debug_Send(0xF1) ; _delay_ms(500) ;
    Serial_Debug_Send((uint8_t)(l_adc_min >> 8)) ; _delay_ms(500) ;
    Serial_Debug_Send((uint8_t)(l_adc_min & 0xFF)) ; _delay_ms(1000) ;
    Serial_Debug_Send(0xF2) ; _delay_ms(500) ;
    Serial_Debug_Send((uint8_t)(l_adc_max >> 8)) ; _delay_ms(500) ;
    Serial_Debug_Send((uint8_t)(l_adc_max & 0xFF)) ; _delay_ms(1000) ;
#endif
  }
}
