// Checks every write/read burst observed by the monitor against the
// dpi/ram_model.c golden model (imported via DPI-C, see
// axi_ram_dpi_import.svh). Writes are mirrored into the golden model;
// reads are compared against it.
class axi_ram_scoreboard extends uvm_subscriber #(axi_ram_seq_item);
    `uvm_component_utils(axi_ram_scoreboard)

    int unsigned num_checked;
    int unsigned num_errors;

    // Per-byte-check transaction stream, viewable in xsim -gui (Scopes ->
    // uvm_test_top.env.scoreboard -> checks) alongside the signal
    // waveform. Not visible in a plain VCD/gtkwave: these are UVM
    // transaction records, not HDL signals - class fields have no
    // hierarchical signal path for a VCD dumper to capture.
    uvm_tr_stream m_stream;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        m_stream = get_tr_stream("checks", "Transactions");
    endfunction

    function void write(axi_ram_seq_item t);
        if (t.op == axi_ram_seq_item::OP_WRITE) begin
            for (int i = 0; i < t.len_bytes; i++)
                ram_model_write_byte(t.addr + i, t.wdata[i]);
        end else begin
            for (int i = 0; i < t.len_bytes; i++) begin
                byte unsigned expected = ram_model_read_byte(t.addr + i);
                bit           match    = (expected === t.rdata[i]);
                uvm_recorder  recorder;

                num_checked++;
                if (!match) begin
                    num_errors++;
                    `uvm_error("SCBD_MISMATCH",
                        $sformatf("addr=0x%0h byte=%0d expected=0x%0h got=0x%0h",
                                  t.addr, i, expected, t.rdata[i]))
                end

                recorder = m_stream.open_recorder(match ? "byte_check_ok" : "byte_check_FAIL", $time, "check");
                if (recorder != null) begin
                    recorder.record_field("addr",     t.addr + i,   64, UVM_HEX);
                    recorder.record_field("byte_idx", i,            32, UVM_DEC);
                    recorder.record_field("expected", expected,      8, UVM_HEX);
                    recorder.record_field("got",      t.rdata[i],    8, UVM_HEX);
                    recorder.record_field("match",    match,         1, UVM_BIN);
                    recorder.close($time);
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
