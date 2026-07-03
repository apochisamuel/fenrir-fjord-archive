#!/bin/bash -eu
cd $SRC    # CRITICAL: Fenrir sets $SRC to the repo root. Do NOT append subdirectory.

for src in src/*.c; do
    obj="${src%.c}.o"
    $CC $CFLAGS -Isrc -c -o "$obj" "$src"
done

ar rcs libmylib.a src/*.o

for fuzzer_src in fuzz/*_fuzzer.cc; do
    fuzzer_name=$(basename "${fuzzer_src%.cc}")
    $CXX $CXXFLAGS -Isrc -o "$OUT/$fuzzer_name" \
        "$fuzzer_src" libmylib.a \
        $LIB_FUZZING_ENGINE
done
