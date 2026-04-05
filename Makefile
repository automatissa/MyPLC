# ==============================================================================
#  MyPLC Makefile
#  Works from: PowerShell / cmd.exe (Windows) · bash (Linux, Raspberry Pi)
#
#  Targets:
#    make                  → build runtime.exe
#    make run              → build and run  (open http://localhost:8080)
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

SIM_SRCS := sim/registry.cpp sim/server.cpp
RT_SRCS  := runtime/main.cpp user/program.cpp
ALL_SRCS := $(PLC_SRCS) $(SIM_SRCS) $(RT_SRCS)

# Flatten path → single object name:  plc/timers/TON.cpp → obj/plc_timers_TON.o
obj_of   = $(OBJDIR)/$(subst /,_,$(patsubst %.cpp,%.o,$(1)))
ALL_OBJS := $(foreach s,$(ALL_SRCS),$(call obj_of,$(s)))

# ── Platform detection ────────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
    # Force cmd.exe so recipes work from PowerShell, cmd, and MSYS2 alike.
    SHELL   := cmd
    MKDIR   := if not exist $(OBJDIR) md $(OBJDIR)
    RM      := (if exist $(OBJDIR) rmdir /s /q $(OBJDIR)) & \
               (if exist $(TARGET) del /q $(TARGET)) & \
               (del /q sample_*.exe 2>nul & exit 0)
    RUN     := $(TARGET)
    NL      := @echo.
    # Static-link runtimes: no MSYS2 DLL dependency, avoids Defender blocking
    LDFLAGS := -static-libgcc -static-libstdc++ -lws2_32
else
    # Linux / Raspberry Pi
    MKDIR   := mkdir -p $(OBJDIR)
    RM      := rm -rf $(OBJDIR) $(TARGET) sample_*.exe
    RUN     := ./$(TARGET)
    NL      := @echo
    LDFLAGS :=
endif

# ── Main targets ──────────────────────────────────────────────────────────────
.PHONY: all run sample samples clean

all: $(TARGET)

$(TARGET): $(OBJDIR) $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(ALL_OBJS) $(LDFLAGS)
	$(NL)
	@echo   Build OK — run 'make run' then open http://localhost:8080
	$(NL)

run: $(TARGET)
	$(RUN)

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
	$(if $(filter $(OS),Windows_NT),$(SAMPLE_BIN),./$(SAMPLE_BIN))

$(SAMPLE_BIN): $(SAMPLE_SRC) $(PLC_SRCS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SAMPLE_SRC) $(PLC_SRCS) $(LDFLAGS)

samples:
	$(NL)
	@echo   Available samples  ^(usage: make sample S=NAME^)
	$(NL)
	@echo     01_TON            Timer On Delay
	@echo     02_TOF            Timer Off Delay
	@echo     03_TP             Timer Pulse
	@echo     04_CTU            Count Up Counter
	@echo     05_CTD            Count Down Counter
	@echo     06_CTUD           Count Up/Down Counter
	@echo     07_R_TRIG_F_TRIG  Rising and Falling Edge Detectors
	@echo     08_SR_RS          SR and RS Bistable Latches
	$(NL)

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	@$(RM)

# ==============================================================================
#  Notes
#  ─────
#  • Debug build: change -O2 to -g -O0 in CXXFLAGS
#  • Windows links -lws2_32 (WinSock2) for the HTTP simulator server
#  • Samples are standalone — they do not depend on sim/ or runtime/
# ==============================================================================
