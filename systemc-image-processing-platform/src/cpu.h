
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

#include <cstring>
#include <iostream>
#include "defines.h"

SC_MODULE(cpu) {
    tlm_utils::simple_initiator_socket<cpu> socket;

    SC_CTOR(cpu) : socket("socket") {
        SC_THREAD(run);
    }

    // Funcion generica para enviar una transaccion TLM
    void transfer(tlm::tlm_command cmd, uint64_t addr, unsigned char* data, unsigned int len) {
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

        socket->b_transport(trans, delay);
        wait(delay);

        if (trans.get_response_status() != tlm::TLM_OK_RESPONSE)
            std::cerr << "[CPU] Error en transaccion @0x"
                      << std::hex << addr << std::dec << std::endl;
    }

    void write_data(uint64_t addr, unsigned char* data, unsigned int len) {
        transfer(tlm::TLM_WRITE_COMMAND, addr, data, len);
    }

    void read_data(uint64_t addr, unsigned char* data, unsigned int len) {
        transfer(tlm::TLM_READ_COMMAND, addr, data, len);
    }

    void write_reg32(uint64_t addr, uint32_t val) {
        write_data(addr, reinterpret_cast<unsigned char*>(&val), sizeof(uint32_t));
    }

    uint32_t read_reg32(uint64_t addr) {
        uint32_t val = 0;
        read_data(addr, reinterpret_cast<unsigned char*>(&val), sizeof(uint32_t));
        return val;
    }


    // Flujo principal del sistema
    void run() {

        // PASO 1: Leer imagen RGB desde disk hacia RAM
        // El routing envia la transaccion a disk_storage (lectura desde input.raw) y el resultado se escribe directamente en el buffer.
        // Luego ese buffer se escribe en RAM.
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 1: Cargando imagen desde disco..." << std::endl;

        // Buffer temporal para la imagen RGB
        std::vector<unsigned char> img_rgb(sys_cfg::RGB_SIZE);

        // READ desde disco: routing envia a disk_storage -> lee input.raw
        read_data(sys_cfg::DISK_BASE_ADDR, img_rgb.data(), sys_cfg::RGB_SIZE);

        // WRITE hacia RAM: routing envia a ram_mem
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 1b: Escribiendo imagen en RAM..." << std::endl;
        write_data(sys_cfg::INPUT_IMG_ADDR, img_rgb.data(), sys_cfg::RGB_SIZE);

        // PASO 2: Configurar registros del acelerador
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 2: Configurando acelerador..." << std::endl;

        write_reg32(sys_cfg::REG_BASE_IN,    static_cast<uint32_t>(sys_cfg::INPUT_IMG_ADDR));
        write_reg32(sys_cfg::REG_BASE_OUT,   static_cast<uint32_t>(sys_cfg::OUTPUT_IMG_ADDR));
        write_reg32(sys_cfg::REG_NUM_PIXELS, static_cast<uint32_t>(sys_cfg::IMG_WIDTH * sys_cfg::IMG_HEIGHT));

        // PASO 3: Iniciar acelerador
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 3: Iniciando acelerador..." << std::endl;
        write_reg32(sys_cfg::REG_CONTROL, 1);

        // PASO 4: Polling — esperar REG_STATUS == 1 (acelerador listo)
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 4: Esperando acelerador..." << std::endl;
        uint32_t status = 0;
        while (status == 0) {
            wait(sc_time(100, SC_NS));
            status = read_reg32(sys_cfg::REG_STATUS);
        }
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Acelerador listo." << std::endl;

        // PASO 5: Leer imagen gris de RAM y guardarla en disco
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 5: Leyendo imagen gris de RAM..." << std::endl;

        std::vector<unsigned char> img_gray(sys_cfg::GRAY_SIZE);
        read_data(sys_cfg::OUTPUT_IMG_ADDR, img_gray.data(), sys_cfg::GRAY_SIZE);

        // WRITE hacia disco: routing envia a disk_storage -> escribe output.raw
        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Paso 5b: Guardando imagen en disco..." << std::endl;
        write_data(sys_cfg::DISK_BASE_ADDR, img_gray.data(), sys_cfg::GRAY_SIZE);

        std::cout << "@" << sc_time_stamp()
                  << " [CPU] Flujo completo. Deteniendo simulacion." << std::endl;
        sc_stop();
    }
};

