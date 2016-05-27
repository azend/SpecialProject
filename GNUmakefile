
# Toolchain configuration
CC=g++
CFLAGS=-g -Wall #-DUNIT_TESTING
GDB=gdb
GDBFLAGS=-tui -x gdb_cmds

# Compilation configuration
EXEC=main
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

# Link final executable
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

# Compile object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean debug

# Clean built files
clean:
	rm -f $(EXEC) $(OBJECTS)

debug: $(EXEC)
	$(GDB) $(GDBFLAGS) $(EXEC)

todo:
	grep -R "@todo" .
