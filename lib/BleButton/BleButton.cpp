#include "BleButton.h"
#include "ble_debug_logs.h"

static const char DEVICE_NAME[]= "WL-TEST";
static Ticker led_flipper;
static const uint8_t MSD[] = {(uint8_t)'W',(uint8_t)'L', 
(uint8_t)(NRF_FICR->DEVICEID[1] >> 24),(uint8_t)(NRF_FICR->DEVICEID[1] >> 16),
(uint8_t)(NRF_FICR->DEVICEID[1] >> 8),(uint8_t)(NRF_FICR->DEVICEID[1] && 0xff),
(uint8_t)(NRF_FICR->DEVICEID[0] >> 24),(uint8_t)(NRF_FICR->DEVICEID[0] >> 16), 
(uint8_t)(NRF_FICR->DEVICEID[0] >> 8),(uint8_t)(NRF_FICR->DEVICEID[0] & 0xff)};
static const Gap::ConnectionParams_t PREFFERED_CONNECTION_PARAMS = {16, 24, 0, 400}; 

static const char * debugDisconnectionReason(const ble::disconnection_reason_t &reason)
{
    switch (reason.value())
    {
    case ble::disconnection_reason_t::AUTHENTICATION_FAILURE:
        return "AUTHENTICATION_FAILURE";
    case ble::disconnection_reason_t::CONNECTION_TIMEOUT:
        return "CONNECTION_TIMEOUT";
    case ble::disconnection_reason_t::REMOTE_USER_TERMINATED_CONNECTION:
        return "REMOTE_USER_TERMINATED_CONNECTION";
    case ble::disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES:
        return "REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES";
    case ble::disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF:
        return "REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF";
    case ble::disconnection_reason_t::LOCAL_HOST_TERMINATED_CONNECTION:
        return "LOCAL_HOST_TERMINATED_CONNECTION";
    case ble::disconnection_reason_t::UNACCEPTABLE_CONNECTION_PARAMETERS:
        return "UNACCEPTABLE_CONNECTION_PARAMETERS";
    default:
        return "Illegal error code!";
    }
}

static void printAdvData(const char * string, mbed::Span<const uint8_t> data)
{
    BLE_LOG("%s DATA SIZE: %lu\r\n",string,data.size());
    BLE_LOG("%s DATA: 0x", string);
    for(int i=0; i<data.size(); i++)
    {
        BLE_LOG("%02X",data[i]);
    }
    BLE_LOG("\r\n");
}

static void printMacAddress()
{
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);
    BLE_LOG("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
        BLE_LOG("%02x:", address[i]);
    }
    BLE_LOG("%02x\r\n", address[0]);
}

void BleButton::configureAdvertising()
{
    // configure advertising parameters
    ble::AdvertisingParameters adv_params(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(_adv_int))
        );
    adv_params.setTxPower(0); // Accepted values are -40, -20, -16, -12, -8, -4, 0, 4 (dbm)
    
    ble_error_t err = _ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_params);
    if (err != BLE_ERROR_NONE) {
        BLE_LOG("Setting adv params failure: %s\r\n", BLE::errorToString(err));
        return;
    }
    
    // build advertising data
    _adv_data_builder.setFlags();
    _adv_data_builder.setLocalServiceList(mbed::make_Span(&DummyServiceUUID,1),false);
    _adv_data_builder.setAppearance(ble::adv_data_appearance_t::GENERIC_TAG);
    
    printAdvData("adv",_adv_data_builder.getAdvertisingData());
    
    // setup advertising
    err = _ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,_adv_data_builder.getAdvertisingData());
    if (err != BLE_ERROR_NONE) {
        BLE_LOG("Setting adv payload failure: %s\r\n", BLE::errorToString(err));
        return;
    }

    // build scan response data

    _adv_data_builder.clear();
    _adv_data_builder.setManufacturerSpecificData(mbed::make_Span(MSD,10));
    _adv_data_builder.setName(DEVICE_NAME);

    printAdvData("adv scan response",_adv_data_builder.getAdvertisingData());

    // Generic Access Service setup
    _ble.gap().setDeviceName((const uint8_t*)DEVICE_NAME);
    _ble.gap().setAppearance(GapAdvertisingData::GENERIC_TAG);
    _ble.gap().setPreferredConnectionParams(&PREFFERED_CONNECTION_PARAMS);
    // LEGACY
    // _ble.gap().setTxPower(0); // Accepted values are -40, -20, -16, -12, -8, -4, 0, 4 (dbm)

    // setup scan response
    err = _ble.gap().setAdvertisingScanResponse(ble::LEGACY_ADVERTISING_HANDLE,_adv_data_builder.getAdvertisingData());
    if (err != BLE_ERROR_NONE)
    {
        BLE_LOG("Setting adv scan response failure: %s\r\n", BLE::errorToString(err));
        return;
    }
    
    // start advertising
    err = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (err != BLE_ERROR_NONE)
    {
        BLE_LOG("Failure to start advertising: %s\r\n", BLE::errorToString(err));
        return;
    }
}

void BleButton::onBleInitComplete(BLE::InitializationCompleteCallbackContext *context)
{
    ble_error_t &err = context->error;
    
    if (err != BLE_ERROR_NONE) {
        BLE_LOG("Initialisation failed: %s\r\n", BLE::errorToString(err));
        return;
    }
    
    printMacAddress();

    _dummy_service = new DummyService(_ble,_q,0);
    
    _button.mode(PullUp);
    _button.fall(_q->event(Callback<void(bool)>(this,&BleButton::buttonCallback),true));
    _button.rise(_q->event(Callback<void(bool)>(this,&BleButton::buttonCallback),false));
    led_flipper.attach(Callback<void()>(this,&BleButton::flipLed),1.0);
    configureAdvertising();

    BLE_LOG("BleButton initialisation completed!\r\n");
}

void BleButton::start()
{
    _ble.gap().setEventHandler(this);
    _ble.init(this, &BleButton::onBleInitComplete);
    BLE_LOG("Starting BleButton device...\r\n");
    _q->dispatch_forever();
}

BleButton::BleButton(BLE &ble, const BleButtonConfig_t * config, events::EventQueue *q)
: _ble(ble)
, _q(q)
, _button(config->button_pin)
, _led1(config->ble_ind_pin,1)
, _led2(config->ble_conn_pin,1)
, _adv_int(config->adv_int)
, _connected(false)
, _adv_data_builder(_adv_buffer)
{}

void BleButton::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    _connected = false;
    BLE_LOG("Device disconnected: %s\r\n",debugDisconnectionReason(event.getReason()));
    led_flipper.detach();
    led_flipper.attach(Callback<void()>(this,&BleButton::flipLed),1.0);

}

void BleButton::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getStatus() == BLE_ERROR_NONE) {
        _connected = true;
        BLE_LOG("Device connected.\r\n");
        led_flipper.detach();
        _led2.write(0);
        ble_error_t err = _ble.gap().updateConnectionParameters(
                          event.getConnectionHandle(),
                          ble::conn_interval_t(PREFFERED_CONNECTION_PARAMS.minConnectionInterval),
                          ble::conn_interval_t(PREFFERED_CONNECTION_PARAMS.maxConnectionInterval),
                          ble::slave_latency_t(PREFFERED_CONNECTION_PARAMS.slaveLatency),
                          ble::supervision_timeout_t(PREFFERED_CONNECTION_PARAMS.connectionSupervisionTimeout));
        if (err != BLE_ERROR_NONE)
        {
            BLE_LOG("Failure to update connection params: %s\r\n", BLE::errorToString(err));
        }
    }
}

void BleButton::buttonCallback(bool pressed)
{
    if(pressed)
    {
        _led1.write(0);
        BLE_LOG("Button pressed\r\n");
        _dummy_service->updateDummy2(1);
    }
    else
    {
        _led1.write(1);
        BLE_LOG("Button released\r\n");
        _dummy_service->updateDummy2(0);
    }
}

void BleButton::flipLed()
{
    _led2 = !_led2;
}

void BleButton::onConnectionParametersUpdateComplete(const ble::ConnectionParametersUpdateCompleteEvent &event)
{
    BLE_LOG("Connection Parameters Update completed!\r\n");
}