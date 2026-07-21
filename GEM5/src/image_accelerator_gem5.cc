/**
 * image_accelerator_gem5.cc
 *
 * Implementacion del wrapper GEM5 para el acelerador SystemC.
 * Ubicacion en GEM5: ~/gem5/src/dev/image_accelerator_gem5.cc
 */

#include "dev/image_accelerator_gem5.hh"
#include "base/trace.hh"

#include <tlm.h>
#include <systemc.h>

namespace gem5 {

// -----------------------------------------------------------------------
// ImageAcceleratorGem5
// -----------------------------------------------------------------------
ImageAcceleratorGem5::ImageAcceleratorGem5(const Params &p)
    : SimObject(p),
      accel("image_accelerator"),
      cpu_side_port("cpu_side", this)
{}

Port &
ImageAcceleratorGem5::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "cpu_side")
        return cpu_side_port;
    return SimObject::getPort(if_name, idx);
}

AddrRangeList
ImageAcceleratorGem5::getAddrRanges() const
{
    AddrRangeList ranges;
    ranges.push_back(AddrRange(
        sys_cfg::ACCEL_BASE_ADDR,
        sys_cfg::ACCEL_BASE_ADDR + sys_cfg::ACCEL_REG_SIZE - 1
    ));
    return ranges;
}

// -----------------------------------------------------------------------
// AccelPort
// -----------------------------------------------------------------------
ImageAcceleratorGem5::AccelPort::AccelPort(
    const std::string &name, ImageAcceleratorGem5 *owner)
    : ResponsePort(name, owner), wrapper(owner)
{}

AddrRangeList
ImageAcceleratorGem5::AccelPort::getAddrRanges() const
{
    return wrapper->getAddrRanges();
}

void
ImageAcceleratorGem5::AccelPort::doAccess(PacketPtr pkt)
{
    // Construir tlm_generic_payload desde el Packet de GEM5
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

    // Ajustar offset desde ACCEL_BASE (igual que routing.h)
    uint64_t offset = pkt->getAddr() - sys_cfg::ACCEL_BASE_ADDR;

    trans.set_address(offset);
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
    // target_socket es un simple_target_socket que expone b_transport
    // directamente mediante operator->
    tlm_utils::simple_target_socket<image_accelerator> &sock =
        wrapper->accel.target_socket;

    sock->b_transport(trans, delay);
}

Tick
ImageAcceleratorGem5::AccelPort::recvAtomic(PacketPtr pkt)
{
    doAccess(pkt);
    pkt->makeAtomicResponse();
    return 1; // 1 tick de latencia
}

void
ImageAcceleratorGem5::AccelPort::recvFunctional(PacketPtr pkt)
{
    doAccess(pkt);
    pkt->makeResponse();
}

bool
ImageAcceleratorGem5::AccelPort::recvTimingReq(PacketPtr pkt)
{
    doAccess(pkt);
    pkt->makeTimingResponse();
    sendTimingResp(pkt);
    return true;
}

// -----------------------------------------------------------------------
// Factory — GEM5 llama esto para instanciar desde Python
// -----------------------------------------------------------------------
ImageAcceleratorGem5 *
ImageAcceleratorParams::create() const
{
    return new ImageAcceleratorGem5(*this);
}

} // namespace gem5
