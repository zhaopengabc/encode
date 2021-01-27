#ifndef NOVA_PUSH_STREAM_H
#define NOVA_PUSH_STREAM_H


enum Protocol
{
    RTSP,
    RTMP,
    HTTP
};

typedef struct inputFrameData
{
    int len;
    char *data;
}TY_INPUT_DATA;

typedef struct frameParam
{
    int channel;
    enum Protocol protocol; //push stream protocol : RTSP RTMP RTCP HTTP
    TY_INPUT_DATA frameData;
}TY_FRAME_PARAM;

typedef struct nova_push_stream
{   
    TY_FRAME_PARAM param;
    int (*pushStream)(TY_FRAME_PARAM *);
}TY_NOVA_PUSH_STREAM;


int nova_pushStream(TY_NOVA_PUSH_STREAM *novaPushStream);


#endif /*NOVA_ENCODE_H*/