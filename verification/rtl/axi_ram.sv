// AXI4-Full RAM slave.
//
// Byte-addressable memory with a full AXI4 (not AXI4-Lite) slave interface:
// separate write-address/write-data/write-response and read-address/read-data
// channels, INCR burst support, per-byte write strobes (WSTRB) and
// configurable data width. Depth defaults to 64 MB to match the system's
// RAM sizing (sys_cfg::RAM_SIZE in the SystemC model).
module axi_ram #(
    parameter int ADDR_WIDTH = 26,          // 2^26 = 64 MB
    parameter int DATA_WIDTH = 32,
    parameter int ID_WIDTH   = 4,
    parameter int MEM_BYTES  = (1 << ADDR_WIDTH)
) (
    input  logic                     aclk,
    input  logic                     aresetn,

    // Write address channel
    input  logic [ID_WIDTH-1:0]      s_axi_awid,
    input  logic [ADDR_WIDTH-1:0]    s_axi_awaddr,
    input  logic [7:0]               s_axi_awlen,
    input  logic [2:0]               s_axi_awsize,
    input  logic [1:0]               s_axi_awburst,
    input  logic                     s_axi_awvalid,
    output logic                     s_axi_awready,

    // Write data channel
    input  logic [DATA_WIDTH-1:0]    s_axi_wdata,
    input  logic [DATA_WIDTH/8-1:0]  s_axi_wstrb,
    input  logic                     s_axi_wlast,
    input  logic                     s_axi_wvalid,
    output logic                     s_axi_wready,

    // Write response channel
    output logic [ID_WIDTH-1:0]      s_axi_bid,
    output logic [1:0]               s_axi_bresp,
    output logic                     s_axi_bvalid,
    input  logic                     s_axi_bready,

    // Read address channel
    input  logic [ID_WIDTH-1:0]      s_axi_arid,
    input  logic [ADDR_WIDTH-1:0]    s_axi_araddr,
    input  logic [7:0]               s_axi_arlen,
    input  logic [2:0]               s_axi_arsize,
    input  logic [1:0]               s_axi_arburst,
    input  logic                     s_axi_arvalid,
    output logic                     s_axi_arready,

    // Read data channel
    output logic [ID_WIDTH-1:0]      s_axi_rid,
    output logic [DATA_WIDTH-1:0]    s_axi_rdata,
    output logic [1:0]               s_axi_rresp,
    output logic                     s_axi_rlast,
    output logic                     s_axi_rvalid,
    input  logic                     s_axi_rready
);

    localparam int STRB_WIDTH  = DATA_WIDTH / 8;
    localparam int BYTE_SHIFT  = $clog2(STRB_WIDTH);

    // Backing store
    logic [7:0] mem [0:MEM_BYTES-1];

    // ---------------------------------------------------------------
    // Write channel
    // ---------------------------------------------------------------
    typedef enum logic [1:0] {W_IDLE, W_DATA, W_RESP} wstate_t;
    wstate_t wstate;

    logic [ID_WIDTH-1:0]   aw_id;
    logic [ADDR_WIDTH-1:0] aw_addr;
    logic [7:0]            aw_len;
    logic [7:0]            w_beat_cnt;

    assign s_axi_awready = (wstate == W_IDLE);
    assign s_axi_wready  = (wstate == W_DATA);
    assign s_axi_bvalid  = (wstate == W_RESP);
    assign s_axi_bid     = aw_id;
    assign s_axi_bresp   = 2'b00; // OKAY

    always_ff @(posedge aclk or negedge aresetn) begin
        if (!aresetn) begin
            wstate     <= W_IDLE;
            aw_id      <= '0;
            aw_addr    <= '0;
            aw_len     <= '0;
            w_beat_cnt <= '0;
        end else begin
            unique case (wstate)
                W_IDLE: begin
                    if (s_axi_awvalid) begin
                        aw_id      <= s_axi_awid;
                        aw_addr    <= s_axi_awaddr;
                        aw_len     <= s_axi_awlen;
                        w_beat_cnt <= '0;
                        wstate     <= W_DATA;
                    end
                end

                W_DATA: begin
                    if (s_axi_wvalid && s_axi_wready) begin
                        automatic logic [ADDR_WIDTH-1:0] beat_addr;
                        beat_addr = aw_addr + (w_beat_cnt << BYTE_SHIFT);
                        for (int b = 0; b < STRB_WIDTH; b++) begin
                            if (s_axi_wstrb[b]) begin
                                mem[beat_addr + b] <= s_axi_wdata[8*b +: 8];
                            end
                        end
                        w_beat_cnt <= w_beat_cnt + 1'b1;
                        if (s_axi_wlast) begin
                            wstate <= W_RESP;
                        end
                    end
                end

                W_RESP: begin
                    if (s_axi_bready) begin
                        wstate <= W_IDLE;
                    end
                end

                default: wstate <= W_IDLE;
            endcase
        end
    end

    // ---------------------------------------------------------------
    // Read channel
    // ---------------------------------------------------------------
    typedef enum logic [1:0] {R_IDLE, R_DATA} rstate_t;
    rstate_t rstate;

    logic [ID_WIDTH-1:0]   ar_id;
    logic [ADDR_WIDTH-1:0] ar_addr;
    logic [7:0]            ar_len;
    logic [7:0]            r_beat_cnt;
    logic [DATA_WIDTH-1:0] rdata_reg;

    assign s_axi_arready = (rstate == R_IDLE);
    assign s_axi_rvalid  = (rstate == R_DATA);
    assign s_axi_rid     = ar_id;
    assign s_axi_rresp   = 2'b00; // OKAY
    assign s_axi_rlast   = (rstate == R_DATA) && (r_beat_cnt == ar_len);
    assign s_axi_rdata   = rdata_reg;

    // Memory reads are registered (BRAM-style) rather than driven from a
    // variable-indexed always_comb read of the full array: xsim does not
    // fully support combinational sensitivity on large indexed array reads
    // (see the "Sensitivity on Associative Array" elaboration warning) and
    // that pattern spins in a zero-time delta-cycle loop instead of
    // advancing simulation time.
    always_ff @(posedge aclk or negedge aresetn) begin
        if (!aresetn) begin
            rstate     <= R_IDLE;
            ar_id      <= '0;
            ar_addr    <= '0;
            ar_len     <= '0;
            r_beat_cnt <= '0;
            rdata_reg  <= '0;
        end else begin
            unique case (rstate)
                R_IDLE: begin
                    if (s_axi_arvalid) begin
                        automatic logic [ADDR_WIDTH-1:0] beat_addr = s_axi_araddr;
                        ar_id      <= s_axi_arid;
                        ar_addr    <= s_axi_araddr;
                        ar_len     <= s_axi_arlen;
                        r_beat_cnt <= '0;
                        rstate     <= R_DATA;
                        for (int b = 0; b < STRB_WIDTH; b++)
                            rdata_reg[8*b +: 8] <= mem[beat_addr + b];
                    end
                end

                R_DATA: begin
                    if (s_axi_rready) begin
                        if (r_beat_cnt == ar_len) begin
                            rstate <= R_IDLE;
                        end else begin
                            automatic logic [ADDR_WIDTH-1:0] beat_addr;
                            beat_addr = ar_addr + ((r_beat_cnt + 1'b1) << BYTE_SHIFT);
                            for (int b = 0; b < STRB_WIDTH; b++)
                                rdata_reg[8*b +: 8] <= mem[beat_addr + b];
                            r_beat_cnt <= r_beat_cnt + 1'b1;
                        end
                    end
                end

                default: rstate <= R_IDLE;
            endcase
        end
    end

    // ---------------------------------------------------------------
    // DPI-C backdoor: lets the UVM golden-model scoreboard and stimulus
    // generators (sv_tb/axi_ram_seq_lib.sv) load/dump the same RAW image
    // files the SystemC model uses, and cross-check DUT memory contents
    // against dpi/ram_model.c without going through the AXI protocol.
    // ---------------------------------------------------------------
    export "DPI-C" function axi_ram_backdoor_read;
    export "DPI-C" function axi_ram_backdoor_write;

    function automatic byte unsigned axi_ram_backdoor_read(input longint unsigned addr);
        return mem[addr];
    endfunction

    function automatic void axi_ram_backdoor_write(input longint unsigned addr, input byte unsigned data);
        mem[addr] = data;
    endfunction

endmodule
