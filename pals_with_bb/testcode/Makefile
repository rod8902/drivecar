PALSWARE := ..
PALS_INCDIR := $(PALSWARE)
PALS_LIBDIR := $(PALSWARE)

CFLAGS = -O2 -Wall -g -Wno-format
CFLAGS += -I. -I$(PALS_INCDIR)
CFLAGS += -DPALS_DEBUG

LDFLAGS += -L$(PALS_LIBDIR)
LDLIBS += -lpals -lpthread -lrt

TARGETS = simple comtest multitest

all: $(TARGETS)

$(TARGETS): $(PALS_LIBDIR)/libpals.a

clean:
	$(RM) *.o
	$(RM) $(TARGETS)
	$(RM) .depend

-include .depend

.depend: *.[ch]
	$(CC) $(CFLAGS) -MM *.[ch] > $@
