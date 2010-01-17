CXX = g++
OPTIONS = -Wall -W -O0 -ggdb
INCLUDE_DIR = -I ../lib -I ../implementation
DEST_DIR = .

all: benchmark main gather_patterns

benchmark:
	$(CXX) $(OPTIONS) $(INCLUDE_DIR) -o $(DEST_DIR)/hex_benchmark main_benchmark.cpp

main:
	$(CXX) $(OPTIONS) $(INCLUDE_DIR) -o $(DEST_DIR)/hex main.cpp

gather_patterns:
	$(CXX) $(OPTIONS) $(INCLUDE_DIR) -o $(DEST_DIR)/gather_patterns gather_patterns.cpp

clean:
	rm -f hex hex_benchmark
