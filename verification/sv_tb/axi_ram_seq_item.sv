class axi_ram_seq_item extends uvm_sequence_item;

    typedef enum { OP_WRITE, OP_READ } op_e;

    rand op_e              op;
    rand bit [25:0]        addr;      // ADDR_WIDTH = 26 (64 MB)
    rand byte unsigned     wdata[];   // one entry per byte in the burst
    rand int unsigned      len_bytes; // burst length in bytes

    // Populated by the driver/monitor after a read completes.
    byte unsigned           rdata[];
    bit                      resp_error;

    // AXI4 limits bursts to 256 beats; at 4 bytes/beat that caps a single
    // transaction at 1024 bytes.
    constraint c_len_bytes {
        len_bytes inside {[1:1024]};
    }

    constraint c_wdata_size {
        wdata.size() == len_bytes;
    }

    `uvm_object_utils_begin(axi_ram_seq_item)
        `uvm_field_enum(op_e, op, UVM_ALL_ON)
        `uvm_field_int(addr, UVM_ALL_ON)
        `uvm_field_array_int(wdata, UVM_ALL_ON)
        `uvm_field_int(len_bytes, UVM_ALL_ON)
        `uvm_field_array_int(rdata, UVM_ALL_ON)
        `uvm_field_int(resp_error, UVM_ALL_ON)
    `uvm_object_utils_end

    function new(string name = "axi_ram_seq_item");
        super.new(name);
    endfunction

endclass
