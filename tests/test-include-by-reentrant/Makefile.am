BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test-1.input test-2.input test-3.input
CLEANFILES = scanner.c scanner.h parser.c parser.h test-include-by-reentrant OUTPUT $(OBJS)
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir) -I .
#LDFLAGS = $(top_srcdir)/libfl.a 
#LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = test-include-by-reentrant

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	./$(testname) $(srcdir)/test-1.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
