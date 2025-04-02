# MPS Parser in C++ 

Convert text-based MPS files into Parquet files in C++. 

Translated from the python version here: 
https://github.com/laroccacharly/parse-mps-benchmark

[![MPS Parser Video](https://img.youtube.com/vi/TjY1KIOHbO4/0.jpg)](https://youtu.be/TjY1KIOHbO4)

Install dependencies:
```bash 
brew install cmake eigen apache-arrow nlohmann-json python uv
```

## Usage 

Compile the C++ project 
```bash 
./build.sh 
```
Run tests 
```bash 
./test.sh
```
Parse the first file in `mps_files`
```bash 
./parse_first_mps.sh
```