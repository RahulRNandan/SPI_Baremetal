# SPI (Serial Peripheral Interface) Communication - Baremetal Implementation

SPI (Serial Peripheral Interface) is a synchronous, full-duplex communication protocol typically used to send data between microcontrollers and peripheral devices. Unlike I2C, SPI uses four lines for communication and can achieve higher data transfer rates, making it suitable for fast data exchanges.

## Key Features of SPI:
- **Full-Duplex Communication**: Simultaneous sending and receiving of data.
- **Master-Slave Configuration**: The master generates the clock, and slaves respond to the master’s requests.
- **Four-Wire Communication**:
  - **MOSI** (Master Out, Slave In): Data sent from the master to the slave.
  - **MISO** (Master In, Slave Out): Data sent from the slave to the master.
  - **SCK** (Serial Clock): Clock signal generated by the master.
  - **SS/CS** (Slave Select/Chip Select): Pin used to select a slave device.

A typical SPI communication sequence involves the master asserting the slave select line (SS), sending data on the MOSI line while receiving data on the MISO line, and deasserting the slave select line after the transaction.

---

# SPI Master Code Implementation Algorithm

The following algorithm describes how to implement SPI master communication on an STM32 microcontroller using bare-metal programming.

## Algorithm Breakdown:

### 1. **SPI Initialization**
   - **Enable Clocks**: Enable clocks for both the SPI1 peripheral and GPIOA (for SPI pins).
   - **Configure GPIO Pins**: Set PA5 (SCK), PA6 (MISO), PA7 (MOSI), and PA4 (NSS/CS) for SPI communication.
   - **Set SPI Mode**: Configure SPI mode (master/slave, clock polarity, and phase).
   - **Enable SPI Peripheral**: Enable the SPI1 peripheral.

### 2. **Data Transmission**
   - Wait for the TXE (Transmit Buffer Empty) flag to be set.
   - Write data to the SPI data register (SPI_DR).
   - Wait for the BSY (Busy) flag to be cleared, indicating that the transmission is complete.

### 3. **Data Reception**
   - Wait for the RXNE (Receive Buffer Not Empty) flag to be set.
   - Read data from the SPI data register.

### 4. **End Communication**
   - The communication ends when the master deasserts the slave select (NSS/CS) line, releasing the SPI bus.

---

# STM32 SPI1 Peripheral Initialization and Communication

This project demonstrates how to configure and use SPI1 for communication on an STM32 microcontroller using bare-metal programming.

## Memory-Mapped Registers

```c
#define SPI1_BASE_ADDR  0x40013000  // Base address for SPI1 peripheral
#define RCC_BASE_ADDR   0x40023800  // Base address for RCC (Reset and Clock Control)
#define GPIOA_BASE_ADDR 0x40020000  // Base address for GPIOA (for SPI1 pins)
```

### SPI1 Registers:
- **SPI1_CR1**: Control Register 1 (configures clock polarity, phase, baud rate, and master/slave mode).
- **SPI1_CR2**: Control Register 2 (enables interrupts and DMA, configures frame format).
- **SPI1_SR**: Status Register (monitors SPI bus status).
- **SPI1_DR**: Data Register (for data transmission and reception).

### RCC and GPIOA Registers:
- **RCC_AHB1ENR**: AHB1 Peripheral Clock Enable Register (enables GPIOA clock).
- **RCC_APB2ENR**: APB2 Peripheral Clock Enable Register (enables SPI1 clock).
- **GPIOA_MODER**: GPIO Mode Register (sets GPIO pin mode).
- **GPIOA_AFRL**: GPIO Alternate Function Low Register (assigns alternate function for pins).

---

## Clock Setup

Before configuring SPI1, enable the clocks for GPIOA and SPI1:

```c
RCC_AHB1ENR |= (1 << 0);   // Enable GPIOA clock
RCC_APB2ENR |= (1 << 12);  // Enable SPI1 clock
```

---

## GPIO Configuration

Configure GPIO pins PA4 (NSS), PA5 (SCK), PA6 (MISO), and PA7 (MOSI) for SPI functionality:

```c
GPIOA_MODER &= ~((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14)); // Clear mode bits for PA4, PA5, PA6, PA7
GPIOA_MODER |= (2 << 8) | (2 << 10) | (2 << 12) | (2 << 14);    // Set alternate function mode for PA4, PA5, PA6, PA7
GPIOA_AFRL &= ~((0xF << 16) | (0xF << 20) | (0xF << 24) | (0xF << 28)); // Clear alternate function bits
GPIOA_AFRL |= (5 << 16) | (5 << 20) | (5 << 24) | (5 << 28);    // Set AF5 (SPI1) for PA4, PA5, PA6, PA7
```

---

## SPI Initialization

Configure the SPI1 peripheral as a master, with a baud rate of fPCLK/16 and clock polarity/phase set to mode 0 (CPOL = 0, CPHA = 0):

```c
SPI1_CR1 = 0;  // Clear control register
SPI1_CR1 |= (1 << 2);   // Set master mode
SPI1_CR1 |= (3 << 3);   // Set baud rate to fPCLK/16
SPI1_CR1 |= (0 << 1);   // Set CPOL = 0
SPI1_CR1 |= (0 << 0);   // Set CPHA = 0
SPI1_CR1 |= (1 << 6);   // Enable SPI1 peripheral
```

---

## SPI Read/Write Functions

### Transmit Function

```c
void spi_transmit(uint8_t data) {
    while (!(SPI1_SR & (1 << 1)));  // Wait until TXE (Transmit Buffer Empty) is set
    SPI1_DR = data;                 // Write data to Data Register
    while (SPI1_SR & (1 << 7));     // Wait until BSY (Busy) flag is cleared
}
```

### Receive Function

```c
uint8_t spi_receive() {
    while (!(SPI1_SR & (1 << 0)));  // Wait until RXNE (Receive Buffer Not Empty) is set
    return SPI1_DR;                 // Return received data from Data Register
}
```

---

## SPI Communication Example

This example demonstrates how to perform an SPI communication transaction, where the master sends data and receives a response from the slave.

```c
void spi_exchange_data(uint8_t data) {
    GPIOA->ODR &= ~(1 << 4);  // Pull NSS (CS) low to select the slave
    spi_transmit(data);       // Transmit data
    uint8_t received_data = spi_receive();  // Receive data
    GPIOA->ODR |= (1 << 4);   // Pull NSS (CS) high to deselect the slave
}
```

### Sequence:
1. The master pulls the NSS line low to initiate communication with the slave.
2. The master sends data using the `spi_transmit()` function.
3. The master waits for the response using the `spi_receive()` function.
4. The master pulls the NSS line high to end the communication.

---

This documentation provides a complete guide for configuring and using SPI communication on an STM32 microcontroller in bare-metal programming. It includes the initialization of SPI, data transmission and reception, and example code for typical SPI communication.
```

This document outlines the steps required to configure and implement SPI communication on the STM32 microcontroller, including how to set up the GPIO pins, initialize the SPI peripheral, and write code for transmitting and receiving data. Let me know if you need further customization!
