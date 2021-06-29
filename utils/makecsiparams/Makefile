CC=gcc
CFLAGS=-I./
SRCS=makecsiparams.c bcmwifi_channels.c
OBJS=$(SRCS:.c=.o)
DEPS=types.h typedefs.h bcmwifi_channels.h bcmutils.h bcmdefs.h bcm_cfg.h

$(ODIR)/%.o: $(SRCS) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

makecsiparams: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

ifneq ($(shell uname -m),$(filter $(shell uname -m), armv6l armv7l))
install: libs/armeabi/makecsiparams
	adb push $< /sdcard/
	adb shell 'su -c "mount -o rw,remount /system"'
	adb shell 'su -c "cp /sdcard/makecsiparams /system/bin/makecsiparams"'
	adb shell 'su -c "chmod +x /system/bin/makecsiparams"'

install64: libs/arm64-v8a/makecsiparams
	adb push $< /sdcard/
	adb shell 'su -c "mount -o rw,remount /system"'
	adb shell 'su -c "cp /sdcard/makecsiparams /system/bin/makecsiparams"'
	adb shell 'su -c "chmod +x /system/bin/makecsiparams"'
else
install: makecsiparams
	cp $< /usr/bin/
endif

.PHONY: clean

clean:
	rm -rf $(OBJS)
