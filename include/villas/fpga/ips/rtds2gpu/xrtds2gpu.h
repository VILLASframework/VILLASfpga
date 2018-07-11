// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2017.3
// Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
// 
// ==============================================================

#ifndef XRTDS2GPU_H
#define XRTDS2GPU_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xrtds2gpu_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_BaseAddress;
} XRtds2gpu_Config;
#endif

typedef struct {
    u32 Ctrl_BaseAddress;
    u32 IsReady;
} XRtds2gpu;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XRtds2gpu_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XRtds2gpu_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XRtds2gpu_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XRtds2gpu_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XRtds2gpu_Initialize(XRtds2gpu *InstancePtr, u16 DeviceId);
XRtds2gpu_Config* XRtds2gpu_LookupConfig(u16 DeviceId);
int XRtds2gpu_CfgInitialize(XRtds2gpu *InstancePtr, XRtds2gpu_Config *ConfigPtr);
#else
int XRtds2gpu_Initialize(XRtds2gpu *InstancePtr, const char* InstanceName);
int XRtds2gpu_Release(XRtds2gpu *InstancePtr);
#endif

void XRtds2gpu_Start(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_IsDone(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_IsIdle(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_IsReady(XRtds2gpu *InstancePtr);
void XRtds2gpu_EnableAutoRestart(XRtds2gpu *InstancePtr);
void XRtds2gpu_DisableAutoRestart(XRtds2gpu *InstancePtr);

void XRtds2gpu_Set_baseaddr(XRtds2gpu *InstancePtr, u32 Data);
u32 XRtds2gpu_Get_baseaddr(XRtds2gpu *InstancePtr);
void XRtds2gpu_Set_data_offset(XRtds2gpu *InstancePtr, u32 Data);
u32 XRtds2gpu_Get_data_offset(XRtds2gpu *InstancePtr);
void XRtds2gpu_Set_doorbell_offset(XRtds2gpu *InstancePtr, u32 Data);
u32 XRtds2gpu_Get_doorbell_offset(XRtds2gpu *InstancePtr);
void XRtds2gpu_Set_frame_size(XRtds2gpu *InstancePtr, u32 Data);
u32 XRtds2gpu_Get_frame_size(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_Get_status(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_Get_status_vld(XRtds2gpu *InstancePtr);

void XRtds2gpu_InterruptGlobalEnable(XRtds2gpu *InstancePtr);
void XRtds2gpu_InterruptGlobalDisable(XRtds2gpu *InstancePtr);
void XRtds2gpu_InterruptEnable(XRtds2gpu *InstancePtr, u32 Mask);
void XRtds2gpu_InterruptDisable(XRtds2gpu *InstancePtr, u32 Mask);
void XRtds2gpu_InterruptClear(XRtds2gpu *InstancePtr, u32 Mask);
u32 XRtds2gpu_InterruptGetEnabled(XRtds2gpu *InstancePtr);
u32 XRtds2gpu_InterruptGetStatus(XRtds2gpu *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif