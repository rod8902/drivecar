PALSWARE := ../..
PALS_INCDIR := $(PALSWARE)
PALS_LIBDIR := $(PALSWARE)

CFLAGS = -O2 -Wall -g -Wno-format
CFLAGS += -I. -I$(PALS_INCDIR)
CFLAGS += -DPALS_DEBUG

LDFLAGS += -L$(PALS_LIBDIR)
LDLIBS += -lpals -lpthread -lrt

TARGETS = side supervisor

all: $(TARGETS)

SIDE_OBJS = side.o conf.o

SUPERVISOR_OBJS = supervisor.o conf.o

side: $(SIDE_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SIDE_OBJS) $(LDLIBS)

supervisor: $(SUPERVISOR_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SUPERVISOR_OBJS) $(LDLIBS)

$(TARGETS): $(PALS_LIBDIR)/libpals.a

clean:
	$(RM) *.o
	$(RM) $(TARGETS)
	$(RM) .depend

-include .depend

.depend: *.[ch]
	$(CC) $(CFLAGS) -MM *.[ch] > $@
