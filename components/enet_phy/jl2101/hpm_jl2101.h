/*
 * Copyright (c) 2021-2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HPM_JL2101_H
#define HPM_JL2101_H

/*---------------------------------------------------------------------
 * Includes
 *---------------------------------------------------------------------
 */
#include "hpm_enet_phy.h"
#include "hpm_common.h"
#include "hpm_enet_regs.h"
/*---------------------------------------------------------------------
 *
 *  Macro Const Definitions
 *---------------------------------------------------------------------
 */
#ifndef JL2101_ADDR
#define JL2101_ADDR (1U)
#endif

#define JL2101_ID1  (0x937CU)
#define JL2101_ID2  (0x10U)

/*---------------------------------------------------------------------
 *  Typedef Struct Declarations
 *---------------------------------------------------------------------
 */
typedef struct {
    bool loopback;
    uint8_t speed;
    bool auto_negotiation;
    uint8_t duplex;
} jl2101_config_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
/*---------------------------------------------------------------------
 * Exported Functions
 *---------------------------------------------------------------------
 */
void jl2101_reset(ENET_Type *ptr);
void jl2101_basic_mode_default_config(ENET_Type *ptr, jl2101_config_t *config);
bool jl2101_basic_mode_init(ENET_Type *ptr, jl2101_config_t *config);
void jl2101_get_phy_status(ENET_Type *ptr, enet_phy_status_t *status);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* HPM_JL2101_H */
