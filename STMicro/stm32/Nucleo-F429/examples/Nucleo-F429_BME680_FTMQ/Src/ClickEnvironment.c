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
 * ClickEnvironment.c
 *
 *  Created on: Oct 1, 2019
 *      Author: Roberto
 */

#include <main.h>
#include <math.h>
#include <limits.h>

#include "debug.h"
#include "ClickEnvironment.h"


// #define for UART output
#undef CLICKENVIRONMENT_SERIAL_DEBUG

extern I2C_HandleTypeDef hi2c1;

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
extern UART_HandleTypeDef huart3;
#endif

extern uint32_t millis();

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
	char buf[256];
#endif

struct ClickEnvironment data;

bool enviroSetup(){
	  bme_init();

	  if (!bme_begin(true)) {
		return false;
	  }

	  // Set up oversampling and filter initialization
	  bme_setTemperatureOversampling(BME680_OS_8X);
	  bme_setHumidityOversampling(BME680_OS_2X);
	  bme_setPressureOversampling(BME680_OS_4X);
	  bme_setIIRFilterSize(BME680_FILTER_SIZE_3);
	  bme_setGasHeater(320, 150); // 320*C for 150 ms
	  return true;
}

struct ClickEnvironment read_environment() {
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme_beginReading();
  if (endTime == 0) {
	SERIAL_DEBUG("Failed to begin reading :(\n\r");
	return data;
  }
  // SERIAL_DEBUG_SPRINTF_2("Reading started at %lu  and will finish at %lu\r\n", millis(), endTime);
  // SERIAL_DEBUG("You can do other work during BME680 measurement.\n\r");
  HAL_Delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: data.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme_endReading()) {
	SERIAL_DEBUG("Failed to complete reading :(\n\r");
  }

/*
  SERIAL_DEBUG_SPRINTF_1("Reading completed at %lu\n\r", millis());
  SERIAL_DEBUG_SPRINTF_1("Temperature = %d *C\n\r", (int)data.temperature);
  SERIAL_DEBUG_SPRINTF_1("Pressure = %d hPa\n\r", (int)(data.pressure / 100.0));
  SERIAL_DEBUG_SPRINTF_1("Humidity = %d %%\n\r", (int)data.humidity);
  SERIAL_DEBUG_SPRINTF_1("Gas = %d KOhms\n\r", (int)(data.gas_resistance / 1000.0));
  SERIAL_DEBUG_SPRINTF_1("Approx. Altitude = %d m\n\r", (int)bme_readAltitude(SEALEVELPRESSURE_HPA));
*/
  return data;
}



/********************/

/*!
 * @file Adafruit_BME680.cpp
 *
 * @mainpage Adafruit BME680 temperature, humidity, barometric pressure and gas sensor driver
 *
 * @section intro_sec Introduction
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
 * @section author Author
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

/** SPI object **/
//SPIClass *_spi;

/** These SPI pins must be global in order to work with underlying library **/
//int8_t _BME680_SoftwareSPI_MOSI; ///< Global SPI MOSI pin
//int8_t _BME680_SoftwareSPI_MISO; ///< Global SPI MISO pin
//int8_t _BME680_SoftwareSPI_SCK;  ///< Globak SPI Clock pin

/** Our hardware interface functions **/
static int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
static int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
static void delay_msec(uint32_t ms);

// PUBLIC FUNCTIONS

void bme_init() {
	data._cs = -1;
	data._meas_start = 0;
	data._meas_period = 0;

	data._filterEnabled = false;
	data._tempEnabled = false;
	data._humEnabled = false;
	data._presEnabled = false;
	data._gasEnabled = false;
}

/*!
 *  @brief  Initializes the sensor
 *          Hardware ss initialized, verifies it is in the I2C or SPI bus, then reads
 *          calibration data in preparation for sensor reads.
 *  @param  addr
 *          Optional parameter for the I2C address of BME680. Default is 0x77
 *  @param  initSettings
 *          Optional parameter for initializing the sensor settings.
 *          Default is true.
 *  @return True on sensor initialization success. False on failure.
 */
bool bme_begin(bool initSettings) {
	data._i2caddr = BME680_DEFAULT_ADDRESS;

	data.gas_sensor.dev_id = BME680_DEFAULT_ADDRESS;
	data.gas_sensor.intf = BME680_I2C_INTF;
	data.gas_sensor.read = &i2c_read;
	data.gas_sensor.write = &i2c_write;

	data.gas_sensor.delay_ms = &delay_msec;

    int8_t rslt = bme680_init(&data.gas_sensor);

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
    SERIAL_DEBUG_SPRINTF_1("Result: %d\n\r", rslt);
#endif

  if (rslt != BME680_OK)
    return false;

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
  SERIAL_DEBUG_SPRINTF_1("T1 = %d\n\r", data.gas_sensor.calib.par_t1);
  SERIAL_DEBUG_SPRINTF_1("T2 = %d\n\r", data.gas_sensor.calib.par_t2);
  SERIAL_DEBUG_SPRINTF_1("T3 = %d\n\r", data.gas_sensor.calib.par_t3);
  SERIAL_DEBUG_SPRINTF_1("P1 = %d\n\r", data.gas_sensor.calib.par_p1);
  SERIAL_DEBUG_SPRINTF_1("P2 = %d\n\r", data.gas_sensor.calib.par_p2);
  SERIAL_DEBUG_SPRINTF_1("P3 = %d\n\r", data.gas_sensor.calib.par_p3);
  SERIAL_DEBUG_SPRINTF_1("P4 = %d\n\r", data.gas_sensor.calib.par_p4);
  SERIAL_DEBUG_SPRINTF_1("P5 = %d\n\r", data.gas_sensor.calib.par_p5);
  SERIAL_DEBUG_SPRINTF_1("P6 = %d\n\r", data.gas_sensor.calib.par_p6);
  SERIAL_DEBUG_SPRINTF_1("P7 = %d\n\r", data.gas_sensor.calib.par_p7);
  SERIAL_DEBUG_SPRINTF_1("P8 = %d\n\r", data.gas_sensor.calib.par_p8);
  SERIAL_DEBUG_SPRINTF_1("P9 = %d\n\r", data.gas_sensor.calib.par_p9);
  SERIAL_DEBUG_SPRINTF_1("P10 = %d\n\r", data.gas_sensor.calib.par_p10);
  SERIAL_DEBUG_SPRINTF_1("H1 = %d\n\r", data.gas_sensor.calib.par_h1);
  SERIAL_DEBUG_SPRINTF_1("H2 = %d\n\r", data.gas_sensor.calib.par_h2);
  SERIAL_DEBUG_SPRINTF_1("H3 = %d\n\r", data.gas_sensor.calib.par_h3);
  SERIAL_DEBUG_SPRINTF_1("H4 = %d\n\r", data.gas_sensor.calib.par_h4);
  SERIAL_DEBUG_SPRINTF_1("H5 = %d\n\r", data.gas_sensor.calib.par_h5);
  SERIAL_DEBUG_SPRINTF_1("H6 = %d\n\r", data.gas_sensor.calib.par_h6);
  SERIAL_DEBUG_SPRINTF_1("H7 = %d\n\r", data.gas_sensor.calib.par_h7);
  SERIAL_DEBUG_SPRINTF_1("G1 = %d\n\r", data.gas_sensor.calib.par_gh1);
  SERIAL_DEBUG_SPRINTF_1("G2 = %d\n\r", data.gas_sensor.calib.par_gh2);
  SERIAL_DEBUG_SPRINTF_1("G3 = %d\n\r", data.gas_sensor.calib.par_gh3);
  SERIAL_DEBUG_SPRINTF_1("G1 = %d\n\r", data.gas_sensor.calib.par_gh1);
  SERIAL_DEBUG_SPRINTF_1("G2 = %d\n\r", data.gas_sensor.calib.par_gh2);
  SERIAL_DEBUG_SPRINTF_1("G3 = %d\n\r", data.gas_sensor.calib.par_gh3);
  SERIAL_DEBUG_SPRINTF_1("Heat Range = %d\n\r", data.gas_sensor.calib.res_heat_range);
  SERIAL_DEBUG_SPRINTF_1("Heat Val = %d\n\r", data.gas_sensor.calib.res_heat_val);
  SERIAL_DEBUG_SPRINTF_1("SW Error = %d\n\r", data.gas_sensor.calib.range_sw_err);
#endif

  if (initSettings) {
    bme_setTemperatureOversampling(BME680_OS_8X);
    bme_setHumidityOversampling(BME680_OS_2X);
    bme_setPressureOversampling(BME680_OS_4X);
    bme_setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme_setGasHeater(320, 150); // 320*C for 150 ms
    
  } else {
	  bme_setGasHeater(0, 0);
  }
  // don't do anything till we request a reading
  data.gas_sensor.power_mode = BME680_FORCED_MODE;

  return true;
}

/*!
 *  @brief  Performs a reading and returns the ambient temperature.
 *  @return Temperature in degrees Centigrade
 */
float bme_readTemperature(void) {
  bme_performReading();
  return data.temperature;
}

/*!
 *  @brief Performs a reading and returns the barometric pressure.
 *  @return Barometic pressure in Pascals
 */
float bme_readPressure(void) {
  bme_performReading();
  return data.pressure;
}


/*!
 *  @brief  Performs a reading and returns the relative humidity.
 *  @return Relative humidity as floating point
 */
float bme_readHumidity(void) {
  bme_performReading();
  return data.humidity;
}

/*!
 *  @brief Calculates the resistance of the MOX gas sensor.
 *  @return Resistance in Ohms
 */
uint32_t bme_readGas(void) {
  bme_performReading();
  return data.gas_resistance;
}

/*!
 *  @brief  Calculates the altitude (in meters).
 *          Reads the current atmostpheric pressure (in hPa) from the sensor and calculates
 *          via the provided sea-level pressure (in hPa).
 *  @param  seaLevel
 *          Sea-level pressure in hPa
 *  @return Altitude in meters
 */
float bme_readAltitude(float seaLevel){
    // Equation taken from BMP180 datasheet (page 16):
    //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

    // Note that using the equation from wikipedia can give bad results
    // at high altitude. See this thread for more information:
    //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

    float atmospheric = bme_readPressure() / 100.0F;
    return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/*!
 *  @brief  Performs a full reading of all 4 sensors in the BME680.
 *          Assigns the internal Adafruit_BME680#temperature, Adafruit_BME680#pressure, Adafruit_BME680#humidity
 *          and Adafruit_BME680#gas_resistance member variables
 *  @return True on success, False on failure
 */
bool bme_performReading(void) {
  return bme_endReading();
}

unsigned long bme_beginReading(void) {
  if (data._meas_start != 0) {
    /* A measurement is already in progress */
    return data._meas_start + data._meas_period;
  }

  uint8_t set_required_settings = 0;
  int8_t rslt;

  /* Select the power mode */
  /* Must be set before writing the sensor configuration */
  data.gas_sensor.power_mode = BME680_FORCED_MODE;

  /* Set the required sensor settings needed */
  if (data._tempEnabled) {
    set_required_settings |= BME680_OST_SEL;
  }
  if (data._humEnabled) {
    set_required_settings |= BME680_OSH_SEL;
  }
  if (data._presEnabled) {
    set_required_settings |= BME680_OSP_SEL;
  }
  if (data._filterEnabled) {
    set_required_settings |= BME680_FILTER_SEL;
  }
  if (data._gasEnabled) {
    set_required_settings |= BME680_GAS_SENSOR_SEL;
  }

  /* Set the desired sensor configuration */
#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
  SERIAL_DEBUG("Setting sensor settings\n\r");
#endif
  rslt = bme680_set_sensor_settings(set_required_settings, &data.gas_sensor);
  if (rslt != BME680_OK)
    return 0;

  /* Set the power mode */
#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
  SERIAL_DEBUG("Setting power mode\n\r");
#endif
  rslt = bme680_set_sensor_mode(&data.gas_sensor);
  if (rslt != BME680_OK)
    return 0;

  /* Get the total measurement duration so as to sleep or wait till the
   * measurement is complete */
  uint16_t meas_period;
  bme680_get_profile_dur(&meas_period, &data.gas_sensor);
  
  data._meas_start = millis();
  data._meas_period = meas_period;
  
  return data._meas_start + data._meas_period;
}

bool bme_endReading(void) {
  unsigned long meas_end = bme_beginReading();
  if (meas_end == 0) {
    return false;
  }

  int remaining_millis = bme_remainingReadingMillis();
  if (remaining_millis > 0) {
#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
    SERIAL_DEBUG_SPRINTF_1("Waiting (ms) %d\n\r", remaining_millis);
#endif
    HAL_Delay((uint32_t)(remaining_millis * 2)); /* Delay till the measurement is ready */
  }
  data._meas_start = 0; /* Allow new measurement to begin */
  data._meas_period = 0;

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
  SERIAL_DEBUG_SPRINTF_1("t_fine = %lu\n\r", data.gas_sensor.calib.t_fine);
#endif

  struct bme680_field_data rawData;

  //DEBUG_SPRINTF_1ln("Getting sensor data");
  int8_t rslt = bme680_get_sensor_data(&rawData, &data.gas_sensor);
  if (rslt != BME680_OK)
    return false;

  if (data._tempEnabled) {
    //DEBUG_SPRINTF_1("Temp: "); SERIAL_DEBUG_SPRINTF_1ln(data.temperature / 100.0, 2);
	  data.temperature = rawData.temperature / 100.0;
  } else {
	  data.temperature = NAN;
  }

  if (data._humEnabled) {
    //DEBUG_SPRINTF_1("Hum:  "); SERIAL_DEBUG_SPRINTF_1ln(data.humidity / 1000.0, 2);
	  data.humidity = rawData.humidity / 1000.0;
  } else {
	  data.humidity = NAN;
  }

  if (data._presEnabled) {
    //DEBUG_SPRINTF_1("Pres: "); SERIAL_DEBUG_SPRINTF_1ln(data.pressure, 2);
	  data.pressure = rawData.pressure;
  } else {
	  data.pressure = 0;
  }

  /* Avoid using measurements from an unstable heating setup */
  if (data._gasEnabled) {
    if (rawData.status & BME680_HEAT_STAB_MSK) {
        //SERIAL_DEBUG_SPRINTF_1("Gas resistance: %d\n\r"data.gas_resistance);
    	data.gas_resistance = rawData.gas_resistance;
    } else {
    	data.gas_resistance = 0;
        //SERIAL_DEBUG_("Gas reading unstable!\n\r");
    }
  } else {
    data.gas_resistance = ULONG_MAX; // NAN ?
  }

  return true;
}

/** Value returned by remainingReadingMillis indicating no asynchronous reading has been initiated by beginReading. **/
#define READING_NOT_STARTED -1
/** Value returned by remainingReadingMillis indicating asynchronous reading is complete and calling endReading will not block. **/
#define READING_COMPLETE 0


int bme_remainingReadingMillis(void) {
    if (data._meas_start != 0) {
        /* A measurement is already in progress */
        int remaining_time = (int)data._meas_period - (millis() - data._meas_start);
        return remaining_time < 0 ? READING_COMPLETE : remaining_time;
    }
    return READING_NOT_STARTED;
}

/*!
 *  @brief  Enable and configure gas reading + heater
 *  @param  heaterTemp
 *          Desired temperature in degrees Centigrade
 *  @param  heaterTime
 *          Time to keep heater on in milliseconds
 *  @return True on success, False on failure
 */
bool bme_setGasHeater(uint16_t heaterTemp, uint16_t heaterTime) {
  data.gas_sensor.gas_sett.heatr_temp = heaterTemp;
  data.gas_sensor.gas_sett.heatr_dur = heaterTime;

  if ( (heaterTemp == 0) || (heaterTime == 0) ) {
    // disabled!
	  data.gas_sensor.gas_sett.heatr_ctrl = BME680_DISABLE_HEATER;
	  data.gas_sensor.gas_sett.run_gas = BME680_DISABLE_GAS_MEAS;
	  data._gasEnabled = false;
  } else {
	  data.gas_sensor.gas_sett.heatr_ctrl = BME680_ENABLE_HEATER;
	  data.gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
      data._gasEnabled = true;
  }
  return true;
}


/*!
 *  @brief  Setter for Temperature oversampling
 *  @param  oversample
 *          Oversampling setting, can be BME680_OS_NONE (turn off Temperature reading),
 *          BME680_OS_1X, BME680_OS_2X, BME680_OS_4X, BME680_OS_8X or BME680_OS_16X
 *  @return True on success, False on failure
 */

bool bme_setTemperatureOversampling(uint8_t oversample) {
  if (oversample > BME680_OS_16X) return false;

  data.gas_sensor.tph_sett.os_temp = oversample;

  if (oversample == BME680_OS_NONE)
	  data._tempEnabled = false;
  else
	  data._tempEnabled = true;

  return true;
}


/*!
 *  @brief  Setter for Humidity oversampling
 *  @param  oversample
 *          Oversampling setting, can be BME680_OS_NONE (turn off Humidity reading),
 *          BME680_OS_1X, BME680_OS_2X, BME680_OS_4X, BME680_OS_8X or BME680_OS_16X
 *  @return True on success, False on failure
 */
bool bme_setHumidityOversampling(uint8_t oversample) {
  if (oversample > BME680_OS_16X) return false;

  data.gas_sensor.tph_sett.os_hum = oversample;

  if (oversample == BME680_OS_NONE)
	  data._humEnabled = false;
  else
	  data._humEnabled = true;

  return true;
}


/*!
 *  @brief  Setter for Pressure oversampling
 *  @param  oversample
 *          Oversampling setting, can be BME680_OS_NONE (turn off Pressure reading),
 *          BME680_OS_1X, BME680_OS_2X, BME680_OS_4X, BME680_OS_8X or BME680_OS_16X
 *  @return True on success, False on failure
 */
bool bme_setPressureOversampling(uint8_t oversample) {
  if (oversample > BME680_OS_16X) return false;

  data.gas_sensor.tph_sett.os_pres = oversample;

  if (oversample == BME680_OS_NONE) {
	  data._presEnabled = false;
  } else {
	  data._presEnabled = true;
  }

  return true;
}

/*!
 *  @brief  Setter for IIR filter.
 *  @param  filtersize
 *          Size of the filter (in samples).
 *          Can be BME680_FILTER_SIZE_0 (no filtering), BME680_FILTER_SIZE_1, BME680_FILTER_SIZE_3, BME680_FILTER_SIZE_7, BME680_FILTER_SIZE_15, BME680_FILTER_SIZE_31, BME680_FILTER_SIZE_63, BME680_FILTER_SIZE_127
 *  @return True on success, False on failure
 */
bool bme_setIIRFilterSize(uint8_t filtersize) {
  if (filtersize > BME680_FILTER_SIZE_127) return false;

  data.gas_sensor.tph_sett.filter = filtersize;

  if (filtersize == BME680_FILTER_SIZE_0) {
	  data._filterEnabled = false;
  } else {
	  data._filterEnabled = true;
  }

  return true;
}

/*
 * Functions required by the BOSCH bmee680 official driver
 */

int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Master_Transmit(&hi2c1, dev_id, &reg_addr, 1, HAL_MAX_DELAY);

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
	if (ret != HAL_OK){
	  sprintf(buf, "I2C Read Error %d, dev_id: %d, reg_addr: %d\r\n", ret, dev_id, reg_addr);
	  SERIAL_DEBUG(buf);
	  return ret;
	}
	else{
	  //buf = "I2C Read Sent\r\n";
	}
#endif

	ret = HAL_I2C_Master_Receive(&hi2c1, dev_id, reg_data, len, HAL_MAX_DELAY);

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
	if (ret != HAL_OK){
	  sprintf(buf, "I2C Read Error %d, dev_id: %d, reg_addr: %d\r\n", ret, dev_id, reg_addr);
	  SERIAL_DEBUG(buf);
	  return ret;
	}
	else{
	  //buf = "I2C Read Sent\r\n";
	}
#endif


  return BME680_OK;
}

int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
	HAL_StatusTypeDef ret;

	uint8_t newBuf[len +1];
	newBuf[0] = reg_addr;
	memcpy(newBuf+1, reg_data, len);

	ret = HAL_I2C_Master_Transmit(&hi2c1, dev_id, newBuf, len+1, HAL_MAX_DELAY);

#ifdef CLICKENVIRONMENT_SERIAL_DEBUG
	if (ret != HAL_OK){
		  sprintf(buf, "I2C Write Error %d, dev_id: %d, reg_addr: %d\r\n", ret, dev_id, reg_addr);
		  SERIAL_DEBUG(buf);
		  return ret;
	}
	else{
	  //buf = "I2C Write Sent\r\n";
	}
#endif

  return BME680_OK;
}

static void delay_msec(uint32_t ms){
	HAL_Delay(ms);
}

uint32_t millis() {
  return HAL_GetTick();
}
