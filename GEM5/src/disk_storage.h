#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <fstream>
#include <cstring>

SC_MODULE(disk_storage) {
    tlm_utils::simple_target_socket<disk_storage> target_socket;
    std::ofstream output_file;

    // Note: this module expects binary raw pixel files. 
    SC_CTOR(disk_storage) : target_socket("target_socket"), output_file("../output.raw", std::ios::out | std::ios::binary) {
        target_socket.register_b_transport(this, &disk_storage::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
        tlm::tlm_command cmd = trans.get_command();
        uint64_t addr = trans.get_address();
        unsigned char* data_ptr = trans.get_data_ptr();
        unsigned int data_len = trans.get_data_length();

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            // Write incoming data to the output file at the given address offset
            if (!output_file.is_open()) {
                trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                return;
            }
            output_file.seekp(static_cast<std::streamoff>(addr));
            output_file.write(reinterpret_cast<char*>(data_ptr), data_len);
            output_file.flush();
        } else if (cmd == tlm::TLM_READ_COMMAND) {
            // Read from an input.raw file into the transaction data buffer
            std::ifstream in("../input.raw", std::ios::in | std::ios::binary);
            if (!in) {
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                return;
            }
            in.seekg(static_cast<std::streamoff>(addr));
            in.read(reinterpret_cast<char*>(data_ptr), data_len);
            std::streamsize read_bytes = in.gcount();
            if (static_cast<unsigned int>(read_bytes) < data_len) {
                // zero-pad the remainder
                std::memset(data_ptr + read_bytes, 0, data_len - static_cast<unsigned int>(read_bytes));
            }
        }

        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
};
