# Compiler and flags
CC = gcc
CFLAGS = -g -Wall

# Target name
TARGET = smash.exe

# Source and object files directory
SRCDIR = 1.Smash
OBJDIR = 1.Smash

# Object files
OBJS = $(OBJDIR)/smash.o $(OBJDIR)/commands.o $(OBJDIR)/signals.o $(OBJDIR)/jobs.o

# Default rule: Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rules for building object files
$(OBJDIR)/smash.o: $(SRCDIR)/smash.c $(SRCDIR)/commands.h $(SRCDIR)/signals.h $(SRCDIR)/jobs.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/smash.c -o $(OBJDIR)/smash.o

$(OBJDIR)/commands.o: $(SRCDIR)/commands.c $(SRCDIR)/commands.h $(SRCDIR)/jobs.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/commands.c -o $(OBJDIR)/commands.o

$(OBJDIR)/signals.o: $(SRCDIR)/signals.c $(SRCDIR)/signals.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/signals.c -o $(OBJDIR)/signals.o

$(OBJDIR)/jobs.o: $(SRCDIR)/jobs.c $(SRCDIR)/jobs.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/jobs.c -o $(OBJDIR)/jobs.o

# Cleaning old files before a new build
clean:
	rm -f $(TARGET) $(OBJS) *~ core.*
