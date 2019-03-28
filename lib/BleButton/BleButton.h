#ifndef __BLE_BUTTON_H__
#define __BLE_BUTTON_H__

#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include <events/mbed_events.h>
#include <DummyService.h>

typedef struct BleButtonConfig
{
    PinName button_pin;
    PinName ble_ind_pin;
    PinName ble_conn_pin;
    uint16_t adv_int;
} BleButtonConfig_t;

class BleButton : ble::Gap::EventHandler
{
  public:
    BleButton(BLE &ble, const BleButtonConfig_t *config, events::EventQueue *q);
    void start();

  private:
    void onBleInitComplete(BLE::InitializationCompleteCallbackContext *context);
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event);
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
    void onConnectionParametersUpdateComplete (const ble::ConnectionParametersUpdateCompleteEvent &event);
    void buttonCallback(bool pressed);
    void configureAdvertising();
    void flipLed();
  private:
    BLE &_ble;
    events::EventQueue *_q;
    InterruptIn _button;
    DigitalOut _led1;
    DigitalOut _led2;
    uint16_t _adv_int;
    bool _connected;
    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
    DummyService * _dummy_service;
};

#endif /* __BLE_BUTTON_H__ */