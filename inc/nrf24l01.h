#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// User provided config directory in directory above submodule
#include "../../nrf24l01_config.h"

// Driver constants and types
#include "nrf24l01_defs.h"


/*** Device API ***************************************************************************************************/
/// @addtogroup device_api
/// @{

// General
nrf24l01_err_t nrf24l01_init(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_deinit(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_check_connectivity(nrf24l01_platform_t*);

// Interrupts
nrf24l01_err_t nrf24l01_get_irq_flags(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_irq_mask(nrf24l01_interrupt_mask_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_clear_irq_flags(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_register_rx_dr_callback(nrf24l01_rx_dr_callback_fptr_t, void*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_register_tx_ds_callback(nrf24l01_tx_ds_callback_fptr_t, void*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_register_max_rt_callback(nrf24l01_max_rt_callback_fptr_t, void*, nrf24l01_platform_t*);

// RF + protocol
nrf24l01_err_t nrf24l01_set_tx_power(nrf24l01_tx_power_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_rf_channel(nrf24l01_rf_channel_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_data_rate(nrf24l01_data_rate_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_auto_retransmission(nrf24l01_ar_delay_t, nrf24l01_ar_count_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_crc_scheme(nrf24l01_crc_scheme_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_address_width(nrf24l01_address_width_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_address_width(nrf24l01_address_width_t, nrf24l01_platform_t*);
// nrf24l01_err_t nrf24l01_get_carrier_detect(nrf24l01_address_width_t, nrf24l01_platform_t*);

// Host SPI interface
nrf24l01_err_t nrf24l01_flush_rx(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_flush_tx(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_status(uint8_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_fifo_status(nrf24l01_fifo_type_t, nrf24l01_fifo_status_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_reset_packet_loss_counter(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_retransmit_counters(nrf24l01_ar_lost_t*, nrf24l01_ar_count_t*, nrf24l01_platform_t*);

// Device state
nrf24l01_err_t nrf24l01_get_power_mode(nrf24l01_power_mode_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_power_mode(nrf24l01_power_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_get_operational_mode(nrf24l01_operational_mode_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_operational_mode(nrf24l01_operational_mode_t, nrf24l01_platform_t*);

// Features
nrf24l01_err_t nrf24l01_get_feature_mode(nrf24l01_feature_mode_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_feature_mode(nrf24l01_feature_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_ack_payload_mode(ack_payload_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_dpl_mode(nrf24l01_dpl_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_dta_mode(nrf24l01_dta_mode_t, nrf24l01_platform_t*);

nrf24l01_err_t nrf24l01_get_features(uint8_t*, nrf24l01_platform_t*);

/// @}
/****************************************************************************************************************/
    

/*** Pipe API ***************************************************************************************************/
/// @addtogroup pipe_api
/// @{

// Configure
nrf24l01_err_t nrf24l01_set_pipe_mode(nrf24l01_pipe_t, nrf24l01_pipe_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_pipe_aa_mode(nrf24l01_pipe_t, nrf24l01_pipe_aa_mode_t, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_set_pipe_rx_payload_size(nrf24l01_pipe_t, uint8_t, nrf24l01_platform_t*);

// Transmit
nrf24l01_err_t nrf24l01_write_payload(uint8_t*, uint8_t, nrf24l01_platform_t*);
//nrf24l01_err_t nrf24l01_write_ack_payload(nrf24l01_rx_result, uint8_t*, uint8_t, nrf24l01_platform_t*);

// Receive
nrf24l01_err_t nrf24l01_get_rx_pipe(nrf24l01_pipe_t*, nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_read_payload(nrf24l01_pipe_t*, uint8_t*, uint8_t*, bool, nrf24l01_platform_t*);     // Read from the pipe which received the packet

//Features
nrf24l01_err_t nrf24l01_set_pipe_dpl_mode(nrf24l01_pipe_t, nrf24l01_dpl_mode_t, nrf24l01_platform_t*);

/// @}
/****************************************************************************************************************/

nrf24l01_err_t nrf24l01_start_listening(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_stop_listening(nrf24l01_platform_t*);


void nrf24l01_loop(nrf24l01_platform_t*);

nrf24l01_err_t nrf24l01_set_address(nrf24l01_pipe_t, const uint8_t*, uint8_t, nrf24l01_platform_t*);



/*** Debug API **************************************************************************************************/

#ifdef NRF24L01_ENABLE_PRINT_CONFIG
nrf24l01_err_t nrf24l01_print_config(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_print_status_register(nrf24l01_platform_t*);
nrf24l01_err_t nrf24l01_print_fifo_status_register(nrf24l01_platform_t*);
#else
nrf24l01_err_t nrf24l01_print_config(nrf24l01_platform_t* platform){ return; };
nrf24l01_err_t nrf24l01_print_status_register(nrf24l01_platform_t* platform){ return; };
nrf24l01_err_t nrf24l01_print_fifo_status_register(nrf24l01_platform_t* platform) { return; };
#endif // NRF24L01_ENABLE_PRINT_CONFIG

/****************************************************************************************************************/

#endif // NRF24L01_H
