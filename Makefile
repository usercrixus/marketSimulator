OBJ = \
	srcs/market/Market.o \
	srcs/market/Order.o \
	srcs/market/OrderBook.o \
	srcs/agent/Agent.o \

all: Market.out

Market.out: $(OBJ)
	@echo "compiling is ok"

%.o: %.c
	gcc -c -Wall -Werror -Wextra $^ -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f Market.out