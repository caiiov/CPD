# OpenMPI: Instalação e Execução

Este repositório fornece um guia detalhado para compilar e executar um programa em C++ utilizando OpenMPI e bibliotecas adicionais, como **zlib** e **JSON-C**. Também inclui instruções para criar e executar uma imagem Docker para a aplicação. Siga as etapas abaixo para configurar e executar o projeto no seu ambiente.

---

## Pré-requisitos

Certifique-se de que sua máquina possui os seguintes requisitos instalados:

- **Linux** (recomendado Ubuntu/Debian)
- **Docker**
- **Bibliotecas necessárias**:
  - OpenMPI (`libopenmpi-dev`)
  - JSON (`nlohmann/json.hpp`)
  - Zlib (geralmente já instalado em distribuições Linux)

---

## Instalando Dependências

1. **Atualize os pacotes**  
   Execute o comando abaixo para garantir que os pacotes do sistema estejam atualizados:  
   ```bash
   sudo apt update
2. **Instale o OpenMPI**
  Instale o OpenMPI com o comando:
   ```bash
   sudo apt install libopenmpi-dev 
3. **Instale a biblioteca JSON**
   sudo apt install nlohmann-json3-dev
   ```bash
   sudo apt install nlohmann-json3-dev
4. **Instale o Docker**
   Instale o Docker e seus componentes com o comando:
   ```bash
   sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
---
## Compilando e Executando o Programa 

1. **Compile o programa**
   Utilize o seguinte comando para compilar o código:
   ```bash
   mpic++ -o openmpi openmpi.cpp -lz -ljson-c -lm

2. **Verifique o arquivo executável**
   Após a compilação, use o comando abaixo para listar os arquivos do diretório e verificar se o executável openmpi foi criado:
   ```bash
   ls

3. **Execute o programa**
   Execute o programa utilizando o OpenMPI:
   ```bash
   mpirun -np 4 ./openmpi
- -np 4: Especifica o número de processos paralelos a serem executados.

---
  ## Usando Docker
1. **Crie a imagem Docker**
   Certifique-se de estar no mesmo diretório onde está localizado o arquivo Dockerfile. Em seguida, crie a imagem Docker com o comando:
   ```bash
   sudo docker build -t openmpi .
2. **Execute o container Docker**
   Inicie o container com o seguinte comando:
   ```bash
   sudo docker run --rm -it openmpi
  - --rm: Remove o container automaticamente após a execução.
  - -it: Inicia o container em modo interativo.
