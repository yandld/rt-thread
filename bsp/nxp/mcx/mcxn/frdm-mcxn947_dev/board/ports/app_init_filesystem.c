/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-14     balanceTWK   add sdcard port file
 * 2021-02-26     Meco Man     fix a bug that cannot use fatfs in the main thread at starting up
 * 2024-08-29     Your Name    add tmpfs and improve filesystem structure
 */

#include <rtthread.h>
#include <dfs_fs.h>
#include "dfs_tmpfs.h"

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define TMPFS_MOUNT_PATH            "/"
#define TMPFS_SIZE                  (64 * 1024)  // 64KB for tmpfs

#if defined(BSP_USING_SPI7) && defined(RT_USING_SFUD) && defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
#include <dfs_elm.h>
#include "spi_flash_sfud.h"

#define W25Q64_SPI_DEVICE_NAME      "spi70"
#define W25Q64_SPI_BUS_NAME         "spi7"
#define W25Q64_SPI_FLASH_NAME       "w25qxx"
#define W25Q64_SPI_FLASH_CS_PIN     96

#define W25Q64_FS_MOUNT_PATH        "/sf"
#define SD_FS_MOUNT_PATH            "/sd"
#endif

static int filesystem_init(void)
{
    rt_err_t ret = RT_EOK;
    
    // Mount tmpfs
    if (dfs_mount(RT_NULL, TMPFS_MOUNT_PATH, "tmp", 0, 0) == 0)
    {
        mkdir(W25Q64_FS_MOUNT_PATH, 0);
        mkdir(SD_FS_MOUNT_PATH, 0);
        LOG_I("Tmpfs mounted to root.");
    }
    else
    {
        LOG_E("Failed to mount tmpfs to root.");
        return -RT_ERROR;
    }

#if defined(BSP_USING_SPI7) && defined(RT_USING_SFUD) && defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    // SPI Flash initialization and mounting

    struct rt_spi_device *spi_device = rt_malloc(sizeof(struct rt_spi_device));

    if (!spi_device)
    {
        LOG_E("Failed to allocate memory for SPI device.");
        return -RT_ERROR;
    }

    ret = rt_spi_bus_attach_device_cspin(spi_device, W25Q64_SPI_DEVICE_NAME, W25Q64_SPI_BUS_NAME, W25Q64_SPI_FLASH_CS_PIN, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("SPI flash device attach failed.");
        return -RT_ERROR;
    }
    
    struct rt_spi_configuration cfg =
    {
        .data_width = 8,
        .mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB,
        .max_hz = 50 * 1000 * 1000,
    };
    ret = rt_spi_configure(spi_device, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("SPI bus configuration failed.");
        return -RT_ERROR;
    }

    if (RT_NULL == rt_sfud_flash_probe(W25Q64_SPI_FLASH_NAME, W25Q64_SPI_DEVICE_NAME))
    {
        LOG_E("Flash sfud Failed!");
        return -RT_ERROR;
    }

    if (dfs_mount(W25Q64_SPI_FLASH_NAME, W25Q64_FS_MOUNT_PATH, "elm", 0, 0) != 0)
    {
        LOG_W("Initial ELM FAT mount failed, trying to format block device.");

        if (dfs_mkfs("elm", W25Q64_SPI_FLASH_NAME) != 0)
        {
            LOG_E("Failed to create ELM FAT filesystem.");
            return -RT_ERROR;
        }

        if (dfs_mount(W25Q64_SPI_FLASH_NAME, W25Q64_FS_MOUNT_PATH, "elm", 0, 0) != 0)
        {
            LOG_E("Failed to mount ELM FAT filesystem, check mount point.");
            return -RT_ERROR;
        }
    }

    LOG_I("ELM FAT filesystem mounted on SPI Flash at %s.", W25Q64_FS_MOUNT_PATH);
#endif

    
#ifdef RT_USING_SDIO
    
    rt_thread_mdelay(500);
    
    if (dfs_mount("sd", SD_FS_MOUNT_PATH, "elm", 0, NULL) == 0)
    {
        rt_kprintf("sd mounted to %s\n", SD_FS_MOUNT_PATH);
    }
    else
    {
        rt_kprintf("sd mount to %s failed\n", SD_FS_MOUNT_PATH);
    }
#endif
    
    return RT_EOK;
}

INIT_ENV_EXPORT(filesystem_init);
