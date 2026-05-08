#include <FlixPeriph.h>
#include <MPU6500.h>
#include <Wire.h>

namespace ESP32Sensors {
	namespace AccelGyro {
		static IMUBase* _mpu = nullptr;

		struct DADOS {
			float ax, ay, az;  // acelerômetro em m/s²
			float gx, gy, gz;  // giroscópio em rad/s
		};

		// Lê o registrador WHO_AM_I (0x75) no endereço padrão do MPU (0x68)
		static uint8_t _lerWhoAmI() {
			Wire.beginTransmission(0x68);
			Wire.write(0x75);
			Wire.endTransmission(false);
			Wire.requestFrom((uint8_t)0x68, (uint8_t)1);
			if (!Wire.available()) return 0x00;
			return Wire.read();
		}

		void inicializar(uint8_t sdaPin, uint8_t sclPin) {
			Wire.begin(sdaPin, sclPin);

			uint8_t whoami = _lerWhoAmI();
			if (whoami == 0x00) {
				Serial.println("Erro: nenhum sensor IMU encontrado no endereço 0x68.");
				while (1) delay(10);
			}

			// WHO_AM_I 0x70 = MPU6500 (genérico vendido como MPU6050)
			// WHO_AM_I 0x68 = MPU6050 original (GY-521)
			if (whoami == 0x70) {
				Wire.setClock(400000);
				_mpu = new MPU6500(Wire);
				Serial.println("Sensor: MPU6500 detectado");
			} else {
				_mpu = new MPU6050(Wire);
				Serial.printf("Sensor: MPU6050 detectado (WHO_AM_I=0x%02X)\n", whoami);
			}

			auto& mpu = *_mpu;
			if (mpu.begin() < 0) {
				Serial.println("Erro: falha ao inicializar o IMU.");
				while (1) delay(10);
			}
			mpu.setAccelRange(IMUInterface::ACCEL_RANGE_16G);
			mpu.setGyroRange(IMUInterface::GYRO_RANGE_250DPS);
		}

		DADOS medirAccelGyro() {
			auto& mpu = *_mpu;
			DADOS dados;
			mpu.read();
			mpu.getAccel(dados.ax, dados.ay, dados.az);
			mpu.getGyro(dados.gx, dados.gy, dados.gz);
			return dados;
		}
	}
}