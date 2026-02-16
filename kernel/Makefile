TARGET = kernel.elf
OBJS = main.o graphics.o font.o hankaku.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))

CXXFLAGS += -I/home/tagawa/mikan/osbook/devenv/x86_64-elf/include/c++/v1 \
			-I/home/tagawa/mikan/osbook/devenv/x86_64-elf/include \
			-I/home/tagawa/mikan/osbook/devenv/x86_64-elf/include/freetype2 \
			-I/home/tagawa/mikan/edk2/MdePkg/Include \
			-I/home/tagawa/mikan/edk2/MdePkg/Include/X64 \
			-nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE \
			-D_POSIX_TIMERS -DEFIAPI='__attribute__((ms_abi))' \
			-O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
			-fno-exceptions -fno-rtti -std=c++17
LDFLAGS += -L/home/tagawa/mikan/osbook/devenv/x86_64-elf/lib \
		   --entry KernelMain -z norelro --image-base 0x100000 --static

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o

kernel.elf: $(OBJS) Makefile
	ld.mold $(LDFLAGS) -o kernel.elf $(OBJS)

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<

.%.d: %.cpp
	clang++ $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

hankaku.bin: hankaku.txt
	../tools/makefont.py -o $@ $<

hankaku.o: hankaku.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

.%.d: %.bin
	touch $@

.PHONY: depends
depends:
	$(MAKE) $(DEPENDS)

-include $(DEPENDS)
