// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.1 (64-bit)
// Tool Version Limit: 2024.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xgray_kernel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XGray_kernel_CfgInitialize(XGray_kernel *InstancePtr, XGray_kernel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XGray_kernel_Start(XGray_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL) & 0x80;
    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XGray_kernel_IsDone(XGray_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XGray_kernel_IsIdle(XGray_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XGray_kernel_IsReady(XGray_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XGray_kernel_EnableAutoRestart(XGray_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XGray_kernel_DisableAutoRestart(XGray_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_AP_CTRL, 0);
}

void XGray_kernel_Set_in_r(XGray_kernel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IN_R_DATA, (u32)(Data));
    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IN_R_DATA + 4, (u32)(Data >> 32));
}

u64 XGray_kernel_Get_in_r(XGray_kernel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IN_R_DATA);
    Data += (u64)XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IN_R_DATA + 4) << 32;
    return Data;
}

void XGray_kernel_Set_out_r(XGray_kernel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_OUT_R_DATA, (u32)(Data));
    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_OUT_R_DATA + 4, (u32)(Data >> 32));
}

u64 XGray_kernel_Get_out_r(XGray_kernel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_OUT_R_DATA);
    Data += (u64)XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_OUT_R_DATA + 4) << 32;
    return Data;
}

void XGray_kernel_Set_num_pixels(XGray_kernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_NUM_PIXELS_DATA, Data);
}

u32 XGray_kernel_Get_num_pixels(XGray_kernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_NUM_PIXELS_DATA);
    return Data;
}

void XGray_kernel_InterruptGlobalEnable(XGray_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_GIE, 1);
}

void XGray_kernel_InterruptGlobalDisable(XGray_kernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_GIE, 0);
}

void XGray_kernel_InterruptEnable(XGray_kernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IER);
    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IER, Register | Mask);
}

void XGray_kernel_InterruptDisable(XGray_kernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IER);
    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IER, Register & (~Mask));
}

void XGray_kernel_InterruptClear(XGray_kernel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGray_kernel_WriteReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_ISR, Mask);
}

u32 XGray_kernel_InterruptGetEnabled(XGray_kernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_IER);
}

u32 XGray_kernel_InterruptGetStatus(XGray_kernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGray_kernel_ReadReg(InstancePtr->Control_BaseAddress, XGRAY_KERNEL_CONTROL_ADDR_ISR);
}

