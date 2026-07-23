#ifndef __GEM5_BRIDGE_H__
#define __GEM5_BRIDGE_H__

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <queue>
#include "defines.h"

/**
 * @brief Bridge module to connect gem5's ports to your SystemC platform
 * 
 * This module acts as an adapter between:
 * - gem5's memory interface (if gem5 CPU is present)
 * - Your SystemC TLM-2.0 platform
 * 
 * USE CASE: When you want gem5 to drive transactions through your platform
 * 
 * EXAMPLE:
 *   gem5_bridge gem5_bridge_inst("gem5_bridge");
 *   gem5_bridge_inst.init_socket.bind(your_platform_target_socket);
 */
SC_MODULE(gem5_bridge) {
    // Socket to connect to your SystemC platform
    tlm_utils::simple_initiator_socket<gem5_bridge> init_socket;
    
    // Optional: target socket if gem5 needs to write back
    tlm_utils::simple_target_socket<gem5_bridge> target_socket;

    // Statistics
    sc_uint<64> transaction_count;
    sc_time total_latency;

    SC_CTOR(gem5_bridge) 
        : init_socket("init_socket"), target_socket("target_socket"),
          transaction_count(0), total_latency(SC_ZERO_TIME) 
    {
        target_socket.register_b_transport(this, &gem5_bridge::b_transport);
    }

    /**
     * @brief Forward transaction from gem5 to SystemC platform
     * 
     * @param trans TLM generic payload from gem5
     * @param delay Accumulated latency
     */
    virtual void b_transport(tlm::tlm_generic_payload& trans, 
                           sc_time& delay) {
        // Record transaction start time
        sc_time start_time = sc_time_stamp();
        
        // Log transaction
        log_transaction(trans, delay, "gem5->SystemC");
        
        // Forward to your platform
        init_socket->b_transport(trans, delay);
        
        // Update statistics
        transaction_count++;
        total_latency += (sc_time_stamp() - start_time) + delay;
        
        // Update response
        if (trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
            std::cerr << "[gem5_bridge] Transaction failed at @"
                      << std::hex << trans.get_address() << std::dec 
                      << std::endl;
        }
    }

    /**
     * @brief Log transaction details
     */
    void log_transaction(const tlm::tlm_generic_payload& trans,
                        const sc_time& delay,
                        const char* direction) {
        std::cout << "@" << sc_time_stamp() 
                  << " [" << direction << "] ";
        
        if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            std::cout << "READ";
        } else if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            std::cout << "WRITE";
        } else {
            std::cout << "UNKNOWN";
        }
        
        std::cout << " @0x" << std::hex << trans.get_address() << std::dec
                  << " len=" << trans.get_data_length()
                  << " delay=" << delay << std::endl;
    }

    /**
     * @brief Get statistics
     */
    void print_stats() {
        std::cout << "\n=== gem5_bridge Statistics ===" << std::endl;
        std::cout << "Total transactions: " << transaction_count << std::endl;
        std::cout << "Total latency: " << total_latency << std::endl;
        if (transaction_count > 0) {
            std::cout << "Average latency: " 
                      << (total_latency.to_double() / transaction_count.to_uint64()) 
                      << " ps" << std::endl;
        }
        std::cout << "==============================\n" << std::endl;
    }
};

#endif // __GEM5_BRIDGE_H__
