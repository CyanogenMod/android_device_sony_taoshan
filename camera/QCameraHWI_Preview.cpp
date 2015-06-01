/*
** Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "QCameraHWI_Preview"
#include <utils/Log.h>
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "QCameraHAL.h"
#include "QCameraHWI.h"
#include <gralloc_priv.h>

#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)

/* QCameraHWI_Preview class implementation goes here*/
/* following code implement the preview mode's image capture & display logic of this class*/

namespace android {

// ---------------------------------------------------------------------------
// Preview Callback
// ---------------------------------------------------------------------------
static void preview_notify_cb(mm_camera_ch_data_buf_t *frame,
                                void *user_data)
{
  QCameraStream_preview *pme = (QCameraStream_preview *)user_data;
  mm_camera_ch_data_buf_t *bufs_used = 0;
  ALOGV("%s: E", __func__);
  /* for peview data, there is no queue, so directly use*/
  if(pme==NULL) {
    ALOGE("%s: X : Incorrect cookie",__func__);
    /*Call buf done*/
    return;
  }

  pme->processPreviewFrame(frame);
  ALOGV("%s: X", __func__);
}

status_t QCameraStream_preview::setPreviewWindow(preview_stream_ops_t* window)
{
    status_t retVal = NO_ERROR;
    ALOGE(" %s: E ", __FUNCTION__);
    if( window == NULL) {
        ALOGW(" Setting NULL preview window ");
        /* TODO: Current preview window will be invalidated.
         * Release all the buffers back */
       // relinquishBuffers();
    }
    Mutex::Autolock lock(mStopCallbackLock);
    mPreviewWindow = window;
    ALOGV(" %s : X ", __FUNCTION__ );
    return retVal;
}

status_t QCameraStream_preview::getBufferFromSurface() {
    int err = 0;
    int numMinUndequeuedBufs = 0;
  int format = 0;
  status_t ret = NO_ERROR;
  int gralloc_usage;

    ALOGI(" %s : E ", __FUNCTION__);

    if( mPreviewWindow == NULL) {
    ALOGE("%s: mPreviewWindow = NULL", __func__);
        return INVALID_OPERATION;
  }
    cam_ctrl_dimension_t dim;

  //mDisplayLock.lock();
    ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_DIMENSION,&dim);

	format = mHalCamCtrl->getPreviewFormatInfo().Hal_format;
	if(ret != NO_ERROR) {
        ALOGE("%s: display format %d is not supported", __func__, dim.prev_format);
    goto end;
  }
  numMinUndequeuedBufs = 0;
  if(mPreviewWindow->get_min_undequeued_buffer_count) {
    err = mPreviewWindow->get_min_undequeued_buffer_count(mPreviewWindow, &numMinUndequeuedBufs);
    if (err != 0) {
       ALOGE("get_min_undequeued_buffer_count  failed: %s (%d)",
            strerror(-err), -err);
       ret = UNKNOWN_ERROR;
       goto end;
    }
  }
    mHalCamCtrl->mPreviewMemoryLock.lock();
    mHalCamCtrl->mPreviewMemory.buffer_count = kPreviewBufferCount + numMinUndequeuedBufs;
    if(mHalCamCtrl->isZSLMode()) {
      if(mHalCamCtrl->getZSLQueueDepth() > numMinUndequeuedBufs)
        mHalCamCtrl->mPreviewMemory.buffer_count +=
            mHalCamCtrl->getZSLQueueDepth() - numMinUndequeuedBufs;
    }
    err = mPreviewWindow->set_buffer_count(mPreviewWindow, mHalCamCtrl->mPreviewMemory.buffer_count );
    if (err != 0) {
         ALOGE("set_buffer_count failed: %s (%d)",
                    strerror(-err), -err);
         ret = UNKNOWN_ERROR;
     goto end;
    }
    err = mPreviewWindow->set_buffers_geometry(mPreviewWindow,
                dim.display_width, dim.display_height, format);
    if (err != 0) {
         ALOGE("set_buffers_geometry failed: %s (%d)",
                    strerror(-err), -err);
         ret = UNKNOWN_ERROR;
     goto end;
    }

    ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_VFE_OUTPUT_ENABLE, &mVFEOutputs);
    if(ret != MM_CAMERA_OK) {
        ALOGE("get parm MM_CAMERA_PARM_VFE_OUTPUT_ENABLE  failed");
        ret = BAD_VALUE;
        goto end;
    }

    //as software encoder is used to encode 720p, to enhance the performance
    //cashed pmem is used here
    if(mVFEOutputs == 1)
        gralloc_usage = GRALLOC_USAGE_HW_CAMERA_WRITE | CAMERA_GRALLOC_HEAP_ID | CAMERA_GRALLOC_FALLBACK_HEAP_ID;
    else
        gralloc_usage = GRALLOC_USAGE_HW_CAMERA_WRITE | CAMERA_GRALLOC_HEAP_ID | CAMERA_GRALLOC_FALLBACK_HEAP_ID |
                    CAMERA_GRALLOC_CACHING_ID;
    err = mPreviewWindow->set_usage(mPreviewWindow, gralloc_usage);
    if(err != 0) {
    /* set_usage error out */
        ALOGE("%s: set_usage rc = %d", __func__, err);
        ret = UNKNOWN_ERROR;
        goto end;
    }
    ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_HFR_FRAME_SKIP, &mHFRFrameSkip);
    if(ret != MM_CAMERA_OK) {
        ALOGE("get parm MM_CAMERA_PARM_HFR_FRAME_SKIP  failed");
        ret = BAD_VALUE;
        goto end;
    }
	for (int cnt = 0; cnt < mHalCamCtrl->mPreviewMemory.buffer_count; cnt++) {
		int stride;
		err = mPreviewWindow->dequeue_buffer(mPreviewWindow,
										&mHalCamCtrl->mPreviewMemory.buffer_handle[cnt],
										&mHalCamCtrl->mPreviewMemory.stride[cnt]);
		if(!err) {
          ALOGE("%s: dequeue buf hdl =%p, size =%d, stride=%d", __func__, *mHalCamCtrl->mPreviewMemory.buffer_handle[cnt],
                ((struct private_handle_t *)(*mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]))->size,  mHalCamCtrl->mPreviewMemory.stride[cnt]);
                    err = mPreviewWindow->lock_buffer(this->mPreviewWindow,
                                       mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]);
                    mHalCamCtrl->mPreviewMemory.local_flag[cnt] = BUFFER_OWNED;
		} else {
          mHalCamCtrl->mPreviewMemory.local_flag[cnt] = BUFFER_NOT_OWNED;
          ALOGE("%s: dequeue_buffer idx = %d err = %d", __func__, cnt, err);
        }

		ALOGE("%s: dequeue buf: %p\n", __func__, mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]);

		if(err != 0) {
            ALOGE("%s: dequeue_buffer failed: %s (%d)", __func__,
                    strerror(-err), -err);
            ret = UNKNOWN_ERROR;
			for(int i = 0; i < cnt; i++) {
                            if( mHalCamCtrl->mPreviewMemory.local_flag[i] != BUFFER_NOT_OWNED) {
                  err = mPreviewWindow->cancel_buffer(mPreviewWindow,
                                          mHalCamCtrl->mPreviewMemory.buffer_handle[i]);
                }
                mHalCamCtrl->mPreviewMemory.local_flag[i] = BUFFER_NOT_OWNED;
                ALOGE("%s: cancel_buffer: hdl =%p", __func__,  (*mHalCamCtrl->mPreviewMemory.buffer_handle[i]));
				mHalCamCtrl->mPreviewMemory.buffer_handle[i] = NULL;
			}
            memset(&mHalCamCtrl->mPreviewMemory, 0, sizeof(mHalCamCtrl->mPreviewMemory));
			goto end;
		}

		mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt] =
		    (struct private_handle_t *)(*mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]);
#ifdef USE_ION
        mHalCamCtrl->mPreviewMemory.main_ion_fd[cnt] = open("/dev/ion", O_RDONLY);
        if (mHalCamCtrl->mPreviewMemory.main_ion_fd[cnt] < 0) {
            ALOGE("%s: failed: could not open ion device\n", __func__);
        } else {
            mHalCamCtrl->mPreviewMemory.ion_info_fd[cnt].fd =
                mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->fd;
            if (ioctl(mHalCamCtrl->mPreviewMemory.main_ion_fd[cnt],
              ION_IOC_IMPORT, &mHalCamCtrl->mPreviewMemory.ion_info_fd[cnt]) < 0)
              ALOGE("ION import failed\n");
        }
#endif
		mHalCamCtrl->mPreviewMemory.camera_memory[cnt] =
		    mHalCamCtrl->mGetMemory(mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->fd,
			mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->size, 1, (void *)this);
		ALOGE("%s: idx = %d, fd = %d, size = %d, offset = %d", __func__,
            cnt, mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->fd,
      mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->size,
      mHalCamCtrl->mPreviewMemory.private_buffer_handle[cnt]->offset);
  }


  memset(&mHalCamCtrl->mMetadata, 0, sizeof(mHalCamCtrl->mMetadata));
  memset(mHalCamCtrl->mFace, 0, sizeof(mHalCamCtrl->mFace));

    ALOGI(" %s : X ",__FUNCTION__);
end:
  //mDisplayLock.unlock();
  mHalCamCtrl->mPreviewMemoryLock.unlock();

    return ret;
}

status_t QCameraStream_preview::putBufferToSurface() {
    int err = 0;
    status_t ret = NO_ERROR;

    ALOGI(" %s : E ", __FUNCTION__);

    mHalCamCtrl->mPreviewMemoryLock.lock();
	for (int cnt = 0; cnt < mHalCamCtrl->mPreviewMemory.buffer_count; cnt++) {
        if (cnt < mHalCamCtrl->mPreviewMemory.buffer_count) {
            if (NO_ERROR != mHalCamCtrl->sendUnMappingBuf(MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW, cnt, mCameraId,
                                                          CAM_SOCK_MSG_TYPE_FD_UNMAPPING)) {
                ALOGE("%s: sending data Msg Failed", __func__);
            }
        }

        mHalCamCtrl->mPreviewMemory.camera_memory[cnt]->release(mHalCamCtrl->mPreviewMemory.camera_memory[cnt]);
#ifdef USE_ION
        struct ion_handle_data ion_handle;
        ion_handle.handle = mHalCamCtrl->mPreviewMemory.ion_info_fd[cnt].handle;
        if (ioctl(mHalCamCtrl->mPreviewMemory.main_ion_fd[cnt], ION_IOC_FREE, &ion_handle)
            < 0)
            ALOGE("%s: ion free failed\n", __func__);
        close(mHalCamCtrl->mPreviewMemory.main_ion_fd[cnt]);
#endif
             if( mHalCamCtrl->mPreviewMemory.local_flag[cnt] != BUFFER_NOT_OWNED) {
               err = mPreviewWindow->cancel_buffer(mPreviewWindow, mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]);
               ALOGD("%s: cancel_buffer: hdl =%p", __func__,  (*mHalCamCtrl->mPreviewMemory.buffer_handle[cnt]));
             }
             mHalCamCtrl->mPreviewMemory.local_flag[cnt] = BUFFER_NOT_OWNED;

		ALOGD(" put buffer %d successfully", cnt);
	}

    if (mDisplayBuf.preview.buf.mp != NULL) {
        delete[] mDisplayBuf.preview.buf.mp;
        mDisplayBuf.preview.buf.mp = NULL;
    }

    mHalCamCtrl->mPreviewMemoryLock.unlock();
	memset(&mHalCamCtrl->mPreviewMemory, 0, sizeof(mHalCamCtrl->mPreviewMemory));
    ALOGI(" %s : X ",__FUNCTION__);
    return NO_ERROR;
}


status_t  QCameraStream_preview::getBufferNoDisplay( )
{
  int err = 0;
  status_t ret = NO_ERROR;
  int i, num_planes, frame_len, y_off, cbcr_off;
  cam_ctrl_dimension_t dim;
  uint32_t planes[VIDEO_MAX_PLANES];

  ALOGI("%s : E ", __FUNCTION__);


  ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_DIMENSION, &dim);
  if(ret != NO_ERROR) {
      ALOGE("%s: display format %d is not supported", __func__, dim.prev_format);
    goto end;
  }
  mHalCamCtrl->mPreviewMemoryLock.lock();
  mHalCamCtrl->mNoDispPreviewMemory.buffer_count = kPreviewBufferCount;
  if(mHalCamCtrl->isZSLMode()) {
    if(mHalCamCtrl->getZSLQueueDepth() > kPreviewBufferCount - 3)
      mHalCamCtrl->mNoDispPreviewMemory.buffer_count =
      mHalCamCtrl->getZSLQueueDepth() + 3;
  }

  num_planes = dim.display_frame_offset.num_planes;
  for ( i = 0; i < num_planes; i++) {
    planes[i] = dim.display_frame_offset.mp[i].len;
  }

  frame_len = dim.picture_frame_offset.frame_len;
  y_off = dim.picture_frame_offset.mp[0].offset;
  cbcr_off = dim.picture_frame_offset.mp[1].offset;
  ALOGE("%s: main image: rotation = %d, yoff = %d, cbcroff = %d, size = %d, width = %d, height = %d",
       __func__, dim.rotation, y_off, cbcr_off, frame_len,
       dim.display_width, dim.display_height);
  if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mNoDispPreviewMemory,
     mHalCamCtrl->mNoDispPreviewMemory.buffer_count,
     frame_len, y_off, cbcr_off, MSM_PMEM_MAINIMG,
     NULL,NULL, num_planes, planes) < 0) {
              ret = NO_MEMORY;
              goto end;
  };

  memset(&mHalCamCtrl->mMetadata, 0, sizeof(mHalCamCtrl->mMetadata));
  memset(mHalCamCtrl->mFace, 0, sizeof(mHalCamCtrl->mFace));

  ALOGI(" %s : X ",__FUNCTION__);
end:
  //mDisplayLock.unlock();
  mHalCamCtrl->mPreviewMemoryLock.unlock();

  return NO_ERROR;
}

status_t   QCameraStream_preview::freeBufferNoDisplay()
{
  int err = 0;
  status_t ret = NO_ERROR;

  ALOGI(" %s : E ", __FUNCTION__);

  //mDisplayLock.lock();
  mHalCamCtrl->mPreviewMemoryLock.lock();
  for (int cnt = 0; cnt < mHalCamCtrl->mNoDispPreviewMemory.buffer_count; cnt++) {
      if (cnt < mHalCamCtrl->mNoDispPreviewMemory.buffer_count) {
          if (NO_ERROR != mHalCamCtrl->sendUnMappingBuf(MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW,
                       cnt, mCameraId, CAM_SOCK_MSG_TYPE_FD_UNMAPPING)) {
              ALOGE("%s: sending data Msg Failed", __func__);
          }
      }
  }
  mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mNoDispPreviewMemory);
  memset(&mHalCamCtrl->mNoDispPreviewMemory, 0, sizeof(mHalCamCtrl->mNoDispPreviewMemory));
  if (mDisplayBuf.preview.buf.mp != NULL) {
      delete[] mDisplayBuf.preview.buf.mp;
      mDisplayBuf.preview.buf.mp = NULL;
  }

  mHalCamCtrl->mPreviewMemoryLock.unlock();
  ALOGI(" %s : X ",__FUNCTION__);
  return NO_ERROR;
}

void QCameraStream_preview::notifyROIEvent(fd_roi_t roi)
{
    camera_memory_t *data = mHalCamCtrl->mGetMemory(-1, 1, 1, NULL);
    switch (roi.type) {
    case FD_ROI_TYPE_HEADER:
        {
            mDisplayLock.lock();
            mNumFDRcvd = 0;
            memset(mHalCamCtrl->mFace, 0, sizeof(mHalCamCtrl->mFace));
            mHalCamCtrl->mMetadata.faces = mHalCamCtrl->mFace;
            mHalCamCtrl->mMetadata.number_of_faces = roi.d.hdr.num_face_detected;
            if(mHalCamCtrl->mMetadata.number_of_faces > MAX_ROI)
              mHalCamCtrl->mMetadata.number_of_faces = MAX_ROI;
            mDisplayLock.unlock();

            if (mHalCamCtrl->mMetadata.number_of_faces == 0) {
                // Clear previous faces
                mHalCamCtrl->mCallbackLock.lock();
                camera_data_callback pcb = mHalCamCtrl->mDataCb;
                mHalCamCtrl->mCallbackLock.unlock();

                if (pcb && (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_METADATA)){
                    ALOGE("%s: Face detection RIO callback", __func__);
                    pcb(CAMERA_MSG_PREVIEW_METADATA, data, 0, &mHalCamCtrl->mMetadata, mHalCamCtrl->mCallbackCookie);
                }
            }
        }
        break;
    case FD_ROI_TYPE_DATA:
        {
            mDisplayLock.lock();
            int idx = roi.d.data.idx;
            if (idx >= mHalCamCtrl->mMetadata.number_of_faces) {
                mDisplayLock.unlock();
                ALOGE("%s: idx %d out of boundary %d", __func__, idx, mHalCamCtrl->mMetadata.number_of_faces);
                break;
            }

            mHalCamCtrl->mFace[idx].id = roi.d.data.face.id;
            mHalCamCtrl->mFace[idx].score = roi.d.data.face.score;

            
            // top
            mHalCamCtrl->mFace[idx].rect[0] =
               roi.d.data.face.face_boundary.x*2000/mHalCamCtrl->mDimension.display_width - 1000;
            //right
            mHalCamCtrl->mFace[idx].rect[1] =
               roi.d.data.face.face_boundary.y*2000/mHalCamCtrl->mDimension.display_height - 1000;
            //bottom
            mHalCamCtrl->mFace[idx].rect[2] =  mHalCamCtrl->mFace[idx].rect[0] +
               roi.d.data.face.face_boundary.dx*2000/mHalCamCtrl->mDimension.display_width;
            //left
            mHalCamCtrl->mFace[idx].rect[3] = mHalCamCtrl->mFace[idx].rect[1] +
               roi.d.data.face.face_boundary.dy*2000/mHalCamCtrl->mDimension.display_height;

            mHalCamCtrl->mFace_info[idx].face_id = roi.d.data.face.id;
            mHalCamCtrl->mFace_info[idx].face_rect.x = roi.d.data.face.face_boundary.x;
            mHalCamCtrl->mFace_info[idx].face_rect.y = roi.d.data.face.face_boundary.y;
            mHalCamCtrl->mFace_info[idx].face_rect.dx = roi.d.data.face.face_boundary.dx;
            mHalCamCtrl->mFace_info[idx].face_rect.dy = roi.d.data.face.face_boundary.dy;
            mHalCamCtrl->mFace_info[idx].fd_pos.x = mHalCamCtrl->mFace_info[idx].face_rect.x +
                                                        (mHalCamCtrl->mFace_info[idx].face_rect.dx/2);
            mHalCamCtrl->mFace_info[idx].fd_pos.y = mHalCamCtrl->mFace_info[idx].face_rect.y +
                                                        (mHalCamCtrl->mFace_info[idx].face_rect.dy/2);
            mHalCamCtrl->mFace_info[idx].display_width = mHalCamCtrl->mDimension.display_width;
            mHalCamCtrl->mFace_info[idx].display_height = mHalCamCtrl->mDimension.display_height;

            // Center of left eye
            mHalCamCtrl->mFace[idx].left_eye[0] =
              roi.d.data.face.left_eye_center[0]*2000/mHalCamCtrl->mDimension.display_width - 1000;
            mHalCamCtrl->mFace[idx].left_eye[1] =
              roi.d.data.face.left_eye_center[1]*2000/mHalCamCtrl->mDimension.display_height - 1000;

            // Center of right eye
            mHalCamCtrl->mFace[idx].right_eye[0] =
              roi.d.data.face.right_eye_center[0]*2000/mHalCamCtrl->mDimension.display_width - 1000;
            mHalCamCtrl->mFace[idx].right_eye[1] =
              roi.d.data.face.right_eye_center[1]*2000/mHalCamCtrl->mDimension.display_height - 1000;

            // Center of mouth
            mHalCamCtrl->mFace[idx].mouth[0] =
              roi.d.data.face.mouth_center[0]*2000/mHalCamCtrl->mDimension.display_width - 1000;
            mHalCamCtrl->mFace[idx].mouth[1] =
              roi.d.data.face.mouth_center[1]*2000/mHalCamCtrl->mDimension.display_height - 1000;

            /*
            mHalCamCtrl->mFace[idx].smile_degree = roi.d.data.face.smile_degree;
            mHalCamCtrl->mFace[idx].smile_score = roi.d.data.face.smile_confidence;
            mHalCamCtrl->mFace[idx].blink_detected = roi.d.data.face.blink_detected;
            mHalCamCtrl->mFace[idx].face_recognised = roi.d.data.face.is_face_recognised;
            mHalCamCtrl->mFace[idx].gaze_angle = roi.d.data.face.gaze_angle;

            //newly added
            // upscale by 2 to recover from demaen downscaling
            mHalCamCtrl->mFace[idx].updown_dir = roi.d.data.face.updown_dir*2;
            mHalCamCtrl->mFace[idx].leftright_dir = roi.d.data.face.leftright_dir*2;
            mHalCamCtrl->mFace[idx].roll_dir = roi.d.data.face.roll_dir*2;

            mHalCamCtrl->mFace[idx].leye_blink = roi.d.data.face.left_blink;
            mHalCamCtrl->mFace[idx].reye_blink = roi.d.data.face.right_blink;
            mHalCamCtrl->mFace[idx].left_right_gaze = roi.d.data.face.left_right_gaze;
            mHalCamCtrl->mFace[idx].top_bottom_gaze = roi.d.data.face.top_bottom_gaze;
            ALOGE("%s: Face(%d, %d, %d, %d), leftEye(%d, %d), rightEye(%d, %d), mouth(%d, %d), smile(%d, %d), face_recg(%d)", __func__,
               mHalCamCtrl->mFace[idx].rect[0],  mHalCamCtrl->mFace[idx].rect[1],
               mHalCamCtrl->mFace[idx].rect[2],  mHalCamCtrl->mFace[idx].rect[3],
               mHalCamCtrl->mFace[idx].left_eye[0], mHalCamCtrl->mFace[idx].left_eye[1],
               mHalCamCtrl->mFace[idx].right_eye[0], mHalCamCtrl->mFace[idx].right_eye[1],
               mHalCamCtrl->mFace[idx].mouth[0], mHalCamCtrl->mFace[idx].mouth[1],
               mHalCamCtrl->mFace[idx].smile_degree, mHalCamCtrl->mFace[idx].smile_score,
               mHalCamCtrl->mFace[idx].face_recognised);
            ALOGE("%s: gaze(%d, %d, %d), updown(%d), leftright(%d), roll(%d), blink(%d, %d, %d)", __func__,
               mHalCamCtrl->mFace[idx].gaze_angle,  mHalCamCtrl->mFace[idx].left_right_gaze,
               mHalCamCtrl->mFace[idx].top_bottom_gaze,  mHalCamCtrl->mFace[idx].updown_dir,
               mHalCamCtrl->mFace[idx].leftright_dir, mHalCamCtrl->mFace[idx].roll_dir,
               mHalCamCtrl->mFace[idx].blink_detected,
               mHalCamCtrl->mFace[idx].leye_blink, mHalCamCtrl->mFace[idx].reye_blink);
*/
             mNumFDRcvd++;
             mDisplayLock.unlock();

             if (mNumFDRcvd == mHalCamCtrl->mMetadata.number_of_faces) {
                 mHalCamCtrl->mCallbackLock.lock();
                 camera_data_callback pcb = mHalCamCtrl->mDataCb;
                 mHalCamCtrl->mCallbackLock.unlock();

                 if (pcb && (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_METADATA)){
                     ALOGE("%s: Face detection RIO callback with %d faces detected (score=%d)", __func__, mNumFDRcvd, mHalCamCtrl->mFace[idx].score);
                     pcb(CAMERA_MSG_PREVIEW_METADATA, data, 0, &mHalCamCtrl->mMetadata, mHalCamCtrl->mCallbackCookie);
                 }
             }
        }
        break;
    }
    if(NULL != data) data->release(data);
}

/*void QCameraStream_preview::notifyROIEvent(fd_roi_t roi) {
    ALOGI("%s Dummy function implementation ", __func__);
    return;
}*/


status_t QCameraStream_preview::initDisplayBuffers()
{
  status_t ret = NO_ERROR;
  int width = 0;  /* width of channel  */
  int height = 0; /* height of channel */
  uint32_t frame_len = 0; /* frame planner length */
  int buffer_num = 4; /* number of buffers for display */
  const char *pmem_region;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];
  void *vaddr = NULL;
  cam_ctrl_dimension_t dim;
  int i;

  ALOGE("%s:BEGIN",__func__);
  memset(&mHalCamCtrl->mMetadata, 0, sizeof(camera_frame_metadata_t));
  mHalCamCtrl->mPreviewMemoryLock.lock();
  memset(&mHalCamCtrl->mPreviewMemory, 0, sizeof(mHalCamCtrl->mPreviewMemory));
  mHalCamCtrl->mPreviewMemoryLock.unlock();
  memset(&mNotifyBuffer, 0, sizeof(mNotifyBuffer));

/* get preview size, by qury mm_camera*/
  memset(&dim, 0, sizeof(cam_ctrl_dimension_t));

  memset(&(this->mDisplayStreamBuf),0, sizeof(this->mDisplayStreamBuf));

  ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_DIMENSION, &dim);
  if (MM_CAMERA_OK != ret) {
    ALOGE("%s: error - can't get camera dimension!", __func__);
    ALOGE("%s: X", __func__);
    return BAD_VALUE;
  }else {
    width =  dim.display_width,
    height = dim.display_height;
  }

  ret = getBufferFromSurface();
  if(ret != NO_ERROR) {
    ALOGE("%s: cannot get memory from surface texture client, ret = %d", __func__, ret);
    return ret;
  }

  /* set 4 buffers for display */
  mHalCamCtrl->mPreviewMemoryLock.lock();
  memset(&mDisplayStreamBuf, 0, sizeof(mDisplayStreamBuf));
  this->mDisplayStreamBuf.num = mHalCamCtrl->mPreviewMemory.buffer_count;
  this->myMode=myMode; /*Need to assign this in constructor after translating from mask*/
  num_planes = dim.display_frame_offset.num_planes;
  for(i =0; i< num_planes; i++) {
     planes[i] = dim.display_frame_offset.mp[i].len;
  }
   this->mDisplayStreamBuf.frame_len = dim.display_frame_offset.frame_len;

  memset(&mDisplayBuf, 0, sizeof(mDisplayBuf));
  mDisplayBuf.preview.buf.mp = new mm_camera_mp_buf_t[mDisplayStreamBuf.num];
  if (!mDisplayBuf.preview.buf.mp) {
    ALOGE("%s Error allocating memory for mplanar struct ", __func__);
    ret = NO_MEMORY;
    goto error;
  }
  memset(mDisplayBuf.preview.buf.mp, 0,
    mDisplayStreamBuf.num * sizeof(mm_camera_mp_buf_t));

  /*allocate memory for the buffers*/
  for(i = 0; i < mDisplayStreamBuf.num; i++){
	  if (mHalCamCtrl->mPreviewMemory.private_buffer_handle[i] == NULL)
		  continue;
      mDisplayStreamBuf.frame[i].fd = mHalCamCtrl->mPreviewMemory.private_buffer_handle[i]->fd;
      mDisplayStreamBuf.frame[i].cbcr_off = planes[0];
      mDisplayStreamBuf.frame[i].y_off = 0;
      mDisplayStreamBuf.frame[i].path = OUTPUT_TYPE_P;
	  mHalCamCtrl->mPreviewMemory.addr_offset[i] =
	      mHalCamCtrl->mPreviewMemory.private_buffer_handle[i]->offset;
      mDisplayStreamBuf.frame[i].buffer =
          (long unsigned int)mHalCamCtrl->mPreviewMemory.camera_memory[i]->data;
      mDisplayStreamBuf.frame[i].ion_alloc.len = mHalCamCtrl->mPreviewMemory.private_buffer_handle[i]->size;
      mDisplayStreamBuf.frame[i].ion_dev_fd = mHalCamCtrl->mPreviewMemory.main_ion_fd[i];
      mDisplayStreamBuf.frame[i].fd_data = mHalCamCtrl->mPreviewMemory.ion_info_fd[i];

    ALOGE("%s: idx = %d, fd = %d, size = %d, cbcr_offset = %d, y_offset = %d, "
      "offset = %d, vaddr = 0x%x", __func__, i, mDisplayStreamBuf.frame[i].fd,
      mHalCamCtrl->mPreviewMemory.private_buffer_handle[i]->size,
      mDisplayStreamBuf.frame[i].cbcr_off, mDisplayStreamBuf.frame[i].y_off,
      mHalCamCtrl->mPreviewMemory.addr_offset[i],
      (uint32_t)mDisplayStreamBuf.frame[i].buffer);

    ret = mHalCamCtrl->sendMappingBuf(
                        MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW,
                        i,
                        mDisplayStreamBuf.frame[i].fd,
                        mHalCamCtrl->mPreviewMemory.private_buffer_handle[i]->size,
                        mCameraId, CAM_SOCK_MSG_TYPE_FD_MAPPING);
    if (NO_ERROR != ret) {
      ALOGE("%s: sending mapping data Msg Failed", __func__);
      goto error;
    }

    mDisplayBuf.preview.buf.mp[i].frame = mDisplayStreamBuf.frame[i];
    mDisplayBuf.preview.buf.mp[i].frame_offset = mHalCamCtrl->mPreviewMemory.addr_offset[i];
    mDisplayBuf.preview.buf.mp[i].num_planes = num_planes;

    /* Plane 0 needs to be set seperately. Set other planes
     * in a loop. */
    mDisplayBuf.preview.buf.mp[i].planes[0].length = planes[0];
    mDisplayBuf.preview.buf.mp[i].planes[0].m.userptr = mDisplayStreamBuf.frame[i].fd;
    mDisplayBuf.preview.buf.mp[i].planes[0].data_offset = 0;
    mDisplayBuf.preview.buf.mp[i].planes[0].reserved[0] =
      mDisplayBuf.preview.buf.mp[i].frame_offset;
    for (int j = 1; j < num_planes; j++) {
      mDisplayBuf.preview.buf.mp[i].planes[j].length = planes[j];
      mDisplayBuf.preview.buf.mp[i].planes[j].m.userptr =
        mDisplayStreamBuf.frame[i].fd;
      mDisplayBuf.preview.buf.mp[i].planes[j].data_offset = 0;
      mDisplayBuf.preview.buf.mp[i].planes[j].reserved[0] =
        mDisplayBuf.preview.buf.mp[i].planes[j-1].reserved[0] +
        mDisplayBuf.preview.buf.mp[i].planes[j-1].length;
    }

    for (int j = 0; j < num_planes; j++)
      ALOGE("Planes: %d length: %d userptr: %lu offset: %d\n", j,
        mDisplayBuf.preview.buf.mp[i].planes[j].length,
        mDisplayBuf.preview.buf.mp[i].planes[j].m.userptr,
        mDisplayBuf.preview.buf.mp[i].planes[j].reserved[0]);
  }/*end of for loop*/

 /* register the streaming buffers for the channel*/
  mDisplayBuf.ch_type = MM_CAMERA_CH_PREVIEW;
  mDisplayBuf.preview.num = mDisplayStreamBuf.num;
  mHalCamCtrl->mPreviewMemoryLock.unlock();
  ALOGE("%s:END",__func__);
  return NO_ERROR;

error:
    mHalCamCtrl->mPreviewMemoryLock.unlock();
    putBufferToSurface();

    ALOGV("%s: X", __func__);
    return ret;
}

status_t QCameraStream_preview::initPreviewOnlyBuffers()
{
  status_t ret = NO_ERROR;
  int width = 0;  /* width of channel  */
  int height = 0; /* height of channel */
  uint32_t frame_len = 0; /* frame planner length */
  int buffer_num = 4; /* number of buffers for display */
  const char *pmem_region;
  uint8_t num_planes = 0;
  uint32_t planes[VIDEO_MAX_PLANES];

  cam_ctrl_dimension_t dim;

  ALOGE("%s:BEGIN",__func__);
  memset(&mHalCamCtrl->mMetadata, 0, sizeof(camera_frame_metadata_t));
  mHalCamCtrl->mPreviewMemoryLock.lock();
  memset(&mHalCamCtrl->mNoDispPreviewMemory, 0, sizeof(mHalCamCtrl->mNoDispPreviewMemory));
  mHalCamCtrl->mPreviewMemoryLock.unlock();
  memset(&mNotifyBuffer, 0, sizeof(mNotifyBuffer));

/* get preview size, by qury mm_camera*/
  memset(&dim, 0, sizeof(cam_ctrl_dimension_t));
  ret = cam_config_get_parm(mCameraId, MM_CAMERA_PARM_DIMENSION, &dim);
  if (MM_CAMERA_OK != ret) {
    ALOGE("%s: error - can't get camera dimension!", __func__);
    ALOGE("%s: X", __func__);
    return BAD_VALUE;
  }else {
    width =  dim.display_width;
    height = dim.display_height;
  }

  ret = getBufferNoDisplay( );
  if(ret != NO_ERROR) {
    ALOGE("%s: cannot get memory from surface texture client, ret = %d", __func__, ret);
    return ret;
  }

  /* set 4 buffers for display */
  memset(&mDisplayStreamBuf, 0, sizeof(mDisplayStreamBuf));
  mHalCamCtrl->mPreviewMemoryLock.lock();
  this->mDisplayStreamBuf.num = mHalCamCtrl->mNoDispPreviewMemory.buffer_count;
  this->myMode=myMode; /*Need to assign this in constructor after translating from mask*/
  num_planes = dim.display_frame_offset.num_planes;
  for (int i = 0; i < num_planes; i++) {
    planes[i] = dim.display_frame_offset.mp[i].len;
  }
  this->mDisplayStreamBuf.frame_len = dim.display_frame_offset.frame_len;

  memset(&mDisplayBuf, 0, sizeof(mDisplayBuf));
  mDisplayBuf.preview.buf.mp = new mm_camera_mp_buf_t[mDisplayStreamBuf.num];
  if (!mDisplayBuf.preview.buf.mp) {
    ALOGE("%s Error allocating memory for mplanar struct ", __func__);
  }
  memset(mDisplayBuf.preview.buf.mp, 0,
    mDisplayStreamBuf.num * sizeof(mm_camera_mp_buf_t));

  /*allocate memory for the buffers*/
  void *vaddr = NULL;
  for(int i = 0; i < mDisplayStreamBuf.num; i++){
	  if (mHalCamCtrl->mNoDispPreviewMemory.camera_memory[i] == NULL)
		  continue;
      mDisplayStreamBuf.frame[i].fd = mHalCamCtrl->mNoDispPreviewMemory.fd[i];
      mDisplayStreamBuf.frame[i].cbcr_off = planes[0];
      mDisplayStreamBuf.frame[i].y_off = 0;
      mDisplayStreamBuf.frame[i].path = OUTPUT_TYPE_P;
      mDisplayStreamBuf.frame[i].buffer =
          (long unsigned int)mHalCamCtrl->mNoDispPreviewMemory.camera_memory[i]->data;
      mDisplayStreamBuf.frame[i].ion_dev_fd = mHalCamCtrl->mNoDispPreviewMemory.main_ion_fd[i];
      mDisplayStreamBuf.frame[i].fd_data = mHalCamCtrl->mNoDispPreviewMemory.ion_info_fd[i];

    ALOGE("%s: idx = %d, fd = %d, size = %d, cbcr_offset = %d, y_offset = %d, "
      "vaddr = 0x%x", __func__, i, mDisplayStreamBuf.frame[i].fd,
      frame_len,
      mDisplayStreamBuf.frame[i].cbcr_off, mDisplayStreamBuf.frame[i].y_off,
      (uint32_t)mDisplayStreamBuf.frame[i].buffer);

    if (NO_ERROR != mHalCamCtrl->sendMappingBuf(
                        MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW,
                        i,
                        mDisplayStreamBuf.frame[i].fd,
                        mHalCamCtrl->mNoDispPreviewMemory.size,
                        mCameraId, CAM_SOCK_MSG_TYPE_FD_MAPPING)) {
      ALOGE("%s: sending mapping data Msg Failed", __func__);
    }

    mDisplayBuf.preview.buf.mp[i].frame = mDisplayStreamBuf.frame[i];
    mDisplayBuf.preview.buf.mp[i].frame_offset = mDisplayStreamBuf.frame[i].y_off;
    mDisplayBuf.preview.buf.mp[i].num_planes = num_planes;

    /* Plane 0 needs to be set seperately. Set other planes
     * in a loop. */
    mDisplayBuf.preview.buf.mp[i].planes[0].length = planes[0];
    mDisplayBuf.preview.buf.mp[i].planes[0].m.userptr = mDisplayStreamBuf.frame[i].fd;
    mDisplayBuf.preview.buf.mp[i].planes[0].data_offset = 0;
    mDisplayBuf.preview.buf.mp[i].planes[0].reserved[0] =
      mDisplayBuf.preview.buf.mp[i].frame_offset;
    for (int j = 1; j < num_planes; j++) {
      mDisplayBuf.preview.buf.mp[i].planes[j].length = planes[j];
      mDisplayBuf.preview.buf.mp[i].planes[j].m.userptr =
        mDisplayStreamBuf.frame[i].fd;
      mDisplayBuf.preview.buf.mp[i].planes[j].data_offset = 0;
      mDisplayBuf.preview.buf.mp[i].planes[j].reserved[0] =
        mDisplayBuf.preview.buf.mp[i].planes[j-1].reserved[0] +
        mDisplayBuf.preview.buf.mp[i].planes[j-1].length;
    }

    for (int j = 0; j < num_planes; j++)
      ALOGE("Planes: %d length: %d userptr: %lu offset: %d\n", j,
        mDisplayBuf.preview.buf.mp[i].planes[j].length,
        mDisplayBuf.preview.buf.mp[i].planes[j].m.userptr,
        mDisplayBuf.preview.buf.mp[i].planes[j].reserved[0]);
  }/*end of for loop*/

 /* register the streaming buffers for the channel*/
  mDisplayBuf.ch_type = MM_CAMERA_CH_PREVIEW;
  mDisplayBuf.preview.num = mDisplayStreamBuf.num;
  mHalCamCtrl->mPreviewMemoryLock.unlock();
  ALOGE("%s:END",__func__);
  return NO_ERROR;

end:
  if (MM_CAMERA_OK == ret ) {
    ALOGV("%s: X - NO_ERROR ", __func__);
    return NO_ERROR;
  }

    ALOGV("%s: out of memory clean up", __func__);
  /* release the allocated memory */

  ALOGV("%s: X - BAD_VALUE ", __func__);
  return BAD_VALUE;
}


void QCameraStream_preview::dumpFrameToFile(struct msm_frame* newFrame)
{
  int32_t enabled = 0;
  int frm_num;
  uint32_t  skip_mode;
  char value[PROPERTY_VALUE_MAX];
  char buf[32];
  int w, h;
  static int count = 0;
  cam_ctrl_dimension_t dim;
  int file_fd;
  int rc = 0;
  int len;
  unsigned long addr;
  unsigned long * tmp = (unsigned long *)newFrame->buffer;
  addr = *tmp;
  status_t ret = cam_config_get_parm(mHalCamCtrl->mCameraId,
                 MM_CAMERA_PARM_DIMENSION, &dim);

  w = dim.display_width;
  h = dim.display_height;
  len = (w * h)*3/2;
  count++;
  if(count < 100) {
    snprintf(buf, sizeof(buf), "/data/misc/camera/mzhu%d.yuv", count);
    file_fd = open(buf, O_RDWR | O_CREAT, 0777);

    rc = write(file_fd, (const void *)addr, len);
    ALOGE("%s: file='%s', vaddr_old=0x%x, addr_map = 0x%p, len = %d, rc = %d",
          __func__, buf, (uint32_t)newFrame->buffer, (void *)addr, len, rc);
    close(file_fd);
    ALOGE("%s: dump %s, rc = %d, len = %d", __func__, buf, rc, len);
  }
}

status_t QCameraStream_preview::processPreviewFrameWithDisplay(
  mm_camera_ch_data_buf_t *frame)
{
  ALOGV("%s",__func__);
  int err = 0;
  int msgType = 0;
  int i;
  camera_memory_t *data = NULL;
  camera_frame_metadata_t *metadata = NULL;

  Mutex::Autolock lock(mStopCallbackLock);
  if(!mActive) {
    ALOGE("Preview Stopped. Returning callback");
    return NO_ERROR;
  }

  if(mHalCamCtrl==NULL) {
    ALOGE("%s: X: HAL control object not set",__func__);
    /*Call buf done*/
    return BAD_VALUE;
  }
  mHalCamCtrl->mCallbackLock.lock();
  camera_data_timestamp_callback rcb = mHalCamCtrl->mDataCbTimestamp;
  void *rdata = mHalCamCtrl->mCallbackCookie;
  mHalCamCtrl->mCallbackLock.unlock();
  nsecs_t timeStamp = seconds_to_nanoseconds(frame->def.frame->ts.tv_sec) ;
  timeStamp += frame->def.frame->ts.tv_nsec;

  if(mFirstFrameRcvd == false) {
  mm_camera_util_profile("HAL: First preview frame received");
  mFirstFrameRcvd = true;
  }

  if (UNLIKELY(mHalCamCtrl->mDebugFps)) {
      mHalCamCtrl->debugShowPreviewFPS();
  }
  //dumpFrameToFile(frame->def.frame);
  mHalCamCtrl->dumpFrameToFile(frame->def.frame, HAL_DUMP_FRM_PREVIEW);

  mHalCamCtrl->mPreviewMemoryLock.lock();
  mNotifyBuffer[frame->def.idx] = *frame;

  ALOGI("Enqueue buf handle %p\n",
  mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);

#ifdef USE_ION
  struct ion_flush_data cache_inv_data;
  int ion_fd;
  ion_fd = frame->def.frame->ion_dev_fd;
  cache_inv_data.vaddr = (void *)frame->def.frame->buffer;
  cache_inv_data.fd = frame->def.frame->fd;
  cache_inv_data.handle = frame->def.frame->fd_data.handle;
  cache_inv_data.length = frame->def.frame->ion_alloc.len;

  if (mHalCamCtrl->cache_ops(ion_fd, &cache_inv_data, ION_IOC_CLEAN_CACHES) < 0)
    ALOGE("%s: Cache clean for Preview buffer %p fd = %d failed", __func__,
      cache_inv_data.vaddr, cache_inv_data.fd);
  if (mHalCamCtrl->cache_ops(ion_fd, &cache_inv_data,  ION_IOC_CLEAN_INV_CACHES) < 0)
    ALOGE("%s: Cache clean for Preview buffer %p fd = %d failed", __func__,
      cache_inv_data.vaddr, cache_inv_data.fd);
#endif

  if(mHFRFrameSkip == 1)
  {
      const char *str = mHalCamCtrl->mParameters.get(
                          QCameraParameters::KEY_QC_VIDEO_HIGH_FRAME_RATE);
      if(str != NULL){
      int is_hfr_off = 0;
      mHFRFrameCnt++;
      if(!strcmp(str, QCameraParameters::VIDEO_HFR_OFF)) {
          is_hfr_off = 1;
          err = this->mPreviewWindow->enqueue_buffer(this->mPreviewWindow,
            (buffer_handle_t *)mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
      } else if (!strcmp(str, QCameraParameters::VIDEO_HFR_2X)) {
          mHFRFrameCnt %= 2;
      } else if (!strcmp(str, QCameraParameters::VIDEO_HFR_3X)) {
          mHFRFrameCnt %= 3;
      } else if (!strcmp(str, QCameraParameters::VIDEO_HFR_4X)) {
          mHFRFrameCnt %= 4;
      }
      if(mHFRFrameCnt == 0)
          err = this->mPreviewWindow->enqueue_buffer(this->mPreviewWindow,
            (buffer_handle_t *)mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
      else if(!is_hfr_off)
          err = this->mPreviewWindow->cancel_buffer(this->mPreviewWindow,
            (buffer_handle_t *)mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
      } else
          err = this->mPreviewWindow->enqueue_buffer(this->mPreviewWindow,
            (buffer_handle_t *)mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
  } else {
      err = this->mPreviewWindow->enqueue_buffer(this->mPreviewWindow,
          (buffer_handle_t *)mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
  }
  if(err != 0) {
    ALOGE("%s: enqueue_buffer failed, err = %d", __func__, err);
  } else {
   ALOGD("%s: enqueue_buffer hdl=%p", __func__, *mHalCamCtrl->mPreviewMemory.buffer_handle[frame->def.idx]);
    mHalCamCtrl->mPreviewMemory.local_flag[frame->def.idx] = BUFFER_NOT_OWNED;
  }
  buffer_handle_t *buffer_handle = NULL;
  int tmp_stride = 0;
  err = this->mPreviewWindow->dequeue_buffer(this->mPreviewWindow,
              &buffer_handle, &tmp_stride);
  if (err == NO_ERROR && buffer_handle != NULL) {

    ALOGD("%s: dequed buf hdl =%p", __func__, *buffer_handle);
    for(i = 0; i < mHalCamCtrl->mPreviewMemory.buffer_count; i++) {
        if(mHalCamCtrl->mPreviewMemory.buffer_handle[i] == buffer_handle) {
          mHalCamCtrl->mPreviewMemory.local_flag[i] = BUFFER_OWNED;
          break;
        }
    }
    if (i < mHalCamCtrl->mPreviewMemory.buffer_count ) {
      err = this->mPreviewWindow->lock_buffer(this->mPreviewWindow, buffer_handle);
      if(MM_CAMERA_OK != cam_evt_buf_done(mCameraId, &mNotifyBuffer[i])) {
        ALOGE("BUF DONE FAILED");
      }
    }
  } else
      ALOGE("%s: error in dequeue_buffer, enqueue_buffer idx = %d, no free buffer now", __func__, frame->def.idx);
  /* Save the last displayed frame. We'll be using it to fill the gap between
     when preview stops and postview start during snapshot.*/
  mLastQueuedFrame = &(mDisplayStreamBuf.frame[frame->def.idx]);
  mHalCamCtrl->mPreviewMemoryLock.unlock();

  mHalCamCtrl->mCallbackLock.lock();
  camera_data_callback pcb = mHalCamCtrl->mDataCb;
  mHalCamCtrl->mCallbackLock.unlock();
  ALOGD("Message enabled = 0x%x", mHalCamCtrl->mMsgEnabled);

  camera_memory_t *previewMem = NULL;

  if (pcb != NULL) {
       ALOGD("%s: mMsgEnabled =0x%x, preview format =%d", __func__,
            mHalCamCtrl->mMsgEnabled, mHalCamCtrl->mPreviewFormat);
      //Sending preview callback if corresponding Msgs are enabled
      if(mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
          ALOGE("Q%s: PCB callback enabled", __func__);
          msgType |=  CAMERA_MSG_PREVIEW_FRAME;
          int previewBufSize;
          /* The preview buffer size sent back in the callback should be (width*height*bytes_per_pixel)
           * As all preview formats we support, use 12 bits per pixel, buffer size = previewWidth * previewHeight * 3/2.
           * We need to put a check if some other formats are supported in future. (punits) */
          if((mHalCamCtrl->mPreviewFormat == CAMERA_YUV_420_NV21) || (mHalCamCtrl->mPreviewFormat == CAMERA_YUV_420_NV12) ||
                    (mHalCamCtrl->mPreviewFormat == CAMERA_YUV_420_YV12))
          {
            if(mHalCamCtrl->mPreviewFormat == CAMERA_YUV_420_YV12) {
              previewBufSize = ((mHalCamCtrl->mPreviewWidth+15)/16) *16* mHalCamCtrl->mPreviewHeight +
                ((mHalCamCtrl->mPreviewWidth/2+15)/16)*16* mHalCamCtrl->mPreviewHeight;
            } else {
              previewBufSize = mHalCamCtrl->mPreviewWidth * mHalCamCtrl->mPreviewHeight * 3/2;
            }
              if(previewBufSize != mHalCamCtrl->mPreviewMemory.private_buffer_handle[frame->def.idx]->size) {
                  previewMem = mHalCamCtrl->mGetMemory(mHalCamCtrl->mPreviewMemory.private_buffer_handle[frame->def.idx]->fd,
                  previewBufSize, 1, mHalCamCtrl->mCallbackCookie);
                  if (!previewMem || !previewMem->data) {
                      ALOGE("%s: mGetMemory failed.\n", __func__);
                  } else {
                      data = previewMem;
                  }
              } else
                    data = mHalCamCtrl->mPreviewMemory.camera_memory[frame->def.idx];
          } else {
                data = mHalCamCtrl->mPreviewMemory.camera_memory[frame->def.idx];
                ALOGE("Invalid preview format, buffer size in preview callback may be wrong.");
          }
      } else {
          data = NULL;
      }
      if(msgType) {
          mStopCallbackLock.unlock();
          if(mActive)
            pcb(msgType, data, 0, metadata, mHalCamCtrl->mCallbackCookie);
          if (previewMem)
              previewMem->release(previewMem);
      }
	  ALOGD("end of cb");
  } else {
    ALOGD("%s PCB is not enabled", __func__);
  }
  if(rcb != NULL && mVFEOutputs == 1)
  {
      int flagwait = 1;
      if(mHalCamCtrl->mStartRecording == true &&
              ( mHalCamCtrl->mMsgEnabled & CAMERA_MSG_VIDEO_FRAME))
      {
        if (mHalCamCtrl->mStoreMetaDataInFrame)
        {
          if(mHalCamCtrl->mRecordingMemory.metadata_memory[frame->def.idx])
          {
              flagwait = 1;
              mStopCallbackLock.unlock();
              rcb(timeStamp, CAMERA_MSG_VIDEO_FRAME,
                      mHalCamCtrl->mRecordingMemory.metadata_memory[frame->def.idx],
                      0, mHalCamCtrl->mCallbackCookie);
          }else
              flagwait = 0;
      }
      else
      {
              mStopCallbackLock.unlock();
              rcb(timeStamp, CAMERA_MSG_VIDEO_FRAME,
                      mHalCamCtrl->mPreviewMemory.camera_memory[frame->def.idx],
                      0, mHalCamCtrl->mCallbackCookie);
      }

      if(flagwait){
          Mutex::Autolock rLock(&mHalCamCtrl->mRecordFrameLock);
          if (mHalCamCtrl->mReleasedRecordingFrame != true) {
              mHalCamCtrl->mRecordWait.wait(mHalCamCtrl->mRecordFrameLock);
          }
          mHalCamCtrl->mReleasedRecordingFrame = false;
      }
      }
  }
  /* Save the last displayed frame. We'll be using it to fill the gap between
     when preview stops and postview start during snapshot.*/
  //mLastQueuedFrame = frame->def.frame;
/*
  if(MM_CAMERA_OK != cam_evt_buf_done(mCameraId, frame))
  {
      ALOGE("BUF DONE FAILED");
      return BAD_VALUE;
  }
*/
  return NO_ERROR;
}


status_t QCameraStream_preview::processPreviewFrameWithOutDisplay(
  mm_camera_ch_data_buf_t *frame)
{
  ALOGV("%s",__func__);
  int err = 0;
  int msgType = 0;
  int i;
  camera_memory_t *data = NULL;
  camera_frame_metadata_t *metadata = NULL;

  Mutex::Autolock lock(mStopCallbackLock);
  if(!mActive) {
    ALOGE("Preview Stopped. Returning callback");
    return NO_ERROR;
  }
  if(mHalCamCtrl==NULL) {
    ALOGE("%s: X: HAL control object not set",__func__);
    /*Call buf done*/
    return BAD_VALUE;
  }

  if (UNLIKELY(mHalCamCtrl->mDebugFps)) {
      mHalCamCtrl->debugShowPreviewFPS();
  }
  //dumpFrameToFile(frame->def.frame);
  mHalCamCtrl->dumpFrameToFile(frame->def.frame, HAL_DUMP_FRM_PREVIEW);

  mHalCamCtrl->mPreviewMemoryLock.lock();
  mNotifyBuffer[frame->def.idx] = *frame;

  /* Save the last displayed frame. We'll be using it to fill the gap between
     when preview stops and postview start during snapshot.*/
  mLastQueuedFrame = &(mDisplayStreamBuf.frame[frame->def.idx]);
  mHalCamCtrl->mPreviewMemoryLock.unlock();

  mHalCamCtrl->mCallbackLock.lock();
  camera_data_callback pcb = mHalCamCtrl->mDataCb;
  mHalCamCtrl->mCallbackLock.unlock();
  ALOGD("Message enabled = 0x%x", mHalCamCtrl->mMsgEnabled);

  camera_memory_t *previewMem = NULL;
  int previewWidth, previewHeight;
  mHalCamCtrl->mParameters.getPreviewSize(&previewWidth, &previewHeight);

#ifdef USE_ION
  struct ion_flush_data cache_inv_data;
  int ion_fd;
  ion_fd = frame->def.frame->ion_dev_fd;
  cache_inv_data.vaddr = (void *)frame->def.frame->buffer;
  cache_inv_data.fd = frame->def.frame->fd;
  cache_inv_data.handle = frame->def.frame->fd_data.handle;
  cache_inv_data.length = frame->def.frame->ion_alloc.len;

  if (mHalCamCtrl->cache_ops(ion_fd, &cache_inv_data, ION_IOC_CLEAN_CACHES) < 0)
    ALOGE("%s: Cache clean for Preview buffer %p fd = %d failed", __func__,
      cache_inv_data.vaddr, cache_inv_data.fd);
#endif

  if (pcb != NULL) {
      //Sending preview callback if corresponding Msgs are enabled
      if(mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
          msgType |=  CAMERA_MSG_PREVIEW_FRAME;
          int previewBufSize;
          /* For CTS : Forcing preview memory buffer lenth to be
             'previewWidth * previewHeight * 3/2'.
              Needed when gralloc allocated extra memory.*/
          //Can add this check for other formats as well.
          if( mHalCamCtrl->mPreviewFormat == CAMERA_YUV_420_NV21) {
              previewBufSize = previewWidth * previewHeight * 3/2;
              if(previewBufSize != mHalCamCtrl->mPreviewMemory.private_buffer_handle[frame->def.idx]->size) {
                  previewMem = mHalCamCtrl->mGetMemory(mHalCamCtrl->mPreviewMemory.private_buffer_handle[frame->def.idx]->fd,
                  previewBufSize, 1, mHalCamCtrl->mCallbackCookie);
                  if (!previewMem || !previewMem->data) {
                      ALOGE("%s: mGetMemory failed.\n", __func__);
                  } else {
                      data = previewMem;
                  }
              } else
                    data = mHalCamCtrl->mPreviewMemory.camera_memory[frame->def.idx];//mPreviewHeap->mBuffers[frame->def.idx];
          } else
                data = mHalCamCtrl->mPreviewMemory.camera_memory[frame->def.idx];//mPreviewHeap->mBuffers[frame->def.idx];
      } else {
          data = NULL;
      }

      if(mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_METADATA){
          msgType  |= CAMERA_MSG_PREVIEW_METADATA;
          metadata = &mHalCamCtrl->mMetadata;
      } else {
          metadata = NULL;
      }
      if(msgType) {
          mStopCallbackLock.unlock();
          if(mActive)
            pcb(msgType, data, 0, metadata, mHalCamCtrl->mCallbackCookie);
          if (previewMem)
              previewMem->release(previewMem);
      }

      if(MM_CAMERA_OK != cam_evt_buf_done(mCameraId, &mNotifyBuffer[frame->def.idx])) {
          ALOGE("BUF DONE FAILED");
      }

      ALOGD("end of cb");
  }

  return NO_ERROR;
}

status_t QCameraStream_preview::processPreviewFrame (
  mm_camera_ch_data_buf_t *frame)
{
  if (mHalCamCtrl->isNoDisplayMode()) {
    return processPreviewFrameWithOutDisplay(frame);
  } else {
    return processPreviewFrameWithDisplay(frame);
  }
}

// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------

QCameraStream_preview::
QCameraStream_preview(int cameraId, camera_mode_t mode)
  : QCameraStream(cameraId,mode),
    mFirstFrameRcvd(false),
    mLastQueuedFrame(NULL),
    mNumFDRcvd(0)
  {
    mHalCamCtrl = NULL;
    ALOGE("%s: E", __func__);
    ALOGE("%s: X", __func__);
  }
// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------

QCameraStream_preview::~QCameraStream_preview() {
    ALOGV("%s: E", __func__);
	if(mActive) {
		stop();
	}
	if(mInit) {
		release();
	}
	mInit = false;
	mActive = false;
    ALOGV("%s: X", __func__);

}
// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------

status_t QCameraStream_preview::init() {

  status_t ret = NO_ERROR;
  ALOGV("%s: E", __func__);

  ret = QCameraStream::initChannel (mCameraId, MM_CAMERA_CH_PREVIEW_MASK);
  if (NO_ERROR!=ret) {
    ALOGE("%s E: can't init native cammera preview ch\n",__func__);
    return ret;
  }

  ALOGE("Debug : %s : initChannel",__func__);
  /* register a notify into the mmmm_camera_t object*/
  (void) cam_evt_register_buf_notify(mCameraId, MM_CAMERA_CH_PREVIEW,
                                     preview_notify_cb,
                                     MM_CAMERA_REG_BUF_CB_INFINITE,
                                     0,this);
  ALOGE("Debug : %s : cam_evt_register_buf_notify",__func__);
  buffer_handle_t *buffer_handle = NULL;
  int tmp_stride = 0;
  mInit = true;
  return ret;
}
// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------

status_t QCameraStream_preview::start()
{
    ALOGV("%s: E", __func__);
    status_t ret = NO_ERROR;
    cam_format_t previewFmt;

    Mutex::Autolock lock(mStopCallbackLock);

    /* call start() in parent class to start the monitor thread*/
    //QCameraStream::start ();
    previewFmt = mHalCamCtrl->getPreviewFormat();
    setFormat(MM_CAMERA_CH_PREVIEW_MASK, previewFmt);

    if (mHalCamCtrl->isNoDisplayMode()) {
        if(NO_ERROR!=initPreviewOnlyBuffers()){
            return BAD_VALUE;
        }
    } else {
        if(NO_ERROR!=initDisplayBuffers()){
            return BAD_VALUE;
        }
    }
    ALOGE("Debug : %s : initDisplayBuffers",__func__);

    ret = cam_config_prepare_buf(mCameraId, &mDisplayBuf);
    ALOGE("Debug : %s : cam_config_prepare_buf",__func__);
    if(ret != MM_CAMERA_OK) {
        ALOGV("%s:reg preview buf err=%d\n", __func__, ret);
        ret = BAD_VALUE;
        goto error;
    }else {
        ret = NO_ERROR;
    }

	/* For preview, the OP_MODE we set is dependent upon whether we are
       starting camera or camcorder. For snapshot, anyway we disable preview.
       However, for ZSL we need to set OP_MODE to OP_MODE_ZSL and not
       OP_MODE_VIDEO. We'll set that for now in CamCtrl. So in case of
       ZSL we skip setting Mode here */

    if (!(myMode & CAMERA_ZSL_MODE)) {
        ALOGE("Setting OP MODE to MM_CAMERA_OP_MODE_VIDEO");
        mm_camera_op_mode_type_t op_mode=MM_CAMERA_OP_MODE_VIDEO;
        ret = cam_config_set_parm (mCameraId, MM_CAMERA_PARM_OP_MODE,
                                        &op_mode);
        ALOGE("OP Mode Set");

        if(MM_CAMERA_OK != ret) {
          ALOGE("%s: X :set mode MM_CAMERA_OP_MODE_VIDEO err=%d\n", __func__, ret);
          ret = BAD_VALUE;
          goto error;
        }
    }else {
        ALOGE("Setting OP MODE to MM_CAMERA_OP_MODE_ZSL");
        mm_camera_op_mode_type_t op_mode=MM_CAMERA_OP_MODE_ZSL;
        ret = cam_config_set_parm (mCameraId, MM_CAMERA_PARM_OP_MODE,
                                        &op_mode);
        if(MM_CAMERA_OK != ret) {
          ALOGE("%s: X :set mode MM_CAMERA_OP_MODE_ZSL err=%d\n", __func__, ret);
          ret = BAD_VALUE;
          goto error;
        }
     }

    /* call mm_camera action start(...)  */
    ALOGE("Starting Preview/Video Stream. ");
    mFirstFrameRcvd = false;
    ret = cam_ops_action(mCameraId, TRUE, MM_CAMERA_OPS_PREVIEW, 0);

    if (MM_CAMERA_OK != ret) {
      ALOGE ("%s: preview streaming start err=%d\n", __func__, ret);
      ret = BAD_VALUE;
      goto error;
    }

    ALOGE("Debug : %s : Preview streaming Started",__func__);
    ret = NO_ERROR;

    mActive =  true;
    goto end;

error:
    putBufferToSurface();
end:
    ALOGE("%s: X", __func__);
    return ret;
  }


// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------
  void QCameraStream_preview::stop() {
    ALOGE("%s: E", __func__);
    int ret=MM_CAMERA_OK;

    if(!mActive) {
      return;
    }
    Mutex::Autolock lock(mStopCallbackLock);
    mActive =  false;
    /* unregister the notify fn from the mmmm_camera_t object*/

    ALOGI("%s: Stop the thread \n", __func__);
    /* call stop() in parent class to stop the monitor thread*/
    ret = cam_ops_action(mCameraId, FALSE, MM_CAMERA_OPS_PREVIEW, 0);
    if(MM_CAMERA_OK != ret) {
      ALOGE ("%s: camera preview stop err=%d\n", __func__, ret);
    }
    ret = cam_config_unprepare_buf(mCameraId, MM_CAMERA_CH_PREVIEW);
    if(ret != MM_CAMERA_OK) {
      ALOGE("%s:Unreg preview buf err=%d\n", __func__, ret);
      //ret = BAD_VALUE;
    }

    /* In case of a clean stop, we need to clean all buffers*/
    ALOGE("Debug : %s : Buffer Unprepared",__func__);
    /*free camera_memory handles and return buffer back to surface*/
    if (! mHalCamCtrl->isNoDisplayMode() ) {
      putBufferToSurface();
    } else {
      freeBufferNoDisplay( );
    }

    ALOGE("%s: X", __func__);

  }
// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------
  void QCameraStream_preview::release() {

    ALOGE("%s : BEGIN",__func__);
    int ret=MM_CAMERA_OK,i;

    if(!mInit)
    {
      ALOGE("%s : Stream not Initalized",__func__);
      return;
    }

    if(mActive) {
      this->stop();
    }

    ret= QCameraStream::deinitChannel(mCameraId, MM_CAMERA_CH_PREVIEW);
    ALOGE("Debug : %s : De init Channel",__func__);
    if(ret != MM_CAMERA_OK) {
      ALOGE("%s:Deinit preview channel failed=%d\n", __func__, ret);
      //ret = BAD_VALUE;
    }

    (void)cam_evt_register_buf_notify(mCameraId, MM_CAMERA_CH_PREVIEW,
                                      NULL,
                                      (mm_camera_register_buf_cb_type_t)NULL,
                                      0,
                                      (void*)NULL);
    mInit = false;
    ALOGE("%s: END", __func__);

  }

QCameraStream*
QCameraStream_preview::createInstance(int cameraId,
                                      camera_mode_t mode)
{
  QCameraStream* pme = new QCameraStream_preview(cameraId, mode);
  return pme;
}
// ---------------------------------------------------------------------------
// QCameraStream_preview
// ---------------------------------------------------------------------------

void QCameraStream_preview::deleteInstance(QCameraStream *p)
{
  if (p){
    ALOGV("%s: BEGIN", __func__);
    p->release();
    delete p;
    p = NULL;
    ALOGV("%s: END", __func__);
  }
}


/* Temp helper function */
void *QCameraStream_preview::getLastQueuedFrame(void)
{
    return mLastQueuedFrame;
}

// ---------------------------------------------------------------------------
// No code beyone this line
// ---------------------------------------------------------------------------
}; // namespace android
