/*
 * irq-main.c- Sigmastar
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
#include <linux/io.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/syscore_ops.h>
#include <asm/mach/irq.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip.h>
#include <linux/irqdesc.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>

#include "irqs.h"
#include "registers.h"

#include "ss_private.h"
#include "ms_platform.h"
#include "ms_types.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

/*         _ _ _ _ _ _ _ _ _ _                  */
/*        |                   |                 */
/*        |  PM_SLEEP_IRQ(32) |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*        |                   |                 */
/*        |    MS_FIQ (32)    |                 */
/*        |_ _ _ _ _ _ _ _ _ _| ms_fiq          */
/*        |                   |                 */
/*        |    MS_IRQ (64)    |                 */
/*        |_ _ _ _ _ _ _ _ _ _| ms_irq          */
/*        |                   |                 */
/*        |  ARM_INTERNAL(32) |                 */
/*        |_ _ _ _ _ _ _ _ _ _| gic_spi         */
/*        |                   |                 */
/*        |      PPI (16)     |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*        |                   |                 */
/*        |      SGI (16)     |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*                                              */

#ifdef CONFIG_PM_SLEEP
/**
 * struct ms_main_irq_priv - private main interrupt data
 * @irq_polarity:   irq polarity
 * @fiq_polarity:   fiq polarity
 */
struct sstar_main_irq_priv
{
    u16 irq_mask[(GIC_SPI_MS_IRQ_NR) >> 4];
    u16 fiq_mask[(GIC_SPI_MS_FIQ_NR) >> 4];
    u16 irq_polarity[(GIC_SPI_MS_IRQ_NR) >> 4];
    u16 fiq_polarity[(GIC_SPI_MS_FIQ_NR) >> 4];
};

static struct sstar_main_irq_priv sstar_main_irq_priv;
#endif // CONFIG_PM_SLEEP

static void sstar_main_irq_ack(struct irq_data *d)
{
    s16 sstar_fiq;

    sstar_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (sstar_fiq >= 0 && sstar_fiq < GIC_SPI_MS_FIQ_NR)
    {
        // Don't use SETREG16_BIT_OP to clear status, because the register type is write 1 clear.
        OUTREG16((BASE_REG_INTRCTL_PA + REG_ID_4C + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (sstar_fiq >= GIC_SPI_MS_FIQ_NR)
    {
        pr_err("[%s] Unknown hwirq %lu, ms_fiq %d\n", __func__, d->hwirq, sstar_fiq);
        return;
    }

    if (d && d->chip && d->parent_data && d->parent_data->chip->irq_ack)
    {
        irq_chip_ack_parent(d);
    }
}

static void sstar_main_irq_eoi(struct irq_data *d)
{
    s16 sstar_fiq;

    sstar_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (sstar_fiq >= 0 && sstar_fiq < GIC_SPI_MS_FIQ_NR)
    {
        // Don't use SETREG16_BIT_OP to clear status, because the register type is write 1 clear.
        OUTREG16((BASE_REG_INTRCTL_PA + REG_ID_4C + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (sstar_fiq >= GIC_SPI_MS_FIQ_NR)
    {
        pr_err("[%s] Unknown hwirq %lu, ms_fiq %d\n", __func__, d->hwirq, sstar_fiq);
        return;
    }

    irq_chip_eoi_parent(d);
}

static void sstar_main_irq_mask(struct irq_data *d)
{
    s16 sstar_irq;
    s16 sstar_fiq;

    sstar_irq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    sstar_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;
    pr_debug("[%s]  hwirq %lu %s=%d\n", __func__, d->hwirq, (sstar_fiq >= 0) ? "sstar_fiq" : "sstar_irq",
             (sstar_fiq >= 0) ? sstar_fiq : sstar_irq);

    if (sstar_fiq >= 0 && sstar_fiq < GIC_SPI_MS_FIQ_NR)
    {
        SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_44 + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (sstar_irq >= 0 && sstar_irq < GIC_SPI_MS_IRQ_NR)
    {
        SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_54 + (sstar_irq / 16) * 4), (1 << (sstar_irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_mask_parent(d);
}

static void sstar_main_irq_unmask(struct irq_data *d)
{
    s16 sstar_irq;
    s16 sstar_fiq;

    sstar_irq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    sstar_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (sstar_fiq >= 0 && sstar_fiq < GIC_SPI_MS_FIQ_NR)
    {
        CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_44 + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (sstar_irq >= 0 && sstar_irq < GIC_SPI_MS_IRQ_NR)
    {
        CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_54 + (sstar_irq / 16) * 4), (1 << (sstar_irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_unmask_parent(d);
}

#ifdef CONFIG_SMP
static int sstar_irq_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
    // use a very simple implementation here...
    return irq_chip_set_affinity_parent(data, dest, true);
}
#endif

static int sstar_irq_set_priority(struct irq_data *data, irqpriority_t priority)
{
    // use a very simple implementation here...
    return irq_chip_set_priority_parent(data, priority);
}

static int sstar_main_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
    s16 sstar_irq;
    s16 sstar_fiq;

    if ((flow_type & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH)
    {
        pr_err("Not support IRQ_TYPE_EDGE_BOTH mode 0x%x\n", flow_type);
        return 0;
    }

    sstar_irq = data->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    sstar_fiq = data->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (sstar_fiq >= 0 && sstar_fiq < GIC_SPI_MS_FIQ_NR)
    {
        if (flow_type & IRQ_TYPE_EDGE_FALLING)
            SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_48 + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_48 + (sstar_fiq / 16) * 4), (1 << (sstar_fiq % 16)));
    }
    else if (sstar_irq >= 0 && sstar_irq < GIC_SPI_MS_IRQ_NR)
    {
        if (flow_type & IRQ_TYPE_LEVEL_LOW)
            SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_58 + (sstar_irq / 16) * 4), (1 << (sstar_irq % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_58 + (sstar_irq / 16) * 4), (1 << (sstar_irq % 16)));
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, data->hwirq);
        return -EINVAL;
    }
    irq_chip_set_type_parent(data, flow_type);
    return 0;
}

static struct irq_chip sstar_main_intc_irqchip = {
    .name         = "SSTAR_MAIN_INTC",
    .irq_ack      = sstar_main_irq_ack,
    .irq_eoi      = sstar_main_irq_eoi,
    .irq_mask     = sstar_main_irq_mask,
    .irq_unmask   = sstar_main_irq_unmask,
    .irq_set_type = sstar_main_irq_set_type,
#ifdef CONFIG_SMP
    .irq_set_affinity = sstar_irq_set_affinity,
#endif
    .irq_set_priority = sstar_irq_set_priority,
};

static int sstar_main_intc_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec, unsigned long *hwirq,
                                            unsigned int *type)
{
    if (is_of_node(fwspec->fwnode))
    {
        if (fwspec->param_count != 3)
            return -EINVAL;

        /* No PPI should point to this domain */
        if (fwspec->param[0] != 0)
            return -EINVAL;

        *hwirq = fwspec->param[1];
        *type  = fwspec->param[2];
        return 0;
    }

    return -EINVAL;
}

static int sstar_main_intc_domain_alloc(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    struct irq_fwspec  parent_fwspec;
    irq_hw_number_t    hwirq;
    unsigned int       i;

    if (fwspec->param_count != 3)
        return -EINVAL; /* Not GIC compliant */

    if (fwspec->param[0] != GIC_SPI)
        return -EINVAL; /* No PPI should point to this domain */

    hwirq = fwspec->param[1];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_hwirq_and_chip(domain, virq + i, hwirq + i, &sstar_main_intc_irqchip, NULL);
        pr_debug("[SSTAR_MAIN_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq + i, virq + i);
    }

    parent_fwspec        = *fwspec;
    parent_fwspec.fwnode = domain->parent->fwnode;
    return irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, &parent_fwspec);
}

static void sstar_main_intc_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++)
    {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}

struct irq_domain_ops sstar_main_intc_domain_ops = {
    .translate = sstar_main_intc_domain_translate,
    .alloc     = sstar_main_intc_domain_alloc,
    .free      = sstar_main_intc_domain_free,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_irqchip_suspend(void)
{
    unsigned int i, num;

    num = (GIC_SPI_MS_IRQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        if (i >= 0)
        {
            sstar_main_irq_priv.irq_mask[i]     = INREG16(BASE_REG_INTRCTL_PA + REG_ID_54 + (i << 2));
            sstar_main_irq_priv.irq_polarity[i] = INREG16(BASE_REG_INTRCTL_PA + REG_ID_58 + (i << 2));
        }
    }
    num = (GIC_SPI_MS_FIQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        sstar_main_irq_priv.fiq_mask[i]     = INREG16(BASE_REG_INTRCTL_PA + REG_ID_44 + (i << 2));
        sstar_main_irq_priv.fiq_polarity[i] = INREG16(BASE_REG_INTRCTL_PA + REG_ID_48 + (i << 2));
    }

    pr_debug("sstar_irqchip_suspend\n\n");
    return 0;
}

static void sstar_irqchip_resume(void)
{
    unsigned int i, num;

    num = (GIC_SPI_MS_IRQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        if (i >= 0)
        {
            OUTREG16(BASE_REG_INTRCTL_PA + REG_ID_54 + (i << 2), sstar_main_irq_priv.irq_mask[i]);
            OUTREG16(BASE_REG_INTRCTL_PA + REG_ID_58 + (i << 2), sstar_main_irq_priv.irq_polarity[i]);
        }
    }

    num = (GIC_SPI_MS_FIQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        // in mask  ---> set polarity ---> clear  status --- > unmask
        OUTREG16(BASE_REG_INTRCTL_PA + REG_ID_48 + (i << 2), sstar_main_irq_priv.fiq_polarity[i]);
        OUTREG16(BASE_REG_INTRCTL_PA + REG_ID_4C + (i << 2), 0xFFFF);
        OUTREG16(BASE_REG_INTRCTL_PA + REG_ID_44 + (i << 2), sstar_main_irq_priv.fiq_mask[i]);
    }

    pr_debug("sstar_irqchip_resume\n\n");
}

struct syscore_ops sstar_irq_syscore_ops = {
    .suspend = sstar_irqchip_suspend,
    .resume  = sstar_irqchip_resume,
};
#endif

static int __init sstar_init_main_intc(struct device_node *np, struct device_node *interrupt_parent)
{
    struct irq_domain *parent_domain, *domain;

    if (!interrupt_parent)
    {
        pr_err("%s: %s no parent\n", __func__, np->name);
        return -ENODEV;
    }

    pr_err("%s: np->name=%s, parent=%s\n", __func__, np->name, interrupt_parent->name);

    parent_domain = irq_find_host(interrupt_parent);
    if (!parent_domain)
    {
        pr_err("%s: %s unable to obtain parent domain\n", __func__, np->name);
        return -ENXIO;
    }

    domain = irq_domain_add_hierarchy(parent_domain, 0, GIC_SPI_ARM_INTERNAL_NR + GIC_SPI_MS_IRQ_NR + GIC_SPI_MS_FIQ_NR,
                                      np, &sstar_main_intc_domain_ops, NULL);

    if (!domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }
#ifdef CONFIG_PM_SLEEP
    register_syscore_ops(&sstar_irq_syscore_ops);
#endif
    return 0;
}

IRQCHIP_DECLARE(sstar_main_intc, "sstar,main-intc", sstar_init_main_intc);
