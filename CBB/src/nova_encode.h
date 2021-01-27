/*
* Function : 
*    define encode API include 
*    1. option set ;
*    2. init encode ;
*    3. create encode ;
*    4. get H264/H265 data;
*    5. destory encode;
*
* Author : zhaopeng
* Time : 2021/01/11
*/
#include "st_common.h"
#include "st_vif.h"
#include "st_vpe.h"
#include "st_venc.h"
#include "mi_divp.h"

#ifndef NOVA_ENCODE_H
#define NOVA_ENCODE_H

enum AVMediaType
{
    AVMEDIA_TYPE_NONE,
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO
};
enum AVEncodeFormat
{
    AV_ENCDOE_ID_NONE,
    AV_ENCDOE_ID_MJPEG,
    AV_ENCDOE_ID_MPEG4,
    AV_ENCDOE_ID_H264,
    AV_ENCDOE_ID_H265
};
enum YUVDataType
{
    YUV_SEMIPLANAR_420,
    YUV_SEMIPLANAR_422
};
typedef struct resolution_rate
{
    int width;
    int height;
    int rate;
}TY_RESOLUTION_RATE;

typedef struct encode_out_data
{
    int frameLen;
    char *buf;
}TY_ENCODE_OUT_DATA;


typedef struct encode_option
{
    int channel;
    enum AVMediaType mediaType;
    enum YUVDataType inputDataType;
    enum AVEncodeFormat encodeFormat;
    TY_RESOLUTION_RATE  resolutionRate;
    TY_ENCODE_OUT_DATA outdata;
}TY_ENCODE_OPTION;

typedef struct encode_inside_parameter
{
    MI_VIF_DEV vifDev;
    MI_VIF_CHN vifChn;
    MI_VPE_CHANNEL VpeChn;
    MI_U32 u32InputPort;
    MI_U32 DivpChn;
    MI_VENC_CHN VencChn;
    MI_U32 u32VencDevId;

    MI_SYS_PixelFormat_e ePixFormat;
    MI_VPE_HDRType_e eHdrType;

    MI_SYS_PixelFormat_e inputYUVType;
    enum AVEncodeFormat encodeFormat;
    TY_RESOLUTION_RATE  resolutionRate;

}TY_ENCODE_INSIDE_PARAM;
int nova_find_encoder_by_name(char* decoderName);

typedef struct nova_encode
{
    const char *name; // "Hisi" "RK" "Sigmastar"
    TY_ENCODE_OPTION option;
    TY_ENCODE_INSIDE_PARAM  insideParam;
    int (*encodeCreate)(void *);
    int (*encodeGetData)(void *);
    int (*close)(void *);
}TY_NOVA_ENCODER;

typedef struct nova_encoder_queue
{
    TY_NOVA_ENCODER *encoders;
}TY_NOVA_ENCODER_QUEUE;



#endif /*NOVA_ENCODE_H*/