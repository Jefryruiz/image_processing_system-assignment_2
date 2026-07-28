// Checks every write/read burst observed by the monitor against the
// dpi/ram_model.c golden model (imported via DPI-C, see
// axi_ram_dpi_import.svh). Writes are mirrored into the golden model;
// reads are compared against it.
class axi_ram_scoreboard extends uvm_subscriber #(axi_ram_seq_item);
    `uvm_component_utils(axi_ram_scoreboard)

    int unsigned num_checked;
    int unsigned num_errors;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void write(axi_ram_seq_item t);
        if (t.op == axi_ram_seq_item::OP_WRITE) begin
            for (int i = 0; i < t.len_bytes; i++)
                ram_model_write_byte(t.addr + i, t.wdata[i]);
        end else begin
            for (int i = 0; i < t.len_bytes; i++) begin
                byte unsigned expected = ram_model_read_byte(t.addr + i);
                num_checked++;
                if (expected !== t.rdata[i]) begin
                    num_errors++;
                    `uvm_error("SCBD_MISMATCH",
                        $sformatf("addr=0x%0h byte=%0d expected=0x%0h got=0x%0h",
                                  t.addr, i, expected, t.rdata[i]))
                end
            end
        end
    endfunction

    function void report_phase(uvm_phase phase);
        `uvm_info("SCBD_SUMMARY",
            $sformatf("bytes checked=%0d errors=%0d", num_checked, num_errors),
            UVM_LOW)
        if (num_errors == 0 && num_checked > 0)
            `uvm_info("SCBD_SUMMARY", "*** SCOREBOARD PASSED ***", UVM_LOW)
    endfunction

endclass
