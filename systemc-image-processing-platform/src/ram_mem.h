#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>
#include "defines.h"

SC_MODULE(ram_mem) {
    tlm_utils::simple_target_socket<ram_mem> target_socket;
    std::vector<uint8_t> memory;

    SC_CTOR(ram_mem) : target_socket("target_socket") {
        memory.resize(sys_cfg::RAM_SIZE, 0);
        target_socket.register_b_transport(this, &ram_mem::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
        // Handle incoming transactions
        tlm::tlm_command cmd = trans.get_command();
        tlm::tlm_generic_payload *p = &trans;
        uint64_t addr = p->get_address();
        unsigned char* data_ptr = p->get_data_ptr();
        unsigned int data_len = p->get_data_length();

        if (addr + data_len > memory.size()) {
            p->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            std::copy(data_ptr, data_ptr + data_len, memory.begin() + addr);
        } else if (cmd == tlm::TLM_READ_COMMAND) {
            std::copy(memory.begin() + addr, memory.begin() + addr + data_len, data_ptr);
        }

        p->set_response_status(tlm::TLM_OK_RESPONSE);
    }
};
