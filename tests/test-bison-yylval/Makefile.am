BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner.l parser.y test.input main.c
CLEANFILES = scanner.c parser.c parser.h test-bison-yylval $(OBJS)
OBJS = parser.o scanner.o main.o

INCLUDES = -I $(srcdir) -I $(top_srcdir)/tests
#LDFLAGS = $(top_srcdir)/libfl.a 
YFLAGS = --defines --output=parser.c --name-prefix="test"

testname = test-bison-yylval

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

parser.h: parser.c
main.o: parser.h
