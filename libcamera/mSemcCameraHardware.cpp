/*
** Copyright 2008, Google Inc.
** Copyright (c) 2009, Code Aurora Forum. All rights reserved.
** Copyright 2010, 2011, Sony Ericsson Mobile Communciations AB
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

//#define CAMHW_DEBUG

#define CAMHW_DEBUG

#ifdef CAMHW_DEBUG
#define ALOG_NDEBUG 1
#endif

#define SCRITCH_OFF 0

#define LOG_TAG "SemcCameraHardware"
#include <cutils/log.h>

#ifndef CAMHW_DEBUG
#undef ALOGD
#undef ALOGD
#undef ALOGI
#undef ALOGD
#define ALOGD(...) ((void)0)
#define ALOGD(...) ((void)0)
#define ALOGI(...) ((void)0)
#define ALOGD(...) ((void)0)
#endif

#undef ALOGV
#define ALOGV ALOGD
#include "SemcCameraHardware.h"

#include <utils/Errors.h>
#include <utils/threads.h>
#include <binder/MemoryHeapPmem.h>
#include <utils/String16.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <math.h>
#if HAVE_ANDROID_OS
#include <linux/android_pmem.h>
#endif
#include <linux/ioctl.h>
#include <camera/CameraParameters.h>

#include "linux/msm_mdp.h"
#include <linux/fb.h>

#include <CameraHardwareInterface.h>

#define LIKELY(exp)   __builtin_expect(!!(exp), 1)
#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)
#define MSM_CAMERA_CONTROL "/dev/msm_camera/control0"
typedef  unsigned char      boolean;     /* Boolean value type. */
typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
typedef  signed long int    int32;       /* Signed 32 bit value */
typedef  signed short       int16;       /* Signed 16 bit value */
typedef  signed char        int8;        /* Signed 8  bit value */
typedef  unsigned char      byte;        /* Unsigned 8  bit value type. */

#define MDP_CSS_RGB2YUV 0
#define MDP_CCS_YUV2RGB 1


extern "C" {
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/system_properties.h>
#include <sys/time.h>
#include <stdlib.h>

#include <msm_camera.h>

#define DEFAULT_PICTURE_WIDTH 3264 
#define DEFAULT_PICTURE_HEIGHT 2448

#define DEFAULT_PREVIEW_FRAME_RATE    15
#define DEFAULT_PREVIEW_FRAME_RATE_EX 30

#define CAMERA_HORIZONTAL_VIEW_ANGLE_DEFAULT 52
#define CAMERA_VERTICAL_VIEW_ANGLE_DEFAULT  40

#define CAMERA_FOCAL_LENGTH_DEFAULT 4.65
#define FOCAL_LENGTH_DECIMAL_PRECISON 465


#define THUMBNAIL_BUFFER_SIZE (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3/2)
#define MAX_ZOOM_LEVEL 5
#define NOT_FOUND -1
// Number of video buffers held by kernal (initially 1,2 &3)
#define ACTIVE_VIDEO_BUFFERS 3

#if DLOPEN_LIBMMCAMERA
#include <dlfcn.h>

void* (*LINK_cam_conf)(void *data);
void* (*LINK_cam_frame)(void *data);
bool  (*LINK_jpeg_encoder_init)();
void  (*LINK_jpeg_encoder_join)();
bool  (*LINK_jpeg_encoder_encode)(const cam_ctrl_dimension_t *dimen,
                                  const uint8_t *thumbnailbuf, int thumbnailfd,
                                  const uint8_t *snapshotbuf, int snapshotfd,
                                  common_crop_t *scaling_parms, exif_tags_info_t *exif_data,
                                  int exif_table_numEntries, int jpegPadding);
void (*LINK_camframe_terminate)(void);
//for 720p
// Function to add a video buffer to free Q
void (*LINK_camframe_free_video)(struct msm_frame *frame);
// Function pointer , called by camframe when a video frame is available.
void (**LINK_camframe_video_callback)(struct msm_frame * frame);
// To flush free Q in cam frame.
void (*LINK_cam_frame_flush_free_video)(void);

int8_t (*LINK_jpeg_encoder_setMainImageQuality)(uint32_t quality);
int8_t (*LINK_jpeg_encoder_setThumbnailQuality)(uint32_t quality);
int8_t (*LINK_jpeg_encoder_setRotation)(uint32_t rotation);
int8_t (*LINK_jpeg_encoder_setLocation)(const camera_position_type *location);
const struct camera_size_type *(*LINK_default_sensor_get_snapshot_sizes)(int *len);
int (*LINK_launch_cam_conf_thread)(void);
int (*LINK_release_cam_conf_thread)(void);
int8_t (*LINK_zoom_crop_upscale)(uint32_t width, uint32_t height,
    uint32_t cropped_width, uint32_t cropped_height, uint8_t *img_buf);

// callbacks
void  (**LINK_mmcamera_camframe_callback)(struct msm_frame *frame);
void  (**LINK_mmcamera_jpegfragment_callback)(uint8_t *buff_ptr,
                                              uint32_t buff_size);
void  (**LINK_mmcamera_jpeg_callback)(jpeg_event_t status);
void  (**LINK_mmcamera_shutter_callback)(common_crop_t *crop);
void  (**LINK_camframe_timeout_callback)(void);
#else
#define LINK_cam_conf cam_conf
#define LINK_cam_frame cam_frame
#define LINK_jpeg_encoder_init jpeg_encoder_init
#define LINK_jpeg_encoder_join jpeg_encoder_join
#define LINK_jpeg_encoder_encode jpeg_encoder_encode
#define LINK_camframe_terminate camframe_terminate
#define LINK_jpeg_encoder_setMainImageQuality jpeg_encoder_setMainImageQuality
#define LINK_jpeg_encoder_setThumbnailQuality jpeg_encoder_setThumbnailQuality
#define LINK_jpeg_encoder_setRotation jpeg_encoder_setRotation
#define LINK_jpeg_encoder_setLocation jpeg_encoder_setLocation
#define LINK_default_sensor_get_snapshot_sizes default_sensor_get_snapshot_sizes
#define LINK_launch_cam_conf_thread launch_cam_conf_thread
#define LINK_release_cam_conf_thread release_cam_conf_thread
#define LINK_zoom_crop_upscale zoom_crop_upscale
extern void (*mmcamera_camframe_callback)(struct msm_frame *frame);
extern void (*mmcamera_jpegfragment_callback)(uint8_t *buff_ptr,
                                      uint32_t buff_size);
extern void (*mmcamera_jpeg_callback)(jpeg_event_t status);
extern void (*mmcamera_shutter_callback)(common_crop_t *crop);
#endif

typedef struct ipl_image_struct{
    uint32_t  dx;            // Number of pixels in the x dirctn or in a row
    uint32_t  dy;            // Number of pixels in the y dirctn or in a col
    uint32_t cFormat;        // JUST A GUESS
    unsigned char* imgPtr; // Pointer to the image data
    uint8_t* clrPtr;         // Pointer to the Color data

}ipl_image_type;

#define ipl_bip_type void

int (*LINK_ipl_downsize)(ipl_image_type* input_img_ptr,
                                     ipl_image_type* output_img_ptr,
                                     ipl_bip_type*   bip_ptr);




/* #define CAMERA_FIRMWARE_UPDATE */
#ifdef  CAMERA_FIRMWARE_UPDATE
#define SENSOR_FIRMWARE_UPDATE_RESULT_INVALID   666
void (*LINK_sensor_set_firmware_fd)(int);
int  (*LINK_sensor_get_firmware_update_result)(void);
#endif // CAMERA_FIRMWARE_UPDATE

#define NV_CAM_SCENE1_STRUCT_SIZE  24
#define NV_CAM_SCENE2_STRUCT_SIZE  84
#define NV_CAM_SCENE3_STRUCT_SIZE  84
#define NV_CAM_SCENE4_STRUCT_SIZE  64
#define NV_CAM_ALL_SCENE_STRUCT_SIZE (NV_CAM_SCENE1_STRUCT_SIZE \
                                    + NV_CAM_SCENE2_STRUCT_SIZE \
                                    + NV_CAM_SCENE3_STRUCT_SIZE \
                                    + NV_CAM_SCENE4_STRUCT_SIZE)
static bool scene_nv_data_initialized = false;
static uint8_t scene_nv_data[NV_CAM_ALL_SCENE_STRUCT_SIZE];

} // extern "C"

typedef struct crop_info_struct {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} zoom_crop_info;

union zoomimage
{
    char d[sizeof(struct mdp_blit_req_list) + sizeof(struct mdp_blit_req) * 1];
    struct mdp_blit_req_list list;
} zoomImage;

//Default to VGA
#define DEFAULT_PREVIEW_WIDTH 640
#define DEFAULT_PREVIEW_HEIGHT 480

board_property boardProperties[] = {
        {TARGET_MSM7625, 0x00000fff},
        {TARGET_MSM7627, 0x000006ff},
        {TARGET_MSM7630, 0x00000fff},
        {TARGET_QSD8250, 0x00000fff}
};

static int supportedPictureSizesCount;

#ifdef Q12
#undef Q12
#endif

#define Q12 4096

static const target_map targetList [] = {
    { "msm7625", TARGET_MSM7625 },
    { "msm7627", TARGET_MSM7627 },
    { "qsd8250", TARGET_QSD8250 },
    { "msm7630", TARGET_MSM7630 }
};
static targetType mCurrentTarget = TARGET_MAX;
static struct mdp_ccs mdp_ccs_save;

struct mdp_ccs mdp_ccs_hd={
    MDP_CCS_YUV2RGB,
        {0x254,0x000,0x331,0x254,0xff38,0xfe61,0x254,0x409,0x000,},
        {0x1f0,0x180,0x180}
    };

#define DEFAULT_THUMBNAIL_SETTING 2
#define THUMBNAIL_WIDTH_STR "160"
#define THUMBNAIL_HEIGHT_STR "120"
#define THUMBNAIL_SMALL_HEIGHT 144

static int attr_lookup(const str_map arr[], int len, const char *name, int mw_value = -1)
{
    ALOGD("START attr_lookup name[%s] : mw_value[%d]",name,mw_value);

    if(strcmp(name, CAMERA_DEFAULT_VALUE) != 0) {
        for (int i = 0; i < len; i++) {
            if (!strcmp(arr[i].desc, name)) {
                ALOGD("END attr_lookup : name case return [%d]",arr[i].val);
                return arr[i].val;
            }
        }
    }
    ALOGD("END attr_lookup return NOT_FOUND");
    return NOT_FOUND;
}

// round to the next power of two
static inline unsigned clp2(unsigned x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

static int exif_table_numEntries = 0;
#define MAX_EXIF_TABLE_ENTRIES 33
exif_tags_info_t exif_data[MAX_EXIF_TABLE_ENTRIES];
static zoom_crop_info zoomCropInfo;
static void *mLastQueuedFrame = NULL;

namespace android {

static camera_size_type supportedPreviewSizes[PREVIEW_SIZE_COUNT];
static unsigned int previewSizeCount;

static const int PICTURE_FORMAT_JPEG = 1;

#define DONT_CARE 0

struct SensorType {
    const char *name;
    int rawPictureWidth;
    int rawPictureHeight;
    bool hasAutoFocusSupport;
    int max_supported_snapshot_width;
    int max_supported_snapshot_height;
};

static SensorType sensorTypes[] = {
        { "8mp", 3320, 2468, true,  3264, 2448 },
        { "5mp", 2608, 1960, true,  2592, 1944 },
        { "3mp", 2064, 1544, false, 2048, 1536 },
        { "2mp", 3200, 1200, false, 1600, 1200 } 
};


static SensorType * sensorType;


#define NAME_NOTHING ""

static const str_map whitebalance[] = {
    { CameraParameters::WHITE_BALANCE_AUTO,             CAMERA_WB_AUTO},
    { CameraParameters::WHITE_BALANCE_INCANDESCENT,     CAMERA_WB_INCANDESCENT},
    { CameraParameters::WHITE_BALANCE_FLUORESCENT,      CAMERA_WB_FLUORESCENT},
    { CameraParameters::WHITE_BALANCE_DAYLIGHT,         CAMERA_WB_DAYLIGHT},
    { CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,  CAMERA_WB_CLOUDY_DAYLIGHT}
};

static const str_map autoexposure[] = {
    { CameraParameters::AUTO_EXPOSURE_FRAME_AVG,        CAMERA_AEC_FRAME_AVERAGE},
    { CameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED,  CAMERA_AEC_CENTER_WEIGHTED},
    { CameraParameters::AUTO_EXPOSURE_SPOT_METERING,    CAMERA_AEC_SPOT_METERING}
};

static const str_map picture_formats[] = {
    {CameraParameters::PIXEL_FORMAT_JPEG,               PICTURE_FORMAT_JPEG}
};

static const str_map scene_mode[] = {
    {CameraParameters::SCENE_MODE_AUTO,                 CAMERA_BESTSHOT_AUTO},
    {CameraParameters::SCENE_MODE_PORTRAIT,             CAMERA_BESTSHOT_PORTRAIT},
    {CameraParameters::SCENE_MODE_LANDSCAPE,            CAMERA_BESTSHOT_LANDSCAPE},
    {CameraParameters::SCENE_MODE_NIGHT,                CAMERA_BESTSHOT_NIGHT},
    {CameraParameters::SCENE_MODE_NIGHT_PORTRAIT,       CAMERA_BESTSHOT_NIGHT_PORTRAIT},
    {CameraParameters::SCENE_MODE_BEACH,                CAMERA_BESTSHOT_SNOW},
    {CameraParameters::SCENE_MODE_SNOW,                 CAMERA_BESTSHOT_SNOW},
    {CameraParameters::SCENE_MODE_SPORTS,               CAMERA_BESTSHOT_SPORTS},
    {CameraParameters::SCENE_MODE_PARTY,                CAMERA_BESTSHOT_PARTY}
};
static const str_map exposure_compensation[] = {
    { COMPENSATION_M9,                           CAMERA_EXPOSURE_COMP_M9_3},
    { COMPENSATION_M8,                           CAMERA_EXPOSURE_COMP_M8_3},
    { COMPENSATION_M7,                           CAMERA_EXPOSURE_COMP_M7_3},
    { COMPENSATION_M6,                           CAMERA_EXPOSURE_COMP_M6_3},
    { COMPENSATION_M5,                           CAMERA_EXPOSURE_COMP_M5_3},
    { COMPENSATION_M4,                           CAMERA_EXPOSURE_COMP_M4_3},
    { COMPENSATION_M3,                           CAMERA_EXPOSURE_COMP_M3_3},
    { COMPENSATION_M2,                           CAMERA_EXPOSURE_COMP_M2_3},
    { COMPENSATION_M1,                           CAMERA_EXPOSURE_COMP_M1_3},
    { COMPENSATION_ZERO,                         CAMERA_EXPOSURE_COMP_ZERO},
    { COMPENSATION_P1,                           CAMERA_EXPOSURE_COMP_P1_3},
    { COMPENSATION_P2,                           CAMERA_EXPOSURE_COMP_P2_3},
    { COMPENSATION_P3,                           CAMERA_EXPOSURE_COMP_P3_3},
    { COMPENSATION_P4,                           CAMERA_EXPOSURE_COMP_P4_3},
    { COMPENSATION_P5,                           CAMERA_EXPOSURE_COMP_P5_3},
    { COMPENSATION_P6,                           CAMERA_EXPOSURE_COMP_P6_3},
    { COMPENSATION_P7,                           CAMERA_EXPOSURE_COMP_P7_3},
    { COMPENSATION_P8,                           CAMERA_EXPOSURE_COMP_P8_3},
    { COMPENSATION_P9,                           CAMERA_EXPOSURE_COMP_P9_3}
};



static const str_map focus_modes[] = {
    { CameraParameters::FOCUS_MODE_AUTO,     AF_MODE_NORMAL},
    { CameraParameters::FOCUS_MODE_INFINITY, DONT_CARE },
    { CameraParameters::FOCUS_MODE_MACRO,    AF_MODE_MACRO }
};

static const str_map flash[] = {
    { CameraParameters::FLASH_MODE_OFF,  LED_MODE_OFF },
    //Remove Auto not working
    //{ CameraParameters::FLASH_MODE_AUTO, LED_MODE_AUTO },
    { CameraParameters::FLASH_MODE_ON,   LED_MODE_ON },
    { "torch", LED_MODE_ON }
};


#if SCRITCH_OFF
static const str_map af_mode[] = {
    { CameraParameters::SINGLE,                         CAMERA_AF_FRAME_SINGLE},
    { CameraParameters::MULTI,                          CAMERA_AF_FRAME_MULTI},
    { CameraParameters::AREA,                           CAMERA_AF_FRAME_SPECIFIED}
};

static const str_map caf_mode[] = {
    { CameraParameters::CAF_MODE_OFF,                   false},
    { CameraParameters::CAF_MODE_ON,                    true}
};

static const str_map hjr_mode[] = {
    { CameraParameters::HJR_MODE_OFF,                   false},
    { CameraParameters::HJR_MODE_ON,                    true}
};

static const str_map smile_mode[] = {
    { CameraParameters::SMILE_MODE_OFF,                 DONT_CARE},
    { CameraParameters::SMILE_MODE_ON,                  DONT_CARE}
};

static const str_map face_mode[] = {
    { CameraParameters::FACE_MODE_OFF,                  DONT_CARE},
    { CameraParameters::FACE_MODE_ON,                   DONT_CARE}
};

static const str_map orientation[] = {
    { CameraParameters::ORIENTATION_PORTRAIT,           DONT_CARE},
    { CameraParameters::ORIENTATION_LANDSCAPE,          DONT_CARE}
};

static const str_map confguration[] = {
    { CameraParameters::CONFIGURATION_ORIENTATION,      DONT_CARE}
};

#endif//SCRITCH_OFF


static const str_map antibanding[] = {
    { CameraParameters::ANTIBANDING_OFF,  CAMERA_ANTIBANDING_OFF },
    { CameraParameters::ANTIBANDING_50HZ, CAMERA_ANTIBANDING_50HZ },
    { CameraParameters::ANTIBANDING_60HZ, CAMERA_ANTIBANDING_60HZ },
    { CameraParameters::ANTIBANDING_AUTO, CAMERA_ANTIBANDING_AUTO }
};
static const camera_int_map_type framerate[] = {
    {  7 },
    { 10 },
    { 12 },
    { 15 },
    { 24 },
    { 30 }
};
static const str_map effects[] = {
    { CameraParameters::EFFECT_NONE,                    CAMERA_EFFECT_OFF},
    { CameraParameters::EFFECT_MONO,                    CAMERA_EFFECT_MONO},
    { CameraParameters::EFFECT_NEGATIVE,                CAMERA_EFFECT_NEGATIVE},
    { CameraParameters::EFFECT_SEPIA,                   CAMERA_EFFECT_SEPIA},
    { CameraParameters::EFFECT_POSTERIZE,               CAMERA_EFFECT_POSTERIZE}
};

typedef struct {
    const char* set_key;
    const char* set_value;
} setparam_initialize_t;

setparam_initialize_t initial_setting_value[] = {
    { CameraParameters::KEY_SCENE_MODE,                         CameraParameters::SCENE_MODE_AUTO                 },
    { CameraParameters::KEY_WHITE_BALANCE,                      CameraParameters::WHITE_BALANCE_AUTO              },
    { CameraParameters::KEY_AUTO_EXPOSURE,                      CameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED   },
    { CameraParameters::KEY_FOCUS_MODE,                         CameraParameters::FOCUS_MODE_AUTO                 },
    { CameraParameters::KEY_EXPOSURE_COMPENSATION,               COMPENSATION_ZERO},

    { CameraParameters::KEY_ZOOM,                               "1"},
    { CameraParameters::KEY_ZOOM_SUPPORTED,                     "true"},                      
    { CameraParameters::KEY_ZOOM_RATIOS,                        "100,200,250,300,400,500,600,750"},
    { CameraParameters::KEY_MAX_NUM_DETECTED_FACES_SW,          "1"},
    { CameraParameters::KEY_SUPPORTED_FACE_DETECTION,           "true"},
    { CameraParameters::KEY_FACE_DETECTION,                     CameraParameters::FACE_DETECTION_OFF},
};
#define INITIAL_SETTING_VALUE_COUNT (sizeof(initial_setting_value)/sizeof(setparam_initialize_t))

#define ISO_VALUE_MIN 7
typedef struct {
    unsigned int iso_max;
    unsigned int iso_min;
    unsigned int record_value;
} exifinfo_iso_t;

exifinfo_iso_t iso_record_value[] = {
    {    8,   11,   10  },
    {   12,   14,   12  },
    {   15,   17,   16  },
    {   18,   22,   20  },
    {   23,   28,   25  },
    {   29,   35,   32  },
    {   36,   44,   40  },
    {   45,   56,   50  },
    {   57,   71,   64  },
    {   72,   89,   80  },
    {   90,  112,  100  },
    {  113,  141,  125  },
    {  142,  178,  160  },
    {  179,  224,  200  },
    {  225,  282,  250  },
    {  283,  356,  320  },
    {  357,  449,  400  },
    {  450,  565,  500  },
    {  566,  712,  640  },
    {  713,  890,  800  },
    {  891, 1122, 1000  },
    { 1123, 1414, 1250  },
    { 1415, 1782, 1600  },
    { 1783, 2245, 2000  },
    { 2246, 2828, 2500  },
    { 2829, 3564, 3200  },
    { 3565, 4490, 4000  },
    { 4491, 5657, 5000  },
    { 5658, 7127, 6400  },
    { 7128, 8909, 8000  },
};
#define ISO_RECORD_VALUE_COUNT (sizeof(iso_record_value)/sizeof(exifinfo_iso_t))

static bool parameter_string_initialized = false;
static String8 preview_size_values;
static String8 video_size_values;
static String8 picture_size_values;
static String8 antibanding_values;
static String8 effect_values;
static String8 autoexposure_values;
static String8 whitebalance_values;
static String8 flash_values;
static String8 focus_mode_values;
static String8 picture_format_values;
static String8 framerate_values;
static String8 thumbnail_size_values;

static String8 scene_mode_values;
static Mutex g_RegisterBufRunningLock;


//thumbnail sizes
static const camera_size_type supportedThumbnailSizes[] = {
    { 160, 120 },
    { 0, 0 } // for CTS
 };

static String8 create_sizes_str(const camera_size_type *sizes, int len) {
    String8 str;
    char buffer[32];

    if (len > 0) {
        sprintf(buffer, "%dx%d", sizes[0].width, sizes[0].height);
        str.append(buffer);
    }
    for (int i = 1; i < len; i++) {
        sprintf(buffer, ",%dx%d", sizes[i].width, sizes[i].height);
        str.append(buffer);
    }
    return str;
}

static String8 create_values_int(const camera_int_map_type *values, int len) {
    String8 str;
    char buffer[CAMERA_INT_MAP_MAX_DIGIT];

    if (len > 0) {
        sprintf(buffer, "%d", values[0].value);
        str.append(buffer);
    }
    for (int i = 1; i < len; i++) {
        sprintf(buffer, ",%d", values[i].value);
        str.append(buffer);
    }
    return str;
}

static String8 create_values_str(const str_map *values, int len) {
    String8 str;

    if (len > 0) {
        str.append(values[0].desc);
    }
    for (int i = 1; i < len; i++) {
        str.append(",");
        str.append(values[i].desc);
    }
    return str;
}

extern "C" {
//------------------------------------------------------------------------
//   : 720p busyQ funcitons
//   --------------------------------------------------------------------
static struct fifo_queue g_busy_frame_queue =
    {0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};
};
/*===========================================================================
 * FUNCTION      cam_frame_wait_video
 *
 * DESCRIPTION    this function waits a video in the busy queue
 * ===========================================================================*/

static void cam_frame_wait_video (void)
{
    ALOGD("START cam_frame_wait_video :");

    if ((g_busy_frame_queue.num_of_frames) <=0){
        pthread_cond_wait(&(g_busy_frame_queue.wait), &(g_busy_frame_queue.mut));
    }

    ALOGV("cam_frame_wait_video X");

    return;
}

/*===========================================================================
 * FUNCTION      cam_frame_flush_video
 *
 * DESCRIPTION    this function deletes all the buffers in  busy queue
 * ===========================================================================*/
void cam_frame_flush_video (void)
{
    ALOGD("START cam_frame_flush_video : in n = %d\n", g_busy_frame_queue.num_of_frames);

    pthread_mutex_lock(&(g_busy_frame_queue.mut));

    while (g_busy_frame_queue.front)
    {
       //dequeue from the busy queue
       struct fifo_node *node  = dequeue (&g_busy_frame_queue);
       if(node)
           free(node);

       ALOGV("cam_frame_flush_video: node \n");
    }
    pthread_mutex_unlock(&(g_busy_frame_queue.mut));

    ALOGD("END cam_frame_flush_video: out n = %d\n", g_busy_frame_queue.num_of_frames);

    return ;
}
/*===========================================================================
 * FUNCTION      cam_frame_get_video
 *
 * DESCRIPTION    this function returns a video frame from the head
 * ===========================================================================*/
static struct msm_frame * cam_frame_get_video()
{
    struct msm_frame *p = NULL;
    ALOGV("cam_frame_get_video... in\n");
    ALOGV("cam_frame_get_video... got lock\n");
    if (g_busy_frame_queue.front)
    {
        //dequeue
       struct fifo_node *node  = dequeue (&g_busy_frame_queue);
       if (node)
       {
           p = (struct msm_frame *)node->f;
           free (node);
       }
       ALOGV("cam_frame_get_video... out = %x\n", p->buffer);
    }
    return p;
}

/*===========================================================================
 * FUNCTION      cam_frame_post_video
 *
 * DESCRIPTION    this function add a busy video frame to the busy queue tails
 * ===========================================================================*/
static void cam_frame_post_video (struct msm_frame *p)
{
    if (!p)
    {
        ALOGE("post video , buffer is null");
        return;
    }
    ALOGV("cam_frame_post_video... in = %x\n", (unsigned int)(p->buffer));
    pthread_mutex_lock(&(g_busy_frame_queue.mut));
    ALOGV("post_video got lock. q count before enQ %d", g_busy_frame_queue.num_of_frames);
    //enqueue to busy queue
    struct fifo_node *node = (struct fifo_node *)malloc (sizeof (struct fifo_node));
    if (node)
    {
        ALOGV(" post video , enqueing in busy queue");
        node->f = p;
        node->next = NULL;
        enqueue (&g_busy_frame_queue, node);
        ALOGV("post_video got lock. q count after enQ %d", g_busy_frame_queue.num_of_frames);
    }
    else
    {
        ALOGE("cam_frame_post_video error... out of memory\n");
    }

    pthread_mutex_unlock(&(g_busy_frame_queue.mut));
    pthread_cond_signal(&(g_busy_frame_queue.wait));

    ALOGV("cam_frame_post_video... out = %x\n", p->buffer);

    return;
}

//-------------------------------------------------------------------------------------
static Mutex singleton_lock;

static void receive_camframe_callback(struct msm_frame *frame);
static void receive_camframe_video_callback(struct msm_frame *frame); // 720p
static void receive_jpeg_fragment_callback(uint8_t *buff_ptr, uint32_t buff_size);
static void receive_jpeg_callback(jpeg_event_t status);
static void receive_shutter_callback(common_crop_t *crop);
static void receive_camframetimeout_callback(void);
static int fb_fd = -1;
static int32_t mMaxZoom = 0;
static bool native_get_maxzoom(int camfd, void *pZm);

//AutoFocus issues

void (**LINK_camaf_callback)(struct msm_af_results_t * af_result);


static void mm_camautofocus_callback(struct msm_af_results_t * af_result);

static int dstOffset = 0;

static int camerafd;
pthread_t w_thread;

void *opencamerafd(void *data) {
    camerafd = open(MSM_CAMERA_CONTROL, O_RDWR);
    return NULL;
}

/* When using MDP zoom, double the preview buffers. The usage of these
 * buffers is as follows:
 * 1. As all the buffers comes under a single FD, and at initial registration,
 * this FD will be passed to surface flinger, surface flinger can have access
 * to all the buffers when needed.
 * 2. Only "kPreviewBufferCount" buffers (SrcSet) will be registered with the
 * camera driver to receive preview frames. The remaining buffers (DstSet),
 * will be used at HAL and by surface flinger only when crop information
 * is present in the frame.
 * 3. When there is no crop information, there will be no call to MDP zoom,
 * and the buffers in SrcSet will be passed to surface flinger to display.
 * 4. With crop information present, MDP zoom will be called, and the final
 * data will be placed in a buffer from DstSet, and this buffer will be given
 * to surface flinger to display.
 */
#define NUM_MORE_BUFS 2

SemcCameraHardware::SemcCameraHardware()
    : mParameters(),
      mCameraRunning(false),
      mPreviewInitialized(false),
      mFrameThreadRunning(false),
      mVideoThreadRunning(false),
      mSnapshotThreadRunning(false),
      mSnapshotFormat(0),
      mReleasedRecordingFrame(false),
      mPreviewFrameSize(0),
      mRawSize(0),
      mCameraControlFd(-1),
      mAutoFocusThreadRunning(false),
      mAutoFocusFd(-1),
      mBrightness(0),
      mInPreviewCallback(false),
      mUseOverlay(0),
      mOverlay(0),
      mMsgEnabled(0),
      mNotifyCallback(0),
      mDataCallback(0),
      mDataCallbackTimestamp(0),
      mCallbackCookie(0),
      mDebugFps(0)

     ,mPreviewHeap(NULL)
     ,mRawHeap(NULL)
     ,mJpegHeap(NULL)
     ,mStateCameraHal(CAMERAHAL_STATE_IDLE)
     ,mThumbnailWidth(0)
     ,mThumbnailHeigth(0)
     ,mSemcCameraFlag(0)
     ,mFrameCnt(0)
     ,mCameraPicsize(0)
     ,mCameraPreviewsize(0)
     ,mThumbnailsize(0)
     ,mThumbnailBufferSize(0)
     ,mZoom(-1)
     ,mASSBuffer(NULL)
     ,mASSBufferSize(0)
     ,mConfigurationFlag(false)
     ,mRecognitionFrameCallbackCookie(0)
     ,mAbstractCallbackCookie(0)
     ,mPreviewFlag(false)
     ,mAfterTakepictureFlag(false)
     ,mFocusLockValue(false)
     ,mCancelAutoFocusFlg(false)
     ,mCurrentAspectValue(0)
     ,mFrameRate(DEFAULT_PREVIEW_FRAME_RATE)
     ,mFrameRateSetFrom(CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_SCENE)

     ,mEncodeLocation(true)
     ,mTakepictureFlag(false)
     ,mTakepictureThreadJoin(false)
     ,mReceiveExifDataFlag(false)
     ,mCameraExitFlag(false)
     ,mEncoderThreadInfo(0)
{
    ALOGD("START SemcCameraHardware::constructor");

    // Start opening camera device in a separate thread/ Since this
    // initializes the sensor hardware, this can take a long time. So,
    // start the process here so it will be ready by the time it's
    // needed.
    if ((pthread_create(&w_thread, NULL, opencamerafd, NULL)) != 0) {
        ALOGE("Camera open thread creation failed");
    }

    memset(&mDimension, 0, sizeof(mDimension));
    memset(&mCrop, 0, sizeof(mCrop));
    memset(&zoomCropInfo, 0, sizeof(zoom_crop_info));
    mCurrentTarget = TARGET_QSD8250;
    char value[PROPERTY_VALUE_MAX];
    property_get("persist.debug.sf.showfps", value, "0");
    mDebugFps = atoi(value);
    if( mCurrentTarget == TARGET_MSM7630 )
        kPreviewBufferCountActual = kPreviewBufferCount;
    else
        kPreviewBufferCountActual = kPreviewBufferCount + NUM_MORE_BUFS;

    switch(mCurrentTarget){
        case TARGET_MSM7627:
            jpegPadding = 8;
            break;
        case TARGET_QSD8250:
        case TARGET_MSM7630:
            jpegPadding = 0;
            break;
        default:
            jpegPadding = 0;
            break;
    }

    memset(&mExifInfo, 0, sizeof(exifinfo_t));
    memset(&mKeyScene, NULL, SET_PARAM_KEY_SCENE_LENGTH);
    memset(&mSceneMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mWhitebalance, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mExposureCompensation, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mPreviewMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mAutoExposure, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mAfMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mFocusMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mHJR, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mSmileMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mFacedetectMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mCafMode, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mAntibanding, NULL, CAMERA_STRING_VALUE_MAXLEN);
    memset(&mEffect, NULL, CAMERA_STRING_VALUE_MAXLEN);

    ALOGD("END SemcCameraHardware::constructor");

}


void SemcCameraHardware::filterPreviewSizes(){

    unsigned int bitMask = 0;
    unsigned int prop = 0;
    for(prop=0;prop<sizeof(boardProperties)/sizeof(board_property);prop++){
        if(mCurrentTarget == boardProperties[prop].target){
            bitMask = boardProperties[prop].previewSizeMask;
            break;
        }
    }

    if(bitMask){
        unsigned int mask = 1<<(PREVIEW_SIZE_COUNT-1);
        previewSizeCount=0;
        unsigned int i = 0;
        while(mask){
            if(mask&bitMask)
                supportedPreviewSizes[previewSizeCount++] =
                        preview_sizes[i];
            i++;
            mask = mask >> 1;
        }
    }
}

//filter Picture sizes based on max width and height
void SemcCameraHardware::filterPictureSizes(){
    int i;
    for(i=0;i<PICTURE_SIZE_COUNT;i++){
        if(((picture_sizes[i].width <=
                sensorType->max_supported_snapshot_width) &&
           (picture_sizes[i].height <=
                   sensorType->max_supported_snapshot_height))){
            picture_sizes_ptr = picture_sizes + i;
            supportedPictureSizesCount = PICTURE_SIZE_COUNT - i  ;
            return ;
        }
    }
}

void SemcCameraHardware::initDefaultParameters()
{
    ALOGD("START initDefaultParameters");

    // Initialize constant parameter strings. This will happen only once in the
    // lifetime of the mediaserver process.
    if (!parameter_string_initialized) {
        findSensorType();

        antibanding_values = create_values_str(
            antibanding, sizeof(antibanding) / sizeof(str_map));
        effect_values = create_values_str(
            effects, sizeof(effects) / sizeof(str_map));
        autoexposure_values = create_values_str(
            autoexposure, sizeof(autoexposure) / sizeof(str_map));
        whitebalance_values = create_values_str(
            whitebalance, sizeof(whitebalance) / sizeof(str_map));
        flash_values = create_values_str(
            flash, sizeof(flash) / sizeof(str_map));


        //filter preview sizes
        filterPreviewSizes();
        preview_size_values = create_sizes_str(
            supportedPreviewSizes, previewSizeCount);

		video_size_values = create_sizes_str(
			video_sizes, sizeof(video_sizes));

        picture_size_values = create_sizes_str(
                picture_sizes, PICTURE_SIZE_COUNT_SEMC);

        picture_format_values = create_values_str(
            picture_formats, sizeof(picture_formats)/sizeof(str_map));

        framerate_values = create_values_int(
            framerate, sizeof(framerate) / sizeof(camera_int_map_type));
        thumbnail_size_values = create_sizes_str(
            supportedThumbnailSizes, sizeof(supportedThumbnailSizes) / sizeof(camera_size_type));


        //Set Focus Mode
        parameter_string_initialized = true;
    }

    setInitialValue();

    mParameters.setPreviewSize(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT);
    mParameters.setVideoSize(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT);
    mDimension.display_width = DEFAULT_PREVIEW_WIDTH;
    mDimension.display_height = DEFAULT_PREVIEW_HEIGHT;
    mDimension.postview_width = DEFAULT_PREVIEW_WIDTH;
    mDimension.postview_height = DEFAULT_PREVIEW_HEIGHT;

    mParameters.setPreviewFrameRate(DEFAULT_PREVIEW_FRAME_RATE);

    mParameters.setPreviewFormat("yuv420sp"); // informative
    mParameters.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420SP);

    mParameters.setPictureSize(DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
    mDimension.picture_width = DEFAULT_PICTURE_WIDTH;
    mDimension.picture_height = DEFAULT_PICTURE_HEIGHT;
    mParameters.setPictureFormat("jpeg"); // informative

    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, CAMERAHAL_JPEGQUALITY_FINE_MAX);
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH,
                    THUMBNAIL_WIDTH_STR); // informative
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT,
                    THUMBNAIL_HEIGHT_STR); // informative
    mDimension.ui_thumbnail_width =
            CAMERA_NORMALTHUMBNAIL_WIDTH;
    mDimension.ui_thumbnail_height =
            CAMERA_NORMALTHUMBNAIL_HEIGHT;
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, CAMERAHAL_THUMBNAILQUALITY_FINE_MAX);

    mParameters.set(CameraParameters::KEY_ANTIBANDING,
                    CameraParameters::ANTIBANDING_AUTO);
    mParameters.set(CameraParameters::KEY_EFFECT,
                    CameraParameters::EFFECT_NONE);

    mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE,
                    CameraParameters::AUTO_EXPOSURE_FRAME_AVG);
    mParameters.set(CameraParameters::KEY_WHITE_BALANCE,
                    CameraParameters::WHITE_BALANCE_AUTO);
    mParameters.set(CameraParameters::KEY_FOCUS_MODE,
                    CameraParameters::FOCUS_MODE_AUTO);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
                    "yuv420sp");

    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
                    preview_size_values.string());
    //Set Video Keys
    mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
                    video_size_values.string());

    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
                    picture_size_values.string());

    mParameters.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING,
                    antibanding_values);
    mParameters.set(CameraParameters::KEY_SUPPORTED_EFFECTS, effect_values);

    mParameters.set(CameraParameters::KEY_SUPPORTED_AUTO_EXPOSURE, autoexposure_values);
    mParameters.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
                    whitebalance_values);
    mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                    focus_mode_values);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
                    picture_format_values);

    mParameters.set(CameraParameters::KEY_PICTURE_FORMAT,
                    CameraParameters::PIXEL_FORMAT_JPEG);

    mParameters.set(CameraParameters::KEY_SCENE_MODE, CameraParameters::SCENE_MODE_AUTO);
    mParameters.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES, scene_mode_values.string());

    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES, framerate_values);

    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, COMPENSATION_M9);
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, COMPENSATION_P9),
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0.333333333");
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, COMPENSATION_ZERO);

    mParameters.set(CameraParameters::KEY_FLASH_MODE, CameraParameters::FLASH_MODE_ON);
    mParameters.set(CameraParameters::KEY_SUPPORTED_FLASH_MODES, flash_values);

    mParameters.set(CameraParameters::KEY_FOCAL_LENGTH,CAMERA_FOCAL_LENGTH_DEFAULT);
    mParameters.set(CameraParameters::KEY_FOCUS_DISTANCES, "10,1000,Infinity");
    mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES,
                    thumbnail_size_values.string());
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE,"(5000,30000)");
    mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE,"5000,30000"); //min,max
    mParameters.setFloat(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE,
                    CAMERA_HORIZONTAL_VIEW_ANGLE_DEFAULT);
    mParameters.setFloat(CameraParameters::KEY_VERTICAL_VIEW_ANGLE,
                    CAMERA_VERTICAL_VIEW_ANGLE_DEFAULT);


    //ALOGD("%s FOUND mAX ZOOM %d", __FUNCTION__, maxZoom);

    mUseOverlay = useOverlay();

    /* Initialize the camframe_timeout_flag*/
    Mutex::Autolock l(&mCamframeTimeoutLock);
    camframe_timeout_flag = false;
    mPostViewHeap = NULL;

    ALOGD("END initDefaultParameters");

}

void SemcCameraHardware::findSensorType(){
    mDimension.picture_width = DEFAULT_PICTURE_WIDTH;
    mDimension.picture_height = DEFAULT_PICTURE_HEIGHT;
#if SCRITCH_OFF
    bool ret = native_set_parm(CAMERA_SET_PARM_DIMENSION,
                    sizeof(cam_ctrl_dimension_t), &mDimension);
    if (ret) {
        unsigned int i;
        for (i = 0; i < sizeof(sensorTypes) / sizeof(SensorType); i++) {
            if (sensorTypes[i].rawPictureHeight
                    == mDimension.raw_picture_height) {
                sensorType = sensorTypes + i;
                return;
            }
        }
    }
#endif//SCRITCH_OFF
    //default to 5 mp
    sensorType = sensorTypes;
    return;
}

#define ROUND_TO_PAGE(x)  (((x)+0xfff)&~0xfff)

static bool native_get_scene_nv(int camfd, void *pBuffer, int size);

bool SemcCameraHardware::startCamera()
{
    ALOGD("START startCamera");
#ifdef CAMERA_FIRMWARE_UPDATE
    int file_fd = -1;
#endif //CAMERA_FIRMWARE_UPDATE

    if( mCurrentTarget == TARGET_MAX ) {
        ALOGE(" Unable to determine the target type. Camera will not work ");
        return false;
    }
#if DLOPEN_LIBMMCAMERA
    libmmcamera = ::dlopen("liboemcamera.so", RTLD_NOW);
    ALOGV("loading liboemcamera at %p", libmmcamera);
    if (!libmmcamera) {
        ALOGE("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
        return false;
    }

    libmmipl = ::dlopen("libmmipl.so", RTLD_NOW);
    if (!libmmipl) {
        ALOGE("startCamera FATAL ERROR: could not dlopen libmmipl.so: %s", dlerror());
        goto errlibmmcamera;
    }

    *(void **)&LINK_cam_frame =
        ::dlsym(libmmcamera, "cam_frame");
    *(void **)&LINK_camframe_terminate =
        ::dlsym(libmmcamera, "camframe_terminate");

    *(void **)&LINK_jpeg_encoder_init =
        ::dlsym(libmmcamera, "jpeg_encoder_init");

    *(void **)&LINK_jpeg_encoder_encode =
        ::dlsym(libmmcamera, "jpeg_encoder_encode");

    *(void **)&LINK_jpeg_encoder_join =
        ::dlsym(libmmcamera, "jpeg_encoder_join");

    *(void **)&LINK_mmcamera_camframe_callback =
        ::dlsym(libmmcamera, "mmcamera_camframe_callback");

    *LINK_mmcamera_camframe_callback = receive_camframe_callback;

    *(void **)&LINK_mmcamera_jpegfragment_callback =
        ::dlsym(libmmcamera, "mmcamera_jpegfragment_callback");

    *LINK_mmcamera_jpegfragment_callback = receive_jpeg_fragment_callback;

    *(void **)&LINK_mmcamera_jpeg_callback =
        ::dlsym(libmmcamera, "mmcamera_jpeg_callback");

    *LINK_mmcamera_jpeg_callback = receive_jpeg_callback;

    *(void **)&LINK_camframe_timeout_callback =
        ::dlsym(libmmcamera, "camframe_timeout_callback");

    *LINK_camframe_timeout_callback = receive_camframetimeout_callback;

    // 720 p new recording functions
    *(void **)&LINK_cam_frame_flush_free_video = ::dlsym(libmmcamera, "cam_frame_flush_free_video");

    *(void **)&LINK_camframe_free_video = ::dlsym(libmmcamera, "cam_frame_add_free_video");

    *(void **)&LINK_camframe_video_callback = ::dlsym(libmmcamera, "mmcamera_camframe_videocallback");
        *LINK_camframe_video_callback = receive_camframe_video_callback;

    *(void **)&LINK_mmcamera_shutter_callback =
        ::dlsym(libmmcamera, "mmcamera_shutter_callback");

    *LINK_mmcamera_shutter_callback = receive_shutter_callback;

    *(void**)&LINK_jpeg_encoder_setMainImageQuality =
        ::dlsym(libmmcamera, "jpeg_encoder_setMainImageQuality");

    *(void**)&LINK_jpeg_encoder_setThumbnailQuality =
        ::dlsym(libmmcamera, "jpeg_encoder_setThumbnailQuality");

    *(void**)&LINK_jpeg_encoder_setRotation =
        ::dlsym(libmmcamera, "jpeg_encoder_setRotation");

/* Disabling until support is available.
    *(void**)&LINK_jpeg_encoder_setLocation =
        ::dlsym(libmmcamera, "jpeg_encoder_setLocation");
*/
    *(void **)&LINK_cam_conf =
        ::dlsym(libmmcamera, "cam_conf");

/* Disabling until support is available.
    *(void **)&LINK_default_sensor_get_snapshot_sizes =
        ::dlsym(libmmcamera, "default_sensor_get_snapshot_sizes");
*/
    *(void **)&LINK_launch_cam_conf_thread =
        ::dlsym(libmmcamera, "launch_cam_conf_thread");

    *(void **)&LINK_release_cam_conf_thread =
        ::dlsym(libmmcamera, "release_cam_conf_thread");

/* Disabling until support is available.
    *(void **)&LINK_zoom_crop_upscale =
        ::dlsym(libmmcamera, "zoom_crop_upscale");
*/

    *(void **)&LINK_camaf_callback =
        ::dlsym(libmmcamera, "camaf_callback");

    *LINK_camaf_callback = mm_camautofocus_callback;
#if SCRITCH_OFF

    *(void **)&LINK_camzoom_callback =
        ::dlsym(libmmcamera, "camzoom_callback");

    *LINK_camzoom_callback = mm_camzoom_callback;

    *(void **)&LINK_camanalysis_callback =
        ::dlsym(libmmcamera, "camanalysis_callback");

    *LINK_camanalysis_callback = mm_camanalysis_callback;
#endif//SCRITCH_OFF
    *(void **)&LINK_ipl_downsize =
        ::dlsym(libmmipl, "ipl_downsize");

#ifdef CAMERA_FIRMWARE_UPDATE
    *(void **)&LINK_sensor_set_firmware_fd =
        ::dlsym(libmmcamera, "sensor_set_firmware_fd");

    *(void **)&LINK_sensor_get_firmware_update_result =
        ::dlsym(libmmcamera, "sensor_get_firmware_update_result");
    // 1. I check having file or not for firmware Update.
    //    -When there was a file, I hand the descriptor of the file to the driver.
    //    -When there was not a file, it usually starts.

    file_fd = open("/sdcard/camfirm.bin", O_RDONLY);
    if(file_fd != -1) {
        ALOGE("Firmware binary file was detected.");
        LINK_sensor_set_firmware_fd(file_fd);
    }
#endif //CAMERA_FIRMWARE_UPDATE

#else
    mmcamera_camframe_callback = receive_camframe_callback;
    mmcamera_jpegfragment_callback = receive_jpeg_fragment_callback;
    mmcamera_jpeg_callback = receive_jpeg_callback;
    mmcamera_shutter_callback = receive_shutter_callback;
#endif // DLOPEN_LIBMMCAMERA

    /* The control thread is in libcamera itself. */
    if (pthread_join(w_thread, NULL) != 0) {
        ALOGE("Camera open thread exit failed");
        goto errfirmware;
    }
    mCameraControlFd = camerafd;

    if (mCameraControlFd < 0) {
        ALOGE("startCamera X: %s open failed: %s!",
             MSM_CAMERA_CONTROL,
             strerror(errno));
        goto errfirmware;
    }

    if( mCurrentTarget != TARGET_MSM7630 ){
        fb_fd = open("/dev/graphics/fb0", O_RDWR);
        if (fb_fd < 0) {
            ALOGE("startCamera: fb0 open failed: %s!", strerror(errno));
            goto errcontrol;
        }
        mdp_ccs_save.direction = MDP_CCS_YUV2RGB;
        int ioctlRetVal = true;
        if((ioctlRetVal = ioctl(fb_fd, MSMFB_GET_CCS_MATRIX, &mdp_ccs_save)) < 0) {
            ALOGE("startCamera : MSMFB_GET_CCS_MATRIX: ioctl failed. ioctl return value is %d.", ioctlRetVal);
            mdp_ccs_save.direction = -1;
        }
    }

    /* This will block until the control thread is launched. After that, sensor
     * information becomes available.
     */

    if (LINK_launch_cam_conf_thread()) {
        ALOGE("failed to launch the camera config thread");
        goto errall;
    }
    /*if (!scene_nv_data_initialized) {
        memset(&scene_nv_data, 0, sizeof(scene_nv_data));
        if(native_get_scene_nv(mCameraControlFd, scene_nv_data, sizeof(scene_nv_data)) == false) {
            ALOGE("%s: native_get_scene_nv is error.", __FUNCTION__);
            goto errall;
        }
        scene_nv_data_initialized = true;
        ALOGD("%s: scene_nv_data is initialized", __FUNCTION__);
    }*/

    memset(&mSensorInfo, 0, sizeof(mSensorInfo));
    if (ioctl(mCameraControlFd,
              MSM_CAM_IOCTL_GET_SENSOR_INFO,
              &mSensorInfo) < 0){
        ALOGW("%s: cannot retrieve sensor info!", __FUNCTION__);
    } else {
        ALOGI("%s: camsensor name %s, flash %d", __FUNCTION__,
             mSensorInfo.name, mSensorInfo.flash_enabled);
    }
/* Disabling until support is available.
    picture_sizes = LINK_default_sensor_get_snapshot_sizes(&PICTURE_SIZE_COUNT);
    if (!picture_sizes || !PICTURE_SIZE_COUNT) {
        ALOGE("startCamera X: could not get snapshot sizes");
        return false;
    }
*/

    mStateCameraHal = CAMERAHAL_STATE_INIT;

#ifdef CAMERA_FIRMWARE_UPDATE
    if(file_fd != -1) {
        ALOGE("extended sleep for firmware update.");
        // 2. I extend sleep time when there was a file.
        usleep(300*1000*1000);
        // 3. I acquire firmware update upshot from device driver.
        // 4. I output debugging log.
        int result;
        result = LINK_sensor_get_firmware_update_result();
        while(SENSOR_FIRMWARE_UPDATE_RESULT_INVALID == result) {
            ALOGE("still firmware update processing...");
            usleep(30*1000*1000);
            result = LINK_sensor_get_firmware_update_result();
        }
        if(!result) {
            ALOGE("The camera firmware update succeeded : result = %d ",result);
        } else {
            ALOGE("The camera firmware update failed : result = %d ",result);
        }
        // 5. Close the file descriptor.
        /*close(file_fd);*/
        // remove("/sdcard/camfirm.bin");
        // 6. I send back an error to application from HAL
        //    ##The application finishes a camera by receiving an error.
        goto errall;
    }
#endif //CAMERA_FIRMWARE_UPDATE

    ALOGD("END startCamera : mStateCameraHal = %d", mStateCameraHal);

    return true;
errall:
    if(fb_fd > -1) {
        ALOGE("close frame buffer.");
        close(fb_fd);
        fb_fd = -1;
    }
errcontrol:
    if(mCameraControlFd > -1) {
        ALOGE("close camera control.");
        close(mCameraControlFd);
        mCameraControlFd = -1;
    }
errfirmware:
#ifdef CAMERA_FIRMWARE_UPDATE
    if(file_fd > -1) {
        ALOGE("close firmware binary file.");
        close(file_fd);
        file_fd = -1;
    }
#endif //CAMERA_FIRMWARE_UPDATE
errlibmmipl:
    if(libmmipl) {
        unsigned ref_ipl = ::dlclose(libmmipl);
        ALOGE("startCamera : dlclose(libmmipl) refcount %d", ref_ipl);
        libmmipl = NULL;
    }
errlibmmcamera:
    if(libmmcamera) {
        unsigned ref = ::dlclose(libmmcamera);
        ALOGE("startCamera : dlclose(libmmcamera) refcount %d", ref);
        libmmcamera = NULL;
    }
    return false;
}

status_t SemcCameraHardware::dump(int fd,
                                      const Vector<String16>& args) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    // Dump internal primitives.
    result.append("SemcCameraHardware::dump");
    snprintf(buffer, 255, "mMsgEnabled (%d)\n", mMsgEnabled);
    result.append(buffer);
    int width, height;
    mParameters.getPreviewSize(&width, &height);
    snprintf(buffer, 255, "preview width(%d) x height (%d)\n", width, height);
    result.append(buffer);
    mParameters.getPictureSize(&width, &height);
    snprintf(buffer, 255, "raw width(%d) x height (%d)\n", width, height);
    result.append(buffer);
    snprintf(buffer, 255,
             "preview frame size(%d), raw size (%d), jpeg size (%d) "
             "and jpeg max size (%d)\n", mPreviewFrameSize, mRawSize,
             mJpegSize, mJpegMaxSize);
    result.append(buffer);
    write(fd, result.string(), result.size());

    // Dump internal objects.
    if (mPreviewHeap != 0) {
        mPreviewHeap->dump(fd, args);
    }
    if (mRawHeap != 0) {
        mRawHeap->dump(fd, args);
    }
    if (mJpegHeap != 0) {
        mJpegHeap->dump(fd, args);
    }
    if(mRawSnapshotAshmemHeap != 0 ){
        mRawSnapshotAshmemHeap->dump(fd, args);
    }
    mParameters.dump(fd, args);
    return NO_ERROR;
}

static bool native_get_maxzoom(int camfd, void *pZm)
{
    ALOGV("native_get_maxzoom E");

    struct msm_ctrl_cmd ctrlCmd;
    int32_t *pZoom = (int32_t *)pZm;

    ctrlCmd.type       = CAMERA_GET_PARM_MAXZOOM;
    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.length     = sizeof(int32_t);
    ctrlCmd.value      = pZoom;
    ctrlCmd.resp_fd    = camfd;

    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        ALOGE("native_get_maxzoom: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }
    ALOGD("ctrlCmd.value = %d", *(int32_t *)ctrlCmd.value);
    memcpy(pZoom, (int32_t *)ctrlCmd.value, sizeof(int32_t));

    ALOGV("native_get_maxzoom X");
    return true;
}

static bool native_set_afmode(int camfd, isp3a_af_mode_t af_type)
{
    int rc;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type = CAMERA_SET_PARM_AUTO_FOCUS;
    ctrlCmd.length = sizeof(af_type);
    ctrlCmd.value = &af_type;
    ctrlCmd.resp_fd = camfd; // FIXME: this will be put in by the kernel

    if ((rc = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0)
        ALOGE("native_set_afmode: ioctl fd %d error %s\n",
             camfd,
             strerror(errno));

    ALOGV("native_set_afmode: ctrlCmd.status == %d\n", ctrlCmd.status);
    return rc >= 0 && ctrlCmd.status == CAMERA_EXIT_CB_DONE;
}

static bool native_cancel_afmode(int camfd)
{
    int rc;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 0;
    ctrlCmd.type = CAMERA_AUTO_FOCUS_CANCEL;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = -1; // there's no response fd

    if ((rc = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND_2, &ctrlCmd)) < 0)
    {
        ALOGE("native_cancel_afmode: ioctl fd %d error %s\n",
             camfd,
             strerror(errno));
        return false;
    }

    return true;
}

static bool native_start_preview(int camfd)
{
    ALOGD("START native_start_preview");

    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_START_PREVIEW;
    ctrlCmd.length     = 0;
    ctrlCmd.resp_fd    = camfd; // FIXME: this will be put in by the kernel

    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        ALOGE("native_start_preview: MSM_CAM_IOCTL_CTRL_COMMAND fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_start_preview");

    return true;
}

static bool native_get_picture (int camfd, common_crop_t *crop)
{
    ALOGD("START native_get_picture");

    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.length     = sizeof(common_crop_t);
    ctrlCmd.value      = crop;

    if(ioctl(camfd, MSM_CAM_IOCTL_GET_PICTURE, &ctrlCmd) < 0) {
        ALOGE("native_get_picture: MSM_CAM_IOCTL_GET_PICTURE fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_get_picture");

    return true;
}

static bool native_stop_preview(int camfd)
{
    ALOGD("START native_stop_preview");

    struct msm_ctrl_cmd ctrlCmd;
    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_STOP_PREVIEW;
    ctrlCmd.length     = 0;
    ctrlCmd.resp_fd    = camfd; // FIXME: this will be put in by the kernel

    if(ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        ALOGE("native_stop_preview: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_stop_preview");

    return true;
}
static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}


static void
setSocTorchMode(bool enable)
{
    char const*const FLASH = FLASHLIGHT; 
    if (enable){
            int value = 255;
            write_int(FLASH, value);
    } else {
            int value = 0;
            write_int(FLASH, value);
   }
}


static bool native_prepare_snapshot(int camfd, const CameraParameters& params)
{
    ALOGD("START native_prepare_snapshot");

    int ioctlRetVal = true;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type       = CAMERA_PREPARE_SNAPSHOT;
    ctrlCmd.length     = 0;
    ctrlCmd.value      = NULL;
    ctrlCmd.resp_fd = camfd;

    const char *str = params.get(CameraParameters::KEY_FLASH_MODE);


    ALOGD("%s, Flash Value %s", __FUNCTION__, str);

    if (str != NULL) {
       int32_t value = attr_lookup(flash, sizeof(flash) / sizeof(str_map), str);
       if (value != NOT_FOUND) {
            bool shouldBeOn = strcmp(str, "on") == 0;
            setSocTorchMode(shouldBeOn);
       }
    }



    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        ALOGE("native_prepare_snapshot: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_prepare_snapshot");

    return true;
}


static bool native_start_snapshot(int camfd)
{
    ALOGD("START native_start_snapshot");

    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_START_SNAPSHOT;
    ctrlCmd.length     = 0;
    ctrlCmd.resp_fd    = camfd; // FIXME: this will be put in by the kernel


    if(ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        ALOGE("native_start_snapshot: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_start_snapshot");

    return true;
}

static bool native_start_raw_snapshot(int camfd)
{
    ALOGD("START native_start_raw_snapshot");

    int ret;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type = CAMERA_START_RAW_SNAPSHOT;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = camfd;

    if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_start_raw_snapshot: ioctl failed. ioctl return value "\
             "is %d \n", ret);
        return false;
    }

    ALOGD("END native_start_raw_snapshot");

    return true;
}


static bool native_stop_snapshot (int camfd)
{
    ALOGD("START native_stop_snapshot");

    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 0;
    ctrlCmd.type       = CAMERA_STOP_SNAPSHOT;
    ctrlCmd.length     = 0;
    ctrlCmd.resp_fd    = -1;


    setSocTorchMode(0);

    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND_2, &ctrlCmd) < 0) {
        ALOGE("native_stop_snapshot: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    ALOGD("END native_stop_snapshot");

    return true;
}
/*===========================================================================
 * FUNCTION    - native_start_recording -
 *
 * DESCRIPTION:
 *==========================================================================*/
static bool native_start_recording(int camfd)
{
    ALOGD("START native_start_recording");

    int ret;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type = CAMERA_START_RECORDING;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = camfd;

    if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_start_recording: ioctl failed. ioctl return value "\
            "is %d \n", ret);
        return false;
    }
    ALOGV("native_start_recording: ioctl good. ioctl return value is %d \n",ret);

  /* TODO: Check status of postprocessing if there is any,
   *       PP status should be in  ctrlCmd */


    ALOGD("END native_start_recording");

    return true;
}

/*===========================================================================
 * FUNCTION    - native_stop_recording -
 *
 * DESCRIPTION:
 *==========================================================================*/
static bool native_stop_recording(int camfd)
{
    ALOGD("START native_stop_recording");

    int ret;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type = CAMERA_STOP_RECORDING;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = camfd;

    if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_stop_recording: ioctl failed. ioctl return value is %d \n",
        ret);
        return false;
    }

    ALOGD("END native_stop_recording returned %d", ret);

    return true;
}
/*===========================================================================
 * FUNCTION    - native_start_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
static bool native_start_video(int camfd)
{
    ALOGD("START native_start_video");

    int ret;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type = CAMERA_START_VIDEO;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = camfd;

    if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_start_video: ioctl failed. ioctl return value is %d \n",
        ret);
        return false;
    }

  /* TODO: Check status of postprocessing if there is any,
   *       PP status should be in  ctrlCmd */

    ALOGD("END native_start_video");

    return true;
}

/*===========================================================================
 * FUNCTION    - native_stop_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
static bool native_stop_video(int camfd)
{
    ALOGD("START native_stop_video");

    int ret;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 1000;
    ctrlCmd.type = CAMERA_STOP_VIDEO;
    ctrlCmd.length = 0;
    ctrlCmd.value = NULL;
    ctrlCmd.resp_fd = camfd;
    
    if(MDP_CCS_YUV2RGB == mdp_ccs_save.direction){
        if((ret = ioctl(fb_fd, MSMFB_SET_CCS_MATRIX, &mdp_ccs_save)) < 0) {
            ALOGW("native_stop_video : MSMFB_SET_CCS_MATRIX: ioctl failed. ioctl return value is %d.", ret);
        }
    }

    if ((ret = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_stop_video: ioctl failed. ioctl return value is %d \n",
        ret);
        return false;
    }

    ALOGD("END native_stop_video");

    return true;
}
/*==========================================================================*/

/**
 * @brief native_sync_raw_snapshot_interval
 * @param camfd [IN] Open Information
 * @return unsigned bool
 * @n      false : Processing error
 * @n      true : SUCCESS
 *
 * @brief sync raw snaapshot interval is done to the device driver
 */
static bool native_sync_raw_snapshot_interval (int camfd)
{
    ALOGD("START native_sync_raw_snapshot_interval");
    struct msm_ctrl_cmd ctrlCmd;
    int ioctlRetVal     = true;

    ctrlCmd.timeout_ms  = DRV_TIMEOUT_10K;
    ctrlCmd.type        = CAMERA_SYNC_RAW_SNAPSHOT_INTERVAL;
    ctrlCmd.length      = 0;
    ctrlCmd.value       = NULL;
    ctrlCmd.resp_fd     = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_sync_raw_snapshot_interval: CAMERA_SYNC_RAW_SNAPSHOT_INTERVAL failed... ioctl "\
             "return value is %d \n", ioctlRetVal);
        return false;
    }

    ALOGD("END native_sync_raw_snapshot_interval :ioctlRetVal[%d]", ioctlRetVal);
    return true;
}

/**
 * @brief native_start_autofocus
 * @param camfd [IN] Open Information
 * @return bool
 * @n      false : Processing error
 * @n      true : SUCCESS
 *
 * @brief The start requirement of the auto-focus is done to the device driver
 */
static bool native_start_autofocus (int camfd, bool ae_lock, bool awb_lock, bool focus_lock)
{
    ALOGD("START native_start_autofocus : ae_lock =%d ,awb_lock = %d ,focus_lock = %d", ae_lock, awb_lock, focus_lock);
    int ioctlRetVal;
    struct msm_ctrl_cmd ctrlCmd;
    camera_start_auto_focus_t focustype;

    if(true == ae_lock){
        focustype.camera_ae_lock = CAMERA_AE_LOCK;
    }else{
        focustype.camera_ae_lock = CAMERA_AE_UNLOCK;
    }

    if(true == awb_lock){
        focustype.camera_awb_lock = CAMERA_AWB_LOCK;
    }else{
        focustype.camera_awb_lock = CAMERA_AWB_UNLOCK;
    }

    if(true == focus_lock){
        focustype.camera_focus_lock = CAMERA_FOCUS_LOCK;
    }else{
        focustype.camera_focus_lock = CAMERA_FOCUS_UNLOCK;
    }

    ctrlCmd.timeout_ms = DRV_TIMEOUT_10K;
    ctrlCmd.type       = CAMERA_START_AUTO_FOCUS;
    ctrlCmd.length     = sizeof(camera_start_auto_focus_t);
    ctrlCmd.value      = (void *)&focustype;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_start_autofocus: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
        return false;
    }
    ALOGD("END native_start_autofocus");
    return true;
}
/**
 * @brief native_stop_autofocus
 * @param camfd [IN] Open Information
 * @return bool
 * @n      false : Processing error
 * @n      true : SUCCESS
 *
 * @brief The stop requirement of the auto-focus is done to the device driver
 */
static bool native_stop_autofocus (int camfd)
{
    ALOGD("START native_stop_autofocus");
    int ioctlRetVal;
    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_STOP_AUTO_FOCUS;
    ctrlCmd.length     = 0;
    ctrlCmd.value      = NULL;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_stop_autofocus: ioctl failed. ioctl return value is %d \n", ioctlRetVal);
        return false;
    }
    ALOGD("END native_stop_autofocus");
    return true;
}
/**
 * @brief native_start_autozoom
 * @param camfd [IN]  Camera open information
 * @param value [IN]  Changing value
 * @
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  pass start auto-zoom  to driver
 */
static bool native_start_autozoom(int camfd, int value)
{
    ALOGD("START native_start_autozoom : value = %d" ,value);

    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;
    int driver_zoom_value = value -1;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_100K;
    ctrlCmd.type       = CAMERA_START_AUTO_ZOOM;
    ctrlCmd.length     = sizeof(int32_t);
    ctrlCmd.value      = (void *)&driver_zoom_value;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_start_autozoom: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }
    ALOGD("END native_start_autozoom");
    return true;
}

/**
 * @brief native_stop_autozoom
 * @param camfd [IN]  camera open information
 * @
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  pass stop auto-zoom  to driver
 */
static bool native_stop_autozoom(int camfd)
{
    ALOGD("START native_stop_autozoom");
    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_STOP_AUTO_ZOOM;
    ctrlCmd.length     = 0;
    ctrlCmd.value      = NULL;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_stop_autozoom: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }
    ALOGD("END native_stop_autozoom");
    return true;
}


/**
 * @brief native_step_zoom
 * @param camfd [IN]  camera open information
 * @param value [IN]  Changing value
 * @
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  pass Zoom value to driver
 */
static bool native_step_zoom(int camfd, int value)
{
    ALOGD("START native_step_zoom : value = %d" ,value);
    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;
    int     driver_zoom_value = value -1;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_200K;
    ctrlCmd.type       = CAMERA_SET_PARM_ZOOM;
    ctrlCmd.length     = sizeof(int32_t);
    ctrlCmd.value      = (void *)&driver_zoom_value;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_step_zoom: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }

    ALOGD("END native_step_zoom");
    return true;
}



/**
 * @brief native_set_ae_awb_lock
 * @param camfd    [IN] Open Information
 * @param ae_lock  [IN] AE Lock Information(true:Lock,false:UnLock)
 * @param awb_lock [IN] AWB Lock Information(true:Lock,false:UnLock)
 * @return unsigned bool
 * @n      false : Processing error
 * @n      true  : SUCCESS
 *
 * @brief set ae awb lock is done to the device driver
 */
static bool native_set_ae_awb_lock(int camfd, bool ae_lock, bool awb_lock)
{
    ALOGD("START native_set_ae_awb_lock");
    struct msm_ctrl_cmd  ctrlCmd;
    camera_ae_awb_lock_t camera_ae_awb_lock;
    int ioctlRetVal     = true;

    if(ae_lock){
        camera_ae_awb_lock.camera_ae_lock  = CAMERA_AE_LOCK;
    }else{
        camera_ae_awb_lock.camera_ae_lock  = CAMERA_AE_UNLOCK;
    }

    if(awb_lock){
        camera_ae_awb_lock.camera_awb_lock = CAMERA_AWB_LOCK;
    }else{
        camera_ae_awb_lock.camera_awb_lock = CAMERA_AWB_UNLOCK;
    }

    ctrlCmd.timeout_ms  = DRV_TIMEOUT_5K;
    ctrlCmd.type        = CAMERA_SET_AE_AWB_LOCK;
    ctrlCmd.length      = sizeof(camera_ae_awb_lock_t);
    ctrlCmd.value       = (void *)&camera_ae_awb_lock;
    ctrlCmd.resp_fd     = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_set_ae_awb_lock: ioctl failed. ioctl return value is"\
             "%d \n", ioctlRetVal);
        return false;
    }

    ALOGD("END native_set_ae_awb_lock");
    return true;
}
#if SCRITCH_OFF
/**
 * @brief native_get_firmware_version
 * @param camfd [IN] Open Information
 * @param cahal_get_versionsmfd [OUT] FirmwareVersion
 * @return unsigned bool
 * @n      false : Processing error
 * @n      true : SUCCESS
 *
 * @brief get firmware version is done to the device driver
 */
static bool native_get_firmware_version(int camfd, struct get_versions_t *hal_get_versions)
{
    ALOGD("START native_get_firmware_version");
    int    ioctlRetVal;
    struct msm_ctrl_cmd ctrlCmd;
    camera_get_versions_t camera_get_versions;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_GET_PARM_VERSIONS;
    ctrlCmd.length     = sizeof(camera_get_versions_t);
    ctrlCmd.value      = (void *)&camera_get_versions;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("END native_get_firmware_version: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }

    memcpy(hal_get_versions,&camera_get_versions,sizeof(camera_get_versions_t));

    ALOGD("END native_get_firmware_version:firmware_version = %s", hal_get_versions->firmware);

    return true;
}
/**
 * @brief native_set_aeaf_position
 * @param camfd [IN]  camera open information
 * @param position [IN]  Rect position
 * @param face_cnt [IN]  Face count
 * @
 * @return unsigned bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  Set focus rect
 */
static bool native_set_aeaf_position(int camfd, setAeAfPosition_t position, int face_cnt)
{
    ALOGD("START native_set_aeaf_position");
    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;
    camera_focus_window_rectangles_type focus_rect_type;
    camera_focus_rectangle_dimensions_type rect_dimensions;

    rect_dimensions.x_upper_left = position.x;
    rect_dimensions.y_upper_left = position.y;
    rect_dimensions.width = position.width;
    rect_dimensions.height = position.height;

    focus_rect_type.focus_window_count = (int16_t)face_cnt;
    focus_rect_type.windows_list = (camera_focus_rectangle_dimensions_type *)&rect_dimensions;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_SET_PARM_FOCUS_RECT;
    ctrlCmd.length     = sizeof(camera_focus_window_rectangles_type);
    ctrlCmd.value      = (void *)&focus_rect_type;
    ctrlCmd.resp_fd    = camfd;
    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_set_aeaf_position: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }
    ALOGD("END native_set_aeaf_position");
    return true;
}

#endif//SCRITCH_OFF
/**
 * @brief native_get_scene_nv
 * @param camfd [IN]  camera open information
 * @param *pBuffer [IN]  Heap buffer
 * @param size [IN]  Heap Size
 * @
 * @return unsigned bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  Get Analysis Infomation to driver
 */
static bool native_get_scene_nv(int camfd, void *pBuffer, int size)
{
    ALOGD("START native_get_scene_nv");
    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;
    struct  camera_nv_data_t nv_data_type;

    nv_data_type.nv_buf = (unsigned char *)pBuffer;
    nv_data_type.len = (uint16_t)size;
    nv_data_type.kind = 1;//CAMERA_NV_KIND_ASS;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_GET_PARM_NV_DATA;
    ctrlCmd.length     = sizeof(struct  camera_nv_data_t);
    ctrlCmd.value      = (void *)&nv_data_type;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_get_scene_nv: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }
    ALOGD("END native_get_scene_nv");
    return true;
}

/**
 * @brief native_get_jpegfilesize
 * @param camfd [IN]  camera open information
 * @
 * @return unsigned char
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  The file size of JPEG is acquired
 */
bool SemcCameraHardware::native_get_jpegfilesize(int camfd)
{
    ALOGD("START native_get_jpegfilesize");
    int    ioctlRetVal;
    struct msm_ctrl_cmd ctrlCmd;
    int32_t jpegsize;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_GET_PARM_JPEG_MAIN_IMG_FILE_SIZE;
    ctrlCmd.length     = sizeof(int32_t);
    ctrlCmd.value      = (void *)&jpegsize;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("END native_set_af_antibanding: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }

    mJpegSize = (uint32_t)jpegsize;
    ALOGD("END native_get_jpegfilesize:mJpegSize = %d,jpegsize = %d",mJpegSize,jpegsize);
    return true;
}


typedef struct camera_capture_settings_t{
    uint32_t jpeg_main_img_buf_length; 
} camera_capture_settings_t;

typedef struct camera_jpeg_quality_t{
   uint32_t camera_jpeg_quality_min; 
   uint32_t camera_jpeg_quality_ave;
   uint32_t camera_jpeg_quality_max;
}camera_jpeg_quality_t;
/**
* @brief native_set_jpegquality
* @param camfd [IN] Open Information
* @param *qsize [IN] The table of the jpeg quality
* @return unsigned bool
* @n      false : Processing error
* @n      true : SUCCESS
*
* @brief The setup of the JPEG quality is done to the device driver
*/

static bool native_set_jpegquality(int camfd, jpeg_quality_t *qsize)
{
    ALOGD("START native_set_jpegquality");
    int ioctlRetVal;//, i = 0;
    struct msm_ctrl_cmd ctrlCmd;
    camera_jpeg_quality_t qtype;

    qtype.camera_jpeg_quality_min = qsize->quality_min;
    qtype.camera_jpeg_quality_ave = qsize->quality_ave;
    qtype.camera_jpeg_quality_max = qsize->quality_max;

    ALOGV("native_set_jpegquality : quality_min = %d",qtype.camera_jpeg_quality_min);
    ALOGV("native_set_jpegquality : quality_ave = %d",qtype.camera_jpeg_quality_ave);
    ALOGV("native_set_jpegquality : quality_max = %d",qtype.camera_jpeg_quality_max);


    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_SET_PARM_JPEG_QUALITY;
    ctrlCmd.length     = sizeof(camera_jpeg_quality_t);
    ctrlCmd.value      = (void *)&qtype;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0) {
        ALOGE("native_set_jpegquality: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }
    ALOGD("END native_set_jpegquality");
    return true;
}


/**
 * @brief native_get_capture_setting
 * @param camfd [IN]  camera open information
 * @param hal_capture_setting [OUT] currently capture setting
 * @
 * @return boolean
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  get capture setting
 */
static bool native_get_capture_setting(int camfd ,struct capture_setting_t *hal_capture_setting)
{
    ALOGD("START native_get_capture_setting");

    int ioctlRetVal;
    struct msm_ctrl_cmd ctrlCmd;
    camera_capture_settings_t cam_capture_settings;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_GET_PARM_CAPTURE_SETTINGS;
    ctrlCmd.length     = sizeof(camera_capture_settings_t);
    ctrlCmd.value      = (void *)&cam_capture_settings;
    ctrlCmd.resp_fd    = camfd;

    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0){
        ALOGE("native_get_capture_setting : ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }

    hal_capture_setting->jpeg_main_img_buf_length = cam_capture_settings.jpeg_main_img_buf_length;
    ALOGV("jpeg_main_img_buf_length = %d",hal_capture_setting->jpeg_main_img_buf_length);
    ALOGD("END native_get_capture_setting");
    return true;
}


static cam_frame_start_parms frame_parms;
static int recordingState = 0;

static rat_t latitude[3];
static rat_t longitude[3];
static char lonref[2];
static char latref[2];
static char dateTime[20];
static rat_t altitude;
static char maker[PROPERTY_VALUE_MAX];
static char model[PROPERTY_VALUE_MAX];
static char desc[40];
static char makernote[] = {0xFF, 0xFF, 0xFF, 0xFF};
static char usercomment[] = {0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t gps_id[] = {2,2,0,0};
static char createDateTime[20];
static rat_t expoTime;
static rat_t fNum;
static srat_t shutterSpeed;
static srat_t expoBias;
static rat_t focLen;
static rat_t zoomRatio;
static rat_t focalLength;
static rat_t gpstimestamp[3];
static char gpsDateTime[11];
static char gpsProcessingMethod[100];  //Max length is 100
static const char ExifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };
#define EXIF_ASCII_PREFIX_LEN (sizeof(ExifAsciiPrefix))

static void addExifTag(exif_tag_id_t tagid, exif_tag_type_t type,
                        uint32_t count, uint8_t copy, void *data) {
    ALOGD("START addExifTag : ");

    if(exif_table_numEntries == MAX_EXIF_TABLE_ENTRIES) {
        ALOGE("Number of entries exceeded limit");
        return;
    }

    int index = exif_table_numEntries;
    exif_data[index].tag_id = tagid;
	exif_data[index].tag_entry.type = type;
	exif_data[index].tag_entry.count = count;
	exif_data[index].tag_entry.copy = copy;
    if((type == EXIF_RATIONAL) && (count > 1))
        exif_data[index].tag_entry.data._rats = (rat_t *)data;
    if((type == EXIF_RATIONAL) && (count == 1))
        exif_data[index].tag_entry.data._rat = *(rat_t *)data;
    if((type == EXIF_SRATIONAL) && (count > 1))
        exif_data[index].tag_entry.data._srats = (srat_t *)data;
    if((type == EXIF_SRATIONAL) && (count == 1))
        exif_data[index].tag_entry.data._srat = *(srat_t *)data;
    if(type == EXIF_SHORT)
        exif_data[index].tag_entry.data._short = *(uint16_t *)data;
    if(type == EXIF_UNDEFINED)
        exif_data[index].tag_entry.data._undefined = (uint8_t *)data;
    if(type == EXIF_ASCII)
        exif_data[index].tag_entry.data._ascii = (char *)data;
    if((type == EXIF_BYTE) && (count > 1))
        exif_data[index].tag_entry.data._bytes = (uint8_t *)data;
    if((type == EXIF_BYTE) && (count == 1))
        exif_data[index].tag_entry.data._byte = *(uint8_t *)data;

    // Increase number of entries
    exif_table_numEntries++;

    ALOGD("END addExifTag : ");

}

static void parseLatLong(const char *latlonString, int *pDegrees,
                           int *pMinutes, int *pSeconds, bool *bIsNegative) {

    double value = atof(latlonString);
    if (value < 0) {
        *bIsNegative = true;
    } else {
        *bIsNegative = false;
    }

    value = fabs(value);
    int degrees = (int) value;

    double remainder = value - degrees;
    int minutes = (int) (remainder * 60);
    int seconds = (int) (((remainder * 60) - minutes) * 60 * 1000);

    *pDegrees = degrees;
    *pMinutes = minutes;
    *pSeconds = seconds;
}

static void setLatLon(exif_tag_id_t tag, const char *latlonString, bool skip_ref) {

    int degrees, minutes, seconds;
    bool isNegative;

    parseLatLong(latlonString, &degrees, &minutes, &seconds, &isNegative);

    rat_t value[3] = { {degrees, 1},
                       {minutes, 1},
                       {seconds, 1000} };

    if(tag == EXIFTAGID_GPS_LATITUDE) {
        memcpy(latitude, value, sizeof(latitude));
        addExifTag(EXIFTAGID_GPS_LATITUDE, EXIF_RATIONAL, 3,
                    1, (void *)latitude);

        if (!skip_ref) {
            //set Latitude Ref
            latref[0] = isNegative ? 'S' : 'N';
            latref[1] = '\0';
            ALOGV("Using implicit LATITUDE_REF: '%s'", latref);
            addExifTag(EXIFTAGID_GPS_LATITUDE_REF, EXIF_ASCII, 2,
                        1, (void *)latref);
        }
    } else {
        memcpy(longitude, value, sizeof(longitude));
        addExifTag(EXIFTAGID_GPS_LONGITUDE, EXIF_RATIONAL, 3,
                    1, (void *)longitude);

        if (!skip_ref) {
            //set Longitude Ref
            lonref[0] = isNegative ? 'W' : 'E';
            lonref[1] = '\0';
            ALOGV("Using implicit LONGITUDE_REF: '%s'", lonref);
            addExifTag(EXIFTAGID_GPS_LONGITUDE_REF, EXIF_ASCII, 2,
                        1, (void *)lonref);
        }
    }
}

void SemcCameraHardware::setGpsParameters() {
    ALOGD("START setGpsParameters : ");

    const char *str = NULL;
    const char *str_semc_ref = NULL;
    bool  use_semc_ref;

    /* gps version id */
    addExifTag(EXIFTAGID_GPS_VERSION_ID, EXIF_BYTE, 4, 0, (void *)&gps_id);

    // QC / SEMC specific key
    str_semc_ref = mParameters.get(CameraParameters::KEY_GPS_LATITUDE_REF);
    use_semc_ref = (str_semc_ref != NULL);

    //Set Latitude
    str = mParameters.get(CameraParameters::KEY_GPS_LATITUDE);
    if(str != NULL) {
        setLatLon(EXIFTAGID_GPS_LATITUDE, str, use_semc_ref);
        //set Latitude Ref
        // check if SEMC specific extention is used
        if (use_semc_ref) {
            strncpy(latref, str_semc_ref, 1);
            latref[1] = '\0';
            ALOGV("Using SEMC LATITUDE_REF: '%s'", latref);
            addExifTag(EXIFTAGID_GPS_LATITUDE_REF, EXIF_ASCII, 2,
                        1, (void *)latref);
        }
    }
    str = NULL;
    str_semc_ref = NULL;

    // QC / SEMC specific key
    str_semc_ref = mParameters.get(CameraParameters::KEY_GPS_LONGITUDE_REF);
    use_semc_ref = (str_semc_ref != NULL);

    //set Longitude
    str = mParameters.get(CameraParameters::KEY_GPS_LONGITUDE);
    if(str != NULL) {
        setLatLon(EXIFTAGID_GPS_LONGITUDE, str, use_semc_ref);
        //set Longitude Ref
        if (use_semc_ref) {
            strncpy(lonref, str_semc_ref, 1);
            lonref[1] = '\0';
            ALOGV("Using SEMC LONGITUDE_REF: '%s'", lonref);
            addExifTag(EXIFTAGID_GPS_LONGITUDE_REF, EXIF_ASCII, 2,
                        1, (void *)lonref);
        }
    }

    //set Altitude
    str = NULL;
    str = mParameters.get(CameraParameters::KEY_GPS_ALTITUDE);
    if(str != NULL) {
        int value = atoi(str);
        int ref = (value < 0) ? 1 : 0; // 0: sea level; 1: sea level negative
        value = abs(value);
        rat_t alt_value = {value, 1000};
        memcpy(&altitude, &alt_value, sizeof(altitude));
        addExifTag(EXIFTAGID_GPS_ALTITUDE, EXIF_RATIONAL, 1,
                    1, (void *)&altitude);

        // check if QC /SEMC specific key is used
        int semc_ref = mParameters.getInt(CameraParameters::KEY_GPS_ALTITUDE_REF);
        if( !(semc_ref < 0 || semc_ref > 1) ) {
            ALOGV("Using SEMC ALTITUDE_REF: '%s'", semc_ref);
            ref = semc_ref;
        }

        //set AltitudeRef
        addExifTag(EXIFTAGID_GPS_ALTITUDE_REF, EXIF_BYTE, 1,
                    1, (void *)&ref);
    }
    str = NULL;
    str = mParameters.get(CameraParameters::KEY_GPS_TIMESTAMP);
    if(str != NULL) {
          long timevalue = atol(str);
          struct tm *gtm;
          gtm = gmtime(&timevalue);
          rat_t stampvalue[3] = { {gtm->tm_hour, 1},
                                  {gtm->tm_min,  1},
                                  {gtm->tm_sec,  1} };
          memcpy(gpstimestamp, stampvalue, sizeof(gpstimestamp));
          addExifTag(EXIFTAGID_GPS_TIMESTAMP, EXIF_RATIONAL, 3,
                      1, (void *)&gpstimestamp);

      /*set GpsDateStamp*/

      char date[11];
      memset(date, 0x00, sizeof(date));
      memset(gpsDateTime, 0x00, sizeof(gpsDateTime));

      sprintf(date,"%04d:%02d:%02d",gtm->tm_year,gtm->tm_mon,
      gtm->tm_mday);

      if (strlen(date) >= 1) {
      strncpy(gpsDateTime, date, 10);
      gpsDateTime[10] = '\0';
              ALOGD("GPS DateStamp is %s", gpsDateTime);
      addExifTag(EXIFTAGID_GPS_DATESTAMP, EXIF_ASCII,
                  11, 1, (void *)&gpsDateTime);
      }
    }
    /* set ProcessingMethod */
    str = NULL;
    str = mParameters.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    if(str != NULL) {
            memset(gpsProcessingMethod, 0x00, sizeof(gpsProcessingMethod));
            memcpy(gpsProcessingMethod, ExifAsciiPrefix,EXIF_ASCII_PREFIX_LEN);   //copy the first 8 bytes identifier
            memcpy((char *)gpsProcessingMethod+EXIF_ASCII_PREFIX_LEN, str,strlen(str)); //append the string passed by the test case.
            addExifTag(EXIFTAGID_GPS_PROCESSINGMETHOD, EXIF_UNDEFINED,
                EXIF_ASCII_PREFIX_LEN+strlen(str), 1, (void *)gpsProcessingMethod);
        }
}

bool SemcCameraHardware::native_jpeg_encode(void)
{
    ALOGD("START native_jpeg_encode : ");

    int jpeg_quality = mParameters.getInt("jpeg-quality");
    if (jpeg_quality >= 0) {
        ALOGV("native_jpeg_encode, current jpeg main img quality =%d",
             jpeg_quality);
        if(!LINK_jpeg_encoder_setMainImageQuality(jpeg_quality)) {
            ALOGE("native_jpeg_encode set jpeg-quality failed");
            return false;
        }
    }

    int thumbnail_quality = mParameters.getInt("jpeg-thumbnail-quality");
    if (thumbnail_quality >= 0) {
        ALOGV("native_jpeg_encode, current jpeg thumbnail quality =%d",
             thumbnail_quality);
        if(!LINK_jpeg_encoder_setThumbnailQuality(thumbnail_quality)) {
            ALOGE("native_jpeg_encode set thumbnail-quality failed");
            return false;
        }
    }

//    jpeg_set_location();
    if (((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
        jpeg_set_location();
        if (mEncodeLocation) {
            ALOGV("native_jpeg_encode:setGpsParameters_call(3rdParty)");
            setGpsParameters();
        }
    } else {
        if(mParameters.getInt(CameraParameters::KEY_GPS_STATUS) == 1) {
            ALOGV("native_jpeg_encode:setGpsParameters_call(Signature)");
	        setGpsParameters();
	    }
    }

    setExifParameters();
    //Because of the preview size thumbnail image, we need process to convert for the thumbnail size.
    if(!getThumbnailInternal()){
        ALOGE("native_jpeg_encode: getThumbnailInternal is error.");
        return false;
    }

    cam_ctrl_dimension_t thumnail_dmension;
    memset(&thumnail_dmension, 0, sizeof(cam_ctrl_dimension_t));

    thumnail_dmension.orig_picture_dx = CAMERA_NORMALTHUMBNAIL_WIDTH;
    thumnail_dmension.orig_picture_dy = CAMERA_NORMALTHUMBNAIL_HEIGHT;
    thumnail_dmension.thumbnail_width  = CAMERA_NORMALTHUMBNAIL_WIDTH;
    thumnail_dmension.thumbnail_height = CAMERA_NORMALTHUMBNAIL_HEIGHT;

   int thumbfd = mThumbnailHeap->mHeap->getHeapID();
   uint8_t * thumbnailHeap = (uint8_t *)mThumbnailHeap->mHeap->base();;
   int width = -1, height = -1;
   width = mParameters.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
   height = mParameters.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
   if (width == 0 || height == 0) {
       thumbnailHeap = NULL;
       thumbfd = 0;
   }



    if (!LINK_jpeg_encoder_encode(&thumnail_dmension,
                                  thumbnailHeap,
                                  thumbfd,
                                  (uint8_t *)mRawHeap->mHeap->base(),
                                  mRawHeap->mHeap->getHeapID(),
                                  &mCrop, exif_data, exif_table_numEntries,
                                  jpegPadding/2)) {
        ALOGE("END native_jpeg_encode: jpeg_encoder_encode is error.");
        return false;
    }

    return true;
}

bool SemcCameraHardware::native_set_parm(
    cam_ctrl_type type, uint16_t length, void *value)
{
    ALOGD("START native_set_parm : cam_ctrl_type[%d]",type);


    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = (uint16_t)type;
    ctrlCmd.length     = length;
    // FIXME: this will be put in by the kernel
    ctrlCmd.resp_fd    = mCameraControlFd;
    ctrlCmd.value = value;

    ALOGV("%s: fd %d, type %d, length %d", __FUNCTION__,
         mCameraControlFd, type, length);
    if (ioctl(mCameraControlFd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0 ||
                ctrlCmd.status != CAM_CTRL_SUCCESS) {
        ALOGE("%s: error (%s): fd %d, type %d, length %d, status %d",
             __FUNCTION__, strerror(errno),
             mCameraControlFd, type, length, ctrlCmd.status);
        return false;
    }

    ALOGD("END native_set_parm : ");

    return true;
}

void SemcCameraHardware::jpeg_set_location()
{
    bool encode_location = true;
    camera_position_type pt;

#define PARSE_LOCATION(what,type,fmt,desc) do {                                \
        pt.what = 0;                                                           \
        const char *what##_str = mParameters.get("gps-"#what);                 \
        ALOGV("GPS PARM %s --> [%s]", "gps-"#what, what##_str);                 \
        if ((what##_str) && (strlen(what##_str) >= 1)) {                       \
            type what = 0;                                                     \
            if (sscanf(what##_str, fmt, &what) == 1)                           \
                pt.what = what;                                                \
            else {                                                             \
                ALOGE("GPS " #what " %s could not"                              \
                     " be parsed as a " #desc, what##_str);                    \
                encode_location = false;                                       \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            ALOGV("GPS " #what " not specified: "                               \
                 "defaulting to zero in EXIF header.");                        \
            encode_location = false;                                           \
       }                                                                       \
    } while(0)

    PARSE_LOCATION(timestamp, long, "%ld", "long");
    if (!pt.timestamp) pt.timestamp = time(NULL);
    PARSE_LOCATION(altitude, short, "%hd", "short");
    PARSE_LOCATION(latitude, double, "%lf", "double float");
    PARSE_LOCATION(longitude, double, "%lf", "double float");

#undef PARSE_LOCATION

    if (encode_location) {
        ALOGD("setting image location ALT %d LAT %lf LON %lf",
             pt.altitude, pt.latitude, pt.longitude);
/* Disabling until support is available.
        if (!LINK_jpeg_encoder_setLocation(&pt)) {
            ALOGE("jpeg_set_location: LINK_jpeg_encoder_setLocation failed.");
        }
*/
    }
    else ALOGV("not setting image location");

    mEncodeLocation = encode_location;
}

void SemcCameraHardware::runFrameThread(void *data)
{
    ALOGV("runFrameThread E");

    int cnt;

#if DLOPEN_LIBMMCAMERA
    // We need to maintain a reference to libqcamera.so for the duration of the
    // frame thread, because we do not know when it will exit relative to the
    // lifetime of this object.  We do not want to dlclose() libqcamera while
    // LINK_cam_frame is still running.
    void *libhandle = ::dlopen("liboemcamera.so", RTLD_NOW);
    ALOGV("FRAME: loading libqcamera at %p", libhandle);
    if (!libhandle) {
        ALOGE("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
    }
    if (libhandle)
#endif
    {
        LINK_cam_frame(data);
    }

    mPreviewHeap.clear();
    if(( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250))
        mRecordHeap.clear();

#if DLOPEN_LIBMMCAMERA
    if (libhandle) {
        ::dlclose(libhandle);
        ALOGV("FRAME: dlclose(libqcamera)");
    }
#endif

    mFrameThreadWaitLock.lock();
    mFrameThreadRunning = false;
    mFrameThreadWait.signal();
    mFrameThreadWaitLock.unlock();

    ALOGV("runFrameThread X");
}

void SemcCameraHardware::runVideoThread(void *data)
{
    ALOGD("runVideoThread E");
    msm_frame* vframe = NULL;

    while(true) {
        pthread_mutex_lock(&(g_busy_frame_queue.mut));

        // Exit the thread , in case of stop recording..
        mVideoThreadWaitLock.lock();
        if(mVideoThreadExit){
            ALOGV("Exiting video thread..");
            mVideoThreadWaitLock.unlock();
            pthread_mutex_unlock(&(g_busy_frame_queue.mut));
            break;
        }
        mVideoThreadWaitLock.unlock();

        ALOGV("in video_thread : wait for video frame ");
        // check if any frames are available in busyQ and give callback to
        // services/video encoder
        cam_frame_wait_video();
        ALOGV("video_thread, wait over..");

        // Exit the thread , in case of stop recording..
        mVideoThreadWaitLock.lock();
        if(mVideoThreadExit){
            ALOGE("Exiting video thread..");
            mVideoThreadWaitLock.unlock();
            pthread_mutex_unlock(&(g_busy_frame_queue.mut));
            break;
        }
        mVideoThreadWaitLock.unlock();

        // Get the video frame to be encoded
        vframe = cam_frame_get_video ();
        pthread_mutex_unlock(&(g_busy_frame_queue.mut));
        ALOGV("in video_thread : got video frame ");

        if (UNLIKELY(mDebugFps)) {
            debugShowVideoFPS();
        }

        if(vframe != NULL) {
            // Find the offset within the heap of the current buffer.
            ALOGV("Got video frame :  buffer %d base %d ", vframe->buffer, mRecordHeap->mHeap->base());
            ssize_t offset =
                (ssize_t)vframe->buffer - (ssize_t)mRecordHeap->mHeap->base();
            ALOGV("offset = %d , alignsize = %d , offset later = %d", offset, mRecordHeap->mAlignedBufferSize, (offset / mRecordHeap->mAlignedBufferSize));

            offset /= mRecordHeap->mAlignedBufferSize;

            // dump frames for test purpose
#ifdef DUMP_VIDEO_FRAMES
            static int frameCnt = 0;
            if (frameCnt >= 11 && frameCnt <= 13 ) {
                char buf[128];
                sprintf(buf, "/data/%d_v.yuv", frameCnt);
                int file_fd = open(buf, O_RDWR | O_CREAT, 0777);
                ALOGV("dumping video frame %d", frameCnt);
                if (file_fd < 0) {
                    ALOGE("cannot open file\n");
                }
                else
                {
                    write(file_fd, (const void *)vframe->buffer,
                        vframe->cbcr_off * 3 / 2);
                }
                close(file_fd);
          }
          frameCnt++;
#endif
            // Enable IF block to give frames to encoder , ELSE block for just simulation
#if 1
            ALOGV("in video_thread : got video frame, before if check giving frame to services/encoder");
            mCallbackLock.lock();
            int msgEnabled = mMsgEnabled;
            data_callback_timestamp rcb = mDataCallbackTimestamp;
            void *rdata = mCallbackCookie;
            mCallbackLock.unlock();

            if(rcb != NULL && (msgEnabled & CAMERA_MSG_VIDEO_FRAME) ) {
                ALOGV("in video_thread : got video frame, giving frame to services/encoder");
                /* Extract the timestamp of this frame */
                nsecs_t timeStamp = nsecs_t(vframe->ts.tv_sec)*1000000000LL + vframe->ts.tv_nsec;
                rcb(timeStamp, CAMERA_MSG_VIDEO_FRAME, mRecordHeap->mBuffers[offset], rdata);
            }
#else
            // 720p output2  : simulate release frame here:
            ALOGE("in video_thread simulation , releasing the video frame");
            LINK_camframe_free_video(vframe);
#endif

        } else ALOGE("in video_thread get frame returned null");


    } // end of while loop

    mVideoThreadWaitLock.lock();
    mVideoThreadRunning = false;
    mVideoThreadWait.signal();
    mVideoThreadWaitLock.unlock();

    ALOGV("runVideoThread X");
}

void *video_thread(void *user)
{
    ALOGV("video_thread E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->runVideoThread(user);
    }
    else ALOGE("not starting video thread: the object went away!");
    ALOGV("video_thread X");
    return NULL;
}

void *frame_thread(void *user)
{
    ALOGD("frame_thread E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->runFrameThread(user);
    }
    else ALOGW("not starting frame thread: the object went away!");
    ALOGD("frame_thread X");
    return NULL;
}

bool SemcCameraHardware::initPreview()
{
    ALOGD("START initPreview : ");

    // See comments in deinitPreview() for why we have to wait for the frame
    // thread here, and why we can't use pthread_join().
    int videoWidth, videoHeight;
    mParameters.getPreviewSize(&previewWidth, &previewHeight);

    if((CAMERA_FWVGASIZE_WIDTH == previewWidth && CAMERA_FWVGASIZE_HEIGHT == previewHeight) ||
       (CAMERA_OHDFWVGASIZE_WIDTH == previewWidth && CAMERA_OHDFWVGASIZE_HEIGHT == previewHeight)){
        previewWidth = CAMERA_FWVGASIZE_DRV_WIDTH;
    }

    
    videoWidth = previewWidth;  // temporary , should be configurable later
    videoHeight = previewHeight;
    ALOGV("initPreview E: preview size=%dx%d videosize = %d x %d", previewWidth, previewHeight, videoWidth, videoHeight );

    if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250)) {
        mDimension.video_width = videoWidth;
        mDimension.video_width = CEILING16(mDimension.video_width);
        mDimension.video_height = videoHeight;
        // for 720p , preview can be 768X432
        previewWidth = mDimension.display_width;
        previewHeight= mDimension.display_height;
        ALOGV("initPreview : preview size=%dx%d videosize = %d x %d", previewWidth, previewHeight, mDimension.video_width, mDimension.video_height );
    }


    mFrameThreadWaitLock.lock();
    while (mFrameThreadRunning) {
        ALOGV("initPreview: waiting for old frame thread to complete.");
        mFrameThreadWait.wait(mFrameThreadWaitLock);
        ALOGV("initPreview: old frame thread completed.");
    }
    mFrameThreadWaitLock.unlock();

    mSnapshotThreadWaitLock.lock();
    while (mSnapshotThreadRunning) {
        ALOGV("initPreview: waiting for old snapshot thread to complete.");
        mSnapshotThreadWait.wait(mSnapshotThreadWaitLock);
        ALOGV("initPreview: old snapshot thread completed.");
    }
    mSnapshotThreadWaitLock.unlock();

    int cnt = 0;
    mPreviewFrameSize = previewWidth * previewHeight * 3/2;
    dstOffset = 0;
    mPreviewHeap = new PmemPool("/dev/pmem_adsp",
                                MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
                                mCameraControlFd,
                                MSM_PMEM_PREVIEW, //MSM_PMEM_OUTPUT2,
                                mPreviewFrameSize,
                                kPreviewBufferCountActual,
                                mPreviewFrameSize,
                                "preview");

    if (!mPreviewHeap->initialized()) {
        mPreviewHeap.clear();
        ALOGE("initPreview X: could not initialize Camera preview heap.");
        return false;
    }
    if( mCurrentTarget == TARGET_MSM7630 ) {
	if(mPostViewHeap == NULL) {
	    ALOGV(" Allocating Postview heap ");
	    /* mPostViewHeap should be declared only for 7630 target */
	    mPostViewHeap =
		new PmemPool("/dev/pmem_adsp",
			MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
			mCameraControlFd,
			MSM_PMEM_PREVIEW, //MSM_PMEM_OUTPUT2,
			mPreviewFrameSize,
			1,
			mPreviewFrameSize,
			"postview");

	    if (!mPostViewHeap->initialized()) {
        mPreviewHeap.clear();
		mPostViewHeap.clear();
		ALOGE(" Failed to initialize Postview Heap");
		return false;
	    }
	}
    }

    if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250) ) {

        // Allocate video buffers after allocating preview buffers.
        if(!initRecord()){
            mPreviewHeap.clear();
            mPostViewHeap.clear();
            ALOGE("END initPreview : initRecord Error");
            return false;
        }
    }

    //int mode = 1;
    //bool ret_set_param = native_set_parm(CAMERA_SET_PARM_PREVIEW_MODE, sizeof(mode), (void *)&mode);
    //if(false == ret_set_param) {
    //    mPreviewHeap.clear();
    //    mPostViewHeap.clear();
    //    ALOGE("END initPreview : set previewmode Error");
    //    return false;
    //}

#if SCRITCH_OFF
    int fps_mode = CAMERA_LIMIT_MINIMUM_FPS_DISABLE;
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) // case 3rdparty
        fps_mode = CAMERA_LIMIT_MINIMUM_FPS_ENABLE;

    ret_set_param = native_set_parm(CAMERA_SET_LIMIT_MINIMUM_FPS, sizeof(fps_mode), (void *)&fps_mode);
    if(false == ret_set_param) {
        mPreviewHeap.clear();
        mPostViewHeap.clear();
        ALOGE("END initPreview : set limit minimum fps Error");
        return false;
    }
#endif//SCRITCH_OFF

    if((CAMERA_FWVGASIZE_DRV_WIDTH == mDimension.display_width) && (CAMERA_FWVGASIZE_DRV_HEIGHT == mDimension.display_height)){
        mDimension.display_width = CAMERA_FWVGASIZE_DRV_WIDTH;
        mDimension.ui_thumbnail_width  = CAMERA_WQVGASIZE_WIDTH;
        mDimension.ui_thumbnail_height = CAMERA_WQVGASIZE_HEIGHT;
        mDimension.thumbnail_width  = CAMERA_WQVGASIZE_WIDTH;
        mDimension.thumbnail_height = CAMERA_WQVGASIZE_HEIGHT;
    }else{
        mDimension.ui_thumbnail_width  = CAMERA_QVGASIZE_WIDTH;
        mDimension.ui_thumbnail_height = CAMERA_QVGASIZE_HEIGHT;
        mDimension.thumbnail_width  = CAMERA_QVGASIZE_WIDTH;
        mDimension.thumbnail_height = CAMERA_QVGASIZE_HEIGHT;
    }

    if (!picsizeCheck(mDimension.picture_width ,mDimension.picture_height)) {
        mPreviewHeap.clear();
        mPostViewHeap.clear();
        ALOGE("END initPreview  picsizeCheck failed.  Not starting preview.");
        return false;
    }

    if (((CAMERA_HD720PSIZE_WIDTH == mDimension.video_width) && (CAMERA_HD720PSIZE_HEIGHT == mDimension.video_height))
        && (((CAMERA_SIGNATURE_HD720P_DISPSIZE_WIDTH == mDimension.display_width) && (CAMERA_SIGNATURE_HD720P_DISPSIZE_HEIGHT == mDimension.display_height))
        || ((CAMERA_3RDPARTY_HD720P_DISPSIZE_WIDTH == mDimension.display_width) && (CAMERA_3RDPARTY_HD720P_DISPSIZE_HEIGHT == mDimension.display_height)))) {
        mDimension.postview_width = CAMERA_FWVGASIZE_DRV_WIDTH;
        mDimension.postview_height = CAMERA_FWVGASIZE_DRV_HEIGHT;
    } else {
        mDimension.postview_width = mDimension.display_width;
        mDimension.postview_height = mDimension.display_height;
    }

    // mDimension will be filled with thumbnail_width, thumbnail_height,
    // orig_picture_dx, and orig_picture_dy after this function call. We need to
    // keep it for jpeg_encoder_encode.
    bool ret = native_set_parm(CAMERA_SET_PARM_DIMENSION,
                               sizeof(cam_ctrl_dimension_t), &mDimension);

    if (ret) {
        for (cnt = 0; cnt < kPreviewBufferCount; cnt++) {
            frames[cnt].fd = mPreviewHeap->mHeap->getHeapID();
            frames[cnt].buffer =
                (uint32_t)mPreviewHeap->mHeap->base() + mPreviewHeap->mAlignedBufferSize * cnt;
            frames[cnt].y_off = 0;
            frames[cnt].cbcr_off = previewWidth * previewHeight;
            frames[cnt].path = OUTPUT_TYPE_P; // MSM_FRAME_ENC;
        }

        mFrameThreadWaitLock.lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        frame_parms.frame = frames[kPreviewBufferCount - 1];
        frame_parms.video_frame =  recordframes[ACTIVE_VIDEO_BUFFERS];

        ALOGV ("initpreview before cam_frame thread carete , video frame  buffer=%lu fd=%d y_off=%d cbcr_off=%d \n",
          (unsigned long)frame_parms.video_frame.buffer, frame_parms.video_frame.fd, frame_parms.video_frame.y_off,
          frame_parms.video_frame.cbcr_off);
        mFrameThreadRunning = !pthread_create(&mFrameThread,
                                              &attr,
                                              frame_thread,
                                              (void*)&(frame_parms));
        ret = mFrameThreadRunning;
        mFrameThreadWaitLock.unlock();
    } else {
        mPreviewHeap.clear();
        mPostViewHeap.clear();
        ALOGE("initPreview SET_PARM_DIMENSION failed");
    }

    ALOGD("END initPreview ");

    return ret;
}

void SemcCameraHardware::deinitPreview(void)
{
    ALOGD("START deinitPreview : ");


    // When we call deinitPreview(), we signal to the frame thread that it
    // needs to exit, but we DO NOT WAIT for it to complete here.  The problem
    // is that deinitPreview is sometimes called from the frame-thread's
    // callback, when the refcount on the Camera client reaches zero.  If we
    // called pthread_join(), we would deadlock.  So, we just call
    // LINK_camframe_terminate() in deinitPreview(), which makes sure that
    // after the preview callback returns, the camframe thread will exit.  We
    // could call pthread_join() in initPreview() to join the last frame
    // thread.  However, we would also have to call pthread_join() in release
    // as well, shortly before we destroy the object; this would cause the same
    // deadlock, since release(), like deinitPreview(), may also be called from
    // the frame-thread's callback.  This we have to make the frame thread
    // detached, and use a separate mechanism to wait for it to complete.

    LINK_camframe_terminate();

    ALOGD("END deinitPreview : ");
}

bool SemcCameraHardware::initRawSnapshot()
{
    ALOGV("initRawSnapshot E");

    //get width and height from Dimension Object
    bool ret = native_set_parm(CAMERA_SET_PARM_DIMENSION,
                               sizeof(cam_ctrl_dimension_t), &mDimension);

    if(!ret){
        ALOGE("initRawSnapshot X: failed to set dimension");
        return false;
    }
    int rawSnapshotSize = mDimension.raw_picture_height *
                           mDimension.raw_picture_width;

    ALOGV("raw_snapshot_buffer_size = %d, raw_picture_height = %d, "\
         "raw_picture_width = %d",
          rawSnapshotSize, mDimension.raw_picture_height,
          mDimension.raw_picture_width);

    if (mRawSnapShotPmemHeap != NULL) {
        ALOGV("initRawSnapshot: clearing old mRawSnapShotPmemHeap.");
        mRawSnapShotPmemHeap.clear();
    }

    //Pmem based pool for Camera Driver
    mRawSnapShotPmemHeap = new PmemPool("/dev/pmem_adsp",
                                    MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
                                    mCameraControlFd,
                                    MSM_PMEM_RAW_MAINIMG,
                                    rawSnapshotSize,
                                    1,
                                    rawSnapshotSize,
                                    "raw pmem snapshot camera");

    if (!mRawSnapShotPmemHeap->initialized()) {
        mRawSnapShotPmemHeap.clear();
        ALOGE("initRawSnapshot X: error initializing mRawSnapshotHeap");
        return false;
    }
    ALOGV("initRawSnapshot X");
    return true;

}

bool SemcCameraHardware::initRaw(bool initJpegHeap)
{
    int rawWidth, rawHeight;

    mParameters.getPictureSize(&rawWidth, &rawHeight);

    ALOGD("START initRaw : picture size=%dx%d", rawWidth, rawHeight);

    if (mJpegHeap != NULL) {
        ALOGV("initRaw: clearing old mJpegHeap.");
        mJpegHeap.clear();
    }

    if (mThumbnailHeap != NULL) {
        mThumbnailHeap.clear();
        memset(&mExifInfo, 0, sizeof(exifinfo_t));
    }

    mRawSize = mDimension.postview_width * mDimension.postview_height * 1.5;


    if( mCurrentTarget == TARGET_MSM7627 )
             mJpegMaxSize = CEILING16(rawWidth) * CEILING16(rawHeight) * 3 / 2;
    else
             mJpegMaxSize = rawWidth * rawHeight * 3 / 2;

    if(mRawHeap != NULL){
        mRawHeap.clear();
    }

    ALOGV("initRaw: initializing mRawHeap.");
    mRawHeap =
        new PmemPool("/dev/pmem_adsp",
                     MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
                     mCameraControlFd,
                     MSM_PMEM_MAINIMG,

                     mRawSize,

                     kRawBufferCount,
                     mRawSize,
                     "snapshot camera");

    if (!mRawHeap->initialized()) {
	ALOGE("initRaw X failed ");
	mRawHeap.clear();
	ALOGE("initRaw X: error initializing mRawHeap");
	return false;
    }

    ALOGV("do_mmap snapshot pbuf = %p, pmem_fd = %d",
         (uint8_t *)mRawHeap->mHeap->base(), mRawHeap->mHeap->getHeapID());

    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0){
        if(mRawHeapSub != NULL){
            mRawHeapSub.clear();
        }
        int rawSizeSub;

        rawSizeSub = mDimension.postview_width * mDimension.postview_height * 1.5;

        mRawHeapSub = new AshmemPool(rawSizeSub,
                                     kJpegBufferCount,
                                     rawSizeSub,
                                     "snapshot camera");

        if (!mRawHeapSub->initialized()) {
            mRawHeapSub.clear();
            mRawHeap.clear();
            ALOGE("initRaw X: error initializing mRawHeapSub");
            return false;
        }
    }

        // Thumbnails

        if(CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize || CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize || CAMERAHAL_PICSIZE_6M == mCameraPicsize){
            mThumbnailBufferSize =  CAMERA_WQVGASIZE_WIDTH * CAMERA_WQVGASIZE_HEIGHT * 1.5 ;
        }else{
            mThumbnailBufferSize =  CAMERA_QVGASIZE_WIDTH * CAMERA_QVGASIZE_HEIGHT * 1.5 ;
        }

        mThumbnailHeap =
            new PmemPool("/dev/pmem_adsp",
                         MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
                         mCameraControlFd,
                         MSM_PMEM_THUMBNAIL,

                         mThumbnailBufferSize,
                         1,
                         mThumbnailBufferSize,

                         "thumbnail");

        if (!mThumbnailHeap->initialized()) {
            mThumbnailHeap.clear();

            mRawHeapSub.clear();

            mRawHeap.clear();
            ALOGE("initRaw X failed: error initializing mThumbnailHeap.");
            return false;
        }

    ALOGD("END initRaw ");

    return true;
}


void SemcCameraHardware::deinitRawSnapshot()
{
    ALOGV("deinitRawSnapshot E");
    mRawSnapShotPmemHeap.clear();
    mRawSnapshotAshmemHeap.clear();
    ALOGV("deinitRawSnapshot X");
}

void SemcCameraHardware::deinitRaw()
{
    ALOGD("START deinitRaw : ");
    ALOGV("deinitRaw E");

    mThumbnailHeap.clear();
    mJpegHeap.clear();
    mRawHeap.clear();
    mDisplayHeap.clear();
    mRawHeapSub.clear();
    mJpegSnapShotHeap.clear();

    ALOGD("END deinitRaw : ");
}

void SemcCameraHardware::release()
{
    ALOGD("START release : ");

    Mutex::Autolock l(&mLock);
    mCameraExitFlag = true;
#if DLOPEN_LIBMMCAMERA
    if (libmmcamera == NULL) {
        ALOGE("ERROR: multiple release!");
        return;
    }
#else
#warning "Cannot detect multiple release when not dlopen()ing libqcamera!"
#endif

    int cnt, rc;
    struct msm_ctrl_cmd ctrlCmd;
    Mutex::Autolock singletonLock(&singleton_lock);

    if (mCameraRunning) {
        if(mDataCallbackTimestamp && (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME)) {
            mRecordFrameLock.lock();
            mReleasedRecordingFrame = true;
            mRecordWait.signal();
            mRecordFrameLock.unlock();
        }
        stopPreviewInternal();
    }
#if SCRITCH_OFF
    if(setFlashlightBrightness(mParameters, CameraParameters::TALLY_LIGHT,
       CameraParameters::TALLY_LIGHT_OFF) != NO_ERROR) {
        ALOGE("release : setFlashlightBrightness error.");
    }
#endif//SCRITCH_OFF

    if( mCurrentTarget == TARGET_MSM7630 ) {
	mPostViewHeap.clear();
        mPostViewHeap = NULL;
    }
    if(mReceiveExifDataFlag) {
        ALOGV("takePicture: waiting for old snapshot thread to complete.");
        mReleaseWaitLock.lock();
        mReleaseWait.wait(mReleaseWaitLock);
        mReleaseWaitLock.unlock();
        ALOGV("takePicture: old snapshot thread completed.");
    }

    LINK_jpeg_encoder_join();
    if(mEncoderThreadInfo) {
        ALOGV("mEncoderThreadInfo is not null");
        pthread_join(mEncoderThreadInfo,NULL);
    }
    deinitRaw();
    deinitRawSnapshot();
    {
	Mutex::Autolock l(&mCamframeTimeoutLock);
	if(!camframe_timeout_flag) {

	    ctrlCmd.timeout_ms = 5000;
	    ctrlCmd.length = 0;
	    ctrlCmd.type = (uint16_t)CAMERA_EXIT;
	    ctrlCmd.resp_fd = mCameraControlFd; // FIXME: this will be put in by the kernel
	    if (ioctl(mCameraControlFd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0)
		ALOGE("ioctl CAMERA_EXIT fd %d error %s",
			mCameraControlFd, strerror(errno));

	}
    }
    LINK_release_cam_conf_thread();
    close(mCameraControlFd);
    mCameraControlFd = -1;
    if(fb_fd >= 0) {
        close(fb_fd);
        fb_fd = -1;
    }
#if DLOPEN_LIBMMCAMERA
    if (libmmcamera) {
        ::dlclose(libmmcamera);
        ALOGV("dlclose(libqcamera)");
        libmmcamera = NULL;
    }
    if(libmmipl) {
        ::dlclose(libmmipl);
        ALOGV("release : dlclose(libmmipl)");
        libmmipl = NULL;
    }
#endif
    singleton.clear();

    ALOGD("END release : ");

}

SemcCameraHardware::~SemcCameraHardware()
{
    ALOGD("START ~SemcCameraHardware : ");

    ALOGD("END ~SemcCameraHardware : ");

}

sp<IMemoryHeap> SemcCameraHardware::getRawHeap() const
{
    ALOGD("getRawHeap : ");

    return mDisplayHeap != NULL ? mDisplayHeap->mHeap : NULL;
}

sp<IMemoryHeap> SemcCameraHardware::getPreviewHeap() const
{

    ALOGD("getPreviewHeap : ");

    return mPreviewHeap != NULL ? mPreviewHeap->mHeap : NULL;
}

status_t SemcCameraHardware::startPreviewInternal()
{
    ALOGD("START startPreviewInternal");
    
    int ioctlRetVal = true;

    ALOGV("in startPreviewInternal : E");

    if(mCameraRunning) {
        ALOGV("startPreview X: preview already running.");
        return NO_ERROR;
    }
    if(CAMERAHAL_STATE_TAKEPICSTART == mStateCameraHal){
        mInSnapshotModeWaitLock.lock();
        ALOGV("startPreviewInternal: waiting complete.");
        mInSnapshotModeWait.wait(mInSnapshotModeWaitLock);
        ALOGV("startPreviewInternal: old waiting completed.");
        mInSnapshotModeWaitLock.unlock();
    }else {
        if((CAMERAHAL_STATE_INIT != mStateCameraHal) &&
           (CAMERAHAL_STATE_PREVIEWSTOP != mStateCameraHal) &&
           (CAMERAHAL_STATE_TAKEPICDONE != mStateCameraHal)){
            ALOGW("startPreviewInternal state Error");
            return UNKNOWN_ERROR;
        }
    }

    if (!mPreviewInitialized) {
        mLastQueuedFrame = NULL;
        mPreviewInitialized = initPreview();
        if (!mPreviewInitialized) {
            ALOGE("startPreview X initPreview failed.  Not starting preview.");
            return UNKNOWN_ERROR;
        }
    }

    CameraHalState oldState = mStateCameraHal;
    mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
    mPreviewFlag = false;

    {
        Mutex::Autolock cameraRunningLock(&mCameraRunningLock);
        if(( mCurrentTarget != TARGET_MSM7630 ) &&
                (mCurrentTarget != TARGET_QSD8250))
            mCameraRunning = native_start_preview(mCameraControlFd);
        else
            mCameraRunning = native_start_video(mCameraControlFd);
    }

    if(!mCameraRunning) {
        deinitPreview();
        mPreviewInitialized = false;
        mOverlay = NULL;
        ALOGE("startPreview X: native_start_preview failed!");

        mStateCameraHal = oldState;

        return UNKNOWN_ERROR;
    }

    //Reset the Gps Information
    exif_table_numEntries = 0;

    if(native_get_maxzoom(mCameraControlFd, (void *)&mMaxZoom) == true){
        ALOGD("Maximum zoom value is %d", mMaxZoom);
        mParameters.set("zoom-supported", "true");
    } else {
        ALOGE("Failed to get maximum zoom value...setting max zoom to zero");
        mParameters.set("zoom-supported", "false");
        mMaxZoom = 0;
    }
    mParameters.set("max-zoom",mMaxZoom);
    {
        Mutex::Autolock lock(&mSetCollective3rdParty);
        ALOGD("%s mPreviewFlag %d", __FUNCTION__, mPreviewFlag);
        while(!mPreviewFlag) {
            ALOGD("%s, Waiting to get Signal", __FUNCTION__);
            mSetCollective3rdPartyWait.wait(mSetCollective3rdParty);
        }
        ALOGD("Calling setCollective3rdParty");
        setCollective3rdParty();
    }

    ALOGD("END startPreviewInternal");

    return NO_ERROR;
}

status_t SemcCameraHardware::startPreview()
{
    ALOGD("START startPreview");

    Mutex::Autolock l(&mLock);
    return startPreviewInternal();
}

void SemcCameraHardware::stopPreviewInternal()
{
    ALOGD("START stopPreviewInternal mCameraRunning[%d]", mCameraRunning);

    if (mCameraRunning) {

        // Cancel auto focus. only 3rdparty
        if ( ((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0 )
        {
            if (CAMERAHAL_STATE_AFSTART == mStateCameraHal) {
                if (mNotifyCallback && (mMsgEnabled & CAMERA_MSG_FOCUS)) {
                    cancelAutoFocusInternal();
                }
            }
        }

    
        Mutex::Autolock l(&mCamframeTimeoutLock);
        {
            Mutex::Autolock cameraRunningLock(&mCameraRunningLock);
            if(!camframe_timeout_flag) {
                if (( mCurrentTarget != TARGET_MSM7630 ) &&
                        (mCurrentTarget != TARGET_QSD8250))
                    mCameraRunning = !native_stop_preview(mCameraControlFd);
                else
                    mCameraRunning = !native_stop_video(mCameraControlFd);

        mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTOP;
        mFrameCnt = 0;

            } else {
                /* This means that the camframetimeout was issued.
                 * But we did not issue native_stop_preview(), so we
                 * need to update mCameraRunning to indicate that
                 * Camera is no longer running. */
                mCameraRunning = 0;
            }
        }

	if (!mCameraRunning && mPreviewInitialized) {
	    deinitPreview();
	    if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250)) {
		mVideoThreadWaitLock.lock();
		ALOGV("in stopPreviewInternal: making mVideoThreadExit 1");
		mVideoThreadExit = 1;
		mVideoThreadWaitLock.unlock();
		//  720p : signal the video thread , and check in video thread if stop is called, if so exit video thread.
		pthread_mutex_lock(&(g_busy_frame_queue.mut));
		pthread_cond_signal(&(g_busy_frame_queue.wait));
		pthread_mutex_unlock(&(g_busy_frame_queue.mut));
                /* Flush the Busy Q */
                cam_frame_flush_video();
                /* Flush the Free Q */
                LINK_cam_frame_flush_free_video();
	    }
	    mPreviewInitialized = false;
	}
	else ALOGE("stopPreviewInternal: failed to stop preview");
    }

    mPreviewFlag = false;

    ALOGD("END stopPreviewInternal mCameraRunning[%d]", mCameraRunning);

}

void SemcCameraHardware::stopPreview()
{
    ALOGD("START stopPreview");

    Mutex::Autolock l(&mLock);
    {
        if (mDataCallbackTimestamp && (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME))
            return;
    }
    stopPreviewInternal();

    mAfterTakepictureFlag = false;

    ALOGD("END stopPreview");

}

void SemcCameraHardware::runAutoFocus()
{
    bool status = true;
    void *libhandle = NULL;
    isp3a_af_mode_t afMode;

    mAutoFocusThreadLock.lock();
    // Skip autofocus if focus mode is infinity.
    if ((mParameters.get(CameraParameters::KEY_FOCUS_MODE) == 0)
           || (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),
               CameraParameters::FOCUS_MODE_INFINITY) == 0)) {
        goto done;
    }

    mAutoFocusFd = open(MSM_CAMERA_CONTROL, O_RDWR);
    if (mAutoFocusFd < 0) {
        ALOGE("%s autofocus: cannot open %s: %s",
            __FUNCTION__,
             MSM_CAMERA_CONTROL,
             strerror(errno));
        mAutoFocusThreadRunning = false;
        mAutoFocusThreadLock.unlock();
        return;
    }

#if DLOPEN_LIBMMCAMERA
    // We need to maintain a reference to libqcamera.so for the duration of the
    // AF thread, because we do not know when it will exit relative to the
    // lifetime of this object.  We do not want to dlclose() libqcamera while
    // LINK_cam_frame is still running.
    libhandle = ::dlopen("liboemcamera.so", RTLD_NOW);
    ALOGV("AF: loading libqcamera at %p", libhandle);
    if (!libhandle) {
        ALOGE("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
        close(mAutoFocusFd);
        mAutoFocusFd = -1;
        mAutoFocusThreadRunning = false;
        mAutoFocusThreadLock.unlock();
        return;
    }
#endif

    afMode = (isp3a_af_mode_t)attr_lookup(focus_modes,
                                sizeof(focus_modes) / sizeof(str_map),
                                mParameters.get(CameraParameters::KEY_FOCUS_MODE));

    /* This will block until either AF completes or is cancelled. */
    ALOGV("af start (fd %d mode %d)", mAutoFocusFd, afMode);
    status_t err;
    err = mAfLock.tryLock();
    if(err == NO_ERROR) {
        {
            Mutex::Autolock cameraRunningLock(&mCameraRunningLock);
            if(mCameraRunning){
                ALOGV("Start AF");
                status = native_set_afmode(mAutoFocusFd, afMode);
            }else{
                ALOGV("As Camera preview is not running, AF not issued");
                status = false;
            }
        }
        mAfLock.unlock();
    }
    else{
        //AF Cancel would have acquired the lock,
        //so, no need to perform any AF
        ALOGV("%s As Cancel auto focus is in progress, auto focus request "
                "is ignored", __FUNCTION__);
        status = false;
    }

    ALOGV("af done: %d", (int)status);
    close(mAutoFocusFd);
    mAutoFocusFd = -1;

done:
    mAutoFocusThreadRunning = false;
    mAutoFocusThreadLock.unlock();

    mCallbackLock.lock();
    bool autoFocusEnabled = mNotifyCallback && (mMsgEnabled & CAMERA_MSG_FOCUS);
    notify_callback cb = mNotifyCallback;
    void *data = mCallbackCookie;
    mCallbackLock.unlock();
    if (autoFocusEnabled)
        cb(CAMERA_MSG_FOCUS, status, 0, data);

#if DLOPEN_LIBMMCAMERA
    if (libhandle) {
        ::dlclose(libhandle);
        ALOGV("AF: dlclose(libqcamera)");
    }
#endif
}

status_t SemcCameraHardware::cancelAutoFocusInternal()
{
    ALOGD("START cancelAutoFocusInternal");

    CameraHalState oldState = mStateCameraHal;
    mStateCameraHal = CAMERAHAL_STATE_AFCANCEL;

    {
        Mutex::Autolock cbLock(&mCallbackLock);
        if (native_stop_autofocus(mCameraControlFd) == false) {
            ALOGE("END cancelAutoFocusInternal : native_stop_autofocus is false");
            mStateCameraHal = oldState;
            return UNKNOWN_ERROR;
        }
        mCancelAutoFocusFlg = true;
    }

    Mutex::Autolock lock( &mCancelAutoFocus );
    while( mCancelAutoFocusFlg )
    {
        mCancelAutoFocusWait.wait( mCancelAutoFocus );
    }

    mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
    ALOGD("END cancelAutoFocusInternal");
    return NO_ERROR;

}

void *auto_focus_thread(void *user)
{
    ALOGV("auto_focus_thread E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->runAutoFocus();
    }
    else ALOGW("not starting autofocus: the object went away!");
    ALOGV("auto_focus_thread X");
    return NULL;
}
#if 0
status_t SemcCameraHardware::autoFocus()
{
    ALOGD("autoFocus E");
    Mutex::Autolock l(&mLock);

    if (mCameraControlFd < 0) {
        ALOGD("not starting autofocus: main control fd %d", mCameraControlFd);
        return UNKNOWN_ERROR;
    }

    {
        mAutoFocusThreadLock.lock();
        if (!mAutoFocusThreadRunning) {
            if (native_prepare_snapshot(mCameraControlFd, mParameters) == false) {
               ALOGD("native_prepare_snapshot failed!\n");
               setSocTorchMode(0);
               mAutoFocusThreadLock.unlock();
               return UNKNOWN_ERROR;
            }

            // Create a detached thread here so that we don't have to wait
            // for it when we cancel AF.
            pthread_t thr;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            mAutoFocusThreadRunning =
                !pthread_create(&thr, &attr,
                                auto_focus_thread, NULL);
            if (!mAutoFocusThreadRunning) {
                ALOGD("failed to start autofocus thread");
                mAutoFocusThreadLock.unlock();
                return UNKNOWN_ERROR;
            }
        }
        mAutoFocusThreadLock.unlock();
    }

    ALOGD("autoFocus X");
    return NO_ERROR;
}
#else
status_t SemcCameraHardware::autoFocus()
{
    ALOGD("START autoFocus : mStateCameraHal[%d]",mStateCameraHal);
    Mutex::Autolock l(&mLock);

    mCallbackLock.lock();
    bool autoFocusEnabled = mNotifyCallback && (mMsgEnabled & CAMERA_MSG_FOCUS);
    notify_callback cb = mNotifyCallback;
    void *data = mCallbackCookie;
    mCallbackLock.unlock();

    if (mCameraControlFd < 0) {
        ALOGE("END autoFocus : not starting autofocus: main control fd %d", mCameraControlFd);
        return UNKNOWN_ERROR;
    }

    if (true == mAutoFocusThreadRunning) {
        ALOGW("END autoFocus : AutoFocus is already in progress");
        return NO_ERROR;
    }

    if(CAMERAHAL_STATE_INIT == mStateCameraHal){
        ALOGW("END autoFocus : mStateCameraHal is INIT");
        return NO_ERROR;
    }else if(CAMERAHAL_STATE_PREVIEWSTART != mStateCameraHal){
        ALOGW("END autoFocus : Status Error mStateCameraHal[%d]",mStateCameraHal);
        return UNKNOWN_ERROR;
    }

    if (strcmp(mParameters.get(CameraParameters::KEY_FOCUS_MODE),
               CameraParameters::FOCUS_MODE_INFINITY) == 0) {
        if (autoFocusEnabled) {
            ALOGD("%s Calling CB 1", __FUNCTION__);
            cb(CAMERA_MSG_FOCUS, true, 0, data);
        }
        ALOGD("END autoFocus : KEY_FOCUS_MODE is FOCUS_MODE_INFINITY");
        return NO_ERROR;
    }


    //TODO test setting afmode in here.
    isp3a_af_mode_t afMode;

    afMode = (isp3a_af_mode_t)attr_lookup(focus_modes,
                                sizeof(focus_modes) / sizeof(str_map),
                                mParameters.get(CameraParameters::KEY_FOCUS_MODE));
    //if(native_set_parm(CAMERA_SET_PARM_AF_FRAME, sizeof(afMode), (void *)&afMode, DRV_TIMEOUT_10K))
    //{
    //    ALOGD("%s AFmode set to %s", __FUNCTION__, mParameters.get(CameraParameters::KEY_FOCUS_MODE));
    //}else{
    //    ALOGD("%s AFmode not set correctly", __FUNCTION__);
    //}
#if SCRITCH_OFF
    if(setAfMode(mParameters, CameraParameters::AF_MODE, CameraParameters::SINGLE) != NO_ERROR) {
        if (autoFocusEnabled) {
            cb(CAMERA_MSG_FOCUS, false, 0, data);
        }
        ALOGD("END autoFocus :setAfMode is error.");
        return UNKNOWN_ERROR;
    }
#endif//SCRITCH_OFF

    CameraHalState oldState = mStateCameraHal;
    mStateCameraHal = CAMERAHAL_STATE_AFSTART;
    mFocusLockValue = true;

    const char *str = mParameters.get(CameraParameters::KEY_FLASH_MODE);


    ALOGD("%s, Flash Value %s", __FUNCTION__, str);

    if (str != NULL) {
       int32_t value = attr_lookup(flash, sizeof(flash) / sizeof(str_map), str);
       if (value != NOT_FOUND) {
            bool shouldBeOn = strcmp(str, "on") == 0;
            setSocTorchMode(shouldBeOn);
       }
    }

    if(native_start_autofocus(mCameraControlFd, false, false, mFocusLockValue) == false){
        if (autoFocusEnabled) {
            ALOGD("%s Calling CB 2", __FUNCTION__);
            cb(CAMERA_MSG_FOCUS, false, 0, data);
        }
        ALOGE("END autoFocus :native_start_autofocus is error.");
        mStateCameraHal = oldState;
        return UNKNOWN_ERROR;
    }
    mAutoFocusThreadRunning = true;

    ALOGD("END autoFocus : mStateCameraHal[%d]",mStateCameraHal);
    return NO_ERROR;

}
#endif

status_t SemcCameraHardware::cancelAutoFocus()
{
    ALOGD("START cancelAutoFocus");

    Mutex::Autolock l(&mLock);

    int rc = NO_ERROR;
    if (mCameraRunning && mNotifyCallback && (mMsgEnabled & CAMERA_MSG_FOCUS)) {
        rc = cancelAutoFocusInternal();
    }

    setSocTorchMode(0); 
    ALOGD("END cancelAutoFocus : return[%d]",rc);
    return rc;
}

void SemcCameraHardware::runSnapshotThread(void *data)
{
    ALOGD("START runSnapshotThread");
    mTakepictureFlag = true;

    if (!native_start_snapshot(mCameraControlFd)) {
        ALOGE("runSnapshotThread: native_start_snapshot failed!");
        takePictureErrorCallback();
        return;
    }

    if (!receiveRawPicture()) {
        ALOGE("runSnapshotThread: receiveRawPicture failed!");
        takePictureErrorCallback();
        return;
    }

    if (mTakepictureThreadJoin == true) {
        ALOGD("mTakepictureThreadJoin:true before startrawsnapshot() call");
        mSnapshotThreadWaitLock.lock();
        mSnapshotThreadRunning = false;
        mSnapshotThreadWait.signal();
        mSnapshotThreadWaitLock.unlock();
        mTakepictureThreadJoin = false;
        mTakepictureFlag = false;
        return;
    }

    retCbErrType ret = startrawsnapshot();
    mTakepictureFlag = false;
    if(RET_FAIL_YUV_CB == ret) {
        ALOGE("runSnapshotThread : startrawsnapshot is error(YUV callback failed)");
        takePictureErrorCallback();
        return;
    } else if(RET_FAIL_JPEG_CB == ret) {
        ALOGE("runSnapshotThread : startrawsnapshot is error(JPEG callback failed)");
        takePictureErrorCallback();
        return;
    }

    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
        if (mTakepictureThreadJoin == true) {
            ALOGD("mTakepictureThreadJoin:true before LINK_jpeg_encoder_init() call");
            mSnapshotThreadWaitLock.lock();
            mSnapshotThreadRunning = false;
            mSnapshotThreadWait.signal();
            mSnapshotThreadWaitLock.unlock();
            mTakepictureThreadJoin = false;
            return;
        }
        mInSnapshotModeWaitLock.lock();
        mInSnapshotModeWait.signal();
        mInSnapshotModeWaitLock.unlock();

        mReceiveExifDataFlag = true;
        if (LINK_jpeg_encoder_init()) {
            if (native_jpeg_encode()) {
                ALOGV("runSnapshotThread : LOCK ACQUIRED ");
                mSnapshotThreadWaitLock.lock();
                mSnapshotThreadRunning = false;
                mSnapshotThreadWait.signal();
                mSnapshotThreadWaitLock.unlock();
                ALOGV("runSnapshotThread: SIGNALLED QCS_IDLE ");
                ALOGD("END runSnapshotThread : native_jpeg_encode(success)");
                mTakepictureThreadJoin = false;
                return;
            } else {
                ALOGE("runSnapshotThread : native_jpeg_encode is error");
                takePictureErrorCallback();
               if(mCameraExitFlag){
                   mReleaseWaitLock.lock();
                   mReleaseWait.signal();
                   mReleaseWaitLock.unlock();
                }
                mReceiveExifDataFlag = false;
                return;
            }
        } else {
            ALOGE("runSnapshotThread : LINK_jpeg_encoder_init is error");
            takePictureErrorCallback();
            if(mCameraExitFlag){
                mReleaseWaitLock.lock();
                mReleaseWait.signal();
                mReleaseWaitLock.unlock();
            }
            mReceiveExifDataFlag = false;
            return;
        }
    } else if (mDataCallback && (mMsgEnabled & (CAMERA_MSG_RAW_IMAGE | CAMERA_MSG_POSTVIEW_FRAME))) {
        if (((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
            ssize_t offset;
            ssize_t snapshot_offset = 0;
            if((CAMERA_FWVGASIZE_DRV_WIDTH == mDimension.postview_width)
                && (CAMERA_FWVGASIZE_DRV_HEIGHT == mDimension.postview_height)) {
                int preview_width, preview_height;

                mParameters.getPreviewSize(&preview_width, &preview_height);

                if(CAMERA_FWVGASIZE_WIDTH == preview_width
                    && CAMERA_FWVGASIZE_HEIGHT == preview_height) {
                    offset = (CAMERA_FWVGASIZE_WIDTH * CAMERA_FWVGASIZE_HEIGHT *
                              1.5 * snapshot_offset);
                    convertImage864to854((uint8_t *)mRawHeapSub->mHeap->base(),
                                         (uint8_t *)mRawHeap->mHeap->base() );
                }else if (CAMERA_OHDFWVGASIZE_WIDTH == preview_width
                    && CAMERA_OHDFWVGASIZE_HEIGHT == preview_height) {
                    offset = (CAMERA_OHDFWVGASIZE_WIDTH * CAMERA_OHDFWVGASIZE_HEIGHT *
                              1.5 * snapshot_offset);
                    convertImage864to848((uint8_t *)mRawHeapSub->mHeap->base(),
                                         (uint8_t *)mRawHeap->mHeap->base() );
                } else {
                    offset = (mDimension.postview_width * mDimension.postview_height *
                              1.5 * snapshot_offset);
                    memcpy((uint8_t *)mRawHeapSub->mHeap->base() ,
                           (uint8_t *)mRawHeap->mHeap->base(), 0 + mRawSize);
                }
            } else {
                offset = (CAMERA_VGASIZE_WIDTH * CAMERA_VGASIZE_HEIGHT  * 1.5 * snapshot_offset);
                memcpy((uint8_t *)mRawHeapSub->mHeap->base() , (uint8_t *)mRawHeap->mHeap->base(), 0 + mRawSize);
            }
            if (mRawHeapSub != NULL && mRawHeap != NULL){
                if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE){
                    mDataCallback(CAMERA_MSG_RAW_IMAGE, mRawHeapSub->mBuffers[offset],  mCallbackCookie);
                }
                if(mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME){
                    mDataCallback(CAMERA_MSG_POSTVIEW_FRAME, mRawHeapSub->mBuffers[offset],
                                  mCallbackCookie);
                }
            }
        }
    } else {
        ALOGD("JPEG callback and RAW callback is NULL.");
        mSnapshotThreadWaitLock.lock();
        mSnapshotThreadRunning = false;
        mSnapshotThreadWait.signal();
        mSnapshotThreadWaitLock.unlock();
        mTakepictureThreadJoin = false;
        return;
    }
    deinitRaw();
    mTakepictureThreadJoin = false;

    ALOGD("END runSnapshotThread");

}

void *snapshot_thread(void *user)
{
    ALOGD("START snapshot_thread");

    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->runSnapshotThread(user);
    }
    else ALOGW("not starting snapshot thread: the object went away!");

    ALOGD("END snapshot_thread");

    return NULL;
}

status_t SemcCameraHardware::takePicture()
{
    ALOGD("START takePicture : mMsgEnabled[%d]",mMsgEnabled);

    Mutex::Autolock l(&mLock);

    if(CAMERAHAL_STATE_INIT == mStateCameraHal){
        ALOGW("END takePicture : mStateCameraHal is INIT");
        return NO_ERROR;
    }else if((CAMERAHAL_STATE_PREVIEWSTART != mStateCameraHal) &&
            (CAMERAHAL_STATE_PREVIEWSTOP != mStateCameraHal) &&
            (CAMERAHAL_STATE_TAKEPICDONE != mStateCameraHal)){
        if((((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0)){
            if(CAMERAHAL_STATE_AFLOCK != mStateCameraHal){
                ALOGE("END takePicture : Status error(Signature)");
                return UNKNOWN_ERROR;
            }
        }else{
            if(CAMERAHAL_STATE_AFSTART != mStateCameraHal){
                ALOGE("END takePicture : Status error(3rdParty)");
                return UNKNOWN_ERROR;
            }
        }
    }
    if(CAMERAHAL_STATE_TAKEPICDONE == mStateCameraHal){
    //Reset the Gps Information
        exif_table_numEntries = 0;
    }
    // Wait for old snapshot thread to complete.
    mSnapshotThreadWaitLock.lock();
    while (mSnapshotThreadRunning) {
        ALOGV("takePicture: waiting for old snapshot thread to complete.");
        mSnapshotThreadWait.wait(mSnapshotThreadWaitLock);
        ALOGV("takePicture: old snapshot thread completed.");
    }

    if( mCurrentTarget == TARGET_MSM7630 ) {
	/* Store the last frame queued for preview. This
	 * shall be used as postview */
	storePreviewFrameForPostview();
    }

    if(native_set_ae_awb_lock(mCameraControlFd, true, true) == false){
        ALOGE("END takePicture : native_set_ae_awb_lock failed!");
        mSnapshotThreadWaitLock.unlock();
        return UNKNOWN_ERROR;
    }

    //mSnapshotFormat is protected by mSnapshotThreadWaitLock
    if(mParameters.getPictureFormat() != 0 &&
            !strcmp(mParameters.getPictureFormat(),
                    CameraParameters::PIXEL_FORMAT_RAW)){
        ALOGE("END takePicture : PictureFormat failed!");
        return UNKNOWN_ERROR;
    }

    if (!native_prepare_snapshot(mCameraControlFd, mParameters)) {
        setSocTorchMode(0);
        mSnapshotThreadWaitLock.unlock();
        return UNKNOWN_ERROR;
    }

    stopPreviewInternal();
    mStateCameraHal = CAMERAHAL_STATE_TAKEPICSTART;

    if (!initRaw(mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE))) {
        ALOGE("initRaw failed.  Not taking picture.");
        mSnapshotThreadWaitLock.unlock();
        return UNKNOWN_ERROR;
    }

    time_t t;
    struct tm *ltm;
    char getTime[20];
    memset(createDateTime, 0x00, sizeof(createDateTime));
    memset(getTime, 0x00, sizeof(getTime));

    time(&t);
    ltm = localtime(&t);
    sprintf(getTime,"%04d:%02d:%02d %02d:%02d:%02d",ltm->tm_year+1900,ltm->tm_mon+1,
            ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec);

    if (strlen(getTime) >= 1) {
        strncpy(createDateTime, getTime, 19);
        createDateTime[19] = '\0';
        addExifTag(EXIFTAGID_FILE_CHANGE_DATE_TIME, EXIF_ASCII,
                    20, 1, (void *)createDateTime);
    }

    mShutterLock.lock();
    mShutterPending = true;
    mShutterLock.unlock();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    mSnapshotThreadRunning = !pthread_create(&mSnapshotThread,
                                             &attr,
                                             snapshot_thread,
                                             NULL);
    mSnapshotThreadWaitLock.unlock();

    ALOGD("END takePicture ");

    return mSnapshotThreadRunning ? NO_ERROR : UNKNOWN_ERROR;
}

status_t SemcCameraHardware::cancelPicture()
{
    status_t rc;
    ALOGD("START cancelPicture");
    mTakepictureThreadJoin = true;
    while (mSnapshotThreadRunning) {
        ALOGD("cancelPicture: waiting for old snapshot thread to complete.");
        mSnapshotThreadWait.wait(mSnapshotThreadWaitLock);
        ALOGD(": old snapshot thread completed.");
    }
    if (mTakepictureFlag == true) {
        rc = native_stop_snapshot(mCameraControlFd) ? NO_ERROR : UNKNOWN_ERROR;
        mTakepictureFlag = false;
    } else {
        ALOGD("END cancelPicture: mTakepictureFlag : false");
        return NO_ERROR;
    }
    ALOGV("END cancelPicture : %d", rc);
    return rc;
}

void SemcCameraHardware::takePictureErrorCallback()
{
    ALOGD("START takePictureErrorCallback");

    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_RAW_IMAGE)) {
        mDataCallback(CAMERA_MSG_RAW_IMAGE, NULL,  mCallbackCookie);
    }
    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
        mDataCallback(CAMERA_MSG_COMPRESSED_IMAGE, NULL,  mCallbackCookie);
    }
    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME)) {
        mDataCallback(CAMERA_MSG_POSTVIEW_FRAME, NULL,  mCallbackCookie);
    }
    deinitRaw();
    mTakepictureThreadJoin = false;
    mTakepictureFlag = false;

    mSnapshotThreadWaitLock.lock();
    mSnapshotThreadRunning = false;
    mSnapshotThreadWait.signal();
    mSnapshotThreadWaitLock.unlock();

    mStateCameraHal = CAMERAHAL_STATE_TAKEPICDONE;
    ALOGD("END takePictureErrorCallback");
}

status_t SemcCameraHardware::setParameters(const CameraParameters& params)
{
    ALOGD("START setParameters: E params = %p", &params);

    Mutex::Autolock l(&mLock);
    status_t rc, final_rc = NO_ERROR;

    setZoom(params);
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        if (rc = setPreviewSize(params)){
            final_rc = rc;
        }
        ALOGD("final_rc = %d", final_rc);
        if (rc = setPictureSize(params)){
            final_rc = rc;
        }
        ALOGD("final_rc = %d", final_rc);
        if (native_step_zoom(mCameraControlFd, mZoom) == false){
            final_rc = UNKNOWN_ERROR;
        }
        ALOGD("final_rc = %d", final_rc);
#if SCRITCH_OFF
        if (rc = setAfMode(params)){
            final_rc = rc;
        }
        if (rc = setCAFMode(params)){
            final_rc = rc;
        }

#endif
        if (rc = setFocusMode(params)){
            final_rc = rc;
        }
        ALOGD("final_rc = %d", final_rc);
        return final_rc;
    }
    //BEN QRCODE
    if (((rc = setSceneMode(params)))){
        final_rc = rc;
    }

    //Save the Video Size here
    {
        int width, height;
        params.getVideoSize(&width,&height);
        mParameters.setVideoSize(width, height);
    }

    if ((rc = setPreviewSize(params)))  final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setPictureSize(params)))  final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setJpegQuality(params)))  final_rc = rc;
        ALOGD("final_rc = %d", final_rc);

    if ((rc = setAutoExposure(params))) final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setWhiteBalance(params))) final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setGpsLocation(params)))  final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setRotation(params)))     final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setFocusMode(params)))    final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setBrightness(params)))   final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setPictureFormat(params))) final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
//Use J setFlash Function
    if ((rc = setFlash(params)))        final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setExposureCompensation(params))) final_rc = rc;
#if SCRITCH_OFF
    if ((rc = setFlashlightBrightness(params))) final_rc = rc;
#endif//SCRITCH_OFF
    if ((rc = setJpegThumbnailSize(params))) final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    if ((rc = setPreviewFpsRange(params))) final_rc = rc;
        ALOGD("final_rc = %d", final_rc);
    /* case 3rdparty */
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0)
    {
        if ((rc = setAntibanding(params))){
            final_rc = rc;
        }
        ALOGD("final_rc = %d", final_rc);
        if ((rc = setEffect(params))){
            final_rc = rc;
        }
        if (((rc = setFramerate(params)))){
            final_rc = rc;
        }
        ALOGD("final_rc = %d", final_rc);
        setOrientation(params);
        ALOGD("final_rc = %d", final_rc);
    }
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0)
    {

        // Only signature APL support.
        if (((rc = setExposureCompensation(params)))){
            final_rc = rc;
        }
#if SCRITCH_OFF
        if (((rc = setAfMode(params)))){
            final_rc = rc;
        }
        if (((rc = setCAFMode(params)))){
            final_rc = rc;
        }
        if (((rc = setHandJitterReduction(params)))){
            final_rc = rc;
        }
        if (((rc = setSmileMode(params)))){
            final_rc = rc;
        }
#endif//SCRITCH_OFF
    }

    ALOGD("END setParameters: final_rc[%d]",final_rc);

    return final_rc;
}

CameraParameters SemcCameraHardware::getParameters() const
{

    return mParameters;
}

status_t SemcCameraHardware::sendCommand(int32_t command, int32_t arg1,
                                             int32_t arg2)
{
    ALOGV("sendCommand: EX");
    return BAD_VALUE;
}

static CameraInfo sCameraInfo[] = {
    {
        CAMERA_FACING_BACK,
        90,  /* orientation */
    },
};

extern "C" int HAL_getNumberOfCameras()
{
    return sizeof(sCameraInfo) / sizeof(sCameraInfo[0]);
}

extern "C" void HAL_getCameraInfo(int cameraId, struct CameraInfo* cameraInfo)
{
    memcpy(cameraInfo, &sCameraInfo[cameraId], sizeof(CameraInfo));
}

extern "C" sp<CameraHardwareInterface> HAL_openCameraHardware(int cameraId)
{
    ALOGV("openCameraHardware: call createInstance");
    if(cameraId > sizeof(sCameraInfo) / sizeof(sCameraInfo[0]))
	return NULL;
    return SemcCameraHardware::createInstance();
}


wp<SemcCameraHardware> SemcCameraHardware::singleton;

// If the hardware already exists, return a strong pointer to the current
// object. If not, create a new hardware object, put it in the singleton,
// and return it.
sp<CameraHardwareInterface> SemcCameraHardware::createInstance()

{
    ALOGD("createInstance: E");
    Mutex::Autolock lock(&singleton_lock);

    if (singleton != 0) {
        sp<CameraHardwareInterface> hardware = singleton.promote();
        if (hardware != 0) {
            ALOGD("createInstance: X return existing hardware=%p", &(*hardware));
            sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
            if (obj != 0)
            {
                obj->semcCameraExtensionJudgment(true);
            }else{
                ALOGD("END createInstance: not_obj ");
            }
            return hardware;
        }
    }

    {
        struct stat st;
        int rc = stat("/dev/oncrpc", &st);
        if (rc < 0) {
            ALOGD("createInstance: X failed to create hardware: %s", strerror(errno));
            return NULL;
        }
    }

    SemcCameraHardware *cam = new SemcCameraHardware();
    sp<SemcCameraHardware> hardware(cam);
    singleton = hardware;

    if (!cam->startCamera()) {
        ALOGE("%s: startCamera failed!", __FUNCTION__);
        singleton.clear();
        return NULL;
    }
    cam->semcCameraExtensionJudgment(true);
    cam->initDefaultParameters();
    ALOGD("createInstance: X created hardware=%p", &(*hardware));
    return hardware;
}

// For internal use only, hence the strong pointer to the derived type.
sp<SemcCameraHardware> SemcCameraHardware::getInstance()
{
    sp<CameraHardwareInterface> hardware = singleton.promote();
    if (hardware != 0) {
        //    ALOGV("getInstance: X old instance of hardware");
        return sp<SemcCameraHardware>(static_cast<SemcCameraHardware*>(hardware.get()));
    } else {
        ALOGV("getInstance: X new instance of hardware");
        return sp<SemcCameraHardware>();
    }
}
void SemcCameraHardware::receiveRecordingFrame(struct msm_frame *frame)
{
    ALOGV("receiveRecordingFrame E");
    // post busy frame
    if (frame)
    {
        cam_frame_post_video (frame);
    }
    else ALOGE("in  receiveRecordingFrame frame is NULL");
    ALOGV("receiveRecordingFrame X");
}


bool SemcCameraHardware::native_zoom_image(int fd, int srcOffset, int dstOffSet, common_crop_t *crop)
{
    int result = 0;
    struct mdp_blit_req *e;
    struct timeval td1, td2;

    /* Initialize yuv structure */
    zoomImage.list.count = 1;

    e = &zoomImage.list.req[0];

    e->src.width = previewWidth;
    e->src.height = previewHeight;
    e->src.format = MDP_Y_CBCR_H2V2;
    e->src.offset = srcOffset;
    e->src.memory_id = fd;

    e->dst.width = previewWidth;
    e->dst.height = previewHeight;
    e->dst.format = MDP_Y_CBCR_H2V2;
    e->dst.offset = dstOffSet;
    e->dst.memory_id = fd;

    e->transp_mask = 0xffffffff;
    e->flags = 0;
    e->alpha = 0xff;
    if (crop->in2_w != 0 || crop->in2_h != 0) {
        e->src_rect.x = (crop->out2_w - crop->in2_w + 1) / 2 - 1;
        e->src_rect.y = (crop->out2_h - crop->in2_h + 1) / 2 - 1;
        e->src_rect.w = crop->in2_w;
        e->src_rect.h = crop->in2_h;
    } else {
        e->src_rect.x = 0;
        e->src_rect.y = 0;
        e->src_rect.w = previewWidth;
        e->src_rect.h = previewHeight;
    }
    //ALOGV(" native_zoom : SRC_RECT : x,y = %d,%d \t w,h = %d, %d",
    //        e->src_rect.x, e->src_rect.y, e->src_rect.w, e->src_rect.h);

    e->dst_rect.x = 0;
    e->dst_rect.y = 0;
    e->dst_rect.w = previewWidth;
    e->dst_rect.h = previewHeight;

    result = ioctl(fb_fd, MSMFB_BLIT, &zoomImage.list);
    if (result < 0) {
        ALOGE("MSM_FBIOBLT failed! line=%d\n", __LINE__);
        return false;
    }
    return true;
}

void SemcCameraHardware::debugShowPreviewFPS() const
{
    static int mFrameCount;
    static int mLastFrameCount = 0;
    static nsecs_t mLastFpsTime = 0;
    static float mFps = 0;
    mFrameCount++;
    nsecs_t now = systemTime();
    nsecs_t diff = now - mLastFpsTime;
    if (diff > ms2ns(250)) {
        mFps =  ((mFrameCount - mLastFrameCount) * float(s2ns(1))) / diff;
        ALOGI("Preview Frames Per Second: %.4f", mFps);
        mLastFpsTime = now;
        mLastFrameCount = mFrameCount;
    }
}

void SemcCameraHardware::debugShowVideoFPS() const
{
    static int mFrameCount;
    static int mLastFrameCount = 0;
    static nsecs_t mLastFpsTime = 0;
    static float mFps = 0;
    mFrameCount++;
    nsecs_t now = systemTime();
    nsecs_t diff = now - mLastFpsTime;
    if (diff > ms2ns(250)) {
        mFps =  ((mFrameCount - mLastFrameCount) * float(s2ns(1))) / diff;
        ALOGI("Video Frames Per Second: %.4f", mFps);
        mLastFpsTime = now;
        mLastFrameCount = mFrameCount;
    }
}

static ssize_t previewframe_offset = 0;

void SemcCameraHardware::receivePreviewFrame(struct msm_frame *frame)
{
    ALOGV("receivePreviewFrame E");

    if (!mCameraRunning) {
        ALOGE("ignoring preview callback--camera has been stopped");
        return;
    }

    if (UNLIKELY(mDebugFps)) {
        debugShowPreviewFPS();
    }

    mCallbackLock.lock();
    int msgEnabled = mMsgEnabled;
    data_callback pcb = mDataCallback;
    void *pdata = mCallbackCookie;
    data_callback_timestamp rcb = mDataCallbackTimestamp;
    void *rdata = mCallbackCookie;

    void *notify_data = mCallbackCookie;

    mCallbackLock.unlock();

    // Find the offset within the heap of the current buffer.
    ssize_t offset_addr =
        (ssize_t)frame->buffer - (ssize_t)mPreviewHeap->mHeap->base();
    ssize_t offset = offset_addr / mPreviewHeap->mAlignedBufferSize;

    previewframe_offset = offset;

    common_crop_t *crop = (common_crop_t *) (frame->cropinfo);

    mInPreviewCallback = true;
#if SCRITCH_OFF
    if(mUseOverlay) {
	if(mOverlay != NULL) {

	    mOverlayLock.lock();
	    if (crop->in2_w != 0 || crop->in2_h != 0) {
		zoomCropInfo.x = (crop->out2_w - crop->in2_w + 1) / 2 - 1;
		zoomCropInfo.y = (crop->out2_h - crop->in2_h + 1) / 2 - 1;
		zoomCropInfo.w = crop->in2_w;
		zoomCropInfo.h = crop->in2_h;
		mOverlay->setCrop(zoomCropInfo.x, zoomCropInfo.y,
			zoomCropInfo.w, zoomCropInfo.h);
	    } else {
                // Reset zoomCropInfo variables. This will ensure that
                // stale values wont be used for postview
                zoomCropInfo.w = crop->in2_w;
                zoomCropInfo.h = crop->in2_h;
            }
	    mOverlay->queueBuffer((void *)offset_addr);
            mLastQueuedFrame = (void *)frame->buffer;
	    mOverlayLock.unlock();
	}
    } else {
#endif//SCRITCH_OFF
	if (crop->in2_w != 0 || crop->in2_h != 0) {
	    dstOffset = (dstOffset + 1) % NUM_MORE_BUFS;
	    offset = kPreviewBufferCount + dstOffset;
	    ssize_t dstOffset_addr = offset * mPreviewHeap->mAlignedBufferSize;
	    if( !native_zoom_image(mPreviewHeap->mHeap->getHeapID(),
			offset_addr, dstOffset_addr, crop)) {
		ALOGE(" Error while doing MDP zoom ");
                offset = offset_addr / mPreviewHeap->mAlignedBufferSize;
	    }
	}
#if SCRITCH_OFF
    }
#endif //SCRITCH_OFF


    if(mFrameCnt < CAMERA_CONVERGENCE_FRAME) {
        mFrameCnt++;
        if(1 == mFrameCnt) {
            mPreviewFlag = true;
            Mutex::Autolock lock(&mSetCollective3rdParty);
            mSetCollective3rdPartyWait.signal();
        }
    }

    if (pcb != NULL && (msgEnabled & CAMERA_MSG_PREVIEW_FRAME))
        pcb(CAMERA_MSG_PREVIEW_FRAME, mPreviewHeap->mBuffers[offset],
            pdata);

    // If output2 enabled, Start Recording if recording is enabled by Services
    if( ((mCurrentTarget == TARGET_MSM7630) || (mCurrentTarget == TARGET_QSD8250))  && recordingEnabled() ) {
        if(!recordingState){
            recordingState = 1; // recording started
            ALOGV(" in receivePreviewframe : recording enabled calling startRecording ");
            startRecording();
        }
    }

    // If output  is NOT enabled (targets otherthan 7x30 currently..)
    if( (mCurrentTarget != TARGET_MSM7630 ) &&  (mCurrentTarget != TARGET_QSD8250)) {
        if(rcb != NULL && (msgEnabled & CAMERA_MSG_VIDEO_FRAME)) {
            rcb(systemTime(), CAMERA_MSG_VIDEO_FRAME, mPreviewHeap->mBuffers[offset], rdata);
            Mutex::Autolock rLock(&mRecordFrameLock);
            if (mReleasedRecordingFrame != true) {
                ALOGV("block waiting for frame release");
                mRecordWait.wait(mRecordFrameLock);
                ALOGV("frame released, continuing");
            }
            mReleasedRecordingFrame = false;
        }
    }
    mInPreviewCallback = false;

//    ALOGV("receivePreviewFrame X");
}


bool SemcCameraHardware::initRecord()
{
    ALOGV("initREcord E");

    mRecordFrameSize = (mDimension.video_width  * mDimension.video_height *3)/2;
    mRecordHeap = new PmemPool("/dev/pmem_adsp",
                               MemoryHeapBase::READ_ONLY | MemoryHeapBase::NO_CACHING,
                                mCameraControlFd,
                                MSM_PMEM_VIDEO,
                                mRecordFrameSize,
                                kRecordBufferCount,
                                mRecordFrameSize,
                                "record");
    if (!mRecordHeap->initialized()) {
        mRecordHeap.clear();
        ALOGE("initRecord X: could not initialize record heap.");
        return false;
    }
    for (int cnt = 0; cnt < kRecordBufferCount; cnt++) {
        recordframes[cnt].fd = mRecordHeap->mHeap->getHeapID();
        recordframes[cnt].buffer =
            (uint32_t)mRecordHeap->mHeap->base() + mRecordHeap->mAlignedBufferSize * cnt;
        recordframes[cnt].y_off = 0;
        recordframes[cnt].cbcr_off = mDimension.video_width  * mDimension.video_height;
        recordframes[cnt].path = OUTPUT_TYPE_V;

        ALOGV ("initRecord :  record heap , video buffers  buffer=%lu fd=%d y_off=%d cbcr_off=%d \n",
          (unsigned long)recordframes[cnt].buffer, recordframes[cnt].fd, recordframes[cnt].y_off,
          recordframes[cnt].cbcr_off);
    }

    // initial setup : buffers 1,2,3 with kernel , 4 with camframe , 5,6,7,8 in free Q
    // flush the busy Q
    cam_frame_flush_video();

    mVideoThreadWaitLock.lock();
    while (mVideoThreadRunning) {
        ALOGV("initRecord: waiting for old video thread to complete.");
        mVideoThreadWait.wait(mVideoThreadWaitLock);
        ALOGV("initRecord : old video thread completed.");
    }
    mVideoThreadWaitLock.unlock();

    // flush free queue and add 5,6,7,8 buffers.
    LINK_cam_frame_flush_free_video();
    for(int i=ACTIVE_VIDEO_BUFFERS+1;i <kRecordBufferCount; i++)
        LINK_camframe_free_video(&recordframes[i]);
    ALOGV("initREcord X");

    return true;
}

status_t SemcCameraHardware::startRecording()
{
    ALOGV("startRecording E");
    int ret;
    Mutex::Autolock l(&mLock);
    mReleasedRecordingFrame = false;
    if( (ret=startPreviewInternal())== NO_ERROR){
        if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250))  {
            ALOGV(" in startREcording : calling native_start_recording");
            if(!native_start_recording(mCameraControlFd)){
                ALOGE(" in startREcording : native_start_recording error");
                return UNKNOWN_ERROR;
            }
            recordingState = 1;
            // Remove the left out frames in busy Q and them in free Q.
            // this should be done before starting video_thread so that,
            // frames in previous recording are flushed out.
            ALOGV("frames in busy Q = %d", g_busy_frame_queue.num_of_frames);
            while((g_busy_frame_queue.num_of_frames) >0){
                msm_frame* vframe = cam_frame_get_video ();
                LINK_camframe_free_video(vframe);
            }
            ALOGV("frames in busy Q = %d after deQueing", g_busy_frame_queue.num_of_frames);
            // Start video thread and wait for busy frames to be encoded, this thread
            // should be closed in stopRecording
            mVideoThreadWaitLock.lock();
            mVideoThreadExit = 0;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            mVideoThreadRunning = pthread_create(&mVideoThread,
                                              &attr,
                                              video_thread,
                                              NULL);
            mVideoThreadWaitLock.unlock();
            // Remove the left out frames in busy Q and them in free Q.
        }
    }
    return ret;
}

void SemcCameraHardware::stopRecording()
{
    ALOGD("START stopRecording");

    Mutex::Autolock l(&mLock);
    {
        mRecordFrameLock.lock();
        mReleasedRecordingFrame = true;
        mRecordWait.signal();
        mRecordFrameLock.unlock();

        if(mDataCallback && !(mCurrentTarget == TARGET_QSD8250) &&
                         (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME)) {
            ALOGV("stopRecording: X, preview still in progress");
            return;
        }
    }
    // If output2 enabled, exit video thread, invoke stop recording ioctl
    if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250))  {
        mVideoThreadWaitLock.lock();
        mVideoThreadExit = 1;
        mVideoThreadWaitLock.unlock();
        native_stop_recording(mCameraControlFd);

        pthread_mutex_lock(&(g_busy_frame_queue.mut));
        pthread_cond_signal(&(g_busy_frame_queue.wait));
        pthread_mutex_unlock(&(g_busy_frame_queue.mut));
    }
    else  // for other targets where output2 is not enabled
        stopPreviewInternal();

    recordingState = 0; // recording not started

    ALOGD("END stopRecording");

}

void SemcCameraHardware::releaseRecordingFrame(
       const sp<IMemory>& mem __attribute__((unused)))
{
    ALOGD("START releaseRecordingFrame");

    Mutex::Autolock rLock(&mRecordFrameLock);
    mReleasedRecordingFrame = true;
    mRecordWait.signal();

    // Ff 7x30 : add the frame to the free camframe queue
    if( (mCurrentTarget == TARGET_MSM7630 )  || (mCurrentTarget == TARGET_QSD8250)) {
        ssize_t offset;
        size_t size;
        sp<IMemoryHeap> heap = mem->getMemory(&offset, &size);
        msm_frame* releaseframe = NULL;
        ALOGV(" in release recording frame :  heap base %d offset %d buffer %d ", heap->base(), offset, heap->base() + offset );
        int cnt;
        for (cnt = 0; cnt < kRecordBufferCount; cnt++) {
            if((unsigned int)recordframes[cnt].buffer == (unsigned int)(heap->base()+ offset)){
                ALOGV("in release recording frame found match , releasing buffer %d", (unsigned int)recordframes[cnt].buffer);
                releaseframe = &recordframes[cnt];
                break;
            }
        }
        if(cnt < kRecordBufferCount) {
            // do this only if frame thread is running
            mFrameThreadWaitLock.lock();
            if(mFrameThreadRunning )
                LINK_camframe_free_video(releaseframe);

            mFrameThreadWaitLock.unlock();
        } else {
            ALOGE("in release recordingframe XXXXX error , buffer not found");
            for (int i=0; i< kRecordBufferCount; i++) {
                 ALOGE(" recordframes[%d].buffer = %d", i, (unsigned int)recordframes[i].buffer);
            }
        }
    }

    ALOGD("END releaseRecordingFrame");
}

bool SemcCameraHardware::recordingEnabled()
{
    return mCameraRunning && mDataCallbackTimestamp && (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME);
}

void SemcCameraHardware::notifyShutter(common_crop_t *crop, bool mPlayShutterSoundOnly)
{
    mShutterLock.lock();
    image_rect_type size;

    size.width = (uint32_t)mDimension.postview_width;
    size.height = (uint32_t)mDimension.postview_height;

    if(mPlayShutterSoundOnly) {
        /* At this point, invoke Notify Callback to play shutter sound only.
         * We want to call notify callback again when we have the
         * yuv picture ready. This is to reduce blanking at the time
         * of displaying postview frame. Using ext2 to indicate whether
         * to play shutter sound only or register the postview buffers.
         */
        mDisplayHeap = mRawHeap;
        mNotifyCallback(CAMERA_MSG_SHUTTER, (int32_t)&size, mPlayShutterSoundOnly,
                            mCallbackCookie);
        mShutterLock.unlock();
        return;
    }

    if (mShutterPending && mNotifyCallback && (mMsgEnabled & CAMERA_MSG_SHUTTER)) {
        ALOGV("out2_w=%d, out2_h=%d, in2_w=%d, in2_h=%d",
             crop->out2_w, crop->out2_h, crop->in2_w, crop->in2_h);
        ALOGV("out1_w=%d, out1_h=%d, in1_w=%d, in1_h=%d",
             crop->out1_w, crop->out1_h, crop->in1_w, crop->in1_h);

        mDisplayHeap = mRawHeap;
        /* Now, invoke Notify Callback to unregister preview buffer
         * and register postview buffer with surface flinger. Set ext2
         * as 0 to indicate not to play shutter sound.
         */
        mNotifyCallback(CAMERA_MSG_SHUTTER, (int32_t)&size, 0,
                        mCallbackCookie);
        mShutterPending = false;
    }
    mShutterLock.unlock();
}

static void receive_shutter_callback(common_crop_t *crop)
{
    ALOGV("receive_shutter_callback: E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        /* Just play shutter sound at this time */
        obj->notifyShutter(crop, true);
    }
    ALOGV("receive_shutter_callback: X");
}

// Crop the picture in place.
static void crop_yuv420(uint32_t width, uint32_t height,
                 uint32_t cropped_width, uint32_t cropped_height,
                 uint8_t *image)
{
    uint32_t i, x, y;
    uint8_t* chroma_src, *chroma_dst;

    // Calculate the start position of the cropped area.
    x = (width - cropped_width) / 2;
    y = (height - cropped_height) / 2;
    x &= ~1;
    y &= ~1;

    // Copy luma component.
    for(i = 0; i < cropped_height; i++)
        memcpy(image + i * cropped_width,
               image + width * (y + i) + x,
               cropped_width);

    chroma_src = image + width * height;
    chroma_dst = image + cropped_width * cropped_height;

    // Copy chroma components.
    cropped_height /= 2;
    y /= 2;
    for(i = 0; i < cropped_height; i++)
        memcpy(chroma_dst + i * cropped_width,
               chroma_src + width * (y + i) + x,
               cropped_width);
}


void SemcCameraHardware::receiveRawSnapshot(){
    ALOGV("receiveRawSnapshot E");

    Mutex::Autolock cbLock(&mCallbackLock);
    /* Issue notifyShutter with mPlayShutterSoundOnly as TRUE */
    notifyShutter(&mCrop, true);

    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {

        if(native_get_picture(mCameraControlFd, &mCrop) == false) {
            ALOGE("receiveRawSnapshot X: native_get_picture failed!");
            return;
        }
        /* Its necessary to issue another notifyShutter here with
         * mPlayShutterSoundOnly as FALSE, since that is when the
         * preview buffers are unregistered with the surface flinger.
         * That is necessary otherwise the preview memory wont be
         * deallocated.
         */
        notifyShutter(&mCrop, false);

        //Create a Ashmem heap to copy data from PMem heap for application layer
        if(mRawSnapshotAshmemHeap != NULL){
            ALOGV("receiveRawSnapshot: clearing old mRawSnapShotAshmemHeap.");
            mRawSnapshotAshmemHeap.clear();
        }
        mRawSnapshotAshmemHeap = new AshmemPool(
                                        mRawSnapShotPmemHeap->mBufferSize,
                                        mRawSnapShotPmemHeap->mNumBuffers,
                                        mRawSnapShotPmemHeap->mFrameSize,
                                        "raw ashmem snapshot camera"
                                        );

        if(!mRawSnapshotAshmemHeap->initialized()){
            ALOGE("receiveRawSnapshot X: error initializing mRawSnapshotHeap");
            deinitRawSnapshot();
            return;
        }

        memcpy(mRawSnapshotAshmemHeap->mHeap->base(),
                mRawSnapShotPmemHeap->mHeap->base(),
                mRawSnapShotPmemHeap->mHeap->getSize());
       if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE))
           mDataCallback(CAMERA_MSG_COMPRESSED_IMAGE, mRawSnapshotAshmemHeap->mBuffers[0],
                mCallbackCookie);

    }

    //cleanup
    deinitRawSnapshot();

    ALOGV("receiveRawSnapshot X");
}

bool SemcCameraHardware::receiveRawPicture()
{
    ALOGD("START receiveRawPicture");

    Mutex::Autolock cbLock(&mCallbackLock);
    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_RAW_IMAGE)) {
        if(native_get_picture(mCameraControlFd, &mCrop) == false) {
            ALOGE("receiveRawPicture : native_get_picture failed!");
            return false;
        }

        // By the time native_get_picture returns, picture is taken. Call
        // shutter callback if cam config thread has not done that.
        notifyShutter(&mCrop, false);
    }
    ALOGD("END receiveRawPicture");
    return true;
}

void SemcCameraHardware::receiveJpegPictureFragment(
    uint8_t *buff_ptr, uint32_t buff_size)
{
    ALOGD("START receiveJpegPictureFragment : buff_size[%d]",(int)buff_size);

    sp<AshmemPool> autoThumbnailbuff = mThumbnailBuffHeap;

    mThumbnailBuffHeap =  new AshmemPool(buff_size + mThumbnailsize,
                                         kJpegBufferCount,
                                         buff_size + mThumbnailsize,
                                         "thumbnail_buffer");
    if(!mThumbnailBuffHeap->initialized()) {
        ALOGE("receiveJpegPicture : mThumbnailBuffHeap initialized error");
        mThumbnailBuffHeap.clear();
        return;
    }

    if(autoThumbnailbuff != NULL) {
        memcpy((uint8_t *)mThumbnailBuffHeap->mHeap->base() , (uint8_t *)autoThumbnailbuff->mHeap->base(), mThumbnailsize);
        autoThumbnailbuff.clear();
    }

    memcpy((uint8_t *)mThumbnailBuffHeap->mHeap->base()+mThumbnailsize , buff_ptr, buff_size);
    mThumbnailsize += buff_size;

    ALOGD("END receiveJpegPictureFragment ");

}

void SemcCameraHardware::receiveJpegPicture(void)
{
    ALOGD("START receiveJpegPicture : mThumbnailsize[%d]",(int)mThumbnailsize);
    do {
        if (mThumbnailBuffHeap == NULL) {
            ALOGE("receiveJpegPicture : mThumbnailBuffHeap is NULL");
            mStateCameraHal = CAMERAHAL_STATE_TAKEPICDONE;
            break;
        }
        if (!receiveExifData()) {
            ALOGE("receiveJpegPicture : receiveExifData error");
            mStateCameraHal = CAMERAHAL_STATE_TAKEPICDONE;
            break;
        }
    } while(0);

    mAfterTakepictureFlag = true;
        mJpegSnapShotHeap.clear();

    if(!mCameraExitFlag){
        LINK_jpeg_encoder_join();
        deinitRaw();
    }else{
        mEncoderThreadInfo = pthread_self();
        mReleaseWaitLock.lock();
        mReleaseWait.signal();
        mReleaseWaitLock.unlock();
    }
    mReceiveExifDataFlag = false;
    //Turn OFF the flash
    setSocTorchMode(0);
    ALOGD("END receiveJpegPicture");

}

bool SemcCameraHardware::previewEnabled()
{
    return mCameraRunning && mDataCallback && (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME);
}

status_t SemcCameraHardware::setPreviewSize(const CameraParameters& params)
{
    int width, height;
    size_t i = 0;
    params.getPreviewSize(&width, &height);

    ALOGD("START setPreviewSize : requested preview size %d x %d", width, height);

    // Validate the preview size
    for (i = 0; i < previewSizeCount; ++i) {
        if (width == supportedPreviewSizes[i].width
           && height == supportedPreviewSizes[i].height) {
            mParameters.setPreviewSize(supportedPreviewSizes[i].width, supportedPreviewSizes[i].height);
            // 720p , preview can be 768X432 (currently for 7x30 and 8k
            // targets)
            if(width == 1280 && height == 720 &&
             ((mCurrentTarget == TARGET_MSM7630) || (mCurrentTarget == TARGET_QSD8250))){
                 if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
                ALOGD("change preview resolution to 768X432 since recording is in 720p");
                     mDimension.display_width = CAMERA_SIGNATURE_HD720P_DISPSIZE_WIDTH;
                     mDimension.display_height= CAMERA_SIGNATURE_HD720P_DISPSIZE_HEIGHT;
                 }else{
                     mDimension.display_width = CAMERA_3RDPARTY_HD720P_DISPSIZE_WIDTH;
                     mDimension.display_height= CAMERA_3RDPARTY_HD720P_DISPSIZE_HEIGHT;
                 }
            }else {
                mDimension.display_width = supportedPreviewSizes[i].width;
                mDimension.display_height= supportedPreviewSizes[i].height;
            }
#if SCRITCH_OFF
            mCurrentAspectValue = supportedPreviewSizes[i].aspect;
#endif//SCRITCH_OFF
            ALOGD("setPreviewSize : preview size %d x %d", supportedPreviewSizes[i].width, supportedPreviewSizes[i].height);

            if(((CAMERA_FWVGASIZE_WIDTH == mDimension.display_width) && (CAMERA_FWVGASIZE_HEIGHT == mDimension.display_height)) ||
               ((CAMERA_OHDFWVGASIZE_WIDTH == mDimension.display_width) && (CAMERA_OHDFWVGASIZE_HEIGHT == mDimension.display_height))){
                mDimension.display_width = CAMERA_FWVGASIZE_DRV_WIDTH;
            }
            break;
       }
    }

    if (i == previewSizeCount) {
        mParameters.setPreviewSize(DEFAULT_PREVIEW_WIDTH, DEFAULT_PREVIEW_HEIGHT);
        mDimension.display_width = DEFAULT_PREVIEW_WIDTH;
        mDimension.display_height= DEFAULT_PREVIEW_HEIGHT;
        mCurrentAspectValue = ASPECT_VGA;
        ALOGD("setPreviewSize : Invalid preview size requested: ");
    }
    return NO_ERROR;
}

status_t SemcCameraHardware::setPictureSize(const CameraParameters& params)
{
    int width, height;
    int i = 0;
    params.getPictureSize(&width, &height);

    ALOGD("START setPictureSize :requested preview size %d x %d", width, height);

    // Validate the picture size
    for (i = PICTURE_SIZE_COUNT_SEMC - 1; i > -1; --i) {
            if (width >= picture_sizes[i].width
              && height >= picture_sizes[i].height) {
                mParameters.setPictureSize(picture_sizes[i].width, picture_sizes[i].height);
                mDimension.picture_width = picture_sizes[i].width;
                mDimension.picture_height = picture_sizes[i].height;
                ALOGD("END setPictureSize : size width %d height %d", picture_sizes[i].width, picture_sizes[i].height);
                return NO_ERROR;
            }
    }

#if SCRITCH_OFF
    if(-1 == i){
        for(i = 0; i < PICTURE_SIZE_COUNT_SEMC; i++){
            if(mCurrentAspectValue == picture_sizes[i].aspect){
                mParameters.setPictureSize(picture_sizes[i].width, picture_sizes[i].height);
                mDimension.picture_width = picture_sizes[i].width;
                mDimension.picture_height = picture_sizes[i].height;
                ALOGD("END setPictureSize : size width %d height %d", picture_sizes[i].width, picture_sizes[i].height);
                return NO_ERROR;
            }
        }
    }
#endif//SCRITCH_OFF
    ALOGE("END setPictureSize :Invalid picture size requested: %dx%d", width, height);
    return BAD_VALUE;
}

status_t SemcCameraHardware::setJpegQuality(const CameraParameters& params,
                                                const char *key /* = NULL */,
                                                const char *str_value /* = NULL */,
                                                int int_value /* = -1 */,
                                                bool collective /* = false */)
{
    ALOGD("%s E", __FUNCTION__);

    status_t rc = NO_ERROR;
    int quality = params.getInt(CameraParameters::KEY_JPEG_QUALITY);

    if(key != NULL) {
        quality = int_value;
    }

    if (quality > 0 && quality <= 100) {
        mParameters.set(CameraParameters::KEY_JPEG_QUALITY, quality);
    } else {
        ALOGE("Invalid jpeg quality=%d", quality);
        rc = BAD_VALUE;
    }

    quality = params.getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
    if (quality > 0 && quality <= 100) {
        mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, quality);
    } else {
        ALOGE("Invalid jpeg thumbnail quality=%d", quality);
        rc = BAD_VALUE;
    }
    ALOGD("%s X", __FUNCTION__);
    return rc;
}
status_t SemcCameraHardware::setZoom(const CameraParameters& params,
                                                const char *key /* = NULL */,
                                                const char *str_value /* = NULL */,
                                                int int_value /* = -1 */,
                                                bool collective /* = false */)
{
    ALOGD("%s E", __FUNCTION__);

    status_t rc = NO_ERROR;
    int step_value = params.getInt(CameraParameters::KEY_ZOOM);

    if(key != NULL) {
        step_value = int_value;
    }
    ALOGD("%s Provided step %d", __FUNCTION__, step_value);
    if(1>step_value){
        step_value = 1;
    }
    if(true == mPreviewFlag)
    {
      if(native_step_zoom(mCameraControlFd, step_value) == false)
      {
         ALOGE("END setZoom : native_step_zoom is error.");
         return UNKNOWN_ERROR;
      }
     }
     mZoom = step_value;


    ALOGD("%s ZOOM VALUE = %d", __FUNCTION__, mZoom);

    ALOGD("%s X", __FUNCTION__);
    return rc;
}


status_t SemcCameraHardware::setEffect(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setEffect : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            /* This root is Sinagature apl string string setting */
            str = str_value;
        }
    }else{
        /* This root is setParameters(pkg) and setCollective setting */
        str = params.get(CameraParameters::KEY_EFFECT);
    }
    int32_t value = attr_lookup(effects, sizeof(effects) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag) {
            if((strcmp(mEffect, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_EFFECT, sizeof(value), (void *)&value)) {
                    ALOGD("END setEffect : native_set_parm(CAMERA_SET_PARM_EFFECT) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mEffect, str);
                }
            }else{
                ALOGD("setEffect : Effect has already been setting.");
            }
        }

        /* ParametersClass setting. */
        if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
            mParameters.set(CameraParameters::KEY_EFFECT, str);
        }else{
            mParameters.set(CameraParameters::KEY_EFFECT, int_value);
        }

        ALOGD("END setEffect : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setEffect : BAD_VALUE str[%s] int_value[%d]",str ,int_value);
    return BAD_VALUE;
}

status_t SemcCameraHardware::setAutoExposure(const CameraParameters& params,
                                                 const char *key /* = NULL */,
                                                 const char *str_value /* = NULL */,
                                                 int int_value /* = -1 */,
                                                 bool collective /* = false */)

{
    ALOGD("START setAutoExposure : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::KEY_AUTO_EXPOSURE);
    }
    int32_t value = attr_lookup(autoexposure, sizeof(autoexposure) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mAutoExposure, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_EXPOSURE, sizeof(value), (void *)&value)) {
                    ALOGD("END setAutoExposure : native_set_parm(CAMERA_SET_PARM_EXPOSURE) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mAutoExposure, str);
                }
            }else{
                ALOGD("setAutoExposure : Auto Exposure has already been setting.");
            }
        }
        //ParametersClass setting.
        if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
            mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE, str);
        }else{
            mParameters.set(CameraParameters::KEY_AUTO_EXPOSURE, int_value);
        }
        ALOGD("END setAutoExposure : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setAutoExposure : BAD_VALUE str[%s] int_value[%d]",str ,int_value );

    return BAD_VALUE;
}

status_t SemcCameraHardware::setSharpness(const CameraParameters& params)
{
    ALOGD("START setSharpness");
    int sharpness = params.getInt(CameraParameters::KEY_SHARPNESS);
    mParameters.set(CameraParameters::KEY_SHARPNESS, sharpness);
    ALOGD("END setSharpness : NO_ERROR");
    return NO_ERROR;

}

status_t SemcCameraHardware::setContrast(const CameraParameters& params)
{
    ALOGD("START setContrast");
    int contrast = params.getInt(CameraParameters::KEY_CONTRAST);
    mParameters.set(CameraParameters::KEY_CONTRAST, contrast);
    ALOGD("END setContrast : NO_ERROR");
    return NO_ERROR;

}

status_t SemcCameraHardware::setSaturation(const CameraParameters& params)
{
    ALOGD("START setSaturation");
    int saturation = params.getInt(CameraParameters::KEY_SATURATION);
    mParameters.set(CameraParameters::KEY_SATURATION, saturation);
    ALOGD("END setSaturation : NOERROR");
    return NO_ERROR;

}

status_t SemcCameraHardware::setBrightness(const CameraParameters& params) {
    ALOGD("START setBrightness");
    int brightness = params.getInt("luma-adaptation");
    mParameters.set("luma-adaptation", brightness);
    ALOGD("END setBrightness : NOERROR");
    return NO_ERROR;

}

status_t SemcCameraHardware::setWhiteBalance(const CameraParameters& params,
                                                 const char *key /* = NULL */,
                                                 const char *str_value /* = NULL */,
                                                 int int_value /* = -1 */,
                                                 bool collective /* = false */)

{
    ALOGD("START setWhiteBalance : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::KEY_WHITE_BALANCE);
    }
    int32_t value = attr_lookup(whitebalance, sizeof(whitebalance) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        ALOGD("%s preview flag %s", __FUNCTION__, mPreviewFlag?"TRUE":"FALSE");
        if(true == mPreviewFlag){
            if((strcmp(mWhitebalance, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_WB, sizeof(value), (void *)&value)) {
                    ALOGD("END setWhiteBalance : native_set_parm(CAMERA_SET_PARM_WB) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mWhitebalance, str);
                }
            }else{
                ALOGD("setWhiteBalance :WhiteBalance has already been setting.");
            }
        }
        //ParametersClass setting.
        if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
            mParameters.set(CameraParameters::KEY_WHITE_BALANCE, str);
        }else{
            mParameters.set(CameraParameters::KEY_WHITE_BALANCE, int_value);
        }
        ALOGD("END setWhiteBalance : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setWhiteBalance : BAD_VALUE str[%s] int_value[%d]",str ,int_value );

    return BAD_VALUE;
}



status_t SemcCameraHardware::setFlash(const CameraParameters& params)
{
    const char *str = params.get(CameraParameters::KEY_FLASH_MODE);
    mParameters.set(CameraParameters::KEY_FLASH_MODE, str);

    ALOGD("flash mode value: %s", (str == NULL) ? "NULL" : str);

    if (str != NULL) {
       int32_t value = attr_lookup(flash, sizeof(flash) / sizeof(str_map), str);
       if (value != NOT_FOUND) {
            bool shouldBeOn = strcmp(str, "torch") == 0;
            setSocTorchMode(shouldBeOn);
       }
       return NO_ERROR;
    }
    ALOGD("Invalid flash mode value: %s", (str == NULL) ? "NULL" : str);
    return BAD_VALUE;

}

status_t SemcCameraHardware::setAntibanding(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setAntibanding : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            /* This root is Sinagature apl string string setting */
            str = str_value;
        }
    }else{
        /* This root is setParameters(pkg) and setCollective setting */
        str = params.get(CameraParameters::KEY_ANTIBANDING);
    }
    int32_t value = attr_lookup(antibanding, sizeof(antibanding) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag) {
            if((strcmp(mAntibanding, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_ANTIBANDING, sizeof(value), (void *)&value)) {
                    ALOGD("END setAntibanding : native_set_parm(CAMERA_SET_PARM_ANTIBANDING) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mAntibanding, str);
                }
            }else{
                ALOGD("setAntibanding : Antibanding has already been setting.");
            }
        }

        /* ParametersClass setting. */
        if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
            mParameters.set(CameraParameters::KEY_ANTIBANDING, str);
        }else{
            mParameters.set(CameraParameters::KEY_ANTIBANDING, int_value);
        }

        ALOGD("END setAntibanding : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setAntibanding : BAD_VALUE str[%s] int_value[%d]",str ,int_value);
    return BAD_VALUE;
}

status_t SemcCameraHardware::setLensshadeValue(const CameraParameters& params)
{
    ALOGD("START setLensshadeValue");
    const char *str = params.get(CameraParameters::KEY_LENSSHADE);
    mParameters.set(CameraParameters::KEY_LENSSHADE, str);
    ALOGD("END setLensshadeValue");
    return NO_ERROR;

}

status_t  SemcCameraHardware::setISOValue(const CameraParameters& params) {
    ALOGD("START setISOValue");
    const char *str = params.get(CameraParameters::KEY_ISO_MODE);
    mParameters.set(CameraParameters::KEY_ISO_MODE, str);
    ALOGD("END setISOValue");
    return NO_ERROR;

}


status_t SemcCameraHardware::setGpsLocation(const CameraParameters& params)
{
    const char *latitude = params.get(CameraParameters::KEY_GPS_LATITUDE);
    if (latitude) {
        mParameters.set(CameraParameters::KEY_GPS_LATITUDE, latitude);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_LATITUDE, "");
    }

    const char *latitudeRef = params.get(CameraParameters::KEY_GPS_LATITUDE_REF);
    if (latitudeRef) {
        mParameters.set(CameraParameters::KEY_GPS_LATITUDE_REF, latitudeRef);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_LATITUDE_REF, "");
    }

    const char *longitude = params.get(CameraParameters::KEY_GPS_LONGITUDE);
    if (longitude) {
        mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, longitude);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, "");
    }

    const char *longitudeRef = params.get(CameraParameters::KEY_GPS_LONGITUDE_REF);
    if (longitudeRef) {
        mParameters.set(CameraParameters::KEY_GPS_LONGITUDE_REF, longitudeRef);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_LONGITUDE_REF, "");
    }

    const char *altitudeRef = params.get(CameraParameters::KEY_GPS_ALTITUDE_REF);
    if (altitudeRef) {
        mParameters.set(CameraParameters::KEY_GPS_ALTITUDE_REF, altitudeRef);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_ALTITUDE_REF, "");
    }

    const char *altitude = params.get(CameraParameters::KEY_GPS_ALTITUDE);
    if (altitude) {
        mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, altitude);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, "");
    }

    const char *status = params.get(CameraParameters::KEY_GPS_STATUS);
    if (status) {
        mParameters.set(CameraParameters::KEY_GPS_STATUS, status);
    }

    const char *dateTime = params.get(CameraParameters::KEY_EXIF_DATETIME);
    if (dateTime) {
        mParameters.set(CameraParameters::KEY_EXIF_DATETIME, dateTime);
    }

    const char *timestamp = params.get(CameraParameters::KEY_GPS_TIMESTAMP);
    if (timestamp) {
        mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP, timestamp);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_TIMESTAMP, "");
    }

    const char *ProcessingMethod = params.get(CameraParameters::KEY_GPS_PROCESSING_METHOD);
    if (ProcessingMethod) {
        mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, ProcessingMethod);
    } else {
        mParameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, "");
    }

    return NO_ERROR;
}

status_t SemcCameraHardware::setRotation(const CameraParameters& params,
                                               const char *key /* = NULL */,
                                               const char *str_value /* = NULL */,
                                               int int_value /* = -1 */)
{
    ALOGD("START setRotation : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    status_t rc = NO_ERROR;
    int rotation = 0;
    if (NULL == key) {
        rotation = params.getInt(CameraParameters::KEY_ROTATION);
    } else {
        rotation = int_value;
    }
    if (rotation != NOT_FOUND) {
        if (rotation == 0 || rotation == 90 || rotation == 180
            || rotation == 270) {
          mParameters.set(CameraParameters::KEY_ROTATION, rotation);
        } else {
            ALOGE("Invalid rotation value: %d", rotation);
            rc = BAD_VALUE;
        }
    }
    ALOGD("END setRotation : NO_ERROR");
    return rc;
}

//status_t SemcCameraHardware::setZoom(const CameraParameters& params)
//{
//    status_t rc = NO_ERROR;
//
//    ALOGD("START setZoom");
//    int32_t zoom_level = params.getInt("zoom");
//    mParameters.set("zoom", zoom_level);
//    ALOGD("END setZoom");
//
//    return rc;
//}

status_t SemcCameraHardware::setFocusMode(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)

{
    ALOGD("START setFocusMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;
    int32_t value = NOT_FOUND;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::KEY_FOCUS_MODE);
    }
    value = attr_lookup(focus_modes, sizeof(focus_modes) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mFocusMode, str) != 0) /*|| (true == collective)*/) {
                if(true != native_set_parm(CAMERA_SET_PARM_AF_RANGE, sizeof(value), (void *)&value, DRV_TIMEOUT_10K)) {
                    ALOGD("END setFocusMode : native_set_parm(CAMERA_SET_PARM_AF_RANGE) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mFocusMode, str);
                }
            }else{
                ALOGD("setFocusMode : Focus mode(AutoFocusRange) has already been setting.");
            }
        }
        //ParametersClass setting.
        if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
            mParameters.set(CameraParameters::KEY_FOCUS_MODE, str);
        }else{
            mParameters.set(CameraParameters::KEY_FOCUS_MODE, int_value);
        }
        ALOGD("END setFocusMode : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setFocusMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value );

    return BAD_VALUE;
}
status_t SemcCameraHardware::setOrientation(const CameraParameters& params)
{
    const char *str = params.get("orientation");

    if (str != NULL) {
        if (strcmp(str, CAMERAHAL_ORIENTATION_PORTRAIT) == 0 || strcmp(str, CAMERAHAL_ORIENTATION_LANDSCAPE) == 0) {
            // Camera service needs this to decide if the preview frames and raw
            // pictures should be rotated.
            mParameters.set(ORIENTATION, str);
        } else {
            ALOGD("Invalid orientation value: %s", str);
            return BAD_VALUE;
        }
    }
    return NO_ERROR;
}
#if 0
status_t SemcCameraHardware::setOrientation(const CameraParameters& params,
                                                const char *key /* = NULL */,
                                                const char *str_value /* = NULL */,
                                                int int_value /* = -1 */,
                                                bool collective /* = false */)

{
    ALOGD("START setOrientation : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::ORIENTATION);
        if(str == NULL)
        {
            return NO_ERROR;
        }
    }
    int32_t value = attr_lookup(orientation, sizeof(orientation) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
            mParameters.set(CameraParameters::ORIENTATION, str);
        }else{
            mParameters.setOrientation(int_value);
        }
        ALOGD("END setOrientation : NO_ERROR" );
        return NO_ERROR;
    }
    ALOGD("END setOrientation : BAD_VALUE" );
    return BAD_VALUE;

}
#endif
status_t SemcCameraHardware::setPictureFormat(const CameraParameters& params)
{
    ALOGD("START setPictureFormat");
    const char * str = params.get(CameraParameters::KEY_PICTURE_FORMAT);
    mParameters.set(CameraParameters::KEY_PICTURE_FORMAT, str);
    ALOGD("END setPictureFormat");

    return NO_ERROR;
}

SemcCameraHardware::MemPool::MemPool(int buffer_size, int num_buffers,
                                         int frame_size,
                                         const char *name) :
    mBufferSize(buffer_size),
    mNumBuffers(num_buffers),
    mFrameSize(frame_size),
    mBuffers(NULL), mName(name)
{
    int page_size_minus_1 = getpagesize() - 1;
    mAlignedBufferSize = (buffer_size + page_size_minus_1) & (~page_size_minus_1);
}

void SemcCameraHardware::MemPool::completeInitialization()
{
    // If we do not know how big the frame will be, we wait to allocate
    // the buffers describing the individual frames until we do know their
    // size.

    if (mFrameSize > 0) {
        mBuffers = new sp<MemoryBase>[mNumBuffers];
        for (int i = 0; i < mNumBuffers; i++) {
            mBuffers[i] = new
                MemoryBase(mHeap,
                           i * mAlignedBufferSize,
                           mFrameSize);
        }
    }
}

SemcCameraHardware::AshmemPool::AshmemPool(int buffer_size, int num_buffers,
                                               int frame_size,
                                               const char *name) :
    SemcCameraHardware::MemPool(buffer_size,
                                    num_buffers,
                                    frame_size,
                                    name)
{
    ALOGV("constructing MemPool %s backed by ashmem: "
         "%d frames @ %d uint8_ts, "
         "buffer size %d",
         mName,
         num_buffers, frame_size, buffer_size);

    int page_mask = getpagesize() - 1;
    int ashmem_size = buffer_size * num_buffers;
    ashmem_size += page_mask;
    ashmem_size &= ~page_mask;

    mHeap = new MemoryHeapBase(ashmem_size);

    completeInitialization();
}

static bool register_buf(int camfd,
                         int size,
                         int frame_size,
                         int pmempreviewfd,
                         uint32_t offset,
                         uint8_t *buf,
                         int pmem_type,
                         bool vfe_can_write,
                         bool register_buffer = true);

SemcCameraHardware::PmemPool::PmemPool(const char *pmem_pool,
                                           int flags,
                                           int camera_control_fd,
                                           int pmem_type,
                                           int buffer_size, int num_buffers,
                                           int frame_size,
                                           const char *name) :
    SemcCameraHardware::MemPool(buffer_size,
                                    num_buffers,
                                    frame_size,
                                    name),
    mPmemType(pmem_type),
    mCameraControlFd(dup(camera_control_fd))
{
    ALOGV("constructing MemPool %s backed by pmem pool %s: "
         "%d frames @ %d bytes, buffer size %d",
         mName,
         pmem_pool, num_buffers, frame_size,
         buffer_size);

    ALOGV("%s: duplicating control fd %d --> %d",
         __FUNCTION__,
         camera_control_fd, mCameraControlFd);

    bool ret = false;

    // Make a new mmap'ed heap that can be shared across processes.
    // mAlignedBufferSize is already in 4k aligned. (do we need total size necessary to be in power of 2??)
    mAlignedSize = mAlignedBufferSize * num_buffers;

    sp<MemoryHeapBase> masterHeap =
        new MemoryHeapBase(pmem_pool, mAlignedSize, flags);

    if (masterHeap->getHeapID() < 0) {
        ALOGE("failed to construct master heap for pmem pool %s", pmem_pool);
        masterHeap.clear();
        return;
    }

    sp<MemoryHeapPmem> pmemHeap = new MemoryHeapPmem(masterHeap, flags);
    if (pmemHeap->getHeapID() >= 0) {
        pmemHeap->slap();
        masterHeap.clear();
        mHeap = pmemHeap;
        pmemHeap.clear();

        mFd = mHeap->getHeapID();
        if (::ioctl(mFd, PMEM_GET_SIZE, &mSize)) {
            ALOGE("pmem pool %s ioctl(PMEM_GET_SIZE) error %s (%d)",
                 pmem_pool,
                 ::strerror(errno), errno);
            mHeap.clear();
            return;
        }

        ALOGV("pmem pool %s ioctl(fd = %d, PMEM_GET_SIZE) is %ld",
             pmem_pool,
             mFd,
             mSize.len);
        ALOGD("mBufferSize=%d, mAlignedBufferSize=%d\n", mBufferSize, mAlignedBufferSize);
        // Unregister preview buffers with the camera drivers.  Allow the VFE to write
        // to all preview buffers except for the last one.
        // Only Register the preview, snapshot and thumbnail buffers with the kernel.
        if( (strcmp("postview", mName) != 0) ){
            int num_buf = num_buffers;
            if(!strcmp("preview", mName)) num_buf = kPreviewBufferCount;
            ALOGD("num_buffers = %d", num_buf);
            for (int cnt = 0; cnt < num_buf; ++cnt) {
                int active = 1;
                if(pmem_type == MSM_PMEM_VIDEO){
                     active = (cnt<ACTIVE_VIDEO_BUFFERS);
                     ALOGV(" pmempool creating video buffers : active %d ", active);
                }
                else if (pmem_type == MSM_PMEM_PREVIEW){
                     active = (cnt < ACTIVE_VIDEO_BUFFERS);
                }
                ret = register_buf(mCameraControlFd,
                               mBufferSize,
                               mFrameSize,
                               mHeap->getHeapID(),
                               mAlignedBufferSize * cnt,
                               (uint8_t *)mHeap->base() + mAlignedBufferSize * cnt,
                               pmem_type,
                               active);
                if( true != ret ){
                    ALOGE("mHeap register_buf error active %d", active);
                    mHeap.clear();
                    return;
                }
            }
        }

        completeInitialization();
    }
    else ALOGE("pmem pool %s error: could not create master heap!",
              pmem_pool);
}

SemcCameraHardware::PmemPool::~PmemPool()
{
    ALOGV("%s: %s E", __FUNCTION__, mName);
    if (mHeap != NULL) {
        // Unregister preview buffers with the camera drivers.
        //  Only Unregister the preview, snapshot and thumbnail
        //  buffers with the kernel.
        if( (strcmp("postview", mName) != 0) ){
            int num_buffers = mNumBuffers;
            if(!strcmp("preview", mName)) num_buffers = kPreviewBufferCount;
            for (int cnt = 0; cnt < num_buffers; ++cnt) {
                register_buf(mCameraControlFd,
                         mBufferSize,
                         mFrameSize,
                         mHeap->getHeapID(),
                         mAlignedBufferSize * cnt,
                         (uint8_t *)mHeap->base() + mAlignedBufferSize * cnt,
                         mPmemType,
                         false,
                         false /* unregister */);
            }
        }
    }
    ALOGV("destroying PmemPool %s: closing control fd %d",
         mName,
         mCameraControlFd);
    close(mCameraControlFd);
    ALOGV("%s: %s X", __FUNCTION__, mName);
}

SemcCameraHardware::MemPool::~MemPool()
{
    ALOGV("destroying MemPool %s", mName);
    if (mFrameSize > 0)
        delete [] mBuffers;
    mHeap.clear();
    ALOGV("destroying MemPool %s completed", mName);
}

static bool register_buf(int camfd,
                         int size,
                         int frame_size,
                         int pmempreviewfd,
                         uint32_t offset,
                         uint8_t *buf,
                         int pmem_type,
                         bool vfe_can_write,
                         bool register_buffer)
{
    Mutex::Autolock lock(g_RegisterBufRunningLock);
    struct msm_pmem_info pmemBuf;

    pmemBuf.type     = pmem_type;
    pmemBuf.fd       = pmempreviewfd;
    pmemBuf.offset   = offset;
    pmemBuf.len      = size;
    pmemBuf.vaddr    = buf;
    pmemBuf.y_off    = 0;

    if(pmem_type == MSM_PMEM_RAW_MAINIMG)
        pmemBuf.cbcr_off = 0;
    else
        pmemBuf.cbcr_off = PAD_TO_WORD(frame_size * 2 / 3);

    pmemBuf.active   = vfe_can_write;

    ALOGV("register_buf: camfd = %d, reg = %d buffer = %p",
         camfd, !register_buffer, buf);
    if (ioctl(camfd,
              register_buffer ?
              MSM_CAM_IOCTL_REGISTER_PMEM :
              MSM_CAM_IOCTL_UNREGISTER_PMEM,
              &pmemBuf) < 0) {
        ALOGE("register_buf: MSM_CAM_IOCTL_(UN)REGISTER_PMEM fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }
    return true;
}

status_t SemcCameraHardware::MemPool::dump(int fd, const Vector<String16>& args) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, 255, "SemcCameraHardware::AshmemPool::dump\n");
    result.append(buffer);
    if (mName) {
        snprintf(buffer, 255, "mem pool name (%s)\n", mName);
        result.append(buffer);
    }
    if (mHeap != 0) {
        snprintf(buffer, 255, "heap base(%p), size(%d), flags(%d), device(%s)\n",
                 mHeap->getBase(), mHeap->getSize(),
                 mHeap->getFlags(), mHeap->getDevice());
        result.append(buffer);
    }
    snprintf(buffer, 255,
             "buffer size (%d), number of buffers (%d), frame size(%d)",
             mBufferSize, mNumBuffers, mFrameSize);
    result.append(buffer);
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

static void receive_camframe_callback(struct msm_frame *frame)
{
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->receivePreviewFrame(frame);
    }
}

static void receive_jpeg_fragment_callback(uint8_t *buff_ptr, uint32_t buff_size)
{
    ALOGV("receive_jpeg_fragment_callback E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->receiveJpegPictureFragment(buff_ptr, buff_size);
    }
    ALOGV("receive_jpeg_fragment_callback X");
}

static void receive_jpeg_callback(jpeg_event_t status)
{
    ALOGV("receive_jpeg_callback E (completion status %d)", status);
    if (status == JPEG_EVENT_DONE) {
        sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
        if (obj != 0) {
            obj->receiveJpegPicture();
        }
    }
    ALOGV("receive_jpeg_callback X");
}
// 720p : video frame calbback from camframe
static void receive_camframe_video_callback(struct msm_frame *frame)
{
    ALOGV("receive_camframe_video_callback E");
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
			obj->receiveRecordingFrame(frame);
		 }
    ALOGV("receive_camframe_video_callback X");
}

void SemcCameraHardware::setCallbacks(notify_callback notify_cb,
                             data_callback data_cb,
                             data_callback_timestamp data_cb_timestamp,
                             void* user)
{
    Mutex::Autolock lock(mLock);
    mNotifyCallback = notify_cb;
    mDataCallback = data_cb;
    mDataCallbackTimestamp = data_cb_timestamp;
    mCallbackCookie = user;
}

void SemcCameraHardware::enableMsgType(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    mMsgEnabled |= msgType;
}

void SemcCameraHardware::disableMsgType(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    mMsgEnabled &= ~msgType;
}

bool SemcCameraHardware::msgTypeEnabled(int32_t msgType)
{
    return (mMsgEnabled & msgType);
}

bool SemcCameraHardware::useOverlay(void)
{
    if( mCurrentTarget == TARGET_MSM7630 ) {
        /* Only 7x30 supports Overlay */
        mUseOverlay = true;
    } else
        mUseOverlay = false;

    ALOGV(" Using Overlay : %s ", mUseOverlay ? "YES" : "NO" );
    return mUseOverlay;
}

status_t SemcCameraHardware::setOverlay(const sp<Overlay> &Overlay)
{
    if( Overlay != NULL) {
        ALOGV(" Valid overlay object ");
        mOverlayLock.lock();
        mOverlay = Overlay;
        mOverlayLock.unlock();
    } else {
        ALOGE(" Overlay object NULL. returning ");
        mOverlay = NULL;
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

void SemcCameraHardware::receive_camframetimeout(void) {
    ALOGV("receive_camframetimeout: E");
    Mutex::Autolock l(&mCamframeTimeoutLock);
    ALOGD(" Camframe timed out. Not receiving any frames from camera driver ");
    camframe_timeout_flag = true;
    ALOGD("receive_camframetimeout: X");
}

static void receive_camframetimeout_callback(void) {
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->receive_camframetimeout();
    }
}

void SemcCameraHardware::storePreviewFrameForPostview(void) {
    ALOGV(" storePreviewFrameForPostview : E ");

    /* Since there is restriction on the maximum overlay dimensions
     * that can be created, we use the last preview frame as postview
     * for 7x30. */
    ALOGV(" Copying the preview buffer to postview buffer %d  ",
         mPreviewFrameSize);
    if( mPostViewHeap != NULL && mLastQueuedFrame != NULL) {
	memcpy(mPostViewHeap->mHeap->base(),
		(uint8_t *)mLastQueuedFrame, mPreviewFrameSize );
    } else
        ALOGE(" Failed to store Preview frame. No Postview ");

    ALOGV(" storePreviewFrameForPostview : X ");
}

status_t SemcCameraHardware::getBufferInfo(sp<IMemory>& Frame, size_t *alignedSize) {
    status_t ret;
    ALOGV(" getBufferInfo : E ");
    if( ( mCurrentTarget == TARGET_MSM7630 ) || (mCurrentTarget == TARGET_QSD8250))
    {
        if( mRecordHeap != NULL){
            ALOGV(" Setting valid buffer information ");
            Frame = mRecordHeap->mBuffers[0];
            if( alignedSize != NULL) {
                *alignedSize = mRecordHeap->mAlignedBufferSize;
                ALOGV(" HAL : alignedSize = %d ", *alignedSize);
                ret = NO_ERROR;
            } else {
                ALOGE(" HAL : alignedSize is NULL. Cannot update alignedSize ");
                ret = UNKNOWN_ERROR;
            }
        } else {
            ALOGE(" RecordHeap is null. Buffer information wont be updated ");
            Frame = NULL;
            ret = UNKNOWN_ERROR;
        }
    } else {
        if(mPreviewHeap != NULL) {
            ALOGV(" Setting valid buffer information ");
            Frame = mPreviewHeap->mBuffers[0];
            if( alignedSize != NULL) {
                *alignedSize = mPreviewHeap->mAlignedBufferSize;
                ALOGV(" HAL : alignedSize = %d ", *alignedSize);
                ret = NO_ERROR;
            } else {
                ALOGE(" HAL : alignedSize is NULL. Cannot update alignedSize ");
                ret = UNKNOWN_ERROR;
            }
        } else {
            ALOGE(" PreviewHeap is null. Buffer information wont be updated ");
            Frame = NULL;
            ret = UNKNOWN_ERROR;
        }
    }
    ALOGV(" getBufferInfo : X ");
    return ret;
}

/**
 * @brief semcCameraExtensionJudgment
 * @param startflg [IN] Start cause flag
 * @return void
 *
 * @brief Judge the start cause of the camera
 */
void SemcCameraHardware::semcCameraExtensionJudgment(bool startflg)
{
    ALOGD("START semcCameraExtensionJudgment: startflg =%d", startflg);
    if(startflg){
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0)
        {
            mSemcCameraFlag = 0x01;
        }
    }
    else{
        mSemcCameraFlag = 0x00;
    }
    ALOGD("END semcCameraExtensionJudgment: mSemcCameraFlag =%d", mSemcCameraFlag);

}
/**
 * @brief SemcCameraHardware::semcCameraExtensionService
 * @return void
 *
 * @brief The inside function to judge the start cause of the camera
 */
void SemcCameraHardware::semcCameraExtensionService()
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0){
        ALOGD("START semcCameraExtensionService");
        Mutex::Autolock l(&mLock);
        mSemcCameraFlag = mSemcCameraFlag << 1;

        mSemcCameraFlag |= 0x01;
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0)
        {
            setInitialValue();
        }
    }

    ALOGD("END semcCameraExtensionService");
}

/**
 * @brief SemcCameraHardware::setInitialValue
 * @return void
 * @brief CameraParameters initial value setting
 */
void SemcCameraHardware::setInitialValue(){

    ALOGD("START setInitialValue");
    size_t i;
#if SCRITCH_OFF
    struct get_versions_t hal_get_versions;
    memset(&hal_get_versions, 0, sizeof(get_versions_t));

    
    native_get_firmware_version(mCameraControlFd, &hal_get_versions);
    ALOGD("setInitialValue:firmware_version = %s", hal_get_versions.firmware);
    mParameters.set("firmware-version",hal_get_versions.firmware);
#endif//SCRITCH_OFF

    for(i = 0; i < INITIAL_SETTING_VALUE_COUNT; i++){
        //CameraParameters initial value setting
        mParameters.set(initial_setting_value[i].set_key, initial_setting_value[i].set_value);
    }
    mParameters.setPreviewFrameRate(DEFAULT_PREVIEW_FRAME_RATE_EX);
    mParameters.set(ORIENTATION, CAMERAHAL_ORIENTATION_LANDSCAPE);
    focus_mode_values = create_values_str(focus_modes, sizeof(focus_modes) / sizeof(str_map));
    mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                    focus_mode_values);

    scene_mode_values = create_values_str(scene_mode, sizeof(scene_mode) / sizeof(str_map));
    mParameters.set(CameraParameters::KEY_SUPPORTED_SCENE_MODES,
                    scene_mode_values);
    mZoom = 1;
    ALOGD("END setInitialValue");
}

#if SCRITCH_OFF
void SemcCameraHardware::setCallbacks(notify_callback notify_cb,
                                          data_callback data_cb,
                                          data_callback_timestamp data_cb_timestamp,
                                          void* user)
{
    ALOGD("START setCallbacks(SEMC)");
    Mutex::Autolock lock(mLock);
    mNotifyCallback = notify_cb;
    mDataCallback = data_cb;
    mDataCallbackTimestamp = data_cb_timestamp;
    mCallbackCookie = user;
    ALOGD("END setCallbacks(SEMC)");
}

#endif//SCRITCH_OFF


status_t SemcCameraHardware::setSceneMode(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setSceneMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;
    int framerate = 0;
    int32_t value = NOT_FOUND;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::KEY_SCENE_MODE);
        ALOGD("setSceneMode : Get string name[%s] ]", str );

    }
    value = attr_lookup(scene_mode, sizeof(scene_mode) / sizeof(str_map), str, int_value);

    //if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mSceneMode, str) != 0) || (strcmp(mKeyScene, CameraParameters::KEY_SCENE_MODE) != 0)){
                if(true != native_set_parm(CAMERA_SET_PARM_BESTSHOT_MODE, sizeof(value), (void *)&value)) {
                    ALOGD("END native_set_parm(CAMERA_SET_PARM_BESTSHOT_MODE) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                    strcpy(mSceneMode, str);
                }
                strcpy(mKeyScene, CameraParameters::KEY_SCENE_MODE);
                switch(value) {

                    case CAMERA_BESTSHOT_AUTO:
                    case CAMERA_BESTSHOT_PORTRAIT:
                    case CAMERA_BESTSHOT_SPORTS:
                    case CAMERA_BESTSHOT_NIGHT:
                    case CAMERA_BESTSHOT_NIGHT_PORTRAIT:
                    case CAMERA_BESTSHOT_PARTY:
                        strcpy(mWhitebalance, CameraParameters::WHITE_BALANCE_AUTO);
                        break;
                    case CAMERA_BESTSHOT_LANDSCAPE:
                    case CAMERA_BESTSHOT_SNOW:
                        strcpy(mWhitebalance, CameraParameters::WHITE_BALANCE_DAYLIGHT);
                        break;
                    default:
                        ALOGE("scene mode is not support parameter");
                        break;
                }
                /* case 3rdparty */
                if ( ((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0 ) {
                    if((CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_FRAME_RATE == mFrameRateSetFrom) || (true == collective)) {
                        if(true != native_set_parm(CAMERA_SET_PARM_FPS, sizeof(framerate), (void *)&framerate)) {
                            ALOGD("END native_set_parm(CAMERA_SET_PARM_FPS) : ERROR");
                            return UNKNOWN_ERROR;
                        }
                        mFrameRateSetFrom = CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_SCENE;
                    }
                }
            }else{
                ALOGD("%s Scene mode has already been set", __FUNCTION__);
                
            }
        }
        //ParametersClass setting.
        if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
            mParameters.set(CameraParameters::KEY_SCENE_MODE, str);
        }else{
            mParameters.set(CameraParameters::KEY_SCENE_MODE, int_value);
        }
        ALOGD("END setSceneMode : NO_ERROR");
        return NO_ERROR;
    //}
    //ALOGD("END setSceneMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value );
    return BAD_VALUE;
}
#if SCRITCH_OFF
status_t SemcCameraHardware::setSceneRecognition(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setSceneRecognition : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::SCENE_RECOGNITION);
    }
    if(strcmp(str, CameraParameters::RECOGNITION_STOP) == 0){
        strcpy(mSceneMode, str);
        mParameters.set(CameraParameters::SCENE_RECOGNITION, str);
        return NO_ERROR;
    }

    int32_t value = attr_lookup(scene_recognition, sizeof(scene_recognition) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mSceneMode, str) != 0)
             || (strcmp(mKeyScene, CameraParameters::SCENE_RECOGNITION) != 0)
             || (true == collective)){
                if(true != native_set_parm(CAMERA_SET_PARM_BESTSHOT_MODE, sizeof(value), (void *)&value)) {
                    ALOGD("END native_set_parm(CAMERA_SET_PARM_BESTSHOT_MODE) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mSceneMode, str);
                }
                strcpy(mKeyScene, CameraParameters::SCENE_RECOGNITION);
            }else{
                ALOGD("setSceneRecognition :SceneRecognition has already been setting.");
            }
        }
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            //ParametersClass setting.
            if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                mParameters.set(CameraParameters::SCENE_RECOGNITION, str);
            }else{
                mParameters.set(CameraParameters::SCENE_RECOGNITION, int_value);
            }
        }
        ALOGD("END setSceneRecognition : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setSceneRecognition : BAD_VALUE str[%s] int_value[%d]",str ,int_value );
    return BAD_VALUE;
}
#endif //SCRITCH_OFF
status_t SemcCameraHardware::setExposureCompensation(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setExposureCompensation : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::KEY_EXPOSURE_COMPENSATION);
    }
    int32_t value = attr_lookup(exposure_compensation, sizeof(exposure_compensation) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mExposureCompensation, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_EXPOSURE_COMPENSATION, sizeof(value), (void *)&value)) {
                    ALOGD("END setExposureCompensation : native_set_parm(CAMERA_SET_PARM_EXPOSURE_COMPENSATION) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mExposureCompensation, str);
                }
            }else{
                ALOGD("setExposureCompensation :ExposureCompensation has already been setting.");
            }
        }
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            //ParametersClass setting.
            if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, str);
            }else{
                mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, int_value);
            }
        }
        ALOGD("END setExposureCompensation : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setExposureCompensation : BAD_VALUE str[%s] int_value[%d]",str ,int_value );
    return BAD_VALUE;
}

status_t SemcCameraHardware::setFramerate(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setFramerate : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    int framerate = 0;
    if(NULL == key) {
        framerate = params.getPreviewFrameRate();
    } else {
        framerate = int_value;
    }

    // case 3rdpatry
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
        //FrameRate setting becomes variable in the case of "0"
        if(0 == framerate) {
            framerate = 1;
        }
    }
    if(mFrameRate != framerate) {
        if(true != native_set_parm(CAMERA_SET_PARM_FPS, sizeof(framerate), (void *)&framerate)) {
            ALOGD("END native_set_parm(CAMERA_SET_PARM_FPS) : ERROR");
            return UNKNOWN_ERROR;
        }
        mFrameRate = framerate;

        /* case 3rdpatry */
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
            mFrameRateSetFrom = CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_FRAME_RATE;
        }
    }else{
        /* case 3rdpatry */
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
            if((true == collective) && (CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_FRAME_RATE == mFrameRateSetFrom)) {
                if(true != native_set_parm(CAMERA_SET_PARM_FPS, sizeof(framerate), (void *)&framerate)) {
                    ALOGD("END native_set_parm(CAMERA_SET_PARM_FPS) : ERROR");
                    return UNKNOWN_ERROR;
                }
                mFrameRate = framerate;
                mFrameRateSetFrom = CAMERAHAL_PREVIEW_FRAME_RATE_SET_FROM_FRAME_RATE;
            }else{
                ALOGD("setFramerate : Framerate has already been setting or Set after setSceneMode");
            }
        }else{
            if(true == collective) {
                if(true != native_set_parm(CAMERA_SET_PARM_FPS, sizeof(framerate), (void *)&framerate)) {
                    ALOGD("END native_set_parm(CAMERA_SET_PARM_FPS) : ERROR");
                    return UNKNOWN_ERROR;
                }
                mFrameRate = framerate;
            }else{
                ALOGD("setFramerate : Framerate has already been setting.");
            }
        }
    }
    if(0 != framerate) {
        mParameters.setPreviewFrameRate(framerate);
    }

    ALOGD("END setFramerate : ret[NO_ERROR]" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setPreviewFpsRange(const CameraParameters& params)
{
    ALOGD("START setPreviewFpsRange");
    int min_fps=-1, max_fps=-1;
    params.getPreviewFpsRange(&min_fps, &max_fps);
    char str[32];
    if (min_fps == 5000 && max_fps == 30000) {
        sprintf(str, "%d,%d", min_fps,max_fps);
        mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, str);
    } else {
        return BAD_VALUE;
    }

    return NO_ERROR;
}

#if SCRITCH_OFF
status_t SemcCameraHardware::setAfMode(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setAfMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::AF_MODE);
    }
    int32_t value = attr_lookup(af_mode, sizeof(af_mode) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if((true == mPreviewFlag) && (strcmp(mCafMode, CameraParameters::CAF_MODE_OFF) == 0)) {
            if((strcmp(mAfMode, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_AF_FRAME, sizeof(value), (void *)&value, DRV_TIMEOUT_10K)) {
                    ALOGD("END setAfMode : native_set_parm(CAMERA_SET_PARM_AF_FRAME) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                    strcpy(mAfMode, str);
                }
            }else{
                ALOGD("setAfMode : Af mode has already been setting.");
            }
        }

        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0)
        {
            //ParametersClass setting.
            if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                mParameters.set(CameraParameters::AF_MODE, str);
            }else{
                mParameters.set(CameraParameters::AF_MODE, int_value);
            }
        }

        ALOGD("END setAfMode : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setAfMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value);
    return BAD_VALUE;
}

status_t SemcCameraHardware::setCAFMode(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setCAFMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;
    int ret = NOT_FOUND;

    if(key != NULL) {
        if(str_value != NULL) {
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    } else{ 
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::CAF_MODE);
    }
    int32_t value = attr_lookup(caf_mode, sizeof(caf_mode) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            if(true == mPreviewFlag) {
                if((strcmp(mCafMode, str) != 0) || (true == collective)) {
                    if(true != native_set_parm(CAMERA_SET_PARM_CAF_MODE, sizeof(value), (void *)&value, DRV_TIMEOUT_10K)) {
                        ALOGD("END setCafMode : native_set_parm(CAMERA_SET_PARM_CAF_MODE) : ERROR");
                        return UNKNOWN_ERROR;
                    }
                    if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                        strcpy(mCafMode, str);
                    }
                    if(strcmp(mCafMode, CameraParameters::CAF_MODE_OFF) == 0) {
                        ALOGD("setCAFMode : The setup of AF Mode,Focus Mode after CAF_OFF is set up ");
                        //af mode setting
                        strcpy(mAfMode, CAMERA_DEFAULT_VALUE);
                        if(setAfMode(mParameters) != NO_ERROR) {
                            ALOGD("setCAFMode : setAfMode(mParameters) : ERROR");
                        }

                        //af range setting
                        strcpy(mFocusMode, CAMERA_DEFAULT_VALUE);
                        if(setFocusMode(mParameters) != NO_ERROR) {
                            ALOGD("setCAFMode : setFocusMode(mParameters) : ERROR");
                        }
                    }
                } else {
                    ALOGD("setCafMode : Caf mode has already been setting.");
                }
            }
            //ParametersClass setting.
            if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                mParameters.set(CameraParameters::CAF_MODE, str);
            } else {
                mParameters.set(CameraParameters::CAF_MODE, int_value);
            }
        }
        ALOGD("END setCafMode : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setCAFMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value);
    return BAD_VALUE;
}

status_t SemcCameraHardware::setHandJitterReduction(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setHandJitterReduction : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::HJR_MODE);
    }
    int32_t value = attr_lookup(hjr_mode, sizeof(hjr_mode) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(true == mPreviewFlag){
            if((strcmp(mHJR, str) != 0) || (true == collective)) {
                if(true != native_set_parm(CAMERA_SET_PARM_HJR, sizeof(value), (void *)&value)) {
                    ALOGD("END setHandJitterReduction : native_set_parm(CAMERA_SET_PARM_HJR) : ERROR");
                    return UNKNOWN_ERROR;
                }
                if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                    strcpy(mHJR, str);
                }
            }else{
                ALOGD("setHandJitterReduction : Antibanding mode has already been setting.");
            }
        }
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            //ParametersClass setting.
            if(strcmp(str, CAMERA_DEFAULT_VALUE) != 0) {
                mParameters.set(CameraParameters::HJR_MODE, str);
            }else{
                mParameters.set(CameraParameters::HJR_MODE, int_value);
            }
        }
        ALOGD("END setHandJitterReduction : NO_ERROR");
        return NO_ERROR;
    }
    ALOGD("END setHandJitterReduction : BAD_VALUE" );
    return BAD_VALUE;
}
status_t SemcCameraHardware::setSmileMode(const CameraParameters& params,
                                              const char *key /* = NULL */,
                                              const char *str_value /* = NULL */,
                                              int int_value /* = -1 */,
                                              bool collective /* = false */)
{
    ALOGD("START setSmileMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::SMILE_MODE);
    }
    int32_t value = attr_lookup(smile_mode, sizeof(smile_mode) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                mParameters.set(CameraParameters::SMILE_MODE, str);
                strcpy(mSmileMode, str);
            }else{
                mParameters.set(CameraParameters::SMILE_MODE, int_value);
            }
        }
        ALOGD("END setSmileMode : NO_ERROR" );
        return NO_ERROR;
    }
    ALOGD("END setSmileMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value );
    return BAD_VALUE;
}
status_t SemcCameraHardware::setFacedetectMode(const CameraParameters& params,
                                             const char *key /* = NULL */,
                                             const char *str_value /* = NULL */,
                                             int int_value /* = -1 */,
                                             bool collective /* = false */)
{
    ALOGD("START setFacedetectMode : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            //This root is Sinagature apl string string setting
            str = str_value;
        }
    }else{
        //This root is setParameters(pkg) and setCollective setting
        str = params.get(CameraParameters::FACE_MODE);
    }
    int32_t value = attr_lookup(face_mode, sizeof(face_mode) / sizeof(str_map), str, int_value);
    if(value != NOT_FOUND) {
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            if((strcmp(str, CAMERA_DEFAULT_VALUE) != 0)) {
                mParameters.set(CameraParameters::FACE_MODE, str);
                strcpy(mFacedetectMode, str);
            }else{
                mParameters.set(CameraParameters::FACE_MODE, int_value);
            }
        }
        ALOGD("END setFacedetectMode : NO_ERROR" );
        return NO_ERROR;
    }
    ALOGD("END setFacedetectMode : BAD_VALUE str[%s] int_value[%d]",str ,int_value );
    return BAD_VALUE;
}

#endif//SCRITCH_OFF
status_t SemcCameraHardware::setGpsLatitude(const CameraParameters& params,
                                                  const char *key /* = NULL */,
                                                  const char *str_value /* = NULL */,
                                                  int int_value /* = -1 */)
{
    ALOGD("START setGpsLatitude : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_LATITUDE, str_value);
        } else {
            ALOGW("END setGpsLatitude : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsLatitude : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsLatitude : NO_ERROR" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setGpsLatitudeRef(const CameraParameters& params,
                                                     const char *key /* = NULL */,
                                                     const char *str_value /* = NULL */,
                                                     int int_value /* = -1 */)
{
    ALOGD("START setGpsLatitudeRef : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_LATITUDE_REF, str_value);
        } else {
            ALOGW("END setGpsLatitudeRef : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsLatitudeRef : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsLatitudeRef : NO_ERROR" );
    return NO_ERROR;
}
status_t SemcCameraHardware::setGpsLongitude(const CameraParameters& params,
                                                   const char *key /* = NULL */,
                                                   const char *str_value /* = NULL */,
                                                   int int_value /* = -1 */)
{
    ALOGD("START setGpsLongitude : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_LONGITUDE, str_value);
        } else {
            ALOGW("END setGpsLongitude : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsLongitude : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsLongitude : NO_ERROR" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setGpsLongitudeRef(const CameraParameters& params,
                                                      const char *key /* = NULL */,
                                                      const char *str_value /* = NULL */,
                                                      int int_value /* = -1 */)
{
    ALOGD("START setGpsLongitudeRef : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_LONGITUDE_REF, str_value);
        } else {
            ALOGW("END setGpsLongitudeRef : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsLongitudeRef : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsLongitudeRef : NO_ERROR" );
    return NO_ERROR;
}
status_t SemcCameraHardware::setGpsAltitude(const CameraParameters& params,
                                                  const char *key /* = NULL */,
                                                  const char *str_value /* = NULL */,
                                                  int int_value /* = -1 */)
{
    ALOGD("START setGpsAltitude : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_ALTITUDE, str_value);
        } else {
            ALOGW("END setGpsAltitude : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsAltitude : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsAltitude : NO_ERROR" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setGpsAltitudeRef(const CameraParameters& params,
                                                     const char *key /* = NULL */,
                                                     const char *str_value /* = NULL */,
                                                     int int_value /* = -1 */)
{
    ALOGD("START setGpsAltitudeRef : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_ALTITUDE_REF, str_value);
        } else {
            ALOGW("END setGpsAltitudeRef : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsAltitudeRef : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsAltitudeRef : NO_ERROR" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setGpsStatus(const CameraParameters& params,
                                                const char *key /* = NULL */,
                                                const char *str_value /* = NULL */,
                                                int int_value /* = -1 */)
{
    ALOGD("START setGpsStatus : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::KEY_GPS_STATUS, str_value);
        } else {
            ALOGW("END setGpsStatus : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setGpsStatus : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setGpsStatus : NO_ERROR" );
    return NO_ERROR;
}

#if SCRITCH_OFF
status_t SemcCameraHardware::setFaceDetection(const CameraParameters& params,
                                                    const char *key /* = NULL */,
                                                    const char *str_value /* = NULL */,
                                                    int int_value /* = -1 */)
{
    ALOGD("START setFaceDetection : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );

    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::FACE_DETECTION, str_value);
        } else {
            ALOGW("END setFaceDetection : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setFaceDetection : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setFaceDetection : NO_ERROR" );
    return NO_ERROR;
}

status_t SemcCameraHardware::setFlashStatus(const CameraParameters& params,
                                                  const char *key /* = NULL */,
                                                  const char *str_value /* = NULL */,
                                                  int int_value /* = -1 */)

{
    ALOGD("START setFlashStatus : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value );
    if (key != NULL) {
        if (str_value != NULL) {
            /* This root is Sinagature apl string string setting */
            mParameters.set(CameraParameters::FLASH_STATUS, str_value);
        } else {
            ALOGW("END setFlashStatus : BAD_VALUE str_value is null");
            return BAD_VALUE;
        }
    } else {
        ALOGW("END setFlashStatus : BAD_VALUE key is null");
        return BAD_VALUE;
    }
    ALOGD("END setFlashStatus : NO_ERROR" );
    return NO_ERROR;
}
status_t SemcCameraHardware::setFlashlightBrightness(const CameraParameters& params,
                                                   const char *key /* = NULL */,
                                                   const char *str_value /* = NULL */,
                                                   int int_value /* = -1 */)
{
    ALOGD("START setFlashlightBrightness : key[%s] str_value[%s] int_value[%d]",key, str_value, int_value);
    int nwr = TALLY_VALUE_INIT, ret = TALLY_VALUE_INIT,
        fd = TALLY_VALUE_INIT, brightness = TALLY_VALUE_INIT;
    char value[CAMERA_STRING_VALUE_MAXLEN] = "";
    const char *str = CAMERA_DEFAULT_VALUE;

    if(key != NULL){
        if(str_value != NULL){
            /* This root is Sinagature apl string string setting */
            str = str_value;
        }
    }else{
        /* This root is setParameters(pkg) setting */
        str = params.get(CameraParameters::TALLY_LIGHT);
    }

    if((strcmp(str, CameraParameters::TALLY_LIGHT_OFF) == 0)){
        brightness = TALLY_VALUE_OFF;
    }else if((strcmp(str, CameraParameters::TALLY_LIGHT_FEEBLE) == 0)){
        brightness = TALLY_VALUE_FEEBLE;
    }else if((strcmp(str, CameraParameters::TALLY_LIGHT_STRONG) == 0)){
        brightness = TALLY_VALUE_STRONG;
    }else{
        ALOGE("END setFlashlightBrightness : BAD_VALUE str[%s] int_value[%d]",str ,int_value);
        return BAD_VALUE;
    }

    fd = open(FLASHLIGHT, O_RDWR);
    if(fd < 0){
        ALOGE("END setFlashlightBrightness : failed to open '%s', errno = %s", FLASHLIGHT, strerror(errno));
        return UNKNOWN_ERROR;
    }

    nwr = sprintf(value, "%d\n", brightness);
    ret = write(fd, value, nwr);
    close(fd);

    if(ret != nwr){
        ALOGE("END setFlashlightBrightness : write error");
        return UNKNOWN_ERROR;
    }
    mParameters.set(CameraParameters::TALLY_LIGHT, str);

    ALOGD("END setFlashlightBrightness : NO_ERROR");
    return NO_ERROR;
}
#endif//SCRITCH_OFF
status_t SemcCameraHardware::setJpegThumbnailSize(const CameraParameters& params)
{
    const char *str = NULL;
    str = params.get(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    if (str != NULL) {
        mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, str);
    }
    str = params.get(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
    if (str != NULL) {
        mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, str);
    }
    return NO_ERROR;
}

#if SCRITCH_OFF
/**
 * @brief SemcCameraHardware::setCollective
 * @return void
 *
 * @brief Set of a parameter is done to the device driver
 */
void SemcCameraHardware::setCollective()
{
    ALOGD("START setCollective");

    Mutex::Autolock l(&mLock);

    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
        setCollectiveSignature();
    }else{
        setCollective3rdParty();
    }

    ALOGD("END setCollective");
}

void SemcCameraHardware::setCollectiveSignature()
{
    ALOGD("START setCollectiveSignature");
    int value = 0;
    int ioctlRetVal = true;

    const char *str = NULL;
    str = mParameters.get(CameraParameters::SCENE_RECOGNITION);
    if(strcmp(str, CameraParameters::RECOGNITION_STOP) == 0) {
        //Scene setting
        if(setSceneMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
            ALOGE("setCollectiveSignature setSceneMode is error.");
        }else{
            strcpy(mKeyScene, CameraParameters::KEY_SCENE_MODE);
        }

        if(!mAfterTakepictureFlag) {
            //HandJitterReduction mode setting
            if(setHandJitterReduction(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
                ALOGE("setCollectiveSignature setHandJitterReduction is error.");
            }
        }else{
            ALOGV("setCollectiveSignature a set after the takepicture is skipped.");
        }

        //zoom setting
        if(native_step_zoom(mCameraControlFd, mZoom) == false){
            ALOGE("setCollectiveSignature native_step_zoom is error.");
        }

        //As for the case except the AUTO, WB and AutoExposure become the most suitable value the SceneMode
        if(strcmp(mSceneMode, CameraParameters::SCENE_MODE_AUTO) == 0) {
            //Whitebalance setting
            if(setWhiteBalance(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
                ALOGE("setCollectiveSignature setWhiteBalance is error.");
            }

            //Auto Exposure(exposure metering) settig
            if(setAutoExposure(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
                ALOGE("setCollectiveSignature setAutoExposure is error.");
            }
        }

        //Exposure Compensation setting
        if(setExposureCompensation(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
            ALOGE("setCollectiveSignature setExposureCompensation is error.");
        }

        value = 0;
        if(!mAfterTakepictureFlag){
            //Framerate setting : FrameRate setting becomes variable in the case of "0"
            if(setFramerate(mParameters, CameraParameters::KEY_PREVIEW_FRAME_RATE, NULL, value, true) != NO_ERROR) {
                ALOGE("setCollectiveSignature setFramerate is error.");
            }
        }else{
            ALOGV("setCollectiveSignature a set after the takepicture is skipped.");
        }
    }else{
        //Scene Recognition setting
        if(strcmp(str, CameraParameters::RECOGNITION_AUTO) != 0) {
            if(setSceneRecognition(mParameters, CameraParameters::SCENE_RECOGNITION, CameraParameters::RECOGNITION_AUTO, -1, true) != NO_ERROR) {
                ALOGE("END setCollectiveSignature : setSceneRecognition(RECOGNITION_AUTO) is error.");
                return;
            }
        }
        if(setSceneRecognition(mParameters, CameraParameters::SCENE_RECOGNITION, str, -1, true) != 0) {
            ALOGE("END setCollectiveSignature : setSceneRecognition is error.");
            return;
        }

        //zoom setting
        if(native_step_zoom(mCameraControlFd, mZoom) == false){
            ALOGE("setCollectiveSignature native_step_zoom is error.");
        }
    }

#if SCRITCH_OFF
    //AF Mode setting
    if(setAfMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollectiveSignature setAfMode is error.");
    }
    //CAF setting
    if(setCAFMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGD("setCollectiveSignature setCAFMode is error.");
    }

#endif //SCRITCH_OFF
    //Focus Mode setting
    if(setFocusMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollectiveSignature setFocusMode is error.");
    }

    if( MDP_CCS_YUV2RGB == mdp_ccs_save.direction ){
        if((CAMERA_SIGNATURE_HD720P_DISPSIZE_WIDTH == mDimension.display_width) && (CAMERA_SIGNATURE_HD720P_DISPSIZE_HEIGHT == mDimension.display_height)) {
            if((ioctlRetVal = ioctl(fb_fd, MSMFB_SET_CCS_MATRIX, &mdp_ccs_hd)) < 0) {
                ALOGE("setCollectiveSignature MSMFB_SET_CCS_MATRIX(720P): ioctl failed. ioctl return value is %d.", ioctlRetVal);
            }
        } else {
            if((ioctlRetVal = ioctl(fb_fd, MSMFB_SET_CCS_MATRIX, &mdp_ccs_save)) < 0) {
                ALOGE("setCollectiveSignature MSMFB_SET_CCS_MATRIX: ioctl failed. ioctl return value is %d.", ioctlRetVal);
            }
        }
    }

    ALOGD("END setCollectiveSignature");
}
#endif//SCRITCH_OFF//beforeSetCollective

void SemcCameraHardware::setCollective3rdParty()
{
    ALOGD("START setCollective3rdParty");
    int ioctlRetVal = true;

    /* Scene setting */
    //BEN QRCODE
    if(setSceneMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setSceneMode is error.");
    }

    /* Antibanding setting */
    if(setAntibanding(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setAntibanding is error.");
    }

    /* Whitebalance setting */
    if(setWhiteBalance(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setWhiteBalance is error.");
    }

    /* Effect setting */
    if(setEffect(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setEffect is error.");
    }

    /* Focus Mode setting */
    if(setFocusMode(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setFocusMode is error.");
    }

    /* Framerate setting */
    if(setFramerate(mParameters, NULL, NULL, -1, true) != NO_ERROR) {
        ALOGE("setCollective3rdParty setFramerate is error.");
    }

    if( MDP_CCS_YUV2RGB == mdp_ccs_save.direction ){
        if((CAMERA_3RDPARTY_HD720P_DISPSIZE_WIDTH == mDimension.display_width) && (CAMERA_3RDPARTY_HD720P_DISPSIZE_HEIGHT == mDimension.display_height)) {
            if((ioctlRetVal = ioctl(fb_fd, MSMFB_SET_CCS_MATRIX, &mdp_ccs_hd)) < 0) {
                ALOGE("setCollective3rdParty MSMFB_SET_CCS_MATRIX(720P): ioctl failed. ioctl return value is %d.", ioctlRetVal);
            }
        } else {
            if((ioctlRetVal = ioctl(fb_fd, MSMFB_SET_CCS_MATRIX, &mdp_ccs_save)) < 0) {
                ALOGE("setCollective3rdParty MSMFB_SET_CCS_MATRIX: ioctl failed. ioctl return value is %d.", ioctlRetVal);
            }
        }
    }

    ALOGD("END setCollective3rdParty");
}

/**
 * @brief SemcCameraHardware::picsizeCheck
 * @param width [IN] Preview Size(width)
 * @param height [IN] Preview Size(height)
 * @return bool
 * @n      false : Parameter error
 * @n      true : SUCCESS
 *
 * @brief The check of the size set up from the camera service is done
 */
bool SemcCameraHardware::picsizeCheck(int width ,int height)
{
    ALOGD("START picsizeCheck : width = %d ,height = %d"
                                                 ,width ,height);

    if(((width == CAMERA_QVGASIZE_WIDTH)&&(height == CAMERA_QVGASIZE_HEIGHT))||
      ((width == CAMERA_QVGASIZE_HEIGHT)&&(height == CAMERA_QVGASIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_QVGA;
    }
    else if(((width == CAMERA_VGASIZE_WIDTH)&&(height == CAMERA_VGASIZE_HEIGHT))||
           ((width == CAMERA_VGASIZE_HEIGHT)&&(height == CAMERA_VGASIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_VGA;
    }
    else if(((width == CAMERA_WVGASIZE_WIDTH)&&(height == CAMERA_WVGASIZE_HEIGHT))||
           ((width == CAMERA_WVGASIZE_HEIGHT)&&(height == CAMERA_WVGASIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_WVGA;
    }
    else if(((width == CAMERA_FWVGASIZE_WIDTH)&&(height == CAMERA_FWVGASIZE_HEIGHT))||
           ((width == CAMERA_FWVGASIZE_HEIGHT)&&(height == CAMERA_FWVGASIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_FWVGA;
    }
    else if(((width == CAMERA_2MSIZE_WIDTH)&&(height == CAMERA_2MSIZE_HEIGHT))||
           ((width == CAMERA_2MSIZE_HEIGHT)&&(height == CAMERA_2MSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_2M;
    }
    else if(((width == CAMERA_FULLHDSIZE_WIDTH)&&(height == CAMERA_FULLHDSIZE_HEIGHT))||
           ((width == CAMERA_FULLHDSIZE_HEIGHT)&&(height == CAMERA_FULLHDSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_FULLHD;
    }
    else if(((width == CAMERA_3MSIZE_WIDTH)&&(height == CAMERA_3MSIZE_HEIGHT))||
           ((width == CAMERA_3MSIZE_HEIGHT)&&(height == CAMERA_3MSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_3M;
    }
    else if(((width == CAMERA_4MWIDESIZE_WIDTH)&&(height == CAMERA_4MWIDESIZE_HEIGHT))||
           ((width == CAMERA_4MWIDESIZE_HEIGHT)&&(height == CAMERA_4MWIDESIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_4MWIDE;
    }
    else if(((width == CAMERA_5MSIZE_WIDTH)&&(height == CAMERA_5MSIZE_HEIGHT))||
           ((width == CAMERA_5MSIZE_HEIGHT)&&(height == CAMERA_5MSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_5M;
    }
    else if(((width == CAMERA_6MSIZE_WIDTH)&&(height == CAMERA_6MSIZE_HEIGHT))||
           ((width == CAMERA_6MSIZE_HEIGHT)&&(height == CAMERA_6MSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_6M;
    }
    else if(((width == CAMERA_8MSIZE_WIDTH)&&(height == CAMERA_8MSIZE_HEIGHT))||
           ((width == CAMERA_8MSIZE_HEIGHT)&&(height == CAMERA_8MSIZE_WIDTH)))
    {
        mCameraPicsize = CAMERAHAL_PICSIZE_8M;
    }
    else
    {
        ALOGW("picsizeCheck : mCameraPicsize Error");
        mCameraPicsize = CAMERAHAL_PICSIZE_NOTSUPPORT;
    }

    switch(mCameraPicsize)
    {
    case CAMERAHAL_PICSIZE_QVGA:
        mCameraPreviewsize = CAMERAHAL_PREVIEWSIZE_QVGA;
        break;
    case CAMERAHAL_PICSIZE_VGA:
    case CAMERAHAL_PICSIZE_2M:
    case CAMERAHAL_PICSIZE_3M:
    case CAMERAHAL_PICSIZE_5M:
    case CAMERAHAL_PICSIZE_8M:
        mCameraPreviewsize = CAMERAHAL_PREVIEWSIZE_VGA;
        break;

    case CAMERAHAL_PICSIZE_FWVGA:
    case CAMERAHAL_PICSIZE_FULLHD:
    case CAMERAHAL_PICSIZE_4MWIDE:
    case CAMERAHAL_PICSIZE_6M:
        mCameraPreviewsize = CAMERAHAL_PREVIEWSIZE_FWVGA;
        break;

    case CAMERAHAL_PICSIZE_WVGA:
        mCameraPreviewsize = CAMERAHAL_PREVIEWSIZE_WVGA;
        break;
    default:
        ALOGW("picsizeCheck : mCameraPreviewsize Error");
        mCameraPreviewsize = CAMERAHAL_PREVIEWSIZE_NOTSUPPORT;
        return false;

    }
    if((CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize) ||
       (CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize) ||
       (CAMERAHAL_PICSIZE_6M == mCameraPicsize))
    {
            mThumbnailWidth = CAMERA_WQVGASIZE_WIDTH;
            mThumbnailHeigth = CAMERA_WQVGASIZE_HEIGHT;
    }else{
            mThumbnailWidth = CAMERA_QVGASIZE_WIDTH;
            mThumbnailHeigth = CAMERA_QVGASIZE_HEIGHT;
    }

    ALOGD("END picsizeCheck");
    return true;
}
/**
 * @brief SemcCameraHardware::setParameters
 * @param &params [OUT]  Object of Parameters class
 * @param *key    [IN]   Parameter's name
 * @param value   [IN]   Parameter's value
 * @
 * @return status_t
 * @n      NO_ERROR      : SUCCESS or Parameter error
 * @brief  Set the camera parameters(Separate setting)
 */
status_t SemcCameraHardware::setParameters(CameraParameters &params, const char *key, int value)
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        Mutex::Autolock l(&mLock);
        ALOGD("START setParameters(params, (char*)key, (int*)value) : params = %p, key = %s, value = %d", &params, key, value);
#if SCRITCH_OFF
        if((strcmp(key, CameraParameters::SCENE_RECOGNITION) != 0)) {
            if((CAMERAHAL_STATE_AFSTART  == mStateCameraHal) ||
               (CAMERAHAL_STATE_AFCANCEL == mStateCameraHal))
            {
                ALOGW("END setParameters(Separate Setting) : mStateCameraHal Status Error, mStateCameraHal = %d", mStateCameraHal);
                return NO_ERROR;
            }
            if(CAMERAHAL_STATE_AFLOCK == mStateCameraHal){
                if((strcmp(mKeyScene, CameraParameters::SCENE_RECOGNITION) == 0) &&
                   (strcmp(key, CameraParameters::KEY_FOCUS_MODE) == 0)){
                        //code nothing
                }else{
                    ALOGE("END setParameters(Separate Setting) : mStateCameraHal Status is CAMERAHAL_STATE_AFLOCK, mStateCameraHal = %d", mStateCameraHal);
                    return NO_ERROR;
                }
            }
        }
 #endif//SCRITCH_OFF
        //SceneMode
        if(strcmp(key, CameraParameters::KEY_SCENE_MODE) == 0) {
            //BEN QRCODE
            if(setSceneMode(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setSceneMode is error.");
                return NO_ERROR;
            }
            strcpy(mKeyScene, CameraParameters::KEY_SCENE_MODE);
            ALOGV("setParameters(Separate setting) : mSceneMode = %s, mKeyScene = %s", mSceneMode, mKeyScene);
        }
        //WhiteBalance
        else
         if(strcmp(key, CameraParameters::KEY_WHITE_BALANCE) == 0){
            if(setWhiteBalance(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setWhiteBalance is error.");
                return NO_ERROR;
            }
        }

#if SCRITCH_OFF
        //ExposureCompensation
        else if(strcmp(key, CameraParameters::EXPOSURE_COMPENSATION) == 0){
            if(setExposureCompensation(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setExposureCompensation is error.");
                return NO_ERROR;
            }
        }


        //AutoExposure(ExposureMetering)
        else if(strcmp(key, CameraParameters::KEY_AUTO_EXPOSURE) == 0){
            if(setAutoExposure(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setAutoExposure is error.");
                return NO_ERROR;
            }
        }
        //AutoFocusMode
        else if(strcmp(key, CameraParameters::AF_MODE) == 0){
            if(setAfMode(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setAfMode is error.");
                return NO_ERROR;
            }
        }
        //HandJitterReduction
        else if(strcmp(key, CameraParameters::HJR_MODE) == 0){
            if(setHandJitterReduction(params, key, NULL, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setHandJitterReduction is error.");
                return NO_ERROR;
            }
        }
        //Orientation
        else if(strcmp(key, CameraParameters::ORIENTATION) == 0){
            if(false == previewEnabled())
            {
                if(setOrientation(params) != NO_ERROR) {
                    ALOGW("END setParameters(Separate Setting) : setOrientation error.");
                    return NO_ERROR;
                }
            }else{
                ALOGW("END setParameters(Separate Setting) : status error, mStateCameraHal = %d",mStateCameraHal);
            }
            return NO_ERROR;
        }

        //SmileMode
        else if(strcmp(key, CameraParameters::SMILE_MODE) == 0){
            if(setSmileMode(params, key, NULL, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setSmileMode  error.");
                return NO_ERROR;
            }
        }
        //FacedetectMode
        else if(strcmp(key, CameraParameters::FACE_MODE) == 0){
            if(setFacedetectMode(params, key, NULL, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setFacedetectMode  error.");
                return NO_ERROR;
            }
        }


#endif//SCRITCH_OFF
        //FocusMode(AutoFocusRange)
        else if(strcmp(key, CameraParameters::KEY_FOCUS_MODE) == 0){
            if(setFocusMode(params, key, NULL, value) != NO_ERROR) {
                ALOGD("END setParameters(Separate Setting) : setFocusMode is error.");
                return NO_ERROR;
            }
        }
#if SCRITCH_OFF
        //Configuration
        else if(strcmp(key, CameraParameters::CONFIGURATION) == 0){
            if(false == previewEnabled()){
                mConfigurationFlag = true;
                ALOGD("END setParameters(Separate Setting) : mConfigurationFlag[%d]", mConfigurationFlag);
            }else{
                ALOGD("END setParameters(Separate Setting) : status error, mStateCameraHal = %d",mStateCameraHal);
            }
            return NO_ERROR;
        }
#endif//SCRITCH_OFF

        //JPEG quality
        else if(strcmp(key, CameraParameters::KEY_JPEG_QUALITY) == 0){
            if(setJpegQuality(params, key, NULL, value) != NO_ERROR) {
                ALOGD("END setParameters(Separate Setting) : setJpegQuality  error.");
                return NO_ERROR;
            }
        }
        else if (strcmp(key, CameraParameters::KEY_ZOOM) == 0){
            if(setZoom(params, key, NULL, value) != NO_ERROR){
                ALOGD("%s Failed to set zoom", __FUNCTION__);
                return NO_ERROR;
            }
        }

#if SCRITCH_OFF
        //CAF
        else if(strcmp(key, CameraParameters::CAF_MODE) == 0){
            if(setCAFMode(params, key, NULL, value) != NO_ERROR) {
                ALOGD("END setParameters(Separate Setting) : setCAFMode  error.");
                return NO_ERROR;
            }
        }
#endif//SCRITCH_OFF
        /* ROTATION */
        else if(strcmp(key, CameraParameters::KEY_ROTATION) == 0){
            if(setRotation(params, key, NULL, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setRotation  error.");
                return NO_ERROR;
            }
        }


        else{
            //Not anywhere
            ALOGW("END setParameters(Separate Setting) :  error : params = %p, key = %s, value = %d", &params, key, value);
            return NO_ERROR;
        }
        ALOGD("END setParameters()");
    }
    return NO_ERROR;

}
/**
 * @brief SemcCameraHardware::setParameters(Separete:String type)
 * @param &params [OUT]  Object of Parameters class
 * @param *key    [IN]   Parameter's name
 * @param value   [IN]   Parameter's value
 * @
 * @return status_t
 * @n      NO_ERROR      : SUCCESS or Parameter error
 * @brief  Set the camera parameters(Separate setting)
 */
status_t SemcCameraHardware::setParameters(CameraParameters &params, const char *key, const char *value)
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        Mutex::Autolock l(&mLock);
        ALOGD("START setParameters(params, (char*)key, (char*)value) : params = %p, key = %s, value = %s", &params, key, value);

#if SCRITCH_OFF
        if((strcmp(key, CameraParameters::SCENE_RECOGNITION) != 0) && (strcmp(key, CameraParameters::TALLY_LIGHT) != 0)) {
            if((CAMERAHAL_STATE_AFSTART  == mStateCameraHal) ||
               (CAMERAHAL_STATE_AFCANCEL == mStateCameraHal))
            {
                ALOGW("END setParameters(Separate Setting) : mStateCameraHal Status Error, mStateCameraHal = %d", mStateCameraHal);
                return NO_ERROR;
            }
            if(CAMERAHAL_STATE_AFLOCK == mStateCameraHal){
                if((strcmp(mKeyScene, CameraParameters::SCENE_RECOGNITION) == 0) &&
                   (strcmp(key, CameraParameters::KEY_FOCUS_MODE) == 0)){
                        //code nothing
                }else{
                    ALOGE("END setParameters(Separate Setting) : mStateCameraHal Status is CAMERAHAL_STATE_AFLOCK, mStateCameraHal = %d", mStateCameraHal);
                    return NO_ERROR;
                }
            }
        }

        //SceneMode
        if(strcmp(key, CameraParameters::KEY_SCENE_MODE) == 0) {
            //compare setting value and changing value
            //BEN QRCODE
            if(setSceneMode(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setSceneMode is error.");
                return NO_ERROR;
            }
            ALOGV("setParameters(Separate setting) : mSceneMode = %s, mKeyScene = %s", mSceneMode, mKeyScene);
        }
        //SceneRecognition
        else if(strcmp(key, CameraParameters::SCENE_RECOGNITION) == 0){
            //compare setting value and changing value
            if(setSceneRecognition(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setSceneRecognition is error.");
                return NO_ERROR;
            }
            strcpy(mKeyScene, CameraParameters::SCENE_RECOGNITION);
            ALOGV("setParameters(Separate setting) : mSceneMode = %s, mKeyScene = %s", mSceneMode, mKeyScene);

            char afMode_value[CAMERA_STRING_VALUE_MAXLEN];
            strcpy(afMode_value,CameraParameters::SINGLE);
            if ( (strcmp(mSceneMode, CameraParameters::RECOGNITION_PORTRAIT) == 0)  ||
                 (strcmp(mSceneMode, CameraParameters::RECOGNITION_NIGHTPORTRAIT) == 0)  ||
                 (strcmp(mSceneMode, CameraParameters::RECOGNITION_AGAINSTPORTRAIT) == 0) ) {
                strcpy(afMode_value,CameraParameters::AREA);
            }
            if(setAfMode(params, CameraParameters::AF_MODE, afMode_value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setAfMode is error.");
                return NO_ERROR;
            }
        }

        //WhiteBalance
        else
#endif//SCRITCH_OFF
         if(strcmp(key, CameraParameters::KEY_WHITE_BALANCE) == 0){

            if(setWhiteBalance(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setWhiteBalance is error.");
                return NO_ERROR;
            }
        }
#if SCRITCH_OFF
        //ExposureCompensation
        else if(strcmp(key, CameraParameters::EXPOSURE_COMPENSATION) == 0){
            if(setExposureCompensation(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setExposureCompensation is error.");
                return NO_ERROR;
            }
        }
#endif//SCRITCH_OFF

        //AutoExposure(ExposureMetering)
        else if(strcmp(key, CameraParameters::KEY_AUTO_EXPOSURE) == 0){
            if(setAutoExposure(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setAutoExposure is error.");
                return NO_ERROR;
            }
        }
#if SCRITCH_OFF
        //AutoFocusMode
        else if(strcmp(key, CameraParameters::AF_MODE) == 0){
            if(setAfMode(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setAfMode is error.");
                return NO_ERROR;
            }
        }

        //HandJitterReduction
        else if(strcmp(key, CameraParameters::HJR_MODE) == 0){
            if(setHandJitterReduction(params, key, value) != NO_ERROR) {
                ALOGE("END setParameters(Separate Setting) : setHandJitterReduction is error.");
                return NO_ERROR;
            }
        }
        //Orientation
        else if(strcmp(key, CameraParameters::ORIENTATION) == 0){
            if(false == previewEnabled()) {
                if(setOrientation(params) != NO_ERROR) {
                    ALOGW("END setParameters(Separate Setting) : setOrientation error.");
                    return NO_ERROR;
                }
            }else{
                ALOGW("END setParameters(Separate Setting) : status error, mStateCameraHal = %d",mStateCameraHal);
            }
            return NO_ERROR;
        }

        //SmileMode
        else if(strcmp(key, CameraParameters::SMILE_MODE) == 0){
            if(setSmileMode(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setSmileMode  error.");
                return NO_ERROR;
            }
        }
        //FacedetectMode
        else if(strcmp(key, CameraParameters::FACE_MODE) == 0){
            if(setFacedetectMode(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setFacedetectMode  error.");
                return NO_ERROR;
            }
        }


#endif //SCRITCH_OFF
        //FocusMode(AutoFocusRange)
        else if(strcmp(key, CameraParameters::KEY_FOCUS_MODE) == 0){
            if(setFocusMode(params, key, value) != NO_ERROR) {
                ALOGD("END setParameters(Separate Setting) : setFocusMode is error.");
                return NO_ERROR;
            }
        }
#if SCRITCH_OFF
        //Configuration
        else if(strcmp(key, CameraParameters::CONFIGURATION) == 0){
            if(false == previewEnabled()){
                mConfigurationFlag = true;
                ALOGD("END setParameters(Separate Setting) : mConfigurationFlag[%d]", mConfigurationFlag);
            }else{
                ALOGD("END setParameters(Separate Setting) : status error, mStateCameraHal = %d",mStateCameraHal);
            }
            return NO_ERROR;
        }
#endif//SCRITCH_OFF
        //JPEG quality
        else if(strcmp(key, CameraParameters::KEY_JPEG_QUALITY) == 0){
            ALOGD("END setParameters(Separate Setting) : JPEG quality  error.");
            return NO_ERROR;
        }

#if SCRITCH_OFF
        //CAF
        else if(strcmp(key, CameraParameters::CAF_MODE) == 0){
            if(setCAFMode(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setCAFMode  error.");
                return NO_ERROR;
            }
        }
#endif//SCRITCH_OFF

        /* GPS LATITUDE */
        else if(strcmp(key, CameraParameters::KEY_GPS_LATITUDE) == 0){
            if(setGpsLatitude(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsLatitude  error.");
                return NO_ERROR;
            }
        }

        /* GPS LATITUDE REF */
        else if(strcmp(key, CameraParameters::KEY_GPS_LATITUDE_REF) == 0){
            if(setGpsLatitudeRef(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsLatitudeRef  error.");
                return NO_ERROR;
            }
        }

        /* GPS LONGITUDE */
        else if(strcmp(key, CameraParameters::KEY_GPS_LONGITUDE) == 0){
            if(setGpsLongitude(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsLongitude  error.");
                return NO_ERROR;
            }
        }

        /* GPS LONGITUDE REF */
        else if(strcmp(key, CameraParameters::KEY_GPS_LONGITUDE_REF) == 0){
            if(setGpsLongitudeRef(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsLongitudeRef  error.");
                return NO_ERROR;
            }
        }

        /* GPS ALTITUDE */
        else if(strcmp(key, CameraParameters::KEY_GPS_ALTITUDE) == 0){
            if(setGpsAltitude(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsAltitude  error.");
                return NO_ERROR;
            }
        }

        /* GPS ALTITUDE REF */
        else if(strcmp(key, CameraParameters::KEY_GPS_ALTITUDE_REF) == 0){
            if(setGpsAltitudeRef(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsAltitudeRef  error.");
                return NO_ERROR;
            }
        }

        /* GPS STATUS */
        else if(strcmp(key, CameraParameters::KEY_GPS_STATUS) == 0){
            if(setGpsStatus(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setGpsStatus  error.");
                return NO_ERROR;
            }
        }
#if SCRITCH_OFF
        /* FACE DETECTION */
        else if(strcmp(key, CameraParameters::FACE_DETECTION) == 0){
            if(setFaceDetection(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setFaceDetection  error.");
                return NO_ERROR;
            }
        }
        /* FLASH STATUS */
        else if(strcmp(key, CameraParameters::FLASH_STATUS) == 0){
            if(setFlashStatus(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setFlashStatus  error.");
                return NO_ERROR;
            }
        }
        //TALLY LIGHT
        else if(strcmp(key, CameraParameters::TALLY_LIGHT) == 0){
            if(setFlashlightBrightness(params, key, value) != NO_ERROR) {
                ALOGW("END setParameters(Separate Setting) : setFlashlightBrightness  error.");
                return NO_ERROR;
            }
        }
#endif//SCRITCH_OFF
        else{
            //Not anywhere
            ALOGW("END setParameters(Separate Setting) :  error : params = %p, key = %s, value = %s", &params, key, value);
            return NO_ERROR;
        }
        ALOGD("END setParameters()");
    }
    return NO_ERROR;
}
/**
 * @brief Preview size setting
 * @param &params [OUT] Object of Parameters class
 * @param width [IN] Preview size width
 * @param height [IN] Preview size height
 * @return status_t
 * @n      NO_ERROR      : SUCCESS
 *
 * @brief Preview size setting.
 */
status_t SemcCameraHardware::setPreviewSize(CameraParameters &params, int width, int height)
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        ALOGD("START setPreviewSize : width[%d]height[%d]",width,height);
        Mutex::Autolock l(&mLock);

        for (size_t i = 0; i < previewSizeCount; ++i) {
            if (width >= supportedPreviewSizes[i].width && height >= supportedPreviewSizes[i].height) {
                 mParameters.setPreviewSize(supportedPreviewSizes[i].width, supportedPreviewSizes[i].height);
                // 720p , preview can be 768X432 (currently for 7x30 and 8k
                // targets)
                if(width == 1280 && height == 720 &&
                 ((mCurrentTarget == TARGET_MSM7630) || (mCurrentTarget == TARGET_QSD8250))){
                    ALOGD("change preview resolution to 768X432 since recording is in 720p");
                    mDimension.display_width = 768;
                    mDimension.display_height= 432;
                }else {
                    mDimension.display_width = supportedPreviewSizes[i].width;
                    mDimension.display_height= supportedPreviewSizes[i].height;
                }

                if(((CAMERA_FWVGASIZE_WIDTH == mDimension.display_width) && (CAMERA_FWVGASIZE_HEIGHT == mDimension.display_height)) ||
                   ((CAMERA_OHDFWVGASIZE_WIDTH == mDimension.display_width) && (CAMERA_OHDFWVGASIZE_HEIGHT == mDimension.display_height))){
                    mDimension.display_width = CAMERA_FWVGASIZE_DRV_WIDTH;
                }
                ALOGE("END setPreviewSize : ");
                return NO_ERROR;
            }
        }
        ALOGD("END setPreviewSize : BAD_VALUE");
        return BAD_VALUE;
    }
    return NO_ERROR;
}

/**
 * @brief Picture size setting
 * @param &params [OUT] Object of Parameters class
 * @param width [IN] Picture size width
 * @param height [IN] Picture size height
 * @return status_t
 * @n      NO_ERROR      : SUCCESS
 *
 * @brief Picture size setting.
 */
status_t SemcCameraHardware::setPictureSize(CameraParameters &params, int width, int height)
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        ALOGD("START setPictureSize width[%d]height[%d]",width,height);
        Mutex::Autolock l(&mLock);
        // Validate the picture size
        for (int i = 0; i < PICTURE_SIZE_COUNT_SEMC; ++i) {
            if (width >= picture_sizes[i].width && height >= picture_sizes[i].height) {
                mParameters.setPictureSize(picture_sizes[i].width, picture_sizes[i].height);
                mDimension.picture_width = picture_sizes[i].width;
                mDimension.picture_height = picture_sizes[i].height;
                ALOGD("END setPictureSize :");
                return NO_ERROR;
            }
        }
        ALOGE("END setPictureSize : BAD_VALUE");
        return BAD_VALUE;
    }
    return NO_ERROR;
}
#if SCRITCH_OFF
/**
 * @brief SemcCameraHardware::startAutoFocus
 * @param ae_lock    [IN] AE lock information
 * @param awb_lock   [IN] AWB lock information
 * @param focus_lock [IN] Focus lock information
 * @return status_t
 * @n      NO_ERROR : SUCCESS
 * @n      UNKNOWN_ERROR : Parameter error
 *
 * @brief The inside function to do an auto-focus or a start requirement to the device driver
 */
status_t SemcCameraHardware::startAutoFocus(bool ae_lock, bool awb_lock, bool focus_lock)
{
    ALOGD("START startAutoFocus : mStateCameraHal = %d ", mStateCameraHal);
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0){
        ALOGD("END startAutoFocus : mSemcCameraFlag is 3rdParty");
        return NO_ERROR;
    }
    Mutex::Autolock l(&mLock);
    if(CAMERAHAL_STATE_INIT == mStateCameraHal){
        ALOGW("END startAutoFocus : mStateCameraHal is INIT");
        return NO_ERROR;
    }else if((CAMERAHAL_STATE_PREVIEWSTART != mStateCameraHal) && (CAMERAHAL_STATE_AFLOCK != mStateCameraHal)){
        ALOGW("END startAutoFocus : AutoFocus status Error");
        return UNKNOWN_ERROR;
    }

    CameraHalState oldState = mStateCameraHal;
    mStateCameraHal = CAMERAHAL_STATE_AFSTART;
    mFocusLockValue = focus_lock;
    mFocusPosition = CAMERAHAL_LENS_POSITION_NORMAL;

    if (native_start_autofocus(mCameraControlFd, ae_lock, awb_lock, focus_lock) == false) {
        ALOGE("END startAutoFocus : main:%d start_preview failed!\n", __LINE__);
        mStateCameraHal = oldState;
        return UNKNOWN_ERROR;
    }
    ALOGD("END startAutoFocus");
    return NO_ERROR;
}

/**
 * @brief SemcCameraHardware::converter_afresult
 * @param af_result [IN] autofocus result
 * @param afstate_result [IN] internal autofocus result
 * @param autoFocusEnabled [IN] 3rdParty AF callback enable flag
 * @param startAFEnabled [IN] Signature AF callback enable flag
 * @
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : Parameter error
 * @brief convert Android defined value into Driver defined value.(autofocus result)
 */
bool SemcCameraHardware::converter_afresult(struct msm_af_results_t * af_result, struct af_result_t *hal_afstate_result, bool autoFocusEnabled, bool startAFEnabled)
{
    ALOGD("START converter_afresult : af_result [%d] autoFocusEnabled[%d] startAFEnabled[%d]", af_result->msm_af_result, autoFocusEnabled, startAFEnabled);
    bool rc = true;

    switch(af_result->msm_af_result)
    {
      case MSM_AF_SUCCESS:
        hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_STATE_SUCCESS;
        if(true == autoFocusEnabled && false == startAFEnabled)
        {
            // case 3rdparty
            mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
        }else{
            mStateCameraHal = CAMERAHAL_STATE_AFLOCK;
        }
        break;

      case MSM_AF_FAILED:
        hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_STATE_FAILURE;
        if(true == autoFocusEnabled && false == startAFEnabled)
        {
            // case 3rdparty
            mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
        }else{
            mStateCameraHal = CAMERAHAL_STATE_AFLOCK;
        }
        break;

      case MSM_AF_FORCE_STOP:
        hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_STATE_CANCEL;
        mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
        break;

      case MSM_AF_ERROR:
      default:
        ALOGW("converter_afresult is error");
        if(CAMERAHAL_STATE_AFCANCEL == mStateCameraHal)
        {
            hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_STATE_CANCEL;
            mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
        }else{
            if(true == autoFocusEnabled && false == startAFEnabled)
            {
                // case 3rdparty
                hal_afstate_result->afstate_result = 0xff;
                mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;
            }else{
                hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_STATE_FAILURE;
                mStateCameraHal = CAMERAHAL_STATE_AFLOCK;
            }
        }
        rc = false;
        break;
    }
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0){
        if(hal_afstate_result->afstate_result == CAMERA_AUTOFOCUS_STATE_FAILURE){
            switch(af_result->msm_af_failure_info){
              case MSM_AF_FAILURE_NONE:
                  hal_afstate_result->af_failurestate_info = CAMERA_AUTOFOCUS_FAILURE_NONE;
              break;

              case MSM_AF_FAILURE_NOTFOUND:
                  hal_afstate_result->af_failurestate_info = CAMERA_AUTOFOCUS_FAILURE_POINT_NOTFOUND;
              break;

              case MSM_AF_FAILURE_TOO_CLOSE:
                  hal_afstate_result->af_failurestate_info = CAMERA_AUTOFOCUS_FAILURE_TOO_CLOSE;
              break;

              case MSM_AF_FAILURE_TOO_FAR:
                  hal_afstate_result->af_failurestate_info = CAMERA_AUTOFOCUS_FAILURE_TOO_FAR;
              break;

              default:
                ALOGW("failure_info is error");
                hal_afstate_result->afstate_result = CAMERA_AUTOFOCUS_FAILURE_NONE;
                rc = false;
              break;
            }
        }
    }
    ALOGD("END converter_afresult : afstate_result = %d , af_failurestate_info = %d.", hal_afstate_result->afstate_result, hal_afstate_result->af_failurestate_info);
    return rc;
}
#endif//SCRITCH_OFF
/**
 * @brief SemcCameraHardware::startrawsnapshot
 * @return retCbErrType
 * @n      RET_NO_ERROR  : SUCCESS
 * @n      RET_FAIL_YUV_CB : YUV callback failed
 * @n      RET_FAIL_JPEG_CB : JPEG callback failed
 *
 * @brief The function to start rawsnapshot
 */
retCbErrType SemcCameraHardware::startrawsnapshot()
{
    ALOGD("START startrawsnapshot");
    size_t i = 0;
    size_t size_num = 0;
    int quality = 0;
    jpeg_quality_t *qsize = NULL;

    //unregist the driver's memory(raw/thubmnail).
    native_unregister_snapshot_bufs();

    quality = mParameters.getInt(CameraParameters::KEY_JPEG_QUALITY);

    if((quality >= CAMERAHAL_JPEGQUALITY_ECONOMY_MIN) && (quality <= CAMERAHAL_JPEGQUALITY_ECONOMY_MAX)){
        qsize = jpeg_quality_sizes_economy;
        size_num = sizeof(jpeg_quality_sizes_economy) / sizeof(jpeg_quality_sizes_economy[0]);
    }else if((quality >= CAMERAHAL_JPEGQUALITY_NORMAL_MIN) && (quality <= CAMERAHAL_JPEGQUALITY_NORMAL_MAX)){
        qsize = jpeg_quality_sizes_normal;
        size_num = sizeof(jpeg_quality_sizes_normal) / sizeof(jpeg_quality_sizes_normal[0]);
    }else if((quality >= CAMERAHAL_JPEGQUALITY_FINE_MIN) && (quality <= CAMERAHAL_JPEGQUALITY_FINE_MAX)){
        qsize = jpeg_quality_sizes_fine;
        size_num = sizeof(jpeg_quality_sizes_fine) / sizeof(jpeg_quality_sizes_fine[0]);
    }else{
        ALOGE("END startrawsnapshot : invalid jpeg quality = %d", quality);
        return RET_FAIL_YUV_CB;
    }

    for(i = 0; i < size_num; i++){
        if(mCameraPicsize == qsize[i].picsize){
            break;
        }
    }

    if(size_num == i){
        ALOGE("END startrawsnapshot : quality check failed! mCameraPicsize = %d", mCameraPicsize);
        return RET_FAIL_YUV_CB;
    }
    usleep(10*1000);    //I add a cord temporarily for taking picture trouble.

    if(native_set_jpegquality(mCameraControlFd,&qsize[i]) == false) {
        ALOGE("END startrawsnapshot : native_set_jpegquality  is error");
        return RET_FAIL_YUV_CB;
    }

    struct capture_setting_t hal_capture_setting;
    memset(&hal_capture_setting, 0, sizeof(capture_setting_t));

    if(native_get_capture_setting(mCameraControlFd,&hal_capture_setting) == false) {

        ALOGE("END startrawsnapshot : native_get_capture_setting  is error");
        return RET_FAIL_YUV_CB;
    }
    mJpegSnapShotHeap =
        new PmemPool("/dev/pmem_adsp",
                     MemoryHeapBase::READ_ONLY,
                     mCameraControlFd,
                     MSM_PMEM_RAW_MAINIMG,
                     //mDimension.raw_picture_height * mDimension.raw_picture_height,
                     (int)hal_capture_setting.jpeg_main_img_buf_length + mASSBufferSize,
                     1,
                     (int)hal_capture_setting.jpeg_main_img_buf_length + mASSBufferSize,
                     //mDimension.raw_picture_height * mDimension.raw_picture_height,
                     "Jpeg snapshot camera");

    if (!mJpegSnapShotHeap->initialized()) {
        mJpegSnapShotHeap.clear();
        ALOGE("END startrawsnapshot : error initializing mJpegSnapShotHeap");
        return RET_FAIL_YUV_CB;
    }

    if(native_start_raw_snapshot(mCameraControlFd) == false) {
        ALOGE("END startrawsnapshot : native_start_raw_snapshot  is error");
        return RET_FAIL_YUV_CB;
    }

    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) ) {
        if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
            if(native_sync_raw_snapshot_interval(mCameraControlFd) == false) {
                ALOGE("END startrawsnapshot : native_sync_raw_snapshot_interval  is error");
                return RET_FAIL_YUV_CB;
            } else {
                int raw_width = CAMERA_VGASIZE_WIDTH;
                int raw_height = CAMERA_VGASIZE_HEIGHT;

                if(CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize || CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize || CAMERAHAL_PICSIZE_6M == mCameraPicsize){
                    raw_width = CAMERA_FWVGASIZE_WIDTH;
                }

                // After converts heap to BGRA8888
                mRawBGRA8888Heap =  new AshmemPool(raw_width * raw_height * 4,
                                                   kJpegBufferCount,
                                                   raw_width * raw_height * 4,
                                                   "YUV preview picture");

                if (!mRawBGRA8888Heap->initialized()) {
                    mRawBGRA8888Heap.clear();
                    ALOGE("END startrawsnapshot : error initializing mRawBGRA8888Heap");
                    return RET_FAIL_YUV_CB;
                }
            }

            convertRawcallback();
            mDataCallback(CAMERA_MSG_RAW_IMAGE, mRawBGRA8888Heap->mBuffers[0], mCallbackCookie);

            // Get memory clear
            mRawBGRA8888Heap.clear();
        }
    } else {
        ALOGW("startrawsnapshot : mDataCallback not YUV_CB");
    }

    retCbErrType ret;
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
        ret = RET_FAIL_JPEG_CB;
    }
    else {
        ret = RET_FAIL_YUV_CB;
    }

    if(native_get_picture(mCameraControlFd ,&mCrop)== false) {
        ALOGE("END startrawsnapshot : native_get_picture is error");
        return ret;
    }

    if(false == native_get_exifinfo(mCameraControlFd))
    {
        ALOGE("END startrawsnapshot : native_get_exifinfo is error");
        return ret;
    }

    if( native_get_jpegfilesize(mCameraControlFd) == false){
        ALOGE("END startrawsnapshot : native_get_jpegfilesize is error.");
        return ret;
    }
    ALOGD("END startrawsnapshot");
    return RET_NO_ERROR;
}


bool SemcCameraHardware::native_unregister_snapshot_bufs()
{
    ALOGD("START native_unregister_snapshot_bufs");

    /* For thumbnail*/
    register_buf(mCameraControlFd,
                 mThumbnailBufferSize,
                 mThumbnailBufferSize,
                 mThumbnailHeap->mHeap->getHeapID(),
                 0,
                 (uint8_t *)mThumbnailHeap->mHeap->base(),
                 MSM_PMEM_THUMBNAIL,
                 false,
                 false  /*unregister*/ );

    /* For original snapshot*/
    register_buf(mCameraControlFd,
                 mRawSize,
                 mRawSize,
                 mRawHeap->mHeap->getHeapID(),
                 0,
                 (uint8_t *)mRawHeap->mHeap->base(),
                 MSM_PMEM_MAINIMG,
                 false,
                 false  /*unregister*/ );

    ALOGD("END native_unregister_snapshot_bufs");

    return true;
}




#define PRINT_OPAQUE_STRUCT(p) print_mem((p), sizeof(*p))
void print_mem(void const *vp, size_t n)
{
    unsigned char const *p=(unsigned char const*)vp;
    size_t i;
    for(i=0;i<n; i++)
    {
        ALOGD("%s %d %02x", __FUNCTION__, i, p[i]);
    }
}

/**
 * @brief SemcCameraHardware::native_get_exifinfo
 * @param camfd [IN]  camera open information
 * @
 * @return unsigned bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief  Set focus rect
 */
bool SemcCameraHardware::native_get_exifinfo(int camfd)
{
    ALOGD("START native_get_exifinfo");
    int     ioctlRetVal;
    struct  msm_ctrl_cmd ctrlCmd;
    camera_exif_t exif_type;

    ctrlCmd.timeout_ms = DRV_TIMEOUT_5K;
    ctrlCmd.type       = CAMERA_GET_PARM_EXIF_PARAMS;
    ctrlCmd.length     = sizeof(camera_exif_t);
    ctrlCmd.value      = (void *)&exif_type;
    ctrlCmd.resp_fd    = camfd;

    memset(&exif_type, 0, sizeof(camera_exif_t));
    if((ioctlRetVal = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0){
        ALOGE("native_get_exifinfo: ioctl failed. ioctl return value is %d.", ioctlRetVal);
        return false;
    }


    //PRINT_OPAQUE_STRUCT(&exif_type);
    

    memset(&mExifInfo, 0, sizeof(exifinfo_t));
    mExifInfo.exposure_time.numerator    = exif_type.exposure_time.numerator;
    mExifInfo.exposure_time.denominator  = exif_type.exposure_time.denominator;
    mExifInfo.shutter_speed.numerator    = exif_type.shutter_speed_value.numerator;
    mExifInfo.shutter_speed.denominator  = exif_type.shutter_speed_value.denominator;
    mExifInfo.exposure_value.numerator   = exif_type.exposure_bias_value.numerator;
    mExifInfo.exposure_value.denominator = exif_type.exposure_bias_value.denominator;

    size_t i = 0;
    if(ISO_VALUE_MIN < exif_type.iso_speed_ratings){
        for(i = 0; i < ISO_RECORD_VALUE_COUNT; i++){
            if((exif_type.iso_speed_ratings >= iso_record_value[i].iso_max) && (exif_type.iso_speed_ratings <= iso_record_value[i].iso_min)){
                //Hit
                break;
            }
        }
        if(i == ISO_RECORD_VALUE_COUNT){
            i = ISO_RECORD_VALUE_COUNT - 1;
        }
    }
    mExifInfo.iso_speed = iso_record_value[i].record_value;
    mExifInfo.flash                      = exif_type.flash;
    mExifInfo.distance_range             = exif_type.subject_distance_range;

    ALOGD("END native_get_exifinfo");
    return true;
}
/**
 * @brief SemcCameraHardware::convertImage864to854
 * @param outBuffer [IN]  854x480 buffer size
 * @param inBuffer [IN]  864x480 buffer size
 * @
 * @return status_t
 * @n      NO_ERROR  : SUCCESS
 * @n      INVALID_OPERATION : NULL Buffer Error
 * @brief  Convert image buffer(864x480 to 854x480)
 */
status_t SemcCameraHardware::convertImage864to854(uint8_t* outBuffer, uint8_t* inBuffer)
{
    ALOGD("START convertImage864to854");
    if(NULL == outBuffer || NULL == inBuffer){
        return INVALID_OPERATION;
    }

    int i = 0;
    for(i=0; i < CAMERA_FWVGASIZE_DRV_HEIGHT*1.5; i++){
        memcpy(outBuffer+(i*CAMERA_FWVGASIZE_WIDTH), inBuffer+(i*CAMERA_FWVGASIZE_DRV_WIDTH), CAMERA_FWVGASIZE_WIDTH);
    }
    ALOGD("END convertImage864to854");
    return NO_ERROR;
}

/**
 * @brief SemcCameraHardware::convertImage864to848
 * @param outBuffer [IN]  848x480 buffer size
 * @param inBuffer [IN]  864x480 buffer size
 * @
 * @return status_t
 * @n      NO_ERROR  : SUCCESS
 * @n      INVALID_OPERATION : NULL Buffer Error
 * @brief  Convert image buffer(864x480 to 848x480)
 */
status_t SemcCameraHardware::convertImage864to848(uint8_t* outBuffer, uint8_t* inBuffer)
{
    ALOGD("START convertImage864to848");
    if(NULL == outBuffer || NULL == inBuffer){
        return INVALID_OPERATION;
    }

    int i = 0;
    for(i=0; i < CAMERA_FWVGASIZE_DRV_HEIGHT*1.5; i++){
        memcpy(outBuffer+(i*CAMERA_OHDFWVGASIZE_WIDTH), inBuffer+(i*CAMERA_FWVGASIZE_DRV_WIDTH), CAMERA_OHDFWVGASIZE_WIDTH);
    }
    ALOGD("END convertImage864to848");
    return NO_ERROR;
}

/**
 * @brief SemcCameraHardware::convertRawcallback
 * @
 * @return void
 * @brief The function to convert it YCbCr420LP into BGRA8888
 */
void SemcCameraHardware::convertRawcallback()
{
    ALOGD("START convertRawcallback ");
    int picture_width  = 0;
    int picture_height = 0;

    if(CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize || CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize || CAMERAHAL_PICSIZE_6M == mCameraPicsize){
        picture_width  = CAMERA_FWVGASIZE_DRV_WIDTH;
        picture_height = CAMERA_FWVGASIZE_DRV_HEIGHT;
    }else{
        picture_width = CAMERA_VGASIZE_WIDTH;
        picture_height = CAMERA_VGASIZE_HEIGHT;
    }

    //YCbCr420LP data structure
    camera_image_type ycbcr420LP_img_ptr;
    ycbcr420LP_img_ptr.dx = picture_width;
    ycbcr420LP_img_ptr.dy = picture_height;
    ycbcr420LP_img_ptr.imgPtr = (unsigned char*)mRawHeap->mHeap->base();
    ycbcr420LP_img_ptr.clrPtr = (uint8_t*)mRawHeap->mHeap->base() + (picture_width * picture_height);

    //BGRA8888 data structure
    camera_image_type bgra8888_img_ptr;
    bgra8888_img_ptr.imgPtr = (unsigned char*)mRawBGRA8888Heap->mHeap->base();
    if(CAMERA_FWVGASIZE_DRV_WIDTH != picture_width){
        bgra8888_img_ptr.dx = picture_width;
        bgra8888_img_ptr.dy = picture_height;
    }
    else{
        bgra8888_img_ptr.dx = CAMERA_FWVGASIZE_WIDTH;
        bgra8888_img_ptr.dy = CAMERA_FWVGASIZE_HEIGHT;
    }

    //YCbCr420LP to BGRA8888
    if(!convertYCbCr420lpToBgra8888(&ycbcr420LP_img_ptr, &bgra8888_img_ptr)){
        ALOGE("convertRawcallback :convertYCbCr420lpToBgra8888 Error");
    }
    ALOGD("END convertRawcallback ");
}

/**
 * @brief SemcCameraHardware::convertYCbCr420lpToBgra8888
 * @param input_img_ptr  [IN]  Points to the input image
 * @param output_img_ptr [OUT] Points to the output image
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief This function converts between image types
 */
bool SemcCameraHardware::convertYCbCr420lpToBgra8888(camera_image_type* input_img_ptr, camera_image_type* output_img_ptr)
{
    /* check parameter */
    if (!input_img_ptr  || !input_img_ptr->imgPtr || !output_img_ptr || !output_img_ptr->imgPtr)
    {
        ALOGE("END convertYCbCr420lpToBgra8888 : parameter error");
        return false;
    }

    register unsigned char* inputImgPtr = input_img_ptr->imgPtr;
    register uint8_t* outputImgPtr = output_img_ptr->imgPtr;

    // initialize output image ptr
    memset(outputImgPtr, 0xFF, output_img_ptr->dx * output_img_ptr->dy * 4);

	//REMOVED for open sourcing due to license restrictions.

    ALOGD("END convertYCbCr420lpToBrga8888 ");
    return true;
}

/**
 * @brief SemcCameraHardware::getThumbnailInternal
 * @
 * @return bool
 * @brief The function to make to thumnail JPEG data
 */
bool SemcCameraHardware::getThumbnailInternal()
{
    ALOGD("START getThumbnailInternal ");
#if DLOPEN_LIBMMCAMERA == 1
    int picture_width    = 0;
    int picture_height   = 0;
    int thumbnail_width  = 0;
    int thumbnail_height = 0;
    sp<AshmemPool> yuv420Heap = NULL;


    //16:9 Thumbnail size
    if(CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize || CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize || CAMERAHAL_PICSIZE_6M == mCameraPicsize){
        picture_width    = CAMERA_WQVGASIZE_WIDTH;
        picture_height   = CAMERA_WQVGASIZE_HEIGHT;
        thumbnail_width  = CAMERA_FWVGATHUMBNAIL_WIDTH;
        thumbnail_height = CAMERA_FWVGATHUMBNAIL_HEIGHT;
    }else{
    //other
        picture_width    = CAMERA_QVGASIZE_WIDTH;
        picture_height   = CAMERA_QVGASIZE_HEIGHT;
        thumbnail_width  = CAMERA_NORMALTHUMBNAIL_WIDTH;
        thumbnail_height = CAMERA_NORMALTHUMBNAIL_HEIGHT;
    }

    yuv420Heap = new AshmemPool(
                         thumbnail_width * thumbnail_height * 1.5,
                         kRawBufferCount,
                         thumbnail_width * thumbnail_height * 1.5,
                         "make thumbnail size");

    if (!yuv420Heap->initialized()) {
        ALOGE("getThumbnailInternal X failed: error initializing yuv420Heap.");
        yuv420Heap.clear();
        return false;
    }

    ipl_image_type yuv420_img_ptr;
    ipl_image_type yuv420_thumbnail_img_ptr;
    #define IPL_YCrCb420_LINE_PK 1

    yuv420_img_ptr.dx      = picture_width;
    yuv420_img_ptr.dy      = picture_height;
    yuv420_img_ptr.cFormat = IPL_YCrCb420_LINE_PK;
    yuv420_img_ptr.imgPtr  = (unsigned char*)mThumbnailHeap->mHeap->base();
    yuv420_img_ptr.clrPtr  = (uint8_t*)mThumbnailHeap->mHeap->base() + ( picture_width * picture_height );

    yuv420_thumbnail_img_ptr.dx      = thumbnail_width;
    yuv420_thumbnail_img_ptr.dy      = thumbnail_height;
    yuv420_thumbnail_img_ptr.cFormat = IPL_YCrCb420_LINE_PK;
    yuv420_thumbnail_img_ptr.imgPtr  = (unsigned char*)yuv420Heap->mHeap->base();
    yuv420_thumbnail_img_ptr.clrPtr  = (uint8_t*)yuv420Heap->mHeap->base() + ( thumbnail_width * thumbnail_height );
    int res = LINK_ipl_downsize(&yuv420_img_ptr, &yuv420_thumbnail_img_ptr, NULL);
    ALOGD("%s_ received %d", __FUNCTION__, res);
    //Downsize
    if(1 != res){
        ALOGE("getThumbnailInternal :ipl_downsize Error");
        yuv420Heap.clear();
        return false;
    }

    //16:9 Thumbnail size -> Size cut 216x120 to 160x120
    if(CAMERAHAL_PICSIZE_FULLHD == mCameraPicsize || CAMERAHAL_PICSIZE_4MWIDE == mCameraPicsize || CAMERAHAL_PICSIZE_6M == mCameraPicsize){
        //Size Cut 216x120 to 160x120
        void *inputBuffer = (void *)yuv420Heap->mHeap->base();
        void *outputBuffer = (void *)mThumbnailHeap->mHeap->base();
        for(int cnt = 0; cnt <= CAMERA_NORMALTHUMBNAIL_HEIGHT*1.5; cnt++){
            memcpy((uint8_t *)outputBuffer+(cnt * CAMERA_NORMALTHUMBNAIL_WIDTH),(uint8_t *)inputBuffer+(cnt * CAMERA_FWVGATHUMBNAIL_WIDTH +28),CAMERA_NORMALTHUMBNAIL_WIDTH);
        }
    }else{
       //other
       memcpy((uint8_t *)mThumbnailHeap->mHeap->base(),(uint8_t *)yuv420Heap->mHeap->base(), (CAMERA_NORMALTHUMBNAIL_WIDTH * CAMERA_NORMALTHUMBNAIL_HEIGHT * 1.5) );
    }
#endif // DLOPEN_LIBMMCAMERA == 1
    ALOGD("END getThumbnailInternal ");
    return true;
}


#if SCRITCH_OFF
/**
 * @brief SemcCameraHardware::setAeAfPosition
 * @param position [IN]  Rect position
 * @param face_cnt [IN]  Face count
 * @
 * @return status_t
 * @n      NO_ERROR  : SUCCESS
 * @n      UNKNOWN_ERROR : ERROR
 * @brief
 */
status_t SemcCameraHardware::setAeAfPosition(setAeAfPosition_t position, int face_cnt)
{
    if(((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0)
    {
        ALOGD("START setAeAfPosition");
        Mutex::Autolock l(&mLock);
        if(native_set_aeaf_position(mCameraControlFd, position, face_cnt) == false)
        {
            ALOGE("END setAeAfPosition : native_set_aeaf_position is error.");
            return UNKNOWN_ERROR;
        }
        ALOGD("END setAeAfPosition");
    }
    return NO_ERROR;
}
#endif//SCRITCH_OFF
/**
 * @brief SemcCameraHardware::getFocusPosition
 * @
 * @return int
 * @n      CAMERAHAL_LENS_POSITION_NORMAL : NORMAL
 * @n      CAMERAHAL_LENS_POSITION_MACRO : MACRO
 * @brief
 */
int SemcCameraHardware::getFocusPosition()
{
    return mFocusPosition;
}

/**
 * @brief SemcCameraHardware::getStatus
 * @
 * @return int
 * @n      CameraHAL status(enum CameraHalState)
 * @brief
 */
int SemcCameraHardware::getStatus()
{
//    ALOGD("getStatus : return mStateCameraHal[%d]",mStateCameraHal);
    return mStateCameraHal;
}

/**
 * @brief SemcCameraHardware::getPreviewSize
 * @param *width  [OUT]  preview width
 * @param *height [OUT]  preview height
 * @
 * @return void
 * @brief
 */
void SemcCameraHardware::getPreviewSize(int *width , int *height)
{
//    ALOGD("START getPreviewSize");
     *width  = mDimension.display_width;
     *height = mDimension.display_height;
//     ALOGD("END getPreviewSize : return Display width[%d] height[%d]",*width, *height);
     return;
}


#if SCRITCH_OFF
/**
 * @brief SemcCameraHardware::getSceneNvValue
 * @param *pBuffer [IN]  Heap buffer
 * @param size [IN]  Heap size
 * @
 * @return void
 * @brief
 */
void SemcCameraHardware::getSceneNvValue(void *pBuffer, int size)
{
    ALOGD("START getSceneNvValue");
    Mutex::Autolock l(&mLock);

    if (size <= sizeof(scene_nv_data)) {
        memcpy(pBuffer, scene_nv_data, size);
        ALOGD("%s: nv data was copied, size[%d].", __FUNCTION__, size);
    } else {
        ALOGE("%s: nv data is not copied, \
          the requested size[%d] exceeds scene_nv_data size[%d].",
          __FUNCTION__, size, sizeof(scene_nv_data));
    }

    ALOGD("END getSceneNvValue");
    return;
}
/**
 * @brief SemcCameraHardware::setSceneRecognitionInfo
 * @param *buffer     [IN] ASS information data buffer pointer
 * @param buffer_size [IN] ASS information data buffer size
 * @
 * @return void
 * @brief
 */
void SemcCameraHardware::setSceneRecognitionInfo(void *buffer, int buffer_size)
{
    ALOGD("START setSceneRecognitionInfo : buffer[%p] buffer_size[%d]", buffer, buffer_size);
    mASSBuffer = buffer;
    mASSBufferSize = buffer_size;
    ALOGD("END setSceneRecognitionInfo");
}

/**
 * @brief SemcCameraHardware::getStringvalue
 * @param *key     [IN]
 * @param value [IN]
 * @
 * @return char*
 * @brief
 */
char *SemcCameraHardware::getStringvalue(const char *key, int value)
{
    ALOGD("START getStringvalue : key[%s] value[%d]", key, value);
    if((strcmp(key, CameraParameters::SCENE_RECOGNITION) == 0)) {
        int len = sizeof(scene_recognition) / sizeof(str_map);
        for (int i = 0; i < len; i++) {
            if (scene_recognition[i].mw_val == value) {
                ALOGD("END getStringvalue : return [%s]",scene_recognition[i].desc);
                return (char *)scene_recognition[i].desc;
            }
        }
    }
    ALOGD("END getStringvalue : return NULL");

    return NULL;
}
#endif//SCRITCH_OFF
/**
 * @brief SemcCameraHardware::setExifParameters
 * @return void
 *
 * @brief The setup of exif_tag
 */
#if 1
void SemcCameraHardware::setExifParameters()
{
    ALOGD("START setExifParameters : ");

    const char *str = NULL;
    short sval = 0;

    memset(maker, 0x00, sizeof(maker));
    memset(model, 0x00, sizeof(model));
    memset(dateTime, 0x00, sizeof(dateTime));
    memset(desc, 0x00, sizeof(desc));

	ALOGD("%s before desc", __func__);

    /* 0th IFD TIFF Tags */
    /* description */
    snprintf(desc, strlen("scritch007 camera_lib"), "scritch007 camera_lib");
    addExifTag(EXIFTAGID_IMAGE_DESCRIPTION, EXIF_ASCII, (strlen(desc)+2), 1, (void *)desc);

	ALOGD("%s before get ro.product.brand", __func__);
    /* maker */
    property_get("ro.product.brand",maker,"");
    if (strlen(maker) >= 1) {
        addExifTag(EXIFTAGID_EXIF_CAMERA_MAKER, EXIF_ASCII, (strlen(maker))+1, 1, (void *)maker);
    }


	ALOGD("%s before ro.product.device", __func__);
    /* model */
    property_get("ro.product.device",model,"");
    if (strlen(model) >= 1) {
        addExifTag(EXIFTAGID_EXIF_CAMERA_MODEL, EXIF_ASCII, (strlen(model))+1, 1, (void *)model);
    }


	ALOGD("%s before rotation", __func__);
    /* rotation */
    int rot = mParameters.getInt(CameraParameters::KEY_ROTATION);
    if (rot == 90) {
        sval = 6;
    } else if (rot == 180) {
        sval = 3;
    } else if (rot == 270) {
        sval = 8;
    } else {
        sval = 1;
    }

    ALOGD("%s setting orientation to %d info was %d", __FUNCTION__, sval, rot);
    addExifTag(EXIFTAGID_ORIENTATION, EXIF_SHORT, 1, 1, (void *)&sval);

    // set focal length tag
    int focalLengthValue = (int) (mParameters.getFloat(
                CameraParameters::KEY_FOCAL_LENGTH) * FOCAL_LENGTH_DECIMAL_PRECISON);
    rat_t focalLengthRational = {focalLengthValue, FOCAL_LENGTH_DECIMAL_PRECISON};
    memcpy(&focalLength, &focalLengthRational, sizeof(focalLengthRational));
    addExifTag(EXIFTAGID_FOCAL_LENGTH, EXIF_RATIONAL, 1,
                1, (void *)&focalLength);


    /* YCbCrPositioning */
    sval = 2;
    addExifTag(EXIFTAGID_YCBCR_POSITIONING, EXIF_SHORT, 1, 1, (void *)&sval);

    /* 0th Exif Tags */
    /* exposure time */
    expoTime.num   = mExifInfo.exposure_time.numerator;
    expoTime.denom = mExifInfo.exposure_time.denominator;
    addExifTag(EXIFTAGID_EXPOSURE_TIME, EXIF_RATIONAL, 1, 1, (void *)&expoTime);

    /* Flash */
    short flash_sval = 0;
    str = mParameters.get(CameraParameters::KEY_FLASH_MODE);

    if (str != NULL) {
       int32_t value = attr_lookup(flash, sizeof(flash) / sizeof(str_map), str);
       if (value != NOT_FOUND) {
            bool shouldBeOn = strcmp(str, "on") == 0;
            flash_sval = shouldBeOn?1:0;
       }
    }

    addExifTag(EXIFTAGID_FLASH, EXIF_SHORT, 1, 1, (void *)&flash_sval);

    sval = 0;
    addExifTag(EXIFTAGID_EXPOSURE_MODE, EXIF_SHORT, 1, 1, (void *)&sval);

    /* White Balance */
    sval = 0;
    str = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::WHITE_BALANCE_AUTO)) {
            sval = 0;
        } else {
            sval = 1;
        }
        addExifTag(EXIFTAGID_WHITE_BALANCE, EXIF_SHORT, 1, 1, (void *)&sval);
    }

    /* Scene Capture Mode */
    sval = 0;
    str = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::SCENE_MODE_AUTO)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_PORTRAIT)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_LANDSCAPE)) {
            sval = 1;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_NIGHT)) {
            sval = 3;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_SPORTS)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_PARTY)) {
            sval = 2;
        }
        addExifTag(EXIFTAGID_SCENE_CAPTURE_TYPE, EXIF_SHORT, 1, 1, (void *)&sval);
    }

        /* F number */
        fNum.num   = 0x001C;
        fNum.denom = 0x000A;
        addExifTag(EXIFTAGID_F_NUMBER, EXIF_RATIONAL, 1, 1, (void *)&fNum);

        /* ISO Speed Ratings */
        sval = (short)mExifInfo.iso_speed;
        addExifTag(EXIFTAGID_ISO_SPEED_RATINGS, EXIF_SHORT, 1, 1, (void *)&sval);

        /* Data Time Original */
        addExifTag(EXIFTAGID_EXIF_DATE_TIME_ORIGINAL, EXIF_ASCII,
                        20, 1, (void *)createDateTime);

        /* Data Time Digitized */
        addExifTag(EXIFTAGID_EXIF_DATE_TIME_DIGITIZED, EXIF_ASCII,
                        20, 1, (void *)createDateTime);

        /* Shutter Speed */
        shutterSpeed.num   = mExifInfo.shutter_speed.numerator;
        shutterSpeed.denom = mExifInfo.shutter_speed.denominator;
        addExifTag(EXIFTAGID_SHUTTER_SPEED, EXIF_SRATIONAL, 1, 1, (void *)&shutterSpeed);

        /* Exposure Bias */
        expoBias.num   = mExifInfo.exposure_value.numerator;
        expoBias.denom = mExifInfo.exposure_value.denominator;
        addExifTag(EXIFTAGID_EXPOSURE_BIAS_VALUE, EXIF_SRATIONAL, 1, 1, (void *)&expoBias);

    /* Custom Rendered */
    str = mParameters.get(CameraParameters::KEY_EFFECT);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::EFFECT_NONE)) {
            sval = 0;
        } else {
            sval = 1;
        }
        addExifTag(EXIFTAGID_CUSTOM_RENDERED, EXIF_SHORT, 1, 1, (void *)&sval);
    }

        /* Light Source */
        sval = 0;
        if (0x01 & flash_sval) {
            str = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
            if (str != NULL) {
                if (!strcmp(str, CameraParameters::WHITE_BALANCE_AUTO)) {
                    sval = 0;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_DAYLIGHT)) {
                    sval = 9;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT)) {
                    sval = 10;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_FLUORESCENT)) {
                    sval = 2;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_INCANDESCENT)) {
                    sval = 3;
                } else {
                    sval = 0;
                }
            } else {
                sval = 0;
            }
        }
        addExifTag(EXIFTAGID_LIGHT_SOURCE, EXIF_SHORT, 1, 1, (void *)&sval);

        /* Zoom Ratio */
        #define DEV_CAMERA_ZOOM_MAX_RATIO 16
        #define DEV_CAMERA_ZOOM_MIN_RATIO 1
        #define DEV_CAMERA_ZOOM_MIN_STEP  1

        zoomRatio.num = ((DEV_CAMERA_ZOOM_MAX_RATIO - DEV_CAMERA_ZOOM_MIN_RATIO) * 100 )
                / (mMaxZoom - DEV_CAMERA_ZOOM_MIN_STEP)
                * (mZoom - 1) + 100;

        zoomRatio.denom = 0x0064;
        addExifTag(EXIFTAGID_DIGITAL_ZOOM_RATIO, EXIF_RATIONAL, 1, 1, (void *)&zoomRatio);

        /* Subject Distance Range */
        sval = (short)mExifInfo.distance_range;
        addExifTag(EXIFTAGID_SUBJECT_DISTANCE_RANGE, EXIF_SHORT, 1, 1, (void *)&sval);


 
}
#else
void SemcCameraHardware::setExifParameters()
{
    ALOGD("START setExifParameters : ");

    const char *str = NULL;
    short sval = 0;

    memset(maker, 0x00, sizeof(maker));
    memset(model, 0x00, sizeof(model));
    memset(dateTime, 0x00, sizeof(dateTime));
    memset(desc, 0x00, sizeof(desc));

    /* 0th IFD TIFF Tags */
    /* description */
    addExifTag(EXIFTAGID_IMAGE_DESCRIPTION, EXIF_ASCII, 0, 1, (void *)desc);

    /* maker */
    property_get("ro.product.brand",maker,"");
    if (strlen(maker) >= 1) {
        addExifTag(EXIFTAGID_MAKE, EXIF_ASCII, (strlen(maker))+1, 1, (void *)maker);
    }

    /* model */
    property_get("ro.product.device",model,"");
    if (strlen(model) >= 1) {
        addExifTag(EXIFTAGID_MODEL, EXIF_ASCII, (strlen(model))+1, 1, (void *)model);
    }

    /* rotation */
    int rot = mParameters.getInt(CameraParameters::KEY_ROTATION);
    if (rot == 90) {
        sval = 6;
    } else if (rot == 180) {
        sval = 3;
    } else if (rot == 270) {
        sval = 8;
    } else {
        sval = 1;
    }
    addExifTag(EXIFTAGID_ORIENTATION, EXIF_SHORT, 1, 1, (void *)&sval);


    // set focal length tag
    int focalLengthValue = (int) (mParameters.getFloat(
                CameraParameters::KEY_FOCAL_LENGTH) * FOCAL_LENGTH_DECIMAL_PRECISON);
    rat_t focalLengthRational = {focalLengthValue, FOCAL_LENGTH_DECIMAL_PRECISON};
    memcpy(&focalLength, &focalLengthRational, sizeof(focalLengthRational));
    addExifTag(EXIFTAGID_FOCAL_LENGTH, EXIF_RATIONAL, 1,
                1, (void *)&focalLength);


    /* YCbCrPositioning */
    sval = 2;
    addExifTag(EXIFTAGID_YCBCR_POSITIONING, EXIF_SHORT, 1, 1, (void *)&sval);

    /* 0th Exif Tags */
    /* exposure time */
    expoTime.num   = mExifInfo.exposure_time.numerator;
    expoTime.denom = mExifInfo.exposure_time.denominator;
    addExifTag(EXIFTAGID_EXPOSURE_TIME, EXIF_RATIONAL, 1, 1, (void *)&expoTime);

    /* Flash */
    short flash_sval = 0;
    if (((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {
        str = mParameters.get(CameraParameters::FLASH_STATUS);
        if (str != NULL) {
            if ((strcmp(str, CameraParameters::FLASH_STATUS_ON) == 0)) {
                flash_sval = 1;
            }
        }
    }
    addExifTag(EXIFTAGID_FLASH, EXIF_SHORT, 1, 1, (void *)&flash_sval);

    /* Custom Rendered */
    str = mParameters.get(CameraParameters::KEY_EFFECT);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::EFFECT_NONE)) {
            sval = 0;
        } else {
            sval = 1;
        }
        addExifTag(EXIFTAGID_CUSTOM_RENDERED, EXIF_SHORT, 1, 1, (void *)&sval);
    }

    /* Exposure Mode */
    sval = 0;
    addExifTag(EXIFTAGID_EXPOSURE_MODE, EXIF_SHORT, 1, 1, (void *)&sval);

    /* White Balance */
    sval = 0;
    str = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::WHITE_BALANCE_AUTO)) {
            sval = 0;
        } else {
            sval = 1;
        }
        addExifTag(EXIFTAGID_WHITE_BALANCE, EXIF_SHORT, 1, 1, (void *)&sval);
    }

    /* Scene Capture Mode */
    sval = 0;
    str = mParameters.get(CameraParameters::KEY_SCENE_MODE);
    if (str != NULL) {
        if (!strcmp(str, CameraParameters::SCENE_MODE_AUTO)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_PORTRAIT)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_LANDSCAPE)) {
            sval = 1;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_NIGHT)) {
            sval = 3;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_NIGHT_PORTRAIT)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_BEACH_SNOW)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_SPORTS)) {
            sval = 0;
        } else if (!strcmp(str, CameraParameters::SCENE_MODE_PARTY)) {
            sval = 2;
        }
        addExifTag(EXIFTAGID_SCENE_CAPTURE_TYPE, EXIF_SHORT, 1, 1, (void *)&sval);
    }

    /* Only signature App case */
    if (((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) == 0) {

        /* F number */
        fNum.num   = 0x001C;
        fNum.denom = 0x000A;
        addExifTag(EXIFTAGID_F_NUMBER, EXIF_RATIONAL, 1, 1, (void *)&fNum);

        /* ISO Speed Ratings */
        sval = (short)mExifInfo.iso_speed;
        addExifTag(EXIFTAGID_ISO_SPEED_RATINGS, EXIF_SHORT, 1, 1, (void *)&sval);

        /* Data Time Original */
        addExifTag(EXIFTAGID_EXIF_DATE_TIME_ORIGINAL, EXIF_ASCII,
                        20, 1, (void *)createDateTime);

        /* Data Time Digitized */
        addExifTag(EXIFTAGID_EXIF_DATE_TIME_DIGITIZED, EXIF_ASCII,
                        20, 1, (void *)createDateTime);

        /* Shutter Speed */
        shutterSpeed.num   = mExifInfo.shutter_speed.numerator;
        shutterSpeed.denom = mExifInfo.shutter_speed.denominator;
        addExifTag(EXIFTAGID_SHUTTER_SPEED, EXIF_SRATIONAL, 1, 1, (void *)&shutterSpeed);

        /* Exposure Bias */
        expoBias.num   = mExifInfo.exposure_value.numerator;
        expoBias.denom = mExifInfo.exposure_value.denominator;
        addExifTag(EXIFTAGID_EXPOSURE_BIAS_VALUE, EXIF_SRATIONAL, 1, 1, (void *)&expoBias);

        /* Metering Mode */
        sval = 2;
        str = mParameters.get(CameraParameters::KEY_AUTO_EXPOSURE);
        if (str != NULL) {
            if (!strcmp(str, CameraParameters::AUTO_EXPOSURE_FRAME_AVG)) {
                sval = 1;
            } else if (!strcmp(str, CameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED))            {
                sval = 2;
            } else if (!strcmp(str, CameraParameters::AUTO_EXPOSURE_SPOT_METERING)) {
                sval = 3;
            }
        }
#if SCRITCH_OFF
        str = mParameters.get(CameraParameters::FACE_DETECTION);
        if (str != NULL) {
            if ((strcmp(str, CameraParameters::FACE_DETECTION_ON) == 0)) {
                sval = 255;
            }
        }
#endif
        addExifTag(EXIFTAGID_METERING_MODE, EXIF_SHORT, 1, 1, (void *)&sval);

        /* Light Source */
        sval = 0;
        if (0x01 & flash_sval) {
            str = mParameters.get(CameraParameters::KEY_WHITE_BALANCE);
            if (str != NULL) {
                if (!strcmp(str, CameraParameters::WHITE_BALANCE_AUTO)) {
                    sval = 0;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_DAYLIGHT)) {
                    sval = 9;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT)) {
                    sval = 10;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_FLUORESCENT)) {
                    sval = 2;
                } else if (!strcmp(str, CameraParameters::WHITE_BALANCE_INCANDESCENT)) {
                    sval = 3;
                } else {
                    sval = 0;
                }
            } else {
                sval = 0;
            }
        }
        addExifTag(EXIFTAGID_LIGHT_SOURCE, EXIF_SHORT, 1, 1, (void *)&sval);

        /* Focul Length */
        focLen.num =   0x01D1;
        focLen.denom = 0x0064;
        addExifTag(EXIFTAGID_FOCAL_LENGTH, EXIF_RATIONAL, 1, 1, (void *)&focLen);

        /* MakerNote */
        addExifTag(EXIFTAGID_EXIF_MAKER_NOTE, EXIF_UNDEFINED, 0, 1, (void *)makernote);

        /* UserComment */
        addExifTag(EXIFTAGID_EXIF_USER_COMMENT, EXIF_UNDEFINED, 0, 1, (void *)usercomment);

        /* Zoom Ratio */
        #define DEV_CAMERA_ZOOM_MAX_RATIO 16
        #define DEV_CAMERA_ZOOM_MIN_RATIO 1
        #define DEV_CAMERA_ZOOM_MIN_STEP  1

        zoomRatio.num = ((DEV_CAMERA_ZOOM_MAX_RATIO - DEV_CAMERA_ZOOM_MIN_RATIO) * 100 )
                / (mMaxZoom - DEV_CAMERA_ZOOM_MIN_STEP)
                * (mZoom - 1) + 100;

        zoomRatio.denom = 0x0064;
        addExifTag(EXIFTAGID_DIGITAL_ZOOM_RATIO, EXIF_RATIONAL, 1, 1, (void *)&zoomRatio);

        /* Subject Distance Range */
        sval = (short)mExifInfo.distance_range;
        addExifTag(EXIFTAGID_SUBJECT_DISTANCE_RANGE, EXIF_SHORT, 1, 1, (void *)&sval);

    }

    ALOGD("END setExifParameters : ");
}
#endif
 /**
 * @brief SemcCameraHardware::receiveExifData
 *
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief The function to inform a camera service of the jpeg(Exif) Processing completion
 */
bool SemcCameraHardware::receiveExifData(void)
{
    ALOGD("START receiveExifData ");

    uint8_t *thumbnailBuff = (uint8_t *)mThumbnailBuffHeap->mHeap->base();

    int32_t top = 0, bottom = 0;
    int32_t exifHeaderSize = 0;
    const uint32_t topmask = 0xFF00, btmmask = 0x00FF;
    uint32_t jpegDataSize = 0;
    uint16_t xDimension = mDimension.picture_width;
    uint16_t yDimension = mDimension.picture_height;
    int i = 0;
    sp<AshmemPool> jpegExifDataHeap =  NULL;

    if ((thumbnailBuff)[0] != 0xFF || (thumbnailBuff)[1] != 0xD8 ) {
        ALOGE("receiveExifData error : Invalid jpeg data.");
        mJpegSnapShotHeap.clear();
        mThumbnailBuffHeap.clear();
        if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
            mDataCallback(CAMERA_MSG_COMPRESSED_IMAGE, NULL,  mCallbackCookie);
        }
        return false;
    }

    /* Exif header size.(include thumbnail data.) */
    /* indicate APP1 segment size */
    i = SOI_MARKER_LEN + APP1_MARKER_LEN;
    exifHeaderSize = (((thumbnailBuff)[i] << 8 ) | ((thumbnailBuff)[i+1])) + i;
    /* Jpeg main data size */
    jpegDataSize = exifHeaderSize + (mJpegSize-SOI_MARKER_LEN);
    /* indicate Tag ID */
    i = SOI_MARKER_LEN + APP1_SEG_LEN + TIFF_HEADER_LEN + IFD_NUM_LEN;
    do {
        /* Searches EXIF IFD pointer(0x8769) in 0th IFD */
        if ((thumbnailBuff)[i] == 0x87 && (thumbnailBuff)[i+1] == 0x69) {
            i += EXIF_DATA_OFFSET;
            int32_t address = ((thumbnailBuff)[i] << 24 ) | ((thumbnailBuff)[i+1] << 16 ) |
                                ((thumbnailBuff)[i+2] << 8 ) | ((thumbnailBuff)[i+3] );
            i = address + (SOI_MARKER_LEN + APP1_SEG_LEN) + IFD_NUM_LEN;
            continue;
        }

        /* Searches PixelXDimension(0xA002) in EXIF IFD */
        if ((thumbnailBuff)[i] == 0xA0 && (thumbnailBuff)[i+1] == 0x02) {
            top = (xDimension & topmask) >> 8;
            bottom = (xDimension & btmmask);
            /* Replace X dimension */
            (thumbnailBuff)[i+EXIF_DATA_OFFSET] = 0x00;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+1] = 0x00;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+2] = top;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+3] = bottom;
        }

        /* Searches PixelYDimension(0xA003) in EXIF IFD */
        if ((thumbnailBuff)[i] == 0xA0 && (thumbnailBuff)[i+1] == 0x03) {
            top = (yDimension & topmask) >> 8;
            bottom = (yDimension & btmmask);
            /* Replace Y dimension */
            (thumbnailBuff)[i+EXIF_DATA_OFFSET] = 0x00;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+1] = 0x00;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+2] = top;
            (thumbnailBuff)[i+EXIF_DATA_OFFSET+3] = bottom;
            break;
        }
        i += FIELD_LEN;
    } while(exifHeaderSize  > i);

    jpegExifDataHeap = new AshmemPool(jpegDataSize,
                                     kJpegBufferCount,
                                    jpegDataSize,
                                     "EXIF_jpeg");
    if (!jpegExifDataHeap->initialized()) {
        ALOGE("receiveExifData : jpegExifDataHeap initialized error");
        mJpegSnapShotHeap.clear();
        mThumbnailBuffHeap.clear();
        jpegExifDataHeap.clear();
        if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
            mDataCallback(CAMERA_MSG_COMPRESSED_IMAGE, NULL,  mCallbackCookie);
        }
        return false;
    }

    uint8_t *base = (uint8_t *)jpegExifDataHeap->mHeap->base();
    memcpy(base, thumbnailBuff, exifHeaderSize);
    mThumbnailBuffHeap.clear();
    mThumbnailsize = 0;

    uint8_t *mainBuff = (uint8_t *)mJpegSnapShotHeap->mHeap->base();
    memcpy(base+exifHeaderSize, mainBuff+SOI_MARKER_LEN, (mJpegSize-SOI_MARKER_LEN));

    if (((CAMERA_EXTENSION_SIGNATURE >> JUDGMENTBIT) ^ mSemcCameraFlag) != 0) {
        if(mDataCallback && (mMsgEnabled & (CAMERA_MSG_RAW_IMAGE | CAMERA_MSG_POSTVIEW_FRAME))) {
            ssize_t offset;
            ssize_t snapshot_offset = 0;

            if((CAMERA_FWVGASIZE_DRV_WIDTH == mDimension.postview_width) 
                && (CAMERA_FWVGASIZE_DRV_HEIGHT == mDimension.postview_height)) {
                int preview_width, preview_height;
                mParameters.getPreviewSize(&preview_width, &preview_height);
                if(CAMERA_FWVGASIZE_WIDTH == preview_width
                    && CAMERA_FWVGASIZE_HEIGHT == preview_height) {
                    offset = (CAMERA_FWVGASIZE_WIDTH * CAMERA_FWVGASIZE_HEIGHT *
                              1.5 * snapshot_offset);
                    convertImage864to854((uint8_t *)mRawHeapSub->mHeap->base(),
                                         (uint8_t *)mRawHeap->mHeap->base() );
                } else if (CAMERA_OHDFWVGASIZE_WIDTH == preview_width
                    && CAMERA_OHDFWVGASIZE_HEIGHT == preview_height) {
                    offset = (CAMERA_OHDFWVGASIZE_WIDTH * CAMERA_OHDFWVGASIZE_HEIGHT *
                              1.5 * snapshot_offset);
                    convertImage864to848((uint8_t *)mRawHeapSub->mHeap->base(),
                                         (uint8_t *)mRawHeap->mHeap->base() );
                } else {
                    offset = (mDimension.postview_width * mDimension.postview_height *
                              1.5 * snapshot_offset);
                    memcpy((uint8_t *)mRawHeapSub->mHeap->base() ,
                           (uint8_t *)mRawHeap->mHeap->base(), 0 + mRawSize);
                }
            } else {
                offset = (mDimension.postview_width * mDimension.postview_height *
                          1.5 * snapshot_offset);
                memcpy((uint8_t *)mRawHeapSub->mHeap->base() ,
                       (uint8_t *)mRawHeap->mHeap->base(), 0 + mRawSize);
            }

            if (mRawHeapSub != NULL && mRawHeap != NULL) {
                if(mMsgEnabled & CAMERA_MSG_RAW_IMAGE){
                    mDataCallback(CAMERA_MSG_RAW_IMAGE, mRawHeapSub->mBuffers[offset],
                                  mCallbackCookie);
                }
                if(mMsgEnabled & CAMERA_MSG_POSTVIEW_FRAME){
                    mDataCallback(CAMERA_MSG_POSTVIEW_FRAME, mRawHeapSub->mBuffers[offset],
                                  mCallbackCookie);
                }
            }
        }
    }
    mStateCameraHal = CAMERAHAL_STATE_TAKEPICDONE;

    if (mDataCallback && (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
        int index = 0;
        sp<MemoryBase> buffer = new MemoryBase(jpegExifDataHeap->mHeap,
                                               index * jpegExifDataHeap->mBufferSize +
                                               0,
                                               jpegDataSize);

        mDataCallback(CAMERA_MSG_COMPRESSED_IMAGE, buffer, mCallbackCookie);
        buffer = NULL;
    }

    jpegExifDataHeap.clear();
    ALOGD("END receiveExifData ");
    return true;
}

/**
 * @brief SemcCameraHardware::native_set_parm
 * @param type       [IN] Command type
 * @param length     [IN] Length of command
 * @param value      [IN] Parameter's value
 * @param timeout_ms [IN] Time of timeout
 * @
 * @return bool
 * @n      true  : SUCCESS
 * @n      false : ERROR
 * @brief The parameter is done to the device driver.
 */
bool SemcCameraHardware::native_set_parm(cam_ctrl_type type, uint16_t length, void *value, int timeout_ms)
{
    ALOGD("START native_set_parm(set timeout) : fd[%d] cam_ctrl_type[%d] timeout[%d]", mCameraControlFd, type, timeout_ms);

    struct msm_ctrl_cmd ctrlCmd;

    ctrlCmd.timeout_ms = timeout_ms;
    ctrlCmd.type       = (uint16_t)type;
    ctrlCmd.length     = length;
    // FIXME: this will be put in by the kernel
    ctrlCmd.resp_fd    = mCameraControlFd;
    ctrlCmd.value      = value;

    if (ioctl(mCameraControlFd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0 ||
                ctrlCmd.status != CAM_CTRL_SUCCESS) {
        ALOGE("END native_set_parm(set timeout) : error [%s]: fd %d, type [%d], length [%d], status [%d]", strerror(errno), mCameraControlFd, type, length, ctrlCmd.status);
        return false;
    }
    ALOGD("END native_set_parm(set timeout)");
    return true;
}


/**
 * @brief mm_camzoom_callback
 * @param zoom [IN] Processing result
 * @return void
 *
 * @brief The function that an auto-zoom answer from the device driver is taken
 */
static void mm_camzoom_callback(struct msm_zoom_results_t * zoom)
{
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0)
    {
        obj->receiveAutoZoom(zoom);
    }
}

/**
 * @brief mm_camanalysis_callback
 * @param analysis_data [IN] Processing result
 * @return void
 *
 * @brief The function that an auto-zoom answer from the device driver is taken
 */
static void mm_camanalysis_callback(struct msm_analysis_data_t* analysis_data)
{
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0)
    {
        obj->receiveFrameAnalysis(analysis_data);
    }
}

/**
 * @brief SemcCameraHardware::receiveAutoFocus
 * @param af_state [IN] Processing result
 * @return void
 *
 * @brief The function to inform a camera service of the auto-focus Processing completion
 */
void  SemcCameraHardware::receiveAutoFocus(struct msm_af_results_t * af_result)
{
    ALOGD("START receiveAutoFocus : mStateCameraHal = [%d]",mStateCameraHal);

    if((CAMERAHAL_STATE_AFSTART != mStateCameraHal) &&
       (CAMERAHAL_STATE_AFLOCK != mStateCameraHal) &&
       (CAMERAHAL_STATE_AFCANCEL != mStateCameraHal)){
        ALOGW("END receiveAutoFocus : mStateCameraHal Status Error");
        return;
    }
    ALOGD("%s AF_RESULT %d", __FUNCTION__, af_result->msm_af_result);
    mCallbackLock.lock();
    notify_callback autoFocuscb = mNotifyCallback;
    bool autoFocusEnabled = mNotifyCallback && (mMsgEnabled & CAMERA_MSG_FOCUS);
    bool status=false;
    void *data = mCallbackCookie;
    mCallbackLock.unlock();

    if (mCancelAutoFocusFlg && (af_result->msm_af_result >= MSM_AF_FORCE_STOP ))
    {
        Mutex::Autolock lock(&mCancelAutoFocus);
        ALOGV("receiveAutoFocus : LOCK ACQUIRED ");
        mCancelAutoFocusFlg = false;
        mAutoFocusThreadRunning = false;
        mCancelAutoFocusWait.signal();
        ALOGV("receiveAutoFocus: SIGNALLED ");
        return;

    }

    if (!autoFocusEnabled)
    {
        ALOGW("%s Autofocus callback is NULL", __FUNCTION__);
    }

    if(true == mFocusLockValue) {
        usleep(200*1000);
        mFocusLockValue = false;
    }

    ALOGV("receiveAutoFocus : 3rdparty : AF SUCCESS");
    
    if(af_result->msm_af_result<=MSM_AF_FAILED)
    {
        //MSM_AF_FAILED or MSM_AF_SUCCESS same status
        status=true;
    }
    
    autoFocuscb(CAMERA_MSG_FOCUS, status, 0, data);
    mAutoFocusThreadRunning = false;
    mStateCameraHal = CAMERAHAL_STATE_PREVIEWSTART;

    ALOGD("END receiveAutoFocus");
}


/* @brief mm_camautofocus_callback
 * @param af_state [IN] Processing result
 * @return void
 *
 * @brief The function that an auto-focus answer from the device driver is taken
 */
static void mm_camautofocus_callback(struct msm_af_results_t * af_result)
{
    sp<SemcCameraHardware> obj = SemcCameraHardware::getInstance();
    if (obj != 0) {
        obj->receiveAutoFocus(af_result);
    }
}


}; // namespace android
