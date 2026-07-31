#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include "defines.h"
#include "../../dpi/ram_model.h"

// Storage for this module is delegated to verification/dpi/ram_model.c/.h:
// the same module is `import "DPI-C"`'d by the SystemVerilog/UVM testbench
// (verification/sv_tb/axi_ram_dpi_import.svh) to backdoor-load images and to check the
// axi_ram.sv RTL DUT against a golden reference. Linking it here means the
// RAM backing this TLM model and the RAM verified by the UVM environment
// are the exact same DPI-C component, satisfying the assignment's
// "integrate the RAM module using DPI/VPI" requirement.
SC_MODULE(ram_mem) {
    tlm_utils::simple_target_socket<ram_mem> target_socket;

    SC_CTOR(ram_mem) : target_socket("target_socket") {
        ram_model_init(sys_cfg::RAM_SIZE);
        target_socket.register_b_transport(this, &ram_mem::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
        // Handle incoming transactions
        tlm::tlm_command cmd = trans.get_command();
        tlm::tlm_generic_payload *p = &trans;
        uint64_t addr = p->get_address();
        unsigned char* data_ptr = p->get_data_ptr();
        unsigned int data_len = p->get_data_length();

        if (addr + data_len > ram_model_size()) {
            p->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            ram_model_write(addr, data_ptr, data_len);
        } else if (cmd == tlm::TLM_READ_COMMAND) {
            ram_model_read(addr, data_ptr, data_len);
        }

        p->set_response_status(tlm::TLM_OK_RESPONSE);
    }
};
