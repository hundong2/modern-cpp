# README 

latest version 2024-03-28 
this README markdown page make for boost install. 

## install boost 

### download vcpkg 

```
git clone https://github.com/Microsoft/vcpkg.git
```

### Execute vcpkg 

```
.\vcpkg\bootstrap-vcpkg.bat
```

### install integration 

```
.\vcpkg integrate install
```

### install boost library 

```
.\vcpkg\vcpkg install boost:x64-windows
```

### check vcpkg list 

```
.\vcpkg\vcpkg list
```

