// DPI-C imports shared by the UVM testbench.
//
// ram_model_* comes from dpi/ram_model.c - the same golden RAM model that
// is linked directly into the SystemC executable (see
// systemc-image-processing-platform/src/ram_mem.h). Importing it here lets
// the scoreboard/sequences use it as an independent reference and lets test
// sequences load/dump the exact RAW image files the SystemC flow uses.
//
// axi_ram_backdoor_* is exported by the DUT itself (rtl/axi_ram.sv) so the
// scoreboard can peek/poke the DUT's memory array directly without going
// through the AXI protocol.

import "DPI-C" function void ram_model_init(input longint unsigned size);
import "DPI-C" function longint unsigned ram_model_size();
import "DPI-C" function void ram_model_write_byte(input longint unsigned addr, input byte unsigned data);
import "DPI-C" function byte unsigned ram_model_read_byte(input longint unsigned addr);
import "DPI-C" function int ram_model_load_file(input longint unsigned addr, input string path, input int unsigned len);
import "DPI-C" function int ram_model_dump_file(input longint unsigned addr, input string path, input int unsigned len);

import "DPI-C" function byte unsigned axi_ram_backdoor_read(input longint unsigned addr);
import "DPI-C" function void axi_ram_backdoor_write(input longint unsigned addr, input byte unsigned data);
