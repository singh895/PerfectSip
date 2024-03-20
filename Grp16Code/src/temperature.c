#include "stm32f0xx.h"
#include <stdio.h>
#include "temperature.h"

R_T_Table_Row rt_table[RT_TABLE_SIZE] = {
    {48, 38.072, 38.822, 39.583},
    {49, 36.611, 37.346, 38.093},
    {50, 35.119, 35.840, 36.571},
    {51, 33.873, 34.580, 35.299},
    {52, 32.591, 33.284, 33.989},
    {53, 31.363, 32.043, 32.734},
    {54, 30.188, 30.853, 31.530},
    {55, 29.061, 29.713, 30.377},
    {56, 27.982, 28.620, 29.271},
    {57, 26.948, 27.573, 28.210},
    {58, 25.956, 26.568, 27.192},
    {59, 25.006, 25.605, 26.216},
    {60, 24.095, 24.681, 25.279},
    {61, 23.221, 23.794, 24.380},
    {62, 22.382, 22.944, 23.517},
    {63, 21.578, 22.127, 22.688},
    {64, 20.807, 21.344, 21.893},
    {65, 20.066, 20.592, 21.129},
    {66, 19.355, 19.869, 20.395},
    {67, 18.673, 19.175, 19.690},
    {68, 18.017, 18.509, 19.012},
    {69, 17.388, 17.869, 18.361},
    {70, 16.783, 17.253, 17.735},
    {71, 16.203, 16.662, 17.133},
    {72, 15.645, 16.094, 16.555},
    {73, 15.108, 15.548, 15.998},
    {74, 14.593, 15.022, 15.463},
    {75, 14.097, 14.517, 14.948},
    {76, 13.621, 14.031, 14.453},
    {77, 13.163, 13.564, 13.976},
    {78, 12.722, 13.114, 13.517},
    {79, 12.298, 12.681, 13.075},
    {80, 11.890, 12.265, 12.650},
    {81, 11.498, 11.864, 12.241},
    {82, 11.120, 11.478, 11.846}
};

int THIS_MAKES_COMPILER_ANGRY = 100;


void print_string(const char* str) {
    while (*str != '\0') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = *str++;
    }
}

void setup_serial(void) {
    RCC->AHBENR |= 0x00180000;
    GPIOC->MODER  |= 0x02000000;
    GPIOC->AFR[1] |= 0x00020000;
    GPIOD->MODER  |= 0x00000020;
    GPIOD->AFR[0] |= 0x00000200;
    RCC->APB1ENR |= 0x00100000;
    USART5->CR1 &= ~0x00000001;
    USART5->CR1 |= 0x00008000;
    USART5->BRR = 0x340;
    USART5->CR1 |= 0x0000000c;
    USART5->CR1 |= 0x00000001;
}

void setup_adc(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // Enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; // Enable ADC clock

    // Configure PA1 as analog mode
    GPIOA->MODER |= GPIO_MODER_MODER1;

    // Configure ADC
    ADC1->CR |= ADC_CR_ADEN; // Enable ADC
    while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait until ADC is ready

    ADC1->CHSELR |= ADC_CHSELR_CHSEL1; // Select channel 1 for conversion
}

void setup_adcC(void) {
    // Enable GPIOC clock
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= GPIO_MODER_MODER0; // Set PC0 to analog mode
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC1->CR &= ~ADC_CR_ADEN;
    // Set ADC channel
    ADC1->CHSELR = 0; // Clear all channels
    ADC1->CHSELR |= ADC_CHSELR_CHSEL10; // Set channel 10 (connected to PC0)
    ADC1->CFGR1 &= ~ADC_CFGR1_RES;
    ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADCAL;   
    while (ADC1->CR & ADC_CR_ADCAL);
}

float adc_to_resistance(uint16_t adc_value) {
    float voltage = (float)adc_value / 4095.0f * 3.3f; // Assuming 3.3V reference voltage
    float resistance = (10000.0f * voltage) / (3.3f - voltage); // Assuming a 10kΩ resistor in series
    return resistance;
}

int8_t temperature_from_resistance(float resistance) {
    for (size_t i = 0; i < RT_TABLE_SIZE - 1; ++i) {
        if (resistance >= rt_table[i].resistance_min && resistance <= rt_table[i].resistance_max) {
            float ratio = (resistance - rt_table[i].resistance_min) / (rt_table[i].resistance_max - rt_table[i].resistance_min);
            float temperature = rt_table[i].temperature + ratio * (rt_table[i+1].temperature - rt_table[i].temperature);
            return (int8_t)temperature;
        }
    }
    return -1; 
}

float read_adc_voltage(void) {
    ADC1->CR |= ADC_CR_ADSTART; 
    while (!(ADC1->ISR & ADC_ISR_EOC));
    uint16_t adc_value = ADC1->DR;
	float resistance = adc_to_resistance(adc_value);

    int8_t temperature = temperature_from_resistance(resistance);

    return temperature;
}

void print_voltage(float voltage) {
    char buffer[32];
    int wholePart = (int)voltage;
    // int decimalPart = (int)((voltage - wholePart) * 1000); // Assuming 2 decimal places
    sprintf(buffer, "Voltage: %d V\n", wholePart);
    print_string(buffer);
}

float voltage_to_temp(float voltage) {
    float temp = 0-(0.421)*(330/voltage-100)+124.84;
    return temp;
}

float celsius_to_fahrenheit(float celsius) {
    return (celsius * 9 / 5) + 32;
}

void print_temp(float voltage) { 
    char buffer[32];
    float temp = voltage_to_temp(voltage);
    int wholePart = (int)temp;
    // int decimalPart = (int)((temp - wholePart) * 1000); // Assuming 2 decimal places
    sprintf(buffer, "Temprature: %d F\n\n", wholePart);
    print_string(buffer);
}


void setup_gpio(void) {
    // Enable GPIOC and GPIOB clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOBEN;

    // Configure PC0 as output
    GPIOC->MODER |= GPIO_MODER_MODER0_0; // Set PC0 to general purpose output mode

    // Configure PB1 as input with pull-down
    GPIOB->MODER &= ~GPIO_MODER_MODER1; // Clear MODER bits for PB1 to set it as input
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR1_1; // Set PB1 to pull-down mode
}

void setup_button_interrupt(void) {
    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Connect EXTI1 line to PB1
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PB;

    // Enable interrupt on EXTI1 line
    EXTI->IMR |= EXTI_IMR_MR1;

    // Trigger interrupt on rising edge and falling edge
    EXTI->RTSR |= EXTI_RTSR_TR1;
    EXTI->FTSR |= EXTI_FTSR_TR1;

    // Enable EXTI1 line interrupt in NVIC
    NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void EXTI0_1_IRQHandler(void) {
    // Check if EXTI1 interrupt flag is set
    if (EXTI->PR & EXTI_PR_PR1) {
        // Clear EXTI1 interrupt flag
        EXTI->PR |= EXTI_PR_PR1;
    }

    if (GPIOB->IDR & 0x2) {
        GPIOC->BSRR = GPIO_BSRR_BS_0;
    }
    else{
        GPIOC->BSRR = GPIO_BSRR_BR_0;
    }
}

void check_temp_turnoff(float temp){
    if(temp > 102.0 ){
        if(GPIOC->ODR & 0x1){
            //if not already off
            print_string("\nTurned Off Heater\n\n");
        }
        GPIOC->BSRR = GPIO_BSRR_BR_0;  
        }
    else if(temp < 98){
        if(!(GPIOC->ODR & 0x1)){
            //if not already on
            print_string("\nTurned On Heater\n\n");
        }
        GPIOC->BSRR = GPIO_BSRR_BS_0;
    }
    return;    
}

// int main(void) {
//     internal_clock();
//     setup_serial();
//     setup_adc();
//     setup_gpio();
//     setup_button_interrupt();
    

//     while(1) {
//         nano_wait(2000000000);
//         float voltage = read_adc_voltage();
//         print_voltage(voltage);
//         print_temp(voltage);
//         check_temp_turnoff(voltage_to_temp(voltage));
//     }
// }
