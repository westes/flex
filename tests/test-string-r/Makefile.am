BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l
CLEANFILES = scanner.c scanner.h parser.c parser.h test-string-r OUTPUT $(OBJS)
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#LFLAGS = --header="scanner.h"
#YFLAGS = --defines --output=parser.c

testname = test-string-r

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	$(srcdir)/$(testname)$(EXEEXT)

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
