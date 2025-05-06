# Compiler and flags
CC = gcc
CFLAGS = -g -Wall

# Target name
TARGET = smash

# Object files
OBJS = smash.o commands.o signals.o jobs.o

# Default rule: Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rules for building object files
smash.o: smash.c commands.h signals.h jobs.h
	$(CC) $(CFLAGS) -c smash.c -o smash.o

commands.o: commands.c commands.h jobs.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o

signals.o: signals.c signals.h
	$(CC) $(CFLAGS) -c signals.c -o signals.o

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c -o jobs.o

# Clean rule
clean:
	rm -f $(TARGET) $(OBJS) *~ core.*
