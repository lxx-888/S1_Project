# SPDX-License-Identifier: GPL-2.0+

obj-$(CONFIG_$(SPL_TPL_)BUTTON) += button/
obj-$(CONFIG_$(SPL_TPL_)CACHE) += cache/
obj-$(CONFIG_$(SPL_TPL_)CLK) += clk/
obj-$(CONFIG_$(SPL_TPL_)DM) += core/
obj-$(CONFIG_$(SPL_TPL_)DFU) += dfu/
obj-$(CONFIG_$(SPL_TPL_)GPIO) += gpio/
obj-$(CONFIG_$(SPL_TPL_)DRIVERS_MISC) += misc/
obj-$(CONFIG_$(SPL_TPL_)SYSRESET) += sysreset/
obj-$(CONFIG_$(SPL_TPL_)FIRMWARE) +=firmware/
obj-$(CONFIG_$(SPL_TPL_)I2C) += i2c/
obj-$(CONFIG_$(SPL_TPL_)INPUT) += input/
obj-$(CONFIG_$(SPL_TPL_)LED) += led/
obj-$(CONFIG_$(SPL_TPL_)MMC_SUPPORT) += mmc/
obj-y += mtd/
obj-$(CONFIG_$(SPL_)MULTIPLEXER) += mux/
obj-$(CONFIG_$(SPL_TPL_)PCH_SUPPORT) += pch/
obj-$(CONFIG_$(SPL_TPL_)PCI) += pci/
obj-$(CONFIG_$(SPL_TPL_)PHY) += phy/
obj-$(CONFIG_$(SPL_TPL_)PINCTRL) += pinctrl/
obj-$(CONFIG_$(SPL_TPL_)RAM) += ram/
obj-$(CONFIG_$(SPL_TPL_)RTC_SUPPORT) += rtc/
obj-$(CONFIG_$(SPL_TPL_)SERIAL_SUPPORT) += serial/
obj-$(CONFIG_$(SPL_TPL_)SPI_SUPPORT) += spi/
obj-$(CONFIG_$(SPL_TPL_)TIMER) += timer/
obj-$(CONFIG_$(SPL_TPL_)VIRTIO) += virtio/
obj-$(CONFIG_$(SPL_)DM_MAILBOX) += mailbox/
obj-$(CONFIG_$(SPL_)REMOTEPROC) += remoteproc/
obj-$(CONFIG_$(SPL_)SYSINFO) += sysinfo/
obj-$(CONFIG_$(SPL_TPL_)TPM) += tpm/
obj-$(CONFIG_$(SPL_TPL_)ACPI_PMC) += power/acpi_pmc/
obj-$(CONFIG_XEN) += xen/
obj-$(CONFIG_$(SPL_)FPGA) += fpga/

ifndef CONFIG_TPL_BUILD
ifdef CONFIG_SPL_BUILD

obj-$(CONFIG_SPL_BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_SPL_CACHE_SUPPORT) += cache/
obj-$(CONFIG_SPL_CPU) += cpu/
obj-$(CONFIG_SPL_CRYPTO) += crypto/
obj-$(CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/
obj-$(CONFIG_ARMADA_38X) += ddr/marvell/a38x/
obj-$(CONFIG_ARMADA_XP) += ddr/marvell/axp/
obj-$(CONFIG_$(SPL_)ALTERA_SDRAM) += ddr/altera/
obj-$(CONFIG_ARCH_IMX8M) += ddr/imx/imx8m/
obj-$(CONFIG_IMX8ULP_DRAM) += ddr/imx/imx8ulp/
obj-$(CONFIG_SPL_POWER) += power/ power/pmic/
obj-$(CONFIG_SPL_POWER) += power/regulator/
obj-$(CONFIG_SPL_POWER_DOMAIN) += power/domain/
obj-$(CONFIG_SPL_DM_RESET) += reset/
obj-$(CONFIG_SPL_DMA) += dma/
obj-$(CONFIG_SPL_ETH) += net/
obj-$(CONFIG_SPL_ETH) += net/phy/
obj-$(CONFIG_SPL_USB_ETHER) += net/phy/
obj-$(CONFIG_SPL_MUSB_NEW) += usb/musb-new/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/
obj-$(CONFIG_SPL_USB_GADGET) += usb/common/
obj-$(CONFIG_SPL_USB_GADGET) += usb/gadget/udc/
obj-$(CONFIG_SPL_WATCHDOG) += watchdog/
obj-$(CONFIG_SPL_USB_HOST) += usb/host/
obj-$(CONFIG_OMAP_USB_PHY) += usb/phy/
obj-$(CONFIG_SPL_SATA_SUPPORT) += ata/ scsi/
obj-$(CONFIG_HAVE_BLOCK_DEVICE) += block/
obj-$(CONFIG_SPL_THERMAL) += thermal/

endif
endif

ifdef CONFIG_TPL_BUILD

obj-$(CONFIG_TPL_BOOTCOUNT_LIMIT) += bootcount/
obj-$(CONFIG_TPL_MPC8XXX_INIT_DDR_SUPPORT) += ddr/fsl/

endif

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)

obj-y += adc/
obj-y += ata/
obj-y += bus/
obj-$(CONFIG_DM_DEMO) += demo/
obj-$(CONFIG_BIOSEMU) += bios_emulator/
obj-y += block/
obj-$(CONFIG_BOOTCOUNT_LIMIT) += bootcount/
obj-y += cache/
obj-$(CONFIG_CPU) += cpu/
obj-y += crypto/
obj-$(CONFIG_FASTBOOT) += fastboot/
obj-y += misc/
obj-$(CONFIG_MMC) += mmc/
obj-$(CONFIG_NVME) += nvme/
obj-$(CONFIG_PCI_ENDPOINT) += pci_endpoint/
obj-y += dfu/
obj-$(CONFIG_PCH) += pch/
obj-$(CONFIG_DM_REBOOT_MODE) += reboot-mode/
obj-y += rtc/
obj-y += scsi/
obj-y += sound/
obj-y += spmi/
obj-y += video/
obj-y += watchdog/
obj-$(CONFIG_QE) += qe/
obj-$(CONFIG_U_QE) += qe/
obj-y += mailbox/
obj-y += memory/
obj-y += mtd/
obj-y += pwm/
obj-y += reset/
obj-y += input/
# SOC specific infrastructure drivers.
obj-y += smem/
obj-y += thermal/
obj-$(CONFIG_TEE) += tee/
obj-y += axi/
obj-y += ufs/
obj-$(CONFIG_W1) += w1/
obj-$(CONFIG_W1_EEPROM) += w1-eeprom/

obj-$(CONFIG_MACH_PIC32) += ddr/microchip/
obj-$(CONFIG_DM_HWSPINLOCK) += hwspinlock/
obj-$(CONFIG_DM_RNG) += rng/
endif
obj-$(CONFIG_SSTAR_DRIVERS) += sstar/
obj-y += soc/
