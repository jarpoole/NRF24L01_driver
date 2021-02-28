
/**
 * 
 *  @file
 *  @author  Jared Poole
 *  @version 1.1.7
 * 
 *  @section CONFIGURATION
 * 
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
 * 
 *   - NRF24L01_FPTR_RTN_T               used to specify the return type for the platform specific SPI functions
 * 								         If this option is not provided, a signed 8-bit integer type is assumed
 * 
 * 
 * 
 *  @section IMPORTANT TERMINOLOGY
 * 
 *  DPL - Dynamic payload length
 * 				enabled/disabled globally via a feature register
 *              when disabled, packet length determined per RX pipe via NRF24L01_RX_PW_Px registers
 * 				when enabled, packet length determined per packet via
 * 
 *  AA  - Automatic Acknowledge
 * 
 * 	AR  - Automatic Retransmit 
 * 
 *  DTA - Dynamic Transmit ACK
 */		 


#ifndef NRF24L01_DEFS_H
#define NRF24L01_DEFS_H

#ifndef NRF24L01_FPTR_RTN_T
	#warning "Plaform specific function pointer return type not specified, defaulting to int8_t"
	#define NRF24L01_FPTR_RTN_T int8_t
#endif

#ifdef NRF24L01_ENABLE_DEBUG_LOGGING
	#if !defined(NRF24L01_DEBUGGING_PRINTF) || !defined(NRF24L01_DEBUGGING_LOG)
		#error "NRF24L01 debug logging platform specific methods not provided"
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
	
	
/** @addtogroup commands
 * 	@brief NRF24L01 general command definitions
 * 	Format is <b>NRF24L01_CMD_x<b> where x is the name assigned to the command by the datasheet
 * 	@{
 */
#define NRF24L01_CMD_R_REGISTER           (uint8_t)0x00  ///< Register read 0b000XXXXX where XXXXX = 5 bit Register Map Address
#define NRF24L01_CMD_W_REGISTER           (uint8_t)0x20  ///< Register write 0b001XXXXX where XXXXX = 5 bit Register Map Address
#define NRF24L01_CMD_R_RX_PAYLOAD         (uint8_t)0x61  ///< Read RX payload
#define NRF24L01_CMD_W_TX_PAYLOAD         (uint8_t)0xA0  ///< Write TX payload
#define NRF24L01_CMD_FLUSH_TX             (uint8_t)0xE1  ///< Flush TX FIFO
#define NRF24L01_CMD_FLUSH_RX             (uint8_t)0xE2  ///< Flush RX FIFO
#define NRF24L01_CMD_REUSE_TX_PL          (uint8_t)0xE3  ///< Reuse TX payload
#define NRF24L01_CMD_ACTIVATE             (uint8_t)0x50  ///< (De)Activates R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK features
#define NRF24L01_CMD_NOP                  (uint8_t)0xFF  ///< No operation (used for reading status register for example)
// @}

/** @addtogroup commands
 * 	@brief NRF24L01 special feature command definitions
 * 	Format is <b>NRF24L01_FEATURE_CMD_x<b> where x is the name assigned to the command by the datasheet
 *  @{
 */
#define NRF24L01_FEATURE_CMD_R_RX_PL_WID	     (uint8_t)0x60  ///< Read RX-payload width for the top R_RX_PAYLOAD in the RX FIFO.
#define NRF24L01_FEATURE_CMD_W_ACK_PAYLOAD       (uint8_t)0xA8  ///< Write ACK payload 0b10101XXX where XXX = 3 bit pipe identifier
#define NRF24L01_FEATURE_CMD_W_TX_PAYLOAD_NOACK  (uint8_t)0xB0  ///< Write TX payload and disable AUTOACK
// @}



// ADDRESS: NRF24L01 register address definitions
#define NRF24L01_CONFIG_REG_ADDR          (uint8_t)0x00  ///< Configuration register address
#define NRF24L01_EN_AA_REG_ADDR           (uint8_t)0x01  ///< Enable "Auto acknowledgment" control register address
#define NRF24L01_EN_RX_ADDR_REG_ADDR      (uint8_t)0x02  ///< Enable RX addresses register address
#define NRF24L01_SETUP_AW_REG_ADDR        (uint8_t)0x03  ///< Setup of address widths register address
#define NRF24L01_SETUP_RETR_REG_ADDR      (uint8_t)0x04  ///< Setup of automatic retransmit register address
#define NRF24L01_RF_CH_REG_ADDR           (uint8_t)0x05  ///< RF channel register address
#define NRF24L01_RF_SETUP_REG_ADDR        (uint8_t)0x06  ///< RF setup register register address
#define NRF24L01_STATUS_REG_ADDR          (uint8_t)0x07  ///< Status register address
#define NRF24L01_OBSERVE_TX_REG_ADDR      (uint8_t)0x08  ///< Transmit observe register address
#define NRF24L01_RPD_REG_ADDR             (uint8_t)0x09  ///< Received power detector register address
#define NRF24L01_RX_ADDR_P0_REG_ADDR      (uint8_t)0x0A  ///< Receive address data pipe 0 register address
#define NRF24L01_RX_ADDR_P1_REG_ADDR      (uint8_t)0x0B  ///< Receive address data pipe 1 register address
#define NRF24L01_RX_ADDR_P2_REG_ADDR      (uint8_t)0x0C  ///< Receive address data pipe 2 register address
#define NRF24L01_RX_ADDR_P3_REG_ADDR      (uint8_t)0x0D  ///< Receive address data pipe 3 register address
#define NRF24L01_RX_ADDR_P4_REG_ADDR      (uint8_t)0x0E  ///< Receive address data pipe 4 register address
#define NRF24L01_RX_ADDR_P5_REG_ADDR      (uint8_t)0x0F  ///< Receive address data pipe 5 register address
#define NRF24L01_TX_ADDR_REG_ADDR         (uint8_t)0x10  ///< Transmit address register address
#define NRF24L01_RX_PW_P0_REG_ADDR        (uint8_t)0x11  ///< Number of bytes in RX payload in data pipe 0 register address
#define NRF24L01_RX_PW_P1_REG_ADDR        (uint8_t)0x12  ///< Number of bytes in RX payload in data pipe 1 register address
#define NRF24L01_RX_PW_P2_REG_ADDR        (uint8_t)0x13  ///< Number of bytes in RX payload in data pipe 2 register address
#define NRF24L01_RX_PW_P3_REG_ADDR        (uint8_t)0x14  ///< Number of bytes in RX payload in data pipe 3 register address
#define NRF24L01_RX_PW_P4_REG_ADDR        (uint8_t)0x15  ///< Number of bytes in RX payload in data pipe 4 register address
#define NRF24L01_RX_PW_P5_REG_ADDR        (uint8_t)0x16  ///< Number of bytes in RX payload in data pipe 5 register address
#define NRF24L01_FIFO_STATUS_REG_ADDR     (uint8_t)0x17  ///< FIFO status register address
#define NRF24L01_DYNPD_REG_ADDR           (uint8_t)0x1C  ///< Enable dynamic payload length register address
#define NRF24L01_FEATURE_REG_ADDR         (uint8_t)0x1D  ///< Feature register address
// Special definition for use with commands that don't require an address
#define NRF24L01_NO_REG_ADDR_REQUIRED     (uint8_t)0x00 

// Register bits definitions
#define NRF24L01_CONFIG_PRIM_RX           (uint8_t)0x01  ///< PRIM_RX bit in CONFIG register
#define NRF24L01_CONFIG_PWR_UP            (uint8_t)0x02  ///< PWR_UP bit in CONFIG register
#define NRF24L01_FEATURE_EN_DYN_ACK       (uint8_t)0x01  ///< EN_DYN_ACK bit in FEATURE register
#define NRF24L01_FEATURE_EN_ACK_PAY       (uint8_t)0x02  ///< EN_ACK_PAY bit in FEATURE register
#define NRF24L01_FEATURE_EN_DPL           (uint8_t)0x04  ///< EN_DPL bit in FEATURE register
#define NRF24L01_FLAG_RX_DR               (uint8_t)0x40  ///< RX_DR bit (data ready RX FIFO interrupt)
#define NRF24L01_FLAG_TX_DS               (uint8_t)0x20  ///< TX_DS bit (data sent TX FIFO interrupt)
#define NRF24L01_FLAG_MAX_RT              (uint8_t)0x10  ///< MAX_RT bit (maximum number of TX retransmits interrupt)
#define	NRF24L01_ADDR_WIDTH_INVALID       (uint8_t)0x00  ///< Invalid address field width
#define	NRF24L01_ADDR_WIDTH_3_BYTES       (uint8_t)0x01  ///< 3 Byte RX/TX address field width
#define	NRF24L01_ADDR_WIDTH_4_BYTES       (uint8_t)0x02  ///< 4 Byte RX/TX address field width
#define	NRF24L01_ADDR_WIDTH_5_BYTES       (uint8_t)0x03  ///< 5 Byte RX/TX address field width
 
// Register masks definitions 
#define NRF24L01_MASK_REG_MAP             (uint8_t)0x1F  ///< Mask for [4:0] for CMD_RREG and CMD_WREG commands

#define NRF24L01_MASK_STATUS_IRQ          (uint8_t)0x70  ///< Mask for IRQ[6:4] interrupt flag bits in STATUS register
#define NRF24L01_MASK_STATUS_RX_DR        (uint8_t)0x40  ///< Mask for RX_DR[6] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_TX_DS        (uint8_t)0x20  ///< Mask for TX_DS[5] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_MAX_RT       (uint8_t)0x10  ///< Mask for MAX_RT[4] interrupt flag bit in STATUS register
#define NRF24L01_MASK_STATUS_RX_P_NO      (uint8_t)0x0E  ///< Mask for RX_P_NO[3:1] pipe number bits in STATUS register
#define NRF24L01_MASK_STATUS_TX_FULL      (uint8_t)0x01  ///< Mask for TX_FULL[0] flag bit in STATUS register

#define NRF24L01_MASK_OBSERVE_TX_PLOS_CNT (uint8_t)0xF0  ///< Mask for PLOS_CNT[7:4] lost packet count bits
#define NRF24L01_MASK_OBSERVE_TX_ARC_CNT  (uint8_t)0x0F  ///< Mask for ARC_CNT[3:0] resent packet count bits

#define NRF24L01_MASK_CONFIG_INTERRUPTS   (uint8_t)0x70  ///< Mask for INTERRUPT[6:4] bits in CONIF register
#define NRF24L01_MASK_CONFIG_CRC          (uint8_t)0x0C  ///< Mask for CRC[3:2] bits in CONFIG register
#define NRF24L01_MASK_RF_PWR              (uint8_t)0x06  ///< Mask for RF_PWR[2:1] bits in RF_SETUP register
#define NRF24L01_MASK_DATARATE            (uint8_t)0x08  ///< Mask for RD_DR[4] bit in RF_SETUP register
#define NRF24L01_MASK_RETR_ARD            (uint8_t)0xF0  ///< Mask for ARD[7:4] bits in SETUP_RETR register
#define NRF24L01_MASK_RETR_ARC            (uint8_t)0x0F  ///< Mask for ARC[3:0] bits in SETUP_RETR register

#define NRF24L01_MASK_TX_FIFO_STATUS      (uint8_t)0x70  ///< Mask for TX_FIFO[6:4] status bits in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_REUSE       (uint8_t)0x40  ///< Mask for TX_REUSE[5] status bit in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_FULL        (uint8_t)0x20  ///< Mask for TX_FULL[6] status bit in FIFO_STATUS register
#define NRF24L01_MASK_TX_FIFO_EMPTY       (uint8_t)0x10  ///< Mask for TX_EMPTY[4] status bits [5:4] in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_STATUS      (uint8_t)0x03  ///< Mask for RX_FIFO[1:0] status bits in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_FULL        (uint8_t)0x02  ///< Mask for RX_FULL[1] status bit in FIFO_STATUS register
#define NRF24L01_MASK_RX_FIFO_EMPTY       (uint8_t)0x01  ///< Mask for RX_EMPTY[0] status bit in FIFO_STATUS register

#define NRF24L01_MASK_PLOS_CNT            (uint8_t)0xF0  ///< Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
#define NRF24L01_MASK_ARC_CNT             (uint8_t)0x0F  ///< Mask for ARC_CNT[3:0] bits in OBSERVE_TX register
#define NRF24L01_MASK_ADDR_WIDTH          (uint8_t)0x03  ///< Mask for AW[1:0] bits in SETUP_AW register

#define NRF24L01_MASK_DYNPD               (uint8_t)0x3F  ///< Mask for DPL_Px[5:0] bits in DYNPD feature register 
#define NRF24L01_MASK_EN_AA               (uint8_t)0x3F  ///< Mask for ENAA_Px[5:0] bits in EN_AA register 
#define NRF24L01_MASK_EN_RX_ADDR          (uint8_t)0x3F  ///< Mask for ERX_Px[5:0] bits in EN_RXADDR register 


// Timing requirements
#define NRF24L01_CE_TX_MINIMUM_PULSE_US   10   // In microseconds
#define NRF24L01_POWER_UP_US              5000 // In microseconds

// Sizes
#define NRF24L01_TX_FIFO_WIDTH            32  ///< Each TX FIFO entry is 32 bytes wide
#define NRF24L01_TX_FIFO_DEPTH            3   ///< The TX FIFO is 3 entries deep
#define NRF24L01_RX_FIFO_WIDTH            32  ///< Each TX FIFO entry is 32 bytes wide
#define NRF24L01_RX_FIFO_DEPTH            3   ///< The RX FIFO is 3 entries deep

// Magic numbers
#define NRF24L01_MAGIC_NUMBER_ACTIVATE    (uint8_t)0x73


// Must forward declare nrf24l01_platform_t struct as it will be used throughout the following definitions
typedef struct nrf24l01_platform_t nrf24l01_platform_t;

// Retransmit delay
typedef enum {
	NRF24L01_ARD_NONE   = (uint8_t)0x00,  ///< Dummy value for case when retransmission is not used
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
} nrf24l01_ar_delay_t;

// Flags used to mask out interrupt sources. Should be combined with boolean OR
enum {
	RX_DR_INTERRUPT_ENABLE   = (uint8_t)0x00,
	RX_DR_INTERRUPT_DISABLE  = (uint8_t)0x40,
	TX_DS_INTERRUPT_ENABLE   = (uint8_t)0x00,
	TX_DS_INTERRUPT_DISABLE  = (uint8_t)0x20,
	MAX_RT_INTERRUPT_ENABLE  = (uint8_t)0x00,
	MAX_RT_INTERRUPT_DISABLE = (uint8_t)0x10,
};
typedef uint8_t nrf24l01_interrupt_mask_t;

// Retransmit count
typedef uint8_t nrf24l01_ar_count_t;

// Number of lost autoretransmit packets
typedef uint8_t nrf24l01_ar_lost_t;

// Data rate
typedef enum {
	NRF24L01_DR_250KBPS  = (uint8_t)0x20,  ///< 250kbps data rate
	NRF24L01_DR_1MBPS    = (uint8_t)0x00,  ///< 1Mbps data rate
	NRF24L01_DR_2MBPS    = (uint8_t)0x08,  ///< 2Mbps data rate
} nrf24l01_data_rate_t;

// RF output power in TX mode
typedef enum {
	NRF24L01_TXPWR_18DBM = (uint8_t)0x00,  ///< -18dBm
	NRF24L01_TXPWR_12DBM = (uint8_t)0x02,  ///< -12dBm
	NRF24L01_TXPWR_6DBM  = (uint8_t)0x04,  ///< -6dBm
	NRF24L01_TXPWR_0DBM  = (uint8_t)0x06,  ///<  0dBm
} nrf24l01_tx_power_t;

// CRC encoding scheme
typedef enum {
	NRF24L01_CRC_OFF   = (uint8_t)0x00,  ///< CRC disabled
	NRF24L01_CRC_1BYTE = (uint8_t)0x08,  ///< 1-byte CRC
	NRF24L01_CRC_2BYTE = (uint8_t)0x0c,  ///< 2-byte CRC
} nrf24l01_crc_scheme_t;

// RF channel number
typedef uint8_t nrf24l01_rf_channel_t;

// pipe address width
typedef uint8_t nrf24l01_address_width_t;

// Status of the RX/TX FIFOs
typedef enum {
	NRF24L01_STATUS_FIFO_DATA  = (uint8_t)0x00,  ///< The FIFO contains data and available locations
	NRF24L01_STATUS_FIFO_EMPTY = (uint8_t)0x01,  ///< The FIFO is empty
	NRF24L01_STATUS_FIFO_FULL  = (uint8_t)0x02,  ///< The FIFO is full
	NRF24L01_STATUS_FIFO_ERROR = (uint8_t)0x03,  ///< Impossible state: FIFO cannot be empty and full at the same time
} nrf24l01_fifo_status_t;

// Type of FIFO
typedef enum {
	NRF24L01_RX_FIFO  = (uint8_t)0x00,  ///< RX FIFO
	NRF24L01_TX_FIFO  = (uint8_t)0x01,  ///< TX_FIFO
} nrf24l01_fifo_type_t;

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

// Enumeration of RX pipe addresses and TX address
typedef enum {
	NRF24L01_PIPE0        = (uint8_t)0x00,  ///< pipe0
	NRF24L01_PIPE1        = (uint8_t)0x01,  ///< pipe1
	NRF24L01_PIPE2        = (uint8_t)0x02,  ///< pipe2
	NRF24L01_PIPE3        = (uint8_t)0x03,  ///< pipe3
	NRF24L01_PIPE4        = (uint8_t)0x04,  ///< pipe4
	NRF24L01_PIPE5        = (uint8_t)0x05,  ///< pipe5
	NRF24L01_PIPETX       = (uint8_t)0x06,  ///< TX address (not a pipe in fact)
	NRF24L01_PIPE_UNKNOWN = (uint8_t)0x07,  ///< pipe unknown (usually signifies rx pipe is empty)
	NRF24L01_ALL_RX_PIPES,                  ///< Used for configuring all RX pipes at the same time
} nrf24l01_pipe_t;

#define NRF24L01_IS_RX_PIPE(pipe)    ( ((pipe) >= NRF24L01_PIPE0 && (pipe) <= NRF24L01_PIPE5) || ((pipe) == NRF24L01_ALL_RX_PIPES) )
#define NRF24L01_IS_TX_PIPE(pipe)    ( (pipe) == NRF24L01_PIPETX )



// nRF24L01 power control
typedef enum {
	NRF24L01_PWR_DOWN = (uint8_t)0x00,  ///< Power down
	NRF24L01_PWR_UP   = (uint8_t)0x02,  ///< Power up
} nrf24l01_power_mode_t;

// Transceiver mode
typedef enum {
	NRF24L01_MODE_RX = (uint8_t)0x01,  ///< PRX
	NRF24L01_MODE_TX = (uint8_t)0x00,  ///< PTX
} nrf24l01_operational_mode_t;

// Describes if pipe is active or not
typedef enum {
	NRF24L01_PIPE_DISABLED,
	NRF24L01_PIPE_ENABLED,
} nrf24l01_pipe_mode_t;

// State of auto acknowledgment for specified pipe
typedef enum {
	NRF24L01_AA_OFF = (uint8_t)0x00,
	NRF24L01_AA_ON  = (uint8_t)0x01,
} nrf24l01_pipe_aa_mode_t;

// State of the NRF24L01 special features
typedef enum {
	NRF24L01_FEATURES_OFF = (uint8_t)0x00,
	NRF24L01_FEATURES_ON  = (uint8_t)0x01,
} nrf24l01_feature_mode_t;

typedef enum {
	NRF24L01_DPL_OFF = (uint8_t)0x00,
	NRF24L01_DPL_ON  = (uint8_t)0x01,
} nrf24l01_dpl_mode_t;

typedef enum {
	NRF24L01_DTA_OFF = (uint8_t)0x00, 
	NRF24L01_DTA_ON  = (uint8_t)0x01,
} nrf24l01_dta_mode_t;

typedef enum {
	NRF24L01_ACK_PAYLOAD_OFF = (uint8_t)0x00, 
	NRF24L01_ACK_PAYLOAD_ON  = (uint8_t)0x01,
} ack_payload_mode_t;


/*!
 * @brief Hardware initialization function pointer which should be used to configure the
 * hardware before communication is attempted
 * 
 * IRQ pin is active-low. Interrupt pin should be triggered on negative-edge. Pull-up resistor needed
 *
 * @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_platform_init_fptr_t)(void* user_ptr);


/*!
 * @brief SPI Bus deinitialization function pointer which should be used to release the
 * hardware when the driver is deinitialized
 *
 * @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_platform_deinit_fptr_t)(void* user_ptr);


/*!
 * @brief SPI Bus exchange function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in] command      : NRF24L01-specific command byte which should be sent 
 *                           before rx_data and/or tx_data
 * @param[in] rx_data      : Pointer to data buffer into which data which will be
 *                           read. Implementation must handle the case where this
 *                           value is NULL. When non-NULL, this buffer must have a
 * 							 size of 'len'
 * @param[in] tx_data      : Pointer to data buffer in which data to be written
 *                           is stored. Implementation must handle the case where this
 *                           value is NULL. When non-NULL, this buffer must have a
 * 							 size of 'len'
 * @param[in] len          : Number of bytes of data to be read/written.
 * @param[in] user_ptr     : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_spi_exchange_fptr_t)(uint8_t command, uint8_t* rx_data, uint8_t* tx_data, uint8_t len, void* user_ptr);


/*!
 * @brief Set the state of Chip enable GPIO function pointer
 *
 * @param[in] state         : Desired state of the chip enable GPIO
 * @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_gpio_chip_enable_fptr_t)(bool state, void* user_ptr);


/*!
 * @brief Microsecond delay function pointer which should be mapped to
 * the platform specific delay function of the user
 *
 * @param[in] delay      : Number of microseconds to delay
 *
 * @retval 0        -> Success
 * @retval Non zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_delay_us_fptr_t)(uint32_t delay);


/*!
 * @brief Check for interrupt function pointer
 *
 * @param[in] delay      : Number of microseconds to delay
 *
 * @retval 0        -> Interrupt triggered and waiting to be processed
 * @retval Non zero -> No interrupt
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_check_for_interrupt_fptr_t)(void* user_ptr);





/*!
 * @brief User supplied callback function pointer
 *
 * @param[in] delay      : Number of microseconds to delay
 * 
 */
typedef void (*nrf24l01_rx_dr_callback_fptr_t)(uint8_t message_len, nrf24l01_pipe_t pipe, void* user_ptr, nrf24l01_platform_t* platform);


typedef void (*nrf24l01_tx_ds_callback_fptr_t)(nrf24l01_pipe_t pipe, void* user_ptr, nrf24l01_platform_t* platform);


typedef void (*nrf24l01_max_rt_callback_fptr_t)(void* user_ptr, nrf24l01_platform_t* platform);




struct nrf24l01_platform_t{
	nrf24l01_delay_us_fptr_t            delay_us;            ///< Pointer to a platform specific microsecond delay function
	nrf24l01_gpio_chip_enable_fptr_t    gpio_chip_enable;    ///< Pointer to the platform specific GPIO control function
	nrf24l01_platform_init_fptr_t       platform_init;       ///< Pointer to the platform specific hardware initialization function
	nrf24l01_platform_deinit_fptr_t     platform_deinit;     ///< Pointer to the platform specific hardware deinitialization function
	nrf24l01_spi_exchange_fptr_t        spi_exchange;        ///< Pointer to the platform specific SPI full-duplex transfer function
	nrf24l01_check_for_interrupt_fptr_t check_for_interrupt; ///< Pointer to the platform specific
	struct {
		nrf24l01_rx_dr_callback_fptr_t  rx_dr_callback;
		void*                           rx_dr_callback_user_ptr;
		nrf24l01_tx_ds_callback_fptr_t  tx_ds_callback;
		void*                           tx_ds_callback_user_ptr;
		nrf24l01_max_rt_callback_fptr_t max_rt_callback;
		void*                           max_rt_callback_user_ptr;
	} callbacks;
	void*                               user_ptr;            ///< (optional) Pointer to a user-defined hardware configuration struct
};

#endif



