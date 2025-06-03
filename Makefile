# ───────────────────────────────────────────────────────────────────────────
# 1) Compiler flags
CXX       := g++
MYFLAGS   := -std=c++17 -O3 -march=native -Wall -Wextra -Werror
MYFLAGS  += -D_GLIBCXX_USE_CXX11_ABI=1    # match LibTorch ABI

# 2) Where to find torch headers
SYSFLAGS  := \
  -isystem srcs/externalLibrary/libtorch/include \
  -isystem srcs/externalLibrary/libtorch/include/torch/csrc/api/include \
  -Isrcs/externalLibrary/nlohmann

# 3) Linker flags: search LibTorch first, then CUDA’s lib64; embed both in rpath
LDFLAGS   := \
  -Lsrcs/externalLibrary/libtorch/lib \
  -L/usr/local/cuda/lib64 \
  -Wl,-rpath=srcs/externalLibrary/libtorch/lib:/usr/local/cuda/lib64 \
  -Wl,--no-as-needed

# 4) Which libs to link (order matters: torch ⇒ torch_cuda ⇒ torch_cpu ⇒ c10 ⇒ c10_cuda ⇒ cudart)
LDLIBS    := \
  -ltorch \
  -ltorch_cuda \
  -ltorch_cpu \
  -lc10 \
  -lc10_cuda \
  -lcudart \
  -lrt \
  -ldl \
  -lstdc++fs \
  -lpthread \
  -Wl,--as-needed

# 5) Your sources
SRC := \
  srcs/main.cpp \
  srcs/statistics/Statistics.cpp \
  srcs/agent/Agent.cpp \
  srcs/agent/MarketMakerAgent.cpp \
  srcs/agent/TakerAgent.cpp \
  srcs/agent/UntrainedTakerAgent.cpp \
  srcs/agent/models/ModelLSTM.cpp \
  srcs/market/Market.cpp \
  srcs/market/Order.cpp \
  srcs/market/OrderBook.cpp

OBJ := $(SRC:.cpp=.o)

all: Market.out

# Link: place $(LDLIBS) after $(LDFLAGS)
Market.out: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile each .cpp to .o
%.o: %.cpp
	$(CXX) $(MYFLAGS) $(SYSFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f Market.out

re: fclean all

libtorch:
	@echo "Downloading LibTorch (CUDA 12.6)…"
	wget -O srcs/externalLibrary/libtorch.zip \
	  "https://download.pytorch.org/libtorch/cu126/libtorch-cxx11-abi-shared-with-deps-2.7.0%2Bcu126.zip"
	unzip -q srcs/externalLibrary/libtorch.zip -d srcs/externalLibrary
	rm srcs/externalLibrary/libtorch.zip

.PHONY: clean fclean re libtorch
