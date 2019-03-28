#include "DummyService.h"
#include <ble_debug_logs.h>
#include <inttypes.h> 
const UUID DummyServiceUUID("9f8c5e4d-6cf8-4fdc-aa64-3a04a6449432");
const UUID DummyCharacteristic1UUID("9f8c5e4e-6cf8-4fdc-aa64-3a04a6449432");
const UUID DummyCharacteristic2UUID("9f8c5e4f-6cf8-4fdc-aa64-3a04a6449432");

void DummyService::dataWrittenCallback(const GattWriteCallbackParams *params)
{
    // Multi-octet fields within the GATT Profile 
    // shall be sent least significant octet first (Little Endian).
    if((params->handle == _dummy_char1.getValueHandle()) && params->len == 4)
    {
        BLE_LOG("Characterisic dummy1 written!\r\n");
        BLE_LOG("%" PRIi8 ":%" PRIi8 ":%" PRIi8 ":%" PRIi8 "\r\n"
                ,(int8_t)params->data[0],(int8_t)params->data[1]
                ,(int8_t)params->data[2],(int8_t)params->data[3]);
    }
}

void DummyService::dataReadCallback(const GattReadCallbackParams *context)
{
    if(context->handle == _dummy_char2.getValueHandle())
    {
        BLE_LOG("Characteristic dummy2 was read!\r\n");
    }
}

void DummyService::updatesEnabledCallback(GattAttribute::Handle_t handle)
{
    // if(handle == _dummy_char2.getValueHandle())
        BLE_LOG("Characteristic dummy2 notification enabled\r\n");
}

void DummyService::updatesDisabledCallback(GattAttribute::Handle_t handle)
{
    // if(handle == _dummy_service.)
        BLE_LOG("Characteristic dummy2 notification disabled\r\n");

}