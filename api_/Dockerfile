# Imagen base con Python 3.10
FROM python:3.10-slim

# Instalar dependencias del sistema
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio de trabajo
WORKDIR /app

COPY eigen-3.4.0 ./eigen-3.4.0
# Explicitly copy Eigen

# Dependències de c++, Python (requriements.txt)
COPY Makefile requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Copiar archivos al contenedor
COPY . .

# Compilar la biblioteca C++ usando Makefile
RUN make clean && make

# Exponer el puerto correcto
EXPOSE 8000

# Comando para iniciar FastAPI
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]