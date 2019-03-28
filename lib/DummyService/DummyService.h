#ifndef __DUMMY_SERVICE_H__
#define __DUMMY_SERVICE_H__

#include <mbed.h>
#include <ble/BLE.h>
#include <UUID.h>
#include <mbed_events.h>

extern const UUID DummyServiceUUID;
extern const UUID DummyCharacteristic1UUID;
extern const UUID DummyCharacteristic2UUID;

class DummyService
{
    typedef DummyService Self;
    
    union Float {
        float m_float;
        uint8_t m_bytes[sizeof(float)];
    };

public:

    DummyService(BLE &ble, events::EventQueue *q, uint8_t default_init_value)
    :_ble(ble)
    ,_q(q)
    ,_dummy_char1(DummyCharacteristic1UUID, (uint32_t)default_init_value)
    ,_dummy_char2(DummyCharacteristic2UUID, &default_init_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    ,_dummy_service(
    /* uuid */ DummyServiceUUID,
    /* characteristics */ _dummy_chars,
    /* numCharacteristics */ sizeof(_dummy_chars) /
                             sizeof(_dummy_chars[0]))
    ,_server(_ble.gattServer())
    {
        // update internal pointers (value, descriptors and characteristics array)
        _dummy_chars[0] = &_dummy_char1;
        _dummy_chars[1] = &_dummy_char2;
        _server.addService(_dummy_service);
        _server.onDataWritten(as_cb(&Self::dataWrittenCallback));
        if(_server.isOnDataReadAvailable())
            _server.onDataRead(as_cb(&Self::dataReadCallback));
        _server.onUpdatesEnabled(as_cb(&Self::updatesEnabledCallback));
        _server.onUpdatesDisabled(as_cb(&Self::updatesDisabledCallback));
    }

    void updateDummy2(uint8_t value)
    {
        bool enabled;
        ble_error_t err = _server.areUpdatesEnabled(_dummy_char2,&enabled);
        if(err == BLE_ERROR_NONE && enabled)
            _server.write(_dummy_char2.getValueHandle(),&value,1);
        
    }

private:
     /**
     * Helper that construct an event handler from a member function of this
     * instance.
     */
    template<typename Arg>
    FunctionPointerWithContext<Arg> as_cb(void (Self::*member)(Arg))
    {
        return makeFunctionPointer(this, member);
    }

    void dataWrittenCallback(const GattWriteCallbackParams *params);
    void dataReadCallback(const GattReadCallbackParams *context);
    void updatesEnabledCallback(GattAttribute::Handle_t handle);
    void updatesDisabledCallback(GattAttribute::Handle_t handle);
    /**
     * Write Without Response Characteristic declaration helper.
     *
     * @tparam T type of data held by the characteristic.
     */
    template<typename T>
    class WriteWithoutResponseCharacteristic : public GattCharacteristic {
    public:
        /**
         * Construct a characteristic that can be written.
         *
         * @param[in] uuid The UUID of the characteristic.
         * @param[in] initial_value Initial value contained by the characteristic.
         */
        WriteWithoutResponseCharacteristic<T>(const UUID & uuid, const T initial_value) :
            GattCharacteristic(
                /* UUID */ uuid,
                /* Initial value */ reinterpret_cast<uint8_t *>(&_value),
                /* Value size */ sizeof(T),
                /* Value capacity */ sizeof(T),
                /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE,
                /* Descriptors */ NULL,
                /* Num descriptors */ 0,
                /* variable len */ false
            ),
            _value(initial_value) {
        }

        /**
         * Get the value of this characteristic.
         *
         * @param[in] server GattServer instance that contain the characteristic
         * value.
         * @param[in] dst Variable that will receive the characteristic value.
         *
         * @return BLE_ERROR_NONE in case of success or an appropriate error code.
         */
        ble_error_t get(GattServer &server, T& dst) const
        {
            uint16_t value_length = sizeof(dst);
            return server.read(getValueHandle(), (uint8_t *)&dst, &value_length);
        }

        /**
         * Assign a new value to this characteristic.
         *
         * @param[in] server GattServer instance that will receive the new value.
         * @param[in] value The new value to set.
         * @param[in] local_only Flag that determine if the change should be kept
         * locally or forwarded to subscribed clients.
         */
        ble_error_t set(
            GattServer &server, const T &value, bool local_only = false
        ) const {
            return server.write(getValueHandle(), (uint8_t *)&value, sizeof(value), local_only);
        }

        /**
         * Get initial value of this characteristic
         */
        T getInitialValue()
        {
            return _value;
        }

    private:
        T _value;
    };

    // characteristics
    // float on 32bit system - 4B
    BLE & _ble;
    events::EventQueue * _q;
    WriteWithoutResponseCharacteristic<uint32_t> _dummy_char1;
    ReadOnlyGattCharacteristic<uint8_t> _dummy_char2;
    GattCharacteristic* _dummy_chars[2];
    GattService _dummy_service;
    GattServer & _server;
};

#endif /* __DUMMY_SERVICE_H__ */