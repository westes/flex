BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c test-lineno-nr OUTPUT $(OBJS)
OBJS = scanner.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir)

testname = test-lineno-nr

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $(LFLAGS) $<


$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	test `./$(testname)$(EXEEXT)   < $(srcdir)/test.input` -eq \
	     `./$(testname)$(EXEEXT) 1 < $(srcdir)/test.input` || exit 1

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
