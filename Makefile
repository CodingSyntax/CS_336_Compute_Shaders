
CC = gcc
CXX = g++
ECHO = echo
RM = del

CXXFLAGS = -Wall -ggdb
CFLAGS = -Iinclude

LDFLAGS = -Llibs -lOpenGL32 -lglfw3 -lm -lgdi32

BIN = Program
FILES = main glad util
EMBED = shaders/fragment shaders/vertex

OBJS = $(patsubst %, out/%.o, $(FILES))
PROD = $(patsubst %, out/%.prod, $(FILES))
EMBEDED = $(patsubst %, out/%.o, $(EMBED))

all: $(BIN)
#bin/glfw3.dll

#bin/glfw3.dll:
#	$(shell cp libs/glfw3.dll $@)
#   cp libs/glfw3.dll $@

$(BIN): $(OBJS) $(EMBEDED)
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

out/%.o: src/%.frag
	@$(ECHO) Converting $<
	ld -r -b binary -o $@ $<

out/%.o: src/%.vert
	@$(ECHO) Converting $<
	ld -r -b binary -o $@ $<

out/%.prod: src/%.cpp
	@$(ECHO) Compiling $<
	$(CXX) $< -c -o $@

clean:
	@$(ECHO) Removing all generated files
	@-$(RM) out
	@-$(RM) bin/$(BIN).exe
	-mkdir out\shaders
	-mkdir bin
    