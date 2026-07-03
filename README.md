# Fjord Archive Library

Fjord Archive Library is a C implementation for reading the Fjord Archive binary container. The format stores a directory of typed sections for text, nodes, links, groups, paths, metrics, resources, and annotations.

The public API is in `src/fenrir.h`. `make` builds the static library and inspection tool, while `make test` runs the smoke test using a generic seed archive. The three libFuzzer targets exercise document opening, graph-facing query access, and text report export.

## Local use

```sh
make
make test
make fuzzers
./build/fjord_inspect fuzz/corpus/parse_fuzzer/seed_minimal_1.fjar
```

The format is designed around staged parsing: header and directory decoding establish section boundaries; typed section readers populate records; caches and reference resolution build document views; checksum and consistency passes produce the final queryable document.
