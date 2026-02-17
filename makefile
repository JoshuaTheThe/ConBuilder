override MAKEFLAGS += -rR

override OUTPUT := nqo

define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

override DEFAULT_KCC := gcc
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))

override DEFAULT_KLD := gcc
$(eval $(call DEFAULT_VAR,KLD,$(DEFAULT_KLD)))

override DEFAULT_KCFLAGS := -Wall -Wextra -Wpedantic
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

override DEFAULT_KNASMFLAGS := 
$(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))

override DEFAULT_KLDFLAGS := -lm
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

override CFLAGS := \
    -Wall \
    -Wextra \
    -m64 \
    -march=native

override CXXFLAGS := \
    -Wall \
    -Wextra \
    -std=c++20 \
    -m64 \
    -march=native

override KCPPFLAGS := \
    -I ./src \
    $(KCPPFLAGS)

override KNASMFLAGS += \
    -Wall \
    -f elf64

override KLDFLAGS += \
    -m64 \
    -lgcc

override CFILES := $(shell find src -type f -name '*.c' | LC_ALL=C sort)
override CPPFILES := $(shell find src -type f -name '*.cpp' | LC_ALL=C sort)
override ASFILES := $(shell find src -type f -name '*.S' | LC_ALL=C sort)
override NASMFILES := $(shell find src -type f -name '*.asm' | LC_ALL=C sort)

# FIXED: Preserve directory structure in object files
override OBJ := $(CFILES:src/%.c=obj/%.c.o) \
                $(CPPFILES:src/%.cpp=obj/%.cpp.o) \
                $(ASFILES:src/%.S=obj/%.S.o) \
                $(NASMFILES:src/%.asm=obj/%.asm.o)

override HEADER_DEPS := $(CFILES:src/%.c=obj/%.c.d) \
                        $(CPPFILES:src/%.cpp=obj/%.cpp.d) \
                        $(ASFILES:src/%.S=obj/%.S.d)

.PHONY: all
all: bin/$(OUTPUT)

bin/$(OUTPUT): $(OBJ)
	@mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@

-include $(HEADER_DEPS)

obj/%.c.o: src/%.c
	@mkdir -p "$$(dirname $@)"
	$(KCC) $(CFLAGS) $(KCPPFLAGS) -MMD -c $< -o $@

obj/%.cpp.o: src/%.cpp
	@mkdir -p "$$(dirname $@)"
	$(KCC) $(CXXFLAGS) $(KCPPFLAGS) -MMD -c $< -o $@

obj/%.S.o: src/%.S
	@mkdir -p "$$(dirname $@)"
	$(KCC) $(CFLAGS) $(KCPPFLAGS) -c $< -o $@

obj/%.asm.o: src/%.asm
	@mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf bin obj

.PHONY: run
run: all
	./bin/$(OUTPUT)

.PHONY: debug
debug: CXXFLAGS += -O0 -ggdb3
debug: all