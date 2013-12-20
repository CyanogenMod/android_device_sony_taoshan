# Screen resoultion in Pixels.
TARGET_SCREEN_HEIGHT := 854
TARGET_SCREEN_WIDTH := 480

# Torch
PRODUCT_PACKAGES := \
    Torch

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Common Sony Resources
$(call inherit-product, device/sony/common/resources.mk)

# Inherit from taoshan device
$(call inherit-product, device/sony/taoshan/taoshan.mk)

# Set those variables here to overwrite the inherited values.
PRODUCT_NAME := full_taoshan
PRODUCT_DEVICE := taoshan
PRODUCT_BRAND := Sony
PRODUCT_MANUFACTURER := Sony
PRODUCT_MODEL := C2105