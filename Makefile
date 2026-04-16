# ==============================================================================
#  MyPLC Makefile
#  Works from: PowerShell / cmd.exe (Windows) · bash (Linux, Raspberry Pi)
#
#  Targets:
#    make                  → build runtime.exe
#    make run              → build + start runtime (non-blocking, terminal stays free)
#    make flash            → flash ESP32 firmware via PlatformIO
#    make sample S=01_TON  → build and run a standalone sample
#    make samples          → list available samples
#    make clean            → remove build artefacts
# ==============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -I. -Wall -Wextra
OBJDIR   := obj
TARGET   := runtime.exe

# ── Source files ──────────────────────────────────────────────────────────────
PLC_SRCS := \
    plc/timers/TON.cpp      \
    plc/timers/TOF.cpp      \
    plc/timers/TP.cpp       \
    plc/triggers/R_TRIG.cpp \
    plc/triggers/F_TRIG.cpp \
    plc/counters/CTU.cpp    \
    plc/counters/CTD.cpp    \
    plc/counters/CTUD.cpp   \
    plc/bistables/SR.cpp    \
    plc/bistables/RS.cpp

SIM_SRCS    := sim/registry.cpp sim/server.cpp
MODBUS_SRCS := modbus/server.cpp
RT_SRCS     := runtime/main.cpp user/program.cpp
ALL_SRCS    := $(PLC_SRCS) $(SIM_SRCS) $(MODBUS_SRCS) $(RT_SRCS)

# Flatten path → single object name:  plc/timers/TON.cpp → obj/plc_timers_TON.o
obj_of   = $(OBJDIR)/$(subst /,_,$(patsubst %.cpp,%.o,$(1)))
ALL_OBJS := $(foreach s,$(ALL_SRCS),$(call obj_of,$(s)))

# ── Platform detection ────────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
    # MSYS2 UCRT64 — POSIX tools available (mkdir, rm), no cmd.exe needed
    MKDIR   := mkdir -p $(OBJDIR)
    RM      := rm -rf $(OBJDIR) $(TARGET) sample_*.exe
    # Kill running instance before link (Windows locks the exe)
    KILL    := taskkill //F //IM $(TARGET) 2>/dev/null; true
    # Run in background — terminal stays free, output visible in same window
    RUN     := ./$(TARGET) &
    # Static-link runtimes: no MSYS2 DLL dependency, avoids Defender blocking
    LDFLAGS := -static-libgcc -static-libstdc++ -lws2_32
else
    # Linux / Raspberry Pi
    MKDIR   := mkdir -p $(OBJDIR)
    RM      := rm -rf $(OBJDIR) $(TARGET) sample_*.exe
    KILL    := pkill -f $(TARGET) 2>/dev/null; true
    RUN     := ./$(TARGET) &
    LDFLAGS :=
endif

# ── Main targets ──────────────────────────────────────────────────────────────
.PHONY: all run flash sample samples clean

all: $(TARGET)

$(TARGET): $(OBJDIR) $(ALL_OBJS)
	@$(KILL)
	$(CXX) $(CXXFLAGS) -o $@ $(ALL_OBJS) $(LDFLAGS)
	@echo "Build OK — run 'make run' then open http://localhost:8080"

run: $(TARGET)
	$(RUN)
	@echo "Runtime started — dashboard : http://localhost:8080"
	@echo "Modbus TCP      — port 502   (Linux: sudo make run)"

flash:
	cd firmware && pio run -t upload

# ── Object directory ──────────────────────────────────────────────────────────
$(OBJDIR):
	@$(MKDIR)

# ── Compilation rules (generated for every source file) ───────────────────────
define COMPILE_RULE
$(call obj_of,$(1)): $(1)
	$(CXX) $(CXXFLAGS) -c $$< -o $$@
endef
$(foreach s,$(ALL_SRCS),$(eval $(call COMPILE_RULE,$(s))))

# ── Standalone samples ────────────────────────────────────────────────────────
# Usage:  make sample S=01_TON
S          ?= 01_TON
SAMPLE_BIN := sample_$(S).exe
SAMPLE_SRC := samples/$(S)/main.cpp

sample: $(SAMPLE_BIN)
	./$(SAMPLE_BIN)

$(SAMPLE_BIN): $(SAMPLE_SRC) $(PLC_SRCS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SAMPLE_SRC) $(PLC_SRCS) $(LDFLAGS)

samples:
	@echo "Available samples  (usage: make sample S=NAME)"
	@echo "  01_TON            Timer On Delay"
	@echo "  02_TOF            Timer Off Delay"
	@echo "  03_TP             Timer Pulse"
	@echo "  04_CTU            Count Up Counter"
	@echo "  05_CTD            Count Down Counter"
	@echo "  06_CTUD           Count Up/Down Counter"
	@echo "  07_R_TRIG_F_TRIG  Rising and Falling Edge Detectors"
	@echo "  08_SR_RS          SR and RS Bistable Latches"

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	@$(RM)

# ==============================================================================
#  Notes
#  ─────
#  • Debug build: change -O2 to -g -O0 in CXXFLAGS
#  • Modbus server (port 502) needs root on Linux: sudo make run
#  • ESP32 remote I/O: edit firmware/src/io_map.h, then run 'make flash'
#    The ESP32 connects to the RPi as a Modbus TCP Client (no code in program.cpp)
#  • Windows links -lws2_32 (WinSock2) for HTTP + Modbus
#  • Samples are standalone — they do not depend on sim/ or runtime/
# ==============================================================================
