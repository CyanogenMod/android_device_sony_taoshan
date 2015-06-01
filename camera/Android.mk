ifneq ($(USE_CAMERA_STUB),true)
    # When zero we link against libmmcamera; when 1, we dlopen libmmcamera.
    DLOPEN_LIBMMCAMERA:=1
    ifneq ($(BUILD_TINY_ANDROID),true)
      V4L2_BASED_LIBCAM := false
      MM_STILL_V4L2_DRIVER_LIST := msm7627a
      #MM_STILL_V4L2_DRIVER_LIST += msm7630_surf
      #MM_STILL_V4L2_DRIVER_LIST += msm7630_fusion
      MM_STILL_V4L2_DRIVER_LIST += msm8660
      MM_STILL_V4L2_DRIVER_LIST += msm8960
      MM_STILL_V4L2_DRIVER_LIST += msm8974
   ifneq (,$(filter $(MM_STILL_V4L2_DRIVER_LIST),$(TARGET_BOARD_PLATFORM)))
        V4L2_BASED_LIBCAM := true
      endif

      LOCAL_PATH:= $(call my-dir)
      LOCAL_PATH1:= $(call my-dir)

      include $(CLEAR_VARS)

      LOCAL_CFLAGS:= -DDLOPEN_LIBMMCAMERA=$(DLOPEN_LIBMMCAMERA)

      #define BUILD_UNIFIED_CODE
      ifeq ($(TARGET_BOARD_PLATFORM),msm7627a)
        BUILD_UNIFIED_CODE := true
      else
        BUILD_UNIFIED_CODE := false
      endif

      ifeq ($(TARGET_BOARD_PLATFORM),msm7627a)
        LOCAL_CFLAGS+= -DVFE_7X27A
      endif

      ifeq ($(strip $(TARGET_USES_ION)),true)
        LOCAL_CFLAGS += -DUSE_ION
      endif

      LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID # 8660=SMI, Rest=EBI
      LOCAL_CFLAGS += -DCAMERA_ZSL_ION_HEAP_ID=ION_CP_MM_HEAP_ID
      ifeq ($(TARGET_BOARD_PLATFORM),msm8974)
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_MM_HEAP
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
        LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=0
        LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=9
      else ifeq ($(TARGET_BOARD_PLATFORM),msm8960)
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_MM_HEAP
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
        LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=0
        LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=5
      else ifneq (,$(filter msm8660,$(TARGET_BOARD_PLATFORM)))
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP # Don't Care
        LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID # EBI
        LOCAL_CFLAGS += -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=0
        LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=5
      else
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP # Don't Care
        LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=GRALLOC_USAGE_PRIVATE_UNCACHED #uncached
        LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID
        LOCAL_CFLAGS += -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID
        LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=5
      endif

      ifeq ($(V4L2_BASED_LIBCAM),true)

      #  Enable this after fixing all warnings
      #  LOCAL_CFLAGS+= -Werror
        ifeq ($(TARGET_BOARD_PLATFORM),msm7627a)
          LOCAL_HAL_FILES := QCameraHAL.cpp QCameraHWI_Parm.cpp\
            QCameraHWI.cpp QCameraHWI_Preview.cpp \
            QCameraHWI_Record_7x27A.cpp QCameraHWI_Still.cpp \
            QCameraHWI_Mem.cpp QCameraHWI_Display.cpp \
            QCameraStream.cpp QualcommCamera2.cpp QCameraParameters.cpp
        else
          LOCAL_HAL_FILES := QCameraHAL.cpp QCameraHWI_Parm.cpp\
            QCameraHWI.cpp QCameraHWI_Preview.cpp \
            QCameraHWI_Record.cpp QCameraHWI_Still.cpp \
            QCameraHWI_Mem.cpp QCameraHWI_Display.cpp \
            QCameraStream.cpp QualcommCamera2.cpp QCameraParameters.cpp
        endif

      else
        LOCAL_HAL_FILES := QualcommCamera.cpp QualcommCameraHardware.cpp QCameraParameters.cpp
      endif

      LOCAL_CFLAGS+= -DHW_ENCODE

      LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
      LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

      # if debug service layer and up , use stub camera!
      LOCAL_C_INCLUDES += \
        frameworks/base/services/camera/libcameraservice #

      LOCAL_SRC_FILES := $(MM_CAM_FILES) $(LOCAL_HAL_FILES)

      ifneq (,$(filter msm7627,$(TARGET_BOARD_PLATFORM)))
        LOCAL_CFLAGS+= -DNUM_PREVIEW_BUFFERS=6 -D_ANDROID_
      else
        LOCAL_CFLAGS+= -DNUM_PREVIEW_BUFFERS=4 -D_ANDROID_
      endif

      # To Choose neon/C routines for YV12 conversion
      LOCAL_CFLAGS+= -DUSE_NEON_CONVERSION
      # Uncomment below line to enable smooth zoom
      #LOCAL_CFLAGS+= -DCAMERA_SMOOTH_ZOOM

      # Executed only on QCOM BSPs
      ifeq ($(TARGET_USES_QCOM_BSP),true)
      # This flag is used to compile out any features that depend on framework changes
        LOCAL_CFLAGS += -DQCOM_BSP
      endif

       LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/mm-camera \
        $(TARGET_OUT_HEADERS)/mm-camera/common \
        $(TARGET_OUT_HEADERS)/mm-still \
        $(TARGET_OUT_HEADERS)/mm-still/jpeg \

      ifeq ($(V4L2_BASED_LIBCAM),true)
        LOCAL_C_INCLUDES+= $(call project-path-for,qcom-media)/mm-core/inc
        LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/mm-still/mm-omx
        LOCAL_C_INCLUDES+= $(LOCAL_PATH)/mm-camera-interface
      else
        LOCAL_C_INCLUDES+= $(call project-path-for,qcom-media)/libgenlock
      endif

      LOCAL_C_INCLUDES+= $(call project-path-for,qcom-display)/libgralloc
      LOCAL_C_INCLUDES+= $(call project-path-for,qcom-media)/libstagefrighthw

      ifeq ($(V4L2_BASED_LIBCAM),true)
        LOCAL_SHARED_LIBRARIES:= libutils libui libcamera_client liblog libcutils libmmjpeg libmmstillomx libimage-jpeg-enc-omx-comp
      else
         LOCAL_SHARED_LIBRARIES:= libutils libui libcamera_client liblog libcutils libmmjpeg libgenlock
      endif

      LOCAL_SHARED_LIBRARIES+= libbinder libhardware
      ifneq ($(DLOPEN_LIBMMCAMERA),1)
        LOCAL_SHARED_LIBRARIES+= liboemcamera
      else
        LOCAL_SHARED_LIBRARIES+= libdl
      endif

      ifneq ($(call is-platform-sdk-version-at-least,20),true)
      LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
      endif

      LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
      LOCAL_MODULE:= camera_1.0.$(TARGET_BOARD_PLATFORM)
      LOCAL_MODULE_TAGS := optional
      include $(BUILD_SHARED_LIBRARY)

    endif # BUILD_TINY_ANDROID
endif # USE_CAMERA_STUB

#Enable only to compile new interafece and HAL files.
ifeq ($(V4L2_BASED_LIBCAM),true)
include $(LOCAL_PATH1)/QCamera/Android.mk
endif
