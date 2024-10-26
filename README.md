# cvm (cat virtual machine)
A register based bytecode virtual machine compiler.

# features
* Mathematical expressions such as `1+2` or `(5*5) - 10 / 2`.

# potential issues
floating point numbers may not work as intended because im dumb and used `uint16_t` everywhere. will fix this!

# changes
### October 24 2024
* Modified instruction `MOV` to `LDR` to make it more clear that we're loading a register.
* Made `prettyBytecode` debug function `inline`.

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
make
```
then:
```
./cvm [-d -h -o] [...file.cat]
```
