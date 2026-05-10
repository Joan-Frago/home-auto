CC = gcc

SRCDIR = src
INCDIR = inc
OBJDIR = obj
BINDIR = bin

TARGET = $(BINDIR)/unipi_main
OBJS   = $(OBJDIR)/main.o \
		 $(OBJDIR)/digital.o \
		 $(OBJDIR)/relay.o \
		 $(OBJDIR)/modbus.o \
		 $(OBJDIR)/config.o \
		 $(OBJDIR)/tcp_server.o \
		 $(OBJDIR)/device.o \
		 $(OBJDIR)/util.o \
		 $(OBJDIR)/device_xml.o \
		 $(OBJDIR)/logger.o \
		 $(OBJDIR)/loggerconf.o \
		 $(OBJDIR)/historify.o \
		 $(OBJDIR)/fire.o \
		 $(OBJDIR)/db.o \
		 $(OBJDIR)/sb.o

XML2_CFLAGS = $(shell xml2-config --cflags)
XML2_LIBS = $(shell xml2-config --libs)

LIBMODBUS_CFLAGS = $(shell pkg-config --cflags libmodbus)
LIBMODBUS_LIBS = $(shell pkg-config --libs libmodbus)

POSTGRES_INCDIR = $(shell pg_config --includedir)
POSTGRES_LIBDIR = $(shell pg_config --libdir)
POSTGRES_CFLAGS = -I$(POSTGRES_INCDIR) -L$(POSTGRES_LIBDIR)
POSTGRES_LIBS   = -lpq

CFLAGS = -Wall -Wextra -ggdb -I$(INCDIR) $(XML2_CFLAGS) $(LIBMODBUS_CFLAGS) $(POSTGRES_CFLAGS)
LDFLAGS = $(XML2_LIBS) $(LIBMODBUS_LIBS) $(POSTGRES_LIBS)

native: CC = gcc
native: $(TARGET)
	@echo " --> Compiled for x86-64"

arm: CC = arm-linux-gnueabihf-gcc
arm: $(TARGET)
	@echo " --> Compiled for ARM"

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
