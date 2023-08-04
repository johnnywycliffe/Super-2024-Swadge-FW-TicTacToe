//==============================================================================
// Includes
//==============================================================================

#include "hdw-accel.h"
#include "hdw-accel_emu.h"
#include "trigonometry.h"
#include "esp_random.h"
#include "emu_main.h"
#include "macros.h"

#define ONE_G 242

static bool accelInit  = false;
static int16_t _accelX = 0;
static int16_t _accelY = 0;
static int16_t _accelZ = 0;

/**
 * @brief Initialize the accelerometer
 *
 * @param _i2c_port The i2c port to use for the accelerometer
 * @param range The range to measure, between ::QMA_RANGE_2G and ::QMA_RANGE_32G
 * @param bandwidth The bandwidth to measure at, between ::QMA_BANDWIDTH_128_HZ and ::QMA_BANDWIDTH_1024_HZ
 * @return ESP_OK if the accelerometer initialized, or a non-zero value if it did not
 */
esp_err_t initAccelerometer(i2c_port_t _i2c_port, gpio_num_t sda, gpio_num_t scl, gpio_pullup_t pullup, uint32_t clkHz,
                            qma_range_t range, qma_bandwidth_t bandwidth)
{
    // divide up one G evenly between the axes, randomly
    // the math doesn't quite math but honestly it's good enough
    if (emulatorArgs.emulateMotion)
    {
        emulatorSetAccelerometerRotation(ONE_G, emulatorArgs.motionZ, emulatorArgs.motionX);
    }
    else
    {
        // at least give a somewhat sane reading
        _accelZ = ONE_G;
    }
    accelInit = true;
    return ESP_OK;
}

/**
 * @brief Deinitialize the accelerometer (do nothing)
 *
 * @return esp_err_t
 */
esp_err_t deInitAccelerometer(void)
{
    accelInit = false;
    return ESP_OK;
}

/**
 * @brief Read and return the 16-bit step counter
 *
 * Note that this can be configured with ::QMA7981_REG_STEP_CONF_0 through ::QMA7981_REG_STEP_CONF_3
 *
 * @param data The step counter value is written here
 * @return ESP_OK if the step count was read, or a non-zero value if it was not
 */
esp_err_t accelGetStep(uint16_t* data)
{
    if (accelInit)
    {
        // TODO emulate step better
        *data = 0;
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_INVALID_STATE;
    }
}

/**
 * @brief Set the accelerometer's measurement range
 *
 * @param range The range to measure, from ::QMA_RANGE_2G to ::QMA_RANGE_32G
 * @return ESP_OK if the range was set, or a non-zero value if it was not
 */
esp_err_t accelSetRange(qma_range_t range)
{
    if (accelInit)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_INVALID_STATE;
    }
}

/**
 * @brief Read the current acceleration vector from the accelerometer and return
 * the vector through arguments. If the read fails, the last known values are
 * returned instead.
 *
 * @param x The X component of the acceleration vector is written here
 * @param y The Y component of the acceleration vector is written here
 * @param z The Z component of the acceleration vector is written here
 * @return ESP_OK if the acceleration was read, or a non-zero value if it was not
 */
esp_err_t accelGetAccelVec(int16_t* x, int16_t* y, int16_t* z)
{
    if (accelInit)
    {
        if (emulatorArgs.motionJitter)
        {
            // Set each coordinate to the real coordinate +/- the motionJitterAmount, but still clamp the total value to +/- 512
            *x = MAX(-512, MIN(512, _accelX + (esp_random() % (emulatorArgs.motionJitterAmount * 2 + 1) - emulatorArgs.motionJitterAmount - 1)));
            *y = MAX(-512, MIN(512, _accelY + (esp_random() % (emulatorArgs.motionJitterAmount * 2 + 1) - emulatorArgs.motionJitterAmount - 1)));
            *z = MAX(-512, MIN(512, _accelZ + (esp_random() % (emulatorArgs.motionJitterAmount * 2 + 1) - emulatorArgs.motionJitterAmount - 1)));
        }
        else
        {
            *x = _accelX;
            *y = _accelY;
            *z = _accelZ;
        }

        // randomly, approximately every 3 readings
        if (emulatorArgs.motionDrift)
        {
            switch (esp_random() % 6)
            {
                case 0:
                emulatorArgs.motionX = (emulatorArgs.motionX + 1) % 360;
                break;

                case 1:
                emulatorArgs.motionX = (emulatorArgs.motionX + 359) % 360;
                break;

                default:
                break;
            }

            switch (esp_random() % 6)
            {
                case 0:
                emulatorArgs.motionZ = (emulatorArgs.motionZ + 1) % 360;
                break;

                case 1:
                emulatorArgs.motionZ = (emulatorArgs.motionZ + 359) % 360;
                break;

                default:
                break;
            }
        }
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_INVALID_STATE;
    }
}

/**
 * @brief Sets the raw accelerometer reading to be returned by the emulator
 *
 * @param x The x axis value, from -512 to 512, inclusive
 * @param y The y axis value, from -512 to 512, inclusive
 * @param z The z axis value, from -512 to 512, inclusive
 */
void emulatorSetAccelerometer(int16_t x, int16_t y, int16_t z)
{
    if (x >= -512 && x <= 512 && y >= -512 && y <= 512 && z >= -512 && z <= 512)
    {
        _accelX = x;
        _accelY = y;
        _accelZ = z;
    }
}

/**
 * @brief Sets the raw accelerometer reading to the vector defined by the given magnitude and rotations.
 *
 * @param value     The magnitude of the acceleration vector, from -512 to 512, inclusive
 * @param zRotation The counterclockwise rotation about the +Z axis, in degrees
 * @param xRotation The counterclockwise rotation about the +X axis, in degrees
 */
void emulatorSetAccelerometerRotation(int16_t value, uint16_t zRotation, uint16_t xRotation)
{
    if (value >= -512 && value <= 512 && zRotation <= 360 && xRotation <= 360)
    {
        /*
        stackoverflow says:

        x = r * sin(polar) * cos(alpha)
        y = r * sin(polar) * sin(alpha)
        z = r * cos(polar)

        Where:

        r     is the Radius
        alpha is the horizontal angle from the X axis
        polar is the vertical angle from the Z axis
        */
        _accelX = value * getSin1024(zRotation) / 1024 * getCos1024(xRotation) / 1024;
        _accelY = value * getSin1024(zRotation) / 1024 * getSin1024(xRotation) / 1024;
        _accelZ = value * getCos1024(zRotation) / 1024;
    }
}