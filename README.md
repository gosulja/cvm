# cvm (cat virtual machine)
A stack based virtual machine inspired by JVM.

# features
* Mathematical expressions such as `1+2` or `(5*5) - 10 / 2`.
* Variable declarations such as `string name = "blinx";` or `int age = 20;`.
* In built functions such as: `print`, `size`.

# potential issues
floating point numbers may not work as intended because im dumb and used `uint16_t` everywhere. will fix this!

# changes
Using a stack based virtual machine. 

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

# example
```
int age = 20;
print("Age: " + age);

string name = "blinx";
print("Name: " + name);
```
![image](https://github.com/user-attachments/assets/7b81853c-2617-4a4a-b4d9-b12a0f0d2e6a)
