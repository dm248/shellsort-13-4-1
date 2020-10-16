HDR:= sort.h util.h Counter.h PermTable.h

EXE:= \
      z_histogram solver \
      find_cand

all: $(EXE)

clean:
	rm -f $(EXE)

%: %.C  $(HDR)
	g++ -o $@ -Wall -O3 -march=native --std=c++1y -pthread $<


%-prof: %.C
	g++ -o $@ -Wall -O3 -march=native -DPROF -fprofile-generate -ftest-coverage --std=c++1y -pthread $<

%-tune: %.C %
	g++ -o $@ -Wall -O3 -march=native -DPROF -fprofile-use --std=c++1y -pthread $<

.PHONY: all
