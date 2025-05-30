OBJ = \
	market/Market.o \
	market/Order.o \
	market/OrderBook.o \
	agent/Agent.o \

all: Market.out

Market.out: $(OBJ)
	@echo "compiling is ok"

%.o: %.c
	gcc -c -Wall -Werror -Wextra $^ -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f Market.out