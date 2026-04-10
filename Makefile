# ==============================================================================
#  MyPLC Makefile
#  Works from: PowerShell / cmd.exe (Windows) · bash (Linux, Raspberry Pi)
#
#  Targets:
#    make                  → build runtime.exe  (full PLC + web HMI + Modbus server)
#    make run              → build and run  (open http://localhost:8080)
#    make hmi              → build runtime_hmi.exe  (web HMI + Modbus client for RPi)
#    make hmi_run          → build and run the HMI binary
#    make sample S=01_TON  → build and run a standalone sample
#    make samples          → list available samples
#    make clean            → remove build artefacts
# ==============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -I. -Wall -Wextra
OBJDIR   := obj
TARGET   := runtime.exe
HMI_TGT  := runtime_hmi.exe

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

# Full simulator: HTTP + Modbus server + user program
SIM_SRCS := sim/registry.cpp sim/server.cpp sim/modbus_server.cpp
RT_SRCS  := runtime/main.cpp user/program.cpp
ALL_SRCS := $(PLC_SRCS) $(SIM_SRCS) $(RT_SRCS)

# HMI mode: HTTP + Modbus client (no PLC loop, no Modbus server)
HMI_SRCS := \
    $(PLC_SRCS) \
    sim/registry.cpp \
    sim/server.cpp \
    sim/modbus_client.cpp \
    runtime/hmi_main.cpp \
    user/program.cpp

# Flatten path → single object name:  plc/timers/TON.cpp → obj/plc_timers_TON.o
obj_of   = $(OBJDIR)/$(subst /,_,$(patsubst %.cpp,%.o,$(1)))
ALL_OBJS := $(foreach s,$(ALL_SRCS),$(call obj_of,$(s)))

# ── Platform detection ────────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
    SHELL   := cmd
    MKDIR   := if not exist $(OBJDIR) md $(OBJDIR)
    RM      := (if exist $(OBJDIR) rmdir /s /q $(OBJDIR)) & \
               (if exist $(TARGET) del /q $(TARGET)) & \
               (if exist $(HMI_TGT) del /q $(HMI_TGT)) & \
               (del /q sample_*.exe 2>nul & exit 0)
    RUN     := $(TARGET)
    HMI_RUN := $(HMI_TGT)
    NL      := @echo.
    # Static-link runtimes: no MSYS2 DLL dependency, avoids Defender blocking
    LDFLAGS := -static-libgcc -static-libstdc++ -lws2_32
else
    MKDIR   := mkdir -p $(OBJDIR)
    RM      := rm -rf $(OBJDIR) $(TARGET) $(HMI_TGT) sample_*.exe
    RUN     := ./$(TARGET)
    HMI_RUN := ./$(HMI_TGT)
    NL      := @echo
    LDFLAGS :=
endif

# ── Main targets ──────────────────────────────────────────────────────────────
.PHONY: all run hmi hmi_run sample samples clean

all: $(TARGET)

$(TARGET): $(OBJDIR) $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(ALL_OBJS) $(LDFLAGS)
	$(NL)
	@echo   Build OK — run 'make run' then open http://localhost:8080
	$(NL)

run: $(TARGET)
	$(RUN)

# ── HMI target (Raspberry Pi — Modbus client + web dashboard) ─────────────────
# Compiled in one shot with -DHMI_MODE so INIT()/LOOP() in user/program.cpp
# are excluded while PLC_VAR declarations (and thus Modbus addresses) still run.
hmi: $(OBJDIR)
	$(CXX) $(CXXFLAGS) -DHMI_MODE -o $(HMI_TGT) $(HMI_SRCS) $(LDFLAGS)
	$(NL)
	@echo   HMI build OK — run 'make hmi_run' then open http://localhost:8080
	$(NL)

hmi_run: hmi
	$(HMI_RUN)

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
#  • make / make run       → PC simulation (PLC + web HMI + Modbus server on port 502)
#  • make hmi / make hmi_run → RPi HMI mode (web dashboard + Modbus client → ESP32)
#  • make sample S=NN_NAME → standalone sample (no sim/ or runtime/ deps)
#  • Debug build: change -O2 to -g -O0 in CXXFLAGS
#  • Windows links -lws2_32 (WinSock2) for all socket-based components
#  • Port 502 requires root on Linux — use sudo ./runtime.exe or change MODBUS_PORT
# ==============================================================================
