#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

namespace ESP32Sensors {
	namespace Accel {

		Adafruit_MPU6050 mpu;

		void inicializar(uint8_t sda, uint8_t scl) {
			// Inicializa comunicação I2C nos pinos informados
			Wire.begin(sda, scl);
			if (!mpu.begin()) {
				Serial.println("Erro: MPU6050 não encontrado. Verifique as conexões.");
				while (1) delay(10);
			}
            mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
            mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        }

		sensors_event_t medirAccel() {
			sensors_event_t accel, gyro, temp;
			mpu.getEvent(&accel, &gyro, &temp);
			return accel;
		}

	}
}