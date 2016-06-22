LOCAL_PATH := $(call my-dir)

uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk.cpio
$(uncompressed_ramdisk): $(INSTALLED_RAMDISK_TARGET)
	zcat $< > $@

INITSONY := $(PRODUCT_OUT)/utilities/init_sony

INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img
$(INSTALLED_BOOTIMAGE_TARGET): $(PRODUCT_OUT)/kernel $(uncompressed_ramdisk) $(recovery_uncompressed_ramdisk) $(INSTALLED_RAMDISK_TARGET) $(INITSONY) $(PRODUCT_OUT)/utilities/toybox $(PRODUCT_OUT)/utilities/keycheck $(MKBOOTIMG) $(MINIGZIP) $(INTERNAL_BOOTIMAGE_FILES)
	$(call pretty,"Boot image: $@")

	$(hide) rm -fr $(PRODUCT_OUT)/combinedroot
	$(hide) mkdir -p $(PRODUCT_OUT)/combinedroot/sbin

	$(hide) mv $(PRODUCT_OUT)/root/logo.rle $(PRODUCT_OUT)/combinedroot/logo.rle
	$(hide) cp $(uncompressed_ramdisk) $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(recovery_uncompressed_ramdisk) $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(PRODUCT_OUT)/utilities/keycheck $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(PRODUCT_OUT)/utilities/toybox $(PRODUCT_OUT)/combinedroot/sbin/toybox_init

	$(hide) cp $(INITSONY) $(PRODUCT_OUT)/combinedroot/sbin/init_sony
	$(hide) chmod 755 $(PRODUCT_OUT)/combinedroot/sbin/init_sony
	$(hide) ln -s sbin/init_sony $(PRODUCT_OUT)/combinedroot/init

	$(hide) $(MKBOOTFS) $(PRODUCT_OUT)/combinedroot/ > $(PRODUCT_OUT)/combinedroot.cpio
	$(hide) cat $(PRODUCT_OUT)/combinedroot.cpio | $(MINIGZIP) > $(PRODUCT_OUT)/combinedroot.fs
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/combinedroot.fs --cmdline "$(BOARD_KERNEL_CMDLINE)" --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) $(BOARD_MKBOOTIMG_ARGS) -o $(INSTALLED_BOOTIMAGE_TARGET)

	$(hide) ln -f $(INSTALLED_BOOTIMAGE_TARGET) $(PRODUCT_OUT)/boot.elf

INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) \
	$(INSTALLED_BOOTIMAGE_TARGET) \
	$(recovery_kernel)
	@echo ----- Making recovery image ------
	$(hide) $(MINIGZIP) < $(PRODUCT_OUT)/ramdisk-recovery.cpio > $(PRODUCT_OUT)/ramdisk-recovery.img
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/ramdisk-recovery.img --cmdline "$(BOARD_KERNEL_CMDLINE)" --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) $(BOARD_MKBOOTIMG_ARGS) -o $(INSTALLED_RECOVERYIMAGE_TARGET)
	@echo ----- Made recovery image -------- $@
