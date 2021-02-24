
/**
 *  User options are exposed through macro definitions provided in the user supplied "nrf24l01_config.h" file
 *  in the parent directory
 * 
 *  Macro options
 *   - NRF24L01_ENABLE_DEBUG_LOGGING     can be defined to enable driver debug logging
 *   - NRF24L01_DEBUGGING_LOG            must be defined if debug logging is enabled to provide platform specific 
 *                                       method for timestamped, tagged, message logging
 *       Ex: #define NRF24L01_DEBUGGING_LOG( ... )   log(__VA_ARGS__)
 * 
 * 	 - NRF24L01_DEBUGGING_PRINTF         must be defined if debug logging is enabled to provide platform specific 
 *                                       UART interface method
 *       Ex: #define NRF24L01_DEBUGGING_PRINTF( ... )   printf(__VA_ARGS__)
 */		 

#ifndef NRF24L01_DEFS_H
#define NRF24L01_DEFS_H

#ifndef NRF24L01_SPI_FPTR_RTN_T
	#define NRF24L01_SPI_FPTR_RTN_T int8_t
#endif

#ifdef NRF24L01_ENABLE_DEBUG_LOGGING
	#ifndef NRF24L01_DEBUGGING_PRINTF
		#error "NRF24L01 debug logging platform specific method not provided"
	#else
		#define NRF24L01_ENABLE_PRINT_CONFIG

		/*****************************Define helpers for printing registers in binary************************************/
		//Format strings for use with printf style functions
		#define _1BIT_FMT "0b%c"
		#define _2BIT_FMT "0b%c%c"
		#define _3BIT_FMT "0b%c%c%c"
		#define _4BIT_FMT "0b%c%c%c%c"
		#define _5BIT_FMT "0b%c%c%c%c%c"
		#define _6BIT_FMT "0b%c%c%c%c%c%c"
		#define _7BIT_FMT "0b%c%c%c%c%c%c%c"
		#define _8BIT_FMT "0b%c%c%c%c%c%c%c%c"
		//Helper for expanding a single bit to a character
		#define BIT2STR(val,bit)  ((val)&(1<<(bit)))?'1':'0'
		//Macros for expanding a byte into a string of bits without dynamic memory allocation
		#define _1BIT_STR(val)  BIT2STR((val),0)
		#define _2BIT_STR(val)  BIT2STR((val),1),BIT2STR((val),0)
		#define _3BIT_STR(val)  BIT2STR((val),2),BIT2STR((val),1),BIT2STR((val),0)
		#define _4BIT_STR(val)  BIT2STR((val),3),BIT2STR((val),2),BIT2STR((val),1),BIT2STR((val),0)
		#define _5BIT_STR(val)  BIT2STR((val),4),BIT2STR((val),3),BIT2STR((val),2),BIT2STR((val),1), \
								BIT2STR((val),0)
		#define _6BIT_STR(val)  BIT2STR((val),5),BIT2STR((val),4),BIT2STR((val),3),BIT2STR((val),2), \
		                        BIT2STR((val),1),BIT2STR((val),0)
		#define _7BIT_STR(val)  BIT2STR((val),6),BIT2STR((val),5),BIT2STR((val),4),BIT2STR((val),3), \
		                        BIT2STR((val),2),BIT2STR((val),1),BIT2STR((val),0)
		#define _8BIT_STR(val)  BIT2STR((val),7),BIT2STR((val),6),BIT2STR((val),5),BIT2STR((val),4), \
		                        BIT2STR((val),3),BIT2STR((val),2),BIT2STR((val),1),BIT2STR((val),0)
		/****************************************************************************************************************/

	#endif
#endif
	
// COMMAND: NRF24L01 instruction definitions
#define NRF24L01_CMD_R_REGISTER         (uint8_t)0x00 // Register read
#define NRF24L01_CMD_W_REGISTER         (uint8_t)0x20 // Register write
#define NRF24L01_CMD_ACTIVATE           (uint8_t)0x50 // (De)Activates R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK features
#define NRF24L01_CMD_R_RX_PL_WID	    (uint8_t)0x60 // Read RX-payload width for the top R_RX_PAYLOAD in the RX FIFO.
#define NRF24L01_CMD_R_RX_PAYLOAD       (uint8_t)0x61 // Read RX payload
#define NRF24L01_CMD_W_TX_PAYLOAD       (uint8_t)0xA0 // Write TX payload
#define NRF24L01_CMD_W_ACK_PAYLOAD      (uint8_t)0xA8 // Write ACK payload
#define NRF24L01_CMD_W_TX_PAYLOAD_NOACK (uint8_t)0xB0 // Write TX payload and disable AUTOACK
#define NRF24L01_CMD_FLUSH_TX           (uint8_t)0xE1 // Flush TX FIFO
#define NRF24L01_CMD_FLUSH_RX           (uint8_t)0xE2 // Flush RX FIFO
#define NRF24L01_CMD_REUSE_TX_PL        (uint8_t)0xE3 // Reuse TX payload
#define NRF24L01_CMD_LOCK_UNLOCK        (uint8_t)0x50 // Lock/unlock exclusive features
// No operation (used for reading status register for example)
#define NRF24L01_CMD_NOP                (uint8_t)0xFF

// ADDRESS: NRF24L01 register address definitions
#define NRF24L01_CONFIG_REG_ADDR        (uint8_t)0x00 // Configuration register address
#define NRF24L01_EN_AA_REG_ADDR         (uint8_t)0x01 // Enable "Auto acknowledgment" control register address
#define NRF24L01_EN_RX_ADDR_REG_ADDR    (uint8_t)0x02 // Enable RX addresses register address
#define NRF24L01_SETUP_AW_REG_ADDR      (uint8_t)0x03 // Setup of address widths register address
#define NRF24L01_SETUP_RETR_REG_ADDR    (uint8_t)0x04 // Setup of automatic retransmit register address
#define NRF24L01_RF_CH_REG_ADDR         (uint8_t)0x05 // RF channel register address
#define NRF24L01_RF_SETUP_REG_ADDR      (uint8_t)0x06 // RF setup register register address
#define NRF24L01_STATUS_REG_ADDR        (uint8_t)0x07 // Status register address
#define NRF24L01_OBSERVE_TX_REG_ADDR    (uint8_t)0x08 // Transmit observe register address
#define NRF24L01_RPD_REG_ADDR           (uint8_t)0x09 // Received power detector register address
#define NRF24L01_RX_ADDR_P0_REG_ADDR    (uint8_t)0x0A // Receive address data pipe 0 register address
#define NRF24L01_RX_ADDR_P1_REG_ADDR    (uint8_t)0x0B // Receive address data pipe 1 register address
#define NRF24L01_RX_ADDR_P2_REG_ADDR    (uint8_t)0x0C // Receive address data pipe 2 register address
#define NRF24L01_RX_ADDR_P3_REG_ADDR    (uint8_t)0x0D // Receive address data pipe 3 register address
#define NRF24L01_RX_ADDR_P4_REG_ADDR    (uint8_t)0x0E // Receive address data pipe 4 register address
#define NRF24L01_RX_ADDR_P5_REG_ADDR    (uint8_t)0x0F // Receive address data pipe 5 register address
#define NRF24L01_TX_ADDR_REG_ADDR       (uint8_t)0x10 // Transmit address register address
#define NRF24L01_RX_PW_P0_REG_ADDR      (uint8_t)0x11 // Number of bytes in RX payload in data pipe 0 register address
#define NRF24L01_RX_PW_P1_REG_ADDR      (uint8_t)0x12 // Number of bytes in RX payload in data pipe 1 register address
#define NRF24L01_RX_PW_P2_REG_ADDR      (uint8_t)0x13 // Number of bytes in RX payload in data pipe 2 register address
#define NRF24L01_RX_PW_P3_REG_ADDR      (uint8_t)0x14 // Number of bytes in RX payload in data pipe 3 register address
#define NRF24L01_RX_PW_P4_REG_ADDR      (uint8_t)0x15 // Number of bytes in RX payload in data pipe 4 register address
#define NRF24L01_RX_PW_P5_REG_ADDR      (uint8_t)0x16 // Number of bytes in RX payload in data pipe 5 register address
#define NRF24L01_FIFO_STATUS_REG_ADDR   (uint8_t)0x17 // FIFO status register address
#define NRF24L01_DYNPD_REG_ADDR         (uint8_t)0x1C // Enable dynamic payload length register address
#define NRF24L01_FEATURE_REG_ADDR       (uint8_t)0x1D // Feature register address
// Special definition for use with commands that don't require an address
#define NRF24L01_NO_REG_ADDR_REQUIRED   (uint8_t)0x00 

// Register bits definitions
#define NRF24L01_CONFIG_PRIM_RX         (uint8_t)0x01 // PRIM_RX bit in CONFIG register
#define NRF24L01_CONFIG_PWR_UP          (uint8_t)0x02 // PWR_UP bit in CONFIG register
#define NRF24L01_FEATURE_EN_DYN_ACK     (uint8_t)0x01 // EN_DYN_ACK bit in FEATURE register
#define NRF24L01_FEATURE_EN_ACK_PAY     (uint8_t)0x02 // EN_ACK_PAY bit in FEATURE register
#define NRF24L01_FEATURE_EN_DPL         (uint8_t)0x04 // EN_DPL bit in FEATURE register
#define NRF24L01_FLAG_RX_DR             (uint8_t)0x40 // RX_DR bit (data ready RX FIFO interrupt)
#define NRF24L01_FLAG_TX_DS             (uint8_t)0x20 // TX_DS bit (data sent TX FIFO interrupt)
#define NRF24L01_FLAG_MAX_RT            (uint8_t)0x10 // MAX_RT bit (maximum number of TX retransmits interrupt)

// Register masks definitions
#define NRF24L01_MASK_STATUS_IRQ        (uint8_t)0x70 // Mask for IRQ[6:4] interrupt flag bits in STATUS register
#define NRF24L01_MASK_STATUS_RX_DR      (uint8_t)0x40 // Mask for RX_DR[6] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_TX_DS      (uint8_t)0x20 // Mask for TX_DS[5] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_MAX_RT     (uint8_t)0x10 // Mask for MAX_RT[4] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_RX_P_NO    (uint8_t)0x0E // Mask for RX_P_NO[3:1] pipe number bits in STATUS register
#define NRF24L01_MASK_STATUS_TX_FULL    (uint8_t)0x01 // Mask for TX_FULL[0] flag bit in STATUS register

#define NRF24L01_MASK_REG_MAP           (uint8_t)0x1F // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
#define NRF24L01_MASK_CRC               (uint8_t)0x0C // Mask for CRC bits [3:2] in CONFIG register
#define NRF24L01_MASK_RF_PWR            (uint8_t)0x06 // Mask RF_PWR[2:1] bits in RF_SETUP register
#define NRF24L01_MASK_DATARATE          (uint8_t)0x28 // Mask RD_DR_[5,3] bits in RF_SETUP register
#define NRF24L01_MASK_EN_RX             (uint8_t)0x3F // Mask ERX_P[5:0] bits in EN_RXADDR register
#define NRF24L01_MASK_RX_PW             (uint8_t)0x3F // Mask [5:0] bits in RX_PW_Px register
#define NRF24L01_MASK_RETR_ARD          (uint8_t)0xF0 // Mask for ARD[7:4] bits in SETUP_RETR register
#define NRF24L01_MASK_RETR_ARC          (uint8_t)0x0F // Mask for ARC[3:0] bits in SETUP_RETR register

#define NRF24L01_MASK_TX_FIFO_STATUS    (uint8_t)0x70 // Mask for TX_FIFO[6:4] status bits in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_REUSE     (uint8_t)0x40 // Mask for TX_REUSE[5] status bit in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_FULL      (uint8_t)0x20 // Mask for TX_FULL[6] status bit in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_EMPTY     (uint8_t)0x10 // Mask for TX_EMPTY[4] status bits [5:4] in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_STATUS    (uint8_t)0x03 // Mask for RX_FIFO[1:0] status bits in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_FULL      (uint8_t)0x02 // Mask for RX_FULL[1] status bit in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_EMPTY     (uint8_t)0x01 // Mask for RX_EMPTY[0] status bit in FIFO_STATUS register

#define NRF24L01_MASK_PLOS_CNT          (uint8_t)0xF0 // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
#define NRF24L01_MASK_ARC_CNT           (uint8_t)0x0F // Mask for ARC_CNT[3:0] bits in OBSERVE_TX register
#define NRF24L01_MASK_ADDR_WIDTH        (uint8_t)0x03 // Mask for AW[1:0] bits in SETUP_AW register

// Timing requirements
#define NRF24L01_CE_TX_MINIMUM_PULSE_US 10   // In microseconds
#define NRF24L01_POWER_UP_US            5000 // In microseconds

// Sizes
#define NRF24L01_TX_FIFO_LENGTH 32


// Retransmit delay
enum {
	NRF24L01_ARD_NONE   = (uint8_t)0x00, // Dummy value for case when retransmission is not used
	NRF24L01_ARD_250US  = (uint8_t)0x00,
	NRF24L01_ARD_500US  = (uint8_t)0x01,
	NRF24L01_ARD_750US  = (uint8_t)0x02,
	NRF24L01_ARD_1000US = (uint8_t)0x03,
	NRF24L01_ARD_1250US = (uint8_t)0x04,
	NRF24L01_ARD_1500US = (uint8_t)0x05,
	NRF24L01_ARD_1750US = (uint8_t)0x06,
	NRF24L01_ARD_2000US = (uint8_t)0x07,
	NRF24L01_ARD_2250US = (uint8_t)0x08,
	NRF24L01_ARD_2500US = (uint8_t)0x09,
	NRF24L01_ARD_2750US = (uint8_t)0x0A,
	NRF24L01_ARD_3000US = (uint8_t)0x0B,
	NRF24L01_ARD_3250US = (uint8_t)0x0C,
	NRF24L01_ARD_3500US = (uint8_t)0x0D,
	NRF24L01_ARD_3750US = (uint8_t)0x0E,
	NRF24L01_ARD_4000US = (uint8_t)0x0F,
};

// Data rate
enum {
	NRF24L01_DR_250KBPS = (uint8_t)0x20, // 250kbps data rate
	NRF24L01_DR_1MBPS   = (uint8_t)0x00, // 1Mbps data rate
	NRF24L01_DR_2MBPS   = (uint8_t)0x08, // 2Mbps data rate
};

// RF output power in TX mode
enum {
	NRF24L01_TXPWR_18DBM = (uint8_t)0x00, // -18dBm
	NRF24L01_TXPWR_12DBM = (uint8_t)0x02, // -12dBm
	NRF24L01_TXPWR_6DBM  = (uint8_t)0x04, //  -6dBm
	NRF24L01_TXPWR_0DBM  = (uint8_t)0x06, //   0dBm
};

// CRC encoding scheme
enum {
	NRF24L01_CRC_OFF   = (uint8_t)0x00, // CRC disabled
	NRF24L01_CRC_1BYTE = (uint8_t)0x08, // 1-byte CRC
	NRF24L01_CRC_2BYTE = (uint8_t)0x0c, // 2-byte CRC
};

// nRF24L01 power control
enum {
	NRF24L01_PWR_UP   = (uint8_t)0x02, // Power up
	NRF24L01_PWR_DOWN = (uint8_t)0x00, // Power down
};

// 
enum {
	NRF24L01_ADDR_WIDTH_INVALID = 0b00, // Invalid address field width
	NRF24L01_ADDR_WIDTH_3_BYTES = 0b01, // 3 Byte RX/TX address field width
	NRF24L01_ADDR_WIDTH_4_BYTES = 0b10, // 4 Byte RX/TX address field width
	NRF24L01_ADDR_WIDTH_5_BYTES = 0b11, // 5 Byte RX/TX address field width
};

// Transceiver mode
enum {
	NRF24L01_MODE_RX = (uint8_t)0x01, // PRX
	NRF24L01_MODE_TX = (uint8_t)0x00, // PTX
};

enum {
	NRF24L01_DPL_ON  = (uint8_t)0x01, // PRX
	NRF24L01_DPL_OFF = (uint8_t)0x00, // PTX
} ;

// Enumeration of RX pipe addresses and TX address
enum {
	NRF24L01_PIPE0  = (uint8_t)0x00, // pipe0
	NRF24L01_PIPE1  = (uint8_t)0x01, // pipe1
	NRF24L01_PIPE2  = (uint8_t)0x02, // pipe2
	NRF24L01_PIPE3  = (uint8_t)0x03, // pipe3
	NRF24L01_PIPE4  = (uint8_t)0x04, // pipe4
	NRF24L01_PIPE5  = (uint8_t)0x05, // pipe5
	NRF24L01_PIPETX = (uint8_t)0x06, // TX address (not a pipe in fact)
};

// State of auto acknowledgment for specified pipe
enum {
	NRF24L01_AA_OFF = (uint8_t)0x00,
	NRF24L01_AA_ON  = (uint8_t)0x01,
};

// Status of the RX FIFO
enum {
	NRF24L01_STATUS_RXFIFO_DATA  = (uint8_t)0x00, // The RX FIFO contains data and available locations
	NRF24L01_STATUS_RXFIFO_EMPTY = (uint8_t)0x01, // The RX FIFO is empty
	NRF24L01_STATUS_RXFIFO_FULL  = (uint8_t)0x02, // The RX FIFO is full
	NRF24L01_STATUS_RXFIFO_ERROR = (uint8_t)0x03, // Impossible state: RX FIFO cannot be empty and full at the same time
};

// Status of the TX FIFO
enum {
	NRF24L01_STATUS_TXFIFO_DATA  = (uint8_t)0x00, // The TX FIFO contains data and available locations
	NRF24L01_STATUS_TXFIFO_EMPTY = (uint8_t)0x01, // The TX FIFO is empty
	NRF24L01_STATUS_TXFIFO_FULL  = (uint8_t)0x02, // The TX FIFO is full
	NRF24L01_STATUS_TXFIFO_ERROR = (uint8_t)0x03, // Impossible state: TX FIFO cannot be empty and full at the same time
};

// Result of RX FIFO reading
typedef enum {
	NRF24L01_RX_PIPE0  = (uint8_t)0x00, // Packet received from the PIPE#0
	NRF24L01_RX_PIPE1  = (uint8_t)0x01, // Packet received from the PIPE#1
	NRF24L01_RX_PIPE2  = (uint8_t)0x02, // Packet received from the PIPE#2
	NRF24L01_RX_PIPE3  = (uint8_t)0x03, // Packet received from the PIPE#3
	NRF24L01_RX_PIPE4  = (uint8_t)0x04, // Packet received from the PIPE#4
	NRF24L01_RX_PIPE5  = (uint8_t)0x05, // Packet received from the PIPE#5
	NRF24L01_RX_EMPTY  = (uint8_t)0xff, // The RX FIFO is empty
} nrf24l01_rx_result;

// Error codes for NRF24L01
typedef enum {
    NRF24L01_OK = 0,
    NRF24L01_ERR_UNKNOWN,
    NRF24L01_ERR_INVALID_ARG,
	NRF24L01_ERR_INVALID_STATE,
    NRF24L01_ERR_DEVICE_NOT_FOUND,
    NRF24L01_ERR_WRITE,
    NRF24L01_ERR_READ,
} nrf24l01_err_t;


typedef NRF24L01_FPTR_RTN_T (*nrf24l01_spi_init_fptr_t)(void*);
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_spi_deinit_fptr_t)(void*);
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_spi_exchange_fptr_t)(uint8_t, uint8_t*, uint8_t*, size_t, void*);
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_gpio_chip_enable_fptr_t)(bool);
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_delay_us_fptr_t)(uint32_t);

typedef struct{
	nrf24l01_delay_us_fptr_t         delay_us;
	nrf24l01_gpio_chip_enable_fptr_t gpio_chip_enable;
	nrf24l01_spi_init_fptr_t         spi_init;
	nrf24l01_spi_deinit_fptr_t       spi_deinit;
	nrf24l01_spi_exchange_fptr_t     spi_exchange;
	void*                            spi_user_config;
} nrf24l01_platform_t;

#endif