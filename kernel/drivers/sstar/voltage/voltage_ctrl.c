/*
 * voltage_ctrl.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "registers.h"
#include "ms_platform.h"
#include "cam_os_wrapper.h"
#include "cam_inter_os.h"
#include "drv_dualos.h"
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"
#include "voltage_request_init.h"

#define VOLCTRL_DEBUG 0

#if VOLCTRL_DEBUG
#define VOLCTRL_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define VOLCTRL_DBG(fmt, arg...)
#endif
#define VOLCTRL_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#ifdef CONFIG_SSTAR_VOLTAGE_CTRL_WITH_OSC
struct voltage_range
{
    int TT;
    int LV;
};
#endif

struct voltage_ctrl
{
    const char *name;

    u32  gpio_vid_width;
    u32 *gpio_vid_pins;
    u32 *gpio_vid_voltages;

    int                     current_voltage;
    int                     lt_voltage; // Vmin under low temperature
    u8                      voltage_gpio_inited;
    struct device           dev_core;
    u8                      enable_scaling_voltage;
    struct platform_device *pdev;
    CamOsSpinlock_t         core_voltage_spinlock;
    int                     voltage_demander_request_value[VOLTAGE_DEMANDER_MAX];
#ifdef CONFIG_SSTAR_VOLTAGE_CTRL_WITH_OSC
    struct voltage_range voltage_demander_range[VOLTAGE_DEMANDER_MAX];
#endif
    struct list_head list;
};

const char *_gVoltageDemanderName[] = {FOREACH_DEMANDER(GENERATE_STRING)};

static LIST_HEAD(voltage_ctrl_list);

static u8 voltage_subsys_registered;

static struct bus_type voltage_subsys = {
    .name     = "voltage",
    .dev_name = "voltage",
};

#ifdef CONFIG_SS_DUALOS
static int controller_name_to_id(const char *name)
{
    if (!strcmp(name, "core_power"))
        return VOLTAGE_CONTROLLER_CORE;

    return -1;
}
#endif

struct voltage_ctrl *get_voltage_ctrl(const char *name)
{
    struct voltage_ctrl *ctrl = NULL;
    list_for_each_entry(ctrl, &voltage_ctrl_list, list)
    {
        if (!strcmp(ctrl->name, name))
        {
            return ctrl;
        }
    }
    return NULL;
}

static int check_voltage_valid(struct voltage_ctrl *ctrl, int mV)
{
    int voltage_lv_cnt = 0;
    int vcore_total_lv = 1 << ctrl->gpio_vid_width;

    // Check core voltage setting(mV) valid or not.
    for (voltage_lv_cnt = 0; voltage_lv_cnt < vcore_total_lv; voltage_lv_cnt++)
    {
        if (mV == ctrl->gpio_vid_voltages[voltage_lv_cnt])
        {
            break;
        }
    }

    if (voltage_lv_cnt == vcore_total_lv) // If core voltage setting(mV) is invalid, Try to find ceiling setting.
    {
        for (voltage_lv_cnt = 0; voltage_lv_cnt < vcore_total_lv; voltage_lv_cnt++)
        {
            if (mV < ctrl->gpio_vid_voltages[voltage_lv_cnt])
            {
                VOLCTRL_DBG("[Core Voltage] %s: Not support %dmV, use %dmV\n", __FUNCTION__, mV,
                            ctrl->gpio_vid_voltages[voltage_lv_cnt]);
                mV = ctrl->gpio_vid_voltages[voltage_lv_cnt];
                break;
            }
        }

        if (voltage_lv_cnt == vcore_total_lv) // If no ceiling setting, use highest setting.
        {
            voltage_lv_cnt--;
            VOLCTRL_DBG("[Core Voltage] %s: Not support %dmV, use %dmV\n", __FUNCTION__, mV,
                        ctrl->gpio_vid_voltages[voltage_lv_cnt]);
            mV = ctrl->gpio_vid_voltages[voltage_lv_cnt];
        }
    }

    return mV;
}

static void sync_core_voltage(struct voltage_ctrl *ctrl)
{
    int voltage_lv_cnt = 0;
    int gpio_lv_cnt    = 0;
    int i              = 0;
    int vcore_max      = 0;
    int vcore_total_lv = 1 << ctrl->gpio_vid_width;

    CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);

    if (ctrl->voltage_gpio_inited && ctrl->gpio_vid_width && ctrl->enable_scaling_voltage)
    {
        for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
        {
            vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
        }
        if (!vcore_max)
            goto _skip_sync;
        vcore_max = check_voltage_valid(ctrl, vcore_max);
        VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

        if (ctrl->gpio_vid_width > 0)
        {
            for (voltage_lv_cnt = 0; voltage_lv_cnt < vcore_total_lv; voltage_lv_cnt++)
            {
                if (vcore_max == ctrl->gpio_vid_voltages[voltage_lv_cnt])
                {
                    for (gpio_lv_cnt = 0; gpio_lv_cnt < ctrl->gpio_vid_width; gpio_lv_cnt++)
                    {
                        if ((voltage_lv_cnt >> gpio_lv_cnt) & 0x1)
                        {
                            gpio_direction_output(ctrl->gpio_vid_pins[gpio_lv_cnt], 1);
                            VOLCTRL_DBG("[Core Voltage] %s: gpio%d set high\n", __FUNCTION__,
                                        ctrl->gpio_vid_pins[gpio_lv_cnt]);
                        }
                        else
                        {
                            gpio_direction_output(ctrl->gpio_vid_pins[gpio_lv_cnt], 0);
                            VOLCTRL_DBG("[Core Voltage] %s: gpio%d set low\n", __FUNCTION__,
                                        ctrl->gpio_vid_pins[gpio_lv_cnt]);
                        }
                    }

                    ctrl->current_voltage = ctrl->gpio_vid_voltages[voltage_lv_cnt];
                    break;
                }
            }
        }
    }
    else
    {
        VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
    }

    VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

_skip_sync:
    CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);
}

#ifdef CONFIG_SSTAR_VOLTAGE_CTRL_WITH_OSC
static void voltage_level_reconfig(struct voltage_ctrl *ctrl, VOLTAGE_DEMANDER_E demander, int voltage)
{
    VOLCTRL_ERR("[Core Voltage] %s: Not Supported\n", __FUNCTION__);

    if (demander == VOLTAGE_DEMANDER_TEMPERATURE)
        return;

    ctrl->voltage_demander_range[demander].TT = voltage;

    if (demander == VOLTAGE_DEMANDER_USER || (ctrl->gpio_vid_width == 1))
    {
        ctrl->voltage_demander_range[demander].LV = voltage;
    }
    else
    {
        if (voltage == VOLTAGE_CORE_1000)
        {
            ctrl->voltage_demander_range[demander].LV = VOLTAGE_CORE_950;
        }
        else
        {
            ctrl->voltage_demander_range[demander].LV = VOLTAGE_CORE_900;
        }
    }
}

int sync_core_voltage_with_OSC_and_TEMP(const char *name, int useTT)
{
    int                  i;
    int                  update = 0;
    struct voltage_ctrl *ctrl;

    VOLCTRL_ERR("[Core Voltage] %s: Not Supported\n", __FUNCTION__);
    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return 0;
    }
    CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);

    for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
    {
        if (ctrl->voltage_demander_request_value[i] != 0)
        {
            if (useTT)
            {
                if (ctrl->voltage_demander_request_value[i] != ctrl->voltage_demander_range[i].TT)
                {
                    ctrl->voltage_demander_request_value[i] = ctrl->voltage_demander_range[i].TT;
                    update                                  = 1;
                }
            }
            else
            {
                if (ctrl->voltage_demander_request_value[i] != ctrl->voltage_demander_range[i].LV)
                {
                    ctrl->voltage_demander_request_value[i] = ctrl->voltage_demander_range[i].LV;
                    update                                  = 1;
                }
            }
        }
    }
    CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);

    return update;
}

EXPORT_SYMBOL(sync_core_voltage_with_OSC_and_TEMP);
#endif

int set_core_voltage(const char *name, VOLTAGE_DEMANDER_E demander, u32 mV)
{
#ifdef CONFIG_SS_DUALOS
    int controller;
#endif
    int                  voltage_lv_cnt = 0;
    int                  gpio_lv_cnt    = 0;
    int                  i              = 0;
    int                  vcore_max      = 0;
    int                  vcore_total_lv;
    struct voltage_ctrl *ctrl;

    if (demander >= VOLTAGE_DEMANDER_MAX)
    {
        VOLCTRL_DBG("[Core Voltage] %s: demander number out of range (%d)\n", __FUNCTION__, demander);
        return -EINVAL;
    }

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }
#ifdef CONFIG_SS_DUALOS
    if (dualos_rtos_exist())
    {
        if (ctrl->enable_scaling_voltage)
        {
            controller = controller_name_to_id(name);
            if (controller < 0)
            {
                VOLCTRL_DBG("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, name);
                return -EINVAL;
            }

            CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_SET, (u32)controller, (u32)demander, (u32)mV);
        }
    }
    else
#endif
    {
        vcore_total_lv = 1 << ctrl->gpio_vid_width;

        CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);

#ifdef CONFIG_SSTAR_VOLTAGE_CTRL_WITH_OSC
        voltage_level_reconfig(ctrl, demander, mV);
#endif
        ctrl->voltage_demander_request_value[demander] = mV;
        VOLCTRL_DBG("[Core Voltage] %s: %s request %dmV\n", __FUNCTION__, _gVoltageDemanderName[demander], mV);

        if (ctrl->voltage_gpio_inited && ctrl->gpio_vid_width && ctrl->enable_scaling_voltage)
        {
            for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
            {
                vcore_max = max(vcore_max, ctrl->voltage_demander_request_value[i]);
            }
            if (!vcore_max)
                goto _skip_set_vol;
            vcore_max = check_voltage_valid(ctrl, vcore_max);
            VOLCTRL_DBG("[Core Voltage] %s: maximum request is %dmV\n", __FUNCTION__, vcore_max);

            if (ctrl->gpio_vid_width > 0)
            {
                for (voltage_lv_cnt = 0; voltage_lv_cnt < vcore_total_lv; voltage_lv_cnt++)
                {
                    if (vcore_max == ctrl->gpio_vid_voltages[voltage_lv_cnt])
                    {
                        for (gpio_lv_cnt = 0; gpio_lv_cnt < ctrl->gpio_vid_width; gpio_lv_cnt++)
                        {
                            if ((voltage_lv_cnt >> gpio_lv_cnt) & 0x1)
                            {
                                gpio_direction_output(ctrl->gpio_vid_pins[gpio_lv_cnt], 1);
                                VOLCTRL_DBG("[Core Voltage] %s: gpio%d set high\n", __FUNCTION__,
                                            ctrl->gpio_vid_pins[gpio_lv_cnt]);
                            }
                            else
                            {
                                gpio_direction_output(ctrl->gpio_vid_pins[gpio_lv_cnt], 0);
                                VOLCTRL_DBG("[Core Voltage] %s: gpio%d set low\n", __FUNCTION__,
                                            ctrl->gpio_vid_pins[gpio_lv_cnt]);
                            }
                        }

                        ctrl->current_voltage = ctrl->gpio_vid_voltages[voltage_lv_cnt];
                        break;
                    }
                }
            }
        }
        else
        {
            VOLCTRL_DBG("[Core Voltage] %s: voltage scaling not enable\n", __FUNCTION__);
        }

        VOLCTRL_DBG("[Core Voltage] %s: current core voltage %dmV\n", __FUNCTION__, ctrl->current_voltage);

    _skip_set_vol:
        CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);
    }

    return 0;
}
EXPORT_SYMBOL(set_core_voltage);

int get_core_voltage(const char *name, u32 *mV)
{
    struct voltage_ctrl *ctrl;
#ifdef CONFIG_SS_DUALOS
    int controller;
#endif

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }
#ifdef CONFIG_SS_DUALOS
    if (dualos_rtos_exist())
    {
        controller = controller_name_to_id(name);
        if (controller < 0)
        {
            VOLCTRL_DBG("[Core Voltage] %s: invalid controller name %s\n", __FUNCTION__, name);
            return -EINVAL;
        }
        *mV = (int)CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_GET, (u32)controller, 0, 0);
    }
    else
#endif
    {
        *mV = ctrl->current_voltage;
    }

    return 0;
}
EXPORT_SYMBOL(get_core_voltage);

int get_core_lt_voltage(const char *name, int *mV)
{
    struct voltage_ctrl *ctrl;

    *mV = 0;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }
    *mV = ctrl->lt_voltage;

    return 0;
}
EXPORT_SYMBOL(get_core_lt_voltage);

int get_voltage_width(const char *name, unsigned int *num)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    if (num)
    {
        *num = 1 << ctrl->gpio_vid_width;
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(get_voltage_width);

static int core_voltage_get_gpio(struct voltage_ctrl *ctrl)
{
    struct device_node *np = ctrl->pdev->dev.of_node;
    u32                 vid_width;
    u32 *               pbuf_vid_pins     = NULL;
    u32 *               pbuf_vid_voltages = NULL;
    unsigned int        i;
    char                name[10];
    int                 ret;

    if (0 != of_property_read_u32(np, "vid_width", &vid_width) || vid_width < 1)
        goto core_voltage_alloc_err;

    pbuf_vid_pins = vzalloc(sizeof(unsigned int) * vid_width);
    if (!pbuf_vid_pins)
        goto core_voltage_alloc_err;

    pbuf_vid_voltages = vzalloc(sizeof(unsigned int) * (1 << vid_width));
    if (!pbuf_vid_voltages)
        goto core_voltage_alloc_err;

    if (vid_width != of_property_read_variable_u32_array(np, "vid_gpios", pbuf_vid_pins, 0, vid_width))
        goto core_voltage_alloc_err;
    if ((1 << vid_width)
        != of_property_read_variable_u32_array(np, "vid_voltages", pbuf_vid_voltages, 0, (1 << vid_width)))
        goto core_voltage_alloc_err;

#ifdef CONFIG_SS_DUALOS
    if (!dualos_rtos_exist())
#endif
    {
        for (i = 0; i < vid_width; i++)
        {
            sprintf(name, "vid%d", i);
            ret = gpio_request(pbuf_vid_pins[i], (const char *)name);
            if (ret)
                goto core_voltage_alloc_err;
            gpio_export(pbuf_vid_pins[i], 0);
        }
    }

    CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);

    // get Vmin under low temperature
    if (0 != of_property_read_u32(np, "lt_voltage", &ctrl->lt_voltage))
        ctrl->lt_voltage = 0;

    ctrl->gpio_vid_width      = vid_width;
    ctrl->gpio_vid_pins       = pbuf_vid_pins;
    ctrl->gpio_vid_voltages   = pbuf_vid_voltages;
    ctrl->voltage_gpio_inited = 1;

    CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);
    return 0;

core_voltage_alloc_err:
    if (pbuf_vid_pins)
    {
        vfree(pbuf_vid_pins);
    }
    if (pbuf_vid_voltages)
    {
        vfree(pbuf_vid_voltages);
    }
    return -1;
}

int core_voltage_available(const char *name, unsigned int **voltages, unsigned int *num)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    if (voltages && num)
    {
        *num      = 1 << ctrl->gpio_vid_width;
        *voltages = ctrl->gpio_vid_voltages;
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(core_voltage_available);

int core_voltage_pin(const char *name, unsigned int **pins, unsigned int *num)
{
    struct voltage_ctrl *ctrl;

    ctrl = get_voltage_ctrl(name);
    if (!ctrl)
    {
        VOLCTRL_DBG("[Core Voltage] %s: volage ctrl is null (%s)\n", __FUNCTION__, name);
        return -EINVAL;
    }

    if (pins && num)
    {
        *num  = ctrl->gpio_vid_width;
        *pins = ctrl->gpio_vid_pins;
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(core_voltage_pin);

static ssize_t show_scaling_voltage(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);
    str += scnprintf(str, end - str, "%d\n", ctrl->enable_scaling_voltage);
    CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);

    return (str - buf);
}

static ssize_t store_scaling_voltage(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  enable;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &enable) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    CamOsSpinLockIrqSave(&ctrl->core_voltage_spinlock);
    if (enable)
    {
        ctrl->enable_scaling_voltage = 1;
        VOLCTRL_DBG("[Core Voltage] %s: scaling ON\n", __FUNCTION__);
    }
    else
    {
        ctrl->enable_scaling_voltage = 0;
        VOLCTRL_DBG("[Core Voltage] %s: scaling OFF\n", __FUNCTION__);
    }
    CamOsSpinUnlockIrqRestore(&ctrl->core_voltage_spinlock);
#ifdef CONFIG_SS_DUALOS
    if (!dualos_rtos_exist())
#endif
    {
        sync_core_voltage(ctrl);
    }

    return count;
}
DEVICE_ATTR(scaling_voltage, 0644, show_scaling_voltage, store_scaling_voltage);

static ssize_t show_voltage_available(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str         = buf;
    char *               end         = buf + PAGE_SIZE;
    unsigned int         gpio_bitmap = 0;
    unsigned int *       pins        = NULL;
    unsigned int         pin_num     = 0;
    unsigned int *       voltages    = NULL;
    unsigned int         voltage_num = 0;
    u8                   i, j;
    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    str += scnprintf(str, end - str, "\tVcore(mV)");

    if (core_voltage_pin(ctrl->name, &pins, &pin_num) == 0)
    {
        for (i = 0; i < pin_num; i++)
        {
            str += scnprintf(str, end - str, "\tvid_%d(%d)", i, pins[i]);
        }
    }
    else
    {
        goto out;
    }

    str += scnprintf(str, end - str, "\n");

    if (pin_num)
    {
        if (core_voltage_available(ctrl->name, &voltages, &voltage_num) == 0)
        {
            for (j = 0; j < voltage_num; j++, gpio_bitmap++)
            {
                str += scnprintf(str, end - str, "[%d]\t%u", j, voltages[j]);

                for (i = 0; i < pin_num; i++)
                {
                    str += scnprintf(str, end - str, "\t\t%u", (gpio_bitmap & 1 << i) ? 1 : 0);
                }
                str += scnprintf(str, end - str, "\n");
            }
        }
    }

out:
    return (str - buf);
}
DEVICE_ATTR(voltage_available, 0444, show_voltage_available, NULL);

static ssize_t show_voltage_current(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *               str = buf;
    char *               end = buf + PAGE_SIZE;
    int                  voltage_mV;
    unsigned int         i;
    unsigned int *       pins    = NULL;
    unsigned int         pin_num = 0;
    struct voltage_ctrl *ctrl;
#ifdef CONFIG_SS_DUALOS
    unsigned int query_voltage = 0;
#endif

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    core_voltage_pin(ctrl->name, &pins, &pin_num);

    if (!get_core_voltage(ctrl->name, &voltage_mV))
    {
        if (pin_num)
            str += scnprintf(str, end - str, "%d\n", voltage_mV);
        else
            str += scnprintf(str, end - str, "Unknown (not assigned GPIO)\n");
    }
    else
    {
        str += scnprintf(str, end - str, "Get core voltage fail\n");
    }

    for (i = 0; i < VOLTAGE_DEMANDER_MAX; i++)
    {
#ifdef CONFIG_SS_DUALOS
        if (dualos_rtos_exist())
        {
            query_voltage =
                CamInterOsSignal(INTEROS_SC_L2R_CORE_VOLTAGE_GET, (u32)controller_name_to_id(ctrl->name), 1, i);
            if (query_voltage)
            {
                str += scnprintf(str, end - str, "    %-32s%d\n", _gVoltageDemanderName[i], query_voltage);
            }
            else
                str += scnprintf(str, end - str, "    %-32s-\n", _gVoltageDemanderName[i]);
        }
        else
#endif
        {
            if (ctrl->voltage_demander_request_value[i])
                str += scnprintf(str, end - str, "    %-32s%d\n", _gVoltageDemanderName[i],
                                 ctrl->voltage_demander_request_value[i]);
            else
                str += scnprintf(str, end - str, "    %-32s-\n", _gVoltageDemanderName[i]);
        }
    }

    return (str - buf);
}

static ssize_t store_voltage_current(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32                  voltage = 0;
    struct voltage_ctrl *ctrl;

    if (sscanf(buf, "%d", &voltage) <= 0)
        return 0;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        return count;
    }

    if (ctrl->enable_scaling_voltage)
        set_core_voltage(ctrl->name, VOLTAGE_DEMANDER_USER, voltage);
    else
        printk("[Core Voltage] voltage scaling not enable\n");

    return count;
}
DEVICE_ATTR(voltage_current, 0644, show_voltage_current, store_voltage_current);

static ssize_t show_vid_gpio_map(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *        str     = buf;
    char *        end     = buf + PAGE_SIZE;
    unsigned int  i       = 0;
    unsigned int *pins    = NULL;
    unsigned int  pin_num = 0;

    struct voltage_ctrl *ctrl;

    ctrl = dev_get_drvdata(dev);
    if (!ctrl)
    {
        VOLCTRL_ERR("[Core Voltage] %s: volage ctrl is null\n", __FUNCTION__);
        str += scnprintf(str, end - str, "%s\n", "get ctrl error");
        return (str - buf);
    }

    if (core_voltage_pin(ctrl->name, &pins, &pin_num) == 0)
    {
        for (i = 0; i < pin_num; i++)
        {
            str += scnprintf(str, end - str, "vid_%d=%d ", i, pins[i]);
        }
    }

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
DEVICE_ATTR(vid_gpio_map, 0444, show_vid_gpio_map, NULL);

static int sstar_voltage_ctrl_probe(struct platform_device *pdev)
{
    int                  i;
    int                  ret = 0;
    struct voltage_ctrl *ctrl;

    ctrl = kzalloc(sizeof(struct voltage_ctrl), GFP_KERNEL);
    if (!ctrl)
    {
        ret = -ENOMEM;
        goto mem_err;
    }

    ctrl->pdev = pdev;
    ctrl->name = pdev->dev.of_node->name;

    ctrl->dev_core.kobj.name = ctrl->name;
    ctrl->dev_core.bus       = &voltage_subsys;

    CamOsSpinInit(&ctrl->core_voltage_spinlock);

    ret = core_voltage_get_gpio(ctrl);
    if (ret)
    {
        printk(KERN_ERR "Failed to get gpio for voltage control!! %d\n", ret);
        goto voltage_control_init_err;
    }

    VOLCTRL_DBG("[Core Voltage] %s: register sub system\n", __FUNCTION__);

    if (!voltage_subsys_registered)
    {
        ret = subsys_system_register(&voltage_subsys, NULL);
        if (ret)
        {
            printk(KERN_ERR "Failed to register voltage sub system!! %d\n", ret);
            goto voltage_control_init_err;
        }
        voltage_subsys_registered = 1;
    }

    ret = device_register(&ctrl->dev_core);
    if (ret)
    {
        printk(KERN_ERR "Failed to register voltage core device!! %d\n", ret);
        goto voltage_control_init_err;
    }

    platform_set_drvdata(pdev, ctrl);
    dev_set_drvdata(&ctrl->dev_core, ctrl);

    list_add_tail(&ctrl->list, &voltage_ctrl_list);

    device_create_file(&ctrl->dev_core, &dev_attr_scaling_voltage);
    device_create_file(&ctrl->dev_core, &dev_attr_voltage_available);
    device_create_file(&ctrl->dev_core, &dev_attr_voltage_current);
    device_create_file(&ctrl->dev_core, &dev_attr_vid_gpio_map);

    // Enable core voltage scaling
    VOLCTRL_DBG("[Core Voltage] %s: turn-on core voltage scaling\n", __FUNCTION__);
    ctrl->enable_scaling_voltage = 1;
#ifdef CONFIG_SS_DUALOS
    if (!dualos_rtos_exist())
#endif
    {
        // Initialize voltage request for specific IP by chip
        voltage_request_chip(ctrl->name);
        sync_core_voltage(ctrl);
    }

    return 0;

voltage_control_init_err:
    for (i = 0; i < ctrl->gpio_vid_width; i++)
    {
        gpio_free(ctrl->gpio_vid_pins[i]);
    }

    ctrl->gpio_vid_width = 0;
    if (ctrl->gpio_vid_pins)
    {
        vfree(ctrl->gpio_vid_pins);
        ctrl->gpio_vid_pins = NULL;
    }
    if (ctrl->gpio_vid_voltages)
    {
        vfree(ctrl->gpio_vid_voltages);
        ctrl->gpio_vid_voltages = NULL;
    }
mem_err:
    kfree(ctrl);
    return ret;
}

static const struct of_device_id sstar_voltage_ctrl_of_match_table[] = {{.compatible = "sstar,voltage-ctrl"}, {}};
MODULE_DEVICE_TABLE(of, sstar_voltage_ctrl_of_match_table);

static struct platform_driver sstar_voltage_ctrl_driver = {
    .probe = sstar_voltage_ctrl_probe,
    .driver =
        {
            .name           = "sstar,voltage-ctrl",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_voltage_ctrl_of_match_table,
        },
};

builtin_platform_driver(sstar_voltage_ctrl_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar Voltage Control Driver");
MODULE_LICENSE("GPL v2");
