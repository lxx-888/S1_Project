# SPDX-License-Identifier: GPL-2.0
# Makefile for the Linux device tree

obj-y			:= component.o core.o bus.o dd.o syscore.o \
			   driver.o class.o platform.o \
			   cpu.o firmware.o init.o map.o devres.o \
			   attribute_container.o transport_class.o \
			   topology.o container.o property.o cacheinfo.o \
			   swnode.o
obj-$(CONFIG_DEVTMPFS)	+= devtmpfs.o
obj-y			+= power/
obj-$(CONFIG_ISA_BUS_API)	+= isa.o
obj-y				+= firmware_loader/
obj-$(CONFIG_NUMA)	+= node.o
obj-$(CONFIG_MEMORY_HOTPLUG_SPARSE) += memory.o
ifeq ($(CONFIG_SYSFS),y)
obj-$(CONFIG_MODULES)	+= module.o
endif
obj-$(CONFIG_SYS_HYPERVISOR) += hypervisor.o
obj-$(CONFIG_REGMAP)	+= regmap/
obj-$(CONFIG_SOC_BUS) += soc.o
obj-$(CONFIG_PINCTRL) += pinctrl.o
obj-$(CONFIG_DEV_COREDUMP) += devcoredump.o
obj-$(CONFIG_GENERIC_MSI_IRQ_DOMAIN) += platform-msi.o
obj-$(CONFIG_GENERIC_ARCH_TOPOLOGY) += arch_topology.o

obj-y			+= test/
obj-y			+= arm/

ccflags-$(CONFIG_DEBUG_DRIVER) := -DDEBUG

