#Pthreads
---
## Pré-requisitos
Docker instalado no sistema.
No Ubuntu:
```bash
sudo apt update
sudo apt install -y docker.io

Conteúdo do Repositório
pthreads.cpp: Código-fonte principal do analisador de grafos.
Dockerfile: Configuração do Docker para criar o container com o ambiente necessário.

Como Buildar o Container
Clone ou copie este repositório para sua máquina.

Certifique-se de que o arquivo pthreads.cpp, web-google.txt e o Dockerfile estão no mesmo diretório.


Construa a imagem Docker:
docker build -t pthreads .

Como Rodar o Container
1. Executar o Código
```bash
docker run --rm pthreads

O código será executado no container, e as métricas do grafo serão exibidas no terminal.



---


# OpenMP
---
## Pré-requisitos

é necessário ter o json.hpp no mesmo diretório

Para compilar usa o comando:
```bash
g++ -fopenmp -o exec openmp.cpp

Para executar:
```bash
./openmp.cpp





---
# OpenMPI

---


## Instalando Dependências

**Atualize os pacotes**  
   Execute o comando abaixo para garantir que os pacotes do sistema estejam atualizados:  
   ```bash
   sudo apt update
**Instale o OpenMPI**
  Instale o OpenMPI com o comando:
   ```bash
   sudo apt install libopenmpi-dev 
**Instale a biblioteca JSON**
   sudo apt install nlohmann-json3-dev
   ```bash
   sudo apt install nlohmann-json3-dev
**Instale o Docker**
   Instale o Docker e seus componentes com o comando:
   ```bash
   sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
---
## Compilando e Executando o Programa 

**Compile o programa**
   Utilize o seguinte comando para compilar o código:
   ```bash
   mpic++ -o openmpi openmpi.cpp -lz -ljson-c -lm

**Verifique o arquivo executável**
   Após a compilação, use o comando abaixo para listar os arquivos do diretório e verificar se o executável openmpi foi criado:
   ```bash
   ls

**Execute o programa**
   Execute o programa utilizando o OpenMPI:
   ```bash
   mpirun -np 4 ./openmpi
- -np 4: Especifica o número de processos paralelos a serem executados.

---
  ## Usando Docker
**Crie a imagem Docker**
   Certifique-se de estar no mesmo diretório onde está localizado o arquivo Dockerfile. Em seguida, crie a imagem Docker com o comando:
   ```bash
   sudo docker build -t openmpi .
**Execute o container Docker**
   Inicie o container com o seguinte comando:
   ```bash
   sudo docker run --rm -it openmpi
  - --rm: Remove o container automaticamente após a execução.
  - -it: Inicia o container em modo interativo.

Não conseguimos fazer os containers se comunicarem, pedimos desculpas por isso.
Certifique-se de que o arquivo openmpi.cpp, web-google.txt e o Dockerfile estão no mesmo diretório.
---

## Agradecimentos:
Agradecemos ao Vandre e ao Leonardo por disponibilizarem os códigos que serviram como base e aplicação na atividade solicitada pelo professor. Agradecemos também ao professor Alysson pela paciência e pela ajuda em todos os momentos. 