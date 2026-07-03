CC := clang
CXX := clang++
CFLAGS ?= -std=c11 -O2 -g
CXXFLAGS ?= -std=c++17 -O2 -g
BUILD := build
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
LIB := $(BUILD)/libfjord.a

.PHONY: all library tools test clean fuzzers

all: library tools

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

library: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

tools: $(BUILD)/fjord_inspect

$(BUILD)/fjord_inspect: tools/inspect.c $(LIB) | $(BUILD)
	$(CC) $(CFLAGS) -Isrc $< $(LIB) -o $@

$(BUILD)/test_smoke: tests/test_smoke.c $(LIB) | $(BUILD)
	$(CC) $(CFLAGS) -Isrc $< $(LIB) -o $@

test: $(BUILD)/test_smoke
	$(BUILD)/test_smoke fuzz/corpus/parse_fuzzer/seed_minimal_1.fjar

fuzzers: $(LIB) | $(BUILD)
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc fuzz/parse_fuzzer.cc $(LIB) -o $(BUILD)/parse_fuzzer
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc fuzz/graph_fuzzer.cc $(LIB) -o $(BUILD)/graph_fuzzer
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc fuzz/report_fuzzer.cc $(LIB) -o $(BUILD)/report_fuzzer

clean:
	rm -rf $(BUILD) src/*.o libmylib.a
