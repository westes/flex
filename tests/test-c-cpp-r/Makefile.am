BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l test.input
CLEANFILES = scanner.c parser.c parser.h test-c-cpp-r $(OBJS) OUTPUT
OBJS = scanner.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-c-cpp-r

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	$(srcdir)/$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
