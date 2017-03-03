INCLUDE=../install/include
LIBRARY=../install/lib
CXXFLAGS=-std=c++11 -O3
LDFLAGS=-lIlmImf-2_2 -lIlmThread-2_2 -lIex-2_2 -lHalf -lz --llvm-lto 1

.PHONY=clean

EMFLAGS=
#EMFLAGS+=-s ALLOW_MEMORY_GROWTH=1
EMFLAGS+=-s TOTAL_MEMORY=134217728
EMFLAGS+=-s EXPORTED_FUNCTIONS='["_loadEXRRaw", "_loadEXRVec", "_loadEXRStr"]'
EMFLAGS+=-s NO_EXIT_RUNTIME=1

# embind
EMFLAGS+=--bind -DWRAPPER_EMBIND

exr-wrap.js: exr-wrap.cpp
	em++ $(EMFLAGS) $< -o $@ -I$(INCLUDE) -L$(LIBRARY) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f exr-wrap.js exr-wrap.js.mem
