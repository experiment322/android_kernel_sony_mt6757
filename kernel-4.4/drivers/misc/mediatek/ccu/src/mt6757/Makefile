#
# Copyright (C) 2016 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/misc/mediatek/m4u/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/include/mach
ccflags-y += -I$(srctree)/drivers/misc/mediatek/irq/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mmp/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ccu/src/1.0
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ccu/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ccu/src/$(MTK_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/ccu/src/$(MTK_PLATFORM)/ccu_ext_interface/
ccflags-y += -I$(srctree)/drivers/i2c/busses

#obj-$(CONFIG_MTK_VPU) += ccu_hw.o
obj-y += ccu_hw.o ccu_reg.o ccu_kd_mailbox.o ccu_i2c_hw.o

