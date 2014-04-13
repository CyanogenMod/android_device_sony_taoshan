# Copyright (C) 2014 The CyanogenMod Project
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
#
#
#

$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit Sony common (and qcom-common) files.
$(call inherit-product, device/sony/common/resources.mk)
$(call inherit-product, device/sony/qcom-common/qcom-common.mk)

# Copy extra files
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:system/etc/permissions/com.nxp.mifare.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:system/etc/permissions/com.android.nfc_extras.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
	
PRODUCT_AAPT_CONFIG := normal hdpi
PRODUCT_AAPT_PREF_CONFIG := hdpi

PRODUCT_PACKAGES += \
    Torch

# IRSC config
PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/rootdir/system/etc/sec_config:system/etc/sec_config
 
# Init scripts and logo 
PRODUCT_COPY_FILES += \
    device/sony/c2105/rootdir/root/init.qcom.rc:root/init.qcom.rc \
    device/sony/c2105/rootdir/root/logo.rle:root/logo.rle \
    device/sony/c2105/rootdir/root/fstab.qcom:root/fstab.qcom \
    device/sony/c2105/rootdir/root/ueventd.qcom.rc:root/ueventd.qcom.rc \
    device/sony/c2105/rootdir/root/init.qcom.usb.rc:root/init.qcom.usb.rc

# Camcorder	
PRODUCT_COPY_FILES += \
    device/sony/c2105/rootdir/system/etc/media_profiles.xml:system/etc/media_profiles.xml \
    device/sony/c2105/rootdir/system/etc/media_codecs.xml:system/etc/media_codecs.xml 
	
# GPS and audio
PRODUCT_COPY_FILES += \
    device/sony/c2105/rootdir/system/etc/gps.conf:system/etc/gps.conf \
    device/sony/c2105/rootdir/system/etc/audio_policy.conf:system/etc/audio_policy.conf \
    device/sony/c2105/rootdir/system/etc/snd_soc_msm/snd_soc_msm_Sitar:system/etc/snd_soc_msm/snd_soc_msm_Sitar
	
# NFC and thermal daemon	
PRODUCT_COPY_FILES += \
    device/sony/c2105/rootdir/system/etc/nfcee_access.xml:system/etc/nfcee_access.xml \
    device/sony/c2105/rootdir/system/etc/thermald.conf:system/etc/thermald.conf 

# Key layouts	
PRODUCT_COPY_FILES += \
    device/sony/c2105/rootdir/system/usr/keychars/pmic8xxx_pwrkey.kcm:system/usr/keychars/pmic8xxx_pwrkey.kcm \
    device/sony/c2105/rootdir/system/usr/keylayout/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    device/sony/c2105/rootdir/system/usr/keylayout/cyttsp3-i2c.kl:system/usr/keylayout/cyttsp3-i2c.kl \
    device/sony/c2105/rootdir/system/usr/keylayout/msm8930-sitar-snd-card_Button_Jack.kl:system/usr/keylayout/msm8930-sitar-snd-card_Button_Jack.kl \
    device/sony/c2105/rootdir/system/usr/keylayout/keypad_8960.kl:system/usr/keylayout/keypad_8960.kl \
    device/sony/c2105/rootdir/system/usr/keylayout/simple_remote_appkey.kl:system/usr/keylayout/simple_remote_appkey.kl \
    device/sony/c2105/rootdir/system/usr/keylayout/pmic8xxx_pwrkey.kl:system/usr/keylayout/pmic8xxx_pwrkey.kl 
	
# Display
PRODUCT_PACKAGES += \
    hwcomposer.msm8960 \
    gralloc.msm8960 \
    copybit.msm8960 \
	libqdMetaData \
    memtrack.msm8960
	
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/rootdir/system/etc/init.qcom.bt.sh:system/etc/init.qcom.bt.sh
	
# Bluetooth
PRODUCT_PROPERTY_OVERRIDES += \
    ro.qualcomm.bt.hci_transport=smd
	
PRODUCT_PROPERTY_OVERRIDES += \	
	 persist.sys.strictmode.visual=0 \
     persist.sys.strictmode.disable=1
	
# BT
PRODUCT_PACKAGES += \
    hci_qcomm_init
	
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/root/sbin/wait4tad_static:root/sbin/wait4tad_static \
    $(LOCAL_PATH)/rootdir/root/sbin/tad_static:root/sbin/tad_static
	
# Audio
PRODUCT_PACKAGES += \
    alsa.msm8960 \
    audio_policy.msm8960 \
    audio.primary.msm8960 \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler \
    tinymix
	
PRODUCT_PACKAGES += libtime_genoff
	
# Miscellaneous
PRODUCT_PACKAGES += \
    librs_jni \
    com.android.future.usb.accessory
	
# Filesystem management tools
PRODUCT_PACKAGES += \
	setup_fs \
    e2fsck
	
# WIFI MAC update
PRODUCT_PACKAGES += \
    mac-update
