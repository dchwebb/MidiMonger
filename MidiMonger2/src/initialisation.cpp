#include "initialisation.h"

//Btn modeBtn{GPIOC, 13, GpioPin::Type::InputPulldown};
Btn modeBtn{GPIOC, 2, GpioPin::Type::InputPulldown};		// FIXME - set temporarily as conflicts with CV LED2 on module hardware

void InitHardware()
{
	InitClocks();
	InitSysTick();
}


struct PLLDividers {
	uint32_t M;
	uint32_t N;
	uint32_t P;
	uint32_t Q;
};
const PLLDividers mainPLL {4, 180, 2, 7};		// Clock: 8MHz / 4(M) * 168(N) / 2(P) = 180MHz
const PLLDividers saiPLL {6, 144, 4, 0};		// USB:   8MHz / 6(M) * 144(N) / 4(P) = 48MHz

void InitClocks()
{
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;		// enable system configuration clock
	[[maybe_unused]] volatile uint32_t dummy = RCC->APB2ENR & RCC_APB2ENR_SYSCFGEN;		// delay

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;			// Enable Power Control clock
	PWR->CR |= PWR_CR_VOS_0;					// Enable VOS voltage scaling - allows maximum clock speed

	SCB->CPACR |= ((3 << 10 * 2) | (3 << 11 * 2));	// CPACR register: set full access privileges for coprocessors

	RCC->CR |= RCC_CR_HSEON;					// HSE ON
	while ((RCC->CR & RCC_CR_HSERDY) == 0);		// Wait till HSE is ready

	RCC->PLLCFGR = 	(mainPLL.M << RCC_PLLCFGR_PLLM_Pos) |
					(mainPLL.N << RCC_PLLCFGR_PLLN_Pos) |
					(((mainPLL.P >> 1) - 1) << RCC_PLLCFGR_PLLP_Pos) |
					(mainPLL.Q << RCC_PLLCFGR_PLLQ_Pos) |
					RCC_PLLCFGR_PLLSRC_HSE;

	RCC->CFGR |= RCC_CFGR_HPRE_DIV1 |			// HCLK = SYSCLK / 1
				 RCC_CFGR_PPRE1_DIV4 |			// PCLK1 = HCLK / 4 (APB1)
				 RCC_CFGR_PPRE2_DIV2;			// PCLK2 = HCLK / 2 (APB2)

	RCC->CR |= RCC_CR_PLLON;					// Enable the main PLL
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);		// Wait till the main PLL is ready

	// PLLSAI used for USB
	RCC->PLLSAICFGR = (saiPLL.M << RCC_PLLSAICFGR_PLLSAIM_Pos) |
					  (saiPLL.N << RCC_PLLSAICFGR_PLLSAIN_Pos) |
					  (((saiPLL.P >> 1) - 1) << RCC_PLLSAICFGR_PLLSAIP_Pos);

	RCC->CR |= RCC_CR_PLLSAION;					// Enable the SAI PLL for USB

	// Configure Flash prefetch, Instruction cache, Data cache and wait state
	FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;

	// Select the main PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

	// Enable data and instruction cache
	FLASH->ACR |= FLASH_ACR_ICEN;
	FLASH->ACR |= FLASH_ACR_DCEN;
	FLASH->ACR |= FLASH_ACR_PRFTEN;				// Enable the FLASH prefetch buffer

	SystemCoreClockUpdate();					// Update SystemCoreClock variable
}


void InitSysTick()
{
	SysTick_Config(SystemCoreClock / sysTickInterval);		// gives 1ms
	NVIC_SetPriority(SysTick_IRQn, 0);
}


void DelayMS(uint32_t ms)
{
	// Crude delay system
	const uint32_t now = SysTickVal;
	while (now + ms > SysTickVal) {};
}


void InitMidiUART() {
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


void Reboot()
{
	__disable_irq();
	__DSB();

	SysTick->CTRL = 0;							// Disable Systick timer

	// Disable all peripheral clocks
	RCC->AHB1ENR = 0;
	RCC->AHB2ENR = 0;
	RCC->AHB3ENR = 0;
	RCC->APB1ENR = 0;
	RCC->APB2ENR = 0;

	for (uint32_t i = 0; i < 5; ++i) {			// Clear Interrupt Enable Register & Interrupt Pending Register
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	NVIC_SystemReset();
}


void JumpToBootloader()
{
	volatile uint32_t bootAddr = 0x1FFF0000;	// Set the address of the entry point to bootloader
	__disable_irq();							// Disable all interrupts
	SysTick->CTRL = 0;							// Disable Systick timer

	// Disable all peripheral clocks
	RCC->AHB1ENR = 0;
	RCC->AHB2ENR = 0;
	RCC->AHB3ENR = 0;
	RCC->APB1ENR = 0;
	RCC->APB2ENR = 0;

	for (uint32_t i = 0; i < 5; ++i) {			// Clear Interrupt Enable Register & Interrupt Pending Register
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	__enable_irq();								// Re-enable all interrupts
	void (*SysMemBootJump)() = (void(*)()) (*((uint32_t *) (bootAddr + 4)));	// Set up the jump to booloader address + 4
	__set_MSP(*(uint32_t *)bootAddr);			// Set the main stack pointer to the bootloader stack
	SysMemBootJump(); 							// Call the function to jump to bootloader location

	while (1) {
		// Code should never reach this loop
	}
}
