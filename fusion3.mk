# Copyright (C) 2013 The CyanogenMod Project
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

TARGET_PROVIDES_ADRENO_DRIVER := true
# inherit from msm8960-common
$(call inherit-product, device/sony/msm8960-common/msm8960.mk)

COMMON_PATH := device/sony/fusion3-common

DEVICE_PACKAGE_OVERLAYS += $(COMMON_PATH)/overlay

ifneq ($(BOARD_HAVE_RADIO),false)
    DEVICE_PACKAGE_OVERLAYS += $(COMMON_PATH)/overlay-radio
    $(call inherit-product, $(COMMON_PATH)/radio.mk)
endif

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.print.xml:system/etc/permissions/android.software.print.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.google.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml

# GPS
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/gps.conf:system/etc/gps.conf

# WPA supplicant config
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/wifi/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    $(COMMON_PATH)/rootdir/system/etc/wifi/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/fstab.qcom:root/fstab.qcom \
    $(COMMON_PATH)/rootdir/fstab.qcom:recovery/root/fstab.qcom \
    $(COMMON_PATH)/rootdir/twrp.fstab:recovery/root/etc/twrp.fstab

# Prima wifi config
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/firmware/wlan/prima/WCNSS_qcom_cfg.ini:system/etc/firmware/wlan/prima/WCNSS_qcom_cfg.ini

# QCOM Display
PRODUCT_PACKAGES += \
    hwcomposer.msm8960 \
    gralloc.msm8960 \
    copybit.msm8960 \
    memtrack.msm8960

# NFC Support
PRODUCT_PACKAGES += \
    libnfc \
    libnfc_jni \
    Nfc \
    Tag \
    com.android.nfc_extras

# Recovery
PRODUCT_PACKAGES += \
    keycheck

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml

# NFCEE access control
ifeq ($(TARGET_BUILD_VARIANT),user)
    NFCEE_ACCESS_PATH := $(COMMON_PATH)/rootdir/system/etc/nfcee_access.xml
else
    NFCEE_ACCESS_PATH := $(COMMON_PATH)/rootdir/system/etc/nfcee_access_debug.xml
endif

PRODUCT_COPY_FILES += \
    $(NFCEE_ACCESS_PATH):system/etc/nfcee_access.xml

PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/kickstart_checker.sh:system/etc/kickstart_checker.sh

# Recovery
PRODUCT_PACKAGES += \
    extract_elf_ramdisk

# Audio
PRODUCT_PACKAGES += \
    alsa.msm8960 \
    audio.primary.msm8960 \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler \
    tinymix

# BT
PRODUCT_PACKAGES += \
    hci_qcomm_init

# Media
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/media_profiles.xml:system/etc/media_profiles.xml

# Sensors
PRODUCT_PACKAGES += \
    sensors.msm8960

# WIFI MAC update
PRODUCT_PACKAGES += \
    mac-update

# FM Radio
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/init.qcom.fm.sh:system/etc/init.qcom.fm.sh

# Key layouts
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/apq8064-tabla-snd-card_Button_Jack.kl:system/usr/keylayout/apq8064-tabla-snd-card_Button_Jack.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/clearpad.kl:system/usr/keylayout/clearpad.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/keypad_8960.kl:system/usr/keylayout/keypad_8960.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/mhl-rcp.kl:system/usr/keylayout/mhl-rcp.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/pmic8xxx_pwrkey.kl:system/usr/keylayout/pmic8xxx_pwrkey.kl

# Misc
PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory

# Live Wallpapers
PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    VisualizationWallpapers \

# Filesystem management tools
PRODUCT_PACKAGES += \
    e2fsck

# Custom init / uevent
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/init.qcom.rc:root/init.qcom.rc \
    $(COMMON_PATH)/rootdir/ueventd.qcom.rc:root/ueventd.qcom.rc \
    $(COMMON_PATH)/rootdir/init.recovery.qcom.rc:root/init.recovery.qcom.rc \
    $(COMMON_PATH)/rootdir/system/etc/init.qcom.bt.sh:system/etc/init.qcom.bt.sh

# Post recovery script
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/recovery/postrecoveryboot.sh:recovery/root/sbin/postrecoveryboot.sh

# Additional sbin stuff
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/charger:root/charger \
    $(COMMON_PATH)/rootdir/sbin/wait4tad_static:root/sbin/wait4tad_static \
    $(COMMON_PATH)/rootdir/system/bin/tad_static:system/bin/tad_static

# Thermal monitor configuration
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/thermald.conf:system/etc/thermald.conf \
    $(COMMON_PATH)/rootdir/system/etc/disable_msm_thermal.sh:system/etc/disable_msm_thermal.sh

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

# GPS
PRODUCT_PROPERTY_OVERRIDES += \
    persist.gps.qmienabled=true

# Audio
PRODUCT_PROPERTY_OVERRIDES += \
    persist.audio.fluence.mode=endfire \
    persist.audio.handset.mic=analog \
    persist.audio.lowlatency.rec=false \
    ro.qc.sdk.audio.fluencetype=none \
    lpa.decode=true

# Bluetooth
PRODUCT_PROPERTY_OVERRIDES += \
    ro.qualcomm.bt.hci_transport=smd

# DRM
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true

# Touchscreen
PRODUCT_PROPERTY_OVERRIDES += \
    ro.input.noresample=1

# OpenGL ES 3.0
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196608

# Include non-opensource parts
$(call inherit-product, vendor/sony/fusion3-common/fusion3-common-vendor.mk)
