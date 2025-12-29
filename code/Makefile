SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = inc

TARGET = $(BINDIR)/unipi_main
OBJS   = $(OBJDIR)/main.o $(OBJDIR)/digital.o $(OBJDIR)/relay.o $(OBJDIR)/config.o $(OBJDIR)/tcp_server.o $(OBJDIR)/device.o

XML2_CFLAGS = $(shell xml2-config --cflags)
XML2_LIBS = $(shell xml2-config --libs)

CFLAGS = -Wall -I$(INCDIR) $(XML2_CFLAGS)
LDFLAGS = $(XML2_LIBS)

$(TARGET) : $(OBJS)
	@mkdir -p $(BINDIR)
	gcc $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	gcc -c -MD $(CFLAGS) $< -o $@

-include $(OBJDIR)/*.d

.PHONY: clean
clean:
	@rm -r $(OBJDIR) $(BINDIR)
