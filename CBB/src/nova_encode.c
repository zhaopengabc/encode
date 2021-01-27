/*
* create encoder
* Author : zhaopeng
* Time   : 2021/01/11
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>


#include "st_common.h"
#include "st_vif.h"
#include "st_vpe.h"
#include "st_venc.h"
#include "mi_divp.h"

#include "nova_encode.h"

static int setOption(TY_NOVA_ENCODER *encoder)
{
    printf("set option ... \n");
}

TY_ENCODE_INSIDE_PARAM gInside_param;
static MI_BOOL g_bExit = FALSE;

static int initVPE(TY_NOVA_ENCODER *encoder)
{
    ST_VPE_ChannelInfo_T stVpeChannelInfo;

    memset(&stVpeChannelInfo, 0, sizeof(ST_VPE_ChannelInfo_T));
    stVpeChannelInfo.u16VpeMaxW = gInside_param.resolutionRate.width;
    stVpeChannelInfo.u16VpeMaxH = gInside_param.resolutionRate.height;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 0;
    stVpeChannelInfo.u16VpeCropH = 0;
    stVpeChannelInfo.eRunningMode = E_MI_VPE_RUN_CAM_MODE;
    stVpeChannelInfo.eFormat = gInside_param.ePixFormat;
    stVpeChannelInfo.eHDRtype = gInside_param.eHdrType;
    stVpeChannelInfo.eBindSensorId = E_MI_VPE_SENSOR0;

    ST_Vpe_CreateChannel(gInside_param.VpeChn, &stVpeChannelInfo);
    ST_Vpe_StartChannel(gInside_param.VpeChn);
    ST_VPE_PortInfo_T stVpePortInfo;
    memset(&stVpePortInfo, 0, sizeof(ST_VPE_PortInfo_T));

    stVpePortInfo.DepVpeChannel = gInside_param.VpeChn;
    stVpePortInfo.u16OutputWidth = gInside_param.resolutionRate.width;
    stVpePortInfo.u16OutputHeight = gInside_param.resolutionRate.height;
    stVpePortInfo.ePixelFormat = gInside_param.inputYUVType;
    stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    ST_Vpe_StartPort(gInside_param.u32InputPort, &stVpePortInfo);

    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = gInside_param.vifDev;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.vifChn;
    stBindInfo.stSrcChnPort.u32PortId = gInside_param.u32InputPort;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.VpeChn;
    stBindInfo.stDstChnPort.u32PortId = gInside_param.u32InputPort;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_Bind(&stBindInfo);
}
static int initDIVP(TY_NOVA_ENCODER *encoder)
{
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    memset(&stDivpChnAttr, 0x00, sizeof(MI_DIVP_ChnAttr_t));

    stDivpChnAttr.bHorMirror = FALSE;
    stDivpChnAttr.bVerMirror = FALSE;
    stDivpChnAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X = 0;
    stDivpChnAttr.stCropRect.u16Y = 0;
    stDivpChnAttr.stCropRect.u16Width = 0;
    stDivpChnAttr.stCropRect.u16Height = 0;
    stDivpChnAttr.u32MaxWidth = gInside_param.resolutionRate.width;
    stDivpChnAttr.u32MaxHeight = gInside_param.resolutionRate.height;

    MI_DIVP_CreateChn(gInside_param.DivpChn, &stDivpChnAttr);
    MI_DIVP_StartChn(gInside_param.DivpChn);

    MI_DIVP_OutputPortAttr_t stDivpOutputPortAttr;
    stDivpOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutputPortAttr.ePixelFormat = gInside_param.inputYUVType;
    stDivpOutputPortAttr.u32Width = gInside_param.resolutionRate.width;
    stDivpOutputPortAttr.u32Height = gInside_param.resolutionRate.height;
    MI_DIVP_SetOutputPortAttr(gInside_param.DivpChn, &stDivpOutputPortAttr);

    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.VpeChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.DivpChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
}

static int initVENC(TY_NOVA_ENCODER *encoder)
{
    MI_VENC_CHN VencChn = encoder->channel;
    MI_U32 u32VencDevId = 0xff;

    MI_VENC_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0x0, sizeof(MI_VENC_ChnAttr_t));
    MI_U32 u32VenBitRate = 1024 * 1024 * 2;

    if (gInside_param.encodeFormat == AV_ENCDOE_ID_H264)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = gInside_param.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = gInside_param.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = gInside_param.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = gInside_param.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = gInside_param.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    }
    else if (gInside_param.encodeFormat == AV_ENCDOE_ID_H265)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = gInside_param.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = gInside_param.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = gInside_param.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = gInside_param.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = gInside_param.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    }

    ST_Venc_CreateChannel(gInside_param.VencChn, &stChnAttr);
    ST_Venc_StartChannel(gInside_param.VencChn);

    MI_VENC_GetChnDevid(gInside_param.VencChn, &gInside_param.u32VencDevId);
    // vpe port 2 can not attach osd, so use divp
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.DivpChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = gInside_param.u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_Bind(&stBindInfo);
}

static int initVIF(TY_NOVA_ENCODER *encoder)
{
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
    stDevAttr.eIntfMode = stPad0Info.eIntfMode;
    stDevAttr.eWorkMode = E_MI_VIF_WORK_MODE_RGB_FRAMEMODE;
    stDevAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;
    stDevAttr.eDataSeq = stPad0Info.unIntfAttr.stMipiAttr.eDataYUVOrder;
    stDevAttr.eBitOrder = E_MI_VIF_BITORDER_NORMAL;

    ExecFunc(MI_VIF_SetDevAttr(gInside_param.vifDev, &stDevAttr), MI_SUCCESS);
    ExecFunc(MI_VIF_EnableDev(gInside_param.vifDev), MI_SUCCESS);

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
    STCHECKRESULT(ST_Vif_CreatePort(gInside_param.vifChn, 0, &stVifPortInfoInfo));
    STCHECKRESULT(ST_Vif_StartPort(0, gInside_param.vifChn, 0));

    MI_ModuleId_e eVifModeId = E_MI_MODULE_ID_VIF;
    MI_U8 u8MmaHeap[128] = "mma_heap_name0";
    MI_SYS_SetChnMMAConf(eVifModeId, 0, encoder->channel, u8MmaHeap);

    gInside_param.ePixFormat = ePixFormat;
}
static int encodeGetData(TY_NOVA_ENCODER *encoder)
{
    MI_SYS_BufInfo_t stBufInfo;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    MI_U32 u32DevId = 0;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_CHN vencChn;

    s32Ret = MI_VENC_GetChnDevid(vencChn, &u32DevId);
    if (MI_SUCCESS != s32Ret)
    {
        ST_INFO("MI_VENC_GetChnDevid %d error, %X\n", vencChn, s32Ret);
    }

    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    memset(&stStream, 0, sizeof(stStream));
    memset(&stPack, 0, sizeof(stPack));
    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    s32Ret = MI_VENC_Query(vencChn, &stStat);
    if (s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
    {
        return 0;
    }

    s32Ret = MI_VENC_GetStream(vencChn, &stStream, 40);
    if (MI_SUCCESS == s32Ret)
    {
        len = stStream.pstPack[0].u32Len;
        // memcpy(ucpBuf, stStream.pstPack[0].pu8Addr, MIN(len, BufLen));
        printf("len : %d \n", len);
        s32Ret = MI_VENC_ReleaseStream(vencChn, &stStream);
        if (s32Ret != MI_SUCCESS)
        {
            ST_WARN("RELEASE venc buffer fail\n");
        }

        return len;
    }
}
static int encodePutData(TY_NOVA_ENCODER *encoder)
{
}

static int deinitVPE(TY_NOVA_ENCODER *encoder)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = gInside_param.vifDev;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.vifChn;
    stBindInfo.stSrcChnPort.u32PortId = gInside_param.u32InputPort;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.VpeChn;
    stBindInfo.stDstChnPort.u32PortId = gInside_param.u32InputPort;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_UnBind(&stBindInfo);

    ST_Vif_StopPort(gInside_param.vifChn,0);
    ST_Vif_DisableDev(gInside_param.vifDev);
}
static int deinitDIVP(TY_NOVA_ENCODER *encoder)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.VpeChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.DivpChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_UnBind(&stBindInfo);

    ST_Vpe_StopPort(gInside_param.VpeChn, 0);
    ST_Vpe_StopChannel(gInside_param.VpeChn);
    ST_Vpe_DestroyChannel(gInside_param.VpeChn);

}
static int deinitVENC(TY_NOVA_ENCODER *encoder)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.DivpChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = gInside_param.u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = gInside_param.VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    ST_Sys_UnBind(&stBindInfo);


    ST_Venc_StopChannel(gInside_param.VencChn);
    ST_Venc_DestoryChannel(gInside_param.VencChn);

    MI_DIVP_StopChn(gInside_param.DivpChn);  
    MI_DIVP_DestroyChn(gInside_param.DivpChn);
}

static int encodeCreate(TY_NOVA_ENCODER *encoder)
{
    memset(&gInside_param, 0, sizeof(TY_ENCODE_INSIDE_PARAM));
    gInside_param.eHdrType = E_MI_VPE_HDR_TYPE_OFF;
    gInside_param.vifDev = encoder->channel;
    gInside_param.vifChn = gInside_param.vifDev * 4;
    gInside_param.VpeChn = gInside_param.vifDev;
    gInside_param.DivpChn = gInside_param.vifDev;
    gInside_param.VencChn = gInside_param.vifDev * 2;
    gInside_param.u32InputPort = gInside_param.vifDev;

    gInside_param.resolutionRate.width = encoder->option.resolutionRate.width;
    gInside_param.resolutionRate.height = encoder->option.resolutionRate.height;
    gInside_param.resolutionRate.rate = encoder->option.resolutionRate.rate;

    gInside_param.encodeFormat = encoder->option.encodeFormat;

    if (encoder->option.inputDataType == YUV_SEMIPLANAR_420)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    }
    else if (encoder->option.inputDataType == YUV_SEMIPLANAR_422)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    }

    initVIF(encoder);
    initVPE(encoder);
    initDIVP(encoder);
    initVENC(encoder);
}

static int encode_close(TY_NOVA_ENCODER *encoder)
{
    deinitVENC(encoder);
    deinitDIVP(encoder);
    deinitVPE(encoder);
    STCHECKRESULT(ST_Sys_Exit());
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

    NOVA_encoder->close = encode_close;
    NOVA_encoder->encodeCreate = encodeCreate;
    NOVA_encoder->encodeGetData = encodeGetData;

    return encode_queue;
}
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
    TY_NOVA_ENCODER_QUEUE *encoder_queue;
    TY_NOVA_ENCODER encoder;
    encoder.name = "sigmastar";
    encoder.channel = 0;
    // encoder.option.channel = 0;
    encoder.option.inputDataType = YUV_SEMIPLANAR_420;
    encoder.option.mediaType = AVMEDIA_TYPE_VIDEO;
    encoder.option.encodeFormat = AV_ENCDOE_ID_H264;
    encoder.option.resolutionRate.width = 1920;
    encoder.option.resolutionRate.height = 1080;
    encoder.option.resolutionRate.rate = 30;

    encoder_queue = nova_encoder(&encoder);
    encoder_queue->encoders->encodeCreate(&encoder);

    while (!g_bExit)
    {
        sleep(1);
        encoder_queue->encoders->encodeGetData(&encoder);
    }

    encoder_queue->encoders->close(&encoder);

    nova_encoder_free(encoder_queue);
}