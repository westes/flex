BISON = @BISON@
FLEX = $(top_builddir)/flex

EXTRA_DIST = scanner-1.l scanner-2.l main.c
CLEANFILES = scanner-1.c scanner-1.h test-multiple-scanners-r OUTPUT $(OBJS) scanner-2.c scanner-2.h
OBJS = scanner-1.o scanner-2.o main.o

INCLUDES = -I $(srcdir) -I $(top_srcdir) -I $(top_builddir)
#LDFLAGS = $(top_srcdir)/libfl.a 
#YFLAGS = --defines --output=parser.c

testname = test-multiple-scanners-r

scanner-1.c: $(srcdir)/scanner-1.l
	$(FLEX) $(LFLAGS) --header=scanner-1.h $<

scanner-2.c: $(srcdir)/scanner-2.l
	$(FLEX) $(LFLAGS) --header=scanner-2.h $<

$(testname)$(EXEEXT): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(testname)$(EXEEXT)
	./$(testname)$(EXEEXT)

.c.o:
	$(CC) -c -o $@ $(INCLUDES) $<

main.o: scanner-1.h scanner-2.h
scanner-1.h: scanner-1.c
scanner-2.h: scanner-2.c
