/**
 * image_accelerator_gem5.hh
 *
 * Adaptacion del acelerador para GEM5.
 * Este archivo es SEPARADO del original image_accelerator.h
 * para evitar conflictos entre SC_MODULE y gem5::SimObject.
 *
 * Ubicacion en GEM5: ~/gem5/src/dev/image_accelerator_gem5.hh
 */

#ifndef IMAGE_ACCELERATOR_GEM5_HH
#define IMAGE_ACCELERATOR_GEM5_HH

#include "mem/port.hh"
#include "params/ImageAccelerator.hh"
#include "sim/sim_object.hh"
#include "image_accelerator.h"  // modulo original SC_MODULE

namespace gem5 {

/**
 * ImageAcceleratorGem5
 *
 * Wrapper de GEM5 alrededor del SC_MODULE original.
 * Hereda de SimObject (GEM5) y contiene una instancia del
 * image_accelerator original (SystemC).
 *
 * Patron: Wrapper/Adapter — el SC_MODULE no se modifica.
 */
class ImageAcceleratorGem5 : public SimObject {
public:
    PARAMS(ImageAccelerator);

    ImageAcceleratorGem5(const Params &p)
        : SimObject(p),
          // Instancia el acelerador original sin modificarlo
          accel("image_accelerator"),
          cpu_side_port("cpu_side", this)
    {}

    // -------------------------------------------------------------------
    // ADAPTATION 1: getAddrRanges()
    // GEM5 llama esto para construir el mapa de memoria del sistema.
    // -------------------------------------------------------------------
    AddrRangeList getAddrRanges() const {
        AddrRangeList ranges;
        ranges.push_back(AddrRange(
            sys_cfg::ACCEL_BASE_ADDR,
            sys_cfg::ACCEL_BASE_ADDR + sys_cfg::ACCEL_REG_SIZE - 1
        ));
        return ranges;
    }

    Port &getPort(const std::string &if_name,
                  PortID idx = InvalidPortID) override {
        if (if_name == "cpu_side")
            return cpu_side_port;
        return SimObject::getPort(if_name, idx);
    }

private:
    // Instancia del acelerador original (SC_MODULE) — sin modificar
    image_accelerator accel;

    // -------------------------------------------------------------------
    // ADAPTATION 2: AccelPort
    // Convierte GEM5 Packet -> tlm_generic_payload -> b_transport
    // del acelerador original.
    // -------------------------------------------------------------------
    class AccelPort : public SimpleTimingPort {
    public:
        AccelPort(const std::string &name, ImageAcceleratorGem5 *owner)
            : SimpleTimingPort(name, owner), wrapper(owner) {}

        AddrRangeList getAddrRanges() const override {
            return wrapper->getAddrRanges();
        }

    protected:
        Tick recvAtomic(PacketPtr pkt) override {
            tlm::tlm_generic_payload trans;
            sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

            // Ajustar offset igual que routing.h
            trans.set_address(pkt->getAddr() - sys_cfg::ACCEL_BASE_ADDR);
            trans.set_data_ptr(pkt->getPtr<unsigned char>());
            trans.set_data_length(pkt->getSize());
            trans.set_streaming_width(pkt->getSize());
            trans.set_byte_enable_ptr(nullptr);
            trans.set_dmi_allowed(false);
            trans.set_command(pkt->isWrite()
                ? tlm::TLM_WRITE_COMMAND
                : tlm::TLM_READ_COMMAND);
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            // Llamar b_transport del SC_MODULE original
            wrapper->accel.target_socket->b_transport(trans, delay);

            pkt->makeAtomicResponse();
            return 1;
        }

    private:
        ImageAcceleratorGem5 *wrapper;
    };

    AccelPort cpu_side_port;
};

} // namespace gem5

#endif // IMAGE_ACCELERATOR_GEM5_HH
