package axi_ram_pkg;
    import uvm_pkg::*;
    `include "uvm_macros.svh"

    `include "axi_ram_dpi_import.svh"
    `include "axi_ram_seq_item.sv"
    `include "axi_ram_sequencer.sv"
    `include "axi_ram_driver.sv"
    `include "axi_ram_monitor.sv"
    `include "axi_ram_agent.sv"
    `include "axi_ram_scoreboard.sv"
    `include "axi_ram_env.sv"
    `include "axi_ram_seq_lib.sv"
    `include "axi_ram_test_lib.sv"
endpackage
