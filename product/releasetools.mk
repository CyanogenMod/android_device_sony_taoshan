# Release tools packages
PRODUCT_PACKAGES += \
    resize2fs_static \
    toybox_static \
    utility_e2fsck

# Release tools scripts
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/releasetools/unify_userdata/edify_tools.sh:install/unify_userdata/edify_tools.sh \
    $(COMMON_PATH)/releasetools/unify_userdata/main.sh:install/unify_userdata/main.sh \
    $(COMMON_PATH)/releasetools/unify_userdata/part_tools.sh:install/unify_userdata/part_tools.sh \
    $(COMMON_PATH)/releasetools/unify_userdata/prop_tools.sh:install/unify_userdata/prop_tools.sh \
    $(COMMON_PATH)/releasetools/unify_userdata/variables.sh:install/unify_userdata/variables.sh
