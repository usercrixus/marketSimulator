# Flags
MYFLAGS = -std=c++17 -Wall -Wextra -Werror
SYSFLAGS = -isystem srcs/externalLibrary/libtorch/include -isystem srcs/externalLibrary/libtorch/include/torch/csrc/api/include -Isrcs/externalLibrary/nlohmann 
# Linking
LDFLAGS = -Lsrc/externalLibrary/nlohmann -Lsrcs/externalLibrary/libtorch/lib -Wl,-rpath=srcs/externalLibrary/libtorch/lib
LDLIBS = -ltorch -ltorch_cpu -ltorch_cuda -lc10
# Source & Object files
SRC = \
	srcs/statistics/Statistics.cpp \
	srcs/agent/Agent.cpp \
	srcs/agent/MarketMakerAgent.cpp \
	srcs/agent/TakerAgent.cpp \
	srcs/agent/models/ModelLSTM.cpp \
	srcs/market/Market.cpp \
	srcs/market/Order.cpp \
	srcs/market/OrderBook.cpp \
	srcs/main.cpp \

OBJ = $(SRC:.cpp=.o)

all: Market.out

Market.out: $(OBJ)
	g++ $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	g++ $(MYFLAGS) $(SYSFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f Market.out

re: fclean all

libtorch:
	@echo "Downloading LibTorch..."
	wget -O srcs/externalLibrary/libtorch.zip "https://download.pytorch.org/libtorch/cu126/libtorch-cxx11-abi-shared-with-deps-2.7.0%2Bcu126.zip"
	unzip -q srcs/externalLibrary/libtorch.zip -d srcs/
	rm srcs/externalLibrary/libtorch.zip

.PHONY: clean fclean re libtorch