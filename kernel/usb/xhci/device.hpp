/**
 * @file usb/xhci/device.hpp
 *
 * USB デバイスを表すクラスと関連機能．
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "error.hpp"
#include "usb/arraymap.hpp"
#include "usb/device.hpp"
#include "usb/xhci/context.hpp"
#include "usb/xhci/registers.hpp"
#include "usb/xhci/trb.hpp"

namespace usb::xhci {
class Device : public usb::Device {
   public:
    enum class State { kInvalid, kBlank, kSlotAssigning, kSlotAssigned };

    using OnTransferredCallbackType = void(Device *dev, DeviceContextIndex dci,
                                           int completion_code,
                                           int trb_transfer_length,
                                           TRB *issue_trb);

    Device(uint8_t slot_id, DoorbellRegister *dbreg, bool csz);
    ~Device();

    Error Initialize();

    void *DeviceContext() { return ctx_; }
    void *InputContext() { return input_ctx_; }
    bool CSZ() const { return csz_; }

    SlotContext *SlotCtx() { return GetSlotContext(ctx_); }
    const SlotContext *SlotCtx() const { return GetSlotContext(ctx_); }
    EndpointContext *EndpointCtx(int dci) { return GetEndpointContext(ctx_, dci, csz_); }

    InputControlContext *InputCtrlCtx() { return GetInputControlContext(input_ctx_); }
    SlotContext *InputSlotCtx() { return GetInputSlotContext(input_ctx_, csz_); }
    EndpointContext *InputEndpointCtx(int dci) { return GetInputEndpointContext(input_ctx_, dci, csz_); }

    State State() const { return state_; }
    uint8_t SlotID() const { return slot_id_; }

    void SelectForSlotAssignment();
    Ring *AllocTransferRing(DeviceContextIndex index, size_t buf_size);

    Error ControlIn(EndpointID ep_id, SetupData setup_data, void *buf, int len,
                    ClassDriver *issuer) override;
    Error ControlOut(EndpointID ep_id, SetupData setup_data, const void *buf,
                     int len, ClassDriver *issuer) override;
    Error InterruptIn(EndpointID ep_id, void *buf, int len) override;
    Error InterruptOut(EndpointID ep_id, void *buf, int len) override;

    Error OnTransferEventReceived(const TransferEventTRB &trb);

   private:
    void *ctx_;
    void *input_ctx_;
    bool csz_;

    const uint8_t slot_id_;
    DoorbellRegister *const dbreg_;

    enum State state_;
    std::array<Ring *, 31> transfer_rings_{};  // index = dci - 1

    ArrayMap<const void *, const SetupStageTRB *, 16> setup_stage_map_{};
};
}  // namespace usb::xhci
