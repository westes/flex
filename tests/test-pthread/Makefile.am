BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test-1.input test-2.input test-3.input test-4.input test-5.input
CLEANFILES = scanner.c scanner.h parser.c parser.h test-pthread OUTPUT $(OBJS)
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir)
LDFLAGS = -lpthread
#LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = test-pthread

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	./$(testname) $(srcdir)/test-*.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
