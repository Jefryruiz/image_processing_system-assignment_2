#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

#include <vector>
#include <cstring>
#include <iostream>
#include "defines.h"

SC_MODULE(image_accelerator) {
    // Recibe configuracion y control del CPU (via bus)
    tlm_utils::simple_target_socket<image_accelerator>    target_socket;
    // Accede a RAM para leer RGB y escribir Gray (via bus)
    tlm_utils::simple_initiator_socket<image_accelerator> init_socket;

    SC_CTOR(image_accelerator)
        : target_socket("target_socket"), init_socket("initiator_socket"),
          reg_base_in(0), reg_base_out(0), reg_num_pixels(0), reg_status(0)
    {
        target_socket.register_b_transport(this, &image_accelerator::b_transport);
        SC_THREAD(process_image);
    }

private:
    // Registros internos de control y estado
    uint32_t reg_base_in;
    uint32_t reg_base_out;
    uint32_t reg_num_pixels;
    uint32_t reg_status;

    // Evento que dispara el procesamiento
    sc_event start_event;

    // b_transport: recibe escrituras o lecturas de registros desde l CPU
    // Las direcciones llegan ya ajustadas por el routing (offset desde ACCEL_BASE)
    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        uint64_t       addr     = trans.get_address();
        unsigned char* ptr      = trans.get_data_ptr();
        bool           is_write = (trans.get_command() == tlm::TLM_WRITE_COMMAND);

        // REG_CONTROL: arrancar el procesamiento
        if (addr == 0x0C) {
            if (is_write) {
                uint32_t val = 0;
                std::memcpy(&val, ptr, sizeof(uint32_t));
                if (val == 1) {
                    reg_status = 0;
                    start_event.notify();
                    std::cout << "@" << sc_time_stamp()
                              << " [Acelerador] Inicio de procesamiento." << std::endl;
                }
            }
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
            return;
        }

        // Mapa del resto de registros (offset desde ACCEL_BASE_ADDR)
        uint32_t* reg = nullptr;
        if      (addr == 0x00) reg = &reg_base_in;
        else if (addr == 0x04) reg = &reg_base_out;
        else if (addr == 0x08) reg = &reg_num_pixels;
        else if (addr == 0x10) reg = &reg_status;
        else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        if (is_write) std::memcpy(reg, ptr, sizeof(uint32_t));
        else          std::memcpy(ptr, reg, sizeof(uint32_t));

        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    // Acceso a RAM via init_socket -> routing -> ram_memory
    void mem_access(tlm::tlm_command cmd, uint64_t addr, unsigned char* data, unsigned int len) {
        tlm::tlm_generic_payload trans;
        sc_time delay = SC_ZERO_TIME;

        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(data);
        trans.set_data_length(len);
        trans.set_streaming_width(len);
        trans.set_byte_enable_ptr(nullptr);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        init_socket->b_transport(trans, delay);
        wait(delay);

        if (trans.get_response_status() != tlm::TLM_OK_RESPONSE)
            std::cerr << "[Acelerador] Error de memoria @0x"
                      << std::hex << addr << std::dec << std::endl;
    }

    // SC_THREAD: conversion RGB -> Gris, fila por fila
    // Conversion BT.601: gray = 0.299*R + 0.587*G + 0.114*B
    void process_image() {
        std::vector<unsigned char> row_rgb (sys_cfg::IMG_WIDTH * sys_cfg::IMG_CHANNELS);
        std::vector<unsigned char> row_gray(sys_cfg::IMG_WIDTH);

        while (true) {
            wait(start_event);

            std::cout << "@" << sc_time_stamp()
                      << " [Acelerador] Procesando " << reg_num_pixels << " pixeles."
                      << " base_in=0x"  << std::hex << reg_base_in
                      << " base_out=0x" << reg_base_out << std::dec << std::endl;

            for (int y = 0; y < sys_cfg::IMG_HEIGHT; y++) {
                uint64_t addr_in  = reg_base_in  + (uint64_t)y * sys_cfg::IMG_WIDTH * sys_cfg::IMG_CHANNELS;
                uint64_t addr_out = reg_base_out + (uint64_t)y * sys_cfg::IMG_WIDTH;

                // 1. Leer fila RGB desde RAM (5,760 bytes)
                mem_access(tlm::TLM_READ_COMMAND, addr_in, row_rgb.data(), row_rgb.size());

                // 2. Conversion BT.601 pixel a pixel
                for (int x = 0; x < sys_cfg::IMG_WIDTH; x++) {
                    unsigned char r = row_rgb[x * 3 + 0];
                    unsigned char g = row_rgb[x * 3 + 1];
                    unsigned char b = row_rgb[x * 3 + 2];
                    row_gray[x] = static_cast<unsigned char>(
                        0.299f * r + 0.587f * g + 0.114f * b
                    );
                }

                // 3. Escribir fila Gray en RAM (1,920 bytes)
                mem_access(tlm::TLM_WRITE_COMMAND, addr_out, row_gray.data(), row_gray.size());

                wait(sc_time(100, SC_NS)); // latencia por fila
            }

            reg_status = 1;
            std::cout << "@" << sc_time_stamp()
                      << " [Acelerador] Procesamiento completo." << std::endl;
        }
    }
};
