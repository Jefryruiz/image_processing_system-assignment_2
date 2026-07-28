class axi_ram_base_test extends uvm_test;
    `uvm_component_utils(axi_ram_base_test)

    axi_ram_env env;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = axi_ram_env::type_id::create("env", this);
        ram_model_init(64 * 1024 * 1024);
    endfunction

endclass


// Random read/write burst regression against the golden model.
class axi_ram_random_test extends axi_ram_base_test;
    `uvm_component_utils(axi_ram_random_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        axi_ram_random_seq seq = axi_ram_random_seq::type_id::create("seq");
        phase.raise_objection(this);
        if (!$value$plusargs("NUM_BURSTS=%d", seq.num_bursts))
            seq.num_bursts = 50;
        seq.start(env.agent.sequencer);
        phase.drop_objection(this);
    endtask
endclass


// Streams the project's real RAW RGB image through the DUT and dumps the
// read-back contents to sv_tb_output.raw - the RTL/UVM counterpart of the
// SystemC flow's disk round-trip, driven over the same DPI-C golden model.
class axi_ram_image_test extends axi_ram_base_test;
    `uvm_component_utils(axi_ram_image_test)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    task run_phase(uvm_phase phase);
        axi_ram_image_seq seq = axi_ram_image_seq::type_id::create("seq");
        phase.raise_objection(this);
        if (!$value$plusargs("IMG_PATH=%s", seq.input_path))
            seq.input_path = "input.raw";
        if (!$value$plusargs("IMG_OUT=%s", seq.output_path))
            seq.output_path = "sv_tb_output.raw";
        if (!$value$plusargs("IMG_BYTES=%d", seq.num_bytes))
            seq.num_bytes = 262144;
        seq.start(env.agent.sequencer);
        phase.drop_objection(this);
    endtask
endclass
