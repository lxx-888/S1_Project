/*
* sstar_chip.c- Sigmastar
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
#include <linux/semaphore.h>
#include <asm/hardware/cache-l2x0.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include "mdrv_API_version.h"
#include "ss_private.h"
#include "ms_platform.h"
#include "ms_version.h"
#include "registers.h"

#define UNUSED(var) (void)((var) = (var))

#ifdef CONFIG_OUTER_CACHE
	static DEFINE_SPINLOCK(sstar_irq_L2_lock);
#endif

DEFINE_SEMAPHORE(PfModeSem);
DEFINE_MUTEX(FCIE3_mutex);
EXPORT_SYMBOL(FCIE3_mutex);
static struct ss_chip chip_funcs;

const struct SS_BIN_OPTION ss_image_size __attribute__ ((aligned (16))) =
{
	{'#','I','M','G','_','S','Z','#'},
	{0,0,0,0,0,0,0,0}
};

const struct SS_BIN_OPTION ms_XIP_phys_addr __attribute__ ((aligned (16))) =
{
    {'#','X','I','P','_','P','A','#'},
    {0,0,0,0,0,0,0,0}
};


static char NONE_platform_name[]="NONE";
////const struct MS_BIN_OPTION ms_builtin_dtb_size __attribute__ ((aligned (16)))=
////{
////	{'#','D','T','B','_','S','Z','#'},
////	{0,0,0,0,0,0,0,0}
////};
//
//

#ifdef CONFIG_SSTAR_BUILTIN_DTB
#define BUILTIN_DTB_SZ (128*1024)
struct SS_BUILTIN_DTB
{
	struct SS_BIN_OPTION ss_dtb;
	u8 content[BUILTIN_DTB_SZ];
};


struct SS_BUILTIN_DTB builtin_dtb __attribute__ ((aligned (16)))=
{
		.ss_dtb={ {'#','M','S','_','D','T','B','#'}, {0,0,0,0,0,0,0,0}},
};
void *builtin_dtb_start=(void *)builtin_dtb.content;
const u32  builtin_dtb_size=BUILTIN_DTB_SZ;
#endif

#ifdef CONFIG_SS_BUILTIN_UNFDT
#define BUILTIN_UNFDT_SZ (144*1024)
struct SS_BUILTIN_UNFDT
{
	struct SS_BIN_OPTION ss_dtb;
	u8 content[BUILTIN_UNFDT_SZ];
};

struct SS_BUILTIN_UNFDT builtin_unfdt __attribute__ ((aligned (16)))=
{
		.ss_dtb={ {'#','U','N','F','D','T','_','#'}, {0,0,0,0,0,0,0,0}},
};
void *builtin_unfdt_start = (void *)builtin_unfdt.content;
void *unfdt_runtime_base =(void*) &builtin_unfdt.ss_dtb.args[0];
void *fdt_runtime_base =(void*) &builtin_unfdt.ss_dtb.args[4];
#endif

//MS_VERSION LX_VERSION =
//{
//	{'M','V','X'},
//	MVXV_HEAD_VER,
//	MVXV_LIB_TYPE, //R = general release version
//	MVXV_CHIP_ID,
//	MVXV_CHANGELIST,
//	MVXV_COMP_ID,
//	{'#'},
//	{'X','V','M'},
//};

#if defined(MVXV_EXT)
#define MVXV_V2 "MVX" MVXV_HEAD_VER MVXV_LIB_TYPE MVXV_CHIP_ID MVXV_CHANGELIST MVXV_COMP_ID MVXV_EXT "#XVM"
#else
#define MVXV_V2 "MVX" MVXV_HEAD_VER MVXV_LIB_TYPE MVXV_CHIP_ID MVXV_CHANGELIST MVXV_COMP_ID "#XVM"
#endif
const char* LX_VERSION=MVXV_V2;

const char *ms_API_version=""KL_API_VERSION;

void Chip_Flush_MIU_Pipe(void){
        chip_funcs.chip_flush_miu_pipe();
}

void Chip_Flush_MIU_Pipe_Nodsb(void){
		chip_funcs.chip_flush_miu_pipe_nodsb();
}

void Chip_Flush_Dcache_Page(struct page *page){
		chip_funcs.cache_flush_dcache_page(page);
}

void Chip_Flush_Cache_Range_VA_PA(void* pVAddr, ss_phys_addr_t u64PAddr,unsigned long u32Size)
{
    chip_funcs.cache_flush_range_va_pa(pVAddr, u64PAddr, u32Size);
}

void Chip_Flush_Cache_Range(void* pAddr, unsigned long u32Size)
{
    chip_funcs.cache_flush_range(pAddr,u32Size);
}

void Chip_Clean_Cache_Range(void* pAddr, unsigned long u32Size)
{
    chip_funcs.cache_clean_range(pAddr,u32Size);
}

void Chip_Inv_Cache_Range(void* pAddr, unsigned long u32Size)
{
    chip_funcs.cache_invalidate_range(pAddr,u32Size);
}

void Chip_Flush_CacheAll(void)
{
    WARN_ON(1);
    printk("'Chip_Flush_CacheAll' can't be used in arm64, flushing the cache by VA\n");
//    chip_funcs.cache_flush_all();
}


ss_miu_addr_t Chip_Phys_to_MIU(ss_phys_addr_t phys)
{
	return chip_funcs.phys_to_miu(phys);
}

ss_phys_addr_t Chip_MIU_to_Phys(ss_miu_addr_t miu)
{
	return chip_funcs.miu_to_phys(miu);
}

ss_imi_addr_t Chip_Phys_to_IMI(ss_phys_addr_t phys)
{
	return chip_funcs.phys_to_imi(phys);
}

ss_phys_addr_t Chip_IMI_to_Phys(ss_imi_addr_t imi)
{
	return chip_funcs.imi_to_phys(imi);
}

int Chip_Phys_Check_Range_In_MIU(ss_phys_addr_t phys)
{
	return chip_funcs.phys_check_range_in_miu(phys);
}

int Chip_Phys_Check_Range_In_IMI(ss_phys_addr_t phys)
{
	return chip_funcs.phys_check_range_in_imi(phys);
}

int Chip_Get_Device_ID(void)
{
	return chip_funcs.chip_get_device_id();
}

int Chip_Get_Revision(void)
{
	return chip_funcs.chip_get_revision();
}


int Chip_Cache_Outer_Is_Enabled(void)
{
	return chip_funcs.cache_outer_is_enabled();
}


int Chip_Boot_Get_Dev_Type(void)
{
	return (int)((MS_BOOT_DEV_TYPE)chip_funcs.chip_get_boot_dev_type());
}

char* Chip_Get_Platform_Name(void)
{
	char *name=chip_funcs.chip_get_platform_name();
	if(name==NULL || strlen(name) > 15 )
	{
		//printk(KERN_ERR "platform name invalid!! must not be NULL & < 15 chars\n");
		BUG();
	}

	return name;
}

const char* Chip_Get_API_Version(void)
{
	return chip_funcs.chip_get_API_version();
}


//u32 Chip_Get_Image_Size(void)
//{
//	return (u32)(__le32_to_cpu(*((__le64 *)((void *)&ms_.args[0]))));
//}
////
//u32 Chip_Get_DTB_Size(void)
//{
//	return (u32)(__le32_to_cpu(*((__le64 *)((void *)&MS_DTB_SIZE.args[0]))));
//}

/*************************************
*		default chip function
*************************************/

static char* _default_Get_Platform_Name(void)
{
	return NONE_platform_name;
}


static ss_miu_addr_t _default_Phys_to_MIU(ss_phys_addr_t phys)
{
	return phys;
}

static ss_phys_addr_t _default_MIU_to_Phys(ss_miu_addr_t miu)
{
	return miu;
}

static void _default_flush_miu_pipe(void)
{

}
//

//DCACHE_FLUSH function
static void _default_Flush_Dcache_Page(struct page *page)
{
    flush_dcache_page(page);
    Chip_Flush_MIU_Pipe_Nodsb();
}

static void _default_Flush_Cache_Range_VA_PA(void* pVAddr,unsigned long long u64PAddr,unsigned long u32Size)
{
    if(pVAddr == NULL)
    {
        printk("pVAddr is invalid\n");
		return;
	}

	if ((uintptr_t)pVAddr & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: address(0x%px) not aligned with D-cache line size(%d)\n", __FUNCTION__, pVAddr, cache_line_size());
		dump_stack();
	}

	if ((uintptr_t)u32Size & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: size(%ld) not aligned with D-cache line size(%d)\n", __FUNCTION__, u32Size, cache_line_size());
		dump_stack();
	}

	//Clean & Invalid L1
	__dma_flush_area(pVAddr,u32Size);

#ifdef CONFIG_OUTER_CACHE
#ifdef CONFIG_MS_L2X0_PATCH
	if (Chip_Cache_Outer_Is_Enabled()) //check if L2 is enabled
#endif
	{
		//Clean&Inv L2 by range
	    outer_cache.flush_range(u64PAddr,u64PAddr + u32Size);
	}
#endif

#ifndef CONFIG_OUTER_CACHE
	Chip_Flush_MIU_Pipe_Nodsb();
#endif
}

static void _default_Flush_Cache_Range(void* pAddr, unsigned long u32Size)
{
#ifdef CONFIG_OUTER_CACHE
	unsigned long flags;
#endif
	if(pAddr == NULL )
    {
	    printk("pAddr is invalid\n");
	    return;
    }

	if ((uintptr_t)pAddr & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: address(0x%px) not aligned with D-cache line size(%d)\n", __FUNCTION__, pAddr, cache_line_size());
		dump_stack();
	}

	if ((uintptr_t)u32Size & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: size(%ld) not aligned with D-cache line size(%d)\n", __FUNCTION__, u32Size, cache_line_size());
		dump_stack();
	}

	//Clean L1 & Inv L1
	__dma_flush_area(pAddr,u32Size);

#ifdef CONFIG_OUTER_CACHE
#ifdef CONFIG_MS_L2X0_PATCH
    if (Chip_Cache_Outer_Is_Enabled()) //check if L2 is enabled
#endif
    {
        if(!virt_addr_valid(pAddr) || !virt_addr_valid(pAddr+ u32Size - 1 )){
            //Clean&Inv L2 by Way
            spin_lock_irqsave(&sstar_irq_L2_lock, flags);
            outer_cache.flush_all();
            spin_unlock_irqrestore(&sstar_irq_L2_lock, flags);}
        else
            //Clean&Inv L2 by Range
            outer_cache.flush_range(__pa(pAddr) , __pa(pAddr) + u32Size);

    }
#endif

#ifndef CONFIG_OUTER_CACHE
    Chip_Flush_MIU_Pipe_Nodsb();
#endif

}

static void _default_Clean_Cache_Range(void* pAddr, unsigned long u32Size)
{
    if(pAddr == NULL)
    {
        printk("pAddr is invalid\n");
        return;
    }

	if ((uintptr_t)pAddr & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: address(0x%px) not aligned with D-cache line size(%d)\n", __FUNCTION__, pAddr, cache_line_size());
		dump_stack();
	}

	if ((uintptr_t)u32Size & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: size(%ld) not aligned with D-cache line size(%d)\n", __FUNCTION__, u32Size, cache_line_size());
		dump_stack();
	}

    //Clean L1
    //dmac_map_area((const void *)u32Addr,(size_t)u32Size,1);
    __dma_map_area((const void *)pAddr, (size_t)u32Size,DMA_TO_DEVICE);
    //dma_sync_single_for_cpu(NULL, virt_to_phys(pAddr), u32Size, DMA_TO_DEVICE);
#ifdef CONFIG_OUTER_CACHE
#ifdef CONFIG_MS_L2X0_PATCH
    if (Chip_Cache_Outer_Is_Enabled()) //check if L2 is enabled
#endif
    {
        if(!virt_addr_valid(pAddr) || !virt_addr_valid(pAddr+ u32Size - 1))
        {
            //Clean L2 by range
            outer_cache.flush_all();
        }
        else
            //Clean&Inv L2 by Range
            outer_cache.clean_range(__pa(pAddr) , __pa(pAddr) + u32Size);
    }
#endif

    #ifndef CONFIG_OUTER_CACHE
    Chip_Flush_MIU_Pipe_Nodsb();
    #endif
}


static void _default_Inv_Cache_Range(void* pAddr, unsigned long u32Size)
{
    if(pAddr == NULL)
    {
        printk("pAddr is invalid\n");
        return;
    }

	if ((uintptr_t)pAddr & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: address(0x%px) not aligned with D-cache line size(%d)\n", __FUNCTION__, pAddr, cache_line_size());
		dump_stack();
	}

	if ((uintptr_t)u32Size & (cache_line_size() - 1))
	{
		printk(KERN_ERR "%s: size(%ld) not aligned with D-cache line size(%d)\n", __FUNCTION__, u32Size, cache_line_size());
		dump_stack();
	}

#ifdef CONFIG_OUTER_CACHE
#ifdef CONFIG_MS_L2X0_PATCH
    if (Chip_Cache_Outer_Is_Enabled()) //check if L2 is enabled
#endif
    {
        if(!virt_addr_valid(pAddr) || !virt_addr_valid(pAddr + u32Size - 1))
        {
           printk("%s:Input VA can't be converted to PA\n",__func__);
        }
        else
        {
            //Inv L2 by range
            outer_cache.inv_range(__pa(pAddr) , __pa(pAddr)+ u32Size);
        }
    }
#endif
    //Inv L1
    //dmac_map_area((const void *)u32Addr,(size_t)u32Size,2);
    __dma_map_area((const void *)pAddr, (size_t)u32Size,DMA_FROM_DEVICE);
    //dma_sync_single_for_cpu(NULL, virt_to_phys(pAddr), u32Size, DMA_FROM_DEVICE);

}

static void _default_Flush_CacheAll(void)
{

//    __cpuc_flush_icache_all();
//    __cpuc_flush_kern_all();
//    __cpuc_flush_user_all();
#ifdef CONFIG_OUTER_CACHE
#ifdef CONFIG_MS_L2X0_PATCH
    if (Chip_Cache_Outer_Is_Enabled()) //check if L2 is enabled
#endif
    {
    	//Clean&Inv L2 by range
    	outer_cache.flush_all();
    }
#endif

#ifndef CONFIG_OUTER_CACHE
    Chip_Flush_MIU_Pipe_Nodsb();
#endif
}


static int _default_Get_Revision(void)
{
	return 0;
}

static int _default_Get_ID(void)
{
	return (int)DEVICE_ID_END;
}


static int _default_Get_Boot_Dev_Type(void)
{
	return (int)MS_BOOT_DEV_NONE;
}

static int _default_get_storage_type(void)
{
	return (int)MS_STORAGE_UNKNOWN;
}

static int _default_get_package_type(void)
{
	return 0;
}


static int _default_Outer_Cache_Is_Enabled(void)
{
	return 0;
}


static unsigned long long _default_Get_RIU_Phys(void)
{
	return 0;
}

static int _default_Get_RIU_Size(void)
{
	return 0;
}

static int _default_chip_function_set(int id, int param)
{
	UNUSED(id);
	UNUSED(param);
	//printk(KERN_ERR "CHIP_FUNCTION not yet implemented\n!!");
	return -1;
}


static const char* _default_chip_get_API_version(void)
{
	return ms_API_version;
}

static u64 _default_chip_get_us_ticks(void)
{
	return 0;
}

struct ss_chip* sstar_chip_get(void)
{
	return &chip_funcs;
}

void __init sstar_chip_init_default(void)
{
	chip_funcs.cache_clean_range=_default_Clean_Cache_Range;
	chip_funcs.cache_flush_range=_default_Flush_Cache_Range;
	chip_funcs.cache_flush_all=_default_Flush_CacheAll;
	chip_funcs.cache_invalidate_range=_default_Inv_Cache_Range;

	chip_funcs.cache_flush_range_va_pa=_default_Flush_Cache_Range_VA_PA;
       chip_funcs.cache_flush_dcache_page=_default_Flush_Dcache_Page;

	chip_funcs.chip_flush_miu_pipe=_default_flush_miu_pipe;

	chip_funcs.phys_to_miu=_default_Phys_to_MIU;
	chip_funcs.miu_to_phys=_default_MIU_to_Phys;

	chip_funcs.chip_get_device_id=_default_Get_ID;
	chip_funcs.chip_get_revision=_default_Get_Revision;

	chip_funcs.cache_outer_is_enabled=_default_Outer_Cache_Is_Enabled;

	chip_funcs.chip_get_boot_dev_type=_default_Get_Boot_Dev_Type;
	chip_funcs.chip_get_platform_name=_default_Get_Platform_Name;

	chip_funcs.chip_get_riu_phys=_default_Get_RIU_Phys;
	chip_funcs.chip_get_riu_size=_default_Get_RIU_Size;

	chip_funcs.chip_function_set=_default_chip_function_set;

	chip_funcs.chip_get_API_version=_default_chip_get_API_version;

       chip_funcs.chip_get_storage_type=_default_get_storage_type;
	chip_funcs.chip_get_package_type=_default_get_package_type;

	chip_funcs.chip_get_us_ticks=_default_chip_get_us_ticks;

}

unsigned long long Chip_Get_RIU_Phys(void)
{
	return chip_funcs.chip_get_riu_phys();
}

int Chip_Get_RIU_Size(void)
{
	return chip_funcs.chip_get_riu_size();
}

int Chip_Function_Set(int function_id, int param)
{
	return chip_funcs.chip_function_set(function_id,param);
}

int Chip_Get_Storage_Type(void)
{
	return (int)chip_funcs.chip_get_storage_type();
}

int Chip_Get_Package_Type(void)
{
	return (int)chip_funcs.chip_get_package_type();
}

u64 Chip_Get_US_Ticks(void)
{
	return chip_funcs.chip_get_us_ticks();
}

EXPORT_SYMBOL(sstar_chip_get);

EXPORT_SYMBOL(Chip_Flush_MIU_Pipe);
EXPORT_SYMBOL(Chip_Flush_Dcache_Page);
EXPORT_SYMBOL(Chip_Flush_Cache_Range);
EXPORT_SYMBOL(Chip_Clean_Cache_Range);
EXPORT_SYMBOL(Chip_Flush_Cache_Range_VA_PA);
EXPORT_SYMBOL(Chip_Inv_Cache_Range);
EXPORT_SYMBOL(Chip_Flush_CacheAll);
EXPORT_SYMBOL(Chip_Phys_to_MIU);
EXPORT_SYMBOL(Chip_MIU_to_Phys);

EXPORT_SYMBOL(Chip_Cache_Outer_Is_Enabled);

EXPORT_SYMBOL(Chip_Get_Device_ID);
EXPORT_SYMBOL(Chip_Get_Revision);
EXPORT_SYMBOL(Chip_Get_Platform_Name);
EXPORT_SYMBOL(Chip_Boot_Get_Dev_Type);
EXPORT_SYMBOL(Chip_Get_Storage_Type);
EXPORT_SYMBOL(Chip_Get_Package_Type);

EXPORT_SYMBOL(Chip_Get_US_Ticks);

EXPORT_SYMBOL(Chip_Get_RIU_Phys);
EXPORT_SYMBOL(Chip_Get_RIU_Size);
EXPORT_SYMBOL(Chip_Function_Set);





