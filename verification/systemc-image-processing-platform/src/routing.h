#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include "defines.h"

SC_MODULE(routing) {
    //bus targets to receive pkts
    tlm_utils::simple_target_socket<routing> target_cpu;
    tlm_utils::simple_target_socket<routing> target_accelator;
    // bus initiators to send pkts
    tlm_utils::simple_initiator_socket<routing> init_ram_mem;
    tlm_utils::simple_initiator_socket<routing> init_disk_storage;
    tlm_utils::simple_initiator_socket<routing> init_accelator;

    SC_CTOR(routing) : target_cpu("target_cpu"), target_accelator("target_accelator"),
                   init_ram_mem("init_ram_mem"), init_disk_storage("init_disk_storage"), init_accelator("init_accelator") {
        target_cpu.register_b_transport(this, &routing::b_transport_cpu);
        target_accelator.register_b_transport(this, &routing::b_transport_accel);
    }

    void b_transport_cpu(tlm::tlm_generic_payload& trans, sc_time& delay) {
        // handle incoming transactions
        routing_data(trans, delay);
    }

    void b_transport_accel(tlm::tlm_generic_payload& trans, sc_time& delay) {
        // handle incoming transactions
        routing_data(trans, delay);
    }

    // Helper: route a transaction based on its address to the correct initiator
    void routing_data(tlm::tlm_generic_payload& trans, sc_time& delay) {
        uint64_t addr = trans.get_address();

        // Route to RAM
        if (addr >= sys_cfg::RAM_BASE_ADDR && addr < (sys_cfg::RAM_BASE_ADDR + sys_cfg::RAM_SIZE)) {
            trans.set_address(addr - sys_cfg::RAM_BASE_ADDR);
            init_ram_mem->b_transport(trans, delay);
            return;
        }

        // Route to Disk (assume disk region covers image size)
        if (addr >= sys_cfg::DISK_BASE_ADDR && addr < (sys_cfg::DISK_BASE_ADDR + sys_cfg::RGB_SIZE)) {
            trans.set_address(addr - sys_cfg::DISK_BASE_ADDR);
            init_disk_storage->b_transport(trans, delay);
            return;
        }

        // Route to Accelerator
        if (addr >= sys_cfg::ACCEL_BASE_ADDR && addr < (sys_cfg::ACCEL_BASE_ADDR + sys_cfg::RGB_SIZE)) {
            trans.set_address(addr - sys_cfg::ACCEL_BASE_ADDR);
            init_accelator->b_transport(trans, delay);
            return;
        }

        // No matching target
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
};
