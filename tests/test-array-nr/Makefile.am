BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c parser.c parser.h test-array-nr
OBJS = scanner.o # parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)/tests
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-array-nr

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	$(srcdir)/$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
