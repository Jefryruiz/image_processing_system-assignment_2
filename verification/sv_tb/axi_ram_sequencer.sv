class axi_ram_sequencer extends uvm_sequencer #(axi_ram_seq_item);
    `uvm_component_utils(axi_ram_sequencer)

    function new(string name, uvm_component parent);
        super.new(name, parent);
    endfunction
endclass
