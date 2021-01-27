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
    encoder.name = "sigmastar";
    encoder.option.channel = 0;
    encoder.option.inputDataType = YUV_SEMIPLANAR_420;
    encoder.option.mediaType = AVMEDIA_TYPE_VIDEO;
    encoder.option.encodeFormat = AV_ENCDOE_ID_H264;
    encoder.option.resolutionRate.width = 1920;
    encoder.option.resolutionRate.height = 1080;
    encoder.option.resolutionRate.rate = 30;

    encoder_queue = nova_encoder(&encoder);
    encoder_queue->encoders->encodeCreate(&encoder);

    nova_pushStream(&pushStream);
    pushStream.param.channel = 0;
    pushStream.param.protocol = RTSP;
    while (!g_bExit)
    {
        sleep(1);
        encoder_queue->encoders->encodeGetData(&encoder);

        pushStream.param.frameData.len = encoder.option.outdata.frameLen;
        pushStream.param.frameData.data = encoder.option.outdata.buf;
        pushStream.pushStream(&pushStream.param);
    }

    encoder_queue->encoders->close(&encoder);

    nova_encoder_free(encoder_queue);
}
