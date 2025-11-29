SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = inc

TARGET = $(BINDIR)/unipi_main
OBJS   = $(OBJDIR)/main.o $(OBJDIR)/digital.o $(OBJDIR)/relay.o $(OBJDIR)/config.o

CFLAGS = -Wall -I$(INCDIR)

$(TARGET) : $(OBJS)
	@mkdir -p $(BINDIR)
	gcc $(CFLAGS) $(OBJS) -o $(TARGET)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	gcc -c -MD $(CFLAGS) $< -o $@

-include $(OBJDIR)/*.d

.PHONY: clean
clean:
	@rm -r $(OBJDIR) $(BINDIR)
