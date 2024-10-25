# cvm (cat virtual machine)
A stack based bytecode virtual machine compiler inspired by JVM.

# features
* Mathematical expressions such as `1+2` or `(5*5) - 10 / 2`.
* Variable declarations, currently has: `int x = 5;` or `bool flag = true;`
* String literals: `"hello" + " world";` is valid.
* Print built in: `print(4*4);`

# building
```
git clone https://github.com/gosulja/cvm
```
Either run:
```
g++ -Wall -g -o cvm main.cpp
```
or
```
make all
```
then:
```
./build/cvm [-options] [-file]
```

# usage
options
```
-d  : runs the virtual machine in debug mode
-s  : after execution the virtual machine returns the top most value on the stack
-h  : shows a help command
```

examples
```
[blinx@blinx cvms]$ ./build/cvm -d tests/vars.cat
[cvm] executing file: tests/vars.cat
[cvm] stack: [
[cvm] ]
[cvm] PUSH: raw byte = 0x02
[cvm] Pushing integer: 2
[cvm] stack: [
[cvm]      2,
[cvm] ]
2
[cvm] stack: [
[cvm] ]
[cvm] cvm halted.
```

```
[blinx@blinx cvms]$ ./build/cvm -s tests/vars.cat
[cvm] executing file: tests/vars.cat
4
[cvm] result: 4

source file:
int x = 2 * 2;
print(x);
```

```
[blinx@blinx cvms]$ ./build/cvm -h tests/vars.cat
Usage: ./build/cvm [-d] [filename]
  If no filename is provided, starts in REPL mode
```

and enjoy!

# example
![example](https://imgur.com/MJIDd6p.png)
