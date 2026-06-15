#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

SC_MODULE(image_accelerator) {
    tlm_utils::simple_target_socket<image_accelerator> target_socket;
    tlm_utils::simple_initiator_socket<image_accelerator> init_socket;

    SC_CTOR(image_accelerator) : target_socket("target_socket"), init_socket("initiator_socket") {
        target_socket.register_b_transport(this, &image_accelerator::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
        // Handle incoming transactions
    }
};
