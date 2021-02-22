
/**
 * @file
 * @author  Jared Poole <jarpoole>
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
static const uint8_t NRF24L01_RX_PW_PIPE[6] = {
	NRF24L01_RX_PW_P0_REG_ADDR,
	NRF24L01_RX_PW_P1_REG_ADDR,
	NRF24L01_RX_PW_P2_REG_ADDR,
	NRF24L01_RX_PW_P3_REG_ADDR,
	NRF24L01_RX_PW_P4_REG_ADDR,
	NRF24L01_RX_PW_P5_REG_ADDR,
};

// Address lookup table for the address registers
static const uint8_t NRF24L01_ADDR_REGS[7] = {
	NRF24L01_RX_ADDR_P0_REG_ADDR,
	NRF24L01_RX_ADDR_P1_REG_ADDR,
	NRF24L01_RX_ADDR_P2_REG_ADDR,
	NRF24L01_RX_ADDR_P3_REG_ADDR,
	NRF24L01_RX_ADDR_P4_REG_ADDR,
	NRF24L01_RX_ADDR_P5_REG_ADDR,
	NRF24L01_TX_ADDR_REG_ADDR,
};


// Read a register
// input:
//   reg - number of register to read
// return: value of register
static nrf24l01_err_t nrf24l01_read_reg(uint8_t reg_addr, uint8_t* data, nrf24l01_platform_t* platform) {
	if(data == NULL){
		return NRF24L01_ERR_INVALID_ARG;
	}
	uint8_t command = NRF24L01_CMD_R_REGISTER | (reg_addr & NRF24L01_MASK_REG_MAP);

	SPI_FPTR_RTN_T err = platform->spi_exchange(command, data, NULL, sizeof(uint8_t), platform->spi_user_config);
	if(err < 0){
		return NRF24L01_ERR_READ;
	}
	return NRF24L01_OK;
}

// Write a new value to register
// input:
//   reg - number of register to write
//   value - value to write
static nrf24l01_err_t nrf24l01_write_reg(uint8_t reg_addr, uint8_t data, nrf24l01_platform_t* platform) {
	uint8_t command = NRF24L01_CMD_W_REGISTER | (reg_addr & NRF24L01_MASK_REG_MAP);
	SPI_FPTR_RTN_T err = platform->spi_exchange(command, NULL, &data, sizeof(uint8_t), platform->spi_user_config);
	if(err < 0){
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
	uint8_t command = NRF24L01_CMD_R_REGISTER | (reg_addr & NRF24L01_MASK_REG_MAP);
	
	SPI_FPTR_RTN_T err = platform->spi_exchange(command, data, NULL, len, platform->spi_user_config);
	if(err < 0){
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
	uint8_t command = NRF24L01_CMD_W_REGISTER | (reg_addr & NRF24L01_MASK_REG_MAP);

	SPI_FPTR_RTN_T err = platform->spi_exchange(command, NULL, data, len, platform->spi_user_config);
	if(err < 0){
		return NRF24L01_ERR_WRITE;
	}
	return NRF24L01_OK;
}

// Set transceiver to it's initial state
// note: RX/TX pipe addresses remains untouched
nrf24l01_err_t nrf24l01_init(nrf24l01_platform_t* platform) {
	
	// Initialize the platform SPI controller
	SPI_FPTR_RTN_T init_err = platform->spi_init(platform->spi_user_config);
	if(init_err < 0){
		return NRF24L01_ERR_UNKNOWN;
	}

	// Write to registers their initial values
	nrf24l01_err_t err = NRF24L01_OK;
	err |= nrf24l01_write_reg(NRF24L01_CONFIG_REG_ADDR,     0x08, platform);
	err |= nrf24l01_write_reg(NRF24L01_EN_AA_REG_ADDR,      0x3F, platform);
	err |= nrf24l01_write_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, 0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_SETUP_AW_REG_ADDR,   0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_SETUP_RETR_REG_ADDR, 0x03, platform);
	err |= nrf24l01_write_reg(NRF24L01_RF_CH_REG_ADDR,      0x02, platform);
	err |= nrf24l01_write_reg(NRF24L01_RF_SETUP_REG_ADDR,   0x0E, platform);
	err |= nrf24l01_write_reg(NRF24L01_STATUS_REG_ADDR,     0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P0_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P1_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P2_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P3_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P4_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_RX_PW_P5_REG_ADDR,   0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_DYNPD_REG_ADDR,      0x00, platform);
	err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR,    0x00, platform);

	// Clear the FIFO's
	err |= nrf24l01_flush_rx(platform);
	err |= nrf24l01_flush_tx(platform);

	// Clear any pending interrupt flags
	uint8_t temp;
	err |= nrf24l01_get_irq_flags(&temp, platform);

	return err;
}

// Check if the nRF24L01 present
// return:
//   NRF24L01_OK                    - nRF24L01 is online and responding
//   NRF24L01_ERR_DEVICE_NOT_FOUND  - received sequence differs from original
nrf24l01_err_t nrf24l01_check_connectivity(nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;

	// Write test TX address
	uint8_t *ptr = (uint8_t *)NRF24L01_TEST_ADDR;
	err |= nrf24l01_multi_write_reg(NRF24L01_TX_ADDR_REG_ADDR, ptr, sizeof(NRF24L01_TEST_ADDR) - 1, platform);

	//Read TX_ADDR register
	uint8_t rxbuf[5];
	err |= nrf24l01_multi_read_reg(NRF24L01_TX_ADDR_REG_ADDR, &rxbuf[0], sizeof(rxbuf), platform);

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

// Control transceiver power mode
// input:
//   mode - new state of power mode, one of NRF24L01_PWR_xx values
nrf24l01_err_t nrf24l01_set_power_mode(uint8_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	err |= nrf24l01_read_reg(NRF24L01_CONFIG_REG_ADDR, &reg, platform);
	if (mode == NRF24L01_PWR_UP) {
		// Set the PWR_UP bit of CONFIG register to wake the transceiver
		// It goes into Stanby-I mode with consumption about 26uA
		reg |= NRF24L01_CONFIG_PWR_UP;
		err |= nrf24l01_write_reg(NRF24L01_CONFIG_REG_ADDR, reg, platform);
		
		platform->delay_us(NRF24L01_POWER_UP_US);
	} else {
		platform->gpio_chip_enable(false);

		// Clear the PWR_UP bit of CONFIG register to put the transceiver
		// into power down mode with consumption about 900nA
		reg &= ~NRF24L01_CONFIG_PWR_UP;
		err |= nrf24l01_write_reg(NRF24L01_CONFIG_REG_ADDR, reg, platform);
	}

	return err;
}

// Set transceiver operational mode
// input:
//   mode - operational mode, one of nRF24_MODE_xx values
nrf24l01_err_t nrf24l01_set_operational_mode(uint8_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure PRIM_RX bit of the CONFIG register
	err |= nrf24l01_read_reg(NRF24L01_CONFIG_REG_ADDR, &reg, platform);
	reg &= ~NRF24L01_CONFIG_PRIM_RX;
	reg |= (mode & NRF24L01_CONFIG_PRIM_RX);
	err |= nrf24l01_write_reg(NRF24L01_CONFIG_REG_ADDR, reg, platform);

	return err;
}

// Set transceiver DynamicPayloadLength feature for all the pipes
// input:
//   mode - status, one of nRF24_DPL_xx values
nrf24l01_err_t nrf24l01_set_dynamic_payload_length(uint8_t mode, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	err |= nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR, &reg, platform);
	if(mode) {
		err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR, reg | NRF24L01_FEATURE_EN_DPL, platform);
		err |= nrf24l01_write_reg(NRF24L01_DYNPD_REG_ADDR, 0x1F, platform);
	} else {
		err |= nrf24l01_write_reg(NRF24L01_FEATURE_REG_ADDR, reg &~ NRF24L01_FEATURE_EN_DPL, platform);
		err |= nrf24l01_write_reg(NRF24L01_DYNPD_REG_ADDR, 0x0, platform);
	}

	return err;
}

// Enables Payload With Ack. NB Refer to the datasheet for proper retransmit timing.
// input:
//   mode - status, 1 or 0
nrf24l01_err_t nrf24l01_set_payload_with_ack(uint8_t mode, nrf24l01_platform_t* platform) {
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

// Configure transceiver CRC scheme
// input:
//   scheme - CRC scheme, one of nRF24_CRC_xx values
// note: transceiver will forcibly turn on the CRC in case if auto acknowledgment
//       enabled for at least one RX pipe
nrf24l01_err_t nrf24l01_set_crc_scheme(uint8_t scheme, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register
	err |= nrf24l01_read_reg(NRF24L01_CONFIG_REG_ADDR, &reg, platform);
	reg &= ~NRF24L01_MASK_CRC;
	reg |= (scheme & NRF24L01_MASK_CRC);
	err |= nrf24l01_write_reg(NRF24L01_CONFIG_REG_ADDR, reg, platform);

	return err;
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
nrf24l01_err_t nrf24l01_set_rf_channel(uint8_t channel, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_RF_CH_REG_ADDR, channel, platform);
	return err;
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay, one of nRF24_ARD_xx values
//   arc - count of auto retransmits, value form 0 to 15
// note: zero arc value means that the automatic retransmission disabled
nrf24l01_err_t nrf24l01_set_auto_retransmission(uint8_t ard, uint8_t arc, nrf24l01_platform_t* platform) {
	// Set auto retransmit settings (SETUP_RETR register)
	nrf24l01_err_t err;
	err = nrf24l01_write_reg(NRF24L01_SETUP_RETR_REG_ADDR, (uint8_t)((ard << 4) | (arc & NRF24L01_MASK_RETR_ARC)), platform);
	return err;
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
nrf24l01_err_t nrf24l01_set_address_width(uint8_t addr_width, nrf24l01_platform_t* platform) {
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
	nrf24l01_err_t err = nrf24l01_write_reg(NRF24L01_SETUP_AW_REG_ADDR, reg_val, platform);
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
nrf24l01_err_t nrf24l01_set_address(uint8_t pipe, const uint8_t* addr, uint8_t len, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	
	if(pipe == NRF24L01_PIPE0 || pipe == NRF24L01_PIPE1 || pipe == NRF24L01_PIPETX){
		// Get the devices configured address width
		uint8_t addr_width;
		err = nrf24l01_get_address_width(&addr_width, platform);
		if(addr_width != len){
			err = NRF24L01_ERR_INVALID_ARG; 
		}else{
			// Write address in reverse order (LSByte first)
			err = nrf24l01_multi_write_reg(NRF24L01_ADDR_REGS[pipe], addr, len, platform);
		}
	}
	else if(pipe == NRF24L01_PIPE2 || pipe == NRF24L01_PIPE3 || pipe == NRF24L01_PIPE4 || pipe == NRF24L01_PIPE5){
		// Write address LSBbyte only (first byte from the addr buffer)
		err = nrf24l01_write_reg(NRF24L01_ADDR_REGS[pipe], *addr, platform);
	}else{
		// Incorrect pipe number -> do nothing
		err = NRF24L01_ERR_INVALID_ARG;
	}
	return err;
}

// Configure RF output power in TX mode
// input:
//   tx_pwr - RF output power, one of nRF24_TXPWR_xx values
nrf24l01_err_t nrf24l01_set_tx_power(uint8_t tx_pwr, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure RF_PWR[2:1] bits of the RF_SETUP register
	err |= nrf24l01_read_reg(NRF24L01_RF_SETUP_REG_ADDR, &reg, platform);
	reg &= ~NRF24L01_MASK_RF_PWR;
	reg |= tx_pwr;
	err |= nrf24l01_write_reg(NRF24L01_RF_SETUP_REG_ADDR, reg, platform);

	return err;
}

// Configure transceiver data rate
// input:
//   data_rate - data rate, one of nRF24_DR_xx values
nrf24l01_err_t nrf24l01_set_data_rate(uint8_t data_rate, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
	err |= nrf24l01_read_reg(NRF24L01_RF_SETUP_REG_ADDR, &reg, platform);
	reg &= ~NRF24L01_MASK_DATARATE;
	reg |= data_rate;
	err |= nrf24l01_write_reg(NRF24L01_RF_SETUP_REG_ADDR, reg, platform);

	return err;
}

// Configure a specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
//   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
//   payload_len - payload length in bytes
nrf24l01_err_t nrf24l01_set_rx_pipe(uint8_t pipe, uint8_t aa_state, uint8_t payload_len, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Enable the specified pipe (EN_RXADDR register)
	uint8_t temp;
	err |= nrf24l01_read_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, &temp, platform);
	reg = (temp | (1 << pipe)) & NRF24L01_MASK_EN_RX;
	err |= nrf24l01_write_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, reg, platform);

	// Set RX payload length (RX_PW_Px register)
	err |= nrf24l01_write_reg(NRF24L01_ADDR_REGS[pipe], payload_len & NRF24L01_MASK_RX_PW, platform);

	// Set auto acknowledgment for a specified pipe (EN_AA register)
	err |= nrf24l01_read_reg(NRF24L01_EN_AA_REG_ADDR, &reg, platform);
	if (aa_state == NRF24L01_AA_ON) {
		reg |=  (1 << pipe);
	} else {
		reg &= ~(1 << pipe);
	}
	err |= nrf24l01_write_reg(NRF24L01_EN_AA_REG_ADDR, reg, platform);

	return err;
}

// Disable specified RX pipe
// input:
//   PIPE - number of RX pipe, value from 0 to 5
nrf24l01_err_t nrf24l01_close_pipe(uint8_t pipe, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	err |= nrf24l01_read_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, &reg, platform);
	reg &= ~(1 << pipe);
	reg &= NRF24L01_MASK_EN_RX;
	err |= nrf24l01_write_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, reg, platform);

	return err;
}

// Enable the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
nrf24l01_err_t nrf24l01_enable_aa(uint8_t pipe, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	// Set bit in EN_AA register
	err |= nrf24l01_read_reg(NRF24L01_EN_AA_REG_ADDR, &reg, platform);
	reg |= (1 << pipe);
	err |= nrf24l01_write_reg(NRF24L01_EN_AA_REG_ADDR, reg, platform);

	return err;
}

// Disable the auto retransmit (a.k.a. enhanced ShockBurst) for one or all RX pipes
// input:
//   pipe - number of the RX pipe, value from 0 to 5, any other value will disable AA for all RX pipes
nrf24l01_err_t nrf24l01_disable_aa(uint8_t pipe, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err = NRF24L01_OK;
	uint8_t reg;

	if (pipe > 5) {
		// Disable Auto-ACK for ALL pipes
		err |= nrf24l01_write_reg(NRF24L01_EN_AA_REG_ADDR, 0x00, platform);
	} else {
		// Clear bit in the EN_AA register
		err |= nrf24l01_read_reg(NRF24L01_EN_AA_REG_ADDR, &reg, platform);
		reg &= ~(1 << pipe);
		err |= nrf24l01_write_reg(NRF24L01_EN_AA_REG_ADDR, reg, platform);
	}

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
	err = nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, status, platform);
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

	err = nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, &temp, platform);
	if(err == NRF24L01_OK){
		*flags = temp & NRF24L01_MASK_STATUS_IRQ;
	}
	return err;

}

// Get status of the RX FIFO
// return: one of the nRF24_STATUS_RXFIFO_xx values
nrf24l01_err_t nrf24l01_get_status_rx_fifo(uint8_t* status_rx, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	uint8_t temp;

	if(status_rx == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}

	err = nrf24l01_read_reg(NRF24L01_FIFO_STATUS_REG_ADDR, &temp, platform);
	if(err == NRF24L01_OK){
		*status_rx = temp & NRF24L01_MASK_RX_FIFO_STATUS;
	}
	return err;
}

// Get status of the TX FIFO
// return: one of the nRF24_STATUS_TXFIFO_xx values
// note: the TX_REUSE bit ignored
nrf24l01_err_t nrf24l01_get_status_tx_fifo(uint8_t* status_rx, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	uint8_t temp;
	err = nrf24l01_read_reg(NRF24L01_FIFO_STATUS_REG_ADDR, &temp, platform);
	if(err == NRF24L01_OK){
		*status_rx = (temp & NRF24L01_MASK_TX_FIFO_STATUS) >> 4;
	}
	return err;
}

// Get pipe number for the payload available for reading from RX FIFO
// return: pipe number or 0x07 if the RX FIFO is empty
nrf24l01_err_t nrf24l01_get_rx_source(uint8_t* source, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	uint8_t temp;

	if(source == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}

	err = nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, &temp, platform);
	if(err == NRF24L01_OK){
		*source = (temp & NRF24L01_MASK_STATUS_RX_P_NO) >> 1;
	}
	return err;
}

// Get auto retransmit statistic
// return: value of OBSERVE_TX register which contains two counters encoded in nibbles:
//   high - lost packets count (max value 15, can be reseted by write to RF_CH register)
//   low  - retransmitted packets count (max value 15, reseted when new transmission starts)
nrf24l01_err_t nrf24l01_get_retransmit_counters(uint8_t* counters, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;

	if(counters == NULL){
		err = NRF24L01_ERR_INVALID_ARG;
		return err;
	}

	err = nrf24l01_read_reg(NRF24L01_OBSERVE_TX_REG_ADDR, counters, platform);
	return err;
}

// Get the configured address width
// output:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
nrf24l01_err_t nrf24l01_get_address_width(uint8_t* addr_width, nrf24l01_platform_t* platform){
	nrf24l01_err_t err;

	uint8_t temp;
	err = nrf24l01_read_reg(NRF24L01_SETUP_AW_REG_ADDR, &temp, platform);
	temp = temp & NRF24L01_MASK_ADDR_WIDTH;

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
	err |= nrf24l01_read_reg(NRF24L01_RF_CH_REG_ADDR, &reg, platform);
	err |= nrf24l01_write_reg(NRF24L01_RF_CH_REG_ADDR, reg, platform);
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
	err |= nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, &reg, platform);
	reg |= NRF24L01_MASK_STATUS_IRQ;
	err |= nrf24l01_write_reg(NRF24L01_STATUS_REG_ADDR, reg, platform);

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

	SPI_FPTR_RTN_T spi_err = platform->spi_exchange(NRF24L01_CMD_W_TX_PAYLOAD, NULL, data, len, platform->spi_user_config);
	if(spi_err < 0){
		err = NRF24L01_ERR_WRITE;
	}else{
		err = NRF24L01_OK;
	}
	
	platform->gpio_chip_enable(true);
	platform->delay_us(NRF24L01_CE_TX_MINIMUM_PULSE_US);
	platform->gpio_chip_enable(false);
	return err;
}

static nrf24l01_err_t nrf24l01_get_rx_dpl_payload_width(uint8_t* width, nrf24l01_platform_t* platform) {
	nrf24l01_err_t err;
	err = nrf24l01_read_reg(NRF24L01_CMD_R_RX_PL_WID, width, platform);
	return err;

}

static nrf24l01_rx_result nrf24l01_read_payload_generic(uint8_t* pBuf, uint8_t* length, uint8_t dpl, nrf24l01_platform_t* platform) {
	uint8_t pipe;

	// Extract a payload pipe number from the STATUS register
	uint8_t temp;
	nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, &temp, platform);
	pipe = (temp & NRF24L01_MASK_STATUS_RX_P_NO) >> 1;

	// RX FIFO empty?
	if (pipe < 6) {
		// Get payload length
		if(dpl) {
			nrf24l01_get_rx_dpl_payload_width(length, platform);
			if(*length>32) { //broken packet
				*length = 0;
				nrf24l01_flush_rx(platform);
			}
		} else {
			nrf24l01_read_reg(NRF24L01_RX_PW_PIPE[pipe], length, platform);
		}

		// Read a payload from the RX FIFO
		if (*length) {
			nrf24l01_multi_read_reg(NRF24L01_CMD_R_RX_PAYLOAD, pBuf, *length, platform);
		}

		return ((nrf24l01_rx_result)pipe);
	}

	// The RX FIFO is empty
	*length = 0;

	return NRF24L01_RX_EMPTY;
}

// Read top level payload available in the RX FIFO
// input:
//   pBuf - pointer to the buffer to store a payload data
//   length - pointer to variable to store a payload length
// return: one of nRF24_RX_xx values
//   nRF24_RX_PIPEX - packet has been received from the pipe number X
//   nRF24_RX_EMPTY - the RX FIFO is empty
nrf24l01_rx_result nrf24l01_read_payload(uint8_t* pBuf, uint8_t* length, nrf24l01_platform_t* platform) {
	return nrf24l01_read_payload_generic(pBuf, length, 0, platform);
}

nrf24l01_rx_result nrf24l01_read_payload_dpl(uint8_t* pBuf, uint8_t* length, nrf24l01_platform_t* platform) {
	return nrf24l01_read_payload_generic(pBuf, length, 1, platform);
}

nrf24l01_err_t nrf24l01_get_features(uint8_t* features, nrf24l01_platform_t* platform) {
    return nrf24l01_read_reg(NRF24L01_FEATURE_REG_ADDR, features, platform);
}
nrf24l01_err_t nrf24l01_activate_features(nrf24l01_platform_t* platform) {
	/*
    nrf24l01_CSN_L();
    nrf24l01_LL_RW(NRF24L01_CMD_ACTIVATE);
    nrf24l01_LL_RW(0x73);
    nrf24l01_CSN_H();
	*/
	return NRF24L01_OK;
}
nrf24l01_err_t nrf24l01_write_ack_payload(nrf24l01_rx_result pipe, uint8_t* payload, uint8_t len, nrf24l01_platform_t* platform) {
	/*
	nrf24l01_CSN_L();
	nrf24l01_LL_RW(NRF24L01_CMD_W_ACK_PAYLOAD | pipe);
	while (length--) {
		nrf24l01_LL_RW((uint8_t)* payload++);
	}
	nrf24l01_CSN_H();
	*/

	/*
	nrf24l01_err_t err;
	if(data == NULL){
		err = NRF24L01_INVALID_ARG;
		return err;
	}
	err = nrf24l01_multi_write_reg(NRF24L01_CMD_W_TX_PAYLOAD, data, len, platform);
	return err;
	*/

	return NRF24L01_OK;
}


#ifdef NRF24L01_ENABLE_PRINT_CONFIG

// Print nRF24L01+ current configuration (for debug purposes)
void nrf24l01_print_config(nrf24l01_platform_t* platform) {
	uint8_t aw;
	uint8_t buf[5];

	uint8_t reg_temp;

	// Dump nRF24L01+ configuration registers

	// CONFIG
	nrf24l01_read_reg(NRF24L01_CONFIG_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X MASK:"_3BIT_FMT" CRC:"_2BIT_FMT" PWR:%s MODE:P%s\r\n",
		NRF24L01_CONFIG_REG_ADDR, reg_temp, _3BIT_STR(reg_temp >> 4), _2BIT_STR((reg_temp & 0x0c) >> 2), (reg_temp & 0x02) ? "ON" : "OFF", (reg_temp & 0x01) ? "RX" : "TX"
	);
	// EN_AA
	nrf24l01_read_reg(NRF24L01_EN_AA_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X ENAA: ", NRF24L01_EN_AA_REG_ADDR, reg_temp);
	for (uint8_t i = 0; i < 6; i++) {
		NRF24L01_DEBUGGING_PRINTF("[P%1u%s]%s", i, (reg_temp & (1 << i)) ? "+" : "-", (i == 5) ? "\r\n" : " ");
	}
	// EN_RX_ADDR
	nrf24l01_read_reg(NRF24L01_EN_RX_ADDR_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X EN_RXADDR: ", NRF24L01_EN_RX_ADDR_REG_ADDR, reg_temp);
	for (uint8_t i = 0; i < 6; i++) {
		NRF24L01_DEBUGGING_PRINTF("[P%1u%s]%s", i, (reg_temp & (1 << i)) ? "+" : "-", (i == 5) ? "\r\n" : " ");
	}
	// SETUP_AW
	nrf24l01_read_reg(NRF24L01_SETUP_AW_REG_ADDR, &reg_temp, platform);
	aw = (reg_temp & 0x03) + 2;
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X EN_RXADDR="_6BIT_FMT" (address width = %u)\r\n", NRF24L01_SETUP_AW_REG_ADDR, reg_temp, _6BIT_STR(reg_temp & 0x03), aw);
	// SETUP_RETR
	nrf24l01_read_reg(NRF24L01_SETUP_RETR_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X ARD="_4BIT_FMT" ARC="_4BIT_FMT" (retr.delay=%uus, count=%u)\r\n",
		NRF24L01_SETUP_RETR_REG_ADDR, reg_temp, _4BIT_STR(reg_temp >> 4), _4BIT_STR(reg_temp & 0x0F), ((reg_temp >> 4) * 250) + 250, reg_temp & 0x0F
	);
	// RF_CH
	nrf24l01_read_reg(NRF24L01_RF_CH_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X (%.3uGHz)\r\n", NRF24L01_RF_CH_REG_ADDR, reg_temp, 2400 + reg_temp);
	// RF_SETUP
	nrf24l01_read_reg(NRF24L01_RF_SETUP_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X CONT_WAVE:%s PLL_LOCK:%s DataRate=",
		NRF24L01_RF_SETUP_REG_ADDR, reg_temp, (reg_temp & 0x80) ? "ON" : "OFF", (reg_temp & 0x80) ? "ON" : "OFF"
	);
	switch ((reg_temp & 0x28) >> 3) {
		case 0x00:
			NRF24L01_DEBUGGING_PRINTF("1M");
			break;
		case 0x01:
			NRF24L01_DEBUGGING_PRINTF("2M");
			break;
		case 0x04:
			NRF24L01_DEBUGGING_PRINTF("250k");
			break;
		default:
			NRF24L01_DEBUGGING_PRINTF("???");
			break;
	}
	NRF24L01_DEBUGGING_PRINTF("pbs RF_PWR=");
	switch ((reg_temp & 0x06) >> 1) {
		case 0x00:
			NRF24L01_DEBUGGING_PRINTF("-18");
			break;
		case 0x01:
			NRF24L01_DEBUGGING_PRINTF("-12");
			break;
		case 0x02:
			NRF24L01_DEBUGGING_PRINTF("-6");
			break;
		case 0x03:
			NRF24L01_DEBUGGING_PRINTF("0");
			break;
		default:
			NRF24L01_DEBUGGING_PRINTF("???");
			break;
	}
	NRF24L01_DEBUGGING_PRINTF("dBm\r\n");
	// STATUS
	nrf24l01_read_reg(NRF24L01_STATUS_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X IRQ:"_3BIT_FMT" RX_PIPE:%u TX_FULL:%s\r\n",
		NRF24L01_STATUS_REG_ADDR, reg_temp, _3BIT_STR((reg_temp & 0x70) >> 4), (reg_temp & 0x0E) >> 1, (reg_temp & 0x01) ? "YES" : "NO"
	);
	// OBSERVE_TX
	nrf24l01_read_reg(NRF24L01_OBSERVE_TX_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X PLOS_CNT=%u ARC_CNT=%u\r\n", NRF24L01_OBSERVE_TX_REG_ADDR, reg_temp, reg_temp >> 4, reg_temp & 0x0F);
	// RPD
	nrf24l01_read_reg(NRF24L01_RPD_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] 0x%02X RPD=%s\r\n", NRF24L01_RPD_REG_ADDR, reg_temp, (reg_temp & 0x01) ? "YES" : "NO");
	

	uint8_t address_width;
	nrf24l01_get_address_width(&address_width, platform);

	// RX_ADDR_P0
	nrf24l01_multi_read_reg(NRF24L01_RX_ADDR_P0_REG_ADDR, buf, address_width, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P0 \"", NRF24L01_RX_ADDR_P0_REG_ADDR);
	for (uint8_t i = 0; i < address_width; i++){
		NRF24L01_DEBUGGING_PRINTF("0x%02X%s", buf[i], (i == (address_width - 1)) ? "\"\r\n" : ", ");
	}
	// RX_ADDR_P1
	nrf24l01_multi_read_reg(NRF24L01_RX_ADDR_P1_REG_ADDR, buf, aw, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P1 \"", NRF24L01_RX_ADDR_P1_REG_ADDR);
	for (uint8_t i = 0; i < address_width; i++){
		NRF24L01_DEBUGGING_PRINTF("0x%02X%s", buf[i], (i == (address_width - 1)) ? "\"\r\n" : ", ");
	}
	// RX_ADDR_P2
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P2 \"", NRF24L01_RX_ADDR_P2_REG_ADDR);
	for (uint8_t i = 0; i < aw - 1; i++) NRF24L01_DEBUGGING_PRINTF("%c", buf[i]);
	nrf24l01_read_reg(NRF24L01_RX_ADDR_P2_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("%c\"\r\n", reg_temp);
	// RX_ADDR_P3
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P3 \"", NRF24L01_RX_ADDR_P3_REG_ADDR);
	for (uint8_t i = 0; i < aw - 1; i++) NRF24L01_DEBUGGING_PRINTF("%c", buf[i]);
	nrf24l01_read_reg(NRF24L01_RX_ADDR_P3_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("%c\"\r\n", reg_temp);
	// RX_ADDR_P4
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P4 \"", NRF24L01_RX_ADDR_P4_REG_ADDR);
	for (uint8_t i = 0; i < aw - 1; i++) NRF24L01_DEBUGGING_PRINTF("%c", buf[i]);
	nrf24l01_read_reg(NRF24L01_RX_ADDR_P4_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("%c\"\r\n", reg_temp);
	// RX_ADDR_P5
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_ADDR_P5 \"", NRF24L01_RX_ADDR_P5_REG_ADDR);
	for (uint8_t i = 0; i < aw - 1; i++) NRF24L01_DEBUGGING_PRINTF("%c", buf[i]);
	nrf24l01_read_reg(NRF24L01_RX_ADDR_P5_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("%c\"\r\n", reg_temp);
	// TX_ADDR
	nrf24l01_multi_read_reg(NRF24L01_TX_ADDR_REG_ADDR, buf, address_width, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] TX_ADDR    \"", NRF24L01_TX_ADDR_REG_ADDR);
	for (uint8_t i = 0; i < address_width; i++){
		NRF24L01_DEBUGGING_PRINTF("0x%02X%s", buf[i], (i == (address_width - 1)) ? "\"\r\n" : ", ");
	}



	// RX_PW_P0
	nrf24l01_read_reg(NRF24L01_RX_PW_P0_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P0=%u\r\n", NRF24L01_RX_PW_P0_REG_ADDR, reg_temp);
	// RX_PW_P1
	nrf24l01_read_reg(NRF24L01_RX_PW_P1_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P1=%u\r\n", NRF24L01_RX_PW_P1_REG_ADDR, reg_temp);
	// RX_PW_P2
	nrf24l01_read_reg(NRF24L01_RX_PW_P2_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P2=%u\r\n", NRF24L01_RX_PW_P2_REG_ADDR, reg_temp);
	// RX_PW_P3
	nrf24l01_read_reg(NRF24L01_RX_PW_P3_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P3=%u\r\n", NRF24L01_RX_PW_P3_REG_ADDR, reg_temp);
	// RX_PW_P4
	nrf24l01_read_reg(NRF24L01_RX_PW_P4_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P4=%u\r\n", NRF24L01_RX_PW_P4_REG_ADDR, reg_temp);
	// RX_PW_P5
	nrf24l01_read_reg(NRF24L01_RX_PW_P5_REG_ADDR, &reg_temp, platform);
	NRF24L01_DEBUGGING_PRINTF("[0x%02X] RX_PW_P5=%u\r\n", NRF24L01_RX_PW_P5_REG_ADDR, reg_temp);
}

void print_status_register(nrf24l01_platform_t* platform){
	uint8_t status;
    nrf24l01_get_status(&status, platform);

    char* pipe_number;
    switch((status & NRF24L01_MASK_STATUS_RX_P_NO) >> 1){
        case 0:
            pipe_number = "0";
            break;
        case 1:
            pipe_number = "1";
        	break;
        case 2:
            pipe_number = "2";
            break;
        case 3:
            pipe_number = "3";
            break;
        case 4:
            pipe_number = "4";
            break;
        case 5:
            pipe_number = "5";
            break;
        case 7:
            pipe_number = "RX FIFO Empty";
            break;
        default:
            pipe_number = "?";
            break;
    }
    NRF24L01_DEBUGGING_PRINTF("[0x%02X] STATUS_REG="_8BIT_FMT"=0x%02X (RX_DR=%s, TX_DS=%s, MAX_RT=%s, RX_P_NO=%s, TX_FULL=%s)\r\n",
        NRF24L01_STATUS_REG_ADDR,
		_8BIT_STR(status), status, 
        (status & NRF24L01_MASK_STATUS_RX_DR)  ? "1" : "0", 
        (status & NRF24L01_MASK_STATUS_TX_DS)  ? "1" : "0",
        (status & NRF24L01_MASK_STATUS_MAX_RT) ? "1" : "0",
        pipe_number,
        (status & NRF24L01_MASK_STATUS_TX_FULL) ? "1" : "0"
    );
}

void print_fifo_status_register(nrf24l01_platform_t* platform){

	uint8_t rx_status;
	nrf24l01_get_status_rx_fifo(&rx_status, platform);

	uint8_t tx_status;
	nrf24l01_get_status_rx_fifo(&tx_status, platform);

	uint8_t fifo_status = (tx_status << 4) | rx_status;

	NRF24L01_DEBUGGING_PRINTF("[0x%02X] FIFO_STATUS_REG="_8BIT_FMT"=0x%02X (TX_REUSE=%s, TX_FULL=%s, TX_EMPTY=%s, RX_FULL=%s, RX_EMPTY=%s)\r\n",
        NRF24L01_FIFO_STATUS_REG_ADDR,
		_8BIT_STR(fifo_status), fifo_status, 
        (fifo_status & NRF24L01_MASK_TX_FIFO_REUSE)  ? "1" : "0", 
        (fifo_status & NRF24L01_MASK_TX_FIFO_FULL)  ? "1" : "0",
        (fifo_status & NRF24L01_MASK_TX_FIFO_EMPTY) ? "1" : "0",
        (fifo_status & NRF24L01_MASK_RX_FIFO_FULL) ? "1" : "0",
        (fifo_status & NRF24L01_MASK_RX_FIFO_EMPTY) ? "1" : "0"
    );

}

#endif 