# chapter 9

## class 

### friend 

```c++
class Foo
{
    friend class Bar;
};
```

- Bar is possible to use all of the Foo that of private and protected member and method. 

- Specific friend declare.

```c++
class Foo
{
    friend void Bar::processFoo(const Foo&);
}
```

- standalone function ( independent function ) is possible to use friend. 

```c++
class Foo
{
    friend void printFoo(const Foo&);
}
```