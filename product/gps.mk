# GPS configurations
PRODUCT_COPY_FILES += \
   $(LOCAL_PATH)/gps/gps.conf:system/etc/gps.conf

# SEC configurations
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:system/etc/sec_config

# GPS properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.gps.agps_provider=1
