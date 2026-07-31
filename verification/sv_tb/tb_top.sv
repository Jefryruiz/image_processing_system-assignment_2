module tb_top;
    import uvm_pkg::*;
    import axi_ram_pkg::*;

    localparam int ADDR_WIDTH = 26;
    localparam int DATA_WIDTH = 32;
    localparam int ID_WIDTH   = 4;

    logic aclk;
    logic aresetn;

    initial aclk = 0;
    always #5 aclk = ~aclk; // 100 MHz

    initial begin
        aresetn = 0;
        repeat (5) @(posedge aclk);
        aresetn = 1;
    end

    // Instantiated with the interface's own defaults (which match the
    // localparams above) so that plain `virtual axi_if.DRIVER`/`.MONITOR`
    // handles (used here and in axi_ram_driver.sv/axi_ram_monitor.sv)
    // resolve to the same parameter specialization xsim elaborates.
    axi_if vif (.aclk(aclk), .aresetn(aresetn));

    axi_ram #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH), .ID_WIDTH(ID_WIDTH)) dut (
        .aclk         (aclk),
        .aresetn      (aresetn),

        .s_axi_awid   (vif.awid),
        .s_axi_awaddr (vif.awaddr),
        .s_axi_awlen  (vif.awlen),
        .s_axi_awsize (vif.awsize),
        .s_axi_awburst(vif.awburst),
        .s_axi_awvalid(vif.awvalid),
        .s_axi_awready(vif.awready),

        .s_axi_wdata  (vif.wdata),
        .s_axi_wstrb  (vif.wstrb),
        .s_axi_wlast  (vif.wlast),
        .s_axi_wvalid (vif.wvalid),
        .s_axi_wready (vif.wready),

        .s_axi_bid    (vif.bid),
        .s_axi_bresp  (vif.bresp),
        .s_axi_bvalid (vif.bvalid),
        .s_axi_bready (vif.bready),

        .s_axi_arid   (vif.arid),
        .s_axi_araddr (vif.araddr),
        .s_axi_arlen  (vif.arlen),
        .s_axi_arsize (vif.arsize),
        .s_axi_arburst(vif.arburst),
        .s_axi_arvalid(vif.arvalid),
        .s_axi_arready(vif.arready),

        .s_axi_rid    (vif.rid),
        .s_axi_rdata  (vif.rdata),
        .s_axi_rresp  (vif.rresp),
        .s_axi_rlast  (vif.rlast),
        .s_axi_rvalid (vif.rvalid),
        .s_axi_rready (vif.rready)
    );

    // Full signal waveform, correlated against the UVM_ERROR/UVM_FATAL
    // timestamps in xsim.log (see scripts/run_uvm_sim.sh, which extracts
    // those timestamps into sim_uvm/mismatches.log after the run). Requires
    // xelab -debug typical so xsim honors $dumpvars.
    initial begin
        $dumpfile("waves.vcd");
        $dumpvars(0, tb_top);
    end

    initial begin
        uvm_config_db#(virtual axi_if)::set(null, "uvm_test_top.env.agent.driver", "vif", vif);
        uvm_config_db#(virtual axi_if)::set(null, "uvm_test_top.env.agent.monitor", "vif", vif);
        run_test();
    end

endmodule
