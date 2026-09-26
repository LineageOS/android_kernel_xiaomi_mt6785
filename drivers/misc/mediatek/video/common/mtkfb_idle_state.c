// SPDX-License-Identifier: GPL-2.0

#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/sysfs.h>

#include "mtkfb_idle_state.h"

static atomic_t mtkfb_idle = ATOMIC_INIT(0);
static struct kernfs_node *idle_state_kn;

static ssize_t idle_state_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n",
			 atomic_read(&mtkfb_idle) ? "idle" : "active");
}
static DEVICE_ATTR_RO(idle_state);

void mtkfb_idle_state_init(struct device *dev)
{
	if (device_create_file(dev, &dev_attr_idle_state)) {
		pr_warn("%s: failed to create idle_state\n", __func__);
		return;
	}

	idle_state_kn = sysfs_get_dirent(dev->kobj.sd, "idle_state");
}

void mtkfb_idle_state_set(bool idle)
{
	if (idle_state_kn && atomic_xchg(&mtkfb_idle, idle) != idle)
		sysfs_notify_dirent(idle_state_kn);
}
