CFLAGS = -O2 -Wall -g
#CFLAGS += -Wno-format
CFLAGS += -I.
CFLAGS += -DPALS_DEBUG

OBJS = pals_env.o
OBJS += pals_time.o pals_timer.o
OBJS += pals_lock.o
OBJS += sock_tx.o sock_rx.o sock_mcast_tx.o sock_mcast_rx.o
OBJS += pals_task.o pals_task_open.o pals_task_main.o
OBJS += pals_task_add_phase.o
OBJS += pals_tx_port_open.o pals_rx_port_open.o
OBJS += pals_send.o pals_recv.o

SRCS = $(OBJS:.o=.c);

TARGET = libpals.a

all: $(TARGET)

$(TARGET): $(TARGET)($(OBJS))
	ranlib $@

clean:
	$(RM) -f $(OBJS)
	$(RM) -f $(TARGET)
	$(RM) -f .depend

-include .depend

.depend: *.[ch]
	$(CC) $(CFLAGS) -MM *.[ch] > $@
