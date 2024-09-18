# build 

- build information for cppm

```sh
clang++ -std=c++20 SpreadsheetCell.cppm --precompile -o SpreadsheetCell.pcm
clang++ -std=c++20 Main.cpp -fmodule-file=SpreadsheetCell.pcm -o Main.out
```