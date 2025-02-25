/*
 * Copyright (c) 2025 HPMicro & Zoumax
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "board.h"
#include "hpm_clock_drv.h"
#include <stdio.h>
#ifdef HPMSOC_HAS_HPMSDK_DMAV2
#include "hpm_dmav2_drv.h"
#else
#include "hpm_dma_drv.h"
#endif
#include "hpm_dmamux_drv.h"
#include "hpm_uart_drv.h"

/* Define */
#define TEST_UART BOARD_CONSOLE_UART_BASE
#define TEST_UART_CLK_NAME BOARD_CONSOLE_UART_CLK_NAME
// #define TEST_UART_DMA_SRC_TX BOARD_CONSOLE_UART_TX_DMA_REQ
#define TEST_UART_DMA_SRC_TX HPM_DMA_SRC_GPTMR2_2           // 使用GPTMR2_2触发DMA

#define TEST_UART_DMA_CONTROLLER BOARD_APP_HDMA
#define TEST_UART_DMAMUX_CONTROLLER BOARD_APP_DMAMUX
#define TEST_UART_TX_DMA_CH 0
#define TEST_UART_TX_DMAMUX_CH DMA_SOC_CHN_TO_DMAMUX_CHN(TEST_UART_DMA_CONTROLLER, TEST_UART_TX_DMA_CH)

#define TEST_BUFFER_SIZE 64                    // 可配置BUFFER长度，只传一个字节设为1即可
#define APP_USE_LINKED_DMA 1                   // 是否使用链式DMA，6E系列无需使用链式DMA也能完成无限循环的任务

/* dma buffer should be 4-byte aligned */
ATTR_PLACE_AT_NONCACHEABLE_BSS_WITH_ALIGNMENT(4) uint8_t uart_tx_buf[TEST_BUFFER_SIZE];
/* descriptor should be 8-byte aligned */
ATTR_PLACE_AT_NONCACHEABLE_BSS_WITH_ALIGNMENT(8) dma_linked_descriptor_t descriptors[2];

void init_board_app_uart(void)
{
    hpm_stat_t stat;
    uart_config_t config = {0};

    board_init_uart(TEST_UART);

    uart_default_config(TEST_UART, &config);
    config.baudrate = 2000000;
    config.fifo_enable = true;
    config.dma_enable = true;
    config.src_freq_in_hz = clock_get_frequency(TEST_UART_CLK_NAME);
    config.tx_fifo_level = uart_tx_fifo_trg_not_full;
    config.rx_fifo_level = uart_rx_fifo_trg_not_empty;
    stat = uart_init(TEST_UART, &config);
    if (stat != status_success)
    {
        while (1)
        {
        }
    }
}

void init_board_app_dma(void)
{
    hpm_stat_t stat;
    /* 1. config uart circle tx dma */
    dmamux_config(TEST_UART_DMAMUX_CONTROLLER, TEST_UART_TX_DMAMUX_CH, TEST_UART_DMA_SRC_TX, true);
#if defined(APP_USE_LINKED_DMA) && APP_USE_LINKED_DMA
    dma_channel_config_t tx_ch_config = {0};

    /* 1.1 config chain descriptors */
    dma_default_channel_config(TEST_UART_DMA_CONTROLLER, &tx_ch_config);
    tx_ch_config.dst_addr = (uint32_t)&TEST_UART->THR;
    tx_ch_config.dst_width =
        DMA_TRANSFER_WIDTH_BYTE; /*  In DMA handshake case, source width and destination width must be BYTE. */
    tx_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
    tx_ch_config.dst_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;
    tx_ch_config.src_addr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)uart_tx_buf);
    tx_ch_config.src_width =
        DMA_TRANSFER_WIDTH_BYTE; /*  In DMA handshake case, source width and destination width must be BYTE. */
    tx_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;
    tx_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
    tx_ch_config.size_in_byte = TEST_BUFFER_SIZE;
    tx_ch_config.src_burst_size =
        DMA_NUM_TRANSFER_PER_BURST_1T; /*  In DMA handshake case, source burst size must be 1 transfer, that is 0. */
    tx_ch_config.linked_ptr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)&descriptors[1]);
    stat = dma_config_linked_descriptor(TEST_UART_DMA_CONTROLLER, &descriptors[0], TEST_UART_TX_DMA_CH, &tx_ch_config);
    if (stat != status_success)
    {
        while (1)
        {
        };
    }

    tx_ch_config.linked_ptr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)&descriptors[0]);
    stat = dma_config_linked_descriptor(TEST_UART_DMA_CONTROLLER, &descriptors[1], TEST_UART_TX_DMA_CH, &tx_ch_config);
    if (stat != status_success)
    {
        while (1)
        {
        };
    }

    /* 1.2 start tx dma */
    tx_ch_config.linked_ptr = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)&descriptors[0]);
    stat = dma_setup_channel(TEST_UART_DMA_CONTROLLER, TEST_UART_TX_DMA_CH, &tx_ch_config, true);
    if (stat != status_success)
    {
        while (1)
        {
        };
    }
#else
#ifdef HPMSOC_HAS_HPMSDK_DMAV2
    /* 2. config uart tx dma */
    dma_handshake_config_t tx_ch_config = {0};
    dma_default_handshake_config(TEST_UART_DMA_CONTROLLER, &tx_ch_config);
    tx_ch_config.ch_index = TEST_UART_TX_DMA_CH;
    tx_ch_config.dst = (uint32_t)&TEST_UART->THR;
    tx_ch_config.dst_fixed = true;
    tx_ch_config.src = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)uart_tx_buf);
    tx_ch_config.src_fixed = false;
    tx_ch_config.data_width = DMA_TRANSFER_WIDTH_BYTE;
    tx_ch_config.size_in_byte = TEST_BUFFER_SIZE;
    tx_ch_config.en_infiniteloop = true;
    stat = dma_setup_handshake(TEST_UART_DMA_CONTROLLER, &tx_ch_config, true);
    if (stat != status_success)
    {
        while (1)
        {
        };
    }
#endif /* HPMSOC_HAS_HPMSDK_DMAV2 */
#endif /* APP_USE_LINKED_DMA */
}

#include "hpm_gptmr_drv.h"
void GPTMR2_Init(void)
{
    gptmr_channel_config_t config;
    clock_add_to_group(clock_gptmr2, 0);
    gptmr_channel_get_default_config(HPM_GPTMR2, &config);
    uint32_t freq = clock_get_frequency(clock_gptmr2);
    config.reload = freq / 20000;         /* 20khz*/
    config.cmp[0] = config.reload / 2; /*  */
    config.cmp[1] = config.reload;
    config.dma_request_event = gptmr_dma_request_on_cmp0;
    gptmr_channel_config(HPM_GPTMR2, 2, &config, true);
}

hpm_stat_t uart_tx_dma(uint8_t ch_num,
                    UART_Type *uart_ptr,
                    uint32_t src);
int main(void)
{
    board_init();
    for (int i = 0; i < TEST_BUFFER_SIZE; ++i)
    {
        uart_tx_buf[i] = i + '0';
    }
    uart_tx_buf[TEST_BUFFER_SIZE - 1] = '\n';

    init_board_app_uart();
    //init_board_app_dma();
    uart_tx_dma(0,TEST_UART,(uint32_t)uart_tx_buf);
    GPTMR2_Init();
    while (1)
    {
        ;
    }
    return 0;
}




//void GPTMR2_Init(void)
//{
//    gptmr_channel_config_t config;

//    //Gpt2Clock = clock_get_frequency(clock_gptmr2);

//    // GPTMER2.0 = PS_LOOP ISR
//    gptmr_channel_get_default_config(HPM_GPTMR2, &config);
//    config.reload = P_S_Prd;
//    config.enable_software_sync = false;
//    config.synci_edge = GPTMR_CHANNEL_CR_SYNCIREN_MASK;
//    config.cmp[0] = PS_CAL_BGN;
//    config.cmp[1] = PS_CAL_BGN + (P_S_Prd >> 1);
//    config.enable_cmp_output = true;
//    gptmr_channel_config(HPM_GPTMR2,0, &config, false);

//    gptmr_enable_irq(HPM_GPTMR2, GPTMR_CH_CMP_IRQ_MASK(0, 0));
//    gptmr_enable_irq(HPM_GPTMR2, GPTMR_CH_CMP_IRQ_MASK(0, 1));
//    intc_m_enable_irq_with_priority(IRQn_GPTMR2, 1);

//    // GPTMER2.2 = Encoder Trig
//    gptmr_channel_get_default_config(HPM_GPTMR2, &config);
//    config.reload = P_S_Prd;
//    config.enable_software_sync = false;
//    config.synci_edge = GPTMR_CHANNEL_CR_SYNCIREN_MASK;
//    config.cmp[0] = ENCODERTRG;
//    config.cmp[1] = ENCODERTRG + 10;
//    config.enable_cmp_output = true;
//    config.dma_request_event =  gptmr_dma_request_on_cmp0;
//    gptmr_channel_config(HPM_GPTMR2,2, &config, false);

//    gptmr_channel_reset_count(HPM_GPTMR2, 0);
//    gptmr_channel_reset_count(HPM_GPTMR2, 2);
//    gptmr_start_counter(HPM_GPTMR2, 0);      
//    gptmr_start_counter(HPM_GPTMR2, 2);      
    
     
//}

//ATTR_PLACE_AT_NONCACHEABLE_BSS_WITH_ALIGNMENT(8) dma_linked_descriptor_t descriptors[2];


hpm_stat_t uart_tx_dma(uint8_t ch_num,
                    UART_Type *uart_ptr,
                    uint32_t src)
{
    hpm_stat_t stat;
    dma_channel_config_t rx_ch_config = { 0 };
    //dma_disable_channel(HPM_HDMA, ch_num);
    dmamux_config(HPM_DMAMUX, ch_num, HPM_DMA_SRC_GPTMR2_2, true);

    /* 1.1 config chain descriptors */
    dma_default_channel_config(HPM_HDMA, &rx_ch_config);
    rx_ch_config.src_addr = src;
    rx_ch_config.src_width = DMA_TRANSFER_WIDTH_BYTE;  /*  In DMA handshake case, source width and destination width must be BYTE. */
    rx_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;//DMA_ADDRESS_CONTROL_INCREMENT;
    rx_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
    rx_ch_config.dst_addr = (uint32_t)&uart_ptr->THR;
    rx_ch_config.dst_width = DMA_TRANSFER_WIDTH_BYTE;  /*  In DMA handshake case, source width and destination width must be BYTE. */
    rx_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
    rx_ch_config.dst_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;
    rx_ch_config.size_in_byte = 1;
    rx_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;  /*  In DMA handshake case, source burst size must be 1 transfer, that is 0. */
   
    rx_ch_config.linked_ptr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)&descriptors[1]);//link to next dma action
    stat = dma_config_linked_descriptor(HPM_HDMA, &descriptors[0], ch_num, &rx_ch_config);
    
    rx_ch_config.linked_ptr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)&descriptors[0]);
    stat = dma_config_linked_descriptor(HPM_HDMA, &descriptors[1], ch_num, &rx_ch_config);
    
    stat = dma_setup_channel(HPM_HDMA, ch_num, &rx_ch_config, true);
    dmamux_enable_channel(HPM_DMAMUX, ch_num);
     
     return stat;
}

//#define ABSENCBRD 2500000U
//void Encoder_init(void)
//{
//    HPM_IOC->PAD[IOC_PAD_PD07].FUNC_CTL = IOC_PD07_FUNC_CTL_UART9_TXD;  //ABS_TXD
//    HPM_IOC->PAD[IOC_PAD_PD06].FUNC_CTL = IOC_PD06_FUNC_CTL_UART9_RXD;  //ABS_RXD
//    HPM_IOC->PAD[IOC_PAD_PD07].PAD_CTL = IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1);
//    HPM_IOC->PAD[IOC_PAD_PD06].PAD_CTL = IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1);
//    HPM_IOC->PAD[IOC_PAD_PD05].FUNC_CTL = IOC_PD05_FUNC_CTL_UART9_DE;    //ABS_DE
//    AbsCommValue[0][0] = 0x1a;
//    AbsCommValue[0][1] = 0x1a;
//    AbsCommValue[0][2] = 0x1a;
//    uart_config_t Iconfig = {0};
//    uart_default_config(HPM_UART9, &Iconfig);
//    Iconfig.src_freq_in_hz = UartClockHz;
//    Iconfig.baudrate = ABSENCBRD;
//    uart_init(HPM_UART9, &Iconfig);
//    // uart_tx_dma(0,HPM_UART9,(uint32_t)AbsCommValue[0]);
 
//}
