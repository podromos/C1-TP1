SRC=*.c
CFLAGS= -g -lpthread

exec : $(SRC)
	gcc $^ -o $@ $(CFLAGS)

.PHONY:clean
clean :
	rm exec -f