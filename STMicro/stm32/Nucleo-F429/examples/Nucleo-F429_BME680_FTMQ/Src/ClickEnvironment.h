/****************************************************************************************
*
*   Copyright (C) 2020 ConnectEx, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   As a special exception, if other files instantiate templates or
*   use macros or inline functions from this file, or you compile
*   this file and link it with other works to produce a work based
*   on this file, this file does not by itself cause the resulting
*   work to be covered by the GNU General Public License. However
*   the source code for this file must still be made available in
*   accordance with section (3) of the GNU General Public License.
*
*   This exception does not invalidate any other reasons why a work
*   based on this file might be covered by the GNU General Public
*   License.
*
*   For more information: info@connect-ex.com
*
*   For access to source code :
*
*       info@connect-ex.com
*           or
*       github.com/ConnectEx/BACnet-Dev-Kit
*
****************************************************************************************/


/*
 * ClickEnvironment.h
 *
 *  Created on: Oct 1, 2019
 *      Author: Roberto
 */

#ifndef CLICKENVIRONMENT_H
#define CLICKENVIRONMENT_H

#include <stdbool.h>
#include "main.h"
#include "bme680/bme680.h"

/*!
 * @file Adafruit_BME680.h
 *
 * Adafruit BME680 temperature, humidity, barometric pressure and gas sensor driver
 *
 * This is the documentation for Adafruit's BME680 driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit BME680 breakout: https://www.adafruit.com/products/3660
 *
 * These sensors use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the breakout.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#define BME680_DEFAULT_ADDRESS       (0x77 << 1)     ///< The default I2C address 0x77
#define BME680_OTHER_ADDRESS       (0x76 << 1)     ///< The default I2C address
//#define BME680_DEFAULT_SPIFREQ       (1000000)  ///< The default SPI Clock speed

#define SEALEVELPRESSURE_HPA (1013.25)

/*! Adafruit_BME680 Class for both I2C and SPI usage.
 *  Wraps the Bosch library for Arduino usage
 */
struct ClickEnvironment {

    /** Temperature (Celsius) assigned after calling performReading() or endReading() **/
    float temperature;
    /** Pressure (Pascals) assigned after calling performReading() or endReading() **/
    uint32_t pressure;
    /** Humidity (RH %) assigned after calling performReading() or endReading() **/
    float humidity;
    /** Gas resistor (ohms) assigned after calling performReading() or endReading() **/
    uint32_t gas_resistance;

//  private:
    bool _filterEnabled, _tempEnabled, _humEnabled, _presEnabled, _gasEnabled;
    uint8_t _i2caddr;
    int32_t _sensorID;
    int8_t _cs;
    unsigned long _meas_start;
    uint16_t _meas_period;

    struct bme680_dev gas_sensor;
};

extern bool enviroSetup();
extern struct ClickEnvironment read_environment();

extern void bme_init();
extern bool  bme_begin(bool initSettings/* = true*/);
extern float bme_readTemperature();
extern float bme_readPressure();
extern float bme_readHumidity();
extern uint32_t bme_readGas();
extern float bme_readAltitude(float seaLevel);

extern bool bme_setTemperatureOversampling(uint8_t os);
extern bool bme_setPressureOversampling(uint8_t os);
extern bool bme_setHumidityOversampling(uint8_t os);
extern bool bme_setIIRFilterSize(uint8_t fs);
extern bool bme_setGasHeater(uint16_t heaterTemp, uint16_t heaterTime);

// Perform a reading in blocking mode.
extern bool bme_performReading();

/*! @brief Begin an asynchronous reading.
 *  @return When the reading would be ready as absolute time in millis().
 */
extern unsigned long bme_beginReading();

/*! @brief  End an asynchronous reading.
 *          If the asynchronous reading is still in progress, block until it ends.
 *          If no asynchronous reading has started, this is equivalent to performReading().
 *  @return Whether success.
 */
extern bool bme_endReading();

/*! @brief  Get remaining time for an asynchronous reading.
 *          If the asynchronous reading is still in progress, how many millis until its completion.
 *          If the asynchronous reading is completed, 0.
 *          If no asynchronous reading has started, -1 or Adafruit_BME680::reading_not_started.
 *          Does not block.
 *  @return Remaining millis until endReading will not block if invoked.
 */
extern int bme_remainingReadingMillis();

#endif /* CLICKENVIRONMENT_H */
