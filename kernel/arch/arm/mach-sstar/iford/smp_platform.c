/*
* smp_platform.c- Sigmastar
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

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/cp15.h>
#include <asm/smp_plat.h>

#include "ms_platform.h"

extern int iford_platform_cpu_kill(unsigned int cpu);
extern void iford_platform_cpu_die(unsigned int cpu);
extern int iford_platform_cpu_disable(unsigned int cpu);

extern void iford_secondary_startup(void);
void iford_smp_init_cpus(void);
void iford_smp_prepare_cpus(unsigned int max_cpus);
//extern volatile int __cpuinitdata pen_release;
extern volatile int pen_release;
extern void Chip_Flush_CacheAll(void);

#ifdef CONFIG_PM_SLEEP
extern int suspend_status;
#endif

static inline void cpu_enter_lowpower(void)
{
    unsigned int v;

    printk("ms_hotplug.c  cpu_enter_lowpower: in\n");

    flush_cache_all();
    asm volatile(
    "   mcr p15, 0, %1, c7, c5, 0\n"
    "   mcr p15, 0, %1, c7, c10, 4\n"
    /*
     * Turn off coherency
     */
    "   mrc p15, 0, %0, c1, c0, 1\n"
    "   bic %0, %0, #0x20\n"
    "   mcr p15, 0, %0, c1, c0, 1\n"
    "   mrc p15, 0, %0, c1, c0, 0\n"
    "   bic %0, %0, %2\n"
    "   mcr p15, 0, %0, c1, c0, 0\n"
      : "=&r" (v)
      : "r" (0), "Ir" (CR_C)
      : "cc");

    printk("ms_hotplug.c  cpu_enter_lowpower: out\n");
}

static inline void cpu_leave_lowpower(void)
{
    unsigned int v;

    printk("ms_hotplug.c  cpu_leave_lowpower: in\n");

    asm volatile(   "mrc    p15, 0, %0, c1, c0, 0\n"
    "   orr %0, %0, %1\n"
    "   mcr p15, 0, %0, c1, c0, 0\n"
    "   mrc p15, 0, %0, c1, c0, 1\n"
    "   orr %0, %0, #0x20\n"
    "   mcr p15, 0, %0, c1, c0, 1\n"
      : "=&r" (v)
      : "Ir" (CR_C)
      : "cc");

    printk("ms_hotplug.c  cpu_leave_lowpower: out\n");
}

static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
       //printk("ms_hotplug.c  platform_do_lowpower: before %d cpu go into WFI, spurious =%d  \n", cpu, *spurious);

    /*
     * there is no power-control hardware on this platform, so all
     * we can do is put the core into WFI; this is safe as the calling
     * code will have already disabled interrupts
     */
    for (;;) {
        /*
         * here's the WFI
         */
        asm("wfi\n"
            :
            :
            : "memory", "cc");

          //printk("ms_hotplug.c  platform_do_lowpower: %d cpu  wake up, spurious =%d  \n", cpu, *spurious);

        if (pen_release == cpu_logical_map(cpu)) {
            /*
             * OK, proper wakeup, we're done
             */
            break;
        }

        //printk("ms_hotplug.c  platform_do_lowpower: %d cpu  wake up error(cpuID != pen_release), spurious =%d  \n", cpu, *spurious);

        /*
         * Getting here, means that we have come out of WFI without
         * having been woken up - this shouldn't happen
         *
         * Just note it happening - when we're woken, we can report
         * its occurrence.
         */
        (*spurious)++;
    }
}

int iford_platform_cpu_kill(unsigned int cpu)
{
    return 1;
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void iford_platform_cpu_die(unsigned int cpu)
{
    int spurious = 0;
//  printk(KERN_ERR "[%s]\n",__FUNCTION__);

    //printk("ms_hotplug.c  platform_cpu_die: in cpu = %d \n", cpu);

    Chip_Flush_CacheAll();

    /*
     * we're ready for shutdown now, so do it
     */
    //cpu_enter_lowpower();
    platform_do_lowpower(cpu, &spurious);

    /*
     * bring this CPU back into the world of cache
     * coherency, and then restore interrupts
     */
    //cpu_leave_lowpower();

    printk("ms_hotplug.c  platform_cpu_die: out cpu = %d \n", cpu);

    if (spurious)
        pr_warn("CPU%u: %u spurious wakeup calls\n", cpu, spurious);
}

int iford_platform_cpu_disable(unsigned int cpu)
{
    /*
     * we don't allow CPU 0 to be shutdown (it is still too special
     * e.g. clock tick interrupts)
     */

    printk("ms_hotplug.c  platform_cpu_disable: in cpu = %d \n", cpu);
    return cpu == 0 ? -EPERM : 0;
}


/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
//volatile int __cpuinitdata pen_release = -1;
volatile int  pen_release = -1;

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
//static void __cpuinit write_pen_release(int val)
static void write_pen_release(int val)
{
    pen_release = val;
    smp_wmb();
    __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
    outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static DEFINE_SPINLOCK(boot_lock);

void iford_secondary_init(unsigned int cpu)
{

    printk(KERN_DEBUG "[%s]\n",__FUNCTION__);
    /*
     * if any interrupts are already enabled for the primary
     * core (e.g. timer irq), then they will not have been enabled
     * for us: do so
     */
    //gic_secondary_init(0);

    /*
     * let the primary processor know we're out of the
     * pen, then head off into the C entry point
     */
    write_pen_release(-1);

    /*
     * Synchronise with the boot thread.
     */
    spin_lock(&boot_lock);
    spin_unlock(&boot_lock);
}

#define SECOND_START_ADDR_HI            0x1F20404C
#define SECOND_START_ADDR_LO            0x1F204050
#define SECOND_MAGIC_NUMBER_ADDR        0x1F204058

#define CORE_WAKEUP_CONFIG_ADDR    0x1F204054
#define CORE_WAKEUP_FLAG(core)     (0x1 << ((core) << 1))
#define CORE_NS_FLAG(core)         (0x2 << ((core) << 1))

int iford_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    unsigned long timeout;
#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
#define CORE_WAKEUP_TAG_LINUX(core) (1 << (4+core))
#else
    unsigned int  magic_number;
#endif
    printk(KERN_DEBUG "[%s]\n",__FUNCTION__);
    /*
     * Set synchronisation state between this boot processor
     * and the secondary one
     */
    spin_lock(&boot_lock);

    /*
     * This is really belt and braces; we hold unintended secondary
     * CPUs in the holding pen until we're ready for them.  However,
     * since we haven't sent them a soft interrupt, they shouldn't
     * be there.
     */
    write_pen_release(cpu_logical_map(cpu));

#ifdef CONFIG_PM_SLEEP
    if (suspend_status) {
        do{
            OUTREG16(SECOND_START_ADDR_LO,(virt_to_phys(iford_secondary_startup) & 0xFFFF));
        }while(INREG16(SECOND_START_ADDR_LO)!=(virt_to_phys(iford_secondary_startup) & 0xFFFF));

        do{
            OUTREG16(SECOND_START_ADDR_HI,(virt_to_phys(iford_secondary_startup)>>16));
        }while(INREG16(SECOND_START_ADDR_HI)!=(virt_to_phys(iford_secondary_startup)>>16));

        __cpuc_flush_kern_all();
    }
#endif

#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
    {
        u16 wakeup_tag = INREG16(SECOND_MAGIC_NUMBER_ADDR);

        wakeup_tag |= CORE_WAKEUP_TAG_LINUX(cpu);

        do {
            OUTREG16(SECOND_MAGIC_NUMBER_ADDR, wakeup_tag);
        } while(0);
    }
#else
#if 1//def CONFIG_SS_DUALOS
    magic_number = CORE_WAKEUP_FLAG(cpu)|CORE_NS_FLAG(cpu);
    do {
        SETREG16_BIT_OP(CORE_WAKEUP_CONFIG_ADDR, magic_number);
    }while((INREG16(CORE_WAKEUP_CONFIG_ADDR) & magic_number) != magic_number);
#else
    magic_number = 0xBABE;
    do{
        OUTREG16(SECOND_MAGIC_NUMBER_ADDR, magic_number);
    }while(INREG16(SECOND_MAGIC_NUMBER_ADDR)!=magic_number);
#endif
#endif

    /*
     * Send the secondary CPU a soft interrupt, thereby causing
     * the boot monitor to read the system wide flags register,
     * and branch to the address found there.
     */
    printk(KERN_DEBUG "[%s] ipi\n",__FUNCTION__);
    arch_send_wakeup_ipi_mask(cpumask_of(cpu));

    timeout = jiffies + (5 * HZ);
    while (time_before(jiffies, timeout)) {
        smp_rmb();
        if (pen_release == -1)
            break;

        udelay(10);
    }

    /*
     * now the secondary core is starting up let it run its
     * calibrations, then wait for it to finish
     */
    spin_unlock(&boot_lock);

    return pen_release != -1 ? -ENOSYS : 0;
}


#include <asm/smp_scu.h>
#include <linux/io.h>


#define SCU_CTRL        0x00

void iford_smp_prepare_cpus(unsigned int max_cpus)
{
    int i;
    unsigned long secondary_startup_addr;

    printk(KERN_DEBUG "[%s]\n",__FUNCTION__);
    /*
     * Initialise the present map, which describes the set of CPUs
     * actually populated at the present time.
     */
    for (i = 0; i < max_cpus; i++)
    {
        set_cpu_present(i, true);
    }

    if (arm_has_idmap_alias()) {
        secondary_startup_addr = phys_to_idmap(virt_to_phys(iford_secondary_startup));
    } else {
        secondary_startup_addr = virt_to_phys(iford_secondary_startup);
    }

    /*
     * Write the address of secondary startup into the
     * system-wide flags register. The boot monitor waits
     * until it receives a soft interrupt, and then the
     * secondary CPU branches to this address.
     */
    do{
        OUTREG16(SECOND_START_ADDR_LO,(secondary_startup_addr & 0xFFFF));
    }while(INREG16(SECOND_START_ADDR_LO)!=(secondary_startup_addr & 0xFFFF));

    do{
        OUTREG16(SECOND_START_ADDR_HI,(secondary_startup_addr>>16));
    }while(INREG16(SECOND_START_ADDR_HI)!=(secondary_startup_addr>>16));

    __cpuc_flush_kern_all();
}

void iford_smp_init_cpus(void)
{
    printk(KERN_DEBUG "[%s]\n",__FUNCTION__);
}

struct smp_operations iford_smp_ops  = {
    .smp_init_cpus      = iford_smp_init_cpus,
    .smp_prepare_cpus   = iford_smp_prepare_cpus,
    .smp_secondary_init = iford_secondary_init,
    .smp_boot_secondary = iford_boot_secondary,

#ifdef CONFIG_HOTPLUG_CPU
    .cpu_kill           = iford_platform_cpu_kill,
    .cpu_die            = iford_platform_cpu_die,
    .cpu_disable        = iford_platform_cpu_disable,
#endif
};
