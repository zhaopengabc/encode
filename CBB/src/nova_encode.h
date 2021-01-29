#ifndef NOVA_ENCODE_H
#define NOVA_ENCODE_H

/*
* Function : 
*    define encode API。
*    
* 
* Author : zhaopeng
* Time : 2021/01/11
*
* 用户操作方式：

  1、先设置encode的参数，比如分辨率，编码方式等等。
  2、创建编码队列，方便多个编码器创建。把参数传入到编码器内部。调用“nova_encoder”
  3、如果是普通模式，调用encode的回调函数
    1）调用encodeCreate_General
    2）调用encodeGetCompressData_General。此函数调用完后，就能获取到压缩完后数据，H264 or H265
    3）调用encodeDestroy_General
  4、如果是用户层需要操作YUV数据，调用；
    1）调用encodeCreate
    2）调用encodeOutputYUVData
    3）修改YUV回调函数，用户层自己需要处理YUV。
    4）调用encodeGetCompressData.此函数调用完后，就能获取到压缩完后数据，H264 or H265
    5）调用encodeDestroy
*/
#include "st_common.h"
#include "st_vif.h"
#include "st_vpe.h"
#include "st_venc.h"
#include "mi_divp.h"

/*
编码器类型：音频和视频，目前只支持视频。
*/
enum AVMediaType
{
    AVMEDIA_TYPE_NONE,
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO
};
/*
编码器编码格式：H264/H265针对视频流。MPEG4/MJPEG针对图片
*/
enum AVEncodeFormat
{
    AV_ENCDOE_ID_NONE,
    AV_ENCDOE_ID_MJPEG,
    AV_ENCDOE_ID_MPEG4,
    AV_ENCDOE_ID_H264,
    AV_ENCDOE_ID_H265
};
/*
YUV数据格式：目前只支持YUV420和YUV422.
*/
enum YUVDataType
{
    YUV_SEMIPLANAR_420,
    YUV_SEMIPLANAR_422
};
/*
分辨率设置：宽度/高度/帧率
*/
typedef struct resolution_rate
{
    int width;
    int height;
    int rate;
}TY_RESOLUTION_RATE;
/*
编码器压缩后数据，H264 or H265
*/
typedef struct encode_compress_data
{
    unsigned int frameLen;
    char *buf;
}TY_ENCODE_OUT_DATA;
/*
YUV数据类型：保护数据长度，虚拟地址，物理地址，对齐地址。
*/
typedef struct YUV_data
{
    MI_U32 bufSize;
    void *pVirAddr[3];
    unsigned long phyAddr[3];
    unsigned long stride[3];
}TY_YUV_DATA;
/*
网络传输方式：CVR/VBR/FixQp/AVBR
*/
enum NetTransforMode
{
    CBR,
    VBR,
    FixQp,
    AVBR
};
/*
图像质量参赛：最高码率，网络传输方式。
*/
typedef struct image_quality_param
{
    int maxBitRat;
    enum NetTransforMode mode;
}TY_IMAGE_QUALITY;

/*
编码器参数，此接口需要用户设置，包含分辨率设置，压缩数据格式，图像质量。
        YUV数据处理回掉函数，如果普通模式下，不用设置此函数。
*/
typedef struct encode_option
{
    int channel;
    enum AVMediaType mediaType;
    enum YUVDataType YUVDataType;
    enum AVEncodeFormat encodeFormat;
    TY_RESOLUTION_RATE  resolutionRate;
    TY_ENCODE_OUT_DATA compressData;
    TY_IMAGE_QUALITY imageQuality;
    int (*processYUVData)(void *desData,void *srcData);
}TY_ENCODE_OPTION;
/*
编码器内部参数，主要是各个模块通道，像素格式等，为了内部操作方便，引入此函数。
*/
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

/*
编码器对外接口变量，分为两种操作模式，普通模式和特殊模式。
普通模式：encodeCreate_General/encodeGetCompressData_General/encodeDestroy_General。三个组合使用；
特殊模式，是用户需要操作YUV数据，encodeCreate/encodeGetCompressData/encodeOutputYUVData/encodeDestroy。四个组合使用。
注意：两种模式的函数不能混合使用。
*/
typedef struct nova_encode
{
    const char *name; // "Hisi" "RK" "Sigmastar"
    TY_ENCODE_OPTION option;
    TY_ENCODE_INSIDE_PARAM  insideParam;
    int (*encodeCreate)(void *);
    int (*encodeGetCompressData)(void *);
    int (*encodeOutputYUVData)(void *);
    int (*encodeDestroy)(void *);

    int (*encodeCreate_General)(void *);
    int (*encodeGetCompressData_General)(void *);
    int (*encodeDestroy_General)(void *);
}TY_NOVA_ENCODER;
/*
编码器队列，在用户操作时，首先创建队列。
*/
typedef struct nova_encoder_queue
{
    TY_NOVA_ENCODER *encoders;
}TY_NOVA_ENCODER_QUEUE;

TY_NOVA_ENCODER_QUEUE *nova_encoder(TY_NOVA_ENCODER *NOVA_encoder);
TY_NOVA_ENCODER_QUEUE *nova_encoder_alloc(unsigned int decoderSize);
void nova_encoder_free(TY_NOVA_ENCODER_QUEUE *decoderQueue);
#endif /*NOVA_ENCODE_H*/