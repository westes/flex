BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l parser.y test.input
OBJS = $(srcdir)/scanner.o # $(srcdir)/parser.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)/tests
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-c-cpp-nr

scanner.c: $(srcdir)/scanner.l
	$(FLEX) $<

parser.c: $(srcdir)/parser.y
	$(BISON) $(YFLAGS) $<

$(testname)$(EXEEXT): $(OBJS)
	$(CXX) -o $(testname)$(EXEEXT) $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	$(srcdir)/$(testname)$(EXEEXT) < $(srcdir)/test.input

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<
