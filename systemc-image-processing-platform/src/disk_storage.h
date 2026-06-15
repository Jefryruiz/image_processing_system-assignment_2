#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <fstream>

SC_MODULE(disk_storage) {
    tlm_utils::simple_target_socket<disk_storage> target_socket;
    std::ifstream input_file;
    std::ofstream output_file;

    SC_CTOR(disk_storage) : target_socket("target_socket"), output_file("output.txt", std::ios::out) {
        target_socket.register_b_transport(this, &disk_storage::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
        // Handle incoming transactions
    }
};
