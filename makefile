all:
	clang++ -std=c++14 -Weverything -Wno-c++98-compat -pthread -lboost_program_options -lboost_iostreams ./src/*.cpp -o lzw
