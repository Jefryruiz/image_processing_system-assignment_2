set SynModuleInfo {
  {SRCNAME entry_proc MODELNAME entry_proc RTLNAME gray_kernel_entry_proc}
  {SRCNAME read_stage_Pipeline_VITIS_LOOP_15_1 MODELNAME read_stage_Pipeline_VITIS_LOOP_15_1 RTLNAME gray_kernel_read_stage_Pipeline_VITIS_LOOP_15_1
    SUBMODULES {
      {MODELNAME gray_kernel_flow_control_loop_pipe_sequential_init RTLNAME gray_kernel_flow_control_loop_pipe_sequential_init BINDTYPE interface TYPE internal_upc_flow_control INSTNAME gray_kernel_flow_control_loop_pipe_sequential_init_U}
    }
  }
  {SRCNAME read_stage MODELNAME read_stage RTLNAME gray_kernel_read_stage}
  {SRCNAME proc_stage_Pipeline_VITIS_LOOP_24_1 MODELNAME proc_stage_Pipeline_VITIS_LOOP_24_1 RTLNAME gray_kernel_proc_stage_Pipeline_VITIS_LOOP_24_1
    SUBMODULES {
      {MODELNAME gray_kernel_mul_8ns_10ns_17_1_1 RTLNAME gray_kernel_mul_8ns_10ns_17_1_1 BINDTYPE op TYPE mul IMPL auto LATENCY 0 ALLOW_PRAGMA 1}
      {MODELNAME gray_kernel_mac_muladd_8ns_7ns_17ns_17_4_1 RTLNAME gray_kernel_mac_muladd_8ns_7ns_17ns_17_4_1 BINDTYPE op TYPE all IMPL dsp_slice LATENCY 3}
      {MODELNAME gray_kernel_mac_muladd_10ns_8ns_17ns_18_4_1 RTLNAME gray_kernel_mac_muladd_10ns_8ns_17ns_18_4_1 BINDTYPE op TYPE all IMPL dsp_slice LATENCY 3}
    }
  }
  {SRCNAME proc_stage MODELNAME proc_stage RTLNAME gray_kernel_proc_stage}
  {SRCNAME write_stage_Pipeline_VITIS_LOOP_38_1 MODELNAME write_stage_Pipeline_VITIS_LOOP_38_1 RTLNAME gray_kernel_write_stage_Pipeline_VITIS_LOOP_38_1}
  {SRCNAME write_stage MODELNAME write_stage RTLNAME gray_kernel_write_stage}
  {SRCNAME gray_kernel MODELNAME gray_kernel RTLNAME gray_kernel IS_TOP 1
    SUBMODULES {
      {MODELNAME gray_kernel_fifo_w64_d4_S RTLNAME gray_kernel_fifo_w64_d4_S BINDTYPE storage TYPE fifo IMPL srl ALLOW_PRAGMA 1 INSTNAME out_r_c_U}
      {MODELNAME gray_kernel_fifo_w24_d256_A RTLNAME gray_kernel_fifo_w24_d256_A BINDTYPE storage TYPE fifo IMPL memory ALLOW_PRAGMA 1 INSTNAME stream_in_U}
      {MODELNAME gray_kernel_fifo_w32_d2_S RTLNAME gray_kernel_fifo_w32_d2_S BINDTYPE storage TYPE fifo IMPL srl ALLOW_PRAGMA 1 INSTNAME num_pixels_c1_U}
      {MODELNAME gray_kernel_fifo_w8_d256_A RTLNAME gray_kernel_fifo_w8_d256_A BINDTYPE storage TYPE fifo IMPL memory ALLOW_PRAGMA 1 INSTNAME stream_out_U}
      {MODELNAME gray_kernel_fifo_w32_d2_S RTLNAME gray_kernel_fifo_w32_d2_S BINDTYPE storage TYPE fifo IMPL srl ALLOW_PRAGMA 1 INSTNAME num_pixels_c_U}
      {MODELNAME gray_kernel_start_for_write_stage_U0 RTLNAME gray_kernel_start_for_write_stage_U0 BINDTYPE storage TYPE fifo IMPL srl ALLOW_PRAGMA 1 INSTNAME start_for_write_stage_U0_U}
      {MODELNAME gray_kernel_start_for_proc_stage_U0 RTLNAME gray_kernel_start_for_proc_stage_U0 BINDTYPE storage TYPE fifo IMPL srl ALLOW_PRAGMA 1 INSTNAME start_for_proc_stage_U0_U}
      {MODELNAME gray_kernel_gmem_m_axi RTLNAME gray_kernel_gmem_m_axi BINDTYPE interface TYPE adapter IMPL m_axi}
      {MODELNAME gray_kernel_control_s_axi RTLNAME gray_kernel_control_s_axi BINDTYPE interface TYPE interface_s_axilite}
    }
  }
}
