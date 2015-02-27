/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ANDROID_HARDWARE_QCAMERA_PARAMETERS_H
#define ANDROID_HARDWARE_QCAMERA_PARAMETERS_H

//#include <utils/KeyedVector.h>
//#include <utils/String8.h>
#include <camera/CameraParameters.h>

namespace android {

struct FPSRange{
    int minFPS;
    int maxFPS;
    FPSRange(){
        minFPS=0;
        maxFPS=0;
    };
    FPSRange(int min,int max){
        minFPS=min;
        maxFPS=max;
    };
};
class QCameraParameters: public CameraParameters
{
public:
#if 1
    QCameraParameters() : CameraParameters() {};
    QCameraParameters(const String8 &params): CameraParameters(params) {};
    #else
    QCameraParameters() : CameraParameters() {};
    QCameraParameters(const String8 &params) { unflatten(params); }
#endif
    ~QCameraParameters();

    // Supported PREVIEW/RECORDING SIZES IN HIGH FRAME RATE recording, sizes in pixels.
    // Example value: "800x480,432x320". Read only.
    static const char KEY_QC_SUPPORTED_HFR_SIZES[];
    // The mode of preview frame rate.
    // Example value: "frame-rate-auto, frame-rate-fixed".
    static const char KEY_QC_PREVIEW_FRAME_RATE_MODE[];
    static const char KEY_QC_SUPPORTED_PREVIEW_FRAME_RATE_MODES[];
    static const char KEY_QC_PREVIEW_FRAME_RATE_AUTO_MODE[];
    static const char KEY_QC_PREVIEW_FRAME_RATE_FIXED_MODE[];

    static const char KEY_QC_SKIN_TONE_ENHANCEMENT[];
    static const char KEY_QC_SUPPORTED_SKIN_TONE_ENHANCEMENT_MODES[];

    //Touch Af/AEC settings.
    static const char KEY_QC_TOUCH_AF_AEC[];
    static const char KEY_QC_SUPPORTED_TOUCH_AF_AEC[];
    //Touch Index for AEC.
    static const char KEY_QC_TOUCH_INDEX_AEC[];
    //Touch Index for AF.
    static const char KEY_QC_TOUCH_INDEX_AF[];
    // Current auto scene detection mode.
    // Example value: "off" or SCENE_DETECT_XXX constants. Read/write.
    static const char KEY_QC_SCENE_DETECT[];
    // Supported auto scene detection settings.
    // Example value: "off,backlight,snow/cloudy". Read only.
    static const char KEY_QC_SUPPORTED_SCENE_DETECT[];
	   // Returns true if video snapshot is supported. That is, applications
    static const char KEY_QC_FULL_VIDEO_SNAP_SUPPORTED[];
    static const char KEY_QC_POWER_MODE_SUPPORTED[];

    static const char KEY_QC_ISO_MODE[];
    static const char KEY_QC_SUPPORTED_ISO_MODES[];
    static const char KEY_QC_LENSSHADE[];
    static const char KEY_QC_SUPPORTED_LENSSHADE_MODES[];

    static const char KEY_QC_AUTO_EXPOSURE[];
    static const char KEY_QC_SUPPORTED_AUTO_EXPOSURE[];

    static const char KEY_QC_GPS_LATITUDE_REF[];
    static const char KEY_QC_GPS_LONGITUDE_REF[];
    static const char KEY_QC_GPS_ALTITUDE_REF[];
    static const char KEY_QC_GPS_STATUS[];
    static const char KEY_QC_EXIF_DATETIME[];
    static const char KEY_QC_MEMORY_COLOR_ENHANCEMENT[];
    static const char KEY_QC_SUPPORTED_MEM_COLOR_ENHANCE_MODES[];


    static const char KEY_QC_POWER_MODE[];
    static const char KEY_QC_VISION_MODE[];
    static const char KEY_QC_VISION_AE[];

    static const char KEY_QC_ZSL[];
    static const char KEY_QC_SUPPORTED_ZSL_MODES[];
    static const char KEY_QC_VIDEO_HDR[];
    static const char KEY_QC_SUPPORTED_VIDEO_HDR_MODES[];

    static const char KEY_QC_CAMERA_MODE[];

    static const char KEY_QC_VIDEO_HIGH_FRAME_RATE[];
    static const char KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES[];
    static const char KEY_QC_HIGH_DYNAMIC_RANGE_IMAGING[];
    static const char KEY_QC_SUPPORTED_HDR_IMAGING_MODES[];
    static const char KEY_QC_AE_BRACKET_HDR[];


    // DENOISE
    static const char KEY_QC_DENOISE[];
    static const char KEY_QC_SUPPORTED_DENOISE[];

    //Selectable zone AF.
    static const char KEY_QC_SELECTABLE_ZONE_AF[];
    static const char KEY_QC_SUPPORTED_SELECTABLE_ZONE_AF[];

    //Face Detection
    static const char KEY_QC_FACE_DETECTION[];
    static const char KEY_QC_SUPPORTED_FACE_DETECTION[];

    // supported camera features to be queried by Snapdragon SDK
    //Read only
    static const char KEY_QC_SUPPORTED_CAMERA_FEATURES[];

    //Indicates number of faces requested by the application.
    //This value will be rejected if the requested faces
    //greater than supported by hardware.
    //Write only.
    static const char KEY_QC_MAX_NUM_REQUESTED_FACES[];

    //Redeye Reduction
    static const char KEY_QC_REDEYE_REDUCTION[];
    static const char KEY_QC_SUPPORTED_REDEYE_REDUCTION[];
    static const char EFFECT_EMBOSS[];
    static const char EFFECT_SKETCH[];
    static const char EFFECT_NEON[];
    static const char EFFECT_USER_DEFINED1[];
    static const char EFFECT_USER_DEFINED2[];
    static const char EFFECT_USER_DEFINED3[];
    static const char EFFECT_USER_DEFINED4[];
    static const char EFFECT_USER_DEFINED5[];
    static const char EFFECT_USER_DEFINED6[];

    // Values for Touch AF/AEC
    static const char TOUCH_AF_AEC_OFF[];
    static const char TOUCH_AF_AEC_ON[];
    static const char SCENE_MODE_ASD[];
    static const char SCENE_MODE_BACKLIGHT[];
    static const char SCENE_MODE_FLOWERS[];
    static const char SCENE_MODE_AR[];
    static const char SCENE_MODE_HDR[];
    static const char SCENE_DETECT_OFF[];
    static const char SCENE_DETECT_ON[];
    static const char PIXEL_FORMAT_YUV420SP_ADRENO[]; // ADRENO
    static const char PIXEL_FORMAT_RAW[];
    static const char PIXEL_FORMAT_YV12[]; // NV12
    static const char PIXEL_FORMAT_NV12[]; //NV12
    // Normal focus mode. Applications should call
    // CameraHardwareInterface.autoFocus to start the focus in this mode.
    static const char FOCUS_MODE_NORMAL[];
    static const char ISO_AUTO[];
    static const char ISO_HJR[];
    static const char ISO_100[];
    static const char ISO_200[];
    static const char ISO_400[];
    static const char ISO_800[];
    static const char ISO_1600[];
    // Values for Lens Shading
    static const char LENSSHADE_ENABLE[];
    static const char LENSSHADE_DISABLE[];

    // Values for auto exposure settings.
    static const char AUTO_EXPOSURE_FRAME_AVG[];
    static const char AUTO_EXPOSURE_CENTER_WEIGHTED[];
    static const char AUTO_EXPOSURE_SPOT_METERING[];

    static const char KEY_QC_SHARPNESS[];
    static const char KEY_QC_MAX_SHARPNESS[];
    static const char KEY_QC_CONTRAST[];
    static const char KEY_QC_MAX_CONTRAST[];
    static const char KEY_QC_SATURATION[];
    static const char KEY_QC_MAX_SATURATION[];

    static const char KEY_QC_HISTOGRAM[];
    static const char KEY_QC_SUPPORTED_HISTOGRAM_MODES[];
    // Values for HISTOGRAM
    static const char HISTOGRAM_ENABLE[];
    static const char HISTOGRAM_DISABLE[];

    // Values for SKIN TONE ENHANCEMENT
    static const char SKIN_TONE_ENHANCEMENT_ENABLE[];
    static const char SKIN_TONE_ENHANCEMENT_DISABLE[];

    // Values for AVTimer
    static const char KEY_QC_AVTIMER[];
    static const char AVTIMER_ENABLE[];
    static const char AVTIMER_DISABLE[];

    //Value for Video Rotation
    static const char KEY_QC_VIDEO_ROTATION[];

   //Value for Crop when using VT usecase
    static const char KEY_QC_VT_CROP[];
    static const char VT_CROP_ENABLE[];
    static const char VT_CROP_DISABLE[];

    // Values for Denoise
    static const char DENOISE_OFF[];
    static const char DENOISE_ON[];

    // Values for auto exposure settings.
    static const char SELECTABLE_ZONE_AF_AUTO[];
    static const char SELECTABLE_ZONE_AF_SPOT_METERING[];
    static const char SELECTABLE_ZONE_AF_CENTER_WEIGHTED[];
    static const char SELECTABLE_ZONE_AF_FRAME_AVERAGE[];

    // Values for Face Detection settings.
    static const char FACE_DETECTION_OFF[];
    static const char FACE_DETECTION_ON[];

    // Values for MCE settings.
    static const char MCE_ENABLE[];
    static const char MCE_DISABLE[];

    // Values for ZSL settings.
    static const char ZSL_OFF[];
    static const char ZSL_ON[];

    static const char VIDEO_HDR_OFF[];
    static const char VIDEO_HDR_ON[];
    // Values for HDR Bracketing settings.
    static const char AE_BRACKET_HDR_OFF[];
    static const char AE_BRACKET_HDR[];
    static const char AE_BRACKET[];

    // Values for Power mode settings.
    static const char LOW_POWER[];
    static const char NORMAL_POWER[];

    // Values for HFR settings.
    static const char VIDEO_HFR_OFF[];
    static const char VIDEO_HFR_2X[];
    static const char VIDEO_HFR_3X[];
    static const char VIDEO_HFR_4X[];

    // Values for Redeye Reduction settings.
    static const char REDEYE_REDUCTION_ENABLE[];
    static const char REDEYE_REDUCTION_DISABLE[];
    // Values for HDR settings.
    static const char HDR_ENABLE[];
    static const char HDR_DISABLE[];

   // Values for Redeye Reduction settings.
   // static const char REDEYE_REDUCTION_ENABLE[];
   // static const char REDEYE_REDUCTION_DISABLE[];
   // Values for HDR settings.
   //    static const char HDR_ENABLE[];
   //    static const char HDR_DISABLE[];


   static const char KEY_QC_SINGLE_ISP_OUTPUT_ENABLED[];

   static const char KEY_QC_VIDEO_FRAME_FLIP[];
   static const char KEY_QC_PREVIEW_FRAME_FLIP[];
   static const char KEY_QC_SNAPSHOT_FRAME_FLIP[];
   static const char KEY_QC_SNAPSHOT_FRAME_LEN[];
   static const char KEY_QC_SNAPSHOT_FRAME_Y_OFFSET[];
   static const char KEY_QC_SNAPSHOT_FRAME_CBCR_OFFSET[];
   static const char KEY_QC_SNAPSHOT_FRAME_CAM_FORMAT[];
   static const int  MAX_STR_LENGTH;
    enum {
        CAMERA_ORIENTATION_UNKNOWN = 0,
        CAMERA_ORIENTATION_PORTRAIT = 1,
        CAMERA_ORIENTATION_LANDSCAPE = 2,
    };
    int getOrientation() const;
    void setOrientation(int orientation);
    void getSupportedHfrSizes(Vector<Size> &sizes) const;
    void setPreviewFpsRange(int minFPS,int maxFPS);
	void setPreviewFrameRateMode(const char *mode);
    const char *getPreviewFrameRateMode() const;
    void setTouchIndexAec(int x, int y);
    void getTouchIndexAec(int *x, int *y) const;
    void setTouchIndexAf(int x, int y);
    void getTouchIndexAf(int *x, int *y) const;
    void getMeteringAreaCenter(int * x, int *y) const;

};

}; // namespace android
#endif
