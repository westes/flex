BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l parser.y test.input
CLEANFILES = scanner.c scanner.h test-header-r OUTPUT $(OBJS)
OBJS = scanner.o main.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)
#LDFLAGS = $(top_srcdir)/libfl.a 
LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = test-header-r

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	$(srcdir)/$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<

scanner.h: scanner.c
main.o: scanner.h
