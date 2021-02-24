#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// User provided config directory in directory above submodule
#include "../nrf24l01_config.h"

// Driver constants and types
#include "nrf24l01_defs.h"

// Function prototypes
nrf24l01_err_t nrf24l01_init(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_check_connectivity(nrf24l01_platform_t*);

nrf24l01_err_t nrf24l01_enable_aa(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_disable_aa(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_close_pipe(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_reset_packet_loss_counter(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_flush_rx(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_flush_tx(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_clear_irq_flags(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_activate_features(nrf24l01_platform_t*);

nrf24l01_err_t nrf24l01_get_status(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_irq_flags(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_status_rx_fifo(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_status_tx_fifo(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_rx_source(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_retransmit_counters(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_features(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_address_width(uint8_t*, nrf24l01_platform_t*);

nrf24l01_err_t nrf24l01_set_power_mode(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_operational_mode(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_rf_channel(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_auto_retransmission(uint8_t, uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_address_width(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_address(uint8_t, const uint8_t*, uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_tx_power(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_data_rate(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_crc_scheme(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_payload_with_ack(uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_rx_pipe(uint8_t, uint8_t, uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_dynamic_payload_length(uint8_t, nrf24l01_platform_t*);

// Transmit
nrf24l01_err_t nrf24l01_write_payload(uint8_t*, uint8_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_write_ack_payload(nrf24l01_rx_result, uint8_t*, uint8_t, nrf24l01_platform_t*);

// Receive
nrf24l01_rx_result nrf24l01_read_payload(uint8_t*, uint8_t*, nrf24l01_platform_t*);
nrf24l01_rx_result nrf24l01_read_payload_dpl(uint8_t*, uint8_t*, nrf24l01_platform_t*);


#ifdef NRF24L01_ENABLE_PRINT_CONFIG
void nrf24l01_print_config(nrf24l01_platform_t*);
void nrf24l01_print_status_register(nrf24l01_platform_t*);
void nrf24l01_print_fifo_status_register(nrf24l01_platform_t*);
#else
void nrf24l01_print_config(nrf24l01_platform_t* platform){ return; };
void nrf24l01_print_status_register(nrf24l01_platform_t* platform){ return; };
void nrf24l01_print_fifo_status_register(nrf24l01_platform_t* platform) { return; };
#endif // NRF24L01_ENABLE_PRINT_CONFIG


#endif // NRF24L01_H
