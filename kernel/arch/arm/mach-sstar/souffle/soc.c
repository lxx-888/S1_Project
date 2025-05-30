/*
* soc.c- Sigmastar
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
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timecounter.h>
#include <clocksource/arm_arch_timer.h>
#include <linux/memblock.h>

#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <linux/memblock.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "ss_private.h"
#include "ms_msys.h"
#include "memory.h"

/* IO tables */
static struct map_desc sstar_io_desc[] __initdata =
{
    /* Define Registers' physcial and virtual addresses */
        {IO_VIRT,   __phys_to_pfn(IO_PHYS),     IO_SIZE,        MT_DEVICE},
        {SPI_VIRT,  __phys_to_pfn(SPI_PHYS),    SPI_SIZE,       MT_DEVICE},
        {GIC_VIRT,  __phys_to_pfn(GIC_PHYS),    GIC_SIZE,       MT_DEVICE},
        {IMI_VIRT,  __phys_to_pfn(IMI_PHYS),    IMI_SIZE,       MT_DEVICE},
};

static const char *sstar_dt_compat[] __initconst = {
    "sstar,souffle",
    NULL,
};

static void __init sstar_map_io(void)
{
    iotable_init(sstar_io_desc, ARRAY_SIZE(sstar_io_desc));
}

extern struct ss_chip* sstar_chip_get(void);
extern void __init sstar_chip_init_default(void);

static int mcm_rw(int index, int ratio, int write);

static void sstar_uart_disable_line(int line)
{
    //for debug, do not change
    if (line == 0) {
        // UART0_Mode -> X
        //CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4 | BIT5);
        //CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_09, BIT11);
    } else if (line == 1) {
        // UART1_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8 | BIT9);
    } else if (line == 2) {
        // FUART_Mode -> X
        CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0 | BIT1);
    }
}

static void sstar_uart_enable_line(int line)
{
    //for debug, do not change
    if (line == 0) {
        // UART0_Mode -> PAD_UART0_TX/RX
        //SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT4);
    } else if (line == 1) {
        // UART1_Mode -> PAD_UART1_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT8);
    } else if (line == 2) {
        // FUART_Mode -> PAD_FUART_TX/RX
        SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_03, BIT0);
    }
}

static int sstar_get_device_id(void)
{
    return (int)(INREG16(BASE_REG_PMTOP_PA) & 0x00FF);;
}

static int sstar_get_revision(void)
{
    u16 tmp = 0;
    tmp = INREG16((unsigned int)(BASE_REG_PMTOP_PA + REG_ID_67));
    tmp=((tmp >> 8) & 0x00FF);

    return (tmp+1);
}

static void sstar_chip_flush_miu_pipe(void)
{
    // M6P don't need to flush the miu pipe
}

static void sstar_chip_flush_STB_and_miu_pipe(void)
{
    dsb();
    sstar_chip_flush_miu_pipe();
}

static u64 sstar_phys_to_MIU(u64 x)
{
    return ((x) - MIU0_BASE);
}

static u64 sstar_MIU_to_phys(u64 x)
{
    return ((x) + MIU0_BASE);
}


struct soc_device_attribute sstar_soc_dev_attr;

extern const struct of_device_id of_default_bus_match_table[];

static int sstar_get_storage_type(void)
{
//check DIDKEY bank, offset 0x70
#define STORAGE_SPI_NONE                  0x00
#define STORAGE_SPI_NAND_SKIP_SD          BIT2
#define STORAGE_SPI_NAND                  BIT4
#define STORAGE_SPI_NOR                   BIT5
#define STORAGE_SPI_NOR_SKIP_SD           BIT1
#define STORAGE_USB                       BIT12
#define STORAGE_EMMC                      BIT3  //Boot from eMMC
#define STORAGE_EMMC_4                    STORAGE_EMMC
#define STORAGE_EMMC_8                    STORAGE_EMMC|BIT7  //emmc_8bit_mode (0: 4-bit mode, 1: 8-bit mode)
#define STORAGE_EMMC_4_SD1                STORAGE_EMMC|BIT10 //emmc 4-bit source (0: SD0, 1: SD1)
#define STORAGE_BOOT_TYPES                (BIT12|BIT10|BIT7|BIT5|BIT4|BIT3|BIT2|BIT1)

    u16 boot_type = (INREG16(BASE_REG_DIDKEY_PA + 0x70*4) & STORAGE_BOOT_TYPES);

    if (boot_type == STORAGE_SPI_NAND || boot_type == STORAGE_SPI_NAND_SKIP_SD) {
        return (int)MS_STORAGE_SPINAND_ECC;
    } else if ((boot_type & STORAGE_EMMC) == STORAGE_EMMC) {
        return (int)MS_STORAGE_EMMC;
    } else if (boot_type == STORAGE_SPI_NOR || boot_type == STORAGE_SPI_NOR_SKIP_SD) {
        return (int)MS_STORAGE_NOR;
    } else {
        return (int)MS_STORAGE_UNKNOWN;
    }
}

static int sstar_get_package_type(void)
{
    printk(KERN_ERR "!!!!! Machine name [%s] \n", sstar_soc_dev_attr.machine);
    return 0;
}
static char sstar_platform_name[]=CONFIG_SSTAR_SHORT_NAME;

char* sstar_get_platform_name(void)
{
    return sstar_platform_name;
}

static unsigned long long sstar_chip_get_riu_phys(void)
{
    return IO_PHYS;
}

static int sstar_chip_get_riu_size(void)
{
    return IO_SIZE;
}


static int sstar_ir_enable(int param)
{
    printk(KERN_ERR "NOT YET IMPLEMENTED!![%s]",__FUNCTION__);
    return 0;
}


static int sstar_usb_vbus_control(int param)
{

    int ret;
    //int package = mstar_get_package_type();
    int power_en_gpio=-1;

    struct device_node *np;
    int pin_data;
    int port_num = param >> 16;
    int vbus_enable = param & 0xFF;
    if ((vbus_enable<0 || vbus_enable>1) && (port_num<0 || port_num>1)) {
        printk(KERN_ERR "[%s] param invalid:%d %d\n", __FUNCTION__, port_num, vbus_enable);
        return -EINVAL;
    }

    if (power_en_gpio<0) {
        if (0 == port_num) {
            np = of_find_node_by_path("/soc/Sstar-ehci-1");
        } else {
            np = of_find_node_by_path("/soc/Sstar-ehci-2");
        }

        if (!of_property_read_u32(np, "power-enable-pad", &pin_data)) {
            printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
            power_en_gpio = (unsigned char)pin_data;
        } else {
            printk(KERN_ERR "Can't get power-enable-pad from DTS, set default GPIO(%d)\n", pin_data);
            power_en_gpio = PAD_PM_GPIO2;
        }
        ret = gpio_request(power_en_gpio, "USB0-power-enable");
        if (ret < 0) {
            printk(KERN_INFO "Failed to request USB0-power-enable GPIO(%d)\n", power_en_gpio);
            power_en_gpio =-1;
            return ret;
        }
    }
    //disable vbus
    if (0 == vbus_enable) {
        gpio_direction_output(power_en_gpio, 0);
        printk(KERN_INFO "[%s] Disable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
    } else if(1 == vbus_enable) {
        gpio_direction_output(power_en_gpio, 1);
        printk(KERN_INFO "[%s] Enable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
    }
    return 0;
}
static u64 us_ticks_cycle_offset=0;
static u64 us_ticks_factor=1;


static u64 sstar_chip_get_us_ticks(void)
{
	u64 cycles=arch_timer_read_counter();
	u64 usticks=div64_u64(cycles,us_ticks_factor);
	return usticks;
}

void sstar_reset_us_ticks_cycle_offset(void)
{
	us_ticks_cycle_offset=arch_timer_read_counter();
}

static int sstar_chip_function_set(int function_id, int param)
{
    int res=-1;

    printk("CHIP_FUNCTION SET. ID=%d, param=%d\n",function_id,param);
    switch (function_id) {
            case CHIP_FUNC_UART_ENABLE_LINE:
                sstar_uart_enable_line(param);
                break;
            case CHIP_FUNC_UART_DISABLE_LINE:
                sstar_uart_disable_line(param);
                break;
            case CHIP_FUNC_IR_ENABLE:
                sstar_ir_enable(param);
                break;
            case CHIP_FUNC_USB_VBUS_CONTROL:
                sstar_usb_vbus_control(param);
                break;
            case CHIP_FUNC_MCM_DISABLE_ID:
                mcm_rw(param, 0, 1);
                break;
            case CHIP_FUNC_MCM_ENABLE_ID:
                mcm_rw(param, 15, 1);
                break;
        default:
            printk(KERN_ERR "Unsupport CHIP_FUNCTION!! ID=%d\n",function_id);

    }

    return res;
}


static void __init sstar_init_early(void)
{
    struct ss_chip *chip=NULL;
    sstar_chip_init_default();
    chip=sstar_chip_get();

    //enable axi exclusive access
    *(volatile unsigned short *)(0xFD204414) = 0x10;

    chip->chip_flush_miu_pipe=sstar_chip_flush_STB_and_miu_pipe;//dsb
    chip->chip_flush_miu_pipe_nodsb=sstar_chip_flush_miu_pipe;//nodsbchip->phys_to_miu=mstar_phys_to_MIU;
    chip->phys_to_miu=sstar_phys_to_MIU;
    chip->miu_to_phys=sstar_MIU_to_phys;
    chip->chip_get_device_id=sstar_get_device_id;
    chip->chip_get_revision=sstar_get_revision;
    chip->chip_get_platform_name=sstar_get_platform_name;
    chip->chip_get_riu_phys=sstar_chip_get_riu_phys;
    chip->chip_get_riu_size=sstar_chip_get_riu_size;

    chip->chip_function_set=sstar_chip_function_set;
    chip->chip_get_storage_type=sstar_get_storage_type;
    chip->chip_get_package_type=sstar_get_package_type;
    chip->chip_get_us_ticks=sstar_chip_get_us_ticks;

}

#if defined(CONFIG_MIU0_DMA_PFN_OFFSET)
static int sstar_platform_notifier(struct notifier_block *nb,
                      unsigned long event, void *data)
{
    struct device *dev = data;

    if (event != BUS_NOTIFY_ADD_DEVICE)
        return NOTIFY_DONE;

    if (!dev)
        return NOTIFY_BAD;

    if (!dev->of_node) {
        int ret = dma_direct_set_offset(dev, SOC_HIGH_PHYS_START,
                          SOC_LOW_PHYS_START,
                          SOC_HIGH_PHYS_SIZE);
        dev_err(dev, "set dma_offset%08llx%s\n",
              SOC_HIGH_PHYS_START - SOC_LOW_PHYS_START,
              ret ? " failed" : "");
    }
    return NOTIFY_OK;
}

static struct notifier_block platform_nb = {
    .notifier_call = sstar_platform_notifier,
};
#endif

extern char* LX_VERSION;
struct device *parent = NULL;

static void __init sstar_init_machine(void)
{
    struct soc_device *soc_dev;

    pr_info("\n\nVersion : %s\n\n",LX_VERSION);

    sstar_reset_us_ticks_cycle_offset();
    us_ticks_factor=div64_u64(arch_timer_get_rate(),1000000);

    sstar_soc_dev_attr.family = kasprintf(GFP_KERNEL, sstar_platform_name);
    sstar_soc_dev_attr.revision = kasprintf(GFP_KERNEL, "%d", sstar_get_revision());
    sstar_soc_dev_attr.soc_id = kasprintf(GFP_KERNEL, "%u", sstar_get_device_id());
    sstar_soc_dev_attr.api_version = kasprintf(GFP_KERNEL, sstar_chip_get()->chip_get_API_version());
    sstar_soc_dev_attr.machine = kasprintf(GFP_KERNEL, of_flat_dt_get_machine_name());

    soc_dev = soc_device_register(&sstar_soc_dev_attr);
    if (IS_ERR(soc_dev)) {
        kfree((void *)sstar_soc_dev_attr.family);
        kfree((void *)sstar_soc_dev_attr.revision);
        kfree((void *)sstar_soc_dev_attr.soc_id);
        kfree((void *)sstar_soc_dev_attr.machine);
        goto out;
    }

    parent = soc_device_to_device(soc_dev);

    /*
     * Finished with the static registrations now; fill in the missing
     * devices
     */
out:
    of_platform_populate(NULL, of_default_bus_match_table, NULL, parent);

    //write log_buf address to mailbox
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x08), (int)log_buf_addr_get() & 0xFFFF);
    OUTREG16(BASE_REG_MAILBOX_PA+BK_REG(0x09), ((int)log_buf_addr_get() >> 16 )& 0xFFFF);

#if defined(CONFIG_MIU0_DMA_PFN_OFFSET)
    if (PHYS_OFFSET >= SOC_HIGH_PHYS_START) {
        soc_dma_pfn_offset = PFN_DOWN(SOC_HIGH_PHYS_START -
                                      SOC_LOW_PHYS_START);
        bus_register_notifier(&platform_bus_type, &platform_nb);
    }
#endif

    // disable cpu l3 bridge monitor mantisbt:1266
    SETREG16(BASE_REG_L3_BRIDGE_PA+BK_REG(0x71), BIT7);
}

struct mcm_client {
    char* name;
    short index;
    unsigned int reg;
    short slow_down_ratio;
};


static struct mcm_client mcm_clients[] = {
    {"MCU51",       MCM_ID_MCU51,       OFFSET_MCM_ID_MCU51,      0},
    {"URDMA",       MCM_ID_URDMA,       OFFSET_MCM_ID_URDMA,      0},
    {"BDMA",        MCM_ID_BDMA,        OFFSET_MCM_ID_BDMA,       0},
    {"MFE",         MCM_ID_MFE,         OFFSET_MCM_ID_MFE,        0},
    {"JPE",         MCM_ID_JPE,         OFFSET_MCM_ID_JPE,        0},
    {"BACH",        MCM_ID_BACH,        OFFSET_MCM_ID_BACH,       0},
    {"FILE",        MCM_ID_FILE,        OFFSET_MCM_ID_FILE,       0},
    {"UHC0",        MCM_ID_UHC0,        OFFSET_MCM_ID_UHC0,       0},
    {"EMAC",        MCM_ID_EMAC,        OFFSET_MCM_ID_EMAC,       0},
    {"CMDQ",        MCM_ID_CMDQ,        OFFSET_MCM_ID_CMDQ,       0},
    {"ISP_DNR",     MCM_ID_ISP_DNR,     OFFSET_MCM_ID_ISP_DNR,    0},
    {"ISP_DMA",     MCM_ID_ISP_DMA,     OFFSET_MCM_ID_ISP_DMA,    0},
    {"GOP0",        MCM_ID_GOP0,        OFFSET_MCM_ID_GOP0,       0},
    {"SC_DNR",      MCM_ID_SC_DNR,      OFFSET_MCM_ID_SC_DNR,     0},
    {"SC_DNR_SAD",  MCM_ID_SC_DNR_SAD,  OFFSET_MCM_ID_SC_DNR_SAD, 0},
    {"SC_CROP",     MCM_ID_SC_CROP,     OFFSET_MCM_ID_SC_CROP,    0},
    {"SC1_FRM",     MCM_ID_SC1_FRM,     OFFSET_MCM_ID_SC1_FRM,    0},
    {"SC1_SNP",     MCM_ID_SC1_SNP,     OFFSET_MCM_ID_SC1_SNP,    0},
    {"SC1_DBG",     MCM_ID_SC1_DBG,     OFFSET_MCM_ID_SC1_DBG,    0},
    {"SC2_FRM",     MCM_ID_SC2_FRM,     OFFSET_MCM_ID_SC2_FRM,    0},
    {"SC3_FRM",     MCM_ID_SC3_FRM,     OFFSET_MCM_ID_SC3_FRM,    0},
    {"FCIE",        MCM_ID_FCIE,        OFFSET_MCM_ID_FCIE,       0},
    {"SDIO",        MCM_ID_SDIO,        OFFSET_MCM_ID_SDIO,       0},
    {"SC1_SNPI",    MCM_ID_SC1_SNPI,    OFFSET_MCM_ID_SC1_SNPI,   0},
    {"SC2_SNPI",    MCM_ID_SC2_SNPI,    OFFSET_MCM_ID_SC2_SNPI,   0},
    {"CMDQ1",       MCM_ID_CMDQ1,       OFFSET_MCM_ID_CMDQ1,      0},
    {"CMDQ2",       MCM_ID_CMDQ2,       OFFSET_MCM_ID_CMDQ2,      0},
    {"GOP1",        MCM_ID_GOP1,        OFFSET_MCM_ID_GOP1,       0},
    {"GOP2",        MCM_ID_GOP2,        OFFSET_MCM_ID_GOP2,       0},
    {"UHC1",        MCM_ID_UHC1,        OFFSET_MCM_ID_UHC1,       0},
    {"IVE",         MCM_ID_IVE,         OFFSET_MCM_ID_IVE,        0},
    {"VHE",         MCM_ID_VHE,         OFFSET_MCM_ID_VHE,        0},
    {"*ALL_CLIENTS",MCM_ID_ALL,         0,                        0}  //use carefully
};

struct device mcm_dev;

static struct bus_type mcm_subsys = {
    .name = "mcm",
    .dev_name = "mcm",
};

static int mcm_rw(int index, int ratio, int write)
{
    int i, addr;

    if (index == MCM_ID_ALL && write) {
        for(i=0; i<(sizeof(mcm_clients)/sizeof(mcm_clients[0]))-1;i++)
            mcm_rw(i, ratio, write);

        return 0;
    }

    if ((index >= MCM_ID_START) &&(index < MCM_ID_END)) {
        addr = mcm_clients[index].reg;
    } else {
        printk(KERN_ERR "mcm_clients[%d] not exists\n", index);
        return -1;
    }

    if (write)
        OUTREG8(addr, (ratio << 4));
    else
        mcm_clients[index].slow_down_ratio = (INREG8(addr) >> 4);

    return 0;
}


static ssize_t mcm_slow_ratio_store(struct device *dev,  struct device_attribute *attr, const char *buf, size_t n)
{
    int index, ratio;
    sscanf(buf, "%d %d", &index, &ratio);

    if (0 > ratio || ratio > 15) {
        printk(KERN_ERR "MCM slow down ratio should be 0~15\n");
        return -1;
    }

    return mcm_rw(index, ratio, 1)?-1:n;
}

static ssize_t mcm_slow_ratio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i=0;

    for (i=0; i<(sizeof(mcm_clients)/sizeof(mcm_clients[0]))-1; i++) {
        mcm_rw(i, 0, 0);
        str += scnprintf(str, end - str, "[%d] %s: %d\n", mcm_clients[i].index, mcm_clients[i].name, mcm_clients[i].slow_down_ratio);
    }
    str += scnprintf(str, end - str, "[%d] %s\n", mcm_clients[i].index, mcm_clients[i].name);

    return (str - buf);
}

DEVICE_ATTR(mcm_slow_ratio, 0644, mcm_slow_ratio_show, mcm_slow_ratio_store);

static void __init sstar_create_MCM_node(void)
{
    int ret;

    mcm_dev.kobj.name="mcm";
    mcm_dev.bus=&mcm_subsys;

    ret = subsys_system_register(&mcm_subsys, NULL);
    if (ret) {
        printk(KERN_ERR "Failed to register MCM sub system!! %d\n",ret);
        return;
    }

    ret = device_register(&mcm_dev);
    if (ret) {
        printk(KERN_ERR "Failed to register MCM device!! %d\n",ret);
        return;
    }

    device_create_file(&mcm_dev, &dev_attr_mcm_slow_ratio);
}

extern void sstar_create_MIU_node(void);
extern int sstar_pm_init(void);
extern void init_proc_zen(void);
static inline void __init sstar_init_late(void)
{
#ifdef CONFIG_PM_SLEEP
    sstar_pm_init();
#endif
    sstar_create_MCM_node();
}

static void global_reset(enum reboot_mode mode, const char * cmd)
{
    msys_set_rebootType(cmd);

    while (1) {
        OUTREG8(0x1f001cb8, 0x79);
        mdelay(5);
    }
}

static void __init sstar_pm_reserve(void)
{
    // reserve one page in the beginning of dram for store str info and timestamp records
    memblock_reserve(MIU0_BASE, PAGE_SIZE);
}

static long long __init sstar_pv_fixup(void)
{
    long long offset;
    phys_addr_t mem_start, mem_end;
    struct memblock_region *reserved_mem_region;

    mem_start = memblock_start_of_DRAM();
    mem_end = memblock_end_of_DRAM();

    /* nothing to do if we are running out of the <32-bit space */
    if (mem_start >= SOC_LOW_PHYS_START && mem_end   <= SOC_LOW_PHYS_END)
        return 0;

    if (mem_start < SOC_HIGH_PHYS_START || mem_end   > SOC_HIGH_PHYS_END) {
        pr_crit("Invalid address space for memory (%08llx-%08llx)\n",
                (u64)mem_start, (u64)mem_end);
        return 0;
    }

    offset = SOC_HIGH_PHYS_START - SOC_LOW_PHYS_START;

    /* Populate the arch idmap hook */
    arch_phys_to_idmap_offset = -offset;

    /* Reserve corresponding high physical address which has been reserve at low physical address */
    for_each_reserved_mem_region(reserved_mem_region) {
        if ((reserved_mem_region->base >= SOC_LOW_PHYS_START) && \
                (reserved_mem_region->base <= SOC_LOW_PHYS_END)) {
            memblock_reserve(idmap_to_phys(reserved_mem_region->base), reserved_mem_region->size);
        }
    }

    return offset;
}

#ifdef CONFIG_SMP
extern struct smp_operations souffle_smp_ops;
#endif

DT_MACHINE_START(SS_DT, "SStar Soc (Flattened Device Tree)")
    .dt_compat    = sstar_dt_compat,
    .map_io = sstar_map_io,
    .init_machine = sstar_init_machine,
    .init_early = sstar_init_early,
//    .init_time =  ms_init_timer,
//    .init_irq = mstar_init_irqchip,
    .init_late = sstar_init_late,
    .restart = global_reset,
    .reserve = sstar_pm_reserve,
    .pv_fixup = sstar_pv_fixup,
    #ifdef CONFIG_SMP
    .smp    = smp_ops(souffle_smp_ops),
    #endif
MACHINE_END
