# Camera packages
PRODUCT_PACKAGES += \
    camera.qcom \
    libstlport

# Camera SHIM packages
PRODUCT_PACKAGES += \
    libshim_qcopt

# Camera properties
PRODUCT_PROPERTY_OVERRIDES += \
    camera2.portability.force_api=1
