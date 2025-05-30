/*
 * cam_sysfs.c- Sigmastar
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

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/gpio.h>
#include <linux/mm.h>
#include <linux/workqueue.h>
#include <linux/kallsyms.h>
#include <linux/vmalloc.h>
#include <cam_sysfs.h>
#include <cam_os_wrapper.h>
#include <linux/suspend.h>

/**
 * of_address_to_resource - Translate device tree address and return as resource
 *
 * Note that if your address is a PIO address, the conversion will fail if
 * the physical address can't be internally converted to an IO token with
 * pci_address_to_pio(), that is because it's either called to early or it
 * can't be matched to any host bridge IO space
 */
int CamOfAddressToResource(struct device_node *dev, int index, struct resource *r)
{
    return of_address_to_resource(dev, index, r);
}

int CamOfIrqToResource(struct device_node *dev, int index, struct resource *r)
{
    return of_irq_to_resource(dev, index, r);
}

int CamOfPropertyReadU32Index(const struct device_node *np, const char *propname, u32 index, u32 *out_value)
{
    return of_property_read_u32_index(np, propname, index, out_value);
}

int CamOfPropertyReadVariableU32Array(const struct device_node *np, const char *propname, u32 *out_values,
                                      size_t sz_min, size_t sz_max)
{
    return of_property_read_variable_u32_array(np, propname, out_values, sz_min, sz_max);
}
/**
 * platform_get_resource - get a resource for a device
 * @dev: platform device
 * @type: resource type
 * @num: resource index
 */
struct resource *CamPlatformGetResource(struct platform_device *dev, unsigned int type, unsigned int num)
{
    return platform_get_resource(dev, type, num);
}

int CamPlatformDriverRegister(struct platform_driver *drv)
{
    return platform_driver_register(drv);
}

/**
 * platform_driver_unregister - unregister a driver for platform-level devices
 * @drv: platform driver structure
 */
void CamPlatformDriverUnregister(struct platform_driver *drv)
{
    platform_driver_unregister(drv);
}

struct platform_device *CamPlatformDeviceAlloc(const char *name, int id)
{
    return platform_device_alloc(name, id);
}

int CamPlatformDeviceAdd(struct platform_device *pdev)
{
    return platform_device_add(pdev);
}

void CamPlatformDevicePut(struct platform_device *pdev)
{
    platform_device_put(pdev);
}

int CamPlatformDeviceRegister(struct platform_device *pdev)
{
    return platform_device_register(pdev);
}

void CamPlatformDeviceUnregister(struct platform_device *pdev)
{
    platform_device_unregister(pdev);
}

/**
 * platform_get_irq_byname - get an IRQ for a device by name
 * @dev: platform device
 * @name: IRQ name
 */
int CamPlatformGetIrqByname(struct platform_device *dev, const char *name)
{
    return platform_get_irq_byname(dev, name);
}

int __must_check CamSysfsCreateFile(struct kobject *kobj, const struct attribute *attr)
{
    return sysfs_create_file_ns(kobj, attr, NULL);
}

int CamSysfsCreateFiles(struct kobject *kobj, const struct attribute **ptr)
{
    return sysfs_create_files(kobj, ptr);
}

void CamSysfsRemoveFile(struct kobject *kobj, const struct attribute *attr, const void *ns)
{
    sysfs_remove_file_ns(kobj, attr, NULL);
}

void CamSysfsRemoveFiles(struct kobject *kobj, const struct attribute **attr)
{
    sysfs_remove_files(kobj, attr);
}

int CamDeviceRegister(struct device *dev)
{
    return device_register(dev);
}
void CamDeviceUnregister(struct device *dev)
{
    device_unregister(dev);
}

int CamDevicePropertyReadString(struct device *dev, const char *propname, const char **val)
{
    return device_property_read_string(dev, propname, val);
}

struct kobject *CamKobjectCreateAndAdd(const char *name, struct kobject *parent)
{
    return kobject_create_and_add(name, parent);
}

int CamIoremapPageRange(unsigned long addr, unsigned long end, phys_addr_t phys_addr, pgprot_t prot)
{
    return ioremap_page_range(addr, end, phys_addr, prot);
}

void CamDevmKfree(struct device *dev, void *p)
{
    devm_kfree(dev, p);
}

void *CamDevmKmalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return devm_kmalloc(dev, size, gfp);
}

int CamOfPropertyReadU32Array(const struct device_node *np, const char *propname, u32 *out_values, size_t sz)
{
    return of_property_read_u32_array(np, propname, out_values, sz);
}

int CamGpioRequest(unsigned gpio, const char *label)
{
    return gpio_request(gpio, label);
}

unsigned int CamIrqOfParseAndMap(struct device_node *dev, int index)
{
    return irq_of_parse_and_map(dev, index);
}

int CamofPropertyReadU32(const struct device_node *np, const char *propname, u32 *out_value)
{
    return of_property_read_u32(np, propname, out_value);
}

struct workqueue_struct *CamCreatesiglethreadWorkqueue(const char *fmt)
{
    return create_singlethread_workqueue(fmt);
}

int CamGetUserPagesFast(unsigned long start, int nr_pages, int write, struct page **pages)
{
    return get_user_pages_fast(start, nr_pages, write, pages);
}

bool CamFlushWork(struct work_struct *work)
{
    return flush_work(work);
}

int CamSprintSymbol(char *buffer, unsigned long address, bool offset)
{
    if (offset)
        return sprint_symbol(buffer, address);
    else
        return sprint_symbol_no_offset(buffer, address);
}

void CamVmUnmapAliases(void)
{
    vm_unmap_aliases();
}

int CamRegisterPmNotifier(struct notifier_block *nb)
{
    return register_pm_notifier(nb);
}

int CamUnRegisterPmNotifier(struct notifier_block *nb)
{
    return unregister_pm_notifier(nb);
}

int CamTryToFreeze(void)
{
    return try_to_freeze();
}

EXPORT_SYMBOL(CamCreatesiglethreadWorkqueue);
EXPORT_SYMBOL(CamofPropertyReadU32);
EXPORT_SYMBOL(CamIrqOfParseAndMap);
EXPORT_SYMBOL(CamGpioRequest);
EXPORT_SYMBOL(CamDevmKmalloc);
EXPORT_SYMBOL(CamDevmKfree);
EXPORT_SYMBOL(CamIoremapPageRange);
EXPORT_SYMBOL(CamKobjectCreateAndAdd);
EXPORT_SYMBOL(CamDevicePropertyReadString);
EXPORT_SYMBOL(CamDeviceUnregister);
EXPORT_SYMBOL(CamSysfsRemoveFiles);
EXPORT_SYMBOL(CamSysfsRemoveFile);
EXPORT_SYMBOL(CamSysfsCreateFiles);
EXPORT_SYMBOL(CamSysfsCreateFile);
EXPORT_SYMBOL(CamPlatformDriverRegister);
EXPORT_SYMBOL(CamPlatformGetIrqByname);
EXPORT_SYMBOL(CamPlatformDriverUnregister);
EXPORT_SYMBOL(CamPlatformDeviceAlloc);
EXPORT_SYMBOL(CamPlatformDeviceAdd);
EXPORT_SYMBOL(CamPlatformDevicePut);
EXPORT_SYMBOL(CamPlatformDeviceRegister);
EXPORT_SYMBOL(CamPlatformDeviceUnregister);
EXPORT_SYMBOL(CamPlatformGetResource);
EXPORT_SYMBOL(CamOfPropertyReadVariableU32Array);
EXPORT_SYMBOL(CamOfPropertyReadU32Index);
EXPORT_SYMBOL(CamOfIrqToResource);
EXPORT_SYMBOL(CamOfAddressToResource);
EXPORT_SYMBOL(CamOfPropertyReadU32Array);
EXPORT_SYMBOL(CamGetUserPagesFast);
EXPORT_SYMBOL(CamFlushWork);
EXPORT_SYMBOL(CamSprintSymbol);
EXPORT_SYMBOL(CamVmUnmapAliases);
EXPORT_SYMBOL(CamRegisterPmNotifier);
EXPORT_SYMBOL(CamUnRegisterPmNotifier);
EXPORT_SYMBOL(CamTryToFreeze);
