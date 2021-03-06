# EXR Wrapper

This wrapper is intended to be used via emscripten to interface with OpenEXR from
JavaScript. It has the following dependencies:

- [emscripten](http://kripken.github.io/emscripten-site/index.html): To compile the
  dependencies and the wrapper itself. Tested version is emscripten-1.38.12.
- [OpenEXR](http://openexr.com/): The main library. Tested version is 2.2.1. Both the
  openexr and the ilmbase libraries are required.
- [zlib](http://zlib.net/): Dependency for OpenEXR. Tested version is 1.2.11.


# Compiling

In general, compiling with emscripten is quite annoying. The `emconfigure` and `emmake`
tools can help [building the dependencies][0]. To run the compiled javascript applications,
node, which is also part of emscripten, can be used. The result of the build process is a
pair of files, `exr-wrap.js` and `exr-wrap.wasm`. LTO and other options can be set for
more optimization, but their use is [somewhat questionable][1].

0. Prepare
	0. `mkdir install`
	0. `export WRAPPER_INSTALL=$(pwd)/install`
1. Building zlib
	0. `cd $zlibdir`
	1. `emconfigure ./configure --prefix $WRAPPER_INSTALL`
	2. Edit the Makefile: `AR=emar`, `ARFLAGS=r`, `CFLAGS+=-O3`
	3. Optionally add `--llvm-lto 1` to CFLAGS
	3. `emmake make -j12 && make install`
2. Building ilmbase
	0. `cd $ilmbasedir && mkdir build && cd build`
	1. `emconfigure cmake .. -DCMAKE_INSTALL_PREFIX=$WRAPPER_INSTALL -DCMAKE_BUILD_TYPE=Release`
	2. Possibly remove incorrect quotes around the node path in CMakeLists.txt
	3. Edit `config/IlmBaseConfig.h`, setting `HAVE_PTHREAD 0`.
	4. Optionally add `--llvm-lto 1` to compiler and linker flags and set `-O3`.
	5. `emmake make -j12 && make install`
3. Building openexr
	0. cd openexr && mkdir build && cd build
	1. `emconfigure cmake .. -DCMAKE_INSTALL_PREFIX=$WRAPPER_INSTALL -DCMAKE_BUILD_TYPE=Release -DILMBASE_PACKAGE_PREFIX=$WRAPPER_INSTALL`
	2. Possibly set `ZLIB_LIBRARY` and `ZLIB_INCLUDE_DIR`
	3. Optionally add `--llvm-lto 1` to compiler and linker flags and set `-O3`.
	4. `emmake make -j12 && make install`
	5. Building the `dwaLookups.h` and `b44ExpLogTable.h` headers requires using node to
		run the javascript programs built for this purpose. Building with `make VERBOSE=1`
		can help figuring out the exact parameters. Use node to run the created .js
		programs. Alternatively, copy the two headers from a native build.
4. Building the wrapper
	0. `cd wrapp`
	1. `make`

[0]: http://kripken.github.io/emscripten-site/docs/compiling/Building-Projects.html#building-projects
[1]: https://kripken.github.io/emscripten-site/docs/optimizing/Optimizing-Code.html
