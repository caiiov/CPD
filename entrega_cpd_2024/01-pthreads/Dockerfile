# Usar uma imagem base do Ubuntu
FROM ubuntu:20.04

# Definir o fuso horário e a região
ENV TZ=America/Sao_Paulo
RUN apt-get update && apt-get install -y \
    tzdata \
    && ln -fs /usr/share/zoneinfo/$TZ /etc/localtime \
    && dpkg-reconfigure --frontend noninteractive tzdata

# Instalar dependências necessárias
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    cmake \
    libjsoncpp-dev \
    libpthread-stubs0-dev \
    libpthread-workqueue-dev \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    && apt-get clean

# Instalar o JsonCpp (caso não esteja instalado)
RUN apt-get install -y libjsoncpp-dev

# Instalar as dependências de C++ para compilar o código
RUN apt-get install -y \
    libpthread-stubs0-dev \
    libpthread-workqueue-dev

# Criar diretório para o código
WORKDIR /app

# Copiar o código C++ para dentro do container
COPY . /app

# Compilar o código
RUN g++ -o graph_analysis pthreads.cpp -ljsoncpp -lpthread -std=c++17

# Definir o comando para rodar o programa
CMD ["./graph_analysis"]

# Expor a porta caso você queira se conectar de fora (opcional)
EXPOSE 8080
