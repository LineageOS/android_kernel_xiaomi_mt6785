/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __SCP_DVFS_H__
#define __SCP_DVFS_H__

#define SCP_DVFS_USE_PLL		1

#define PLL_ENABLE				(1)
#define PLL_DISABLE				(0)

#define DVFS_STATUS_OK			(0)
#define DVFS_STATUS_BUSY		(-1)
#define DVFS_REQUEST_SAME_CLOCK	(-2)
#define DVFS_STATUS_ERR			(-3)
#define DVFS_STATUS_TIMEOUT		(-4)
#define DVFS_CLK_ERROR			(-5)
#define DVFS_STATUS_CMD_FIX		(-6)
#define DVFS_STATUS_CMD_LIMITED	(-7)
#define DVFS_STATUS_CMD_DISABLE	(-8)

enum scp_state_enum {
	IN_DEBUG_IDLE = 1,
	ENTERING_SLEEP = 2,
	IN_SLEEP = 4,
	ENTERING_ACTIVE = 8,
	IN_ACTIVE = 16,
};

enum clk_opp_enum {
	CLK_26M	 = 26,
	CLK_OPP0 = 196,
	CLK_OPP1 = 260,
	CLK_OPP2 = 280,
	CLK_OPP3 = 360,
	CLK_OPP4 = 416,
};

enum scp_req_r {
	SCP_REQ_RELEASE = 0,
	SCP_REQ_26M = 1 << 0,
	SCP_REQ_IFR = 1 << 1,
	SCP_REQ_SYSPLL1 = 1 << 2,
	SCP_REQ_MAX = 0xffff,
};

struct mt_scp_pll_t {
	struct clk *clk_mux;
	struct clk *clk_pll0;
	struct clk *clk_pll1;
	struct clk *clk_pll2;
	struct clk *clk_pll3;
	struct clk *clk_pll4;
	struct clk *clk_pll5;
	struct clk *clk_pll6;
	struct clk *clk_pll7;
};

extern int scp_pll_ctrl_set(unsigned int pll_ctrl_flag, unsigned int pll_sel);
extern int scp_set_pmic_vcore(unsigned int cur_freq);
extern unsigned int scp_get_dvfs_opp(void);
extern uint32_t scp_get_freq(void);
extern int scp_request_freq(void);
extern void scp_pll_mux_set(unsigned int pll_ctrl_flag);
extern void wait_scp_dvfs_init_done(void);
extern int __init scp_dvfs_init(void);
extern void __exit scp_dvfs_exit(void);
extern int scp_resource_req(unsigned int req_type);

/* scp dvfs variable*/
extern unsigned int scp_expected_freq;
extern unsigned int scp_current_freq;
extern spinlock_t scp_awake_spinlock;

#endif  /* __SCP_DVFS_H__ */