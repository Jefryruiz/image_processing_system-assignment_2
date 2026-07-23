/**
 * image_accelerator_gem5.cc
 * Ubicacion en GEM5: ~/gem5/src/dev/image_accelerator_gem5.cc
 */

#include "dev/image_accelerator_gem5.hh"

#include <tlm.h>
#include <systemc>

namespace gem5 {

// -----------------------------------------------------------------------
// ImageAcceleratorGem5
// -----------------------------------------------------------------------
ImageAcceleratorGem5::ImageAcceleratorGem5(const Params &p)
    : SimObject(p),
      accel("image_accelerator"),
      cpu_side_port("cpu_side", this)
{}

void
ImageAcceleratorGem5::init()
{
    // Notificar al bus que este dispositivo tiene rangos de direcciones.
    // Sin esto el bus nunca registra 0x10000000 y falla con gotAllAddrRanges.
    cpu_side_port.sendRangeChange();
}

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
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

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

    tlm::tlm_fw_transport_if<> &fw_if =
        static_cast<tlm::tlm_fw_transport_if<> &>(
            wrapper->accel.target_socket);
    fw_if.b_transport(trans, delay);
}

Tick
ImageAcceleratorGem5::AccelPort::recvAtomic(PacketPtr pkt)
{
    doAccess(pkt);
    pkt->makeAtomicResponse();
    return 1;
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

} // namespace gem5
