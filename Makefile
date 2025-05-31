# Flags
MYFLAGS = -std=c++17 -Wall -Wextra -Werror
SYSFLAGS = -isystem srcs/libtorch/include -isystem srcs/libtorch/include/torch/csrc/api/include
# Linking
LDFLAGS = -Lsrcs/libtorch/lib -Wl,-rpath=srcs/libtorch/lib
LDLIBS = -ltorch -ltorch_cpu -ltorch_cuda -lc10
# Source & Object files
SRC = \
	srcs/statistics/Statistics.cpp \
	srcs/agent/Agent.cpp \
	srcs/agent/RandomAgent.cpp \
	srcs/market/Market.cpp \
	srcs/market/Order.cpp \
	srcs/market/OrderBook.cpp \

OBJ = $(SRC:.cpp=.o)

all: Market.out

Market.out: $(OBJ)
	@echo "need a main : g++ $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)"

%.o: %.cpp
	g++ $(MYFLAGS) $(SYSFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f Market.out

re: fclean all

libtorch:
	@echo "Downloading LibTorch..."
	wget -O srcs/libtorch.zip "https://download.pytorch.org/libtorch/cu126/libtorch-cxx11-abi-shared-with-deps-2.7.0%2Bcu126.zip"
	unzip -q srcs/libtorch.zip -d srcs/
	rm srcs/libtorch.zip

.PHONY: clean fclean re libtorch