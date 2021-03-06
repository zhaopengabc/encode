#include <stdlib.h>
#include <stdio.h>

#include "nova_pushStream.h"

static int pushStream(TY_FRAME_PARAM *novaPushStream)
{

    // printf("\n\n\nnovaPushStream channel : %d \n",novaPushStream->channel);
    // printf("novaPushStream protocol : %d \n",novaPushStream->protocol);
    // printf("novaPushStream len : %d \n",novaPushStream->frameData.len);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[0]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[1]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[2]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[3]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[4]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[5]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[6]);
    // printf("novaPushStream data : %d \n",novaPushStream->frameData.data[7]);


    FILE* fpSave;
	char filename[32] = {0};
	sprintf(filename, "%s", "./test.h264");
	if ((fpSave = fopen(filename, "ab")) == NULL) //h264保存的文件名  
	{
		printf("Unable to open %s...\n", filename);
		return -1;
	}
	fwrite(novaPushStream->frameData.data, 1, novaPushStream->frameData.len, fpSave);//写数据到文件中
	fclose(fpSave);

    return 0;
}


int nova_pushStream(TY_NOVA_PUSH_STREAM *novaPushStream)
{
    novaPushStream->pushStream = pushStream;

    return 0;
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
