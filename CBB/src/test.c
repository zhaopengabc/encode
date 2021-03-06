#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "nova_pushStream.h"
#include "nova_encode.h"


static MI_BOOL g_bExit = FALSE;

/*
*  Function name : ST_HandleSig 
*  In            : MI_S32 signo 捕获信号量
*  Out           : null
*  Return        : null
*  Description   : 捕获Ctrl+C的信号量。
*/
void ST_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        ST_INFO("catch Ctrl + C, exit normally\n");

        g_bExit = TRUE;
    }
}
/*
*  Function name : processYUVData 
*  In            : TY_YUV_DATA *srcData
*  Out           : TY_YUV_DATA *desData
*  Return        : 
*  Description   : 获取YUV回调函数，可以对YUV进行任何操作
*/
static int processYUVData(TY_YUV_DATA *desData,TY_YUV_DATA *srcData)
{
    int ret = 0;
    // desData->bufSize = srcData->bufSize;
    // desData->pVirAddr[0] = srcData->pVirAddr[0];
    // desData->pVirAddr[1] = srcData->pVirAddr[1];
    // desData->pVirAddr[2] = srcData->pVirAddr[2];

    memcpy(desData,srcData,sizeof(MI_SYS_FrameData_t));

    return ret;
}
/*
*  Function name : nova_encode_0 
*  In            : null
*  Out           : null
*  Return        : 
*  Description   : 操作第一路编码数据。特殊模式
*/
void* nova_encode_0(void *argc)
{
    signal(SIGINT,ST_HandleSig);

    TY_NOVA_PUSH_STREAM pushStream;


    TY_NOVA_ENCODER_QUEUE *encoder_queue;
    TY_NOVA_ENCODER encoder;
    memset(&encoder,0,sizeof(TY_NOVA_ENCODER));
    encoder.name = "sigmastar";
    encoder.option.channel = 0;
    encoder.option.YUVDataType = YUV_SEMIPLANAR_420;
    encoder.option.mediaType = AVMEDIA_TYPE_VIDEO;
    encoder.option.encodeFormat = AV_ENCDOE_ID_H264;
    encoder.option.resolutionRate.width = 1920;
    encoder.option.resolutionRate.height = 1080;
    encoder.option.resolutionRate.rate = 30;
    encoder.option.processYUVData = processYUVData;
    encoder_queue = nova_encoder(&encoder);
    encoder_queue->encoders->encodeCreate(&encoder);


    nova_pushStream(&pushStream);
    pushStream.param.channel = 0;
    pushStream.param.protocol = RTSP;
    pushStream.param.compressFormat = H264;


    while (!g_bExit)
    {
        encoder_queue->encoders->encodeOutputYUVData(&encoder);

        encoder_queue->encoders->encodeGetCompressData(&encoder);

        memset(&(pushStream.param.frameData),0,sizeof(pushStream.param.frameData));
        pushStream.param.frameData.len = encoder.option.compressData.frameLen;
        if(pushStream.param.frameData.len > 0)
        {
            pushStream.param.frameData.data = encoder.option.compressData.buf;
            pushStream.pushStream(&pushStream.param);
        }
        // sleep(1);
    }
    // printf("\n\n\n\n");

    encoder_queue->encoders->encodeDestroy(&encoder);

    nova_encoder_free(encoder_queue);
    pthread_exit(NULL);
}
/*
*  Function name : nova_encode_0 
*  In            : null
*  Out           : null
*  Return        : 
*  Description   : 操作第二路编码数据。普通模式。
*/
void* nova_encode_2(void *argc)
{
    signal(SIGINT,ST_HandleSig);

    TY_NOVA_PUSH_STREAM pushStream;


    TY_NOVA_ENCODER_QUEUE *encoder_queue;
    TY_NOVA_ENCODER encoder;
    memset(&encoder,0,sizeof(TY_NOVA_ENCODER));
    encoder.name = "sigmastar";
    encoder.option.channel = 2;
    encoder.option.YUVDataType = YUV_SEMIPLANAR_420;
    encoder.option.mediaType = AVMEDIA_TYPE_VIDEO;
    encoder.option.encodeFormat = AV_ENCDOE_ID_H264;
    encoder.option.resolutionRate.width = 1280;
    encoder.option.resolutionRate.height = 720;
    encoder.option.resolutionRate.rate = 30;

    encoder_queue = nova_encoder(&encoder);
    encoder_queue->encoders->encodeCreate_General(&encoder);

    nova_pushStream(&pushStream);
    pushStream.param.channel = 2;
    pushStream.param.protocol = RTSP;
    pushStream.param.compressFormat = H264;


    while (!g_bExit)
    {
        encoder_queue->encoders->encodeGetCompressData_General(&encoder);


        memset(&(pushStream.param.frameData),0,sizeof(pushStream.param.frameData));
        pushStream.param.frameData.len = encoder.option.compressData.frameLen;
        if(pushStream.param.frameData.len > 0)
        {
            pushStream.param.frameData.data = encoder.option.compressData.buf;
            pushStream.pushStream(&pushStream.param);
        }

        // sleep(1);
    }
    printf("\n\n\n\n");
    encoder_queue->encoders->encodeDestroy_General(&encoder);

    nova_encoder_free(encoder_queue);
    pthread_exit(NULL);

}
int main()
{
    signal(SIGINT,ST_HandleSig);
    pthread_t pThread_1;
    // pthread_t pThread_2;

    pthread_create(&pThread_1,NULL,nova_encode_0,NULL);
    // pthread_create(&pThread_2,NULL,nova_encode_2,NULL);
    while (!g_bExit)
    {
        sleep(1);
    }
    pthread_cancel(pThread_1);
    // pthread_cancel(pThread_2);
    return 0;
}