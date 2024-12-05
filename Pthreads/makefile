# Nome do executável final (pode ser o nome do conjunto de métricas)
TARGET = metrics_calculator

# Compilador e flags
CXX = g++
CXXFLAGS = -pthread -Wall -O2

# Lista de arquivos fonte
SRCS = $(wildcard *.cpp)

# Arquivos objeto correspondentes
OBJS = $(SRCS:.cpp=.o)

# Regra padrão para compilar tudo
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regra para compilar arquivos objeto individualmente
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJS) $(TARGET)
