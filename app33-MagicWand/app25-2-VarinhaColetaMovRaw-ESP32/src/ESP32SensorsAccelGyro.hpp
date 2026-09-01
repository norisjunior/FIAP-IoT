/* =============================================================================
 * ESP32SensorsAccelGyro.hpp — wrapper de IMU (MPU6050 / MPU6500) via FastIMU
 *
 * POR QUE FastIMU (e não mais FlixPeriph)
 *   A FlixPeriph @1.10.3 amostra por INTERRUPÇÃO de timer de hardware e usa a
 *   API nova do Arduino core >= 3.0 (`timerBegin(freq)`, `timerAlarm`,
 *   `timerAttachInterruptArg`). Como todos os apps do curso estão fixados em
 *   `espressif32@6.12.0` (Arduino core 2.0.17), a FlixPeriph não compila.
 *   A FastIMU (LiquidCGS) lê por POLLING no I2C (Wire) — sem timer, sem ISR —
 *   e compila tanto no core 2.x quanto no 3.x. É a mesma lib usada no app17.
 *
 * CONTRATO PRESERVADO (o coletor e o modelo Edge Impulse dependem disto)
 *   - `medirAccelGyro()` devolve aceleração em m/s²  (Z em repouso ~ 9,81)
 *   - e giroscópio em rad/s
 *   - fundo de escala ±16 g / ±250 dps, DLPF ~184 Hz, taxa interna 1 kHz,
 *     SEM calibração em runtime — exatamente o que a versão FlixPeriph produzia.
 *   A FastIMU entrega accel em g e giro em °/s; a conversão é feita aqui, de
 *   modo que os dois firmwares (coleta e detecção) continuem gerando o mesmo
 *   sinal com que o modelo foi treinado.
 * ===========================================================================*/
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "FastIMU.h"

namespace ESP32Sensors {
	namespace AccelGyro {

		// Driver genérico da FastIMU: mesmo mapa de registradores serve para
		// MPU6050 e MPU6500 (a escala é definida por setAccelRange/setGyroRange).
		static IMU_Generic _mpu;
		static calData     _calib = { 0 };   // zerada: sem calibração em runtime
		static const uint8_t _ADDR = 0x68;

		// Conversões para manter as MESMAS unidades da versão FlixPeriph.
		static constexpr float _G_PARA_MS2    = 9.80665f;              // g   -> m/s²
		static constexpr float _DPS_PARA_RADS = 0.017453292519943295f; // °/s -> rad/s  (PI/180)

		struct DADOS {
			float ax, ay, az;  // acelerômetro em m/s²
			float gx, gy, gz;  // giroscópio em rad/s
		};

		// Lê WHO_AM_I (0x75) no endereço padrão (0x68) — só para diagnóstico e
		// para detectar a ausência do sensor.
		static uint8_t _lerWhoAmI() {
			Wire.beginTransmission(_ADDR);
			Wire.write(0x75);
			Wire.endTransmission(false);
			Wire.requestFrom((uint8_t)_ADDR, (uint8_t)1);
			if (!Wire.available()) return 0x00;
			return Wire.read();
		}

		void inicializar(uint8_t sdaPin, uint8_t sclPin) {
			Wire.begin(sdaPin, sclPin);
			Wire.setClock(400000);

			uint8_t whoami = _lerWhoAmI();
			if (whoami == 0x00) {
				Serial.println("Erro: nenhum sensor IMU encontrado no endereço 0x68.");
				while (1) delay(10);
			}
			// WHO_AM_I 0x70 = MPU6500 (genérico vendido como "MPU6050")
			// WHO_AM_I 0x68 = MPU6050 original (GY-521)
			Serial.printf("Sensor IMU detectado (WHO_AM_I=0x%02X)\n", whoami);

			if (_mpu.init(_calib, _ADDR) != 0) {
				Serial.println("Erro: falha ao inicializar o IMU (FastIMU).");
				while (1) delay(10);
			}

			// Mesma configuração da versão FlixPeriph:
			_mpu.setAccelRange(16);   // ±16 g   (era IMUInterface::ACCEL_RANGE_16G)
			_mpu.setGyroRange(250);   // ±250 dps (era IMUInterface::GYRO_RANGE_250DPS)
			_mpu.setAccelLPF(184);    // DLPF ~184 Hz (default da FlixPeriph)
			_mpu.setGyroLPF(188);     // valor de hardware mais próximo de 184 Hz
			_mpu.setAccelODR(1000);   // taxa interna 1 kHz (FlixPeriph usa SRD=0)
		}

		DADOS medirAccelGyro() {
			AccelData a;
			GyroData  g;
			_mpu.update();
			_mpu.getAccel(&a);   // g
			_mpu.getGyro(&g);    // °/s

			DADOS dados;
			dados.ax = a.accelX * _G_PARA_MS2;
			dados.ay = a.accelY * _G_PARA_MS2;
			dados.az = a.accelZ * _G_PARA_MS2;
			dados.gx = g.gyroX * _DPS_PARA_RADS;
			dados.gy = g.gyroY * _DPS_PARA_RADS;
			dados.gz = g.gyroZ * _DPS_PARA_RADS;
			return dados;
		}
	}
}
