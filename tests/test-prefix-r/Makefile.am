BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c scanner.h parser.c parser.h test-prefix-r OUTPUT $(OBJS)
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = test-prefix-r

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	./$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
