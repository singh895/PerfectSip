#include "stm32f0xx.h"
#include <stdio.h>


void nano_wait(unsigned int);
void setup_serial(void);
void setup_adc(void);
float read_adc_voltage(void);
void print_voltage(float voltage);
void internal_clock();
void setup_adcC(void);
void setup_gpio(void);
void setup_button_interrupt(void);
void EXTI0_1_IRQHandler(void);


void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

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

float read_adc_voltage(void) {
    ADC1->CR |= ADC_CR_ADSTART; // Start ADC conversion
    while (!(ADC1->ISR & ADC_ISR_EOC)); // Wait until conversion is complete

    // Read converted value
    uint16_t adc_value = ADC1->DR;

    // Convert ADC value to voltage
    return (3.3f * adc_value) / 4095.0f;
}

void print_voltage(float voltage) {
    char buffer[32];
    int wholePart = (int)voltage;
    int decimalPart = (int)((voltage - wholePart) * 1000); // Assuming 2 decimal places
    sprintf(buffer, "Voltage: %d.%02d V\n", wholePart, decimalPart);
    print_string(buffer);
}

float voltage_to_temp(float voltage) {
    float temp = 0-(0.421)*(330/voltage-100)+124.84;
    return temp;
}

void print_temp(float voltage) { 
    char buffer[32];
    float temp = voltage_to_temp(voltage);
    int wholePart = (int)temp;
    int decimalPart = (int)((temp - wholePart) * 1000); // Assuming 2 decimal places
    sprintf(buffer, "Temprature: %d.%02d F\n\n", wholePart, decimalPart);
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

int main(void) {
    internal_clock();
    setup_serial();
    setup_adc();
    setup_gpio();
    setup_button_interrupt();
    

    while(1) {
        nano_wait(2000000000);
        float voltage = read_adc_voltage();
        print_voltage(voltage);
        print_temp(voltage);
        check_temp_turnoff(voltage_to_temp(voltage));
    }
}



