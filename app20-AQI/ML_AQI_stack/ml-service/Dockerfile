FROM tensorflow/tensorflow:2.19.0
WORKDIR /app

# Instalar dependências do serviço
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copiar o serviço
COPY service_app.py .

# Modelos virão via volume em /app/model
ENV MODEL_PATH=/app/model/modelo_aqi_nn.keras
ENV PREP_PATH=/app/model/preprocess_aqi.pkl

EXPOSE 8000
CMD ["uvicorn", "service_app:app", "--host", "0.0.0.0", "--port", "8000"]
