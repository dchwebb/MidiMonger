#include "initialisation.h"

#define USE_HSE
#define PLL_M 4
#define PLL_N 144
#define PLL_P 2		//  Main PLL (PLL) division factor for main system clock can be 2 (PLL_P = 0), 4 (PLL_P = 1), 6 (PLL_P = 2), 8 (PLL_P = 3)
#define PLL_Q 6

void SystemClock_Config(void) {

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;			// Enable Power Control clock
	PWR->CR |= PWR_CR_VOS_0;					// Enable VOS voltage scaling - allows maximum clock speed

	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));// CPACR register: set full access privileges for coprocessors

#ifdef USE_HSE
	RCC->CR |= RCC_CR_HSEON;					// HSE ON
	while ((RCC->CR & RCC_CR_HSERDY) == 0);		// Wait till HSE is ready
	RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
#endif

#ifdef USE_HSI
	RCC->CR |= RCC_CR_HSION;					// HSI ON
	while((RCC->CR & RCC_CR_HSIRDY) == 0);		// Wait till HSI is ready
    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) | (RCC_PLLCFGR_PLLSRC_HSI) | (PLL_Q << 24);
#endif

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;			// HCLK = SYSCLK / 1
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;			// PCLK2 = HCLK / 2 (APB2)
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;			// PCLK1 = HCLK / 4 (APB1)
	RCC->CR |= RCC_CR_PLLON;					// Enable the main PLL
	while((RCC->CR & RCC_CR_PLLRDY) == 0);		// Wait till the main PLL is ready

	// Configure Flash prefetch, Instruction cache, Data cache and wait state
	FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;

	// Select the main PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	// Wait till the main PLL is used as system clock source
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);

}

void InitIO()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// reset and clock control - advanced high performance bus - GPIO port A
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;			// reset and clock control - advanced high performance bus - GPIO port B
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;			// reset and clock control - advanced high performance bus - GPIO port C

	// configure PC13 blue button
	//GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved

	//GPIOA->MODER |= GPIO_MODER_MODER7_0;			// Set to output

	/*
	// PB7 is LD2 Blue
	GPIOB->MODER |= GPIO_MODER_MODER7_0;			// Set to output

	// PB14 is LD3 Red
	GPIOB->MODER |= GPIO_MODER_MODER14_0;			// Set to output

	// PA3 for gate 7 output
	GPIOA->MODER |= GPIO_MODER_MODER3_0;			// Set to output

	// PC0 for gate 6 output
	GPIOC->MODER |= GPIO_MODER_MODER0_0;			// Set to output

	// PC3 for gate 5 output
	GPIOC->MODER |= GPIO_MODER_MODER3_0;			// Set to output
*/

}

void InitSysTick()
{
	// Register macros found in core_cm4.h
	SysTick->CTRL = 0;									// Disable SysTick
	SysTick->LOAD = 0xFFFF - 1;							// Set reload register to maximum 2^24 - each tick is around 400us

	// Set priority of Systick interrupt to least urgency (ie largest priority value)
	NVIC_SetPriority (SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	SysTick->VAL = 0;									// Reset the SysTick counter value

	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;		// Select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;			// Enable SysTick interrupt
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;			// Enable SysTick
}

//	Setup Timer 3 on an interrupt to trigger sample acquisition
void InitSampleAcquisition() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;				// Enable Timer 3
	TIM3->PSC = 50;									// Set prescaler
	TIM3->ARR = 140; 								// Set auto reload register

	TIM3->DIER |= TIM_DIER_UIE;						// DMA/interrupt enable register
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 0);					// Lower is higher priority

	TIM3->CR1 |= TIM_CR1_CEN;
	TIM3->EGR |= TIM_EGR_UG;						//  Re-initializes counter and generates update of registers
}

//	Setup Timer 9 to count clock cycles for coverage profiling
void InitCoverageTimer() {
	RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;				// Enable Timer
	TIM9->PSC = 100;
	TIM9->ARR = 65535;

	TIM9->DIER |= TIM_DIER_UIE;						// DMA/interrupt enable register
	NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
	NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 2);		// Lower is higher priority

}

//	Setup Timer 5 to count time between bounces
void InitDebounceTimer() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;				// Enable Timer
	TIM5->PSC = 10000;
	TIM5->ARR = 65535;
}


void InitEncoders() {
	// L Encoder: button on PA10, up/down on PB6 and PB7; R Encoder: Button on PB13, up/down on PC6 and PC7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// reset and clock control - advanced high performance bus - GPIO port A
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;			// reset and clock control - advanced high performance bus - GPIO port B
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;			// reset and clock control - advanced high performance bus - GPIO port C
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;			// Enable system configuration clock: used to manage external interrupt line connection to GPIOs

	// configure PA10 button to fire on an interrupt
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PA;	// Select Pin PA10 which uses External interrupt 2
	EXTI->RTSR |= EXTI_RTSR_TR10;					// Enable rising edge trigger
	EXTI->FTSR |= EXTI_FTSR_TR10;					// Enable falling edge trigger
	EXTI->IMR |= EXTI_IMR_MR10;						// Activate interrupt using mask register

	// configure PB13 button to fire on an interrupt
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR13_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PB;	// Select Pin PB4 which uses External interrupt 2
	EXTI->RTSR |= EXTI_RTSR_TR13;					// Enable rising edge trigger
	EXTI->FTSR |= EXTI_FTSR_TR13;					// Enable falling edge trigger
	EXTI->IMR |= EXTI_IMR_MR13;						// Activate interrupt using mask register

	// L Encoder using timer functionality - PB6 and PB7
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR6_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOB->MODER |= GPIO_MODER_MODER6_1;			// Set alternate function
	GPIOB->AFR[0] |= 2 << 24;						// Alternate function 2 is TIM4_CH1

	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOB->MODER |= GPIO_MODER_MODER7_1;			// Set alternate function
	GPIOB->AFR[0] |= 2 << 28;						// Alternate function 2 is TIM4_CH2

	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;				// Enable Timer 4
	TIM4->PSC = 0;									// Set prescaler
	TIM4->ARR = 0xFFFF; 							// Set auto reload register to max
	TIM4->SMCR |= TIM_SMCR_SMS_0 |TIM_SMCR_SMS_1;	// SMS=011 for counting on both TI1 and TI2 edges
	TIM4->SMCR |= TIM_SMCR_ETF;						// Enable digital filter
	TIM4->CNT = 32000;								// Start counter at mid way point
	TIM4->CR1 |= TIM_CR1_CEN;

	// R Encoder using timer functionality - PC6 and PC7
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR6_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOC->MODER |= GPIO_MODER_MODER6_1;			// Set alternate function
	GPIOC->AFR[0] |= 3 << 24;						// Alternate function 3 is TIM8_CH1

	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR7_0;			// Set pin to pull up:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOC->MODER |= GPIO_MODER_MODER7_1;			// Set alternate function
	GPIOC->AFR[0] |= 3 << 28;						// Alternate function 3 is TIM8_CH2

	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;				// Enable Timer 8
	TIM8->PSC = 0;									// Set prescaler
	TIM8->ARR = 0xFFFF; 							// Set auto reload register to max
	TIM8->SMCR |= TIM_SMCR_SMS_0 |TIM_SMCR_SMS_1;	// SMS=011 for counting on both TI1 and TI2 edges
	TIM8->SMCR |= TIM_SMCR_ETF;						// Enable digital filter
	TIM8->CNT = 32000;								// Start counter at mid way point
	TIM8->CR1 |= TIM_CR1_CEN;



/*
	NVIC_SetPriority(EXTI4_IRQn, 4);				// Lower is higher priority
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_SetPriority(EXTI9_5_IRQn, 4);				// Lower is higher priority
	NVIC_EnableIRQ(EXTI9_5_IRQn);
*/
	NVIC_SetPriority(EXTI15_10_IRQn, 4);			// Lower is higher priority
	NVIC_EnableIRQ(EXTI15_10_IRQn);

}

void InitUART() {
	// PC11 UART4_RX 79
	// [PA1  UART4_RX 24 (AF8) ** NB Dev board seems to have something pulling this pin to ground so can't use]

	RCC->APB1ENR |= RCC_APB1ENR_UART4EN;			// UART4 clock enable

	GPIOC->MODER |= GPIO_MODER_MODER11_1;			// Set alternate function on PC11
	GPIOC->AFR[1] |= 0b1000 << 12;					// Alternate function on PC11 for UART4_RX is 1000: AF8

	int Baud = (SystemCoreClock / 4) / (16 * 31250);
	UART4->BRR |= Baud << 4;						// Baud Rate (called USART_BRR_DIV_Mantissa) = (Sys Clock: 180MHz / APB1 Prescaler DIV4: 45MHz) / (16 * 31250) = 90
	UART4->CR1 &= ~USART_CR1_M;						// Clear bit to set 8 bit word length
	UART4->CR1 |= USART_CR1_RE;						// Receive enable

	// Set up interrupts
	UART4->CR1 |= USART_CR1_RXNEIE;
	NVIC_SetPriority(UART4_IRQn, 3);				// Lower is higher priority
	NVIC_EnableIRQ(UART4_IRQn);

	UART4->CR1 |= USART_CR1_UE;						// USART Enable

}


void InitDAC()
{
	// Once the DAC channelx is enabled, the corresponding GPIO pin (PA4 or PA5) is automatically connected to the analog converter output (DAC_OUTx).
	// Enable DAC and GPIO Clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// Enable GPIO Clock
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;				// Enable DAC Clock

	DAC->CR |= DAC_CR_EN1;							// Enable DAC using PA4 (DAC_OUT1)
	DAC->CR |= DAC_CR_BOFF1;						// Enable DAC channel output buffer to reduce the output impedance

	// output triggered with DAC->DHR12R1 = x;
}




