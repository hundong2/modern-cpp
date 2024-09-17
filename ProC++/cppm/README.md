# cppm build example 

- build information 

```c++
clang++ -std=c++20 Hello.cppm --precompile -o Hello.pcm
clang++ -std=c++20 use.cpp -fprebuilt-module-path=. Hello.pcm -o Hello.out
```

## reference 

[cppm](https://releases.llvm.org/17.0.1/tools/clang/docs/StandardCPlusPlusModules.html)  