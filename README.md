# WOW-SUCH-COMPRESSION

wow-such-compression is a simple implementation of the parallel LZW compression algorithm. 
working well on MacOS & Linux

## Prerequisites
To use wow-such-compression, you must have the following software installed:
* C++ compiler with:
  * Support for C++14
  * Boost 1.63+

## Build
```
$ make
```

## Usage 
```
General options:
  -h [ --help ]              Show help  
  -e [ --encode ] arg        Encode file using several threads  
  -s [ --encode-single ] arg Encode file using one thread
  -d [ --decode ] arg        Decode file using that much threads as we using at
                             encode statement
  -z [ --decode-single ] arg Decode file using one thread
```

### Example 
```
 $ ./lzw -e test.in > test.compressed
 $ ./lzw -d test.compressed > test.decompressed
```