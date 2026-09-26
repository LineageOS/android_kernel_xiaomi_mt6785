/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __MTKFB_IDLE_STATE_H__
#define __MTKFB_IDLE_STATE_H__

#include <linux/device.h>

void mtkfb_idle_state_init(struct device *dev);
void mtkfb_idle_state_set(bool idle);

#endif /* __MTKFB_IDLE_STATE_H__ */
