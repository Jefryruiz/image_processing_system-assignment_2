/**
 * image_accelerator_gem5.hh
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
#include "dev/image_accelerator.h"

namespace gem5 {

class ImageAcceleratorGem5 : public SimObject
{
  public:
    PARAMS(ImageAccelerator);

    ImageAcceleratorGem5(const Params &p);

    // init() se llama despues de que todos los puertos estan conectados.
    // Aqui llamamos sendRangeChange() para notificar al bus.
    void init() override;

    Port &getPort(const std::string &if_name,
                  PortID idx = InvalidPortID) override;

    AddrRangeList getAddrRanges() const;

  private:
    image_accelerator accel;

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
