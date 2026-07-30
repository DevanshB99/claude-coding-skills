CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
BUILD    := build
BIN      := $(BUILD)/quadratic
SRCS     := src/quadratic.cpp src/json.cpp src/ascii_plot.cpp src/http_server.cpp src/main.cpp
OBJS     := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)

.PHONY: all run cli clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

# Start the web front end on http://127.0.0.1:8080/
run: $(BIN)
	./$(BIN) --port 8080 --web web

# Terminal mode: make cli ARGS="1 -3 2"
cli: $(BIN)
	./$(BIN) --cli $(ARGS)

clean:
	rm -rf $(BUILD)

-include $(DEPS)
