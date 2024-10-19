#include <stdint.h>

// Base address for SPI1 peripheral, RCC (Reset and Clock Control), and GPIOA (for SPI1 pins)
#define SPI1_BASE_ADDR  0x40013000  
#define RCC_BASE_ADDR   0x40023800  
#define GPIOA_BASE_ADDR 0x40020000  

// Define SPI1 registers with offsets from the base address
#define SPI1_CR1       *((volatile uint32_t *)(SPI1_BASE_ADDR + 0x00))  // SPI1 Control Register 1
#define SPI1_CR2       *((volatile uint32_t *)(SPI1_BASE_ADDR + 0x04))  // SPI1 Control Register 2
#define SPI1_SR        *((volatile uint32_t *)(SPI1_BASE_ADDR + 0x08))  // SPI1 Status Register
#define SPI1_DR        *((volatile uint32_t *)(SPI1_BASE_ADDR + 0x0C))  // SPI1 Data Register

// Define RCC registers with offsets from the base address
#define RCC_AHB1ENR   *((volatile uint32_t *)(RCC_BASE_ADDR + 0x30))  // RCC AHB1 Peripheral Clock Enable Register
#define RCC_APB2ENR   *((volatile uint32_t *)(RCC_BASE_ADDR + 0x44))  // RCC APB2 Peripheral Clock Enable Register

// Define GPIOA registers with offsets from the base address
#define GPIOA_MODER   *((volatile uint32_t *)(GPIOA_BASE_ADDR + 0x00))  // GPIOA Mode Register
#define GPIOA_AFRL    *((volatile uint32_t *)(GPIOA_BASE_ADDR + 0x20))  // GPIOA Alternate Function Low Register
#define GPIOA_ODR     *((volatile uint32_t *)(GPIOA_BASE_ADDR + 0x14))  // GPIOA Output Data Register

// GPIO Pins for SPI1 (PA4 -> NSS, PA5 -> SCK, PA6 -> MISO, PA7 -> MOSI)
#define NSS_PIN       (1 << 4)   // PA4 - NSS (Slave Select)
#define SCK_PIN       (1 << 5)   // PA5 - SCK (Serial Clock)
#define MISO_PIN      (1 << 6)   // PA6 - MISO (Master In Slave Out)
#define MOSI_PIN      (1 << 7)   // PA7 - MOSI (Master Out Slave In)

// Function prototypes for SPI initialization and data transfer
void spi_init(void);
void spi_transmit(uint8_t data);
uint8_t spi_receive(void);
void spi_exchange_data(uint8_t data);

int main(void) {
    // Initialize SPI1 peripheral
    spi_init();

    // Example SPI communication loop
    while (1) {
        spi_exchange_data(0x55);  // Send data (example: 0x55)
        // Add delay or other operations if needed
    }

    return 0;
}

// SPI1 initialization function
void spi_init(void) {
    // Enable the clock for GPIOA by setting bit 0 in RCC_AHB1ENR
    RCC_AHB1ENR |= (1 << 0);   

    // Enable the clock for SPI1 by setting bit 12 in RCC_APB2ENR
    RCC_APB2ENR |= (1 << 12);  

    // Configure GPIOA pins PA4, PA5, PA6, PA7 for SPI1 (Alternate Function)
    GPIOA_MODER &= ~((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14)); // Clear mode bits for PA4, PA5, PA6, PA7
    GPIOA_MODER |= (2 << 8) | (2 << 10) | (2 << 12) | (2 << 14);    // Set alternate function mode (AF) for PA4, PA5, PA6, PA7

    // Configure the alternate function to AF5 (SPI1) for pins PA4-PA7
    GPIOA_AFRL &= ~((0xF << 16) | (0xF << 20) | (0xF << 24) | (0xF << 28)); // Clear AF bits
    GPIOA_AFRL |= (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);    // Set AF5 (SPI1) for PA4, PA5, PA6, PA7

    // Configure SPI1 settings in the Control Register 1 (SPI1_CR1)
    SPI1_CR1 = 0;  // Clear the control register to start from a known state
    SPI1_CR1 |= (1 << 2);   // Set SPI1 to master mode
    SPI1_CR1 |= (3 << 3);   // Set the baud rate to fPCLK/16 (prescaler of 16)
    SPI1_CR1 |= (0 << 1);   // Set clock polarity (CPOL = 0, clock is low when idle)
    SPI1_CR1 |= (0 << 0);   // Set clock phase (CPHA = 0, data captured on the first edge)
    SPI1_CR1 |= (1 << 6);   // Enable SPI1 by setting the SPE (SPI Enable) bit
}

// Function to transmit a byte of data via SPI
void spi_transmit(uint8_t data) {
    // Wait until the TXE (Transmit Buffer Empty) flag is set in the status register
    while (!(SPI1_SR & (1 << 1)));  
    SPI1_DR = data;  // Write the data to the Data Register (SPI1_DR)
    // Wait until the BSY (Busy) flag is cleared, indicating transmission is complete
    while (SPI1_SR & (1 << 7));     
}

// Function to receive a byte of data via SPI
uint8_t spi_receive(void) {
    // Wait until the RXNE (Receive Buffer Not Empty) flag is set in the status register
    while (!(SPI1_SR & (1 << 0)));  
    return SPI1_DR;  // Return the received data from the Data Register (SPI1_DR)
}

// Function to exchange data (send and receive) over SPI
void spi_exchange_data(uint8_t data) {
    GPIOA_ODR &= ~NSS_PIN;  // Pull NSS (CS) low to select the slave
    spi_transmit(data);     // Transmit the data byte
    uint8_t received_data = spi_receive();  // Receive the data byte
    GPIOA_ODR |= NSS_PIN;   // Pull NSS (CS) high to deselect the slave
}

