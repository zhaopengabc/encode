#include <stdlib.h>
#include <stdio.h>

#include "nova_pushStream.h"

static int pushStream(TY_FRAME_PARAM *novaPushStream)
{

    printf("novaPushStream channel : %d \n",novaPushStream->channel);
    printf("novaPushStream protocol : %d \n",novaPushStream->protocol);
    printf("novaPushStream len : %d \n",novaPushStream->frameData.len);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[0]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[1]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[2]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[3]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[4]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[5]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[6]);
    printf("novaPushStream data : %d \n",novaPushStream->frameData.data[7]);

}


int nova_pushStream(TY_NOVA_PUSH_STREAM *novaPushStream)
{
    novaPushStream->pushStream = pushStream;
}

// int main()
// {
//     TY_NOVA_PUSH_STREAM pushStream;
//     pushStream.param.channel = 0;
//     pushStream.param.protocol = RTSP;
//     pushStream.param.frameData.len = 1;

//     nova_pushStream(&pushStream);

//     pushStream.pushStream(&pushStream.param);

//     return 0;
// }
