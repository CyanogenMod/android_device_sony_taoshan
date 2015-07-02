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

# Architecture
TARGET_ARCH := arm
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := krait

BOARD_VENDOR := sony
BOARD_VENDOR_PLATFORM := taoshan
TARGET_BOOTLOADER_BOARD_NAME := qcom
TARGET_BOARD_PLATFORM := msm8960

TARGET_OTA_ASSERT_DEVICE := C2105,C2104,c2105,c2104,taoshan

BOARD_KERNEL_BASE := 0x80200000
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x02000000
# the androidboot.hardware has impact on loading .rc files
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.hardware=qcom androidboot.bootdevice=msm_sdcc.1 user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 maxcpus=2
BOARD_KERNEL_PAGESIZE := 2048
BOARD_FLASH_BLOCK_SIZE := 131072 # (BOARD_KERNEL_PAGESIZE * 64)

TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true

BOARD_BOOTIMAGE_PARTITION_SIZE := 0x108BB9E
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1258291200
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1711276032
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456

TARGET_KERNEL_SOURCE := kernel/sony/msm8930
TARGET_KERNEL_CONFIG := cyanogenmod_taoshan_defconfig

# Board overrides
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

# GPS
BOARD_HAVE_NEW_QC_GPS := true

TARGET_SPECIFIC_HEADER_PATH += device/sony/taoshan/include

# Audio
BOARD_USES_ALSA_AUDIO := true
BOARD_USES_LEGACY_ALSA_AUDIO := true
TARGET_USES_QCOM_COMPRESSED_AUDIO := true
QCOM_FM_ENABLED := true
BOARD_USES_SEPERATED_FM := true
QCOM_USBAUDIO_ENABLED := true

# Camera
TARGET_PROVIDES_CAMERA_HAL := true
USE_DEVICE_SPECIFIC_CAMERA := true

# Display HAL
USE_OPENGL_RENDERER := true
TARGET_USES_ION := true
TARGET_USES_C2D_COMPOSITION := true
OVERRIDE_RS_DRIVER := libRSDriver_adreno.so

# Wlan
BOARD_HAS_QCOM_WLAN              := true
BOARD_WLAN_DEVICE                := qcwcn
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WIFI_DRIVER_FW_PATH_STA          := "sta"
WIFI_DRIVER_FW_PATH_AP           := "ap"

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_QCOM := true
BLUETOOTH_HCI_USE_MCT := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/sony/taoshan/bluetooth

# Lights HAL
TARGET_PROVIDES_LIBLIGHT := true

# Power HAL
TARGET_POWERHAL_VARIANT := qcom
CM_POWERHAL_EXTENSION := qcom

# QCOM hardware
BOARD_USES_QCOM_HARDWARE := true

# Recovery
TARGET_RECOVERY_FSTAB = device/sony/taoshan/rootdir/root/fstab.qcom
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
BOARD_CUSTOM_BOOTIMG := true
BOARD_CUSTOM_BOOTIMG_MK := device/sony/taoshan/custombootimg.mk

# Healthd
BOARD_CHARGER_ENABLE_SUSPEND := true
BOARD_CHARGER_SHOW_PERCENTAGE := true
RED_LED_PATH := /sys/class/leds/led:rgb_red/brightness
GREEN_LED_PATH := /sys/class/leds/led:rgb_green/brightness
BLUE_LED_PATH := /sys/class/leds/led:rgb_blue/brightness

TARGET_POWERHAL_NO_TOUCH_BOOST := true

BOARD_HARDWARE_CLASS := device/sony/taoshan/cmhw

# inherit qcom common sepolicies
include device/qcom/sepolicy/sepolicy.mk

BOARD_SEPOLICY_DIRS += \
    device/sony/taoshan/sepolicy

include vendor/sony/taoshan/BoardConfigVendor.mk
