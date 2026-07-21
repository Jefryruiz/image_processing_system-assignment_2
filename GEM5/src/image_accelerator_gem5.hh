/**
 * image_accelerator_gem5.hh
 *
 * Wrapper GEM5 para el acelerador SystemC.
 * Ubicacion en GEM5: ~/gem5/src/dev/image_accelerator_gem5.hh
 */

#ifndef IMAGE_ACCELERATOR_GEM5_HH
#define IMAGE_ACCELERATOR_GEM5_HH

#include "base/addr_range.hh"
#include "mem/port.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "params/ImageAccelerator.hh"
#include "sim/sim_object.hh"

// TLM
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

// Acelerador original SystemC
#include "dev/image_accelerator.h"

namespace gem5 {

class ImageAcceleratorGem5 : public SimObject
{
  public:
    PARAMS(ImageAccelerator);

    ImageAcceleratorGem5(const Params &p);

    Port &getPort(const std::string &if_name,
                  PortID idx = InvalidPortID) override;

    AddrRangeList getAddrRanges() const;

  private:
    // Instancia del SC_MODULE original — sin modificar
    image_accelerator accel;

    // ---------------------------------------------------------------
    // AccelPort: ResponsePort de GEM5
    // Convierte Packet -> tlm_generic_payload -> b_transport
    // ---------------------------------------------------------------
    class AccelPort : public ResponsePort
    {
      public:
        AccelPort(const std::string &name, ImageAcceleratorGem5 *owner);

        AddrRangeList getAddrRanges() const override;

      protected:
        Tick recvAtomic(PacketPtr pkt) override;

        void recvFunctional(PacketPtr pkt) override;

        bool recvTimingReq(PacketPtr pkt) override;

        void recvRespRetry() override {}

      private:
        ImageAcceleratorGem5 *wrapper;

        void doAccess(PacketPtr pkt);
    };

    AccelPort cpu_side_port;
};

} // namespace gem5

#endif // IMAGE_ACCELERATOR_GEM5_HH
