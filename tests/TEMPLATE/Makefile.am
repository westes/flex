BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l parser.y test.input
CLEANFILES = scanner.c scanner.h parser.c parser.h TEMPLATE OUTPUT $(OBJS)
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir) -I .
#LDFLAGS = $(top_srcdir)/libfl.a 
#LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = TEMPLATE

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
