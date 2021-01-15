/*
* create encoder
* Author : zhaopeng
* Time   : 2021/01/11
*/
#include <stdlib.h>
#include <stdio.h>

#include "st_vif.h"
#include "st_vpe.h"
#include "st_venc.h"
#include "nova_encode.h"

static int setOption(TY_NOVA_ENCODER *encoder)
{
    printf("set option ... \n");
}

static int init(TY_NOVA_ENCODER *encoder)
{
    printf("init ... \n");
    STCHECKRESULT(ST_Sys_Init());

    MI_SNR_PAD_ID_e eSnrPadId;
    MI_SNR_PADInfo_t stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_PixelFormat_e ePixFormat;

    eSnrPadId = E_MI_SNR_PAD_ID_0;
    MI_SNR_SetRes(eSnrPadId, 0);
    MI_SNR_Enable(eSnrPadId);
    MI_SNR_GetPadInfo(eSnrPadId, &stPad0Info);
    MI_SNR_GetPlaneInfo(eSnrPadId, 0, &stSnrPlane0Info);

    u32CapWidth = stSnrPlane0Info.stCapRect.u16Width;
    u32CapHeight = stSnrPlane0Info.stCapRect.u16Height;
    eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);

    MI_VIF_DevAttr_t stDevAttr;
    memset(&stDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));
    stDevAttr.eDataSeq = stPad0Info.unIntfAttr.stMipiAttr.eDataYUVOrder;
    stDevAttr.eBitOrder = E_MI_VIF_BITORDER_NORMAL;

    ExecFunc(MI_VIF_SetDevAttr(encoder->channel, &stDevAttr), MI_SUCCESS);
    ExecFunc(MI_VIF_EnableDev(encoder->channel), MI_SUCCESS);

    ST_VIF_PortInfo_T stVifPortInfoInfo;
    memset(&stVifPortInfoInfo, 0, sizeof(ST_VIF_PortInfo_T));
    stVifPortInfoInfo.u32RectX = stSnrPlane0Info.stCapRect.u16X;
    stVifPortInfoInfo.u32RectY = stSnrPlane0Info.stCapRect.u16Y;
    stVifPortInfoInfo.u32RectWidth = u32CapWidth;
    stVifPortInfoInfo.u32RectHeight = u32CapHeight;
    stVifPortInfoInfo.u32DestWidth = u32CapWidth;
    stVifPortInfoInfo.u32DestHeight = u32CapHeight;
    stVifPortInfoInfo.eFrameRate = eFrameRate;
    stVifPortInfoInfo.ePixFormat = ePixFormat; //E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_GR;
    STCHECKRESULT(ST_Vif_CreatePort(encoder->channel, 0, &stVifPortInfoInfo));
    STCHECKRESULT(ST_Vif_StartPort(0, encoder->channel, 0));

    MI_ModuleId_e eVifModeId = E_MI_MODULE_ID_VIF;
    MI_U8 u8MmaHeap[128] = "mma_heap_name0";
    MI_SYS_SetChnMMAConf(eVifModeId, 0,encoder->channel, u8MmaHeap);

    ST_VPE_ChannelInfo_T stVpeChannelInfo;
    MI_VPE_HDRType_e eVpeHdrType = E_MI_VPE_HDR_TYPE_OFF;

    memset(&stVpeChannelInfo, 0, sizeof(ST_VPE_ChannelInfo_T));
    stVpeChannelInfo.u16VpeMaxW = u32CapWidth;
    stVpeChannelInfo.u16VpeMaxH = u32CapHeight;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 0;
    stVpeChannelInfo.u16VpeCropH = 0;
    stVpeChannelInfo.eRunningMode = E_MI_VPE_RUN_CAM_MODE;
    stVpeChannelInfo.eFormat = ePixFormat; //E_MI_SYS_PIXEL_FRAME_RGB_BAYER_12BPP_GR;
    stVpeChannelInfo.eHDRtype = eVpeHdrType;


    
}

static int encode_close(TY_NOVA_ENCODER *encoder)
{
    printf("close .... \n");

    STCHECKRESULT(ST_Sys_Exit());
}

static int recevie_packet()
{
    printf("recevie packet .... \n");
}

static int send_frame()
{
    printf("send frame ... \n");
}

TY_NOVA_ENCODER_QUEUE *nova_encoder_alloc(unsigned int decoderSize)
{
    TY_NOVA_ENCODER_QUEUE *decoderQueue = (TY_NOVA_ENCODER_QUEUE *)calloc(decoderSize, sizeof(TY_NOVA_ENCODER_QUEUE));
    if (!decoderQueue)
    {
        return NULL;
    }
    else
    {
        return decoderQueue;
    }
}
void nova_encoder_free(TY_NOVA_ENCODER_QUEUE *decoderQueue)
{
    if (!decoderQueue)
    {
        return;
    }
    else
    {
        free(decoderQueue);
    }
}

TY_NOVA_ENCODER_QUEUE *nova_encoder(TY_NOVA_ENCODER *NOVA_encoder)
{
    TY_NOVA_ENCODER_QUEUE *encode_queue;
    encode_queue = nova_encoder_alloc(1);
    encode_queue->encoders = NOVA_encoder;

    NOVA_encoder->init_encode = init;
    NOVA_encoder->set_encode_option = setOption;
    NOVA_encoder->receive_packet = recevie_packet;
    NOVA_encoder->send_frame = send_frame;
    NOVA_encoder->close = encode_close;

    return encode_queue;
}

int main()
{
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
    encoder_queue->encoders->init_encode(&encoder);
    encoder_queue->encoders->set_encode_option(&encoder);
    encoder_queue->encoders->receive_packet(&encoder);
    encoder_queue->encoders->send_frame(&encoder);
    encoder_queue->encoders->close(&encoder);

    nova_encoder_free(encoder_queue);
}