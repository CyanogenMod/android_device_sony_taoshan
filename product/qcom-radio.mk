# Telephony permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml

# RIL packages
PRODUCT_PACKAGES += \
    libtime_genoff

# Telephony packages
PRODUCT_PACKAGES += \
    telephony-ext
PRODUCT_BOOT_JARS += \
    telephony-ext

# RIL properties
PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/system/lib/libril-qc-qmi-1.so \
    ro.ril.telephony.mqanelements=6 \
    ro.ril.transmitpower=true

# QMI properties
PRODUCT_PROPERTY_OVERRIDES += \
    com.qc.hardware=true

# Radio properties
PRODUCT_PROPERTY_OVERRIDES += \
    persist.radio.apm_sim_not_pwdn=1 \
    ril.subscription.types=NV,RUIM \
    ro.use_data_netmgrd=true \
    telephony.lteOnCdmaDevice=0

# Telephony properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.call_ring.multiple=0
