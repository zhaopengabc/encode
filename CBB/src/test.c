#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "nova_pushStream.h"
#include "nova_encode.h"


static MI_BOOL g_bExit = FALSE;

void ST_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        ST_INFO("catch Ctrl + C, exit normally\n");

        g_bExit = TRUE;
    }
}

int main()
{
    signal(SIGINT,ST_HandleSig);

    TY_NOVA_PUSH_STREAM pushStream;


    TY_NOVA_ENCODER_QUEUE *encoder_queue;
    TY_NOVA_ENCODER encoder;
    memset(&encoder,0,sizeof(TY_NOVA_ENCODER));
    encoder.name = "sigmastar";
    encoder.option.channel = 2;
    encoder.option.inputDataType = YUV_SEMIPLANAR_420;
    encoder.option.mediaType = AVMEDIA_TYPE_VIDEO;
    encoder.option.encodeFormat = AV_ENCDOE_ID_H264;
    encoder.option.resolutionRate.width = 1920;
    encoder.option.resolutionRate.height = 1080;
    encoder.option.resolutionRate.rate = 30;

    encoder_queue = nova_encoder(&encoder);
    // encoder_queue->encoders->encodeCreate(&encoder);
    encoder_queue->encoders->encodeCreate_General(&encoder);

    nova_pushStream(&pushStream);
    pushStream.param.channel = 0;
    pushStream.param.protocol = RTSP;
    pushStream.param.compressFormat = H264;


    while (!g_bExit)
    {
        // encoder_queue->encoders->encodeOutputYUVData(&encoder);
        // memcpy(&(encoder.option.yuvInputData), &(encoder.option.yuvOutputData),sizeof(encoder.option.yuvOutputData));
        // encoder_queue->encoders->encodeInputYUVData(&encoder);

        encoder_queue->encoders->encodeGetCompressData_General(&encoder);
        // encoder_queue->encoders->encodeGetCompressData(&encoder);


        memset(&(pushStream.param.frameData),0,sizeof(pushStream.param.frameData));
        pushStream.param.frameData.len = encoder.option.outdata.frameLen;
        if(pushStream.param.frameData.len > 0)
        {
            pushStream.param.frameData.data = encoder.option.outdata.buf;
            pushStream.pushStream(&pushStream.param);
        }

        sleep(1);
    }
    // encoder_queue->encoders->encodeDestroy(&encoder);

    encoder_queue->encoders->encodeDestroy_General(&encoder);

    nova_encoder_free(encoder_queue);
}
