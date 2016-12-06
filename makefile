all:
	clang++ -std=c++14 -Weverything -Wno-c++98-compat -pthread -lboost_iostreams ./src/main.cpp -o lzw
