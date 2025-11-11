namespace ESP32Sensors {
	namespace AQI {

		// Pinos
		static uint8_t PM25Pin    = 4;
		static uint8_t PM10Pin    = 5;
		static uint8_t NOPin      = 6;
		static uint8_t NO2Pin     = 7;
		static uint8_t NOxPin     = 8;
		static uint8_t NH3Pin     = 3;
		static uint8_t COPin      = 9;
		static uint8_t SO2Pin     = 1;
		static uint8_t O3Pin      = 2;
		static uint8_t BenzenePin = 10;
		static uint8_t ToluenePin = 11;
		static uint8_t XylenePin  = 12;


		struct DADOSAQI {
			float PM25;
			float PM10;
			float NO;
			float NO2;
			float NOx;
			float NH3;
			float CO;
			float SO2;
			float O3;
			float Benzene;
			float Toluene;
			float Xylene;
		};


		// Função de inicialização que recebe os parâmetros de configuração
		void inicializar() {
			pinMode(PM25Pin, INPUT);
			pinMode(PM10Pin, INPUT);
			pinMode(NOPin, INPUT);
			pinMode(NO2Pin, INPUT);
			pinMode(NOxPin, INPUT);
			pinMode(NH3Pin, INPUT);
			pinMode(COPin, INPUT);
			pinMode(SO2Pin, INPUT);
			pinMode(O3Pin, INPUT);
			pinMode(BenzenePin, INPUT);
			pinMode(ToluenePin, INPUT);
			pinMode(XylenePin, INPUT);
			Serial.println("[ESP32SensorsAQI] - Pinos inicializados!");
		}

		// Função que mapeia um valor inteiro para um valor de ponto flutuante
		float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
			return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		}

		DADOSAQI medirAQI() {
			DADOSAQI dados;
			dados.PM25 = 0.0;
			dados.PM10 = 0.0;
			dados.NO = 0.0;
			dados.NO2 = 0.0;
			dados.NOx = 0.0;
			dados.NH3 = 0.0;
			dados.CO = 0.0;
			dados.SO2 = 0.0;
			dados.O3 = 0.0;
			dados.Benzene = 0.0;
			dados.Toluene = 0.0;
			dados.Xylene = 0.0;

			// Leitura e mapeamento de todos os pinos para a estrutura
			int valPM25 = analogRead(PM25Pin);
			dados.PM25 = mapFloat(valPM25, 0, 4095, 0, 250.0);

			int valPM10 = analogRead(PM10Pin);
			dados.PM10 = mapFloat(valPM10, 0, 4095, 0, 430.0);

			int valNO = analogRead(NOPin);
			dados.NO = mapFloat(valNO, 0, 4095, 0, 150.0);

			int valNO2 = analogRead(NO2Pin);
			dados.NO2 = mapFloat(valNO2, 0, 4095, 0, 400.0);

			int valNOx = analogRead(NOxPin);
			dados.NOx = mapFloat(valNOx, 0, 4095, 0, 400.0);

			int valNH3 = analogRead(NH3Pin);
			dados.NH3 = mapFloat(valNH3, 0, 4095, 0, 1800.0);

			int valCO = analogRead(COPin);
			dados.CO = mapFloat(valCO, 0, 4095, 0, 35.0);

			int valSO2 = analogRead(SO2Pin);
			dados.SO2 = mapFloat(valSO2, 0, 4095, 0, 1600.0);

			int valO3 = analogRead(O3Pin);
			dados.O3 = mapFloat(valO3, 0, 4095, 0, 210.0);

			int valBenzene = analogRead(BenzenePin);
			dados.Benzene = mapFloat(valBenzene, 0, 4095, 0, 100.0);

			int valToluene = analogRead(ToluenePin);
			dados.Toluene = mapFloat(valToluene, 0, 4095, 0, 200.0);

			int valXylene = analogRead(XylenePin);
			dados.Xylene = mapFloat(valXylene, 0, 4095, 0, 50.0);

			return dados;
		}
	}
}