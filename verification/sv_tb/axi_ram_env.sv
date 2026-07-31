class axi_ram_env extends uvm_env;
    `uvm_component_utils(axi_ram_env)

    axi_ram_agent      agent;
    axi_ram_scoreboard scoreboard;

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        agent      = axi_ram_agent::type_id::create("agent", this);
        scoreboard = axi_ram_scoreboard::type_id::create("scoreboard", this);
        uvm_config_db#(uvm_active_passive_enum)::set(this, "agent", "is_active", UVM_ACTIVE);
    endfunction

    function void connect_phase(uvm_phase phase);
        super.connect_phase(phase);
        agent.monitor.ap.connect(scoreboard.analysis_export);
    endfunction

endclass
