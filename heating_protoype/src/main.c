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

void print_temp(float temp) { 
    char buffer[32];
    //float temp = voltage_to_temp(voltage);
    int wholePart = (int)temp;
    int decimalPart = (int)((temp - wholePart) * 1000); // Assuming 2 decimal places
    sprintf(buffer, "Temperature: %d.%02d F\n\n", wholePart, decimalPart);
    print_string(buffer);
}
void check_temp_turnoff(float temp, float desired_temp){
    if(temp > desired_temp + 2.5){
        if(GPIOC->ODR & 0x1){
            //if not already off
            print_string("\nTurned Off Heater\n\n");
        }
        GPIOC->BSRR = GPIO_BSRR_BR_0;  
        }
    else if(temp < desired_temp - 2.5){
        if(!(GPIOC->ODR & 0x1)){
            //if not already on
            print_string("\nTurned On Heater\n\n");
        }
        GPIOC->BSRR = GPIO_BSRR_BS_0;
    }    
}

void shiftOut(uint16_t data) {

    for (uint16_t i = 0; i < 16; i++) {
        GPIOC->BSRR = (data & 0x8000) ? (GPIO_BSRR_BS_3 | (1 << 2)) : (GPIO_BSRR_BR_3 | (1 << 2));
        GPIOC->BSRR = (1 << 4);
        GPIOC->BSRR = (1 << 4) << 16;
        data <<= 1;
    }
    GPIOC->BSRR = (1 << 5);
    GPIOC->BSRR = (1 << 5) << 16;
}

int main(void) {
    internal_clock();
    setup_serial();
    setup_adc();
    setup_gpio();
    setup_button_interrupt();
    // LED Code has been added
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Enable GPIOC clock
    GPIOC->MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0; // Set PC3, PC4, and PC5 as outputs
    float desired_temp = 110;
    unsigned int first_LED = 0b0000000000000011;
    
    float change_in_temp = 0; // accumulates until its >= LED_threshold and then is reset to 0 
    float prev_temp = 0;      // previous temperature read by ADC 
    float LED_threshold = 0;  // once temperature has increased by this amount turn on another LED
    int ctrl_var = 1;         // controls the LEDs incrementally turning on 

    while(1) {
        nano_wait(2000000000);
        //print_string("Start of while Loop \n");
        float voltage = read_adc_voltage();
        float curr_temp = voltage_to_temp(voltage);
        print_voltage(voltage);
        check_temp_turnoff(voltage_to_temp(voltage), desired_temp);
        if(prev_temp == 0) {
            LED_threshold = (desired_temp - curr_temp) / 16.0;
        }

        change_in_temp += curr_temp - prev_temp;

        print_string("Threshold");
        print_temp(LED_threshold);
        print_string("Temperature Change");
        print_temp(change_in_temp);
        print_string("Current Temperature: ");
        print_temp(curr_temp);
        print_string("Previous Temperature: ");
        print_temp(prev_temp);

        if(change_in_temp >= LED_threshold) {
            if(ctrl_var < 16) {
                shiftOut(first_LED);
                first_LED |= (1 << (ctrl_var + 1));
                ctrl_var++;
                change_in_temp = 0;
                print_string("LED should turn on \n");
            }
        }
        prev_temp = curr_temp;
        nano_wait(2000000000);
        print_string("End of while Loop \n");
    }
}



