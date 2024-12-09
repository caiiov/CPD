# Usar uma imagem base que suporta C++ e OpenMP
FROM ubuntu:22.04

# Configurar o fuso horário
ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Atualizar pacotes e instalar dependências
RUN apt-get update && apt-get install -y \
    build-essential \
    libomp-dev \
    libjsoncpp-dev \
    cmake \
    g++ \
    && apt-get clean

# Criar diretório de trabalho
WORKDIR /app

# Copiar o código para o container
COPY openmp.cpp .
COPY json.hpp /usr/local/include/
COPY web-Google.txt /app

# Compilar o código
RUN g++ -fopenmp -o exec openmp.cpp 

# Comando padrão para execução
CMD ["./exec"]
