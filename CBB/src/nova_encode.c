/*
* create encoder
* Author : zhaopeng
* Time   : 2021/01/11
* description: sigmastar SSC339G platform code CBB.
* 1.create encode include : 
    1) init VIF;
    2) init VPE . VIF bind VPE;
    3) init DIVP. VPE bind DIVP;
    4) init VENC， DIVP bind VENC;
  2. get frame data : H246 or H265.
  3. destory encode.
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "nova_encode.h"

// static MI_BOOL g_bExit = FALSE;
static int initVIF(TY_ENCODE_INSIDE_PARAM gInside_param)
{
    STCHECKRESULT(ST_Sys_Init());

    MI_SNR_PAD_ID_e eSnrPadId;
    MI_SNR_PADInfo_t stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_PixelFormat_e ePixFormat;

    if(gInside_param.vifChn == 0)
    {
        eSnrPadId = E_MI_SNR_PAD_ID_0;
    }
    else
    {
        eSnrPadId = E_MI_SNR_PAD_ID_1;
    }
    
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
    MI_SYS_SetChnMMAConf(eVifModeId, 0, gInside_param.DivpChn, u8MmaHeap);

    gInside_param.ePixFormat = ePixFormat;
}
static int initVPE(TY_ENCODE_INSIDE_PARAM gInside_param)
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
static int initDIVP(TY_ENCODE_INSIDE_PARAM gInside_param)
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

    // ST_Sys_BindInfo_T stBindInfo;
    // memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    // stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    // stBindInfo.stSrcChnPort.u32DevId = 0;
    // stBindInfo.stSrcChnPort.u32ChnId = gInside_param.VpeChn;
    // stBindInfo.stSrcChnPort.u32PortId = 0;
    // stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    // stBindInfo.stDstChnPort.u32DevId = 0;
    // stBindInfo.stDstChnPort.u32ChnId = gInside_param.DivpChn;
    // stBindInfo.stDstChnPort.u32PortId = 0;
    // stBindInfo.u32SrcFrmrate = 30;
    // stBindInfo.u32DstFrmrate = 30;
    // stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    // STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
}

static int initVENC(TY_ENCODE_INSIDE_PARAM gInside_param)
{
    MI_VENC_CHN VencChn = gInside_param.VencChn;
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

static int deinitVPE(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = insideParam->vifDev;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam->vifChn;
    stBindInfo.stSrcChnPort.u32PortId = insideParam->u32InputPort;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = insideParam->VpeChn;
    stBindInfo.stDstChnPort.u32PortId = insideParam->u32InputPort;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_UnBind(&stBindInfo);

    ST_Vpe_StopPort(insideParam->VpeChn, 0);
    ST_Vpe_StopChannel(insideParam->VpeChn);
    ST_Vpe_DestroyChannel(insideParam->VpeChn);

    ST_Vif_StopPort(insideParam->vifChn, 0);
    ST_Vif_DisableDev(insideParam->vifDev);
}
static int deinitDIVP(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam->VpeChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = insideParam->DivpChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_UnBind(&stBindInfo);
    MI_DIVP_StopChn(insideParam->DivpChn);
    MI_DIVP_DestroyChn(insideParam->DivpChn);
}
static int deinitVENC(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam->DivpChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = insideParam->u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = insideParam->VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    ST_Sys_UnBind(&stBindInfo);

    ST_Venc_StopChannel(insideParam->VencChn);
    ST_Venc_DestoryChannel(insideParam->VencChn);
}

static int encodeGetCompressData(TY_NOVA_ENCODER *encode)
{
    MI_SYS_BufInfo_t stBufInfo;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    MI_U32 u32DevId = 0;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_CHN vencChn;
    char *data;

    TY_NOVA_ENCODER *encoder;
    encoder = (TY_NOVA_ENCODER *)encode;
    vencChn = encoder->insideParam.VencChn;

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
        printf("len : %d \n", len);
        encoder->option.outdata.frameLen = len;
        data = (char *)malloc(len);
        memcpy(data, stStream.pstPack[0].pu8Addr, len);
        encoder->option.outdata.buf = data;
        s32Ret = MI_VENC_ReleaseStream(vencChn, &stStream);
        if (s32Ret != MI_SUCCESS)
        {
            ST_WARN("RELEASE venc buffer fail\n");
        }
        free(data);
        return len;
    }
}
static int encodeOutputYUVData(TY_NOVA_ENCODER *encode)
{
    TY_NOVA_ENCODER *encoder;
    encoder = (TY_NOVA_ENCODER *)encode;

    MI_S32 s32Fd = 0;
    MI_U32 ret = 0;
    MI_SYS_BufInfo_t stBufInfo;
    fd_set read_fds;
    struct timeval TimeoutVal;
    MI_SYS_BUF_HANDLE hHandle;

    MI_SYS_ChnPort_t stChnPort;
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VPE;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = encoder->insideParam.VpeChn;
    stChnPort.u32PortId = 0;

    ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 3);
    if (MI_SUCCESS != ret)
    {
        ST_ERR("MI_SYS_SetChnOutputPortDepth err:%x, chn:%d,port:%d\n", ret, stChnPort.u32ChnId, stChnPort.u32PortId);
        return -1;
    }

    ret = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (MI_SUCCESS != ret)
    {
        ST_ERR("MI_SYS_GetFd 0, error, %X\n", ret);
        return -1;
    }

    // while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);
        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;
        ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (ret < 0)
        {
            ST_ERR("select failed!\n"); // usleep(10 * 1000); continue;
            // continue;
        }
        else if (ret == 0)
        {
            ST_ERR("get vif frame time out\n"); //usleep(10 * 1000);
            // continue;
        }
        else
        {
            if (FD_ISSET(s32Fd, &read_fds))
            {
                memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
                ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle);
                if (ret != MI_SUCCESS)
                {
                    MI_SYS_ChnOutputPortPutBuf(hHandle);
                    // continue;
                }
                else
                {
                    int size = stBufInfo.stFrameData.u32BufSize;
                    printf("size : %d \n", size);
                    encode->option.yuvOutputData.bufSize = stBufInfo.stFrameData.u32BufSize;
                    encode->option.yuvOutputData.pVireAddr[0] = stBufInfo.stFrameData.pVirAddr[0];

                    MI_SYS_ChnOutputPortPutBuf(hHandle);
                }
            }
        }
    }
}

static int encodeInputYUVData(TY_NOVA_ENCODER *encode)
{
    TY_NOVA_ENCODER *encoder;
    encoder = (TY_NOVA_ENCODER *)encode;

    MI_S32 s32Fd = 0;
    MI_U32 ret = 0;
    MI_SYS_BufInfo_t stBufInfo;
    fd_set read_fds;
    struct timeval TimeoutVal;

    struct timeval stTv;
    MI_SYS_ChnPort_t stDivpChnInput;
    MI_SYS_BufConf_t stDivpBufConf;
    MI_SYS_BufInfo_t stDivpBufInfo;
    MI_SYS_BUF_HANDLE hDivpHandle;

    memset(&stDivpChnInput, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnInput.eModId = E_MI_MODULE_ID_DIVP;
    stDivpChnInput.u32DevId = 0;
    stDivpChnInput.u32ChnId = encode->insideParam.DivpChn;
    stDivpChnInput.u32PortId = 0;

    memset(&stDivpBufConf, 0, sizeof(MI_SYS_BufConf_t));
    stDivpBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    gettimeofday(&stTv, NULL);
    stDivpBufConf.u64TargetPts = stTv.tv_sec * 1000000 + stTv.tv_usec;
    stDivpBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stDivpBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stDivpBufConf.stFrameCfg.u16Width = encode->option.resolutionRate.width;
    stDivpBufConf.stFrameCfg.u16Height = encode->option.resolutionRate.height;

    ret = MI_SYS_ChnInputPortGetBuf(&stDivpChnInput, &stDivpBufConf, &stDivpBufInfo, &hDivpHandle, 0);
    if (ret == 0)
    {
        stDivpBufInfo.stFrameData.pVirAddr[0] = encode->option.yuvInputData.pVireAddr[0];
    }

    ret = MI_SYS_ChnInputPortPutBuf(hDivpHandle, &stDivpBufInfo, FALSE);

    return ret;
}
static int encodeDestroy(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;
    deinitVENC(&(encode->insideParam));
    deinitDIVP(&(encode->insideParam));
    deinitVPE(&(encode->insideParam));
    ST_Sys_Exit();
}


static int encodeCreate(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;

    TY_ENCODE_OPTION option = encode->option;

    TY_ENCODE_INSIDE_PARAM gInside_param;
    memset(&gInside_param, 0, sizeof(TY_ENCODE_INSIDE_PARAM));
    gInside_param.eHdrType = E_MI_VPE_HDR_TYPE_OFF;
    gInside_param.vifDev = option.channel;
    gInside_param.vifChn = gInside_param.vifDev * 4;
    gInside_param.VpeChn = gInside_param.vifDev;
    gInside_param.DivpChn = gInside_param.vifDev;
    gInside_param.VencChn = gInside_param.vifDev;
    gInside_param.u32InputPort = 0;

    gInside_param.resolutionRate.width = option.resolutionRate.width;
    gInside_param.resolutionRate.height = option.resolutionRate.height;
    gInside_param.resolutionRate.rate = option.resolutionRate.rate;

    gInside_param.encodeFormat = option.encodeFormat;

    if (option.inputDataType == YUV_SEMIPLANAR_420)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    }
    else if (option.inputDataType == YUV_SEMIPLANAR_422)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    }
    encode->insideParam = gInside_param;

    initVIF(gInside_param);
    initVPE(gInside_param);
    initDIVP(gInside_param);
    initVENC(gInside_param);
}

static int initVIF_General(TY_ENCODE_INSIDE_PARAM gInside_param)
{
    initVIF(gInside_param);
}
static int initVPE_General(TY_ENCODE_INSIDE_PARAM gInside_param)
{
    initVPE(gInside_param);
}
static int initVENC_General(TY_ENCODE_INSIDE_PARAM gInside_param)
{
    MI_VENC_CHN VencChn = gInside_param.VencChn;
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
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = gInside_param.VpeChn;
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
static int encodeCreate_General(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;

    TY_ENCODE_OPTION option = encode->option;

    TY_ENCODE_INSIDE_PARAM gInside_param;
    memset(&gInside_param, 0, sizeof(TY_ENCODE_INSIDE_PARAM));
    gInside_param.eHdrType = E_MI_VPE_HDR_TYPE_OFF;
    gInside_param.vifDev = option.channel;
    gInside_param.vifChn = gInside_param.vifDev * 4;
    gInside_param.VpeChn = gInside_param.vifDev;
    gInside_param.DivpChn = gInside_param.vifDev;
    gInside_param.VencChn = gInside_param.vifDev;
    gInside_param.u32InputPort = 0;

    gInside_param.resolutionRate.width = option.resolutionRate.width;
    gInside_param.resolutionRate.height = option.resolutionRate.height;
    gInside_param.resolutionRate.rate = option.resolutionRate.rate;

    gInside_param.encodeFormat = option.encodeFormat;

    if (option.inputDataType == YUV_SEMIPLANAR_420)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    }
    else if (option.inputDataType == YUV_SEMIPLANAR_422)
    {
        gInside_param.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    }
    encode->insideParam = gInside_param;

    initVIF_General(gInside_param);
    initVPE_General(gInside_param);
    initVENC_General(gInside_param);
}
static int encodeGetCompressData_General(TY_NOVA_ENCODER *encode)
{
    encodeGetCompressData(encode);
}
static int deinitVENC_General(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam->VpeChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = insideParam->u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = insideParam->VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    ST_Sys_UnBind(&stBindInfo);

    ST_Venc_StopChannel(insideParam->VencChn);
    ST_Venc_DestoryChannel(insideParam->VencChn);
}
static int deinitVPE_General(TY_ENCODE_INSIDE_PARAM *gInside_param)
{
    deinitVPE(gInside_param);
}
static int encodeDestroy_General(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;
    deinitVENC_General(&(encode->insideParam));
    deinitVPE_General(&(encode->insideParam));
    ST_Sys_Exit();
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

    NOVA_encoder->encodeDestroy = encodeDestroy;
    NOVA_encoder->encodeCreate = encodeCreate;
    NOVA_encoder->encodeGetCompressData = encodeGetCompressData;
    NOVA_encoder->encodeOutputYUVData = encodeOutputYUVData;
    NOVA_encoder->encodeInputYUVData = encodeInputYUVData;

    NOVA_encoder->encodeCreate_General = encodeCreate_General;
    NOVA_encoder->encodeGetCompressData_General = encodeGetCompressData_General;
    NOVA_encoder->encodeDestroy_General = encodeDestroy_General;


    return encode_queue;
}

/*
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
    encoder.option.channel = 0;
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
        // printf("aaaa \n");
        encoder_queue->encoders->encodeGetData(&encoder);
        // printf("data len : %d \n",encoder.option.outdata.frameLen);
        // printf("data [0] : %d \n",encoder.option.outdata.buf[0]);
    }

    encoder_queue->encoders->close(&encoder);

    nova_encoder_free(encoder_queue);
}
*/