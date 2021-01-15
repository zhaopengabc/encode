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

typedef struct encode_option
{
    int channel;
    enum AVMediaType mediaType;
    enum YUVDataType inputDataType;
    enum AVEncodeFormat encodeFormat;
    TY_RESOLUTION_RATE  resolutionRate;
}TY_ENCODE_OPTION;

int nova_find_encoder_by_name(char* decoderName);

typedef struct nova_encode
{
    const char *name; // "Hisi" "RK" "Sigmastar"
    int channel;
    TY_ENCODE_OPTION option;
    int (*set_encode_option)(TY_ENCODE_OPTION *);
    int (*init_encode)(TY_ENCODE_OPTION*);
    int (*receive_packet)();
    int (*send_frame)();
    int (*close)(TY_ENCODE_OPTION *);
}TY_NOVA_ENCODER;

typedef struct nova_encoder_queue
{
    TY_NOVA_ENCODER *encoders;
}TY_NOVA_ENCODER_QUEUE;

#endif /*NOVA_ENCODE_H*/