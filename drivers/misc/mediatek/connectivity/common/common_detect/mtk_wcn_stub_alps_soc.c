/*
* Copyright (C) 2011-2014 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

/*
 *  ! \file
 *  \brief  Declaration of library functions
 *  Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
 */

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/


/* kernel wmt_build_in_adapter.c already has these, so always ignored */
#ifdef MTK_WCN_REMOVE_KERNEL_MODULE
#undef MTK_WCN_REMOVE_KERNEL_MODULE
#endif

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define CMB_STUB_DBG_LOG                  3
#define CMB_STUB_INFO_LOG                 2
#define CMB_STUB_WARN_LOG                 1

int gCmbStubLogLevel = CMB_STUB_INFO_LOG;

#define CMB_STUB_LOG_PR_INFO(fmt, arg...) \
do { \
	if (gCmbStubLogLevel >= CMB_STUB_INFO_LOG) \
		pr_info(fmt, ##arg); \
} while (0)
#define CMB_STUB_LOG_PR_WARN(fmt, arg...) \
do { \
	if (gCmbStubLogLevel >= CMB_STUB_WARN_LOG) \
		pr_warn(fmt, ##arg); \
} while (0)
#define CMB_STUB_LOG_PR_DBG(fmt, arg...) \
do { \
	if (gCmbStubLogLevel >= CMB_STUB_DBG_LOG) \
		pr_info(fmt, ##arg); \
} while (0)

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <mtk_wcn_cmb_stub.h>
#include <wmt_build_in_adapter.h>
#include "wmt_detect.h"


/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

int gConnectivityChipId = -1;

/*
* current used uart port name, default is "ttyMT2",
* will be changed when wmt driver init
*/
char *wmt_uart_port_desc = "ttyMT2";
EXPORT_SYMBOL(wmt_uart_port_desc);

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

static wmt_aif_ctrl_cb cmb_stub_aif_ctrl_cb;
static wmt_func_ctrl_cb cmb_stub_func_ctrl_cb;
static wmt_thermal_query_cb cmb_stub_thermal_ctrl_cb;
static wmt_trigger_assert_cb cmb_stub_trigger_assert_cb;
static enum CMB_STUB_AIF_X cmb_stub_aif_stat = CMB_STUB_AIF_0;
static wmt_deep_idle_ctrl_cb cmb_stub_deep_idle_ctrl_cb;
static wmt_func_do_reset cmb_stub_do_reset_cb;
static wmt_clock_fail_dump_cb cmb_stub_clock_fail_dump_cb;
/* A temp translation table between COMBO_AUDIO_STATE_X and CMB_STUB_AIF_X.
 * This is used for ALPS backward compatible ONLY!!! Remove this table, related
 * functions, and type definition after modifying other kernel built-in modules,
 * such as AUDIO. [FixMe][GeorgeKuo]
 */
#if 0
static enum CMB_STUB_AIF_X audio2aif[] = {
	[COMBO_AUDIO_STATE_0] = CMB_STUB_AIF_0,
	[COMBO_AUDIO_STATE_1] = CMB_STUB_AIF_1,
	[COMBO_AUDIO_STATE_2] = CMB_STUB_AIF_2,
	[COMBO_AUDIO_STATE_3] = CMB_STUB_AIF_3,
};
#endif

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

#ifndef MTK_WCN_REMOVE_KERNEL_MODULE
static int _mtk_wcn_cmb_stub_query_ctrl(void);
static int _mtk_wcn_cmb_stub_trigger_assert(void);
static void _mtk_wcn_cmb_stub_clock_fail_dump(void);
#endif /* MTK_WCN_REMOVE_KERNEL_MODULE */

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*!
 * \brief A registration function for WMT-PLAT to register itself to CMB-STUB.
 *
 * An MTK-WCN-CMB-STUB registration function provided to WMT-PLAT to register
 * itself and related callback functions when driver being loaded into kernel.
 *
 * \param p_stub_cb a pointer carrying CMB_STUB_CB information
 *
 * \retval 0 operation success
 * \retval -1 invalid parameters
 */
int mtk_wcn_cmb_stub_reg(struct _CMB_STUB_CB_ *p_stub_cb)
{
#ifndef MTK_WCN_REMOVE_KERNEL_MODULE
	struct wmt_platform_bridge pbridge;

	memset(&pbridge, 0, sizeof(struct wmt_platform_bridge));
#endif

	if ((!p_stub_cb)
	    || (p_stub_cb->size != sizeof(struct _CMB_STUB_CB_))) {
		CMB_STUB_LOG_PR_WARN("[cmb_stub] invalid p_stub_cb:0x%p size(%d)\n",
				  p_stub_cb, (p_stub_cb) ? p_stub_cb->size : 0);
		return -1;
	}

	CMB_STUB_LOG_PR_DBG("[cmb_stub] registered, p_stub_cb:0x%p size(%d)\n",
			p_stub_cb, p_stub_cb->size);

	cmb_stub_aif_ctrl_cb = p_stub_cb->aif_ctrl_cb;
	cmb_stub_func_ctrl_cb = p_stub_cb->func_ctrl_cb;
	cmb_stub_thermal_ctrl_cb = p_stub_cb->thermal_query_cb;
	cmb_stub_trigger_assert_cb = p_stub_cb->trigger_assert_cb;
	cmb_stub_deep_idle_ctrl_cb = p_stub_cb->deep_idle_ctrl_cb;
	cmb_stub_do_reset_cb = p_stub_cb->wmt_do_reset_cb;
	cmb_stub_clock_fail_dump_cb = p_stub_cb->clock_fail_dump_cb;

#ifndef MTK_WCN_REMOVE_KERNEL_MODULE
	pbridge.thermal_query_cb = _mtk_wcn_cmb_stub_query_ctrl;
	pbridge.trigger_assert_cb = _mtk_wcn_cmb_stub_trigger_assert;
	pbridge.clock_fail_dump_cb = _mtk_wcn_cmb_stub_clock_fail_dump;
	wmt_export_platform_bridge_register(&pbridge);
#endif

	return 0;
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_reg);
/*!
 * \brief A unregistration function for WMT-PLAT to unregister from CMB-STUB.
 *
 * An MTK-WCN-CMB-STUB unregistration function provided to WMT-PLAT to
 * unregister itself and clear callback function references.
 *
 * \retval 0 operation success
 */
int mtk_wcn_cmb_stub_unreg(void)
{
#ifndef MTK_WCN_REMOVE_KERNEL_MODULE
	wmt_export_platform_bridge_unregister();
#endif

	cmb_stub_aif_ctrl_cb = NULL;
	cmb_stub_func_ctrl_cb = NULL;
	cmb_stub_thermal_ctrl_cb = NULL;
	cmb_stub_deep_idle_ctrl_cb = NULL;
	cmb_stub_do_reset_cb = NULL;
	cmb_stub_clock_fail_dump_cb = NULL;
	CMB_STUB_LOG_PR_INFO("[cmb_stub] unregistered\n");	/* KERN_DEBUG */

	return 0;
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_unreg);

/* stub functions for kernel to control audio path pin mux */
int mtk_wcn_cmb_stub_aif_ctrl(enum CMB_STUB_AIF_X state, enum CMB_STUB_AIF_CTRL ctrl)
{
	int ret;

	if ((state >= CMB_STUB_AIF_MAX)
	    || (ctrl >= CMB_STUB_AIF_CTRL_MAX)) {

		CMB_STUB_LOG_PR_WARN("[cmb_stub] aif_ctrl invalid (%d, %d)\n",
				state, ctrl);
		return -1;
	}

	/* avoid the early interrupt before we register the eirq_handler */
	if (cmb_stub_aif_ctrl_cb) {
		ret = (*cmb_stub_aif_ctrl_cb) (state, ctrl);
		CMB_STUB_LOG_PR_INFO("aif state(%d->%d) ctrl(%d) ret(%d)\n",
		cmb_stub_aif_stat, state, ctrl, ret);	/* KERN_DEBUG */

		cmb_stub_aif_stat = state;
	} else {
		CMB_STUB_LOG_PR_WARN("[cmb_stub] aif_ctrl_cb null\n");
		ret = -2;
	}
	return ret;
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_aif_ctrl);

/* Use a temp translation table between COMBO_AUDIO_STATE_X and CMB_STUB_AIF_X
 * for ALPS backward compatible ONLY!!! Remove this table, related functions,
 * and type definition after modifying other kernel built-in modules, such as
 * AUDIO. [FixMe][GeorgeKuo]
 */

void mtk_wcn_cmb_stub_func_ctrl(unsigned int type, unsigned int on)
{
	if (cmb_stub_func_ctrl_cb)
		(*cmb_stub_func_ctrl_cb) (type, on);
	else
		CMB_STUB_LOG_PR_WARN("[cmb_stub] func_ctrl_cb null\n");
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_func_ctrl);

#ifdef MTK_WCN_REMOVE_KERNEL_MODULE
int mtk_wcn_cmb_stub_query_ctrl(void)
#else
static int _mtk_wcn_cmb_stub_query_ctrl(void)
#endif
{
	signed long temp = 0;

	if (cmb_stub_thermal_ctrl_cb)
		temp = (*cmb_stub_thermal_ctrl_cb) ();
	else
		CMB_STUB_LOG_PR_WARN("[cmb_stub] thermal_ctrl_cb null\n");

	return temp;
}

#ifdef MTK_WCN_REMOVE_KERNEL_MODULE
int mtk_wcn_cmb_stub_trigger_assert(void)
#else
static int _mtk_wcn_cmb_stub_trigger_assert(void)
#endif
{
	int ret = 0;

	if (cmb_stub_trigger_assert_cb)
		ret = (*cmb_stub_trigger_assert_cb) ();
	else
		CMB_STUB_LOG_PR_WARN("[cmb_stub] trigger_assert_cb null\n");

	return ret;
}

#ifndef MTK_WCN_REMOVE_KERNEL_MODULE
void _mtk_wcn_cmb_stub_clock_fail_dump(void)
{
	if (cmb_stub_clock_fail_dump_cb)
		(*cmb_stub_clock_fail_dump_cb) ();
	else
		CMB_STUB_LOG_PR_WARN("[cmb_stub] clock_fail_dump_cb null\n");
}
#endif

/*platform-related APIs*/
/* void clr_device_working_ability(UINT32 clockId, MT6573_STATE state); */
/* void set_device_working_ability(UINT32 clockId, MT6573_STATE state); */

static int _mt_combo_plt_do_deep_idle(enum COMBO_IF src, int enter)
{
	int ret = -1;

#if 0
	if (src != COMBO_IF_UART && src != COMBO_IF_MSDC && src != COMBO_IF_BTIF) {
		CMB_STUB_LOG_PR_WARN("src = %d is error\n", src);
		return ret;
	}
	if (src >= 0 && src < COMBO_IF_MAX)
		CMB_STUB_LOG_PR_INFO("src = %s, to enter deep idle? %d\n",
				combo_if_name[src], enter);
#endif
	/*
	 *  TODO: For Common SDIO configuration, we need to do some judgement between STP and WIFI
	 *  to decide if the msdc will enter deep idle safely
	 */

	switch (src) {
	case COMBO_IF_BTIF:
		if (cmb_stub_deep_idle_ctrl_cb)
			ret = (*cmb_stub_deep_idle_ctrl_cb) (enter);
		else
			CMB_STUB_LOG_PR_WARN("NULL function pointer\n");

		if (ret)
			CMB_STUB_LOG_PR_WARN("%s deep idle fail(%d)\n",
					enter == 1 ? "enter" : "exit", ret);
		else
			CMB_STUB_LOG_PR_DBG("%s deep idle ok(%d)\n",
					enter == 1 ? "enter" : "exit", ret);
		break;
	default:
		break;
	}

	return ret;
}

int mt_combo_plt_enter_deep_idle(enum COMBO_IF src)
{
	/* return 0; */
	/* TODO: [FixMe][GeorgeKuo] handling this depends on common UART or common SDIO */
	return _mt_combo_plt_do_deep_idle(src, 1);
}
EXPORT_SYMBOL(mt_combo_plt_enter_deep_idle);

int mt_combo_plt_exit_deep_idle(enum COMBO_IF src)
{
	/* return 0; */
	/* TODO: [FixMe][GeorgeKuo] handling this depends on common UART or common SDIO */
	return _mt_combo_plt_do_deep_idle(src, 0);
}
EXPORT_SYMBOL(mt_combo_plt_exit_deep_idle);

int mtk_wcn_wmt_chipid_query(void)
{
	CMB_STUB_LOG_PR_INFO("query current consys chipid (0x%x)\n",
			gConnectivityChipId);
	return gConnectivityChipId;
}
EXPORT_SYMBOL(mtk_wcn_wmt_chipid_query);

void mtk_wcn_wmt_set_chipid(int chipid)
{
	CMB_STUB_LOG_PR_INFO("set current consys chipid (0x%x)\n", chipid);
	gConnectivityChipId = chipid;
}
EXPORT_SYMBOL(mtk_wcn_wmt_set_chipid);

int mtk_wcn_cmb_stub_do_reset(unsigned int type)
{
	if (cmb_stub_do_reset_cb)
		return (*cmb_stub_do_reset_cb) (type);
	else
		return -1;
}
EXPORT_SYMBOL(mtk_wcn_cmb_stub_do_reset);
