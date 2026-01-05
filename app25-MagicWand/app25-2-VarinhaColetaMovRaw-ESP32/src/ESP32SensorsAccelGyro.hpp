#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

namespace ESP32Sensors {
	namespace AccelGyro {
		Adafruit_MPU6050 mpu;

		struct DADOS {
			sensors_event_t accel, gyro, temp;
		};

		void inicializar(uint8_t sdaPin, uint8_t sclPin) {
			// Inicializa comunicação I2C nos pinos padrão do ESP32 (SDA, SCL)
			Wire.begin(sdaPin, sclPin);

			if (!mpu.begin()) {
				Serial.println("Erro: MPU6050 não encontrado. Verifique as conexões.");
				while (1) delay(10);
			}
            mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
			mpu.setGyroRange(MPU6050_RANGE_250_DEG);
            mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        }

		DADOS medirAccelGyro() {
			DADOS accel_gyro;
			mpu.getEvent(&accel_gyro.accel, &accel_gyro.gyro, &accel_gyro.temp);
			return accel_gyro;
		}
	}
}