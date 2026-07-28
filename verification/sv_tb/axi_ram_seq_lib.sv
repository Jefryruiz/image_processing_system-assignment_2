// Writes num_bursts random bursts at random word-aligned addresses, then
// reads each one back so the scoreboard can check DUT vs. golden model.
class axi_ram_random_seq extends uvm_sequence #(axi_ram_seq_item);
    `uvm_object_utils(axi_ram_random_seq)

    int unsigned num_bursts    = 50;
    bit [25:0]   addr_span     = 26'h0FF_FFFF; // stay within the 64 MB space

    function new(string name = "axi_ram_random_seq");
        super.new(name);
    endfunction

    task body();
        bit [25:0]   addrs[$];
        int unsigned lens[$];

        for (int i = 0; i < num_bursts; i++) begin
            axi_ram_seq_item wr;
            bit [25:0] addr = ($urandom() % (addr_span >> 10)) << 10; // 1KB aligned

            wr = axi_ram_seq_item::type_id::create("wr");
            start_item(wr);
            if (!wr.randomize() with { op == axi_ram_seq_item::OP_WRITE; addr == local::addr; })
                `uvm_fatal("RANDSEQ", "randomize failed on write item")
            finish_item(wr);

            // Read back exactly the range just written - bytes outside it
            // may be X in the DUT's memory array until first written, which
            // would cause spurious scoreboard mismatches.
            addrs.push_back(addr);
            lens.push_back(wr.len_bytes);
        end

        foreach (addrs[i]) begin
            axi_ram_seq_item rd;
            rd = axi_ram_seq_item::type_id::create("rd");
            start_item(rd);
            rd.op        = axi_ram_seq_item::OP_READ;
            rd.addr      = addrs[i];
            rd.len_bytes = lens[i];
            finish_item(rd);
        end
    endtask
endclass


// Loads a real RAW image file into the golden model via DPI-C
// (dpi/ram_model.c, the same one linked into the SystemC executable), then
// streams it into the axi_ram DUT as AXI write bursts, reads it back, and
// finally dumps the golden model's mirror of the DUT contents back out to
// disk - exercising the exact same image files the SystemC flow uses.
class axi_ram_image_seq extends uvm_sequence #(axi_ram_seq_item);
    `uvm_object_utils(axi_ram_image_seq)

    string       input_path  = "input.raw";
    string       output_path = "sv_tb_output.raw";
    bit [25:0]   base_addr   = 26'h0;
    int unsigned num_bytes   = 262144; // subset by default; full image also supported

    function new(string name = "axi_ram_image_seq");
        super.new(name);
    endfunction

    task body();
        int unsigned burst_size = 1024;
        int unsigned remaining;
        bit [25:0] addr;

        ram_model_init(ram_model_size() == 0 ? 64 * 1024 * 1024 : ram_model_size());
        if (ram_model_load_file(0, input_path, num_bytes) != 0)
            `uvm_fatal("IMGSEQ", $sformatf("could not load %s", input_path))

        // Write phase: stream the golden model's copy of the image to the DUT.
        remaining = num_bytes;
        addr = base_addr;
        while (remaining > 0) begin
            axi_ram_seq_item wr;
            int unsigned this_len = (remaining < burst_size) ? remaining : burst_size;
            wr = axi_ram_seq_item::type_id::create("wr");
            wr.wdata = new[this_len];
            for (int i = 0; i < this_len; i++)
                wr.wdata[i] = ram_model_read_byte(addr + i);
            start_item(wr);
            wr.op        = axi_ram_seq_item::OP_WRITE;
            wr.addr      = addr;
            wr.len_bytes = this_len;
            finish_item(wr);
            addr      += this_len;
            remaining -= this_len;
        end

        // Read-back phase: the scoreboard compares every burst against the
        // golden model that was just populated above.
        remaining = num_bytes;
        addr = base_addr;
        while (remaining > 0) begin
            axi_ram_seq_item rd;
            int unsigned this_len = (remaining < burst_size) ? remaining : burst_size;
            rd = axi_ram_seq_item::type_id::create("rd");
            start_item(rd);
            rd.op        = axi_ram_seq_item::OP_READ;
            rd.addr      = addr;
            rd.len_bytes = this_len;
            finish_item(rd);
            addr      += this_len;
            remaining -= this_len;
        end

        if (ram_model_dump_file(base_addr, output_path, num_bytes) != 0)
            `uvm_fatal("IMGSEQ", $sformatf("could not dump %s", output_path));
        `uvm_info("IMGSEQ", $sformatf("wrote %0d bytes to %s", num_bytes, output_path), UVM_LOW)
    endtask
endclass
