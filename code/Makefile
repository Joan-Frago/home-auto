CC = gcc

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = inc

TARGET = $(BINDIR)/unipi_main
OBJS   = $(OBJDIR)/main.o $(OBJDIR)/digital.o $(OBJDIR)/relay.o $(OBJDIR)/config.o $(OBJDIR)/tcp_server.o $(OBJDIR)/device.o $(OBJDIR)/util.o $(OBJDIR)/device_xml.o $(OBJDIR)/logger.o $(OBJDIR)/loggerconf.o $(OBJDIR)/historify.o

XML2_CFLAGS = $(shell xml2-config --cflags)
XML2_LIBS = $(shell xml2-config --libs)

CFLAGS = -Wall -I$(INCDIR) $(XML2_CFLAGS)
LDFLAGS = $(XML2_LIBS)

native: CC = gcc
native: $(TARGET)
	@echo "\n --> Compiled for x86-64"

arm: CC = arm-linux-gnueabihf-gcc
arm: $(TARGET)
	@echo "\n --> Compiled for ARM"

$(TARGET) : $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) -c -MD $(CFLAGS) $< -o $@

-include $(OBJDIR)/*.d

.PHONY: clean
clean:
	@rm -r $(OBJDIR) $(BINDIR)
