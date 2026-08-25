FROM python:3.12-slim
WORKDIR /app

# Instalar dependências do serviço
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copiar o serviço
COPY service_app.py .

# O modelo vem via volume em /app/model
ENV MODEL_PATH=/app/model/modelo_vaccinesense.pkl

EXPOSE 8000
CMD ["uvicorn", "service_app:app", "--host", "0.0.0.0", "--port", "8000"]
