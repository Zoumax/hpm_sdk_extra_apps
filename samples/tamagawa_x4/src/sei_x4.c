/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "hpm_debug_console.h"
#include "hpm_clock_drv.h"
#include "hpm_synt_drv.h"
#include "hpm_sei_drv.h"

#if defined(CONFIG_NDEBUG_CONSOLE) && CONFIG_NDEBUG_CONSOLE
#include "SEGGER_RTT.h"
#endif

void app_init_4sei_pins(void)
{
    //TODO : Add init pins code
}

int main(void)
{
    sei_tranceiver_config_t tranceiver_config = {0};
    sei_data_format_config_t data_format_config = {0};
    sei_engine_config_t engine_config = {0};
    sei_trigger_input_config_t trigger_input_conifg = {0};
    sei_state_transition_config_t state_transition_config = {0};
    sei_state_transition_latch_config_t state_transition_latch_config = {0};
    sei_sample_config_t sample_config = {0};
    sei_update_config_t update_config = {0};
    uint8_t instr0_idx;
    uint8_t instr1_idx;
    uint8_t instr2_idx;
    uint8_t instr3_idx;

    board_init();
    app_init_4sei_pins();
#if defined(CONFIG_NDEBUG_CONSOLE) && CONFIG_NDEBUG_CONSOLE
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif
    printf("SEI master tamagawa x4 sample\n");
    /* Disable SEI engine */
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_0, false);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_1, false);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_2, false);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_3, false);

    /* [1] tranceiver config */
    tranceiver_config.mode = sei_asynchronous_mode;
    tranceiver_config.tri_sample = false;
    tranceiver_config.src_clk_freq = clock_get_frequency(BOARD_MOTOR_CLK_NAME);
    tranceiver_config.asynchronous_config.wait_len = 0;
    tranceiver_config.asynchronous_config.data_len = 8;
    tranceiver_config.asynchronous_config.parity_enable = false;
    tranceiver_config.asynchronous_config.data_idle_high_z = false;
    tranceiver_config.asynchronous_config.data_idle_state = sei_idle_high_state;
    tranceiver_config.asynchronous_config.baudrate = 2500000;
    sei_tranceiver_config_init(BOARD_SEI, SEI_CTRL_0, &tranceiver_config);
    sei_tranceiver_config_init(BOARD_SEI, SEI_CTRL_1, &tranceiver_config);
    sei_tranceiver_config_init(BOARD_SEI, SEI_CTRL_2, &tranceiver_config);
    sei_tranceiver_config_init(BOARD_SEI, SEI_CTRL_3, &tranceiver_config);

    /* [2] cmd/data register config */
    /* cmd register : send/recv CF */
    data_format_config.mode = sei_check_mode;
    data_format_config.signed_flag = false;
    data_format_config.bit_order = sei_bit_lsb_first;
    data_format_config.word_order = sei_word_nonreverse;
    data_format_config.word_len = 8;
    data_format_config.last_bit = 7;
    data_format_config.first_bit = 0;
    data_format_config.max_bit = 7;
    data_format_config.min_bit = 0;
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_CMD, SEI_CTRL_0, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_CMD, SEI_CTRL_1, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_CMD, SEI_CTRL_2, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_CMD, SEI_CTRL_3, &data_format_config);
    /*
    *  When a trigger occurs, the hardware automatically loads the command from the trigger
    *  command register corresponding to the trigger method into the command register.
    */
    sei_set_trig_input_command_value(BOARD_SEI, SEI_CTRL_0, sei_trig_in_period, 0x1A);    /* Data ID 3 */
    sei_set_trig_input_command_value(BOARD_SEI, SEI_CTRL_1, sei_trig_in_period, 0x1A);
    sei_set_trig_input_command_value(BOARD_SEI, SEI_CTRL_2, sei_trig_in_period, 0x1A);
    sei_set_trig_input_command_value(BOARD_SEI, SEI_CTRL_3, sei_trig_in_period, 0x1A);
    /* data register 2: recv SF, ENID, ALMC */
    data_format_config.mode = sei_data_mode;
    data_format_config.signed_flag = false;
    data_format_config.bit_order = sei_bit_lsb_first;
    data_format_config.word_order = sei_word_nonreverse;
    data_format_config.word_len = 24;
    data_format_config.last_bit = 23;
    data_format_config.first_bit = 0;
    data_format_config.max_bit = 23;
    data_format_config.min_bit = 0;
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_2, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_6, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_10, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_14, &data_format_config);
    /* data register 3: recv ABS0 ABS1 ABS2 */
    data_format_config.mode = sei_data_mode;
    data_format_config.signed_flag = false;
    data_format_config.bit_order = sei_bit_lsb_first;
    data_format_config.word_order = sei_word_nonreverse;
    data_format_config.word_len = 24;
    data_format_config.last_bit = 23;
    data_format_config.first_bit = 0;
    data_format_config.max_bit = 23;
    data_format_config.min_bit = 0;
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_3, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_7, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_11, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_15, &data_format_config);
    /* data register 4: recv ABM0 ABM1 ABM2 */
    data_format_config.mode = sei_data_mode;
    data_format_config.signed_flag = false;
    data_format_config.bit_order = sei_bit_lsb_first;
    data_format_config.word_order = sei_word_nonreverse;
    data_format_config.word_len = 24;
    data_format_config.last_bit = 23;
    data_format_config.first_bit = 0;
    data_format_config.max_bit = 23;
    data_format_config.min_bit = 0;
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_4, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_8, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_12, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_16, &data_format_config);
    /* data register 5: check crc */
    data_format_config.mode = sei_crc_mode;
    data_format_config.signed_flag = false;
    data_format_config.bit_order = sei_bit_lsb_first;
    data_format_config.word_order = sei_word_nonreverse;
    data_format_config.word_len = 8;
    data_format_config.crc_invert = false;
    data_format_config.crc_shift_mode = false;
    data_format_config.crc_len = 8;
    data_format_config.last_bit = 7;
    data_format_config.first_bit = 0;
    data_format_config.max_bit = 7;
    data_format_config.min_bit = 0;
    data_format_config.crc_init_value = 0;
    data_format_config.crc_poly = 1;
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_5, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_9, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_13, &data_format_config);
    sei_cmd_data_format_config_init(BOARD_SEI, SEI_SELECT_DATA, SEI_DAT_17, &data_format_config);
    /* [3] instructions */
    instr0_idx = 0;    /* 0 */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_SEND, 0, SEI_DAT_0, SEI_DAT_1, 8);  /* Send CF */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_1, 8);  /* CF */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_2, 8);  /* SF */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_3, 24); /* ST(POS) */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_2, 8);  /* ENID:0x17 */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_4, 24); /* MT */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_5, SEI_DAT_2, 8);  /* ALMC */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_0, SEI_DAT_5, 8);  /* CRC */
    sei_set_instr(BOARD_SEI, instr0_idx++, SEI_INSTR_OP_HALT, 0, SEI_DAT_0, SEI_DAT_0, 0);  
    instr1_idx = instr0_idx;    /* 9 */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_SEND, 0, SEI_DAT_0, SEI_DAT_1, 8);  /* Send CF */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_1, 8);  /* CF */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_6, 8);  /* SF */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_7, 24); /* ST(POS) */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_6, 8);  /* ENID:0x17 */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_8, 24); /* MT */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_9, SEI_DAT_6, 8);  /* ALMC */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_0, SEI_DAT_9, 8);  /* CRC */
    sei_set_instr(BOARD_SEI, instr1_idx++, SEI_INSTR_OP_HALT, 0, SEI_DAT_0, SEI_DAT_0, 0);
    instr2_idx = instr1_idx;    /* 18 */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_SEND, 0, SEI_DAT_0, SEI_DAT_1, 8);  /* Send CF */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_1, 8);  /* CF */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_10, 8);  /* SF */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_11, 24); /* ST(POS) */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_10, 8);  /* ENID:0x17 */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_12, 24); /* MT */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_13, SEI_DAT_10, 8);  /* ALMC */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_0, SEI_DAT_13, 8);  /* CRC */
    sei_set_instr(BOARD_SEI, instr2_idx++, SEI_INSTR_OP_HALT, 0, SEI_DAT_0, SEI_DAT_0, 0);
    instr3_idx = instr2_idx;        /* 27 */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_SEND, 0, SEI_DAT_0, SEI_DAT_1, 8);  /* Send CF */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_1, 8);  /* CF */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_14, 8);  /* SF */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_15, 24); /* ST(POS) */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_14, 8);  /* ENID:0x17 */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_16, 24); /* MT */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_17, SEI_DAT_14, 8);  /* ALMC */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_RECV_WDG, 0, SEI_DAT_0, SEI_DAT_17, 8);  /* CRC */
    sei_set_instr(BOARD_SEI, instr3_idx++, SEI_INSTR_OP_HALT, 0, SEI_DAT_0, SEI_DAT_0, 0);
    /* [4] state transition config */
    /* latch0 */
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = false;
    state_transition_config.instr_ptr_cfg = sei_state_tran_condition_fall_leave;
    state_transition_config.instr_ptr_value = 0;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr0_idx;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr1_idx;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr2_idx;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);

    /* latch1 */
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = false;
    state_transition_config.instr_ptr_cfg = sei_state_tran_condition_fall_leave;
    state_transition_config.instr_ptr_value = instr0_idx - 2;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr1_idx - 2;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr2_idx - 2;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.instr_ptr_value = instr3_idx - 2;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_0_1, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_1_2, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_2_3, &state_transition_config);
    state_transition_config.disable_clk_check = true;
    state_transition_config.disable_txd_check = true;
    state_transition_config.disable_rxd_check = true;
    state_transition_config.disable_timeout_check = true;
    state_transition_config.disable_instr_ptr_check = true;
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);
    sei_state_transition_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1, SEI_CTRL_LATCH_TRAN_3_0, &state_transition_config);

    /* [5] sample config*/
    sample_config.latch_select = SEI_LATCH_0;
    sei_sample_config_init(BOARD_SEI, SEI_CTRL_0, &sample_config);
    sei_sample_config_init(BOARD_SEI, SEI_CTRL_1, &sample_config);
    sei_sample_config_init(BOARD_SEI, SEI_CTRL_2, &sample_config);
    sei_sample_config_init(BOARD_SEI, SEI_CTRL_3, &sample_config);

    /* [6] update config*/
    update_config.pos_data_use_rx = true;
    update_config.rev_data_use_rx = true;
    update_config.update_on_err = false;
    update_config.latch_select = SEI_LATCH_1;
    update_config.pos_data_idx = SEI_DAT_3;
    update_config.rev_data_idx = SEI_DAT_4;
    update_config.data_register_select = BIT3_MASK | BIT4_MASK;    /* SEI_DAT_3, SEI_DAT_4 */
    sei_update_config_init(BOARD_SEI, SEI_CTRL_0, &update_config);
    update_config.pos_data_idx = SEI_DAT_7;
    update_config.rev_data_idx = SEI_DAT_8;
    update_config.data_register_select = BIT7_MASK | BIT8_MASK;    /* SEI_DAT_7, SEI_DAT_8 */
    sei_update_config_init(BOARD_SEI, SEI_CTRL_1, &update_config);
    update_config.pos_data_idx = SEI_DAT_11;
    update_config.rev_data_idx = SEI_DAT_12;
    update_config.data_register_select = BIT11_MASK | BIT12_MASK;    /* SEI_DAT_11, SEI_DAT_12 */
    sei_update_config_init(BOARD_SEI, SEI_CTRL_2, &update_config);
    update_config.pos_data_idx = SEI_DAT_15;
    update_config.rev_data_idx = SEI_DAT_16;
    update_config.data_register_select = BIT15_MASK | BIT16_MASK;    /* SEI_DAT_15, SEI_DAT_16 */
    sei_update_config_init(BOARD_SEI, SEI_CTRL_3, &update_config);

    /* [7] interrupt config */
    intc_m_enable_irq_with_priority(IRQn_SEI_0, 1);
    intc_m_enable_irq_with_priority(IRQn_SEI_1, 1);
    intc_m_enable_irq_with_priority(IRQn_SEI_2, 1);
    intc_m_enable_irq_with_priority(IRQn_SEI_3, 1);
    sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_0, sei_irq_latch1_event | sei_irq_trx_err_event);
    sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_1, sei_irq_latch1_event | sei_irq_trx_err_event);
    sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_2, sei_irq_latch1_event | sei_irq_trx_err_event);
    sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_3, sei_irq_latch1_event | sei_irq_trx_err_event);
    sei_set_irq_enable(BOARD_SEI, SEI_CTRL_0, sei_irq_latch1_event | sei_irq_trx_err_event, true);
    sei_set_irq_enable(BOARD_SEI, SEI_CTRL_1, sei_irq_latch1_event | sei_irq_trx_err_event, true);
    sei_set_irq_enable(BOARD_SEI, SEI_CTRL_2, sei_irq_latch1_event | sei_irq_trx_err_event, true);
    sei_set_irq_enable(BOARD_SEI, SEI_CTRL_3, sei_irq_latch1_event | sei_irq_trx_err_event, true);

    /* [8] enbale sync timer timestamp */
    synt_enable_timestamp(HPM_SYNT, true);

    /* [9] engine config */
    printf("Started sei x4 engine!\n");
    engine_config.arming_mode = sei_arming_wait_trigger;
    engine_config.data_cdm_idx = 0;
    engine_config.data_base_idx = 0;
    engine_config.wdg_enable = true;
    engine_config.wdg_action = sei_wdg_exec_exception_instr;
    engine_config.wdg_time = 1000;    /* 1000 bits time */
    engine_config.init_instr_idx = 0;
    engine_config.wdg_instr_idx = instr0_idx - 1;
    sei_engine_config_init(BOARD_SEI, SEI_CTRL_0, &engine_config);
    engine_config.init_instr_idx = instr0_idx;
    engine_config.wdg_instr_idx = instr1_idx - 1;
    sei_engine_config_init(BOARD_SEI, SEI_CTRL_1, &engine_config);
    engine_config.init_instr_idx = instr1_idx;
    engine_config.wdg_instr_idx = instr2_idx - 1;
    sei_engine_config_init(BOARD_SEI, SEI_CTRL_2, &engine_config);
    engine_config.init_instr_idx = instr2_idx;
    engine_config.wdg_instr_idx = instr3_idx - 1;
    sei_engine_config_init(BOARD_SEI, SEI_CTRL_3, &engine_config);

    /* [10] start engine and latch modules */

    state_transition_latch_config.enable = true;
    state_transition_latch_config.output_select = SEI_CTRL_LATCH_TRAN_0_1;
    state_transition_latch_config.delay = (48 * (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 1000000)) / 100;    /*  unit: 1us, 0.48us */
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0, &state_transition_latch_config);

    state_transition_latch_config.enable = true;
    state_transition_latch_config.output_select = SEI_CTRL_LATCH_TRAN_0_1;
    state_transition_latch_config.delay = 0;
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1, &state_transition_latch_config);
    sei_state_transition_latch_config_init(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1, &state_transition_latch_config);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_0, true);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_1, true);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_2, true);
    sei_set_engine_enable(BOARD_SEI, SEI_CTRL_3, true);

    /* [11] trigger config */
    trigger_input_conifg.trig_period_enable = true;
    trigger_input_conifg.trig_period_arming_mode = sei_arming_direct_exec;
    trigger_input_conifg.trig_period_sync_enable = false;
    trigger_input_conifg.trig_period_time = (200 * (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 1000));    /* unit: 1ms, 200ms */
    sei_trigger_input_config_init(BOARD_SEI, SEI_CTRL_0, &trigger_input_conifg);
    sei_trigger_input_config_init(BOARD_SEI, SEI_CTRL_1, &trigger_input_conifg);
    sei_trigger_input_config_init(BOARD_SEI, SEI_CTRL_2, &trigger_input_conifg);
    sei_trigger_input_config_init(BOARD_SEI, SEI_CTRL_3, &trigger_input_conifg);

    while (1) {
        ;
    }
}

SDK_DECLARE_EXT_ISR_M(IRQn_SEI_0, isr_sei0)
SDK_DECLARE_EXT_ISR_M(IRQn_SEI_1, isr_sei1)
SDK_DECLARE_EXT_ISR_M(IRQn_SEI_2, isr_sei2)
SDK_DECLARE_EXT_ISR_M(IRQn_SEI_3, isr_sei3)
void isr_sei0(void)
{
    uint32_t sample_latch_tm;
    uint32_t update_latch_tm;
    uint32_t delta;

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_0, sei_irq_latch1_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_0, sei_irq_latch1_event);
        sample_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_0);
        update_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_0, SEI_LATCH_1);
        delta = (update_latch_tm > sample_latch_tm) ? (update_latch_tm - sample_latch_tm) : (update_latch_tm - sample_latch_tm + 0xFFFFFFFFu);
        printf("MT:%#x, ST:%#x, CRC:%#x, sample_tm:%u, update_tm:%u, TimeDelay:%d*0.1us\n",
                sei_get_data_value(BOARD_SEI, SEI_DAT_4),
                sei_get_data_value(BOARD_SEI, SEI_DAT_3),
                sei_get_data_value(BOARD_SEI, SEI_DAT_2) & 0xFF,
                sample_latch_tm, update_latch_tm, delta / (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 10000000));
    }

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_0, sei_irq_trx_err_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_0, sei_irq_trx_err_event);
        printf("TRX Error!\n");
    }
}
void isr_sei1(void)
{
    uint32_t sample_latch_tm;
    uint32_t update_latch_tm;
    uint32_t delta;

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_1, sei_irq_latch1_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_1, sei_irq_latch1_event);
        sample_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_0);
        update_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_1, SEI_LATCH_1);
        delta = (update_latch_tm > sample_latch_tm) ? (update_latch_tm - sample_latch_tm) : (update_latch_tm - sample_latch_tm + 0xFFFFFFFFu);
        printf("MT:%#x, ST:%#x, CRC:%#x, sample_tm:%u, update_tm:%u, TimeDelay:%d*0.1us\n",
                sei_get_data_value(BOARD_SEI, SEI_DAT_8),
                sei_get_data_value(BOARD_SEI, SEI_DAT_7),
                sei_get_data_value(BOARD_SEI, SEI_DAT_6) & 0xFF,
                sample_latch_tm, update_latch_tm, delta / (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 10000000));
    }

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_1, sei_irq_trx_err_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_1, sei_irq_trx_err_event);
        printf("TRX Error!\n");
    }
}
void isr_sei2(void)
{
    uint32_t sample_latch_tm;
    uint32_t update_latch_tm;
    uint32_t delta;

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_2, sei_irq_latch1_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_2, sei_irq_latch1_event);
        sample_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_0);
        update_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_2, SEI_LATCH_1);
        delta = (update_latch_tm > sample_latch_tm) ? (update_latch_tm - sample_latch_tm) : (update_latch_tm - sample_latch_tm + 0xFFFFFFFFu);
        printf("MT:%#x, ST:%#x, CRC:%#x, sample_tm:%u, update_tm:%u, TimeDelay:%d*0.1us\n",
                sei_get_data_value(BOARD_SEI, SEI_DAT_12),
                sei_get_data_value(BOARD_SEI, SEI_DAT_11),
                sei_get_data_value(BOARD_SEI, SEI_DAT_10) & 0xFF,
                sample_latch_tm, update_latch_tm, delta / (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 10000000));
    }

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_2, sei_irq_trx_err_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_2, sei_irq_trx_err_event);
        printf("TRX Error!\n");
    }
}
void isr_sei3(void)
{
    uint32_t sample_latch_tm;
    uint32_t update_latch_tm;
    uint32_t delta;

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_3, sei_irq_latch1_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_3, sei_irq_latch1_event);
        sample_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_0);
        update_latch_tm = sei_get_latch_time(BOARD_SEI, SEI_CTRL_3, SEI_LATCH_1);
        delta = (update_latch_tm > sample_latch_tm) ? (update_latch_tm - sample_latch_tm) : (update_latch_tm - sample_latch_tm + 0xFFFFFFFFu);
        printf("MT:%#x, ST:%#x, CRC:%#x, sample_tm:%u, update_tm:%u, TimeDelay:%d*0.1us\n",
                sei_get_data_value(BOARD_SEI, SEI_DAT_16),
                sei_get_data_value(BOARD_SEI, SEI_DAT_15),
                sei_get_data_value(BOARD_SEI, SEI_DAT_14) & 0xFF,
                sample_latch_tm, update_latch_tm, delta / (clock_get_frequency(BOARD_MOTOR_CLK_NAME) / 10000000));
    }

    if (sei_get_irq_status(BOARD_SEI, SEI_CTRL_3, sei_irq_trx_err_event)) {
        sei_clear_irq_flag(BOARD_SEI, SEI_CTRL_3, sei_irq_trx_err_event);
        printf("TRX Error!\n");
    }
}
