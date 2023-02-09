GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
include ../version.mk
include $(FW_PATH)/definitions.mk

COMMON_SRCS=$(wildcard $(NEXMON_ROOT)/patches/common/*.c)
FW_SRCS=$(wildcard $(FW_PATH)/*.c)

UCODEFILE:=
SPLITPATH=$(subst /, ,$(FW_PATH))
FWUCODE=$(word $(shell expr $(words $(SPLITPATH)) - 1),$(SPLITPATH)).$(lastword $(SPLITPATH)).patch
UCODES=$(filter %$(FWUCODE),$(notdir $(wildcard src/*.patch)))
ifeq ($(UCODEFILE),)
UCODEFILE=csi.ucode.$(FWUCODE)
endif

ifneq ($(findstring bcm43455c0,$(FWUCODE)), )
LOCAL_SRCS=$(wildcard src/*.c) src/ucode_compressed.c
else ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
LOCAL_SRCS=$(wildcard src/*.c) src/nonmuucode_compressed.c src/nonmuucodex_compressed.c src/muucode_compressed.c src/muucodex_compressed.c
else
LOCAL_SRCS=$(wildcard src/*.c) src/ucode_compressed.c src/templateram.c
endif

ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
B43VERSION=b43-v2
else ifneq ($(findstring bcm43455c0,$(FWUCODE)), )
B43VERSION=b43-v3
else ifneq ($(findstring bcm4358,$(FWUCODE)), )
B43VERSION=b43-v3
else
B43VERSION=b43
endif

ADBSERIAL := 
ADBFLAGS := $(ADBSERIAL)

REMOTEADDR := $(REMOTEADDR)

OBJS=$(addprefix obj/,$(notdir $(LOCAL_SRCS:.c=.o)) $(notdir $(COMMON_SRCS:.c=.o)) $(notdir $(FW_SRCS:.c=.o)))

CFLAGS= \
	-fplugin=$(CCPLUGIN) \
	-fplugin-arg-nexmon-objfile=$@ \
	-fplugin-arg-nexmon-prefile=gen/nexmon.pre \
	-fplugin-arg-nexmon-chipver=$(NEXMON_CHIP_NUM) \
	-fplugin-arg-nexmon-fwver=$(NEXMON_FW_VERSION_NUM) \
	-fno-strict-aliasing \
	-DNEXMON_CHIP=$(NEXMON_CHIP) \
	-DNEXMON_FW_VERSION=$(NEXMON_FW_VERSION) \
	-DVERSION_PTR=$(VERSION_PTR) \
	-DWLC_UCODE_WRITE_BL_HOOK_ADDR=$(WLC_UCODE_WRITE_BL_HOOK_ADDR) \
    -DHNDRTE_RECLAIM_0_END_PTR=$(HNDRTE_RECLAIM_0_END_PTR) \
    -DHNDRTE_RECLAIM_UCODES_END_PTR=$(HNDRTE_RECLAIM_UCODES_END_PTR) \
    -DWLC_NONMUUCODE_WRITE_BL_HOOK_ADDR=$(WLC_NONMUUCODE_WRITE_BL_HOOK_ADDR) \
    -DNONMUUCODESTART_PTR=$(NONMUUCODESTART_PTR) \
    -DNONMUUCODESIZE_PTR=$(NONMUUCODESIZE_PTR) \
    -DWLC_NONMUUCODEX_WRITE_BL_HOOK_ADDR=$(WLC_NONMUUCODEX_WRITE_BL_HOOK_ADDR) \
    -DNONMUUCODEXSTART_PTR=$(NONMUUCODEXSTART_PTR) \
    -DNONMUUCODEXSIZE_PTR=$(NONMUUCODEXSIZE_PTR) \
    -DWLC_MUUCODE_WRITE_BL_HOOK_ADDR=$(WLC_MUUCODE_WRITE_BL_HOOK_ADDR) \
    -DMUUCODESTART_PTR=$(MUUCODESTART_PTR) \
    -DMUUCODESIZE_PTR=$(MUUCODESIZE_PTR) \
    -DWLC_MUUCODEX_WRITE_BL_HOOK_ADDR=$(WLC_MUUCODEX_WRITE_BL_HOOK_ADDR) \
    -DMUUCODEXSTART_PTR=$(MUUCODEXSTART_PTR) \
    -DMUUCODEXSIZE_PTR=$(MUUCODEXSIZE_PTR) \
    -DTEMPLATERAMSTART_PTR=$(TEMPLATERAMSTART_PTR) \
	-DPATCHSTART=$(PATCHSTART) \
	-DUCODESIZE=$(UCODESIZE) \
	-DRXE_RXHDR_LEN=$(RXE_RXHDR_LEN) \
	-DRXE_RXHDR_EXTRA=$(RXE_RXHDR_EXTRA) \
	-DGIT_VERSION=\"$(GIT_VERSION)\" \
	-DBUILD_NUMBER=\"$$(cat BUILD_NUMBER)\" \
	-Wall -Werror -Wno-unused-function -Wno-unused-variable \
	-O2 -nostdlib -nostartfiles -ffreestanding -mthumb -march=$(NEXMON_ARCH) \
	-ffunction-sections -fdata-sections \
	-I$(NEXMON_ROOT)/patches/include \
	-Iinclude \
	-I$(FW_PATH)


# only make dhd.ko for bcm4366c0
ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
all: $(RAM_FILE) dhd.ko

dhd.ko: $(RAM_FILE)
	$(Q)cp $(FW_PATH)/dhd.ko .
	$(Q)dd conv=notrunc bs=1 if=$< of=$@ seek=$$((0x4E798)) count=$$((0x10F4CF)) status=none
else
all: $(RAM_FILE)
endif

init: FORCE
	$(Q)if ! test -f BUILD_NUMBER; then echo 0 > BUILD_NUMBER; fi
	$(Q)echo $$(($$(cat BUILD_NUMBER) + 1)) > BUILD_NUMBER
	$(Q)touch src/version.c
	$(Q)make -s -f $(NEXMON_ROOT)/patches/common/header.mk
	$(Q)mkdir -p obj gen log

# only make for bcm43455c0
ifneq ($(findstring bcm43455c0,$(FWUCODE)), )
brcmfmac.ko: check-nexmon-setup-env
ifeq ($(shell uname -m),$(filter $(shell uname -m), armv6l armv7l))
ifeq ($(findstring 4.19,$(shell uname -r)),4.19)
	@printf "\033[0;31m  BUILDING DRIVER for kernel 4.19\033[0m brcmfmac_4.19.y-nexmon/brcmfmac.ko (details: log/driver.log)\n" $@
	$(Q)make -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_4.19.y-nexmon -j2 >log/driver.log
else ifeq ($(findstring 5.4,$(shell uname -r)),5.4)
	@printf "\033[0;31m  BUILDING DRIVER for kernel 5.4\033[0m brcmfmac_5.4.y-nexmon/brcmfmac.ko (details: log/driver.log)\n" $@
	$(Q)make -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_5.4.y-nexmon -j2 >log/driver.log
else ifeq ($(findstring 5.10,$(shell uname -r)),5.10)
	@printf "\033[0;31m  BUILDING DRIVER for kernel 5.10\033[0m brcmfmac_5.10.y-nexmon/brcmfmac.ko (details: log/driver.log)\n" $@
	$(Q)make --trace -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_5.10.y-nexmon -j1 # >log/driver.log
else
	$(warning Warning: Kernel version not supported)
endif
else
	$(warning Warning: Driver can not be compiled on this platform, execute the make command on a raspberry pi)
endif
endif

obj/%.o: src/%.c
	@printf "\033[0;31m  COMPILING\033[0m %s => %s (details: log/compiler.log)\n" $< $@
	$(Q)cat gen/nexmon.pre 2>>log/error.log | gawk '{ if ($$3 != "$@") print; }' > tmp && mv tmp gen/nexmon.pre
	$(Q)$(CC)gcc $(CFLAGS) -c $< -o $@ >>log/compiler.log

obj/%.o: $(NEXMON_ROOT)/patches/common/%.c
	@printf "\033[0;31m  COMPILING\033[0m %s => %s (details: log/compiler.log)\n" $< $@
	$(Q)cat gen/nexmon.pre 2>>log/error.log | gawk '{ if ($$3 != "$@") print; }' > tmp && mv tmp gen/nexmon.pre
	$(Q)$(CC)gcc $(CFLAGS) -c $< -o $@ >>log/compiler.log

obj/%.o: $(FW_PATH)/%.c
	@printf "\033[0;31m  COMPILING\033[0m %s => %s (details: log/compiler.log)\n" $< $@
	$(Q)cat gen/nexmon.pre 2>>log/error.log | gawk '{ if ($$3 != "$@") print; }' > tmp && mv tmp gen/nexmon.pre
	$(Q)$(CC)gcc $(CFLAGS) -c $< -o $@ >>log/compiler.log

gen/nexmon2.pre: $(OBJS)
	@printf "\033[0;31m  PREPARING\033[0m %s => %s\n" "gen/nexmon.pre" $@
	$(Q)cat gen/nexmon.pre | awk '{ if ($$3 != "obj/flashpatches.o" && $$3 != "obj/wrapper.o") { print $$0; } }' > tmp
	$(Q)cat gen/nexmon.pre | awk '{ if ($$3 == "obj/flashpatches.o" || $$3 == "obj/wrapper.o") { print $$0; } }' >> tmp
	$(Q)cat tmp | awk '{ if ($$1 ~ /^0x/) { if ($$3 != "obj/flashpatches.o" && $$3 != "obj/wrapper.o") { if (!x[$$1]++) { print $$0; } } else { if (!x[$$1]) { print $$0; } } } else { print $$0; } }' > gen/nexmon2.pre

gen/nexmon.ld: gen/nexmon2.pre $(OBJS)
	@printf "\033[0;31m  GENERATING LINKER FILE\033[0m gen/nexmon.pre => %s\n" $@
	$(Q)sort gen/nexmon2.pre | gawk -f $(NEXMON_ROOT)/buildtools/scripts/nexmon.ld.awk > $@

gen/nexmon.mk: gen/nexmon2.pre $(OBJS) $(FW_PATH)/definitions.mk
	@printf "\033[0;31m  GENERATING MAKE FILE\033[0m gen/nexmon.pre => %s\n" $@
	$(Q)printf "$(RAM_FILE): gen/patch.elf FORCE\n" > $@
	$(Q)sort gen/nexmon2.pre | \
		gawk -v src_file=gen/patch.elf -f $(NEXMON_ROOT)/buildtools/scripts/nexmon.mk.1.awk | \
		gawk -v ramstart=$(RAMSTART) -f $(NEXMON_ROOT)/buildtools/scripts/nexmon.mk.2.awk >> $@
	$(Q)printf "\nFORCE:\n" >> $@
	$(Q)gawk '!a[$$0]++' $@ > tmp && mv tmp $@

gen/flashpatches.ld: gen/nexmon2.pre $(OBJS)
	@printf "\033[0;31m  GENERATING LINKER FILE\033[0m gen/nexmon.pre => %s\n" $@
	$(Q)sort gen/nexmon2.pre | \
		gawk -f $(NEXMON_ROOT)/buildtools/scripts/flashpatches.ld.awk > $@

ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
FLASHPATCHES=flashpatches.bcm4366.mk.awk
else
FLASHPATCHES=flashpatches.mk.awk
endif
gen/flashpatches.mk: gen/nexmon2.pre $(OBJS) $(FW_PATH)/definitions.mk
	@printf "\033[0;31m  GENERATING MAKE FILE\033[0m gen/nexmon.pre => %s\n" $@
	$(Q)cat gen/nexmon2.pre | gawk \
		-v fp_data_base=$(FP_DATA_BASE) \
		-v fp_config_base=$(FP_CONFIG_BASE) \
		-v fp_data_end_ptr=$(FP_DATA_END_PTR) \
		-v fp_config_base_ptr_1=$(FP_CONFIG_BASE_PTR_1) \
		-v fp_config_end_ptr_1=$(FP_CONFIG_END_PTR_1) \
		-v fp_config_base_ptr_2=$(FP_CONFIG_BASE_PTR_2) \
		-v fp_config_end_ptr_2=$(FP_CONFIG_END_PTR_2) \
		-v ramstart=$(RAMSTART) \
		-v out_file=$(RAM_FILE) \
		-v src_file=gen/patch.elf \
		-f $(NEXMON_ROOT)/buildtools/scripts/$(FLASHPATCHES) > $@

gen/memory.ld: $(FW_PATH)/definitions.mk
	@printf "\033[0;31m  GENERATING LINKER FILE\033[0m %s\n" $@
	$(Q)printf "rom : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(ROMSTART) $(ROMSIZE) > $@
	$(Q)printf "ram : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(RAMSTART) $(RAMSIZE) >> $@
	$(Q)printf "patch : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(PATCHSTART) $(PATCHSIZE) >> $@
ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
	$(Q)printf "ucode : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(NONMUUCODESTART) $$(($(FP_CONFIG_BASE) - $(NONMUUCODESTART))) >> $@
else
	$(Q)printf "ucode : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(UCODESTART) $$(($(FP_CONFIG_BASE) - $(UCODESTART))) >> $@
endif
	$(Q)printf "fpconfig : ORIGIN = 0x%08x, LENGTH = 0x%08x\n" $(FP_CONFIG_BASE) $(FP_CONFIG_SIZE) >> $@

gen/patch.elf: patch.ld gen/nexmon.ld gen/flashpatches.ld gen/memory.ld $(OBJS)
	@printf "\033[0;31m  LINKING OBJECTS\033[0m => %s (details: log/linker.log, log/linker.err)\n" $@
	$(Q)$(CC)ld -T $< -o $@ --gc-sections --print-gc-sections -M >>log/linker.log 2>>log/linker.err

$(RAM_FILE): init gen/patch.elf $(FW_PATH)/$(RAM_FILE) gen/nexmon.mk gen/flashpatches.mk
	$(Q)cp $(FW_PATH)/$(RAM_FILE) $@
	@printf "\033[0;31m  APPLYING FLASHPATCHES\033[0m gen/flashpatches.mk => %s (details: log/flashpatches.log)\n" $@
	$(Q)make -f gen/flashpatches.mk >>log/flashpatches.log 2>>log/flashpatches.log
	@printf "\033[0;31m  APPLYING PATCHES\033[0m gen/nexmon.mk => %s (details: log/patches.log)\n" $@
	$(Q)make -f gen/nexmon.mk >>log/patches.log 2>>log/patches.log

###################################################################
# ucode compression related
###################################################################
ifneq ($(filter $(UCODEFILE),$(UCODES)), )
ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
gen/ucode.asm: $(FW_PATH)/nonmuucode.bin
else
gen/ucode.asm: $(FW_PATH)/ucode.bin
endif
	@printf "\033[0;31m  DISASSEMBLING UCODE\033[0m %s => %s\n" $< $@
	$(Q)$(NEXMON_ROOT)/buildtools/$(B43VERSION)/disassembler/b43-dasm $< $@ --arch 15 --format raw-le32 2>log/disass.log
	$(Q)$(NEXMON_ROOT)/buildtools/$(B43VERSION)/debug/b43-beautifier --asmfile $@ --defs $(NEXMON_ROOT)/buildtools/$(B43VERSION)/debug/include > tmp && mv tmp $@

src/$(UCODEFILE:.patch=.asm): src/$(UCODEFILE) gen/ucode.asm
	@printf "\033[0;31m  PATCHING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cp gen/ucode.asm $@
	$(Q)patch -p1 $@ $< >log/patch.log || true

gen/ucode.bin: src/$(UCODEFILE:.patch=.asm)
	@printf "\033[0;31m  ASSEMBLING UCODE\033[0m %s => %s\n" $< $@
ifneq ($(wildcard $(NEXMON_ROOT)/buildtools/$(B43VERSION)/assembler/b43-asm.bin), )
	$(Q)PATH="$(PATH):$(NEXMON_ROOT)/buildtools/$(B43VERSION)/assembler" $(NEXMON_ROOT)/buildtools/$(B43VERSION)/assembler/b43-asm $< $@ --cpp-args -DRXE_RXHDR_LEN=$(RXE_RXHDR_LEN) -- --format raw-le32 2>log/ass.log
else
	$(error Warning: please compile b43-asm.bin first)
endif

ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
gen/nonmuucode.bin: gen/ucode.bin
	@printf "\033[0;31m  COPYING NON MU UCODE\033[0m %s => %s\n" $< $@
	$(Q)cp $< $@

gen/nonmuucodex.bin: $(FW_PATH)/nonmuucodex.bin
	@printf "\033[0;31m  COPYING NON MU UCODE X\033[0m %s => %s\n" $< $@
	$(Q)cp $< $@

gen/muucode.bin: $(FW_PATH)/muucode.bin
	@printf "\033[0;31m  COPYING MU UCODE\033[0m %s => %s\n" $< $@
	$(Q)cp $< $@

gen/muucodex.bin: $(FW_PATH)/muucodex.bin
	@printf "\033[0;31m  COPYING MU UCODE X\033[0m %s => %s\n" $< $@
	$(Q)cp $< $@

gen/nonmuucode_compressed.bin: gen/nonmuucode.bin
	@printf "\033[0;31m  COMPRESSING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cat $< | $(ZLIBFLATE) > $@

gen/nonmuucodex_compressed.bin: gen/nonmuucodex.bin
	@printf "\033[0;31m  COMPRESSING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cat $< | $(ZLIBFLATE) > $@

gen/muucode_compressed.bin: gen/muucode.bin
	@printf "\033[0;31m  COMPRESSING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cat $< | $(ZLIBFLATE) > $@

gen/muucodex_compressed.bin: gen/muucodex.bin
	@printf "\033[0;31m  COMPRESSING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cat $< | $(ZLIBFLATE) > $@

src/nonmuucode_compressed.c: gen/nonmuucode_compressed.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@

src/nonmuucodex_compressed.c: gen/nonmuucodex_compressed.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@

src/muucode_compressed.c: gen/muucode_compressed.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@

src/muucodex_compressed.c: gen/muucodex_compressed.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@
else
gen/ucode_compressed.bin: gen/ucode.bin
	@printf "\033[0;31m  COMPRESSING UCODE\033[0m %s => %s\n" $< $@
	$(Q)cat $< | $(ZLIBFLATE) > $@

src/ucode_compressed.c: gen/ucode_compressed.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@
endif

else
gen/ucode.asm:
ifneq ($(UCODEFILE),)
	@printf "\033[0;31m  WARNING\033[0m uncompatible or non-existing ucodefile \"%s\" selected, use \"make UCODEFILE=<filename>\" where filename one of: %s\n" $(UCODEFILE) $(UCODES)
else
	@printf "\033[0;31m  WARNING\033[0m empty ucodefile specified, use \"make UCODEFILE=<filename>\" where filename one of: %s\n" $(UCODEFILE) $(UCODES)
endif
	$(Q)exit 1
endif

src/templateram.c: $(FW_PATH)/templateram.bin
	@printf "\033[0;31m  GENERATING C FILE\033[0m %s => %s\n" $< $@
	$(Q)printf "#pragma NEXMON targetregion \"ucode\"\n\n" > $@
	$(Q)cd $(dir $<) && xxd -i $(notdir $<) >> $(shell pwd)/$@

###################################################################

check-nexmon-setup-env:
ifndef NEXMON_SETUP_ENV
	$(error run 'source setup_env.sh' first in the repository\'s root directory)
endif

# bcm43455c0
ifneq ($(findstring bcm43455c0,$(FWUCODE)), )
backup-firmware:
ifeq ($(shell uname -m),$(filter $(shell uname -m), armv6l armv7l))
	$(Q)cp /lib/firmware/brcm/brcmfmac43455-sdio.bin brcmfmac43455-sdio.bin.orig
else
	$(warning Warning: Cannot backup the original firmware on this arch.)
endif

install-firmware: brcmfmac43455-sdio.bin brcmfmac.ko
ifeq ($(shell uname -m),$(filter $(shell uname -m), armv6l armv7l))
	@printf "\033[0;31m  COPYING\033[0m brcmfmac43455-sdio.bin => /lib/firmware/brcm/brcmfmac43455-sdio.bin\n"
	$(Q)sudo cp brcmfmac43455-sdio.bin /lib/firmware/brcm/brcmfmac43455-sdio.bin
ifeq ($(shell lsmod | grep "^brcmfmac" | wc -l), 1)
	@printf "\033[0;31m  UNLOADING\033[0m brcmfmac\n"
	$(Q)sudo rmmod brcmfmac
endif
	$(Q)sudo modprobe brcmutil
	@printf "\033[0;31m  RELOADING\033[0m brcmfmac\n"
ifeq ($(findstring 4.19,$(shell uname -r)),4.19)
	$(Q)sudo insmod brcmfmac_4.19.y-nexmon/brcmfmac.ko
else ifeq ($(findstring 5.4,$(shell uname -r)),5.4)
	$(Q)sudo insmod brcmfmac_5.4.y-nexmon/brcmfmac.ko
else ifeq ($(findstring 5.10,$(shell uname -r)),5.10)
	$(Q)sudo insmod brcmfmac_5.10.y-nexmon/brcmfmac.ko
endif
else
	$(warning Warning: Cannot install firmware on this arch., bcm43430-sdio.bin needs to be copied manually into /lib/firmware/brcm/ on your RPI3)
endif

clean-firmware: FORCE
	@printf "\033[0;31m  CLEANING\033[0m\n"
	$(Q)rm -fr $(RAM_FILE) obj gen log src/ucode_compressed.c src/templateram.c src/*.asm

clean: clean-firmware
ifeq ($(shell uname -m),$(filter $(shell uname -m), armv6l armv7l))
ifeq ($(findstring 4.19,$(shell uname -r)),4.19)
	@printf "\033[0;31m  CLEANING DRIVER\033[0m\n" $@
	$(Q)make -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_4.19.y-nexmon clean
else ifeq ($(findstring 5.4,$(shell uname -r)),5.4)
	@printf "\033[0;31m  CLEANING DRIVER\033[0m\n" $@
	$(Q)make -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_5.4.y-nexmon clean
else ifeq ($(findstring 5.10,$(shell uname -r)),5.10)
	@printf "\033[0;31m  CLEANING DRIVER\033[0m\n" $@
	$(Q)make -C /lib/modules/$(shell uname -r)/build M=$$PWD/brcmfmac_5.10.y-nexmon clean
endif
endif
	$(Q)rm -f BUILD_NUMBER
else ifneq ($(findstring bcm4366c0,$(FWUCODE)), )
install-firmware: dhd.ko
ifneq ($(REMOTEADDR),)
	@printf "\033[0;31m  COPYING TO ROUTER\033[0m %s => /jffs/%s\n" $< $<
	$(Q)scp dhd.ko admin@$$REMOTEADDR:/jffs/dhd.ko
	@printf "\033[0;31m  LOADING\033[0m /jffs/dhd.ko\n"
	$(Q)ssh admin@$$REMOTEADDR "/sbin/rmmod dhd; /sbin/insmod /jffs/dhd.ko"
else
	$(error Warning: Cannot install firmware, no remote address given. Run make with REMOTEADDR=<remote address>.)
endif

clean-firmware: FORCE
	@printf "\033[0;31m  CLEANING\033[0m\n"
	$(Q)rm -fr $(RAM_FILE) dhd.ko obj gen log src/nonmuucode_compressed.c src/nonmuucodex_compressed.c src/muucode_compressed.c src/muucodex_compressed.c src/templateram.c src/*.asm

clean: clean-firmware
	$(Q)rm -f BUILD_NUMBER
else
install-firmware: $(RAM_FILE)
	@printf "\033[0;31m  REMOUNTING /system\033[0m\n"
	$(Q)adb $(ADBFLAGS) shell 'su -c "mount -o rw,remount /system"'
	@printf "\033[0;31m  REMOUNTING /vendor\033[0m\n"
	$(Q)adb $(ADBFLAGS) shell 'su -c "mount -o rw,remount /vendor"'
	@printf "\033[0;31m  COPYING TO PHONE\033[0m %s => /sdcard/%s\n" $< $<
	$(Q)adb $(ADBFLAGS) push $< /sdcard/ >> log/adb.log 2>> log/adb.log
	@printf "\033[0;31m  COPYING\033[0m /sdcard/fw_bcmdhd.bin => /vendor/firmware/fw_bcmdhd.bin\n"
	$(Q)adb $(ADBFLAGS) shell 'su -c "rm /vendor/firmware/fw_bcmdhd.bin && cp /sdcard/fw_bcmdhd.bin /vendor/firmware/fw_bcmdhd.bin"'
	@printf "\033[0;31m  RELOADING FIRMWARE\033[0m\n"
	$(Q)adb $(ADBFLAGS) shell 'su -c "ifconfig wlan0 down && ifconfig wlan0 up"'

backup-firmware: FORCE
	adb $(ADBFLAGS) shell 'su -c "cp /vendor/firmware/fw_bcmdhd.bin /sdcard/fw_bcmdhd.orig.bin"'
	adb $(ADBFLAGS) pull /sdcard/fw_bcmdhd.orig.bin

install-backup: fw_bcmdhd.orig.bin
	adb $(ADBFLAGS) shell 'su -c "mount -o rw,remount /system"' && \
	adb $(ADBFLAGS) push $< /sdcard/ && \
	adb $(ADBFLAGS) shell 'su -c "cp /sdcard/fw_bcmdhd.bin /vendor/firmware/fw_bcmdhd.bin"'
	adb $(ADBFLAGS) shell 'su -c "ifconfig wlan0 down && ifconfig wlan0 up"'

clean-firmware: FORCE
	@printf "\033[0;31m  CLEANING\033[0m\n"
	$(Q)rm -fr fw_bcmdhd.bin obj gen log src/ucode_compressed.c src/templateram.c src/*.asm

clean: clean-firmware
	$(Q)rm -f BUILD_NUMBER
endif

FORCE:
