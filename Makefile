### Compiler Options
CC=mpicc
CFLAGS=-c -Wall
# -lGL -lglut -lGLU# < extra libraries and paths >
LDFLAGS= -lGL -lglut -lGLU
SOURCES = envsim.c global.h global.c mpi_system.h mpi_system.c display.h display.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE = envsim

### Runtime Options
MPIEXEC=mpiexec
MACHINEFILE=cluster.machines

# Main build rule
all: $(SOURCES) $(EXECUTABLE)
	
# Builds executable
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Builds Objects
.c.o:
	$(CC) $(CFLAGS) $< -o $@

# Removes objects and executable
clean:
	rm -rf *o $(EXECUTABLE)

# Runs application with supplied machine file
run:
	$(MPIEXEC) -f $(MACHINEFILE) ./$(EXECUTABLE)
