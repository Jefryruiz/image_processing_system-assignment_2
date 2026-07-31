class axi_ram_driver extends uvm_driver #(axi_ram_seq_item);
    `uvm_component_utils(axi_ram_driver)

    virtual axi_if vif;

    localparam int BEAT_BYTES = 4; // DATA_WIDTH/8

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual axi_if)::get(this, "", "vif", vif))
            `uvm_fatal("NOVIF", "axi_ram_driver: virtual interface not set")
    endfunction

    task run_phase(uvm_phase phase);
        vif.drv_cb.awvalid <= 0;
        vif.drv_cb.wvalid  <= 0;
        vif.drv_cb.bready  <= 1;
        vif.drv_cb.arvalid <= 0;
        vif.drv_cb.rready  <= 1;

        // Do not start driving transactions until reset has been released,
        // otherwise the first AW/AR handshake can race the DUT's reset and
        // be silently dropped (awready/arready are asserted throughout
        // reset since the DUT's FSMs sit in their idle state).
        wait (vif.aresetn === 1'b1);
        @(vif.drv_cb);

        forever begin
            axi_ram_seq_item req;
            seq_item_port.get_next_item(req);
            if (req.op == axi_ram_seq_item::OP_WRITE)
                do_write(req);
            else
                do_read(req);
            seq_item_port.item_done();
        end
    endtask

    task do_write(axi_ram_seq_item req);
        int unsigned nbeats;
        nbeats = (req.len_bytes + BEAT_BYTES - 1) / BEAT_BYTES;

        @(vif.drv_cb);
        vif.drv_cb.awid    <= '0;
        vif.drv_cb.awaddr  <= req.addr;
        vif.drv_cb.awlen   <= nbeats - 1;
        vif.drv_cb.awsize  <= 3'b010; // 4 bytes/beat
        vif.drv_cb.awburst <= 2'b01;  // INCR
        vif.drv_cb.awvalid <= 1;
        do @(vif.drv_cb); while (!vif.drv_cb.awready);
        vif.drv_cb.awvalid <= 0;

        for (int beat = 0; beat < nbeats; beat++) begin
            bit [31:0] word = '0;
            bit [3:0]  strb = '0;
            for (int b = 0; b < BEAT_BYTES; b++) begin
                int idx = beat * BEAT_BYTES + b;
                if (idx < req.len_bytes) begin
                    word[8*b +: 8] = req.wdata[idx];
                    strb[b] = 1'b1;
                end
            end
            vif.drv_cb.wdata  <= word;
            vif.drv_cb.wstrb  <= strb;
            vif.drv_cb.wlast  <= (beat == nbeats - 1);
            vif.drv_cb.wvalid <= 1;
            do @(vif.drv_cb); while (!vif.drv_cb.wready);
        end
        vif.drv_cb.wvalid <= 0;
        vif.drv_cb.wlast  <= 0;

        do @(vif.drv_cb); while (!vif.drv_cb.bvalid);
        req.resp_error = (vif.drv_cb.bresp != 2'b00);
    endtask

    task do_read(axi_ram_seq_item req);
        int unsigned nbeats;
        nbeats = (req.len_bytes + BEAT_BYTES - 1) / BEAT_BYTES;
        req.rdata = new[req.len_bytes];

        @(vif.drv_cb);
        vif.drv_cb.arid    <= '0;
        vif.drv_cb.araddr  <= req.addr;
        vif.drv_cb.arlen   <= nbeats - 1;
        vif.drv_cb.arsize  <= 3'b010;
        vif.drv_cb.arburst <= 2'b01;
        vif.drv_cb.arvalid <= 1;
        do @(vif.drv_cb); while (!vif.drv_cb.arready);
        vif.drv_cb.arvalid <= 0;

        req.resp_error = 0;
        for (int beat = 0; beat < nbeats; beat++) begin
            do @(vif.drv_cb); while (!vif.drv_cb.rvalid);
            for (int b = 0; b < BEAT_BYTES; b++) begin
                int idx = beat * BEAT_BYTES + b;
                if (idx < req.len_bytes)
                    req.rdata[idx] = vif.drv_cb.rdata[8*b +: 8];
            end
            if (vif.drv_cb.rresp != 2'b00) req.resp_error = 1;
        end
    endtask

endclass
