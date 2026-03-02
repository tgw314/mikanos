/**
 * @file usb/xhci/devmgr.hpp
 *
 * USB デバイスの管理機能．
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "error.hpp"
#include "usb/xhci/device.hpp"
#include "usb/xhci/registers.hpp"

namespace usb::xhci {
class DeviceManager {
   public:
    Error Initialize(size_t max_slots, bool csz);
    void **DeviceContexts() const;
    Device *FindByPort(uint8_t port_num, uint32_t route_string) const;
    Device *FindByState(enum Device::State state) const;
    Device *FindBySlot(uint8_t slot_id) const;
    Error AllocDevice(uint8_t slot_id, DoorbellRegister *dbreg);
    Error LoadDCBAA(uint8_t slot_id);
    Error Remove(uint8_t slot_id);
    bool CSZ() const { return csz_; }

   private:
    void **device_context_pointers_;
    size_t max_slots_;
    bool csz_;

    Device **devices_;
};
}  // namespace usb::xhci
