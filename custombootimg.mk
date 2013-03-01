LOCAL_PATH := $(call my-dir)

INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img
$(INSTALLED_BOOTIMAGE_TARGET): $(TARGET_PREBUILT_KERNEL) $(recovery_ramdisk) $(INSTALLED_RAMDISK_TARGET) $(PRODUCT_OUT)/utilities/flash_image $(PRODUCT_OUT)/utilities/busybox $(PRODUCT_OUT)/utilities/erase_image $(MKBOOTIMG) $(MINIGZIP) $(INTERNAL_BOOTIMAGE_FILES)
	$(call pretty,"Boot image: $@")
	$(hide) mkdir -p $(PRODUCT_OUT)/combinedroot/
	$(hide) device/semc/es209ra/releasetools/prepare_kernel $(TARGET_KERNEL_SOURCE)/arch/arm/configs/$(TARGET_KERNEL_CONFIG) $(PRODUCT_OUT) $(PREBUILT_FOLDER)
	$(hide) cp -r $(PRODUCT_OUT)/root/* $(PRODUCT_OUT)/combinedroot/
	$(hide) cp -r $(PRODUCT_OUT)/recovery/root/sbin/* $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) sed -i 's/ro.adb.secure=1//g' $(PRODUCT_OUT)/combinedroot/default.prop
	$(hide) $(MKBOOTFS) $(PRODUCT_OUT)/combinedroot/ > $(PRODUCT_OUT)/combinedroot.cpio
	$(hide) cat $(PRODUCT_OUT)/combinedroot.cpio | $(MINIGZIP) > $(PRODUCT_OUT)/combinedroot.fs
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/combinedroot.fs --base $(BOARD_KERNEL_BASE) --output $@
	$(hide) device/semc/es209ra/releasetools/bin2elf 2 0x20008000 $(PRODUCT_OUT)/kernel 0x20008000 0x0 $(PRODUCT_OUT)/combinedroot.fs 0x24000000 0x80000000
	$(hide) mv result.elf $(PRODUCT_OUT)/result.elf
	$(hide) device/semc/es209ra/releasetools/bin2sin $(PRODUCT_OUT)/result.elf 03000000220000007502000062000000
	$(hide) mv $(PRODUCT_OUT)/result.elf.sin $(PRODUCT_OUT)/kernel.sin
	$(hide) rm $(PRODUCT_OUT)/result.elf
	$(hide) cp $(TARGET_KERNEL_LOADER) $(PRODUCT_OUT)/loader.sin
	$(hide) device/semc/es209ra/releasetools/create_ftf $(PRODUCT_OUT)

INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) \
	$(recovery_ramdisk) \
	$(recovery_kernel)
	@echo ----- Making recovery image ------
	$(MKBOOTIMG) $(INTERNAL_RECOVERYIMAGE_ARGS) --output $@
	@echo ----- Made recovery image -------- $@
	$(hide) $(call assert-max-image-size,$@,$(BOARD_RECOVERYIMAGE_PARTITION_SIZE),raw)

