#include <avr/io.h>
#include "voltage.h"

unsigned int read_adc(void){
    unsigned int adclow = ADCL;
    return(adclow + ((ADCH & 0x03) << 8));
}

unsigned int read_adc(void);

float voltage()
{   	 
    static unsigned char adc_started = 0; 

    if (!adc_started){
        ADMUX = 0b01000000; //ADC0 single ended input on portC0
        ADCSRB = 0b00000000; // Configuring the free running mode p.220
        ADCSRA = 0b11100111; //Enabling ADC, auto-trigger, prescaler(division) 128 p.2
        while (!(ADCSRA & (1 << ADIF)));
            adc_started = 1;
    }
    
    unsigned int adc_value = read_adc();
    float voltage;

    voltage=((adc_value * 5.0) / 1024);

    if (voltage >= 0 && voltage < 5)
    {
        return(voltage);
    }
    else return -1;
}