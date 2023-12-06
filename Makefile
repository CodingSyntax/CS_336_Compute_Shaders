
CC = gcc
CXX = g++
ECHO = echo
RM = del

CXXFLAGS = -Wall -ggdb
CFLAGS = -Iinclude

LDFLAGS = -Llibs -lOpenGL32 -lglfw3dll -lm

BIN = Program
FILES = main glad util

OBJS = $(patsubst %, out/%.o, $(FILES))
PROD = $(patsubst %, out/%.prod, $(FILES))

all: $(BIN)
#bin/glfw3.dll

#bin/glfw3.dll:
#	$(shell cp libs/glfw3.dll $@)
#   cp libs/glfw3.dll $@

$(BIN): $(OBJS)
	@$(ECHO) Linking $@
	$(CXX) $^ -o bin/$@.exe $(LDFLAGS)

prod: $(PROD)
	@$(ECHO) Linking $@
	$(CXX) $^ -o bin/$(BIN)_v$(VERSION).exe $(LDFLAGS)

out/%.o: src/%.cpp
	@$(ECHO) Compiling $<
	$(CXX) $(CXXFLAGS) $(CFLAGS) $< -c -o $@

out/%.o: src/%.c
	@$(ECHO) Compiling $<
	$(CC) $(CXXFLAGS) $(CFLAGS) $< -c -o $@

out/%.to: src/%.tpp
	@$(ECHO) Compiling $<
	$(CC) $(CXXFLAGS) $(CFLAGS) $< -c -o $@	

out/%.prod: src/%.cpp
	@$(ECHO) Compiling $<
	$(CXX) $< -c -o $@

clean:
	@$(ECHO) Removing all generated files
	@$(RM) out
	@$(RM) bin/$(BIN).exe