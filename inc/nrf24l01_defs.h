
/**
 * 
 *  @file
 *  @author  Jared Poole
 *  @version 1.1.7
 * 
 *  @mainpage NRF24L01 driver documentation
 * 
 *  @tableofcontents
 * 
 * 	@section summary    Summary
 * 
 * 	The NRF24L01 is a low-cost single chip 2.4GHz transceiver 
 * 
 *  Key features (as outlined by the datasheet) are
 *   - Worldwide 2.4GHz ISM band operation
 *   - Up to 2Mbps on air data rate
 *   - Ultra low power operation
 *   - 11.3mA TX at 0dBm output power
 *   - 12.3mA RX at 2Mbps air data rate
 *   - 900nA in power down
 *   - 22µA in standby-I
 *   - On chip voltage regulator
 *   - 1.9 to 3.6V supply range
 *   - Enhanced ShockBurst™
 *   - Automatic packet handling
 *   - Auto packet transaction handling
 *   - 6 data pipe MultiCeiver™
 *   - Air compatible with nRF2401A, 02, E1 and E2
 *   - Low cost BOM
 *   - ±60ppm 16MHz crystal
 *   - 5V tolerant inputs
 *   - Compact 20-pin 4x4mm QFN package
 * 
 *  @section physical   Physical device
 *  @subsection pinout  Pinout
 * 
 *  | Pin Number | Pin Name | Full Name           | Function                                                                         |
 *  |------------|----------|---------------------|----------------------------------------------------------------------------------|
 *  | 1          | GND      | Ground              | Module ground (digital)                                                          |
 *  | 2          | VCC      | Power               | 3.3v module power rail                                                           |
 *  | 3          | CE       | Chip Enable         | Active high, places module in RX or TX operating mode from Standby-I             |
 *  | 4          | CSN      | Chip Select Not     | Selects the NRF24L01 on the SPI bus if multiple devices are present, active low  |
 *  | 5          | SCK      | Serial Clock        | SPI bus clock (max 8MHz)                                                         |
 *  | 6          | MOSI     | Master Out Slave In | Port through which data is clocked into the module                               |
 *  | 7          | MISO     | Master In Slave Out | Port through which data is clocked out of the module                             |
 *  | 8          | IRQ      | Interrupt           | Active low, fires when MCU assistance is needed                                  |
 * 
 * 	Detailed information on the SPI interface can be found @htmlonly <a href="nRF24L01_product_specifications.pdf#nameddef=8+Data+and+Control+Interface">here</a> @endhtmlonly
 * 
 * 
 *  @subsection module   Module
 *  @image{inline} html  NRF24L01_pinout.png
 * 
 * 
 *  @section config   Driver configuration
 * 
 *  User options are exposed through macro definitions provided in the user supplied "nrf24l01_config.h" file
 *  in the parent directory
 * 
 *  Macro options
 *   - `NRF24L01_ENABLE_DEBUG_LOGGING`   can be defined to enable driver debug logging
 *   - `NRF24L01_DEBUGGING_LOG`          must be defined if debug logging is enabled to provide platform specific 
 *                                       method for timestamped, tagged, message logging
 *       Ex: #define NRF24L01_DEBUGGING_LOG( ... )   log(__VA_ARGS__)
 * 
 * 	 - `NRF24L01_DEBUGGING_PRINTF`         
 *     must be defined if debug logging is enabled to provide platform specific UART interface method
 *     Ex: `#define NRF24L01_DEBUGGING_PRINTF( ... )   printf(__VA_ARGS__)`
 * 
 *   - `NRF24L01_FPTR_RTN_T`        used to specify the return type for the platform specific SPI functions
 * 								    If this option is not provided, a signed 8-bit integer type is assumed
 * 
 * 
 * 
 *  @section terms   Important terminology
 * 
 *   - **DPL** (Dynamic payload length) enabled/disabled globally via a feature register  
 *      + when disabled, packet length determined per RX pipe via NRF24L01_RX_PW_Px registers  
 * 	    + when enabled, packet length determined per packet via
 * 
 *   - **AA** (Automatic Acknowledge) packets can be automatically acknowledged in enhanced Shortburst mode
 * 
 * 	 - **AR** (Automatic Retransmit) 
 * 
 *   - **DTA** (Dynamic Transmit ACK)
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
	
/// @defgroup commands "SPI Commands"

/// @defgroup reg_addr "SPI Register Addresses"

/// @defgroup reg_bits "SPI Register Bit Definitions"



/** @name General Commands
 * 	@brief NRF24L01 general command definitions
 * 
 *  @ingroup commands
 * 
 *  These commands should be shifted out over MOSI directly after asserting CSN
 * 	Format is <b>NRF24L01_CMD_x</b> where <b>x</b> is the name assigned to the command by the datasheet
 */
/// @{
#define NRF24L01_CMD_R_REGISTER    (uint8_t)0x00  ///< Register read 0b000XXXXX where XXXXX = 5 bit Register Map Address
#define NRF24L01_CMD_W_REGISTER    (uint8_t)0x20  ///< Register write 0b001XXXXX where XXXXX = 5 bit Register Map Address
#define NRF24L01_CMD_R_RX_PAYLOAD  (uint8_t)0x61  ///< Read RX payload
#define NRF24L01_CMD_W_TX_PAYLOAD  (uint8_t)0xA0  ///< Write TX payload
#define NRF24L01_CMD_FLUSH_TX      (uint8_t)0xE1  ///< Flush TX FIFO
#define NRF24L01_CMD_FLUSH_RX      (uint8_t)0xE2  ///< Flush RX FIFO
#define NRF24L01_CMD_REUSE_TX_PL   (uint8_t)0xE3  ///< Reuse TX payload
#define NRF24L01_CMD_ACTIVATE      (uint8_t)0x50  ///< (De)Activates R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK features
#define NRF24L01_CMD_NOP           (uint8_t)0xFF  ///< No operation (used for reading status register for example)
/// @}

/** @name Feature Commands
 * 	@brief NRF24L01 special feature command definitions
 *  
 *  @ingroup commands
 * 
 * 	These commands are only valid after the ACTIVATE command has been send to toggle on the NRF24L01 special features
 * 	Format is <b>NRF24L01_FEATURE_CMD_x</b> where <b>x</b> is the name assigned to the command by the datasheet
 */
/// @{
#define NRF24L01_FEATURE_CMD_R_RX_PL_WID	     (uint8_t)0x60  ///< Read RX-payload width for the top R_RX_PAYLOAD in the RX FIFO.
#define NRF24L01_FEATURE_CMD_W_ACK_PAYLOAD       (uint8_t)0xA8  ///< Write ACK payload 0b10101XXX where XXX = 3 bit pipe identifier
#define NRF24L01_FEATURE_CMD_W_TX_PAYLOAD_NOACK  (uint8_t)0xB0  ///< Write TX payload and disable AUTOACK
/// @}


/** @name General Register Addresses
 * 	@brief NRF24L01 register address definitions
 * 
 *  @ingroup reg_addr
 * 
 * 	Format is <b>NRF24L01_REG_ADDR_r</b> where <b>r</b> is the name assigned to the register by the datasheet
 */
/// @{
#define NRF24L01_REG_ADDR_CONFIG       (uint8_t)0x00  ///< Configuration register address
#define NRF24L01_REG_ADDR_EN_AA        (uint8_t)0x01  ///< Enable "Auto acknowledgment" control register address
#define NRF24L01_REG_ADDR_EN_RXADDR    (uint8_t)0x02  ///< Enable RX addresses register address
#define NRF24L01_REG_ADDR_SETUP_AW     (uint8_t)0x03  ///< Setup of address widths register address
#define NRF24L01_REG_ADDR_SETUP_RETR   (uint8_t)0x04  ///< Setup of automatic retransmit register address
#define NRF24L01_REG_ADDR_RF_CH        (uint8_t)0x05  ///< RF channel register address
#define NRF24L01_REG_ADDR_RF_SETUP     (uint8_t)0x06  ///< RF setup register register address
#define NRF24L01_REG_ADDR_STATUS       (uint8_t)0x07  ///< Status register address
#define NRF24L01_REG_ADDR_OBSERVE_TX   (uint8_t)0x08  ///< Transmit observe register address
#define NRF24L01_REG_ADDR_CD           (uint8_t)0x09  ///< Received power detector register address
#define NRF24L01_REG_ADDR_RX_ADDR_P0   (uint8_t)0x0A  ///< Receive address data pipe 0 register address
#define NRF24L01_REG_ADDR_RX_ADDR_P1   (uint8_t)0x0B  ///< Receive address data pipe 1 register address
#define NRF24L01_REG_ADDR_RX_ADDR_P2   (uint8_t)0x0C  ///< Receive address data pipe 2 register address
#define NRF24L01_REG_ADDR_RX_ADDR_P3   (uint8_t)0x0D  ///< Receive address data pipe 3 register address
#define NRF24L01_REG_ADDR_RX_ADDR_P4   (uint8_t)0x0E  ///< Receive address data pipe 4 register address
#define NRF24L01_REG_ADDR_RX_ADDR_P5   (uint8_t)0x0F  ///< Receive address data pipe 5 register address
#define NRF24L01_REG_ADDR_TX_ADDR      (uint8_t)0x10  ///< Transmit address register address
#define NRF24L01_REG_ADDR_RX_PW_P0     (uint8_t)0x11  ///< Number of bytes in RX payload in data pipe 0 register address
#define NRF24L01_REG_ADDR_RX_PW_P1     (uint8_t)0x12  ///< Number of bytes in RX payload in data pipe 1 register address
#define NRF24L01_REG_ADDR_RX_PW_P2     (uint8_t)0x13  ///< Number of bytes in RX payload in data pipe 2 register address
#define NRF24L01_REG_ADDR_RX_PW_P3     (uint8_t)0x14  ///< Number of bytes in RX payload in data pipe 3 register address
#define NRF24L01_REG_ADDR_RX_PW_P4     (uint8_t)0x15  ///< Number of bytes in RX payload in data pipe 4 register address
#define NRF24L01_REG_ADDR_RX_PW_P5     (uint8_t)0x16  ///< Number of bytes in RX payload in data pipe 5 register address
#define NRF24L01_REG_ADDR_FIFO_STATUS  (uint8_t)0x17  ///< FIFO status register address
/// @}

/** @name Feature Registers
 * 	@brief NRF24L01 special feature register address definitions
 * 
 *  @ingroup reg_addr
 * 
 * 	Format is <b>NRF24L01_FEATURE_REG_ADDR_x</b> where <b>x</b> is the name assigned to the special feature register by the datasheet
 */
/// @{
#define NRF24L01_FEATURE_REG_ADDR_DYNPD    (uint8_t)0x1C  ///< Enable dynamic payload length register address
#define NRF24L01_FEATURE_REG_ADDR_FEATURE  (uint8_t)0x1D  ///< Feature register address
/// @}





/** @name Configuration Register (CONFIG)
 *  @brief Contains IRQ masks, CRC options, and operating mode controls
 *  @details <a href="nRF24L01_product_specifications.pdf#page=53&search=%22CONFIG%22">View in datasheet</a>
 * 	@ingroup reg_bits
 */ 
/// @{
#define NRF24L01_CONFIG_REG_BIT_MASK_RX_DR    	(uint8_t)0x40  ///< RX_DR[6] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_MASK_TX_DS    	(uint8_t)0x20  ///< TX_DS[5] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_MASK_MAX_RT   	(uint8_t)0x10  ///< MAX_RT[4] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_EN_CRC        	(uint8_t)0x08  ///< EN_CRC[3] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_CRCO          	(uint8_t)0x04  ///< CRCO[2] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_PWR_UP        	(uint8_t)0x02  ///< PWR_UP[1] bit in CONFIG register
#define NRF24L01_CONFIG_REG_BIT_PRIM_RX       	(uint8_t)0x01  ///< PRIM_RX[0] bit in CONFIG register
/// @}

/** @name Enhanced ShockBurst™ Enable Register (EN_AA)
 *  @brief Contains enable bits for Enhanced ShockBurst™ on each RX pipe
 *  @details <a href="nRF24L01_product_specifications.pdf#page=53&search=%22EN_AA%22">View in datasheet</a>
 * 	@ingroup reg_bits
 */ 
/// @{
#define NRF24L01_EN_AA_REG_BIT_ENAA_P5        	(uint8_t)0x20  ///< ENAA_P5[5] bit in EN_AA register
#define NRF24L01_EN_AA_REG_BIT_ENAA_P4        	(uint8_t)0x10  ///< ENAA_P4[4] bit in EN_AA register
#define NRF24L01_EN_AA_REG_BIT_ENAA_P3        	(uint8_t)0x08  ///< ENAA_P3[3] bit in EN_AA register
#define NRF24L01_EN_AA_REG_BIT_ENAA_P2        	(uint8_t)0x04  ///< ENAA_P2[2] bit in EN_AA register
#define NRF24L01_EN_AA_REG_BIT_ENAA_P1        	(uint8_t)0x02  ///< ENAA_P1[1] bit in EN_AA register
#define NRF24L01_EN_AA_REG_BIT_ENAA_P0        	(uint8_t)0x01  ///< ENAA_P0[0] bit in EN_AA register
/// @}


/** @name Register Bits
 * 	@brief NRF24L01 register bit definitions
 *  
 *  Format is <b>NRF24L01_r_REG_BIT_x</b> where <b>r</b> is the register name where the bit is found and <b>x</b> is the name assigned to the register bit by the datasheet
 *  If register contains a bit field then REG_BITS is plural
 */
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P5     	(uint8_t)0x20  ///<
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P4     	(uint8_t)0x10  ///<
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P3     	(uint8_t)0x08  ///<
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P2     	(uint8_t)0x04  ///<
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P1     	(uint8_t)0x02  ///<
#define NRF24L01_EN_RXADDR_REG_BIT_ERX_P0     	(uint8_t)0x01  ///<

#define NRF24L01_SETUP_AW_REG_BITS_AW         	(uint8_t)0x03  ///< AW[1:0] bits in SETUP_AW register

#define NRF24L01_SETUP_RETR_REG_BITS_ARD      	(uint8_t)0xF0  ///< ARD[7:4] bits in SETUP_RETR register
#define NRF24L01_SETUP_RETR_REG_BITS_ARC      	(uint8_t)0x0F  ///< ARC[3:0] bits in SETUP_RETR register

#define NRF24L01_RF_CH_REG_BITS_RF_CH         	(uint8_t)0x7F  ///< RF_CH[6:0] bits in RF_CH register

#define NRF24L01_RF_SETUP_REG_BIT_PLL_LOCK    	(uint8_t)0x10  ///< PLL_LOCK[5] bit in RF_SETUP register
#define NRF24L01_RF_SETUP_REG_BIT_RF_DR       	(uint8_t)0x08  ///< RF_DR[4] bit in RF_SETUP register
#define NRF24L01_RF_SETUP_REG_BITS_RF_PWR     	(uint8_t)0x06  ///< RF_PWR[2:1] bits in RF_SETUP register
#define NRF24L01_RF_SETUP_REG_BIT_LNA_HCURR   	(uint8_t)0x01  ///< LNA_HCURR[0] bit in RF_SETUP register

#define NRF24L01_STATUS_REG_BIT_RX_DR         	(uint8_t)0x40  ///< RX_DR[6] interrupt flag bit in STATUS register
#define NRF24L01_STATUS_REG_BIT_TX_DS         	(uint8_t)0x20  ///< TX_DS[5] interrupt flag bit in STATUS register
#define NRF24L01_STATUS_REG_BIT_MAX_RT        	(uint8_t)0x10  ///< MAX_RT[4] interrupt flag bit in STATUS register
#define NRF24L01_STATUS_REG_BITS_RX_P_NO      	(uint8_t)0x0E  ///< RX_P_NO[3:1] pipe number bits in STATUS register
#define NRF24L01_STATUS_REG_BIT_TX_FULL       	(uint8_t)0x01  ///< TX_FULL[0] flag bit in STATUS register

#define NRF24L01_OBSERVE_TX_REG_BITS_PLOS_CNT 	(uint8_t)0xF0  ///< PLOS_CNT[7:4] lost packet count bits in OBSERVE_TX register
#define NRF24L01_OBSERVE_TX_REG_BITS_ARC_CNT  	(uint8_t)0x0F  ///< ARC_CNT[3:0] resent packet count bits in OBSERVE_TX register

#define NRF24L01_CD_REG_BIT_CD                	(uint8_t)0x01  ///< CD[0] bit in CD register

#define NRF24L01_RX_PW_P0_REG_BITS_RX_PW_P0   	(uint8_t)0x3F  ///<

#define NRF24L01_RX_PW_P1_REG_BITS_RX_PW_P1   	(uint8_t)0x3F  ///<

#define NRF24L01_RX_PW_P2_REG_BITS_RX_PW_P2   	(uint8_t)0x3F  ///<

#define NRF24L01_RX_PW_P3_REG_BITS_RX_PW_P3   	(uint8_t)0x3F  ///<

#define NRF24L01_RX_PW_P4_REG_BITS_RX_PW_P4   	(uint8_t)0x3F  ///<

#define NRF24L01_RX_PW_P5_REG_BITS_RX_PW_P5   	(uint8_t)0x3F  ///<

#define NRF24L01_FIFO_STATUS_REG_BIT_TX_REUSE 	(uint8_t)0x40  ///< TX_REUSE[5] status bit in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_REG_BIT_TX_FULL  	(uint8_t)0x20  ///< TX_FULL[6] status bit in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_REG_BIT_TX_EMPTY 	(uint8_t)0x10  ///< TX_EMPTY[4] status bits [5:4] in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_REG_BIT_RX_FULL  	(uint8_t)0x02  ///< RX_FULL[1] status bit in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_REG_BIT_RX_EMPTY 	(uint8_t)0x01  ///< RX_EMPTY[0] status bit in FIFO_STATUS register

#define NRF24L01_DYNPD_REG_BIT_DPL_P5         	(uint8_t)0x20  ///<
#define NRF24L01_DYNPD_REG_BIT_DPL_P4         	(uint8_t)0x10  ///<
#define NRF24L01_DYNPD_REG_BIT_DPL_P3         	(uint8_t)0x08  ///<
#define NRF24L01_DYNPD_REG_BIT_DPL_P2         	(uint8_t)0x04  ///<
#define NRF24L01_DYNPD_REG_BIT_DPL_P1         	(uint8_t)0x02  ///<
#define NRF24L01_DYNPD_REG_BIT_DPL_P0         	(uint8_t)0x01  ///<

#define NRF24L01_FEATURE_REG_BIT_EN_DPL       	(uint8_t)0x04  ///< EN_DPL[2] bit in FEATURE register
#define NRF24L01_FEATURE_REG_BIT_EN_ACK_PAY   	(uint8_t)0x02  ///< EN_ACK_PAY[1] bit in FEATURE register
#define NRF24L01_FEATURE_REG_BIT_EN_DYN_ACK   	(uint8_t)0x01  ///< EN_DYN_ACK[0] bit in FEATURE register



/** @name Register masks
 * 	@brief NRF24L01 register mask definitions
 * 	Format is <b>NRF24L01_r_MASK_x</b> where <b>r</b> is the register name where the bit is found and <b>x</b> an arbitrary assigned name
 *  Each register also has a full mask defined for it under the name NRF24L01_r_MASK_REG
 */
/// @{
///< Mask for all defined bits[6:0] in CONFIG register 
#define NRF24L01_CONFIG_MASK_REG              	(uint8_t)(                                \
													NRF24L01_CONFIG_REG_BIT_MASK_RX_DR  | \
													NRF24L01_CONFIG_REG_BIT_MASK_TX_DS  | \
													NRF24L01_CONFIG_REG_BIT_MASK_MAX_RT | \
													NRF24L01_CONFIG_REG_BIT_EN_CRC      | \
													NRF24L01_CONFIG_REG_BIT_CRCO        | \
													NRF24L01_CONFIG_REG_BIT_PWR_UP      | \
													NRF24L01_CONFIG_REG_BIT_PRIM_RX       \
											  	)
///< Mask for CRC[3:2] bits in CONFIG register 
#define NRF24L01_CONFIG_MASK_INTERRUPT_MASKS  	(uint8_t)(                                \
													NRF24L01_CONFIG_REG_BIT_MASK_RX_DR  | \
													NRF24L01_CONFIG_REG_BIT_MASK_TX_DS  | \
													NRF24L01_CONFIG_REG_BIT_MASK_MAX_RT   \
											  	)
///< Mask for CRC[3:2] bits in EN_AA register 
#define NRF24L01_CONFIG_MASK_CRC   			 	(uint8_t)(                                \
													NRF24L01_CONFIG_REG_BIT_EN_CRC      | \
													NRF24L01_CONFIG_REG_BIT_CRCO          \
											  	)


///< Mask for all defined bits[5:0] in EN_AA register 
#define NRF24L01_EN_AA_MASK_REG              	(uint8_t)(   							  \
													NRF24L01_EN_AA_REG_BIT_ENAA_P5      | \
													NRF24L01_EN_AA_REG_BIT_ENAA_P4      | \
													NRF24L01_EN_AA_REG_BIT_ENAA_P3      | \
													NRF24L01_EN_AA_REG_BIT_ENAA_P2      | \
													NRF24L01_EN_AA_REG_BIT_ENAA_P1      | \
													NRF24L01_EN_AA_REG_BIT_ENAA_P0        \
												)


///< Mask for all defined bits[5:0] in EN_RXADDR register 
#define NRF24L01_EN_RXADDR_MASK_REG             (uint8_t)(   							  \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P5   | \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P4   | \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P3   | \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P2   | \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P1   | \
													NRF24L01_EN_RXADDR_REG_BIT_ERX_P0     \
												)


///< Mask for all defined bits[1:0] in AW register
#define NRF24L01_SETUP_AW_MASK_REG              (uint8_t)NRF24L01_SETUP_AW_REG_BITS_AW


///< Mask for all defined bits[7:0] in SETUP_RETR register 
#define NRF24L01_SETUP_RETR_MASK_REG            (uint8_t)(								  \
													NRF24L01_SETUP_RETR_REG_BITS_ARD    | \
													NRF24L01_SETUP_RETR_REG_BITS_ARC      \
												)


///< Mask for all defined bits[6:0] in RF_CH register 
#define NRF24L01_RF_CH_MASK_REG                 (uint8_t)NRF24L01_RF_CH_REG_BITS_RF_CH


///< Mask for all defined bits[4:0] in RF_SETUP register 
#define NRF24L01_RF_SETUP_MASK_REG              (uint8_t)(								  \
													NRF24L01_RF_SETUP_REG_BIT_PLL_LOCK  | \
													NRF24L01_RF_SETUP_REG_BIT_RF_DR     | \
													NRF24L01_RF_SETUP_REG_BITS_RF_PWR   | \
													NRF24L01_RF_SETUP_REG_BIT_LNA_HCURR   \
												)


///< Mask for all defined bits[6:0] in STATUS register 
#define NRF24L01_STATUS_MASK_REG 				(uint8_t)(								  \
													NRF24L01_STATUS_REG_BIT_RX_DR       | \
													NRF24L01_STATUS_REG_BIT_TX_DS       | \
													NRF24L01_STATUS_REG_BIT_MAX_RT      | \
													NRF24L01_STATUS_REG_BITS_RX_P_NO    | \
													NRF24L01_STATUS_REG_BIT_TX_FULL       \
												)
///< Mask for IRQ[6:4] interrupt flag bits in STATUS register
#define NRF24L01_STATUS_MASK_IRQ_FLAGS			(uint8_t)(								  \
													NRF24L01_STATUS_REG_BIT_RX_DR       | \
													NRF24L01_STATUS_REG_BIT_TX_DS       | \
													NRF24L01_STATUS_REG_BIT_MAX_RT        \
												)


///< Mask for all defined bits[7:0] in OBSERVE_TX register 
#define NRF24L01_OBSERVE_TX_MASK_REG            (uint8_t)(								    \
													NRF24L01_OBSERVE_TX_REG_BITS_PLOS_CNT | \
													NRF24L01_OBSERVE_TX_REG_BITS_ARC_CNT    \
												)


#define NRF24L01_CD_MASK_REG                    (uint8_t)NRF24L01_CD_REG_BIT_CD


#define NRF24L01_RX_PW_P0_MASK_REG              (uint8_t)NRF24L01_RX_PW_P0_REG_BITS_RX_PW_P0
#define NRF24L01_RX_PW_P1_MASK_REG              (uint8_t)NRF24L01_RX_PW_P1_REG_BITS_RX_PW_P1  
#define NRF24L01_RX_PW_P2_MASK_REG              (uint8_t)NRF24L01_RX_PW_P2_REG_BITS_RX_PW_P2 
#define NRF24L01_RX_PW_P3_MASK_REG              (uint8_t)NRF24L01_RX_PW_P3_REG_BITS_RX_PW_P3 
#define NRF24L01_RX_PW_P4_MASK_REG              (uint8_t)NRF24L01_RX_PW_P4_REG_BITS_RX_PW_P4 
#define NRF24L01_RX_PW_P5_MASK_REG              (uint8_t)NRF24L01_RX_PW_P5_REG_BITS_RX_PW_P5 


///< Mask for all defined bits in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_MASK_REG 			(uint8_t)(								     \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_REUSE  | \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_FULL   | \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_EMPTY  | \
													NRF24L01_FIFO_STATUS_REG_BIT_RX_FULL   | \
													NRF24L01_FIFO_STATUS_REG_BIT_RX_EMPTY    \
												)
///< Mask for RX_FIFO[1:0] status bits in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_MASK_RX_FLAGS      (uint8_t)(								     \
													NRF24L01_FIFO_STATUS_REG_BIT_RX_FULL   | \
													NRF24L01_FIFO_STATUS_REG_BIT_RX_EMPTY    \
												)
///< Mask for TX_FIFO[6:4] status bits in FIFO_STATUS register
#define NRF24L01_FIFO_STATUS_MASK_TX_FLAGS      (uint8_t)(								     \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_REUSE  | \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_FULL   | \
													NRF24L01_FIFO_STATUS_REG_BIT_TX_EMPTY    \
												)

///< Mask for DPL_Px[5:0] bits in DYNPD feature register 
#define NRF24L01_DYNPD_MASK_REG             	(uint8_t)(   						  \
													NRF24L01_DYNPD_REG_BIT_DPL_P5   | \
													NRF24L01_DYNPD_REG_BIT_DPL_P4   | \
													NRF24L01_DYNPD_REG_BIT_DPL_P3   | \
													NRF24L01_DYNPD_REG_BIT_DPL_P2   | \
													NRF24L01_DYNPD_REG_BIT_DPL_P1   | \
													NRF24L01_DYNPD_REG_BIT_DPL_P0     \
												)


#define NRF24L01_FEATURE_MASK_REG           	(uint8_t)(								  \
													NRF24L01_FEATURE_REG_BIT_EN_DPL     | \
													NRF24L01_FEATURE_REG_BIT_EN_ACK_PAY | \
													NRF24L01_FEATURE_REG_BIT_EN_DYN_ACK   \
												)											
/// @}




#define NRF24L01_COMMAND_MASK_REG_ADDR    (uint8_t)0x1F  ///< Mask for AAAAA[4:0] register address bits for CMD_RREG and CMD_WREG commands
#define NRF24L01_COMMAND_MASK_PIPE        (uint8_t)0x07  ///< Mask for PPP[2:0] pipe specifier bits for W_ACK_PAYLOAD command


// CRC
#define	NRF24L01_ADDR_WIDTH_INVALID       (uint8_t)0x00  ///< Invalid address field width
#define	NRF24L01_ADDR_WIDTH_3_BYTES       (uint8_t)0x01  ///< 3 Byte RX/TX address field width
#define	NRF24L01_ADDR_WIDTH_4_BYTES       (uint8_t)0x02  ///< 4 Byte RX/TX address field width
#define	NRF24L01_ADDR_WIDTH_5_BYTES       (uint8_t)0x03  ///< 5 Byte RX/TX address field width
 
// RF_PWR
#define	NRF24L01_TX_PWR_18DBM              (uint8_t)0x00  ///< -18dBm
#define	NRF24L01_TX_PWR_12DBM              (uint8_t)0x02  ///< -12dBm
#define	NRF24L01_TX_PWR_6DBM               (uint8_t)0x04  ///< -6dBm
#define	NRF24L01_TX_PWR_0DBM               (uint8_t)0x06  ///<  0dBm

// Timing requirements
#define NRF24L01_CE_TX_MINIMUM_PULSE_US   10   // In microseconds
#define NRF24L01_POWER_UP_US              5000 // In microseconds
#define NRF24L01_TIMING_RF_SETTLING       130  // In microseconds

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

/** @brief Enumeration of RX pipe addresses and TX address
 *  
 *  Found in datasheet as @htmlonly <a href="nRF24L01_product_specifications.pdf#page=55&search=%22RX_P_NO%22">RX_P_NO[3:1] bits in STATUS register</a> @endhtmlonly
 */ 
typedef enum {
	NRF24L01_PIPE0        = (uint8_t)0x00,  ///< pipe0
	NRF24L01_PIPE1        = (uint8_t)0x01,  ///< pipe1
	NRF24L01_PIPE2        = (uint8_t)0x02,  ///< pipe2
	NRF24L01_PIPE3        = (uint8_t)0x03,  ///< pipe3
	NRF24L01_PIPE4        = (uint8_t)0x04,  ///< pipe4
	NRF24L01_PIPE5        = (uint8_t)0x05,  ///< pipe5
	NRF24L01_PIPETX       = (uint8_t)0x06,  ///< TX address (not a pipe in fact)
	NRF24L01_PIPE_UNKNOWN = (uint8_t)0x07,  ///< pipe unknown (usually signifies rx pipe is empty)
	NRF24L01_ALL_RX_PIPES,                  ///< Used in the API to configure all RX pipes at the same time
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


/**
 *  @brief Hardware initialization function pointer which should be used to configure the
 *  hardware before communication is attempted
 *  
 *  Responsibilities
 *   - Configure IRQ MCU input. IRQ output pin is active-low. Interrupt pin should be triggered on negative-edge. Pull-up resistor needed
 *
 *  @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 *  @retval 0        -> Success
 *  @retval Non-zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_platform_init_fptr_t)(void* user_ptr);


/**
 * @brief SPI Bus deinitialization function pointer which should be used to release the
 * hardware when the driver is deinitialized
 *
 * @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non-zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_platform_deinit_fptr_t)(void* user_ptr);


/**
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
 * @retval Non-zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_spi_exchange_fptr_t)(uint8_t command, uint8_t* rx_data, uint8_t* tx_data, uint8_t len, void* user_ptr);


/**
 * @brief Set the state of Chip enable GPIO function pointer
 *
 * @param[in] state         : Desired state of the chip enable GPIO
 * @param[in] user_ptr      : Pointer to user-defined hardware configuration struct
 *
 * @retval 0        -> Success
 * @retval Non-zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_gpio_chip_enable_fptr_t)(bool state, void* user_ptr);


/**
 * @brief Microsecond delay function pointer which should be mapped to
 * the platform specific delay function of the user
 *
 * @param[in] delay      : Number of microseconds to delay
 *
 * @retval 0        -> Success
 * @retval Non-zero -> Fail
 */
typedef NRF24L01_FPTR_RTN_T (*nrf24l01_delay_us_fptr_t)(uint32_t delay);




/** @brief User supplied RX data received callback function pointer
 *  
 *  @param[in] message_size : Length of the received message in bytes
 *  @param[in] pipe         : The RX pipe from which the data was received
 *  @param[in] user_ptr     : The user-defined pointer which was provided when the callback was registered
 *  @param[in] platform     : Driver instance configuration
 */
typedef void (*nrf24l01_rx_dr_callback_fptr_t)(uint8_t message_size, nrf24l01_pipe_t pipe, void* user_ptr, nrf24l01_platform_t* platform);


/** @brief User supplied TX data sent callback function pointer
 *  
 *  @param[in] pipe         : 
 *  @param[in] user_ptr     : The user-defined pointer which was provided when the callback was registered
 *  @param[in] platform     : Driver instance configuration
 */
typedef void (*nrf24l01_tx_ds_callback_fptr_t)(nrf24l01_pipe_t pipe, void* user_ptr, nrf24l01_platform_t* platform);


/** @brief User supplied maximum retries exceeded callback function pointer
 *  
 *  @param[in] user_ptr     : The user-defined pointer which was provided when the callback was registered
 *  @param[in] platform     : Driver instance configuration
 */
typedef void (*nrf24l01_max_rt_callback_fptr_t)(void* user_ptr, nrf24l01_platform_t* platform);




/** @brief Driver instance configuration information
 * 
 *  Encapsulates an entire NRF24L01 driver instance to avoid the need for dynamic memory allocation. The application is responsible for
 *  keeping this structure in scope while the driver instance is in use
 */ 
struct nrf24l01_platform_t{
	nrf24l01_delay_us_fptr_t            delay_us;                 ///< Pointer to a platform specific microsecond delay function
	nrf24l01_gpio_chip_enable_fptr_t    gpio_chip_enable;         ///< Pointer to the platform specific chip enable GPIO control function
	nrf24l01_platform_init_fptr_t       platform_init;            ///< Pointer to the platform specific hardware initialization function
	nrf24l01_platform_deinit_fptr_t     platform_deinit;          ///< Pointer to the platform specific hardware deinitialization function
	nrf24l01_spi_exchange_fptr_t        spi_exchange;             ///< Pointer to the platform specific SPI full-duplex transfer function
	struct {
		nrf24l01_rx_dr_callback_fptr_t  rx_dr_callback;           ///< Callback function pointer to be called when RX data is received and the IRQ interrupt is asserted
		void*                           rx_dr_callback_user_ptr;  ///< User-defined pointer to be passed along when calling rx_dr_callback
		nrf24l01_tx_ds_callback_fptr_t  tx_ds_callback;           ///< Callback function pointer to be called when TX data is sent and the IRQ interrupt is asserted
		void*                           tx_ds_callback_user_ptr;  ///< User-defined pointer to be passed along when calling tx_ds_callback
		nrf24l01_max_rt_callback_fptr_t max_rt_callback;          ///< Callback function pointer to be called when the maximum number of packet transmission retries has been exceeded and the IRQ interrupt is asserted
		void*                           max_rt_callback_user_ptr; ///< User-defined pointer to be passed along when calling max_rt_callback
	} callbacks;
	void*                               user_ptr;                 ///< (optional) Pointer to a user-defined hardware configuration struct
};

#endif



