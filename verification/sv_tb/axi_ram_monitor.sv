class axi_ram_monitor extends uvm_monitor;
    `uvm_component_utils(axi_ram_monitor)

    virtual axi_if vif;
    uvm_analysis_port #(axi_ram_seq_item) ap;

    localparam int BEAT_BYTES = 4;

    function new(string name, uvm_component parent);
        super.new(name, parent);
        ap = new("ap", this);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual axi_if)::get(this, "", "vif", vif))
            `uvm_fatal("NOVIF", "axi_ram_monitor: virtual interface not set")
    endfunction

    task run_phase(uvm_phase phase);
        wait (vif.aresetn === 1'b1);
        fork
            watch_writes();
            watch_reads();
        join
    endtask

    task watch_writes();
        forever begin
            axi_ram_seq_item item;
            bit [25:0] addr;
            byte unsigned bytes_q[$];

            @(vif.mon_cb iff (vif.mon_cb.awvalid && vif.mon_cb.awready));
            addr = vif.mon_cb.awaddr;

            forever begin
                @(vif.mon_cb iff (vif.mon_cb.wvalid && vif.mon_cb.wready));
                for (int b = 0; b < BEAT_BYTES; b++)
                    bytes_q.push_back(vif.mon_cb.wdata[8*b +: 8]);
                if (vif.mon_cb.wlast) break;
            end

            item = axi_ram_seq_item::type_id::create("mon_write_item");
            item.op        = axi_ram_seq_item::OP_WRITE;
            item.addr      = addr;
            item.len_bytes = bytes_q.size();
            item.wdata     = bytes_q;
            ap.write(item);
        end
    endtask

    task watch_reads();
        forever begin
            axi_ram_seq_item item;
            bit [25:0] addr;
            byte unsigned bytes_q[$];

            @(vif.mon_cb iff (vif.mon_cb.arvalid && vif.mon_cb.arready));
            addr = vif.mon_cb.araddr;

            forever begin
                @(vif.mon_cb iff (vif.mon_cb.rvalid && vif.mon_cb.rready));
                for (int b = 0; b < BEAT_BYTES; b++)
                    bytes_q.push_back(vif.mon_cb.rdata[8*b +: 8]);
                if (vif.mon_cb.rlast) break;
            end

            item = axi_ram_seq_item::type_id::create("mon_read_item");
            item.op        = axi_ram_seq_item::OP_READ;
            item.addr      = addr;
            item.len_bytes = bytes_q.size();
            item.rdata     = bytes_q;
            ap.write(item);
        end
    endtask

endclass
