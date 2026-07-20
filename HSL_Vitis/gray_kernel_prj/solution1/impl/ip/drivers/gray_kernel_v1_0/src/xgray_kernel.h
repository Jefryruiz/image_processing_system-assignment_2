// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.1 (64-bit)
// Tool Version Limit: 2024.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XGRAY_KERNEL_H
#define XGRAY_KERNEL_H

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
#include "xgray_kernel_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XGray_kernel_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XGray_kernel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XGray_kernel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XGray_kernel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XGray_kernel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XGray_kernel_ReadReg(BaseAddress, RegOffset) \
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
#ifdef SDT
int XGray_kernel_Initialize(XGray_kernel *InstancePtr, UINTPTR BaseAddress);
XGray_kernel_Config* XGray_kernel_LookupConfig(UINTPTR BaseAddress);
#else
int XGray_kernel_Initialize(XGray_kernel *InstancePtr, u16 DeviceId);
XGray_kernel_Config* XGray_kernel_LookupConfig(u16 DeviceId);
#endif
int XGray_kernel_CfgInitialize(XGray_kernel *InstancePtr, XGray_kernel_Config *ConfigPtr);
#else
int XGray_kernel_Initialize(XGray_kernel *InstancePtr, const char* InstanceName);
int XGray_kernel_Release(XGray_kernel *InstancePtr);
#endif

void XGray_kernel_Start(XGray_kernel *InstancePtr);
u32 XGray_kernel_IsDone(XGray_kernel *InstancePtr);
u32 XGray_kernel_IsIdle(XGray_kernel *InstancePtr);
u32 XGray_kernel_IsReady(XGray_kernel *InstancePtr);
void XGray_kernel_EnableAutoRestart(XGray_kernel *InstancePtr);
void XGray_kernel_DisableAutoRestart(XGray_kernel *InstancePtr);

void XGray_kernel_Set_in_r(XGray_kernel *InstancePtr, u64 Data);
u64 XGray_kernel_Get_in_r(XGray_kernel *InstancePtr);
void XGray_kernel_Set_out_r(XGray_kernel *InstancePtr, u64 Data);
u64 XGray_kernel_Get_out_r(XGray_kernel *InstancePtr);
void XGray_kernel_Set_num_pixels(XGray_kernel *InstancePtr, u32 Data);
u32 XGray_kernel_Get_num_pixels(XGray_kernel *InstancePtr);

void XGray_kernel_InterruptGlobalEnable(XGray_kernel *InstancePtr);
void XGray_kernel_InterruptGlobalDisable(XGray_kernel *InstancePtr);
void XGray_kernel_InterruptEnable(XGray_kernel *InstancePtr, u32 Mask);
void XGray_kernel_InterruptDisable(XGray_kernel *InstancePtr, u32 Mask);
void XGray_kernel_InterruptClear(XGray_kernel *InstancePtr, u32 Mask);
u32 XGray_kernel_InterruptGetEnabled(XGray_kernel *InstancePtr);
u32 XGray_kernel_InterruptGetStatus(XGray_kernel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
