/*
===============================================================================
 Name        : BLE Battery Service
 Author      : uCXpresso
 Version     : v1.0.0
 Copyright   : www.ucxpresso.net
 Description : Battery Service Demo
===============================================================================
 	 	 	 	 	 	 	 	 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2014/10/18 v1.0.0	First Edition.									LEO
 ===============================================================================
 */

#include <uCXpresso.h>

#ifdef DEBUG
#include <debug.h>
#include <class/serial.h>
#define DBG dbg_printf
#else
#define DBG(...)
#endif

// TODO: insert other include files here
#include <class/ble/ble_device.h>
#include <class/ble/ble_service.h>
#include <class/pin.h>

#include <class/adc.h>
#include <class/ble/ble_service_bat.h>
#include <class/ble/ble_conn_params.h>
#include <class/timeout.h>

// TODO: insert other definitions and declarations here
#define DEVICE_NAME                         "nano51822"            	/**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                   "uCXpresso.NRF"        	/**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                    500                		/**< The advertising interval (in ms). */
#define APP_COMPANY_IDENTIFIER				0x0059          		/**< Company identifier for Nordic Semi. as per www.bluetooth.org. */

//
// Board LED define
//
#define BOARD_PCA10001
//#define BOARD_LILYPAD
#include <config/board.h>

//
// Main Routine
//
int main(void) {
#ifdef DEBUG
	CSerial ser;		// declare a UART object
	ser.enable();
	CDebug dbg(ser);	// Debug stream use the UART object
	dbg.start();
#endif
	//
	// SoftDevice
	//
	bleDevice ble;	// use internal 32.768KHz for SoftDevice
	ble.enable();	// enable BLE SoftDevice stack

	// GAP
	ble.m_gap.settings(DEVICE_NAME);	// set Device Name on GAP
	ble.m_gap.tx_power(BLE_TX_0dBm);

	//
	// Add BLE Service
	//
	bleServiceBattery bat(ble);

	//
	// Connection Parameters Update negotiation
	//
	bleConnParams conn(ble);

	//
	// BLE Advertising
	//
	ble.m_advertising.interval(APP_ADV_INTERVAL);			// set advertising interval
	ble.m_advertising.add_uuid_to_complete_list(bat);		// add bat object to the uuid list of advertising
	ble.m_advertising.update();								// update advertisement data

	// Start advertising
	ble.m_advertising.start();

	//
	// Analog
	//
	CAdc::init();
	CAdc::source(VDD_1_3);	// to detect the VDD voltage
	CAdc::enable();

	//
	// LED
	//
	CPin led0(LED_PIN_0);
	CPin led1(LED_PIN_1);
	led0.output();
	led1.output();

	CTimeout tmLED, tmBAT;

	uint16_t value;
	float 	 voltage, percentage;

	//
    // Enter main loop.
	//
    while(1) {
    	//
    	// BLE Battery Service
    	//
    	if ( bat.isAvailable() ) {
    		if ( tmBAT.isExpired(1000) ) {
    			tmBAT.reset();
    			if ( CAdc::read(value) ) {

    				voltage = (value / 1024.0f) * 3.6f;
    				percentage = (value / 1024.0f) * 100;
    				DBG("V=%0.2f P=%0.2f\n", voltage, percentage);

    				bat.send(percentage);
    				led1.toggle();
    			}
    		}
    	} else led1 = LED_OFF;

    	// Negotiate the "connection parameters update"
    	conn.negotiate();

    	//
    	// blink LED
    	//
    	if ( tmLED.isExpired(500) ) {
    		tmLED.reset();
    		led0.toggle();
    	}
    }
}
