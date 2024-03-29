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

## LR Value 

### R Value 

1. Numeric literals, such as 3 and 3.141592414, are Rvalues. So are character literals, such as 'a'.
2. An identifier that names an enumeration constant is an Rvalue. For example,

```sh
enum Color {red, green, blue}
Color enumColor;
enumColor = green;
blud = green; // Error -> blud is an R-Value
```

3. The result of binary + operator is always an Rvalue

```sh
m + 1 = n  // Error. As (m+1) is Rvalue.
```

4. The unary & (address-of) operator requires an Lvalue as its operand. 
That is, &n is a valid expression only if n is an Lvalue.  
Thus, an expression such as &3 is an error. 
Again, 3 does not refer to an object, so it's not addressable. 
Although the unary & requires an Lvalue as its operand, its result is an Rvalue.  

```sh
int n, *p;
p = &n;     // Fine 
&n = p;     // Error: &n is an Rvalue
```

5. In contrast to unary &, unary * produces an lvalue as its result. 
A non-null(valid) pointer p always points to an object, so *p is an lvalue. For example:

```sh
int a[N];
int *p = a;
*p = 3;         // Fine. 

// Although the result is an Lvalue, the operand can be an Rvalue 
 *(p + 1) = 4; // Fine. (p+1) is an Rvalue
```

6. Pre-increment operator expressions results LValues.  

```sh
int nCount = 0;   // nCount represents a persistent object and hence Lvalue
++nCount;         // This expression is an Lvalue as this alters 
                  // and then points to nCount object              
 
// Just to prove that this is an Lvalue, we can do the below operation
++nCount = 5;    // Fine.
```

7. A function call is an Lvalue if and only if the result type is a reference.  

```C++
int& GetBig(int& a, int& b)    // returning reference to make the function call an Lvalue
{
    return ( a > b ? a : b );
}

void main()
{
    int i = 10, j = 50;
    GetBig( i, j ) *= 5;    
    // Here, j = 250. GetBig() returns the ref of j and it gets multiplied by 5 times.
} 
```

8. A reference is a name, so a reference bound to an Rvalue is itself an Lvalue:

```C++
int GetBig(int& a, int& b)    // returning an int to make the function call an Rvalue
{
    return ( a > b ? a : b );
}

void main()
{
    int i = 10, j = 50;
    const int& big = GetBig( i, j );
    // Here, I am binding 'big' an Lvalue to the return value from GetBig(), an Rvalue.

    int& big2 = GetBig(i, j); // Error. big2 is not binding to the return value as big2 
                              //        is not const
} 
```

9. Rvalues are temporaries and don't necessarily point to a memory region but they may hold memory in some cases. It is not advisable to catch this address and do any further operations as it would be a booby trap to work on these temporaries.

```C++
char* fun() { return "Hellow"; } 

int main()
 { 
      char* q = fun();
      q[0]='h';     // Exception is thrown here as fun() returns an temporary memory 
                    // and we are trying to access it !!!
 }
```

10. Post-increment operator expressions results RValues:

```C++
int nCount = 0;  // nCount represents a persistent object and hence Lvalue
nCount++         // This expression is a Rvalue as it copies the value of the 
                 // persistent object, alters it and then returns the temporary copy.

// Just to prove that this is an Rvalue, we can not do the below operation
nCount++ = 5; //Error
```

By summarizing the above points, we can blindly state that:  
If we can take address of an expression (for further operations) safely,  
then it is a lvalue expression, else it is an rvalue expression.  
It makes sense right, as it is preposterous to carry on with a temporary.  

Note: Both Lvalues and Rvalues could be modifiable or non-modifiable. Here are the examples:  

```C++
string strName("Hello");                           // modifiable lvalue
const string strConstName("Hello");                       // const lvalue
string JunkFunction() { return "Hellow World"; /*catch this properly*/}//modifiable rvalue
const string Fun() { return "Hellow World"; }               // const rvalue 
```

### Conversion between Lvalues and Rvalues

Can an Lvalue appear in a context that requires an Rvalue? YES, it can. For example:  

```C++
int a, b;
a = 8;
b = 5;
a = b;
```

This = expression uses the Lvalue sub-expression b as Rvalue. In this case, the compiler performs what is called lvalue-to-rvalue conversion to obtain the value stored in b.  
Now, can an r-value appear in a context that requires an l-value. NO, it can't.  

```C++
3 = a // Error. Here 3 which is an RValue is appearing in the context where
        //        Lvalue is required
```

# reference 

- https://learn.microsoft.com/en-us/cpp/cpp/lvalues-and-rvalues-visual-cpp?view=msvc-170&redirectedfrom=MSDN
- https://www.codeproject.com/Articles/313469/The-Notion-of-Lvalues-and-Rvalues
- https://jeremyko.blogspot.com/2012/08/lvalue-rvalue.html

