TARGET = rpn

CPPFLAGS += -I../../../osbook/devenv/x86_64-elf/include/c++/v1 \
			-I../../../osbook/devenv/x86_64-elf/include \
			-I../../../osbook/devenv/x86_64-elf/include/freetype2 \
			-I../../../edk2/MdePkg/Include \
			-I../../../edk2/MdePkg/Include/X64 \
			-I. -nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE \
			-D_POSIX_TIMERS -DEFIAPI='__attribute__((ms_abi))'
CFLAGS   += -O2 -Wall -g --target=x86_64-elf -ffreestanding
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding \
			-fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += -L../../../osbook/devenv/x86_64-elf/lib \
			--entry main -z norelro --image-base 0 --static

.PHONY: all
all: $(TARGET)

rpn: rpn.o Makefile
	ld.mold $(LDFLAGS) -o rpn rpn.o

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
