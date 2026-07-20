#!/bin/sh

# 
# Vivado(TM)
# runme.sh: a Vivado-generated Runs Script for UNIX
# Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
# 

if [ -z "$PATH" ]; then
  PATH=/home/jezuniga/tools/Xilinx/Vitis/2024.1/bin:/home/jezuniga/tools/Xilinx/Vivado/2024.1/ids_lite/ISE/bin/lin64:/home/jezuniga/tools/Xilinx/Vivado/2024.1/bin
else
  PATH=/home/jezuniga/tools/Xilinx/Vitis/2024.1/bin:/home/jezuniga/tools/Xilinx/Vivado/2024.1/ids_lite/ISE/bin/lin64:/home/jezuniga/tools/Xilinx/Vivado/2024.1/bin:$PATH
fi
export PATH

if [ -z "$LD_LIBRARY_PATH" ]; then
  LD_LIBRARY_PATH=
else
  LD_LIBRARY_PATH=:$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

HD_PWD='/home/jezuniga/mnt/HDD/Maestria/2026Q2/Diseno_de_alto_nivel_de_sistemas_electronicos/image_processing_system/HSL_Vitis/gray_kernel_prj/solution1/impl/verilog/project.runs/bd_0_hls_inst_0_synth_1'
cd "$HD_PWD"

HD_LOG=runme.log
/bin/touch $HD_LOG

ISEStep="./ISEWrap.sh"
EAStep()
{
     $ISEStep $HD_LOG "$@" >> $HD_LOG 2>&1
     if [ $? -ne 0 ]
     then
         exit
     fi
}

EAStep vivado -log bd_0_hls_inst_0.vds -m64 -product Vivado -mode batch -messageDb vivado.pb -notrace -source bd_0_hls_inst_0.tcl
