# Cross-platform Makefile (Linux + Windows MSYS/MinGW + cmd.exe)
# Builds runtime.exe from sources in lib/ and user/src/
# Object files are placed in ./obj, which is deleted by make clean

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Ilib -Iuser/include -Wall -Wextra

OBJDIR := obj
SRCDIR_LIB := lib
SRCDIR_USER := user/src

LIB_SRCS := $(wildcard $(SRCDIR_LIB)/*.cpp)
LIB_OBJS := $(patsubst $(SRCDIR_LIB)/%.cpp,$(OBJDIR)/lib_%.o,$(LIB_SRCS))
USER_SRCS := $(wildcard $(SRCDIR_USER)/*.cpp)
USER_OBJS := $(patsubst $(SRCDIR_USER)/%.cpp,$(OBJDIR)/user_%.o,$(USER_SRCS))

TARGET := runtime.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJDIR) $(LIB_OBJS) $(USER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(LIB_OBJS) $(USER_OBJS)

# Cross-platform mkdir
ifeq ($(OS),Windows_NT)
MKDIR = if not exist $(OBJDIR) mkdir $(OBJDIR)
RM = rmdir /s /q $(OBJDIR) 2>nul & del /q $(TARGET) 2>nul || exit 0
else
MKDIR = mkdir -p $(OBJDIR)
RM = rm -rf $(OBJDIR) $(TARGET)
endif

$(OBJDIR):
	@$(MKDIR)

# Compile library sources
$(OBJDIR)/lib_%.o: $(SRCDIR_LIB)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile user sources
$(OBJDIR)/user_%.o: $(SRCDIR_USER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	@./$(TARGET)

clean:
	@$(RM)

# Notes:
# - Works on Linux, macOS, and Windows (MSYS2/MinGW or cmd.exe)
# - For debugging: set CXXFLAGS := -std=c++17 -g -O0 -Ilib -Iuser/include -Wall -Wextra


# Thanks to AI for this Makefile :)
