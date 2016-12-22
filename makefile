warnings = -Weverything -Wno-c++98-compat
boost_libs =  -lboost_program_options -lboost_iostreams
ssl_libs = -lssl -lcrypto
libs = $(boost_libs) $(ssl_libs) -pthread
sources = ./src/decoder.cpp ./src/encoder.cpp ./src/protector.cpp ./src/utilities.cpp ./src/main.cpp

all:
	clang++ -std=c++14 $(libs) $(warnings) $(sources) -o lzw

licence:
	clang++ -std=c++14 $(libs) $(warnings) ./src/make_licence_file.cpp ./src/protector.cpp -o licence_exec
