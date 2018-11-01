bin=crash

CFLAGS += -Wall -g
LDFLAGS +=

src=shell.c history.c timer.c background.c
obj=$(src:.c=.o)

all: $(bin)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

history.o: history.c history.h
shell.o: shell.c
timer.o: timer.c timer.h
background.o: background.c background.h

clean:
	rm -f $(bin) $(obj)
