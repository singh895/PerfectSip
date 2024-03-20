#ifndef __team16temp__
#define __team16temp__

typedef struct {
    int8_t temperature;       // Temperature in Celsius
    float resistance_min;    // Minimum resistance in KOhms
    float resistance_center; // Center resistance in KOhms
    float resistance_max;    // Maximum resistance in KOhms
} R_T_Table_Row;

#define RT_TABLE_SIZE 35

extern R_T_Table_Row rt_table[RT_TABLE_SIZE];

extern int THIS_MAKES_COMPILER_ANGRY;

void setup_serial(void);
void setup_adc(void);
float read_adc_voltage(void);
void print_voltage(float voltage);
void internal_clock();
void setup_adcC(void);
void setup_gpio(void);
void setup_button_interrupt(void);
void EXTI0_1_IRQHandler(void);
void print_string(const char*);
float adc_to_resistance(uint16_t );
int8_t temperature_from_resistance(float );
float celsius_to_fahrenheit(float );
void print_temp(float );
void check_temp_turnoff(float);

#endif