# Usa una imagen base con Ubuntu
FROM ubuntu:20.04


# Evitar interacciones durante la instalación
ENV DEBIAN_FRONTEND=noninteractive


# Instala dependencias necesarias - depen del que MATLAB necessiti
RUN apt-get update && apt-get install -y \
    unzip \
    xorg \
    libxt6 \
    libxmu6 \
    libxi6 \
    libglu1-mesa \
    python3 \
    python3-pip \
    libgtk-3-0 \
    dbus-x11 \
    libnotify4 \
    libssl-dev \
    libncurses5 \
    libcurl4-openssl-dev \
    libboost-dev \
    sudo \
    wget \
    libxcomposite1 \
    libxcursor1 \
    libxdamage1 \
    libxfixes3 \
    libxrandr2 \
    libxtst6 \
    libasound2 \
    libnss3 \
    libatk1.0-0 \
    libatk-bridge2.0-0 \
    libgbm1 \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*


# Instal·la 'matlab_installer.zip', 'installer_input.txt'i la lisence en un mateix dir
COPY matlab_R2024b_Linux.zip /tmp/
COPY installer_input.txt /tmp/
COPY license.lic /tmp/


# Descomprimeix + permisos execució
RUN unzip /tmp/matlab_R2024b_Linux.zip -d /tmp/matlab_R2024b_Linux && \
    chmod -R 755 /tmp/matlab_R2024b_Linux


# Instal·lació + Verificació
#RUN /tmp/matlab_R2024b_Linux/install -inputFile /tmp/installer_input.txt -mode silent && \
#    ls /usr/local/MATLAB/R2024b || (echo "MATLAB no se instaló" && exit 1)


# Instalación de MATLAB con manejo de errores
RUN /tmp/matlab_R2024b_Linux/install -inputFile /tmp/installer_input.txt -logfile /tmp/install_log.txt || true
ping MATLAB.S.UPF.EDU  # ¿Responde?
telnet MATLAB.S.UPF.EDU 27000  # ¿Conexión exitosa?

# Instala el engine de MATLAB para Python
RUN cd /usr/local/MATLAB/R2024b/extern/engines/python && \
    python3 setup.py install


# Configura la variable PATH para que MATLAB esté disponible
ENV PATH="/usr/local/MATLAB/R2024b/bin:${PATH}"


# Instala FastAPI y Uvicorn
RUN pip3 install fastapi uvicorn


# Copiar la resta dels arxius
COPY . /app
WORKDIR /app


# Expose port
EXPOSE 8000


# Command per engegar la API
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]


