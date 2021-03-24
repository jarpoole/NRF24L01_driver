
/**
 * @file
 * @author  Jared Poole
 * @version 1.1.7
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Driver for the nRF24L01(+) transceiver
 */

#include "nrf24l01.h"

// Fake address to test transceiver presence (5 bytes long)
#define NRF24L01_TEST_ADDR "nRF24"

// Address lookup table for the RX_PW_P# registers
static const uint8_t NRF24L01_RX_PW_PIPE[] = {
	NRF24L01_REG_ADDR_RX_PW_P0,
	NRF24L01_REG_ADDR_RX_PW_P1,
	NRF24L01_REG_ADDR_RX_PW_P2,
	NRF24L01_REG_ADDR_RX_PW_P3,
	NRF24L01_REG_ADDR_RX_PW_P4,
	NRF24L01_REG_ADDR_RX_PW_P5,
};

// Address lookup table for the RX_ADDR_P# address registers
static const uint8_t NRF24L01_ADDR_REGS[] = {
	NRF24L01_REG_ADDR_RX_ADDR_P0,
	NRF24L01_REG_ADDR_RX_ADDR_P1,
	NRF24L01_REG_ADDR_RX_ADDR_P2,
	NRF24L01_REG_ADDR_RX_ADDR_P3,
	NRF24L01_REG_ADDR_RX_ADDR_P4,
	NRF24L01_REG_ADDR_RX_ADDR_P5,
	NRF24L01_REG_ADDR_TX_ADDR,
};

// Static prototypes
static nrf24l01_err_t nrf24l01_read_reg(uint8_t, uint8_t*, nrf24l01_platform_t*);
static nrf24l01_err_t nrf24l01_write_reg(uint8_t, uint8_t, nrf24l01_platform_t*);
static nrf24l01_err_t nrf24l01_multi_read_reg(uint8_t, uint8_t*, uint8_t, nrf24l01_platform_t*);
static nrf24l01_err_t nrf24l01_multi_write_reg(uint8_t, uint8_t*, uint8_t, nrf24l01_platform_t*);


/** @brief Wrapper around platform multi-register read
 * 
 *  Reads a single byte from the provided register address. Interprets platform error codes into NRF24L01 internal error codes
 * 
 *  @param reg_addr : The 8-bit SPI register address
 * 			          @see{regaddr sfraddr}
 *  @param data     : The register contents
 *  @param platform : Driver instance configuration
 * 
 *  @retval NRF24L01_ERR_INVALID_ARG :
 *  @retval NRF24L01_ERR_READ        :
 *  @retval NRF24L01_OK              :
 */ 
static nrf24l01_err_t nrf24l01_read_reg(uint8_t reg_addr, uint8_t* data, nrf24l01_platform_t* platform) {
	if(data == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t command = NRF24L01_CMD_R_REGISTER | (reg_addr & NRF24L01_COMMAND_MASK_REG_ADDR);

	NRF24L01_FPTR_RTN_T err = platform->spi_exchange(command, data, NULL, sizeof(uint8_t), platform->user_ptr);
	if(err != 0){
		return NRF24L01_ERR_READ;
	}
	return NRF24L01_OK;
}

// Write a new value to register
// input:
//   reg - number of register to write
//   value - value to write
static nrf24l01_err_t nrf24l01_write_reg(uint8_t reg_addr, uint8_t data, nrf24l01_platform_t* platform) {
	uint8_t command = NRF24L01_CMD_W_REGISTER | (reg_addr & NRF24L01_COMMAND_MASK_REG_ADDR);
	NRF24L01_FPTR_RTN_T err = platform->spi_exchange(command, NULL, &data, sizeof(uint8_t), platform->user_ptr);
	if(err != 0){
		return NRF24L01_ERR_WRITE;
	}
	return NRF24L01_OK;
}

// Read a multi-byte register
// input:
//   reg - number of register to read
//   pBuf - pointer to the buffer for register data
//   count - number of bytes to read
static nrf24l01_err_t nrf24l01_multi_read_reg(uint8_t reg_addr, uint8_t *data, uint8_t len, nrf24l01_platform_t* platform) {
	if(data == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t command = NRF24L01_CMD_R_REGISTER | (reg_addr & NRF24L01_COMMAND_MASK_REG_ADDR);
	
	NRF24L01_FPTR_RTN_T err = platform->spi_exchange(command, data, NULL, len, platform->user_ptr);
	if(err != 0){
		return NRF24L01_ERR_READ;
	}
	return NRF24L01_OK;
}

// Write a multi-byte register
// input:
//   reg - number of register to write
//   pBuf - pointer to the buffer with data to write
//   count - number of bytes to write
static nrf24l01_err_t nrf24l01_multi_write_reg(uint8_t reg_addr, uint8_t* data, uint8_t len, nrf24l01_platform_t* platform) {
	if(data == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t command = NRF24L01_CMD_W_REGISTER | (reg_addr & NRF24L01_COMMAND_MASK_REG_ADDR);

	NRF24L01_FPTR_RTN_T err = platform->spi_exchange(command, NULL, data, len, platform->user_ptr);
	if(err != 0){
		return NRF24L01_ERR_WRITE;
	}
	return NRF24L01_OK;
}









// Set transceiver to it's initial state
// note: RX/TX pipe addresses remains untouched
nrf24l01_err_t nrf24l01_init(nrf24l01_platform_t* platform) {
	
	// Initialize the platform SPI controller
	NRF24L01_FPTR_RTN_T init_err = platform->platform_init(platform->user_ptr);
	if(init_err != 0){
		return NRF24L01_ERR_UNKNOWN;
	}

	// Write to registers their initial values
	nrf24l01_err_t err = NRF24L01_OK;
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG,     0x08, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_EN_AA,      0x3F, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_EN_RXADDR,  0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_SETUP_AW,   0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_SETUP_RETR, 0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_CH,      0x02, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_SETUP,   0x0E, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_STATUS,     0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P0,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P1,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P2,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P3,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P4,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RX_PW_P5,   0x00, platform);
	//err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_DYNPD,   0x00, platform);
	//err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, 0x00, platform);

	// Clear the FIFO's
	err |= nrf24l01_flush_rx(platform);
	err |= nrf24l01_flush_tx(platform);

	// Clear any pending interrupt flags
	uint8_t temp;
	err |= nrf24l01_get_irq_flags(&temp, platform);

	return err;
}

//Deinitialize the driver and free resources
nrf24l01_err_t nrf24l01_deinit(nrf24l01_platform_t* platform){

	// deinitialize the platform SPI controller
	NRF24L01_FPTR_RTN_T deinit_err = platform->platform_deinit(platform->user_ptr);
	if(deinit_err != 0){
		return NRF24L01_ERR_UNKNOWN;
	}
	return NRF24L01_OK;
}

// Check if the nRF24L01 present
// return:
//   NRF24L01_OK                    - nRF24L01 is online and responding
//   NRF24L01_ERR_DEVICE_NOT_FOUND  - received sequence differs from original
nrf24l01_err_t nrf24l01_check_connectivity(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;

	// Write test TX address
	uint8_t *ptr = (uint8_t *)NRF24L01_TEST_ADDR;
	err |= nrf24l01_multi_write_reg(NRF24L01_REG_ADDR_TX_ADDR, ptr, sizeof(NRF24L01_TEST_ADDR) - 1, platform);

	//Read TX_ADDR register
	uint8_t rxbuf[5];
	err |= nrf24l01_multi_read_reg(NRF24L01_REG_ADDR_TX_ADDR, &rxbuf[0], sizeof(rxbuf), platform);

	// Compare buffers, return error on first mismatch
	for (uint8_t i = 0; i < 5; i++) {
		if (rxbuf[i] != *ptr){
			err = NRF24L01_ERR_DEVICE_NOT_FOUND;
			break;
		}
		ptr++; 
	}
	return err;
}

nrf24l01_err_t nrf24l01_set_pipe_rx_payload_size(nrf24l01_pipe_t pipe, uint8_t payload_len, nrf24l01_platform_t* platform){
	if(payload_len > NRF24L01_RX_FIFO_WIDTH || payload_len == 0){
		return NRF24L01_ERR_INVALID_ARG;
	}else if( !NRF24L01_IS_RX_PIPE(pipe) ){
		return NRF24L01_ERR_INVALID_ARG;
	}

	if(pipe == NRF24L01_ALL_RX_PIPES){
		nrf24l01_err_t err = NRF24L01_OK;
		for(uint8_t pipe_index = NRF24L01_PIPE0; pipe_index <= NRF24L01_PIPE5; pipe_index++){
			err |= nrf24l01_write_reg(NRF24L01_RX_PW_PIPE[pipe_index], payload_len, platform);
		}
		return err;
	}else{
		return nrf24l01_write_reg(NRF24L01_RX_PW_PIPE[pipe], payload_len, platform);
	}
}

// Control transceiver power mode
// input:
//   mode - new state of power mode, one of NRF24L01_PWR_xx values
nrf24l01_err_t nrf24l01_set_power_mode(nrf24l01_power_mode_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	if (mode == NRF24L01_PWR_UP) {
		// Set the PWR_UP bit of CONFIG register to wake the transceiver
		// It goes into Stanby-I mode with consumption about 26uA
		reg |= NRF24L01_CONFIG_REG_BIT_PWR_UP;
		err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG, reg, platform);
		
		platform->delay_us(NRF24L01_POWER_UP_US);
	} else {
		platform->gpio_chip_enable(false, platform->user_ptr);

		// Clear the PWR_UP bit of CONFIG register to put the transceiver
		// into power down mode with consumption about 900nA
		reg &= ~NRF24L01_CONFIG_REG_BIT_PWR_UP;
		err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG, reg, platform);
	}
	return err;
}

nrf24l01_err_t nrf24l01_start_listening(nrf24l01_platform_t* platform){
	NRF24L01_FPTR_RTN_T platform_err = 0;
	platform_err |= platform->gpio_chip_enable(true, platform->user_ptr);
	platform_err |= platform->delay_us(NRF24L01_TIMING_RF_SETTLING);
	return ( (platform_err == 0) ? NRF24L01_OK : NRF24L01_ERR_UNKNOWN );
}
nrf24l01_err_t nrf24l01_stop_listening(nrf24l01_platform_t* platform){
	NRF24L01_FPTR_RTN_T platform_err = 0;
	platform_err |= platform->gpio_chip_enable(false, platform->user_ptr);
	return ( (platform_err == 0) ? NRF24L01_OK : NRF24L01_ERR_UNKNOWN );
}

// Set transceiver operational mode
// input:
//   mode - operational mode, one of nRF24_MODE_xx values
nrf24l01_err_t nrf24l01_set_operational_mode(nrf24l01_operational_mode_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure PRIM_RX bit of the CONFIG register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	reg &= ~NRF24L01_CONFIG_REG_BIT_PRIM_RX;
	reg |= (mode & NRF24L01_CONFIG_REG_BIT_PRIM_RX);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG, reg, platform);

	return err;
}


// Configure transceiver CRC scheme
// input:
//   scheme - CRC scheme, one of nRF24_CRC_xx values
// note: transceiver will forcibly turn on the CRC in case if auto acknowledgment
//       enabled for at least one RX pipe
nrf24l01_err_t nrf24l01_set_crc_scheme(nrf24l01_crc_scheme_t scheme, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	reg &= ~NRF24L01_CONFIG_MASK_CRC;
	reg |= (scheme & NRF24L01_CONFIG_MASK_CRC);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG, reg, platform);

	return err;
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
nrf24l01_err_t nrf24l01_set_rf_channel(uint8_t channel, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_CH, channel, platform);
	return err;
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay, one of nRF24_ARD_xx values
//   arc - count of auto retransmits, value form 0 to 15
// note: zero arc value means that the automatic retransmission disabled
nrf24l01_err_t nrf24l01_set_auto_retransmission(nrf24l01_ar_delay_t ard, nrf24l01_ar_count_t arc, nrf24l01_platform_t* platform) {
	// Set auto retransmit settings (SETUP_RETR register)
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_REG_ADDR_SETUP_RETR, (uint8_t)((ard << 4) | (arc & NRF24L01_SETUP_RETR_REG_BITS_ARC)), platform);
	return err;
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
nrf24l01_err_t nrf24l01_set_address_width(nrf24l01_address_width_t addr_width, nrf24l01_platform_t* platform) {
	uint8_t reg_val;
	if(addr_width == 3){
		reg_val = NRF24L01_ADDR_WIDTH_3_BYTES;
	}else if(addr_width == 4){
		reg_val = NRF24L01_ADDR_WIDTH_4_BYTES;
	}else if(addr_width == 5){
		reg_val = NRF24L01_ADDR_WIDTH_5_BYTES;
	}else{
		return NRF24L01_ERR_INVALID_ARG;
	}
	nrf24l01_err_t err = nrf24l01_write_reg(NRF24L01_REG_ADDR_SETUP_AW, reg_val, platform);
	return err;
}

// Set static RX address for a specified pipe
// input:
//   pipe - pipe to configure address, one of NRF24L01_PIPEx values
//   addr - pointer to the buffer with address
// note: pipe can be a number from 0 to 5 (RX pipes) and 6 (TX pipe)
// note: buffer length must be equal to current address width of transceiver
// note: for pipes[2..5] only first byte of address will be written because
//       pipes 1-5 share the four most significant address bytes
nrf24l01_err_t nrf24l01_set_address(nrf24l01_pipe_t pipe, const uint8_t* addr, nrf24l01_address_width_t addr_width, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	
	if(pipe == NRF24L01_PIPE0 || pipe == NRF24L01_PIPE1 || pipe == NRF24L01_PIPETX){
		// Get the devices configured address width
		nrf24l01_address_width_t config_addr_width;
		err = nrf24l01_get_address_width(&config_addr_width, platform);
		if(config_addr_width != addr_width){
			err = NRF24L01_ERR_INVALID_ARG; 
		}else{
			// Write address in reverse order (LSByte first)
			err = nrf24l01_multi_write_reg(NRF24L01_ADDR_REGS[pipe], addr, addr_width, platform);
		}
	}
	else if(pipe == NRF24L01_PIPE2 || pipe == NRF24L01_PIPE3 || pipe == NRF24L01_PIPE4 || pipe == NRF24L01_PIPE5){
		// Write address LSBbyte only (first byte from the addr buffer)
		err = nrf24l01_write_reg(NRF24L01_ADDR_REGS[pipe], *addr, platform);
	}else{
		// Incorrect pipe number
		err = NRF24L01_ERR_INVALID_ARG;
	}
	return err;
}

// Configure RF output power in TX mode
// input:
//   tx_pwr - RF output power, one of nRF24_TXPWR_xx values
nrf24l01_err_t nrf24l01_set_tx_power(nrf24l01_tx_power_t tx_power, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure RF_PWR[2:1] bits of the RF_SETUP register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_RF_SETUP, &reg, platform);
	reg &= ~NRF24L01_RF_SETUP_REG_BITS_RF_PWR;
	reg |= tx_power;
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_SETUP, reg, platform);

	return err;
}

// Configure transceiver data rate
// input:
//   data_rate - data rate, one of nRF24_DR_xx values
nrf24l01_err_t nrf24l01_set_data_rate(nrf24l01_data_rate_t data_rate, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_RF_SETUP, &reg, platform);
	reg &= ~NRF24L01_RF_SETUP_REG_BIT_RF_DR;
	reg |= data_rate;
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_SETUP, reg, platform);

	return err;
}


// Enable/disable specified RX pipe
// input:
//   pipe - number of RX pipe, value from 0 to 5
//   mode - NRF24L01_PIPE_ENABLED or NRF24L01_PIPE_DISABLED
nrf24l01_err_t nrf24l01_set_pipe_mode(nrf24l01_pipe_t pipe, nrf24l01_pipe_mode_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	
	if( !NRF24L01_IS_RX_PIPE(pipe) ){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t reg;
	uint8_t changes;

	if(pipe == NRF24L01_ALL_RX_PIPES){
		changes = NRF24L01_EN_RXADDR_MASK_REG;
	}else{
		changes = (1 << pipe);
	}

	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_EN_RXADDR, &reg, platform);
	if(mode == NRF24L01_PIPE_ENABLED){
		reg |= changes;
	}else if(mode == NRF24L01_PIPE_DISABLED){
		reg &= ~changes;
	}else{
		return NRF24L01_ERR_INVALID_ARG;
	}
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_EN_RXADDR, reg, platform);

	return err;
}


// Configure the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
nrf24l01_err_t nrf24l01_set_pipe_aa_mode(nrf24l01_pipe_t pipe, nrf24l01_pipe_aa_mode_t aa_mode, nrf24l01_platform_t* platform){
	nrf24l01_err_t err = NRF24L01_OK;
	
	if( !NRF24L01_IS_RX_PIPE(pipe) ){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t reg;
	uint8_t changes;

	if(pipe == NRF24L01_ALL_RX_PIPES){
		changes = NRF24L01_EN_AA_MASK_REG;
	}else{
		changes = (1 << pipe);
	}

	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_EN_AA, &reg, platform);
	if(aa_mode == NRF24L01_AA_ON){
		reg |= changes;
	}else if(aa_mode == NRF24L01_AA_OFF){
		reg &= ~changes;
	}else{
		return NRF24L01_ERR_INVALID_ARG;
	}
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_EN_AA, reg, platform);

	return err;
}

nrf24l01_err_t nrf24l01_set_pipe_dpl_mode(nrf24l01_pipe_t pipe, nrf24l01_dpl_mode_t dpl_mode, nrf24l01_platform_t* platform){
	nrf24l01_err_t err = NRF24L01_OK;
	
	if( !NRF24L01_IS_RX_PIPE(pipe) ){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t reg;
	uint8_t changes;

	if(pipe == NRF24L01_ALL_RX_PIPES){
		changes = NRF24L01_DYNPD_MASK_REG;
	}else{
		changes = (1 << pipe);
	}

	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_DYNPD, &reg, platform);
	if(dpl_mode == NRF24L01_DPL_ON){
		reg |= changes;
	}else if(dpl_mode == NRF24L01_DPL_OFF){
		reg &= ~changes;
	}else{
		return NRF24L01_ERR_INVALID_ARG;
	}
	err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_DYNPD, reg, platform);

	return err;
}


// Get value of the STATUS register
// return: value of STATUS register
nrf24l01_err_t nrf24l01_get_status(uint8_t* status, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;

	if(status == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}
	// The status register is shifted out on MISO regardless of address provided on MOSI so the status register address is provided primarily for debugging visibility here
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_STATUS, status, platform);
	return err;
}

// Get pending IRQ flags
// return: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
nrf24l01_err_t nrf24l01_get_irq_flags(uint8_t* flags, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	uint8_t temp;

	if(flags == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}

	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_STATUS, &temp, platform);
	if(err == NRF24L01_OK){
		*flags = temp & NRF24L01_STATUS_MASK_IRQ_FLAGS;
	}
	return err;
}

nrf24l01_err_t nrf24l01_set_irq_mask(nrf24l01_interrupt_mask_t mask, nrf24l01_platform_t* platform){
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// The PLOS counter is reset after write to RF_CH register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	reg &= ~NRF24L01_CONFIG_MASK_INTERRUPT_MASKS;
	reg |= (mask & NRF24L01_CONFIG_MASK_INTERRUPT_MASKS);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_CONFIG, reg, platform);
	return err;
}

// Get status of the RX FIFO
// return: one of the nRF24_STATUS_RXFIFO_xx values
nrf24l01_err_t nrf24l01_get_fifo_status(nrf24l01_fifo_type_t fifo_type, nrf24l01_fifo_status_t* fifo_status, nrf24l01_platform_t* platform){
	nrf24l01_err_t err;

	if(fifo_status == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}

	uint8_t temp;
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_FIFO_STATUS, &temp, platform);
	if(err == NRF24L01_OK){
		if(fifo_type == NRF24L01_RX_FIFO){
			*fifo_status = temp & NRF24L01_FIFO_STATUS_MASK_RX_FLAGS;
		}else if(fifo_type == NRF24L01_TX_FIFO){
			*fifo_status = (temp & NRF24L01_FIFO_STATUS_MASK_TX_FLAGS) >> 4;
		}else{
			return NRF24L01_ERR_INVALID_ARG;
		}
		return NRF24L01_OK;
	}else{
		return err;
	}
}


// Get pipe number for the payload available for reading from RX FIFO
// return: pipe number or 0x07 if the RX FIFO is empty
nrf24l01_err_t nrf24l01_get_rx_pipe(nrf24l01_pipe_t* pipe, nrf24l01_platform_t* platform) {
	
	if(pipe == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}

	uint8_t status;
	nrf24l01_err_t err = nrf24l01_read_reg(NRF24L01_REG_ADDR_STATUS, &status, platform);
	if(err != NRF24L01_OK){
		return err;
	}

	*pipe = (status & NRF24L01_STATUS_REG_BITS_RX_P_NO) >> 1;
	return NRF24L01_OK;
}

// Get auto retransmit statistic
// return: value of OBSERVE_TX register which contains two counters encoded in nibbles:
//   high - lost packets count (max value 15, can be reset by a write to RF_CH register)
//   low  - retransmitted packets count (max value 15, resets when a new transmission starts)
nrf24l01_err_t nrf24l01_get_retransmit_counters(nrf24l01_ar_count_t* ar_count, nrf24l01_ar_lost_t* ar_lost, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;

	if(ar_count == NULL || ar_lost == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}

	uint8_t counters;
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_OBSERVE_TX, &counters, platform);
	if(err != NRF24L01_OK){
		return err;
	}

	*ar_lost =  (NRF24L01_OBSERVE_TX_REG_BITS_PLOS_CNT & counters) >> 4;
	*ar_count = NRF24L01_OBSERVE_TX_REG_BITS_ARC_CNT & counters;
	return NRF24L01_OK;
}

// Get the configured address width
// output:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
nrf24l01_err_t nrf24l01_get_address_width(nrf24l01_address_width_t* addr_width, nrf24l01_platform_t* platform){
	nrf24l01_err_t err;

	uint8_t temp;
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_SETUP_AW, &temp, platform);
	temp = temp & NRF24L01_SETUP_AW_REG_BITS_AW;

	if(temp == NRF24L01_ADDR_WIDTH_3_BYTES){
		*addr_width = 3;
	}else if(temp == NRF24L01_ADDR_WIDTH_4_BYTES){
		*addr_width = 4;
	}else if(temp == NRF24L01_ADDR_WIDTH_5_BYTES){
		*addr_width = 5;
	}else{
		err = NRF24L01_ERR_READ;
	}
	return err;
}

// Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
nrf24l01_err_t nrf24l01_reset_packet_loss_counter(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// The PLOS counter is reset after write to RF_CH register
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_RF_CH, &reg, platform);
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_RF_CH, reg, platform);
	return err;
}

// Flush the TX FIFO
nrf24l01_err_t nrf24l01_flush_tx(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_CMD_FLUSH_TX, NRF24L01_CMD_NOP, platform);
	return err;
}

// Flush the RX FIFO
nrf24l01_err_t nrf24l01_flush_rx(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_CMD_FLUSH_RX, NRF24L01_CMD_NOP, platform);
	return err;
}

// Clear any pending IRQ flags
nrf24l01_err_t nrf24l01_clear_irq_flags(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	
	// Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
	uint8_t reg;
	err |= nrf24l01_read_reg(NRF24L01_REG_ADDR_STATUS, &reg, platform);
	reg |= NRF24L01_STATUS_MASK_IRQ_FLAGS;
	err |= nrf24l01_write_reg(NRF24L01_REG_ADDR_STATUS, reg, platform);

	return err;
}

// Write TX payload
// input:
//   data - pointer to the buffer with payload data, FIFO is 32 bytes long
//   len - payload length in bytes
nrf24l01_err_t nrf24l01_write_payload(uint8_t* data, uint8_t len, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;

	if(data == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}else if(len > 32){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}

	NRF24L01_FPTR_RTN_T spi_err = platform->spi_exchange(NRF24L01_CMD_W_TX_PAYLOAD, NULL, data, len, platform->user_ptr);
	if(spi_err != 0){
		err = NRF24L01_ERR_WRITE;
	}else{
		err = NRF24L01_OK;
	}
	
	platform->gpio_chip_enable(true, platform->user_ptr);
	platform->delay_us(NRF24L01_CE_TX_MINIMUM_PULSE_US);
	platform->gpio_chip_enable(false, platform->user_ptr);
	return err;
}


static nrf24l01_err_t nrf24l01_get_rx_dpl(uint8_t* width, nrf24l01_platform_t* platform) {
	NRF24L01_FPTR_RTN_T spi_err = platform->spi_exchange(NRF24L01_FEATURE_CMD_R_RX_PL_WID, width, NULL, sizeof(uint8_t), platform->user_ptr);
	if(spi_err != 0){
		return NRF24L01_ERR_WRITE;
	}
	return NRF24L01_OK;
}

nrf24l01_err_t nrf24l01_read_payload(nrf24l01_pipe_t* pipe, uint8_t* rx_data, uint8_t* len, bool dpl, nrf24l01_platform_t* platform) {

	nrf24l01_err_t err;

	// Extract a payload pipe number from the STATUS register
	nrf24l01_pipe_t payload_pipe;
	nrf24l01_get_rx_pipe(&payload_pipe, platform);
	if(payload_pipe == NRF24L01_PIPE_UNKNOWN){
		return NRF24L01_ERR_INVALID_STATE;
	}

	// Get payload length
	uint8_t payload_len;
	if(dpl) {
		nrf24l01_get_rx_dpl(&payload_len, platform);

		//Check for broken packet
		if(payload_len > NRF24L01_RX_FIFO_WIDTH) { 
			nrf24l01_flush_rx(platform);
			return NRF24L01_ERR_UNKNOWN;
		}
	}else {
		nrf24l01_read_reg(NRF24L01_RX_PW_PIPE[payload_pipe], &payload_len, platform);
	}
	
	// Return data
	if(rx_data != NULL){
		//If buffer size was passed in, check that it is large enough before reading
		if(len != NULL && *len < payload_len){
			return NRF24L01_ERR_INVALID_ARG;
		}

		NRF24L01_FPTR_RTN_T spi_err = platform->spi_exchange(NRF24L01_CMD_R_RX_PAYLOAD, rx_data, NULL, payload_len, platform->user_ptr);
		if(spi_err != 0){
			err = NRF24L01_ERR_WRITE;
		}else{
			err = NRF24L01_OK;
		}
	}
	if(len != NULL){
		*len = payload_len;
	}
	if(pipe != NULL){
		*pipe = payload_pipe;
	}
	return NRF24L01_OK;
}

/*
// Read top level payload available in the RX FIFO
// input:
//   rx_data - pointer to the buffer to store a payload data
//   length - pointer to variable to store a payload length
//   platform - 
// return: one of nRF24_RX_xx values
//   nRF24_RX_PIPEX - packet has been received from the pipe number X
//   nRF24_RX_EMPTY - the RX FIFO is empty
nrf24l01_err_t nrf24l01_read_payload(nrf24l01_pipe_t* pipe, uint8_t* rx_data, uint8_t* len, nrf24l01_platform_t* platform) {
	return nrf24l01_read_payload_generic(pipe, rx_data, len, 0, platform);
}
nrf24l01_err_t nrf24l01_read_dynamic_length_payload(nrf24l01_pipe_t* pipe, uint8_t* rx_data, uint8_t* len, nrf24l01_platform_t* platform) {
	return nrf24l01_read_payload_generic(pipe,rx_data, len, 1, platform);
}
*/


nrf24l01_err_t nrf24l01_get_features(uint8_t* features, nrf24l01_platform_t* platform) {
    return nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, features, platform);
}

nrf24l01_err_t nrf24l01_get_power_mode(nrf24l01_power_mode_t* power_mode, nrf24l01_platform_t* platform){
	nrf24l01_err_t err;

	uint8_t reg;
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	if(err == NRF24L01_OK){
		if(reg & NRF24L01_CONFIG_REG_BIT_PWR_UP){
			*power_mode = NRF24L01_PWR_UP;
		}else{
			*power_mode = NRF24L01_PWR_DOWN;
		}
	}
	return err;
}

nrf24l01_err_t nrf24l01_get_operational_mode(nrf24l01_operational_mode_t* operational_mode, nrf24l01_platform_t* platform){
	nrf24l01_err_t err;

	uint8_t reg;
	err = nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg, platform);
	if(err == NRF24L01_OK){
		if(reg & NRF24L01_CONFIG_REG_BIT_PRIM_RX){
			*operational_mode = NRF24L01_MODE_RX;
		}else{
			*operational_mode = NRF24L01_MODE_TX;
		}
	}
	return err;
}

nrf24l01_err_t nrf24l01_get_feature_mode(nrf24l01_feature_mode_t* feature_mode, nrf24l01_platform_t* platform) {
	// There is no direct way to query if the NRF24L01 extra features have been activated or not
	//    Read the state of the FEATURES register (will be all zero if features deactivated)
	//    Attempt to change EN_DYN_ACK (bit 0) of the FEATURES register since it doesn't change the system state in any way except the make an additional command available
	//    Read back the FEATURES register to see if the bit changed
	//    If FEATURES register changed, write the original value back

	if(feature_mode == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}
	nrf24l01_err_t err = NRF24L01_OK;

	// Backupt the features register
	uint8_t features_reg_backup;
	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, &features_reg_backup, platform);

	//Toggle a bit
	err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, features_reg_backup ^ NRF24L01_FEATURE_REG_BIT_EN_DYN_ACK, platform);

	uint8_t features_reg_result;
	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, &features_reg_result, platform);

	if(features_reg_result != features_reg_backup){
		*feature_mode = NRF24L01_FEATURES_ON;
	}else{
		*feature_mode = NRF24L01_FEATURES_OFF;
	}

	//Restore the previous state of the features register
	err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, features_reg_backup, platform);

	return err;

}

/** @brief 
 * 
 *  @param
 *  @param[in] platform : 
 * 
 */
nrf24l01_err_t nrf24l01_set_feature_mode(nrf24l01_feature_mode_t feature_mode, nrf24l01_platform_t* platform){
	
	//Check if we are in power down or standby mode
	nrf24l01_power_mode_t power_mode;
	nrf24l01_get_power_mode(&power_mode, platform);
	if(power_mode != NRF24L01_PWR_DOWN){
		return NRF24L01_ERR_INVALID_STATE;
	}

	//Change the feature mode if needed
	nrf24l01_feature_mode_t current_feature_mode;
	nrf24l01_get_feature_mode(&current_feature_mode, platform);
	if(feature_mode != current_feature_mode){
		//Toggle the feature activation using the ACTIVATE command

		uint8_t magic_number = NRF24L01_MAGIC_NUMBER_ACTIVATE;
		NRF24L01_FPTR_RTN_T spi_err = platform->spi_exchange(NRF24L01_CMD_ACTIVATE, NULL, &magic_number, sizeof(magic_number), platform->user_ptr);
		if(spi_err != 0){
			return NRF24L01_ERR_WRITE;
		}
	}
	return NRF24L01_OK;
}

nrf24l01_err_t nrf24l01_set_dpl_mode(nrf24l01_dpl_mode_t dpl_mode, nrf24l01_platform_t* platform){

	nrf24l01_err_t err = NRF24L01_OK;

	uint8_t features_reg;
	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, &features_reg, platform);
	if(dpl_mode == NRF24L01_DPL_ON){
		features_reg |= NRF24L01_FEATURE_REG_BIT_EN_DPL;
	}else{
		features_reg &= ~NRF24L01_FEATURE_REG_BIT_EN_DPL;
	}
	err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, features_reg, platform);
	return err;
}

/*
nrf24l01_err_t nrf24l01_write_ack_payload(nrf24l01_pipe_t pipe, uint8_t* payload, uint8_t len, nrf24l01_platform_t* platform) {
	
	nrf24l01_CSN_L();
	nrf24l01_LL_RW(NRF24L01_CMD_W_ACK_PAYLOAD | pipe);
	while (length--) {
		nrf24l01_LL_RW((uint8_t)* payload++);
	}
	nrf24l01_CSN_H();
	

	
	nrf24l01_err_t err;
	if(data == NULL){
		err = NRF24L01_INVALID_ARG;
		return err;
	}
	err = nrf24l01_multi_write_reg(NRF24L01_CMD_W_TX_PAYLOAD, data, len, platform);
	return err;
	

	return NRF24L01_OK;
}
*/


/*
// Enables Payload With Ack. NB Refer to the datasheet for proper retransmit timing.
// input:
//   mode - status, 1 or 0
nrf24l01_err_t nrf24l01_set_payload_with_ack_mode(uint8_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR, &reg, platform);
	if(mode) {
		err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR, reg | NRF24L01_FEATURE_EN_ACK_PAY, platform);
	} else {
		err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR, reg &~ NRF24L01_FEATURE_EN_ACK_PAY, platform);
	}

	return err;
}
*/







#ifdef NRF24L01_ENABLE_PRINT_CONFIG

// Print nRF24L01+ current configuration (for debug purposes)
void nrf24l01_print_config(nrf24l01_platform_t* platform) {
	
	uint8_t reg_temp; //Useful when reading single register
	uint8_t buf[5];   //Useful when reading set of registers

	//Field widths (Does not include single-space padding on either side of data)
	int addr_width = 5;
	int name_width = 12;
	int hex_width = 8;
	int bin_width = 12;
	int details_width = 40;

	//Horizontal divider (must be long enough to account for the larger column width above)
	const char* divider = "----------------------------------------"; //40 chars

	// Print horizontal divider
	NRF24L01_DEBUGGING_PRINTF("+%.*s+%.*s+%.*s+%.*s+%.*s\n",
		addr_width+2, divider, name_width+2, divider, hex_width+2, divider, bin_width+2, divider, details_width+2, divider
	);

	// Print header
	NRF24L01_DEBUGGING_PRINTF("| %-*s | %-*s | %-*s | %-*s | %-*s \n",
		addr_width, "addr",
		name_width, "name",
		hex_width, "raw hex", 
		bin_width, "raw bin",
		details_width, "details"
	);
	// Print horizontal divider
	NRF24L01_DEBUGGING_PRINTF("+%.*s+%.*s+%.*s+%.*s+%.*s\n",
		addr_width+2, divider, name_width+2, divider, hex_width+2, divider, bin_width+2, divider, details_width+2, divider
	);

	// CONFIG
	nrf24l01_read_reg(NRF24L01_REG_ADDR_CONFIG, &reg_temp, platform);	
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | INT MASK="_3BIT_FMT" (RX_DR=%s, TX_DS=%s, MAX_RT=%s), CRC="_2BIT_FMT" (%s), PWR=%s, MODE=%s\n",
		addr_width-2, NRF24L01_REG_ADDR_CONFIG,
		name_width, "CONFIG",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		_3BIT_STR((reg_temp & NRF24L01_CONFIG_MASK_INTERRUPT_MASKS) >> 4), 
		(reg_temp & NRF24L01_CONFIG_REG_BIT_MASK_RX_DR)  ? "OFF" : "ON",
		(reg_temp & NRF24L01_CONFIG_REG_BIT_MASK_TX_DS)  ? "OFF" : "ON",
		(reg_temp & NRF24L01_CONFIG_REG_BIT_MASK_MAX_RT) ? "OFF" : "ON",
		_2BIT_STR((reg_temp & NRF24L01_CONFIG_MASK_CRC) >> 2), (reg_temp & NRF24L01_CONFIG_REG_BIT_EN_CRC) ? ( (reg_temp & NRF24L01_CONFIG_REG_BIT_CRCO) ? "2 BYTES": "1 BYTE"): "DISABLED",
		(reg_temp & NRF24L01_CONFIG_REG_BIT_PWR_UP) ? "POWER UP" : "POWER DOWN", (reg_temp & NRF24L01_CONFIG_REG_BIT_PRIM_RX) ? "RX" : "TX"
	);
	
	// EN_AA
	nrf24l01_read_reg(NRF24L01_REG_ADDR_EN_AA, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | Enhanced ShockBurst AA=%s, ",
		addr_width-2, NRF24L01_REG_ADDR_EN_AA,
		name_width, "EN_AA",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		(reg_temp & NRF24L01_EN_AA_MASK_REG)  ? "ON" : "OFF"
	);
	for (int8_t i = 5; i >= 0; i--) {
		NRF24L01_DEBUGGING_PRINTF("PIPE%1u=%s%s", i, (reg_temp & (1 << i)) ? "YES" : "NO", (i == 0) ? "\n" : ", ");
	}

	// EN_RX_ADDR
	nrf24l01_read_reg(NRF24L01_REG_ADDR_EN_RXADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | Enabled pipes are ",
		addr_width-2, NRF24L01_REG_ADDR_EN_RXADDR,
		name_width, "EN_RXADDR",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, ""
	);
	for (int8_t i = 5; i >= 0; i--) {
		NRF24L01_DEBUGGING_PRINTF("PIPE%1u=%s%s", i, (reg_temp & (1 << i)) ? "EN" : "DIS", (i == 0) ? "\n" : ", ");
	}

	// SETUP_AW
	nrf24l01_read_reg(NRF24L01_REG_ADDR_SETUP_AW, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | AW="_2BIT_FMT" ",
		addr_width-2, NRF24L01_REG_ADDR_SETUP_AW,
		name_width, "SETUP_AW",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		_2BIT_STR(reg_temp & NRF24L01_SETUP_AW_REG_BITS_AW)
	);
	switch(reg_temp & NRF24L01_SETUP_AW_REG_BITS_AW){
		case NRF24L01_ADDR_WIDTH_INVALID:
			NRF24L01_DEBUGGING_PRINTF("(Illegal)\n");
			break;
		case NRF24L01_ADDR_WIDTH_3_BYTES:
			NRF24L01_DEBUGGING_PRINTF("(3 byte address)\n");
			break;
		case NRF24L01_ADDR_WIDTH_4_BYTES:
			NRF24L01_DEBUGGING_PRINTF("(4 byte address)\n");
			break;
		case NRF24L01_ADDR_WIDTH_5_BYTES:
			NRF24L01_DEBUGGING_PRINTF("(5 byte address)\n");
			break;
	}

	// SETUP_RETR
	nrf24l01_read_reg(NRF24L01_REG_ADDR_SETUP_RETR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | ARD="_4BIT_FMT" (wait %d+86us), ARC="_4BIT_FMT" (up to %d retransmits)\n",
		addr_width-2, NRF24L01_REG_ADDR_SETUP_RETR,
		name_width, "SETUP_RETR",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		_4BIT_STR(reg_temp & NRF24L01_SETUP_RETR_REG_BITS_ARD), 250*(reg_temp & NRF24L01_SETUP_RETR_REG_BITS_ARD)+250,
		_4BIT_STR(reg_temp & NRF24L01_SETUP_RETR_REG_BITS_ARC), reg_temp & NRF24L01_SETUP_RETR_REG_BITS_ARC
	);

	// RF_CH
	nrf24l01_read_reg(NRF24L01_REG_ADDR_RF_CH, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | CHANNEL=%d (%.3uMHz)\n",
		addr_width-2, NRF24L01_REG_ADDR_RF_CH,
		name_width, "RF_CH",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		(reg_temp & NRF24L01_RF_CH_REG_BITS_RF_CH), 2400 + (reg_temp & NRF24L01_RF_CH_REG_BITS_RF_CH)
	);

	// RF_SETUP
	nrf24l01_read_reg(NRF24L01_REG_ADDR_RF_SETUP, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | PLL_LOCK=%s, RF_DR=%s, ",
		addr_width-2, NRF24L01_REG_ADDR_RF_SETUP,
		name_width, "RF_SETUP",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		(reg_temp & NRF24L01_RF_SETUP_REG_BIT_PLL_LOCK) ? "EN" : "DIS",
		(reg_temp & NRF24L01_RF_SETUP_REG_BIT_RF_DR) ? "2 Mbps" : "1 Mbps"
	);
	switch (reg_temp & NRF24L01_RF_SETUP_REG_BITS_RF_PWR) {
		case NRF24L01_TX_PWR_18DBM:
			NRF24L01_DEBUGGING_PRINTF("RF_PWR=-18dBm, ");
			break;
		case NRF24L01_TX_PWR_12DBM:
			NRF24L01_DEBUGGING_PRINTF("RF_PWR=-12dBm, ");
			break;
		case NRF24L01_TX_PWR_6DBM:
			NRF24L01_DEBUGGING_PRINTF("RF_PWR=-6dBm, ");
			break;
		case NRF24L01_TX_PWR_0DBM:
			NRF24L01_DEBUGGING_PRINTF("RF_PWR=0dBm, ");
			break;
	}
	NRF24L01_DEBUGGING_PRINTF("LNA_HCURR=%s\n", (reg_temp & NRF24L01_RF_SETUP_REG_BIT_LNA_HCURR) ? "YES" : "NO");

	// STATUS
	nrf24l01_read_reg(NRF24L01_REG_ADDR_STATUS, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | INT FLAGS="_3BIT_FMT" (RX_DR=%s, TX_DS=%s, MAX_RT=%s), RX_P_NO="_3BIT_FMT" ",
		addr_width-2, NRF24L01_REG_ADDR_STATUS,
		name_width, "STATUS",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		_3BIT_STR((reg_temp & NRF24L01_STATUS_MASK_IRQ_FLAGS) >> 4), 
		(reg_temp & NRF24L01_STATUS_REG_BIT_RX_DR)  ? "INT" : "NO",
		(reg_temp & NRF24L01_STATUS_REG_BIT_TX_DS)  ? "INT" : "NO",
		(reg_temp & NRF24L01_STATUS_REG_BIT_MAX_RT) ? "INT" : "NO",
		_3BIT_STR((reg_temp & NRF24L01_STATUS_REG_BITS_RX_P_NO) >> 1)
	);
	switch ((reg_temp & NRF24L01_STATUS_REG_BITS_RX_P_NO) >> 1) {
        case 0: NRF24L01_DEBUGGING_PRINTF("(PIPE0), "); break;
        case 1: NRF24L01_DEBUGGING_PRINTF("(PIPE1), "); break;
		case 2: NRF24L01_DEBUGGING_PRINTF("(PIPE2), "); break;
		case 3: NRF24L01_DEBUGGING_PRINTF("(PIPE3), "); break;
		case 4: NRF24L01_DEBUGGING_PRINTF("(PIPE4), "); break;
		case 5: NRF24L01_DEBUGGING_PRINTF("(PIPE0), "); break;
		case 7: NRF24L01_DEBUGGING_PRINTF("(RX FIFO Empty), "); break;
        default: NRF24L01_DEBUGGING_PRINTF("(Unknown), "); break;
    }
	NRF24L01_DEBUGGING_PRINTF("TX_FULL=%s\n", (reg_temp & NRF24L01_STATUS_REG_BIT_TX_FULL) ? "YES" : "NO");

	// OBSERVE_TX
	nrf24l01_read_reg(NRF24L01_REG_ADDR_OBSERVE_TX, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | PLOS_CNT="_4BIT_FMT", ARC_CNT="_4BIT_FMT"\n",
		addr_width-2, NRF24L01_REG_ADDR_OBSERVE_TX,
		name_width, "OBSERVE_TX",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		_4BIT_STR(reg_temp & NRF24L01_OBSERVE_TX_REG_BITS_PLOS_CNT),
		_4BIT_STR(reg_temp & NRF24L01_OBSERVE_TX_REG_BITS_ARC_CNT)
	);

	// CD
	nrf24l01_read_reg(NRF24L01_REG_ADDR_CD, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | CD=%s\n",
		addr_width-2, NRF24L01_REG_ADDR_CD,
		name_width, "CD",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
		(reg_temp & NRF24L01_CD_REG_BIT_CD) ? "YES" : "NO"
	);

	// Get the configured pipe address width and cache it
	uint8_t address_width;
	nrf24l01_get_address_width(&address_width, platform);
	
	//Print out all the pipe rx pipe addresses
	for(uint8_t i = 0; i <= 5; i++){
		char name_buf[] = "RX_ADDR_Px";
		if(i == 0 || i == 1){
			nrf24l01_multi_read_reg(NRF24L01_REG_ADDR_RX_ADDR_P0 + i, buf, address_width, platform);
		}else{
			nrf24l01_read_reg(NRF24L01_REG_ADDR_RX_ADDR_P0 + i, &buf[address_width-1], platform);
		}
		name_buf[9] = (char) i+48; //Convert the index to a character and replace 'x'
		NRF24L01_DEBUGGING_PRINTF(
			"| 0x%-*.2X | %-*s | -%*s | -%*s | RX PIPE%d ADDRESS=[",
			addr_width-2, NRF24L01_REG_ADDR_RX_ADDR_P0 + i,
			name_width, name_buf,
			hex_width-1, "",
			bin_width-1, "",
			/*Details*/
			i
		);
		for (uint8_t i = 0; i < address_width; i++){
			NRF24L01_DEBUGGING_PRINTF("0x%02X%s", buf[i], (i == (address_width - 1)) ? "]\n" : ", ");
		}
	}

	//Print out the tx pipe address
	nrf24l01_multi_read_reg(NRF24L01_REG_ADDR_TX_ADDR, buf, address_width, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | -%*s | -%*s | TX PIPE ADDRESS=[",
		addr_width-2, NRF24L01_REG_ADDR_TX_ADDR,
		name_width, "TX_ADDR",
		hex_width-1, "",
		bin_width-1, ""
	);
	for (uint8_t i = 0; i < address_width; i++){
		NRF24L01_DEBUGGING_PRINTF("0x%02X%s", buf[i], (i == (address_width - 1)) ? "]\n" : ", ");
	}

	//Print out all the pipe rx payload lengths
	for(uint8_t i = 0; i <= 5; i++){
		char name_buf[] = "RX_PW_Px";
		nrf24l01_read_reg(NRF24L01_REG_ADDR_RX_PW_P0 + i, &reg_temp, platform);
		name_buf[7] = (char) i+48; //Convert the index to a character and replace 'x'
		NRF24L01_DEBUGGING_PRINTF(
			"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | PIPE%d RX PAYLOAD LENGTH=%d\n",
			addr_width-2, NRF24L01_REG_ADDR_RX_PW_P0 + i,
			name_width, name_buf,
			hex_width-2, reg_temp,
			_8BIT_STR(reg_temp), bin_width-10, "",
			/*Details*/
			i, reg_temp & NRF24L01_RX_PW_P0_REG_BITS_RX_PW_P0
		);
	}

	// FIFO_STATUS
	nrf24l01_read_reg(NRF24L01_REG_ADDR_FIFO_STATUS, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF(
		"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | TX_REUSE=%s, TX_FULL=%s, TX_EMPTY=%s, RX_FULL=%s, RX_EMPTY=%s\n",
		addr_width-2, NRF24L01_REG_ADDR_FIFO_STATUS,
		name_width, "FIFO_STATUS",
		hex_width-2, reg_temp,
		_8BIT_STR(reg_temp), bin_width-10, "",
		/*Details*/
        (reg_temp & NRF24L01_FIFO_STATUS_REG_BIT_TX_REUSE) ? "YES" : "NO", 
        (reg_temp & NRF24L01_FIFO_STATUS_REG_BIT_TX_FULL)  ? "YES" : "NO",
        (reg_temp & NRF24L01_FIFO_STATUS_REG_BIT_TX_EMPTY) ? "YES" : "NO",
        (reg_temp & NRF24L01_FIFO_STATUS_REG_BIT_RX_FULL)  ? "YES" : "NO",
        (reg_temp & NRF24L01_FIFO_STATUS_REG_BIT_RX_EMPTY) ? "YES" : "NO"
	);

	// Print horizontal divider
	NRF24L01_DEBUGGING_PRINTF("+%.*s+%.*s+%.*s+%.*s+%.*s\n",
		addr_width+2, divider, name_width+2, divider, hex_width+2, divider, bin_width+2, divider, details_width+2, divider
	);

	//Check if special features are active
	nrf24l01_feature_mode_t feature_mode;
	nrf24l01_get_feature_mode(&feature_mode, platform);
	if(feature_mode == NRF24L01_FEATURES_OFF){
		NRF24L01_DEBUGGING_PRINTF("| Special Features Disabled\n");
	}else{
		// DYNPD
		nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_DYNPD, &reg_temp, platform);
		NRF24L01_DEBUGGING_PRINTF(
			"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | Dynamic payloads  ",
			addr_width-2, NRF24L01_FEATURE_REG_ADDR_DYNPD,
			name_width, "DYNPD",
			hex_width-2, reg_temp,
			_8BIT_STR(reg_temp), bin_width-10, ""
		);
		for (int8_t i = 5; i >= 0; i--) {
			NRF24L01_DEBUGGING_PRINTF("DPL_P%1u=%s%s", i, (reg_temp & (1 << i)) ? "EN" : "DIS", (i == 0) ? "\n" : ", ");
		}

		// FEATURE
		nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR_FEATURE, &reg_temp, platform);
		NRF24L01_DEBUGGING_PRINTF(
			"| 0x%-*.2X | %-*s | 0x%-*.2X | "_8BIT_FMT"%*s | Features  EN_DPL=%s, EN_ACK_PAY=%s, EN_DYN_ACK=%s\n",
			addr_width-2, NRF24L01_FEATURE_REG_ADDR_FEATURE,
			name_width, "FEATURE",
			hex_width-2, reg_temp,
			_8BIT_STR(reg_temp), bin_width-10, "",
			(reg_temp & NRF24L01_FEATURE_REG_BIT_EN_DPL) ? "EN" : "DIS",
			(reg_temp & NRF24L01_FEATURE_REG_BIT_EN_ACK_PAY) ? "EN" : "DIS",
			(reg_temp & NRF24L01_FEATURE_REG_BIT_EN_DYN_ACK) ? "EN" : "DIS"
		);
	}

	// Print horizontal divider
	NRF24L01_DEBUGGING_PRINTF("+%.*s+%.*s+%.*s+%.*s+%.*s\n",
		addr_width+2, divider, name_width+2, divider, hex_width+2, divider, bin_width+2, divider, details_width+2, divider
	);

}

void nrf24l01_print_status_register(nrf24l01_platform_t* platform){
	uint8_t status;
    nrf24l01_get_status(&status, platform);

    char* pipe_number;
    switch((status & NRF24L01_STATUS_REG_BITS_RX_P_NO) >> 1){
        case 0: pipe_number = "0"; break;
        case 1: pipe_number = "1"; break;
        case 2: pipe_number = "2"; break;
        case 3: pipe_number = "3"; break;
        case 4: pipe_number = "4"; break;
        case 5: pipe_number = "5"; break;
        case 7: pipe_number = "RX FIFO Empty"; break;
        default: pipe_number = "?"; break;
    }
    NRF24L01_DEBUGGING_PRINTF("[0x%02X] STATUS_REG="_8BIT_FMT"=0x%02X (RX_DR=%s, TX_DS=%s, MAX_RT=%s, RX_P_NO=%s, TX_FULL=%s)\r\n",
        NRF24L01_REG_ADDR_STATUS,
		_8BIT_STR(status), status, 
        (status & NRF24L01_STATUS_REG_BIT_RX_DR)  ? "1" : "0", 
        (status & NRF24L01_STATUS_REG_BIT_TX_DS)  ? "1" : "0",
        (status & NRF24L01_STATUS_REG_BIT_MAX_RT) ? "1" : "0",
        pipe_number,
        (status & NRF24L01_STATUS_REG_BIT_TX_FULL) ? "1" : "0"
    );
}

void nrf24l01_print_fifo_status_register(nrf24l01_platform_t* platform){

	nrf24l01_fifo_status_t rx_status;
	nrf24l01_get_fifo_status(NRF24L01_RX_FIFO, &rx_status, platform);
	nrf24l01_fifo_status_t tx_status;
	nrf24l01_get_fifo_status(NRF24L01_TX_FIFO, &tx_status, platform);
	uint8_t fifo_status = (tx_status << 4) | rx_status;

	NRF24L01_DEBUGGING_PRINTF("[0x%02X] FIFO_STATUS_REG="_8BIT_FMT"=0x%02X (TX_REUSE=%s, TX_FULL=%s, TX_EMPTY=%s, RX_FULL=%s, RX_EMPTY=%s)\r\n",
        NRF24L01_REG_ADDR_FIFO_STATUS,
		_8BIT_STR(fifo_status), fifo_status, 
        (fifo_status & NRF24L01_FIFO_STATUS_REG_BIT_TX_REUSE) ? "1" : "0", 
        (fifo_status & NRF24L01_FIFO_STATUS_REG_BIT_TX_FULL)  ? "1" : "0",
        (fifo_status & NRF24L01_FIFO_STATUS_REG_BIT_TX_EMPTY) ? "1" : "0",
        (fifo_status & NRF24L01_FIFO_STATUS_REG_BIT_RX_FULL)  ? "1" : "0",
        (fifo_status & NRF24L01_FIFO_STATUS_REG_BIT_RX_EMPTY) ? "1" : "0"
    );
}

#endif // NRF24L01_ENABLE_PRINT_CONFIG





/*!
 * @brief Check for interrupt function pointer which should be called often by the user
 * to ensure that radio events are triggered
 *
 * @param[in] delay      : Number of microseconds to delay
 *
 * @retval 0        -> Interrupt triggered
 * @retval Non zero -> No interrupt
 */

void nrf24l01_loop(nrf24l01_platform_t* platform){
	NRF24L01_FPTR_RTN_T rtn = platform->check_for_interrupt(platform->user_ptr);

	if(rtn == 0){
		//Process interrupt and call all registered callbacks

		uint8_t irq_flags;
		nrf24l01_get_irq_flags(&irq_flags, platform);
		if( (irq_flags | NRF24L01_STATUS_REG_BIT_RX_DR) != 0 ){

			if(platform->callbacks.rx_dr_callback != NULL){
				platform->callbacks.rx_dr_callback(32, 0, platform->callbacks.rx_dr_callback_user_ptr, platform);
			}

		}else if( (irq_flags | NRF24L01_STATUS_REG_BIT_TX_DS) != 0 ){

			if(platform->callbacks.tx_ds_callback != NULL){
				platform->callbacks.tx_ds_callback(0, platform->callbacks.tx_ds_callback_user_ptr, platform);
			}

		}else if( (irq_flags | NRF24L01_STATUS_REG_BIT_MAX_RT) != 0 ){

			if(platform->callbacks.max_rt_callback != NULL){
				platform->callbacks.max_rt_callback(platform->callbacks.max_rt_callback_user_ptr, platform);
			}

		}
		nrf24l01_clear_irq_flags(platform);

		return;
	}else{
		return;
	}
}



nrf24l01_err_t nrf24l01_register_rx_dr_callback(nrf24l01_rx_dr_callback_fptr_t callback, void* user_ptr, nrf24l01_platform_t* platform){
	platform->callbacks.rx_dr_callback = callback;
	platform->callbacks.rx_dr_callback_user_ptr = user_ptr;

	return NRF24L01_OK;
}

nrf24l01_err_t nrf24l01_register_tx_ds_callback(nrf24l01_tx_ds_callback_fptr_t callback, void* user_ptr, nrf24l01_platform_t* platform){
	platform->callbacks.tx_ds_callback = callback;
	platform->callbacks.tx_ds_callback_user_ptr = user_ptr;

	return NRF24L01_OK;
}

nrf24l01_err_t nrf24l01_register_max_rt_callback(nrf24l01_max_rt_callback_fptr_t callback, void* user_ptr, nrf24l01_platform_t* platform){
	platform->callbacks.max_rt_callback = callback;
	platform->callbacks.max_rt_callback_user_ptr = user_ptr;

	return NRF24L01_OK;
}