BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l  test.input main.c
CLEANFILES = scanner.c scanner.h test-header-nr $(OBJS) OUTPUT
OBJS = scanner.o main.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-header-nr

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

scanner.h: scanner.c
main.o: scanner.h
