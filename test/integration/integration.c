// Enable
#define NRF24L01_INTEGRATION_TEST_DEBUG

/** The integration tests in this file rely heavily on a BCM2835 driver from Mike McCauley
 *  
 *  See http://www.airspayce.com/mikem/bcm2835/modules.html for the full API
 * 
 *  See https://raspberry-projects.com/pi/programming-in-c/io-pins/bcm2835-by-mike-mccauley for installation
 */

#define NRF24L01_TEST_ENVIRONMENT
#include "../../inc/nrf24l01.h"

#include <bcm2835.h>
#include <stdio.h>

/** Select a SPI clock for testing. The maximum supported NRF24L01+ clock is 8MHz. 
 *  Therefore choose `BCM2835_SPI_CLOCK_DIVIDER_32` for a clock rate of 7.8125MHz
 * 
 *  Options (total H+L clock period):
 *    BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       ///< 65536 = 262.144us = 3.814697260kHz 
 *    BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   ///< 32768 = 131.072us = 7.629394531kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   ///< 16384 = 65.536us = 15.25878906kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    ///< 8192 = 32.768us = 30/51757813kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    ///< 4096 = 16.384us = 61.03515625kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    ///< 2048 = 8.192us = 122.0703125kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    ///< 1024 = 4.096us = 244.140625kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     ///< 512 = 2.048us = 488.28125kHz
 *    BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     ///< 256 = 1.024us = 976.5625MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     ///< 128 = 512ns = = 1.953125MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      ///< 64 = 256ns = 3.90625MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      ///< 32 = 128ns = 7.8125MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      ///< 16 = 64ns = 15.625MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       ///< 8 = 32ns = 31.25MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       ///< 4 = 16ns = 62.5MHz
 *    BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       ///< 2 = 8ns = 125MHz, fastest you can get
 *    BCM2835_SPI_CLOCK_DIVIDER_1     = 1,       ///< 1 = 262.144us = 3.814697260kHz, same as 0/65536
 */
#define SPI_CLOCK   BCM2835_SPI_CLOCK_DIVIDER_1024

/** Select the SPI mode. The NRF24L01+ requires SPI mode 0 (CPOL=0, CPHA=0).
 *  Therefore choose `BCM2835_SPI_MODE0` for the mode
 * 
 *  Options:
 *	  BCM2835_SPI_MODE0 = 0,  // CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
 *    BCM2835_SPI_MODE1 = 1,  // CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
 *    BCM2835_SPI_MODE2 = 2,  // CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
 *    BCM2835_SPI_MODE3 = 3,  // CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
 *    //(SPI_MODE_# | SPI_CS_HIGH)=Chip Select active high, (SPI_MODE_# | SPI_NO_CS)=1 device per bus no Chip Select
 */
#define SPI_MODE    BCM2835_SPI_MODE0

/** Select the CS mode. The NRF24L01+ requires an active low CS pin.
 *  Therefore choose `0` for the CS mode
 *  
 *  Options:
 *    0,  // CS pin is active low
 *    1,  // CS pin is active high
 */
#define SPI_CS_MODE 0

/** Select the device chip enable pin
 * 
 *  Options:
 *    Any of RPI_V2_GPIO_P1_x
 */
#define RADIO0_CE   RPI_V2_GPIO_P1_15 // GPIO 22 on raspberry pi diagram
#define RADIO1_CE   RPI_V2_GPIO_P1_16 // GPIO 23 on raspberry pi diagram

/** Select the host notification interrupt pin
 * 
 *  Options:
 *    Any of RPI_V2_GPIO_P1_x
 */
#define RADIO0_IRQ  RPI_V2_GPIO_P1_18 // GPIO 24 on raspberry pi diagram
#define RADIO1_IRQ  RPI_V2_GPIO_P1_22 // GPIO 25 on raspberry pi diagram

/** Struct for multi-device support
 * 
 */
typedef struct {
    uint8_t cs;      ///< One of `BCM2835_SPI_CS0` or `BCM2835_SPI_CS1`
    uint8_t ce_pin;  ///< One of `DEVICE0_CE` or `DEVICE1_CE`
    uint8_t irq_pin; ///< One of `DEVICE0_IRQ` or `DEVICE1_IRQ`
} raspi_nrf24l01_t;

// Forward declare the IO helper functions
static int8_t raspi_gpio_chip_enable_nrf24l01(bool, void*);
static int8_t raspi_delay_us_nrf24l01(uint32_t);
static int8_t raspi_platform_init_nrf24l01(void*);
static int8_t raspi_platform_deinit_nrf24l01(void*);
static int8_t raspi_spi_exchange_data_nrf24l01(uint8_t, uint8_t*, uint8_t*, uint8_t, void*);
static int8_t raspi_check_for_interrupt_nrf24l01(void*);

int main(void) {
    printf("Starting tests...\n");

    int init = bcm2835_init();
    if (init != 1) {
        printf("`bcm2835_init` failed...");
        exit(1);
    }

    // Setup SPI pins
    int begin = bcm2835_spi_begin();
    if (begin != 1) {
        printf("`bcm2835_spi_begin` failed...");
        exit(1);
    }

    // Set CS pins polarity to low
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, SPI_CS_MODE);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, SPI_CS_MODE);

    // Configure SPI clock
    bcm2835_spi_setClockDivider(SPI_CLOCK);

    // Set SPI data mode
    bcm2835_spi_setDataMode(SPI_MODE);

    // Set SPI bit order
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);

    // Configure the NRF24L01+ driver
    nrf24l01_platform_t radio0 = {
        .gpio_chip_enable    = raspi_gpio_chip_enable_nrf24l01,
        .delay_us            = raspi_delay_us_nrf24l01,
        .platform_init       = raspi_platform_init_nrf24l01,
        .platform_deinit     = raspi_platform_deinit_nrf24l01,
        .spi_exchange        = raspi_spi_exchange_data_nrf24l01,
        .check_for_interrupt = raspi_check_for_interrupt_nrf24l01,
        .user_ptr            = &(raspi_nrf24l01_t){
            .ce_pin  = RADIO0_CE,
            .irq_pin = RADIO0_IRQ,
            .cs      = BCM2835_SPI_CS0,
        },
    };
    nrf24l01_platform_t radio1 = {
        .gpio_chip_enable    = raspi_gpio_chip_enable_nrf24l01,
        .delay_us            = raspi_delay_us_nrf24l01,
        .platform_init       = raspi_platform_init_nrf24l01,
        .platform_deinit     = raspi_platform_deinit_nrf24l01,
        .spi_exchange        = raspi_spi_exchange_data_nrf24l01,
        .check_for_interrupt = raspi_check_for_interrupt_nrf24l01,
        .user_ptr            = &(raspi_nrf24l01_t){
            .ce_pin  = RADIO1_CE,
            .irq_pin = RADIO1_IRQ,
            .cs      = BCM2835_SPI_CS1,
        },
    };
    nrf24l01_err_t err = NRF24L01_OK;
    err |= nrf24l01_init(&radio0);
    //err |= nrf24l01_init(&device1);

    while (true) {

        printf("Checking connectivity...\n");
        //nrf24l01_err_t err = nrf24l01_set_address_width(5, &device0);
        err = nrf24l01_check_connectivity(&radio0);
        printf("Result: %d\n", err);

        bcm2835_delay(3000);
    }

    //Return SPI pins to default inputs state
    bcm2835_spi_end();

    bcm2835_close();
}

static int8_t raspi_gpio_chip_enable_nrf24l01(bool state, void* user_ptr) {
    raspi_nrf24l01_t* radio = (raspi_nrf24l01_t*)user_ptr;
    bcm2835_gpio_write(radio->ce_pin, state);
    return 0;
}
static int8_t raspi_delay_us_nrf24l01(uint32_t delay) {
    bcm2835_delayMicroseconds(delay);
    return 0;
}
static int8_t raspi_platform_init_nrf24l01(void* user_ptr) {
    raspi_nrf24l01_t* radio = (raspi_nrf24l01_t*)user_ptr;

    // Configure chip enable pin
    bcm2835_gpio_fsel(radio->ce_pin, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_set_pud(radio->ce_pin, BCM2835_GPIO_PUD_DOWN);

    // Configure host interrupt pin
    bcm2835_gpio_set_pud(radio->irq_pin, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_afen(radio->irq_pin);
    return 0;
}
static int8_t raspi_platform_deinit_nrf24l01(void* user_ptr) {
    raspi_nrf24l01_t* radio = (raspi_nrf24l01_t*)user_ptr;
    bcm2835_gpio_write(radio->ce_pin, LOW);
    bcm2835_gpio_set_pud(radio->ce_pin, BCM2835_GPIO_PUD_OFF);
    return 0;
}
static int8_t raspi_check_for_interrupt_nrf24l01(void* user_ptr) {
    raspi_nrf24l01_t* radio  = (raspi_nrf24l01_t*)user_ptr;
    uint8_t           result = bcm2835_gpio_eds(radio->irq_pin);
    if (result == 1) {
        bcm2835_gpio_set_eds(radio->irq_pin);
        return 0;
    } else {
        return -1;
    }
}
static int8_t raspi_spi_exchange_data_nrf24l01(uint8_t command, uint8_t* rx_data, uint8_t* tx_data, uint8_t len, void* user_ptr) {

#ifdef NRF24L01_INTEGRATION_TEST_DEBUG
    printf("\n********************SPI TRANSACTION START********************\n");
#endif // NRF24L01_INTEGRATION_TEST_DEBUG

#ifdef NRF24L01_INTEGRATION_TEST_DEBUG
    printf("Command = 0x%02X, Length = %d\n", command, len);
    if (tx_data != NULL) {
        printf("Write = [");
        for (int i = 0; i < len; i++) {
            printf("0x%02X, ", tx_data[i]);
        }
        printf("]\n");
    } else {
        printf("Write = NULL\n");
    }
#endif // NRF24L01_INTEGRATION_TEST_DEBUG

    if (rx_data == NULL && tx_data == NULL) {
        return -1; //Must provide either rx or tx memory
    } else if (user_ptr == NULL) {
        return -1; //Must provide the device location data
    }
    raspi_nrf24l01_t* radio = (raspi_nrf24l01_t*)user_ptr;

    // Set with CS pin to use for the upcoming transfer
    bcm2835_spi_chipSelect(radio->cs);

    uint8_t data_buffer[len + 1]; // VLA, requires C99
    data_buffer[0] = command;

    if (tx_data != NULL) {
        memcpy(&data_buffer[1], tx_data, len);
    } else {
        memset(&data_buffer[1], 0, len);
    }

    bcm2835_spi_transfern(&data_buffer[0], len + 1); //data_buffer used for tx and rx

    if (rx_data != NULL) {
        memcpy(rx_data, &data_buffer[1], len);
    }

#ifdef NRF24L01_INTEGRATION_TEST_DEBUG
    if (rx_data != NULL) {
        printf("Read = [");
        for (int i = 0; i < len; i++) {
            printf("0x%02X, ", rx_data[i]);
        }
        printf("]\n");
    } else {
        printf("Read = NULL\n");
    }
#endif // NRF24L01_INTEGRATION_TEST_DEBUG

#ifdef NRF24L01_INTEGRATION_TEST_DEBUG
    printf("*********************SPI TRANSACTION END*********************\n");
#endif // NRF24L01_INTEGRATION_TEST_DEBUG

    return 0;
}
