/*
** Copyright 2008, Google Inc.
** Copyright (c) 2009-2012, The Linux Foundation. All rights reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_HARDWARE_QCAMERA_STREAM_H
#define ANDROID_HARDWARE_QCAMERA_STREAM_H


#include <utils/threads.h>

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/threads.h>

#include "QCameraHWI.h"
#include "QCameraHWI_Mem.h"
#include "QCamera_Intf.h"
extern "C" {
#include <mm_camera_interface2.h>

#define DEFAULT_STREAM_WIDTH 320
#define DEFAULT_STREAM_HEIGHT 240
#define DEFAULT_LIVESHOT_WIDTH 2592
#define DEFAULT_LIVESHOT_HEIGHT 1944

#define MM_CAMERA_CH_PREVIEW_MASK    (0x01 << MM_CAMERA_CH_PREVIEW)
#define MM_CAMERA_CH_VIDEO_MASK      (0x01 << MM_CAMERA_CH_VIDEO)
#define MM_CAMERA_CH_SNAPSHOT_MASK   (0x01 << MM_CAMERA_CH_SNAPSHOT)

} /* extern C*/



typedef struct snap_hdr_record_t_ {
  bool hdr_on;
  int  num_frame;
  int  num_raw_received;

  /*in terms of 2^(n/6), e.g -6 means (1/2)x, while 12 is 4x*/
  int  exp[MAX_HDR_EXP_FRAME_NUM];
  mm_camera_ch_data_buf_t *recvd_frame[MAX_HDR_EXP_FRAME_NUM];
} snap_hdr_record_t;


namespace android {

class QCameraHardwareInterface;

class StreamQueue {
private:
    Mutex mQueueLock;
    Condition mQueueWait;
    bool mInitialized;

    //Vector<struct msm_frame *> mContainer;
    Vector<void *> mContainer;
public:
    StreamQueue();
    virtual ~StreamQueue();
    bool enqueue(void *element);
    void flush();
    void* dequeue();
    void init();
    void deinit();
    bool isInitialized();
bool isEmpty();
};


class QCameraStream { //: public virtual RefBase{

public:
    bool mInit;
    bool mActive;

    virtual status_t    init();
    virtual status_t    start();
    virtual void        stop();
    virtual void        release();

    status_t setFormat(uint8_t ch_type_mask, cam_format_t previewFmt);
    status_t setMode(int enable);

    virtual void        setHALCameraControl(QCameraHardwareInterface* ctrl);

    //static status_t     openChannel(mm_camera_t *, mm_camera_channel_type_t ch_type);
    virtual status_t    initChannel(int cameraId, uint32_t ch_type_mask);
    virtual status_t    deinitChannel(int cameraId, mm_camera_channel_type_t ch_type);
    virtual void releaseRecordingFrame(const void *opaque)
    {
      ;
    }
#if 0 // mzhu
    virtual status_t getBufferInfo(sp<IMemory>& Frame, size_t *alignedSize)
    {
      return NO_ERROR;
    }
#endif // mzhu
    virtual void prepareHardware()
    {
      ;
    }
    virtual sp<IMemoryHeap> getHeap() const{return NULL;}
    virtual status_t    initDisplayBuffers(){return NO_ERROR;}
    virtual status_t initPreviewOnlyBuffers(){return NO_ERROR;}
    virtual sp<IMemoryHeap> getRawHeap() const {return NULL;}
    virtual void *getLastQueuedFrame(void){return NULL;}
    virtual status_t takePictureZSL(void){return NO_ERROR;}
    virtual status_t takeLiveSnapshot(){return NO_ERROR;}
    virtual status_t takePictureLiveshot(mm_camera_ch_data_buf_t* recvd_frame,
                                 cam_ctrl_dimension_t *dim,
                                 int frame_len){return NO_ERROR;}
	virtual void setModeLiveSnapshot(bool){;}
    virtual status_t initSnapshotBuffers(cam_ctrl_dimension_t *dim,
                                 int num_of_buf){return NO_ERROR;}

    virtual void setFullSizeLiveshot(bool){};
    /* Set the ANativeWindow */
    virtual int setPreviewWindow(preview_stream_ops_t* window) {return NO_ERROR;}
    virtual void notifyROIEvent(fd_roi_t roi) {;}
    virtual void notifyWDenoiseEvent(cam_ctrl_status_t status, void * cookie) {};
    virtual void resetSnapshotCounters(void ){};
    virtual void InitHdrInfoForSnapshot(bool HDR_on, int number_frames, int *exp ) {};
    virtual void notifyHdrEvent(cam_ctrl_status_t status, void * cookie) {};

    QCameraStream();
    QCameraStream(int, camera_mode_t);
    virtual             ~QCameraStream();
    QCameraHardwareInterface*  mHalCamCtrl;
    mm_camera_ch_crop_t mCrop;

    int mCameraId;
    camera_mode_t myMode;

    mutable Mutex mStopCallbackLock;
    int     mSnapshotDataCallingBack;
    int     mFreeSnapshotBufAfterDataCb;
private:
   StreamQueue mBusyQueue;
   StreamQueue mFreeQueue;
public:
     friend void liveshot_callback(mm_camera_ch_data_buf_t *frame,void *user_data);
};

/*
*   Record Class
*/
class QCameraStream_record : public QCameraStream {
public:
  status_t    init();
  status_t    start() ;
  void        stop()  ;
  void        release() ;

  static QCameraStream*  createInstance(int cameraId, camera_mode_t);
  static void            deleteInstance(QCameraStream *p);

  QCameraStream_record() {};
  virtual             ~QCameraStream_record();

  status_t processRecordFrame(void *data);
  status_t initEncodeBuffers();
  status_t getBufferInfo(sp<IMemory>& Frame, size_t *alignedSize);
  //sp<IMemoryHeap> getHeap() const;

  void releaseRecordingFrame(const void *opaque);
  void debugShowVideoFPS() const;

  status_t takeLiveSnapshot();
private:
  QCameraStream_record(int, camera_mode_t);
  void releaseEncodeBuffer();

  cam_ctrl_dimension_t             dim;
  bool mDebugFps;

  mm_camera_reg_buf_t              mRecordBuf;
  //int                              record_frame_len;
  //static const int                 maxFrameCnt = 16;
  //camera_memory_t                 *mCameraMemoryPtr[maxFrameCnt];
  //int                              mNumRecordFrames;
  //sp<PmemPool>                     mRecordHeap[maxFrameCnt];
  struct msm_frame                *recordframes;
  //uint32_t                         record_offset[VIDEO_BUFFER_COUNT];
  mm_camera_ch_data_buf_t          mRecordedFrames[MM_CAMERA_MAX_NUM_FRAMES];
  //Mutex                            mRecordFreeQueueLock;
  //Vector<mm_camera_ch_data_buf_t>  mRecordFreeQueue;

  int mJpegMaxSize;
  QCameraStream *mStreamSnap;

};

class QCameraStream_preview : public QCameraStream {
public:
    status_t    init();
    status_t    start() ;
    void        stop()  ;
    void        release() ;

    static QCameraStream*  createInstance(int, camera_mode_t);
    static void            deleteInstance(QCameraStream *p);

    QCameraStream_preview() {};
    virtual             ~QCameraStream_preview();
    void *getLastQueuedFrame(void);
    /*init preview buffers with display case*/
    status_t initDisplayBuffers();
    /*init preview buffers without display case*/
    status_t initPreviewOnlyBuffers();

    status_t processPreviewFrame(mm_camera_ch_data_buf_t *frame);

    /*init preview buffers with display case*/
    status_t processPreviewFrameWithDisplay(mm_camera_ch_data_buf_t *frame);
    /*init preview buffers without display case*/
    status_t processPreviewFrameWithOutDisplay(mm_camera_ch_data_buf_t *frame);

    int setPreviewWindow(preview_stream_ops_t* window);
    void notifyROIEvent(fd_roi_t roi);
    friend class QCameraHardwareInterface;

private:
    QCameraStream_preview(int cameraId, camera_mode_t);
    /*allocate and free buffers with display case*/
    status_t                 getBufferFromSurface();
    status_t                 putBufferToSurface();

    /*allocate and free buffers without display case*/
    status_t                 getBufferNoDisplay();
    status_t                 freeBufferNoDisplay();

    void                     dumpFrameToFile(struct msm_frame* newFrame);
    bool                     mFirstFrameRcvd;

    int8_t                   my_id;
    mm_camera_op_mode_type_t op_mode;
    cam_ctrl_dimension_t     dim;
    struct msm_frame        *mLastQueuedFrame;
    mm_camera_reg_buf_t      mDisplayBuf;
    mm_cameara_stream_buf_t  mDisplayStreamBuf;
    Mutex                   mDisplayLock;
    preview_stream_ops_t   *mPreviewWindow;
    static const int        kPreviewBufferCount = PREVIEW_BUFFER_COUNT;
    mm_camera_ch_data_buf_t mNotifyBuffer[16];
    int8_t                  mNumFDRcvd;
    int                     mVFEOutputs;
    int                     mHFRFrameCnt;
    int                     mHFRFrameSkip;
};

/* Snapshot Class - handle data flow*/
class QCameraStream_Snapshot : public QCameraStream {
public:
    status_t    init();
    status_t    start();
    void        stop();
    void        release();
    void        prepareHardware();
    static QCameraStream* createInstance(int cameraId, camera_mode_t);
    static void deleteInstance(QCameraStream *p);

    status_t takePictureZSL(void);
    status_t takePictureLiveshot(mm_camera_ch_data_buf_t* recvd_frame,
                                 cam_ctrl_dimension_t *dim,
                                 int frame_len);
    status_t receiveRawPicture(mm_camera_ch_data_buf_t* recvd_frame);
    void receiveCompleteJpegPicture(jpeg_event_t event);
	void jpegErrorHandler(jpeg_event_t event);
    void receiveJpegFragment(uint8_t *ptr, uint32_t size);
    void deInitBuffer(void);
    sp<IMemoryHeap> getRawHeap() const;
    int getSnapshotState();
    /*Temp: to be removed once event handling is enabled in mm-camera*/
    void runSnapshotThread(void *data);
    bool isZSLMode();
    void setFullSizeLiveshot(bool);
    void notifyWDenoiseEvent(cam_ctrl_status_t status, void * cookie);
    friend void liveshot_callback(mm_camera_ch_data_buf_t *frame,void *user_data);
    void resetSnapshotCounters(void );
    void InitHdrInfoForSnapshot(bool HDR_on, int number_frames, int *exp );
    void notifyHdrEvent(cam_ctrl_status_t status, void * cookie);

private:
    QCameraStream_Snapshot(int, camera_mode_t);
    virtual ~QCameraStream_Snapshot();

    /* snapshot related private members */
    status_t initJPEGSnapshot(int num_of_snapshots);
    status_t initRawSnapshot(int num_of_snapshots);
    status_t initZSLSnapshot(void);
    status_t initFullLiveshot(void);
	status_t cancelPicture();
    void notifyShutter(common_crop_t *crop,
                       bool play_shutter_sound);
    status_t initSnapshotBuffers(cam_ctrl_dimension_t *dim,
                                 int num_of_buf);
    status_t initRawSnapshotBuffers(cam_ctrl_dimension_t *dim,
                                    int num_of_buf);
    status_t deinitRawSnapshotBuffers(void);
    status_t deinitSnapshotBuffers(void);
    status_t initRawSnapshotChannel(cam_ctrl_dimension_t* dim,
                                    int num_snapshots);
    status_t initSnapshotFormat(cam_ctrl_dimension_t *dim);
    status_t takePictureRaw(void);
    status_t takePictureJPEG(void);
    status_t startStreamZSL(void);
    void deinitSnapshotChannel(mm_camera_channel_type_t);
    status_t configSnapshotDimension(cam_ctrl_dimension_t* dim);
    status_t encodeData(mm_camera_ch_data_buf_t* recvd_frame,
                        common_crop_t *crop_info,
                        int frame_len,
                        bool enqueued);
    status_t encodeDisplayAndSave(mm_camera_ch_data_buf_t* recvd_frame,
                                  bool enqueued);
    status_t setZSLChannelAttribute(void);
    void handleError();
    void setSnapshotState(int state);
    void setModeLiveSnapshot(bool);
    bool isLiveSnapshot(void);
    void stopPolling(void);
    bool isFullSizeLiveshot(void);
    status_t doWaveletDenoise(mm_camera_ch_data_buf_t* frame);
    status_t sendWDenoiseStartMsg(mm_camera_ch_data_buf_t * frame);
    void lauchNextWDenoiseFromQueue();
    status_t doHdrProcessing( );

    /* Member variables */

    int mSnapshotFormat;
    int mPictureWidth;
    int mPictureHeight;
    cam_format_t mPictureFormat;
    int mPostviewWidth;
    int mPostviewHeight;
    int mThumbnailWidth;
    int mThumbnailHeight;
    cam_format_t mThumbnailFormat;
	int mJpegOffset;
    int mSnapshotState;
    int mNumOfSnapshot;
	int mNumOfRecievedJPEG;
    bool mModeLiveSnapshot;
    bool mBurstModeFlag;
	int mActualPictureWidth;
    int mActualPictureHeight;
    bool mJpegDownscaling;
    sp<AshmemPool> mJpegHeap;
    /*TBD:Bikas: This is defined in HWI too.*/
#ifdef USE_ION
    sp<IonPool>  mDisplayHeap;
    sp<IonPool>  mPostviewHeap;
#else
    sp<PmemPool>  mDisplayHeap;
    sp<PmemPool>  mPostviewHeap;
#endif
    mm_camera_ch_data_buf_t *mCurrentFrameEncoded;
    mm_cameara_stream_buf_t mSnapshotStreamBuf;
    mm_cameara_stream_buf_t mPostviewStreamBuf;
    StreamQueue             mSnapshotQueue;
    static const int        mMaxSnapshotBufferCount = 16;
    int                     mSnapshotBufferNum;
    int                     mMainfd[mMaxSnapshotBufferCount];
    int                     mThumbfd[mMaxSnapshotBufferCount];
    int                     mMainSize;
    int                     mThumbSize;
	camera_memory_t        *mCameraMemoryPtrMain[mMaxSnapshotBufferCount];
	camera_memory_t        *mCameraMemoryPtrThumb[mMaxSnapshotBufferCount];
    int                     mJpegSessionId;
	int                     dump_fd;
    bool mFullLiveshot;
    StreamQueue             mWDNQueue; // queue to hold frames while one frame is sent out for WDN
    bool                    mIsDoingWDN; // flag to indicate if WDN is going on (one frame is sent out for WDN)
	bool                    mDropThumbnail;
	int                     mJpegQuality;

    bool                    mIsRawChAcquired;
    bool                    mIsJpegChAcquired;
    snap_hdr_record_t       mHdrInfo;
}; // QCameraStream_Snapshot


}; // namespace android

#endif
