/*
** Copyright (c) 2011-2013 The Linux Foundation. All rights reserved.
**
** Not a Contribution, Apache license notifications and license are retained
** for attribution purposes only.
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

/*#error uncomment this for compiler test!*/

#define LOG_TAG "QCameraHWI_Rdi"
#include <utils/Log.h>
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "QCameraHAL.h"
#include "QCameraHWI.h"
#include <gralloc_priv.h>

#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)
#define LIKELY(exp) __builtin_expect(!!(exp), 1)

/* QCameraHWI_Raw class implementation goes here*/
/* following code implement the RDI logic of this class*/

namespace android {

status_t QCameraStream_Rdi::updatePreviewMetadata(const void *yuvMetadata)
{
    memset(mHalCamCtrl->mFace, 0, sizeof(mHalCamCtrl->mFace));
    mHalCamCtrl->mMetadata.faces = mHalCamCtrl->mFace;
    mHalCamCtrl->mMetadata.number_of_faces = 0;

    //TODO: Fill out face information based on yuvMetadata
    camera_data_callback pcb = mHalCamCtrl->mDataCb;

    if (pcb && (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_METADATA)) {
        ALOGE("%s: Face detection RIO callback", __func__);
        pcb(CAMERA_MSG_PREVIEW_METADATA, NULL, 0,
            &mHalCamCtrl->mMetadata, mHalCamCtrl->mCallbackCookie);
    }

    return NO_ERROR;
}

uint32_t QCameraStream_Rdi::swapByteEndian(uint32_t jpegLength)
{
    jpegLength = (jpegLength >> 24) |
                 ((jpegLength << 8) & 0x00FF0000) |
                 ((jpegLength >> 8) & 0x0000FF00) |
                 (jpegLength << 24);
    return jpegLength;

}

status_t QCameraStream_Rdi::processJpegData(const void *jpegInfo, const void *jpegData,
                                            jpeg_info_t * jpeg_info, mm_camera_super_buf_t *jpeg_buf)
{
    status_t ret = NO_ERROR;
    uint8_t * NV12_meta = (uint8_t *)jpegInfo+0x800; //YUV meta offset 2048 bytes
    jpegInfo = (uint8_t *)jpegInfo+0xF; //First 16 bytes JPEG INFO start marker
    uint8_t jpegMode = *((uint8_t *)jpegInfo); // 1st byte 0x0
    uint8_t jpegCount = *((uint8_t *)jpegInfo + 1); //2nd byte 0x1
    uint32_t jpegLength = *((uint32_t *)jpegInfo + 1); // 2 reserved then 4 bytes 0x4
    uint32_t jpegFrameId = *((uint32_t *)jpegInfo + 2); // 4 bytes reserved 0x8
    uint32_t yuvFrameId = *((uint32_t *)(NV12_meta+0xE)); // 16 bytes of YUV meta start marker

    jpegLength = swapByteEndian(jpegLength);
    jpegFrameId = swapByteEndian(jpegFrameId);
    yuvFrameId = swapByteEndian(yuvFrameId);
    ALOGE("jpegMode %d jpegCount %x jpegLength %d, jpegFrameId %d yuvFrameId %d",
          jpegMode, jpegCount, (int)jpegLength, (int)jpegFrameId, (int)yuvFrameId);

    jpeg_info->NV12_meta = NV12_meta;
    jpeg_info->jpegInfo = (uint8_t *)jpegInfo;
    jpeg_info->jpegMode = jpegMode;
    jpeg_info->jpegCount = jpegCount;
    jpeg_info->jpegLength = jpegLength;
    jpeg_info->jpegFrameId = jpegFrameId;
    jpeg_info->yuvFrameId = yuvFrameId;

    if (jpegMode == META_MODE) {
        /* Meta  Data parsing */
    } else if (jpegMode == NORMAL_JPEG_MODE) {
        if(mJpegSuperBuf != NULL){
            free(mJpegSuperBuf);
            mJpegSuperBuf = NULL;
        }
        mJpegSuperBuf = (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
        memcpy(mJpegSuperBuf, jpeg_buf, sizeof(mm_camera_super_buf_t));
        mJpegSuperBuf->split_jpeg = 0;
        *((uint32_t *)jpegInfo + 1) = jpegLength;
        ret = JPEG_RECEIVED;
    } else if (jpegMode == SPLIT_JPEG_MODE) {
        // Meta + splitted JPEG_1 or JPEG_2
        ALOGD("%s: split jpeg case: jpegCount %x jpegLength %d, yuvFrameId %d ",
            __func__, jpegCount, (int)jpegLength, (int)yuvFrameId);
        // if this is the first segment, alloc 16MB and copy, return JPEG_PENDING
        if (jpegCount == 0) {
           if(mJpegSuperBuf != NULL) {
               free(mJpegSuperBuf);
               mJpegSuperBuf = NULL;
           }
           mJpegSuperBuf = (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
           mJpegSuperBuf->camera_handle = jpeg_buf->camera_handle;
           mJpegSuperBuf->ch_id = jpeg_buf->ch_id;
           mJpegSuperBuf->num_bufs = jpeg_buf->num_bufs;
           mJpegSuperBuf->bufs[0] = (mm_camera_buf_def_t*)malloc(sizeof(mm_camera_buf_def_t));
           mJpegSuperBuf->bufs[0]->buf_idx = jpeg_buf->bufs[0]->buf_idx;
           mJpegSuperBuf->bufs[0]->fd = jpeg_buf->bufs[0]->fd;
           mJpegSuperBuf->bufs[0]->frame_idx = jpeg_buf->bufs[0]->frame_idx;
           mJpegSuperBuf->bufs[0]->frame_len = jpeg_buf->bufs[0]->frame_len;
           mJpegSuperBuf->bufs[0]->mem_info = jpeg_buf->bufs[0]->mem_info;
           mJpegSuperBuf->bufs[0]->num_planes = jpeg_buf->bufs[0]->num_planes;
           memcpy(mJpegSuperBuf->bufs[0]->planes, jpeg_buf->bufs[0]->planes, sizeof(struct v4l2_plane *));
           mJpegSuperBuf->bufs[0]->stream_id = jpeg_buf->bufs[0]->stream_id;
           mJpegSuperBuf->bufs[0]->ts = jpeg_buf->bufs[0]->ts;
           mJpegSuperBuf->split_jpeg = 1;
           mJpegSuperBuf->bufs[0]->buffer = (void *)malloc(COMBINED_BUF_SIZE);
           mJpegSuperBuf->bufs[0]->frame_len = COMBINED_BUF_SIZE;
           memcpy(mJpegSuperBuf->bufs[0]->buffer, jpeg_buf->bufs[0]->buffer, JPEG_DATA_OFFSET);
           memcpy((uint8_t*)(mJpegSuperBuf->bufs[0]->buffer) + JPEG_DATA_OFFSET, jpegData, jpegLength);

           //need to qbuf the old 8MB buffer
           //Since status is JPEG_PENDING, qbuf will be called 
           //at the end of processRdiFrame()
           ret = JPEG_PENDING;
        } else if (jpegCount == 1) {
           // if this is second segemnt, copy after first, return JPEG_RECEIVED
           memcpy(((uint8_t*)mJpegSuperBuf->bufs[0]->buffer) + FIRST_JPEG_SIZE + JPEG_DATA_OFFSET, jpegData, jpegLength);
           *(uint32_t *)((uint8_t*)mJpegSuperBuf->bufs[0]->buffer+0x13) = jpegLength + FIRST_JPEG_SIZE;
           //need to qbuf the old 8MB buffer
           qbuf_helper(jpeg_buf);
           ret = JPEG_RECEIVED;
        }
     } else if (jpegMode >= HDR_JPEG_MODE && jpegMode <= BEST_PHOTO420_MODE) {
        ALOGD("%s: Received %d mode, Count %x ", __func__, jpegMode,  jpegCount);
        if(mJpegSuperBuf != NULL){
            free(mJpegSuperBuf);
            mJpegSuperBuf = NULL;
        }
        mJpegSuperBuf = (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
        memcpy(mJpegSuperBuf, jpeg_buf, sizeof(mm_camera_super_buf_t));
        mJpegSuperBuf->split_jpeg = 0;
        *((uint32_t *)jpegInfo + 1) = jpegLength;
        ret = RAW_RECEIVED;
    } else {
        ALOGE("Error - %d mode This is not supported yet.", jpegMode);
        ret = BAD_VALUE;
    }
    ALOGD("%s: X", __func__);
    return ret;
}

status_t QCameraStream_Rdi::initStream(uint8_t no_cb_needed, uint8_t stream_on)
{
    status_t ret = NO_ERROR;

    ALOGI(" %s : E ", __FUNCTION__);
    mNumBuffers = PREVIEW_BUFFER_COUNT;
    if(mHalCamCtrl->isZSLMode()) {
        if(mNumBuffers < mHalCamCtrl->getZSLQueueDepth() + 3) {
            mNumBuffers = mHalCamCtrl->getZSLQueueDepth() + 3;
        }
    }
    ret = QCameraStream::initStream(no_cb_needed, stream_on);
end:
    ALOGI(" %s : X ", __FUNCTION__);
    return ret;
}

int QCameraStream_Rdi::getBuf(mm_camera_frame_len_offset *frame_offset_info,
                              uint8_t num_bufs,
                              uint8_t *initial_reg_flag,
                              mm_camera_buf_def_t  *bufs)
{
    int ret = 0;
    ALOGE("%s:BEGIN",__func__);

    if (num_bufs > mNumBuffers) {
        mNumBuffers = num_bufs;
    }
    if ((mNumBuffers == 0) || (mNumBuffers > MM_CAMERA_MAX_NUM_FRAMES)) {
        ALOGE("%s: Invalid number of buffers (=%d) requested!",
             __func__, mNumBuffers);
        return BAD_VALUE;
    }

    memset(mRdiBuf, 0, sizeof(mRdiBuf));
    memcpy(&mFrameOffsetInfo, frame_offset_info, sizeof(mFrameOffsetInfo));
    ret = mHalCamCtrl->initHeapMem(&mHalCamCtrl->mRdiMemory,
                                   mNumBuffers,
                                   mFrameOffsetInfo.frame_len,
                                   MSM_PMEM_MAINIMG,
                                   &mFrameOffsetInfo,
                                   mRdiBuf);
    if(MM_CAMERA_OK == ret) {
        for(int i = 0; i < num_bufs; i++) {
            bufs[i] = mRdiBuf[i];
            initial_reg_flag[i] = true;
            bufs[i].frame_offset_info = *frame_offset_info;
        }
    }

    ALOGV("%s: X - ret = %d ", __func__, ret);
    return ret;
}

int QCameraStream_Rdi::putBuf(uint8_t num_bufs, mm_camera_buf_def_t *bufs)
{
    int ret = 0;
    ALOGE(" %s : E ", __FUNCTION__);
    ret = mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mRdiMemory);
    ALOGI(" %s : X ",__FUNCTION__);
    return ret;
}

void QCameraStream_Rdi::dumpFrameToFile(mm_camera_buf_def_t* newFrame)
{
    char buf[32];
    int file_fd;
    int i;
    const char *ext = "raw";
    int w, h;
    static int count = 0;
    const char *name = "rdi";
    unsigned long int size;
    w = mHalCamCtrl->mRdiWidth;
    h = mHalCamCtrl->mRdiHeight;

    if(LIKELY(count >= 10))
        return;

    if (newFrame != NULL) {
        char * str;
        size = w*h; /* buffer size without padding */
        snprintf(buf, sizeof(buf), "/data/misc/camera/%s_%d_%d_%d.%s", name, w, h, count,ext);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        if (file_fd < 0) {
            ALOGE("%s: cannot open file\n", __func__);
        } else {
            ALOGE("dumping RDI frame to file %s: size = %ld", buf, size);
            void* y_off =((uint8_t*)(newFrame->buffer)) + newFrame->planes[0].data_offset;
            write(file_fd, (const void *)(y_off), size);
            close(file_fd);
        }
        count++;
    }
}

status_t QCameraStream_Rdi::processVisionModeFrame(
    mm_camera_super_buf_t *frame) {

    status_t rc = NO_ERROR;
    camera_data_callback pcb;
    camera_memory_t *data = NULL;
    camera_frame_metadata_t *metadata = NULL;
    uint32_t msgType=0x00;

    ALOGE("%s: E", __func__);
    /* Show preview FPS for debug*/
    if (UNLIKELY(mHalCamCtrl->mDebugFps)) {
        mHalCamCtrl->debugShowPreviewFPS();
    }

    if(!mActive) {
        ALOGE("RDI Streaming Stopped. Returning callback");
        return NO_ERROR;
    }
    ALOGV("%s: MessageEnabled = 0x%x", __func__, mHalCamCtrl->mMsgEnabled);

    if(mHalCamCtrl->mDataCb != NULL) {
        if (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
            msgType |=  CAMERA_MSG_PREVIEW_FRAME;
            data = mHalCamCtrl->mRdiMemory.camera_memory[frame->\
                bufs[0]->buf_idx];
        }
        if (mActive && msgType) {
            ALOGV("sending data callback for vision mode");
            mHalCamCtrl->mDataCb(msgType, data, 0, metadata, mHalCamCtrl->mCallbackCookie);
        }
    }
    return rc;
}

status_t QCameraStream_Rdi::processRdiFrame(
  mm_camera_super_buf_t *frame)
{
    ALOGV("%s: E",__func__);
    int msgType = 0;
    int i;
    status_t status = NO_ERROR, ret = NO_ERROR;
    camera_data_callback pcb;
    uint8_t * jpeg_info;
    status_t err = NO_ERROR;

    if(!mActive) {
        ALOGE("RDI Streaming Stopped. Returning callback");
        return NO_ERROR;
    }

    if(mHalCamCtrl->mVisionModeFlag) {
        dumpFrameToFile(frame->bufs[0]);
        this->processVisionModeFrame(frame);
        qbuf_helper(frame);
        return NO_ERROR;
    }
    jpeg_info = (uint8_t *)(frame->bufs[0]->buffer);
    if (jpeg_info == NULL) {
        ALOGE("%s: Error: Received null jpeg_info", __func__);
        return BAD_VALUE;
    }

    if(mHalCamCtrl==NULL) {
      ALOGE("%s: X: HAL control object not set",__func__);
      /*Call buf done*/
      ret = BAD_VALUE;
      qbuf_helper(frame);
      return ret;
    }
    ALOGD("RDI2 frame idx %d", frame->bufs[0]->frame_idx);

    if (mHalCamCtrl->mDataCb != NULL) {
       //process JPEG info and data.
        jpeg_info_t jpegInfo;
        status = processJpegData(frame->bufs[0]->buffer,
               ((uint8_t*)(frame->bufs[0]->buffer)) + JPEG_DATA_OFFSET,&jpegInfo, frame);
        if(status == JPEG_PENDING){
            ALOGD("%s: split jpeg, finished with the first jpeg", __func__);
        }
        else if (status == JPEG_RECEIVED || status == RAW_RECEIVED) {
            /* set rawdata proc thread and jpeg notify thread to active state */
            mHalCamCtrl->mNotifyTh->sendCmd(CAMERA_CMD_TYPE_START_DATA_PROC, FALSE,TRUE);
            mHalCamCtrl->mDataProcTh->sendCmd(CAMERA_CMD_TYPE_START_DATA_PROC, TRUE,FALSE);
            mm_camera_super_buf_t* dup_frame =
                   (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
            if (dup_frame == NULL) {
                ALOGE("%s: Error allocating memory to save received_frame structure.", __func__);
                qbuf_helper(frame);
                ret = BAD_VALUE;
            } else {
                memcpy(dup_frame, mJpegSuperBuf, sizeof(mm_camera_super_buf_t));
                free (mJpegSuperBuf);
                mJpegSuperBuf = NULL;
                mHalCamCtrl->mSuperBufQueue.enqueue(dup_frame);
                if(status == RAW_RECEIVED){
                    ALOGD("For HDR Raw snapshot, just enqueue raw buffer and proceed ");
                    mHalCamCtrl->mNotifyTh->sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE,FALSE);
                    return NO_ERROR;
                }
                ALOGV("Request_super_buf matching frame id %d ",jpegInfo.jpegFrameId);
                mm_camera_req_buf_t req_buf;
                req_buf.num_buf_requested = 1;
                req_buf.yuv_frame_id = jpegInfo.jpegFrameId;
                ret = mHalCamCtrl->mCameraHandle->ops->request_super_buf_by_frameId(
                        mHalCamCtrl->mCameraHandle->camera_handle,
                        mHalCamCtrl->mChannelId,
                        &req_buf);

                if (MM_CAMERA_OK != ret) {
                    ALOGE("error - request_super_buf failed.");
                    qbuf_helper(frame);
                }
                ret = NO_ERROR; //TODO: return or queue back the buff
            }
        } else if (ret != NO_ERROR) {
            ALOGE("processJpegData failed with %d", ret);
            qbuf_helper(frame);
            ret = BAD_VALUE;
        }  
    }
    if (status != JPEG_RECEIVED) {
        ALOGE("%s: Return RDI2 buffer back Qbuf\n", __func__);
        qbuf_helper(frame);
    }
    ALOGV("%s: X", __func__);
    return ret;
}

void QCameraStream_Rdi::qbuf_helper(mm_camera_super_buf_t *frame)
{
    for (int i=0; i<frame->num_bufs; i++) {
        if (frame->bufs[i] != NULL) {
            if(MM_CAMERA_OK != p_mm_ops->ops->qbuf(mCameraHandle, mChannelId,
                                    frame->bufs[i])){
                     ALOGE("%s: Buf done failed for buffer %p", __func__, frame->bufs[i]);
            }
        }
        mHalCamCtrl->cache_ops((QCameraHalMemInfo_t *)(frame->bufs[i]->mem_info),
                           frame->bufs[i]->buffer,
                           ION_IOC_CLEAN_CACHES);
    }
}


// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

QCameraStream_Rdi::
QCameraStream_Rdi(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode,
                        QCameraHardwareInterface* camCtrl)
  : QCameraStream(CameraHandle,
                 ChannelId,
                 Width,
                 Height,
                 Format,
                 NumBuffers,
                 mm_ops,
                 imgmode,
                 mode,
                 camCtrl)
{
    mJpegSuperBuf = NULL;
    ALOGV("%s: E", __func__);
    ALOGV("%s: X", __func__);
}
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

QCameraStream_Rdi::~QCameraStream_Rdi() {
    ALOGV("%s: E", __func__);
    release();
    ALOGV("%s: X", __func__);

}

void QCameraStream_Rdi::release() {
    ALOGE("%s : BEGIN",__func__);
    streamOff(0);
    deinitStream();
    ALOGE("%s: END", __func__);
}
// ---------------------------------------------------------------------------
// No code beyone this line
// ---------------------------------------------------------------------------
}; // namespace android
