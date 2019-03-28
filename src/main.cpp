#include <BleButton.h>

events::EventQueue * ble_event_queue_ptr;

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
    ble_event_queue_ptr->call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    // Initialise ble EventQueue
    ble_event_queue_ptr = mbed_event_queue();

    // Create BleButton config params
    BleButtonConfig_t params = {
        .button_pin = BUTTON,
        .ble_ind_pin = NC,
        .ble_conn_pin = NC,
        .adv_int = 1000
    };

    // Acquire BLE instance and initialise events' process function
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    // Create ButtonBeacon demo object
    BleButton demo(ble, &params, ble_event_queue_ptr);

    // Start demo
    demo.start();
}