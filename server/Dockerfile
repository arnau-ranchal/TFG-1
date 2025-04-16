# Fase de construcción
FROM python:3.10-slim as builder

WORKDIR /app
COPY Makefile requirements.txt ./

# Instalar solo dependencias de compilación
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

RUN pip install --user -r requirements.txt

COPY . .
RUN make clean && make

# Fase final
FROM python:3.10-slim

WORKDIR /app

# Copiar solo lo necesario desde builder
COPY --from=builder /root/.local /root/.local
COPY --from=builder /app .

# Asegurar que los scripts en .local están en el PATH
ENV PATH=/root/.local/bin:$PATH

# Variables de entorno configurables
ENV PORT=8000
ENV PYTHONUNBUFFERED=1

EXPOSE ${PORT}

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "${PORT}"]