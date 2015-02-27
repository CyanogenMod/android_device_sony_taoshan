# Copyright 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Product-specific compile-time definitions.

include vendor/sony/taoshan/BoardConfigVendor.mk

# inherit qcom common sepolicies
include device/qcom/sepolicy/sepolicy.mk

# inherit from Sony common
include device/sony/common/BoardConfigCommon.mk

# inherit from msm8960-common
include device/sony/msm8960-common/BoardConfigCommon.mk

USE_CAMERA_STUB := false

TARGET_BOARD_PLATFORM := msm8960
TARGET_CPU_VARIANT := krait
BOARD_VENDOR_PLATFORM := taoshan
TARGET_BOOTLOADER_BOARD_NAME := qcom

TARGET_OTA_ASSERT_DEVICE := C2105,C2104,c2105,c2104,taoshan

TARGET_GLOBAL_CFLAGS += -mfpu=neon-vfpv4 -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mfpu=neon-vfpv4 -mfloat-abi=softfp
COMMON_GLOBAL_CFLAGS += -D__ARM_USE_PLD -D__ARM_CACHE_LINE_SIZE=64

BOARD_KERNEL_BASE := 0x80200000
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x02000000
# the androidboot.hardware has impact on loading .rc files
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.hardware=qcom androidboot.selinux=permissive androidboot.bootdevice=msm_sdcc.1 user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 maxcpus=2
BOARD_KERNEL_PAGESIZE := 2048
BOARD_FLASH_BLOCK_SIZE := 131072 # (BOARD_KERNEL_PAGESIZE * 64)

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4

BOARD_BOOTIMAGE_PARTITION_SIZE := 0x108BB9E
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1258291200
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1711276032
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456

BOARD_EGL_CFG := device/sony/taoshan/rootdir/system/lib/egl/egl.cfg

BOARD_VOLD_MAX_PARTITIONS := 33
BOARD_VOLD_DISC_HAS_MULTIPLE_MAJORS := true
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true
TARGET_USE_CUSTOM_LUN_FILE_PATH := /sys/devices/platform/msm_hsusb/gadget/lun%d/file

TARGET_KERNEL_SOURCE := kernel/sony/msm8930
TARGET_KERNEL_CONFIG := cyanogenmod_taoshan_defconfig

BOARD_USES_QCOM_HARDWARE := true
TARGET_USES_QCOM_BSP := true

# Bionic
TARGET_USE_QCOM_BIONIC_OPTIMIZATION := true

# Enable Minikin text layout engine (will be the default soon)
USE_MINIKIN := true

# Include an expanded selection of fonts
EXTENDED_FONT_FOOTPRINT := true

# MMap compatibility
BOARD_USES_LEGACY_MMAP := true

# Use dlmalloc
MALLOC_IMPL := dlmalloc

# GPS
BOARD_HAVE_NEW_QC_GPS := true

TARGET_SPECIFIC_HEADER_PATH += device/sony/taoshan/include

BOARD_RIL_NO_CELLINFOLIST := true

BOARD_USES_ALSA_AUDIO := true
BOARD_USES_LEGACY_ALSA_AUDIO := true
TARGET_USES_QCOM_COMPRESSED_AUDIO := true
QCOM_ANC_HEADSET_ENABLED := false
AUDIO_FEATURE_ENABLED_FM := true

TARGET_ENABLE_QC_AV_ENHANCEMENTS := true

USE_DEVICE_SPECIFIC_CAMERA := true
COMMON_GLOBAL_CFLAGS += -DNEEDS_VECTORIMPL_SYMBOLS
COMMON_GLOBAL_CFLAGS += -DSONY_CAM_PARAMS

# Vendor Init
TARGET_UNIFIED_DEVICE := true
TARGET_INIT_VENDOR_LIB := libinit_msm
TARGET_LIBINIT_DEFINES_FILE := device/sony/taoshan/init/init_taoshan.c

# Wlan
BOARD_HAS_QCOM_WLAN              := true
BOARD_WLAN_DEVICE                := qcwcn
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/wlan.ko"
WIFI_DRIVER_MODULE_NAME          := "wlan"
WIFI_DRIVER_FW_PATH_STA          := "sta"
WIFI_DRIVER_FW_PATH_AP           := "ap"

BOARD_HAVE_BLUETOOTH_QCOM := true
BLUETOOTH_HCI_USE_MCT := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/sony/taoshan/bluetooth

BOARD_RIL_NO_SEEK := true

# Recovery
TARGET_RECOVERY_FSTAB = device/sony/taoshan/rootdir/root/fstab.qcom
BOARD_HAS_NO_SELECT_BUTTON := true
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
BOARD_CUSTOM_BOOTIMG_MK := device/sony/taoshan/custombootimg.mk

BOARD_USES_QC_TIME_SERVICES := true

TARGET_POWERHAL_NO_TOUCH_BOOST := true

BOARD_HARDWARE_CLASS := device/sony/taoshan/cmhw

# TWRP configs
DEVICE_RESOLUTION := 480x854
RECOVERY_GRAPHICS_USE_LINELENGTH := true
TW_HAS_NO_RECOVERY_PARTITION := true
TW_FLASH_FROM_STORAGE := true
TW_EXTERNAL_STORAGE_PATH := "/external_sd"
TW_EXTERNAL_STORAGE_MOUNT_POINT := "external_sd"
TW_DEFAULT_EXTERNAL_STORAGE := true
TW_INCLUDE_JB_CRYPTO := true
TW_CRYPTO_FS_TYPE := "ext4"
TW_CRYPTO_REAL_BLKDEV := "/dev/block/platform/msm_sdcc.1/by-name/userdata"
TW_CRYPTO_MNT_POINT := "/data"
TW_CRYPTO_FS_OPTIONS := "nosuid,nodev,barrier=1,noauto_da_alloc,discard"
TW_CRYPTO_FS_FLAGS := "0x00000406"
TW_CRYPTO_KEY_LOC := "footer"
TW_INCLUDE_FUSE_EXFAT := true
TW_BRIGHTNESS_PATH := /sys/class/leds/lcd-backlight/brightness
TW_MAX_BRIGHTNESS := 255
TW_NO_USB_STORAGE := true

TARGET_USES_LOGD := false

BOARD_SEPOLICY_DIRS += \
    device/sony/taoshan/sepolicy

BOARD_SEPOLICY_UNION += \
       adbd.te \
       device.te \
       dhcp.te \
       file_contexts \
       init_shell.te \
       location.te \
       mediaserver.te \
       mm-qcamerad.te \
       mpdecision.te \
       netd.te \
       netmgrd.te \
       rild.te \
       rmt_storage.te \
       sdcardd.te \
       shell.te \
       system_app.te \
       system_server.te \
       thermal-engine.te \
       ueventd.te \
       vold.te \
       wpa.te
