/*
* create encoder
* Author : nova zhaopeng
* Time   : 2021/01/11
* description: sigmastar SSC339G platform code CBB. divided into General mode and special mode
* General mode : 
    1.create General encode include : 
        1) init VIF;
        2) init VPE . VIF bind VPE;
        4) init VENC， VPE bind VENC;
    2. get frame data : H246 or H265.
    3. destory encode.
* Special mode : The user can get YUV data and manually feed the YUV data into the encoder
    1. create encode include 
        1) init VIF;
        2) init VPE . VIF bind VPE;
        3) init DIVP. 
        4) init VENC， DIVP bind VENC;
    2. get YUV data from VPE mode;
    3. put YUV data to DIVP mode;DIVP mode can implement the zoom.
    4. get frame data : H246 or H265.
    5. destory encode.
*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "nova_encode.h"

/*
*  Function name : initVIF 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 初始化VIF模块，在初始化VIF之前，需要初始化sensor，来作为数据源。
*/
static int initVIF(TY_ENCODE_INSIDE_PARAM insideParam)
{
    int ret = 0;
    STCHECKRESULT(ST_Sys_Init());

    MI_SNR_PAD_ID_e eSnrPadId;
    MI_SNR_PADInfo_t stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_PixelFormat_e ePixFormat;

    if(insideParam.vifChn == 0)
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

    ExecFunc(MI_VIF_SetDevAttr(insideParam.vifDev, &stDevAttr), MI_SUCCESS);
    ExecFunc(MI_VIF_EnableDev(insideParam.vifDev), MI_SUCCESS);

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
    STCHECKRESULT(ST_Vif_CreatePort(insideParam.vifChn, 0, &stVifPortInfoInfo));
    STCHECKRESULT(ST_Vif_StartPort(0, insideParam.vifChn, 0));

    MI_ModuleId_e eVifModeId = E_MI_MODULE_ID_VIF;
    MI_U8 u8MmaHeap[128] = "mma_heap_name0";
    MI_SYS_SetChnMMAConf(eVifModeId, 0, insideParam.DivpChn, u8MmaHeap);

    insideParam.ePixFormat = ePixFormat;

    return ret;
}

/*
*  Function name : initVPE 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 初始化VPE模块，并且设置VIF绑定VPE。
*/
static int initVPE(TY_ENCODE_INSIDE_PARAM insideParam)
{
    ST_VPE_ChannelInfo_T stVpeChannelInfo;

    memset(&stVpeChannelInfo, 0, sizeof(ST_VPE_ChannelInfo_T));
    stVpeChannelInfo.u16VpeMaxW = insideParam.resolutionRate.width;
    stVpeChannelInfo.u16VpeMaxH = insideParam.resolutionRate.height;
    stVpeChannelInfo.u32X = 0;
    stVpeChannelInfo.u32Y = 0;
    stVpeChannelInfo.u16VpeCropW = 0;
    stVpeChannelInfo.u16VpeCropH = 0;
    stVpeChannelInfo.eRunningMode = E_MI_VPE_RUN_CAM_MODE;
    stVpeChannelInfo.eFormat = insideParam.ePixFormat;
    stVpeChannelInfo.eHDRtype = insideParam.eHdrType;
    stVpeChannelInfo.eBindSensorId = E_MI_VPE_SENSOR0;

    ST_Vpe_CreateChannel(insideParam.VpeChn, &stVpeChannelInfo);
    ST_Vpe_StartChannel(insideParam.VpeChn);
    ST_VPE_PortInfo_T stVpePortInfo;
    memset(&stVpePortInfo, 0, sizeof(ST_VPE_PortInfo_T));

    stVpePortInfo.DepVpeChannel = insideParam.VpeChn;
    stVpePortInfo.u16OutputWidth = insideParam.resolutionRate.width;
    stVpePortInfo.u16OutputHeight = insideParam.resolutionRate.height;
    stVpePortInfo.ePixelFormat = insideParam.inputYUVType;
    stVpePortInfo.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    ST_Vpe_StartPort(insideParam.u32InputPort, &stVpePortInfo);

    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = insideParam.vifDev;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam.vifChn;
    stBindInfo.stSrcChnPort.u32PortId = insideParam.u32InputPort;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = insideParam.VpeChn;
    stBindInfo.stDstChnPort.u32PortId = insideParam.u32InputPort;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_Bind(&stBindInfo);
}
/*
*  Function name : initDIVP 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 初始化DIVP模块，但是不设置DIVP和VPE的绑定关系，因为需要从VPE的模块中获取YUV数据，然后在用户层做相关操作，然后
*                  再把YUV送给DIVP模块。
*/
static int initDIVP(TY_ENCODE_INSIDE_PARAM insideParam)
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
    stDivpChnAttr.u32MaxWidth = insideParam.resolutionRate.width;
    stDivpChnAttr.u32MaxHeight = insideParam.resolutionRate.height;

    MI_DIVP_CreateChn(insideParam.DivpChn, &stDivpChnAttr);
    MI_DIVP_StartChn(insideParam.DivpChn);

    MI_DIVP_OutputPortAttr_t stDivpOutputPortAttr;
    stDivpOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutputPortAttr.ePixelFormat = insideParam.inputYUVType;
    stDivpOutputPortAttr.u32Width = insideParam.resolutionRate.width;
    stDivpOutputPortAttr.u32Height = insideParam.resolutionRate.height;
    MI_DIVP_SetOutputPortAttr(insideParam.DivpChn, &stDivpOutputPortAttr);

    // ST_Sys_BindInfo_T stBindInfo;
    // memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    // stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    // stBindInfo.stSrcChnPort.u32DevId = 0;
    // stBindInfo.stSrcChnPort.u32ChnId = insideParam.VpeChn;
    // stBindInfo.stSrcChnPort.u32PortId = 0;
    // stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    // stBindInfo.stDstChnPort.u32DevId = 0;
    // stBindInfo.stDstChnPort.u32ChnId = insideParam.DivpChn;
    // stBindInfo.stDstChnPort.u32PortId = 0;
    // stBindInfo.u32SrcFrmrate = 30;
    // stBindInfo.u32DstFrmrate = 30;
    // stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    // STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
}
/*
*  Function name : initVENC 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 初始化VENC模块，并且设置DIVP和VENC模块的绑定。
*/
static int initVENC(TY_ENCODE_INSIDE_PARAM insideParam)
{
    MI_VENC_CHN VencChn = insideParam.VencChn;
    MI_U32 u32VencDevId = 0xff;

    MI_VENC_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0x0, sizeof(MI_VENC_ChnAttr_t));
    MI_U32 u32VenBitRate = 1024 * 1024 * 2;

    if (insideParam.encodeFormat == AV_ENCDOE_ID_H264)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = insideParam.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    }
    else if (insideParam.encodeFormat == AV_ENCDOE_ID_H265)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = insideParam.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    }

    ST_Venc_CreateChannel(insideParam.VencChn, &stChnAttr);
    ST_Venc_StartChannel(insideParam.VencChn);

    MI_VENC_GetChnDevid(insideParam.VencChn, &insideParam.u32VencDevId);
    // vpe port 2 can not attach osd, so use divp
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam.DivpChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = insideParam.u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = insideParam.VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_Bind(&stBindInfo);
}
/*
*  Function name : deinitVPE 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM *insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 去初始化VPE模块，解绑定VIF和VPE模块，销毁VIF和VPE相关的通道设置。
*/
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
/*
*  Function name : deinitDIVP 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM *insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 去初始化DIVP模块
*/
static int deinitDIVP(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    // ST_Sys_BindInfo_T stBindInfo;
    // memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    // stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    // stBindInfo.stSrcChnPort.u32DevId = 0;
    // stBindInfo.stSrcChnPort.u32ChnId = insideParam->VpeChn;
    // stBindInfo.stSrcChnPort.u32PortId = 0;
    // stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    // stBindInfo.stDstChnPort.u32DevId = 0;
    // stBindInfo.stDstChnPort.u32ChnId = insideParam->DivpChn;
    // stBindInfo.stDstChnPort.u32PortId = 0;
    // stBindInfo.u32SrcFrmrate = 30;
    // stBindInfo.u32DstFrmrate = 30;
    // stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    // ST_Sys_UnBind(&stBindInfo);
    MI_DIVP_StopChn(insideParam->DivpChn);
    MI_DIVP_DestroyChn(insideParam->DivpChn);
}
/*
*  Function name : deinitVENC 内部函数
*  In            : Y_ENCODE_INSIDE_PARAM *insideParam （内部参数）
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 去初始化VENC模块，解绑定DIVP和VENC模块，销毁VENC相关的通道设置。
*/
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
/*
*  Function name : encodeGetCompressData 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 获取压缩后数据，H264 or H265;数据反馈在encode的结构体里，包含长度和有效数据。
*/
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
        encoder->option.compressData.frameLen = len;
        data = (char *)malloc(len);
        memcpy(data, stStream.pstPack[0].pu8Addr, len);
        encoder->option.compressData.buf = data;
        s32Ret = MI_VENC_ReleaseStream(vencChn, &stStream);
        if (s32Ret != MI_SUCCESS)
        {
            ST_WARN("RELEASE venc buffer fail\n");
        }
        free(data);
        return len;
    }
    return s32Ret;
}
/*
*  Function name : encodeOutputYUVData 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 对外输出YUV数据。从VPE的Outpu获取YUV数据，并且把数据提供给接口层，方便用户操作YUV数据。
*/
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
/*
*  Function name : encodeInputYUVData 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 用户输入YUV数据到编码器中，用户层把YUV数据输入到编码器。
*/
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
/*
*  Function name : encodeDestroy 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 销毁编码器。解除各个模块的绑定。
*/
static int encodeDestroy(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;
    deinitVENC(&(encode->insideParam));
    deinitDIVP(&(encode->insideParam));
    deinitVPE(&(encode->insideParam));
    ST_Sys_Exit();
}

/*
*  Function name : encodeCreate 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 创建编码器。根据用户传入的通道号设置，各模块的通道数。初始化VIF，VPE，DIVP，VENC。此接口是特殊模式下
*                  需要用户层获取YUV数据的创建方式。VIF--->VPE(YUV) ---->DIVP----->VENC
*/
static int encodeCreate(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;

    TY_ENCODE_OPTION option = encode->option;

    TY_ENCODE_INSIDE_PARAM insideParam;
    memset(&insideParam, 0, sizeof(TY_ENCODE_INSIDE_PARAM));
    insideParam.eHdrType = E_MI_VPE_HDR_TYPE_OFF;
    insideParam.vifDev = option.channel;
    insideParam.vifChn = insideParam.vifDev * 4;
    insideParam.VpeChn = insideParam.vifDev;
    insideParam.DivpChn = insideParam.vifDev;
    insideParam.VencChn = insideParam.vifDev;
    insideParam.u32InputPort = 0;

    insideParam.resolutionRate.width = option.resolutionRate.width;
    insideParam.resolutionRate.height = option.resolutionRate.height;
    insideParam.resolutionRate.rate = option.resolutionRate.rate;

    insideParam.encodeFormat = option.encodeFormat;

    if (option.YUVDataType == YUV_SEMIPLANAR_420)
    {
        insideParam.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    }
    else if (option.YUVDataType == YUV_SEMIPLANAR_422)
    {
        insideParam.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    }
    encode->insideParam = insideParam;

    initVIF(insideParam);
    initVPE(insideParam);
    initDIVP(insideParam);
    initVENC(insideParam);
}
/*
*  Function name : initVIF_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，初始化VIF，和特殊模式下保持一致。
*/
static int initVIF_General(TY_ENCODE_INSIDE_PARAM insideParam)
{
    initVIF(insideParam);
}
/*
*  Function name : initVPE_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，初始化VPE，和特殊模式下保持一致。
*/
static int initVPE_General(TY_ENCODE_INSIDE_PARAM insideParam)
{
    initVPE(insideParam);
}
/*
*  Function name : initVENC_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，初始化VENC，和特殊模式下不一样，此模式下VPE绑定VENC。
*/
static int initVENC_General(TY_ENCODE_INSIDE_PARAM insideParam)
{
    MI_VENC_CHN VencChn = insideParam.VencChn;
    MI_U32 u32VencDevId = 0xff;

    MI_VENC_ChnAttr_t stChnAttr;
    memset(&stChnAttr, 0x0, sizeof(MI_VENC_ChnAttr_t));
    MI_U32 u32VenBitRate = 1024 * 1024 * 2;

    if (insideParam.encodeFormat == AV_ENCDOE_ID_H264)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = insideParam.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    }
    else if (insideParam.encodeFormat == AV_ENCDOE_ID_H265)
    {
        stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = insideParam.resolutionRate.width;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = insideParam.resolutionRate.height;
        stChnAttr.stVeAttr.stAttrH265e.u32BFrameNum = 2;
        stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;

        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = insideParam.resolutionRate.rate;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
        stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    }

    ST_Venc_CreateChannel(insideParam.VencChn, &stChnAttr);
    ST_Venc_StartChannel(insideParam.VencChn);

    MI_VENC_GetChnDevid(insideParam.VencChn, &insideParam.u32VencDevId);
    // vpe port 2 can not attach osd, so use divp
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = insideParam.VpeChn;
    stBindInfo.stSrcChnPort.u32PortId = 0;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32DevId = insideParam.u32VencDevId;
    stBindInfo.stDstChnPort.u32ChnId = insideParam.VencChn;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    ST_Sys_Bind(&stBindInfo);
}
/*
*  Function name : encodeCreate_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，创建编码器，其中VIF-0-->VPE--->VENC
*/
static int encodeCreate_General(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;

    TY_ENCODE_OPTION option = encode->option;

    TY_ENCODE_INSIDE_PARAM insideParam;
    memset(&insideParam, 0, sizeof(TY_ENCODE_INSIDE_PARAM));
    insideParam.eHdrType = E_MI_VPE_HDR_TYPE_OFF;
    insideParam.vifDev = option.channel;
    insideParam.vifChn = insideParam.vifDev * 4;
    insideParam.VpeChn = insideParam.vifDev;
    insideParam.DivpChn = insideParam.vifDev;
    insideParam.VencChn = insideParam.vifDev;
    insideParam.u32InputPort = 0;

    insideParam.resolutionRate.width = option.resolutionRate.width;
    insideParam.resolutionRate.height = option.resolutionRate.height;
    insideParam.resolutionRate.rate = option.resolutionRate.rate;

    insideParam.encodeFormat = option.encodeFormat;

    if (option.YUVDataType == YUV_SEMIPLANAR_420)
    {
        insideParam.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    }
    else if (option.YUVDataType == YUV_SEMIPLANAR_422)
    {
        insideParam.inputYUVType = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422;
    }
    encode->insideParam = insideParam;

    initVIF_General(insideParam);
    initVPE_General(insideParam);
    initVENC_General(insideParam);
}
/*
*  Function name : encodeGetCompressData 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 获取压缩后数据，H264 or H265;数据反馈在encode的结构体里，包含长度和有效数据。和普通模式保持一致
*/
static int encodeGetCompressData_General(TY_NOVA_ENCODER *encode)
{
    encodeGetCompressData(encode);
}
/*
*  Function name : deinitVENC_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，去初始化VENC模块，解除VPE和VENC模块。
*/
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
/*
*  Function name : deinitVPE_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，去初始化VPE模块，解除VPE和VIF模块。和特殊模式保持一致。
*/
static int deinitVPE_General(TY_ENCODE_INSIDE_PARAM *insideParam)
{
    deinitVPE(insideParam);
}
/*
*  Function name : encodeDestroy_General 接口函数
*  In            : YTY_NOVA_ENCODER *encode 唯一结构体
*  Out           : null
*  Return        : 0 : success; other is error 
*  Description   : 普通模式下，去初始化编码器。
*/
static int encodeDestroy_General(void *encoder)
{
    TY_NOVA_ENCODER *encode;
    encode = (TY_NOVA_ENCODER *)encoder;
    deinitVENC_General(&(encode->insideParam));
    deinitVPE_General(&(encode->insideParam));
    ST_Sys_Exit();
}
/*
*  Function name : nova_encoder_alloc 接口函数
*  In            : 申请队列个数。
*  Out           : null
*  Return        : TY_NOVA_ENCODER_QUEUE decoderQueue;
*  Description   : 1. 申请一个队列，里面包含TY_NOVA_ENCODER 模块。
*/
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
/*
*  Function name : nova_encoder_free 接口函数
*  In            : TY_NOVA_ENCODER_QUEUE *decoderQueue.需要是否的队列。
*  Out           : null
*  Return        : null;
*  Description   : 3.释放申请的队列。
*/
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
/*
*  Function name : nova_encoder 接口函数
*  In            : TY_NOVA_ENCODER *NOVA_encoder。用户层传入的编码器参数。
*  Out           : NULL
*  Return        : TY_NOVA_ENCODER_QUEUE * 返回创建的队列。 
*  Description   : 1. 根据用户输入的encoer,创建队列。
*                  2. 根据输入的encoder关联相关的函数。如果至少普通模式，用户需要调用“encodeCreate_General”，“encodeGetCompressData_General”，“encodeDestroy_General”
*                     如果是特殊模式，需要获取YUV，需要调用“encodeCreate”，“encodeOutputYUVData”，“encodeInputYUVData”，“encodeGetCompressData”，“encodeDestroy”。
*/
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
    encoder.option.YUVDataType = YUV_SEMIPLANAR_420;
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
        // printf("data len : %d \n",encoder.option.compressData.frameLen);
        // printf("data [0] : %d \n",encoder.option.compressData.buf[0]);
    }

    encoder_queue->encoders->close(&encoder);

    nova_encoder_free(encoder_queue);
}
*/