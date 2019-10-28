CC=gcc
CFLAGS=-I./
SRCS=makecsiparams.c bcmwifi_channels.c
OBJS=$(SRCS:.c=.o)
DEPS=types.h typedefs.h bcmwifi_channels.h bcmutils.h bcmdefs.h bcm_cfg.h

$(ODIR)/%.o: $(SRCS) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

makecsiparams: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJS)
