/*
 * Copyright (c) 2021-2024 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*---------------------------------------------------------------------
 * Includes
 *---------------------------------------------------------------------
 */
#include "hpm_enet_drv.h"
#include "hpm_jl2101_regs.h"
#include "hpm_jl2101.h"

/*---------------------------------------------------------------------
 * Internal API
 *---------------------------------------------------------------------
 */
static bool jl2101_check_id(ENET_Type *ptr)
{
    uint16_t id1, id2;

    id1 = enet_read_phy(ptr, JL2101_ADDR, JL2101_PHYID1);
    id2 = enet_read_phy(ptr, JL2101_ADDR, JL2101_PHYID2);

    if (JL2101_PHYID1_OUI_MSB_GET(id1) == JL2101_ID1 && JL2101_PHYID2_OUI_LSB_GET(id2) == JL2101_ID2) {
        return true;
    } else {
        return false;
    }
}

/*---------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------
 */
void jl2101_reset(ENET_Type *ptr)
{
    uint16_t data;

    /* PHY reset */
    enet_write_phy(ptr, JL2101_ADDR, JL2101_BMCR, JL2101_BMCR_RESET_SET(1));

    /* wait until the reset is completed */
    do {
        data = enet_read_phy(ptr, JL2101_ADDR, JL2101_BMCR);
    } while (JL2101_BMCR_RESET_GET(data));
}

void jl2101_basic_mode_default_config(ENET_Type *ptr, jl2101_config_t *config)
{
    (void)ptr;

    config->loopback         = false;                        /* Disable PCS loopback mode */
    #if defined(__DISABLE_AUTO_NEGO) && (__DISABLE_AUTO_NEGO)
    config->auto_negotiation = false;                        /* Disable Auto-Negotiation */
    config->speed            = enet_phy_port_speed_100mbps;
    config->duplex           = enet_phy_duplex_full;
    #else
    config->auto_negotiation = true;                         /* Enable Auto-Negotiation */
    #endif
}

bool jl2101_basic_mode_init(ENET_Type *ptr, jl2101_config_t *config)
{
    uint16_t data = 0;

    data |= JL2101_BMCR_RESET_SET(0)                        /* Normal operation */
         |  JL2101_BMCR_LOOPBACK_SET(config->loopback)      /* configure PCS loopback mode */
         |  JL2101_BMCR_ANE_SET(config->auto_negotiation)   /* configure Auto-Negotiation */
         |  JL2101_BMCR_PWD_SET(0)                          /* Normal operation */
         |  JL2101_BMCR_ISOLATE_SET(0)                      /* Normal operation */
         |  JL2101_BMCR_RESTART_AN_SET(0)                   /* Normal operation (ignored when Auto-Negotiation is disabled) */
         |  JL2101_BMCR_COLLISION_TEST_SET(0);              /* Normal operation */

    if (config->auto_negotiation == 0) {
        data |= JL2101_BMCR_SPEED0_SET(config->speed) | JL2101_BMCR_SPEED1_SET(config->speed >> 1);   /* Set port speed */
        data |= JL2101_BMCR_DUPLEX_SET(config->duplex);                                                /* Set duplex mode */
    }

    /* check the id of jl2101 */
    if (jl2101_check_id(ptr) == false) {
        return false;
    }

    enet_write_phy(ptr, JL2101_ADDR, JL2101_BMCR, data);
    /* Disable CLKOUT */
    enet_write_phy(ptr, JL2101_ADDR, JL2101_PAGSR, 0xA43);
    data = enet_read_phy(ptr, JL2101_ADDR, JL2101_PHYCR2_P_A43);
    data |= JL2101_PHYCR2_P_A43_CLKOUT_ENABLE_SET(0);
    enet_write_phy(ptr, JL2101_ADDR, JL2101_PHYCR2_P_A43, data);
    enet_write_phy(ptr, JL2101_ADDR, JL2101_PAGSR, 0);

    return true;
}


void jl2101_get_phy_status(ENET_Type *ptr, enet_phy_status_t *status)
{
    uint16_t data;

    /* select page 0xA43 */
    enet_write_phy(ptr, JL2101_ADDR, JL2101_PAGSR, 0xA43);

    data = enet_read_phy(ptr, JL2101_ADDR, JL2101_PHYSR_P_A43);
    status->enet_phy_link = JL2101_PHYSR_P_A43_LINK_STATUS_GET(data);
    status->enet_phy_speed = JL2101_PHYSR_P_A43_SPEED_STATUS_GET(data) == 0 ? enet_phy_port_speed_10mbps : JL2101_PHYSR_P_A43_SPEED_STATUS_GET(data) == 1 ? enet_phy_port_speed_100mbps : enet_phy_port_speed_1000mbps;
    status->enet_phy_duplex = JL2101_PHYSR_P_A43_DUPLEX_STATUS_GET(data);

    enet_write_phy(ptr, JL2101_ADDR, JL2101_PAGSR, 0);
}
