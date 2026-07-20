set ModuleHierarchy {[{
"Name" : "gray_kernel","ID" : "0","Type" : "dataflow",
"SubInsts" : [
	{"Name" : "entry_proc_U0","ID" : "1","Type" : "sequential"},
	{"Name" : "read_stage_U0","ID" : "2","Type" : "sequential",
		"SubInsts" : [
		{"Name" : "grp_read_stage_Pipeline_VITIS_LOOP_15_1_fu_82","ID" : "3","Type" : "sequential",
			"SubLoops" : [
			{"Name" : "VITIS_LOOP_15_1","ID" : "4","Type" : "pipeline"},]},]},
	{"Name" : "proc_stage_U0","ID" : "5","Type" : "sequential",
		"SubInsts" : [
		{"Name" : "grp_proc_stage_Pipeline_VITIS_LOOP_24_1_fu_50","ID" : "6","Type" : "sequential",
			"SubLoops" : [
			{"Name" : "VITIS_LOOP_24_1","ID" : "7","Type" : "pipeline"},]},]},
	{"Name" : "write_stage_U0","ID" : "8","Type" : "sequential",
		"SubInsts" : [
		{"Name" : "grp_write_stage_Pipeline_VITIS_LOOP_38_1_fu_69","ID" : "9","Type" : "sequential",
			"SubLoops" : [
			{"Name" : "VITIS_LOOP_38_1","ID" : "10","Type" : "pipeline"},]},]},]
}]}