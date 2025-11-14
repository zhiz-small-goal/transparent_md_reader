# C++ const 与 constexpr 详解

## 目录
1. [const 是什么？为什么需要 const？](#1-const-是什么为什么需要-const)
2. [const 变量](#2-const-变量)
3. [const 指针（重点难点）](#3-const-指针重点难点)
4. [const 引用](#4-const-引用)
5. [const 成员函数](#5-const-成员函数)
6. [mutable 关键字](#6-mutable-关键字)
7. [constexpr 是什么？](#7-constexpr-是什么)
8. [const vs constexpr](#8-const-vs-constexpr)
9. [实际使用场景](#9-实际使用场景)
10. [常见错误和陷阱](#10-常见错误和陷阱)
11. [总结对比表](#11-总结对比表)

---

## 1. const 是什么？为什么需要 const？

### 什么是 const？

**`const` 是一个关键字，表示"常量"（constant），意思是"不可改变的"。**

就像现实生活中：
- 你的出生日期：一旦确定，就不能改变
- 圆周率 π = 3.14159...：永远不变
- 一个人的身份证号：一旦分配，就不能修改

在C++中，`const` 用来告诉编译器：
> "这个东西一旦设定了值，就不能再修改了，如果有人试图修改，编译器要报错！"

### 为什么需要 const？

#### 理由1：防止意外修改
```cpp
const double PI = 3.14159;
// ... 几百行代码之后 ...
PI = 3.14;  // ❌ 编译器报错：不能修改 const 变量
```
**好处：** 如果你不小心写了 `PI = 3.14`，编译器会立即提醒你，避免了难以发现的bug。

#### 理由2：表达意图（代码更易读）
```cpp
void printMessage(const std::string& msg) {
    std::cout << msg << std::endl;
    // 看到 const，就知道这个函数不会修改 msg
}
```
**好处：** 其他人（或未来的你）看到 `const`，就知道这个参数不会被修改。

#### 理由3：编译器优化
```cpp
const int MAX_SIZE = 1000;
// 编译器可能直接把 MAX_SIZE 替换为 1000，而不是从内存读取
```
**好处：** 编译器知道值不会变，可以做更激进的优化。

---

## 2. const 变量

### 2.1 基本语法拆解

```cpp
const int x = 10;
```

**逐个符号拆解：**
- `const` ：关键字，表示"常量"
- `int` ：类型，表示"整数"
- `x` ：变量名
- `=` ：赋值符号
- `10` ：初始值

**完整含义：**
> 创建一个名为 `x` 的常量，类型是 `int`，值是 `10`，这个值**永远不能被修改**。

### 2.2 const 变量的规则

#### 规则1：const 变量必须初始化
```cpp
const int x = 10;  // ✅ 正确：创建时立即初始化

const int y;       // ❌ 错误：const 变量必须初始化
y = 20;            // 即使后面赋值也不行
```

**为什么？**
> 因为 const 变量不能被修改，如果不初始化，它就永远没有值了。

#### 规则2：const 变量不能被修改
```cpp
const int x = 10;
x = 20;  // ❌ 错误：不能修改 const 变量
```

#### 规则3：const 变量可以被读取
```cpp
const int x = 10;
int y = x;  // ✅ 正确：可以读取 const 变量的值
std::cout << x << std::endl;  // ✅ 正确：可以输出
```

### 2.3 完整示例

```cpp
#include <iostream>

int main() {
    const int MAX_STUDENTS = 30;  // 教室最多容纳30个学生
    
    std::cout << "最大学生数: " << MAX_STUDENTS << std::endl;  // ✅ 可以读取
    
    int currentStudents = 25;
    if (currentStudents < MAX_STUDENTS) {
        std::cout << "还有空位" << std::endl;
    }
    
    // MAX_STUDENTS = 40;  // ❌ 错误：不能修改 const 变量
    
    return 0;
}
```

### 2.4 const 的位置

```cpp
const int x = 10;  // 方式1：const 在前（常用）
int const y = 20;  // 方式2：const 在后（少见但合法）
```

**两种写法完全等价！**
- `const int x` 和 `int const x` 是一样的
- 但大多数人习惯写 `const int x`

**助记：** 对于指针，`const` 的位置非常重要（见下一节）！

---

## 3. const 指针（重点难点）

### 3.1 为什么 const 指针这么复杂？

指针有两个"层面"：
1. **指针本身**：指针变量存储的地址
2. **指针指向的内容**：地址对应的值

`const` 可以修饰其中任何一个，或者两个都修饰！

### 3.2 指向常量的指针（Pointer to const）

#### 语法拆解
```cpp
const int* ptr = &x;
```

- `const` ：常量修饰符
- `int` ：类型
- `*` ：指针符号
- `ptr` ：指针名
- `=` ：赋值符号
- `&x` ：取 x 的地址

**完整含义：**
> `ptr` 是一个指针，它指向一个 `const int`（常量整数）。
> **不能通过 `ptr` 修改它指向的内容**，但 `ptr` 本身可以指向别的地址。

**助记口诀：** "const 在前，内容不变"

#### 完整示例
```cpp
#include <iostream>

int main() {
    int a = 10;
    int b = 20;
    
    const int* ptr = &a;  // ptr 指向 a
    
    std::cout << "*ptr = " << *ptr << std::endl;  // 输出: 10（✅ 可以读取）
    
    // *ptr = 30;  // ❌ 错误：不能通过 ptr 修改内容
    
    ptr = &b;  // ✅ 正确：可以让 ptr 指向 b
    std::cout << "*ptr = " << *ptr << std::endl;  // 输出: 20
    
    a = 30;  // ✅ 正确：可以直接修改 a
    std::cout << "a = " << a << std::endl;  // 输出: 30
    
    return 0;
}
```

**图示：**
```
内存:  [10]  [20]
        ↑     ↑
        a     b

const int* ptr = &a;
ptr 指向 a，但不能通过 ptr 修改 a
*ptr = 30;  // ❌ 不能修改

ptr = &b;  // ✅ 可以改变 ptr 的指向
```

#### 等价写法
```cpp
const int* ptr = &x;  // 方式1：常用
int const* ptr = &x;  // 方式2：等价，但少见
```

### 3.3 常量指针（const Pointer）

#### 语法拆解
```cpp
int* const ptr = &x;
```

- `int` ：类型
- `*` ：指针符号
- `const` ：常量修饰符（注意位置：在 `*` 之后）
- `ptr` ：指针名
- `=` ：赋值符号
- `&x` ：取 x 的地址

**完整含义：**
> `ptr` 是一个常量指针，它指向一个 `int`。
> **`ptr` 本身不能改变指向**（永远指向同一个地址），但可以通过 `ptr` 修改它指向的内容。

**助记口诀：** "const 在后，指向不变"

#### 完整示例
```cpp
#include <iostream>

int main() {
    int a = 10;
    int b = 20;
    
    int* const ptr = &a;  // ptr 永远指向 a
    
    std::cout << "*ptr = " << *ptr << std::endl;  // 输出: 10（✅ 可以读取）
    
    *ptr = 30;  // ✅ 正确：可以通过 ptr 修改 a 的值
    std::cout << "a = " << a << std::endl;  // 输出: 30
    
    // ptr = &b;  // ❌ 错误：不能改变 ptr 的指向
    
    return 0;
}
```

**图示：**
```
内存:  [10]  [20]
        ↑     
        a     b

int* const ptr = &a;
ptr 永远指向 a，不能改变指向
ptr = &b;  // ❌ 不能改变指向

*ptr = 30;  // ✅ 可以修改 a 的值
```

### 3.4 指向常量的常量指针（const Pointer to const）

#### 语法拆解
```cpp
const int* const ptr = &x;
```

- `const` ：第一个 const，修饰内容
- `int` ：类型
- `*` ：指针符号
- `const` ：第二个 const，修饰指针本身
- `ptr` ：指针名
- `=` ：赋值符号
- `&x` ：取 x 的地址

**完整含义：**
> `ptr` 是一个常量指针，指向一个常量整数。
> **既不能改变 `ptr` 的指向，也不能通过 `ptr` 修改内容**。

**助记口诀：** "两个 const，都不能变"

#### 完整示例
```cpp
#include <iostream>

int main() {
    int a = 10;
    int b = 20;
    
    const int* const ptr = &a;  // ptr 永远指向 a，且不能修改 a
    
    std::cout << "*ptr = " << *ptr << std::endl;  // 输出: 10（✅ 可以读取）
    
    // *ptr = 30;  // ❌ 错误：不能通过 ptr 修改内容
    // ptr = &b;   // ❌ 错误：不能改变 ptr 的指向
    
    a = 30;  // ✅ 正确：可以直接修改 a
    std::cout << "*ptr = " << *ptr << std::endl;  // 输出: 30
    
    return 0;
}
```

**图示：**
```
内存:  [10]  [20]
        ↑     
        a     b

const int* const ptr = &a;
ptr 永远指向 a，既不能改变指向，也不能修改内容
ptr = &b;   // ❌ 不能改变指向
*ptr = 30;  // ❌ 不能修改内容
```

### 3.5 三种 const 指针对比表

| 类型 | 语法 | 能否修改内容 | 能否改变指向 | 助记口诀 |
|------|------|-------------|-------------|---------|
| 指向常量的指针 | `const int* ptr` | ❌ 否 | ✅ 是 | "const在前，内容不变" |
| 常量指针 | `int* const ptr` | ✅ 是 | ❌ 否 | "const在后，指向不变" |
| 指向常量的常量指针 | `const int* const ptr` | ❌ 否 | ❌ 否 | "两个const，都不能变" |
| 普通指针 | `int* ptr` | ✅ 是 | ✅ 是 | 无限制 |

### 3.6 记忆技巧：从右往左读

**规则：** 从右往左读，遇到 `*` 时说"指针"，遇到 `const` 时说"常量"。

#### 示例1：`const int* ptr`
- 从右往左读：`ptr` → `*`（指针）→ `int`（整数）→ `const`（常量）
- 翻译：`ptr` 是一个指针，指向常量整数
- 结论：不能修改内容，可以改变指向

#### 示例2：`int* const ptr`
- 从右往左读：`ptr` → `const`（常量）→ `*`（指针）→ `int`（整数）
- 翻译：`ptr` 是一个常量指针，指向整数
- 结论：可以修改内容，不能改变指向

#### 示例3：`const int* const ptr`
- 从右往左读：`ptr` → `const`（常量）→ `*`（指针）→ `int`（整数）→ `const`（常量）
- 翻译：`ptr` 是一个常量指针，指向常量整数
- 结论：既不能修改内容，也不能改变指向

### 3.7 实际应用场景

#### 场景1：函数参数（不修改内容）
```cpp
void printArray(const int* arr, int size) {
    for (int i = 0; i < size; ++i) {
        std::cout << arr[i] << " ";
        // arr[i] = 0;  // ❌ 编译器阻止意外修改
    }
}
```

#### 场景2：字符串字面量
```cpp
const char* message = "Hello, World!";  // 字符串字面量不能修改
// message[0] = 'h';  // ❌ 错误
```

#### 场景3：固定的配置指针
```cpp
int globalConfig = 100;
int* const configPtr = &globalConfig;  // 配置指针永远指向 globalConfig
*configPtr = 200;  // ✅ 可以修改配置值
// configPtr = &otherVar;  // ❌ 不能改变指向
```

---

## 4. const 引用

### 4.1 基本语法拆解

```cpp
const int& ref = x;
```

- `const` ：常量修饰符
- `int` ：类型
- `&` ：引用符号
- `ref` ：引用名
- `=` ：绑定符号
- `x` ：原始变量

**完整含义：**
> `ref` 是 `x` 的引用（别名），类型是 `int`，但是**只能读取，不能修改**。

### 4.2 const 引用的特点

```cpp
#include <iostream>

int main() {
    int x = 10;
    const int& ref = x;  // ref 是 x 的只读引用
    
    std::cout << "ref = " << ref << std::endl;  // ✅ 可以读取
    
    // ref = 20;  // ❌ 错误：不能通过 const 引用修改
    
    x = 20;  // ✅ 可以直接修改 x
    std::cout << "ref = " << ref << std::endl;  // 输出: 20（ref 看到最新值）
    
    return 0;
}
```

**关键理解：**
- `const int& ref = x` 表示"不能通过 `ref` 修改 `x`"
- 但可以直接修改 `x`
- `ref` 始终反映 `x` 的最新值

### 4.3 const 引用可以绑定到临时值

#### 普通引用不能绑定到临时值
```cpp
int& ref = 10;  // ❌ 错误：普通引用不能绑定到临时值
```

#### const 引用可以绑定到临时值
```cpp
const int& ref = 10;  // ✅ 正确：const 引用可以绑定到临时值
std::cout << ref << std::endl;  // 输出: 10
```

**为什么？**
> 因为 `const` 保证了不会修改这个临时值，编译器会创建一个临时变量存储 `10`，然后让 `ref` 绑定到它。临时变量的生命周期被延长到 `ref` 的生命周期。

### 4.4 函数参数中的 const 引用（最常用）

```cpp
#include <iostream>
#include <string>

// ❌ 不好：传值，会复制整个字符串
void printBad(std::string str) {
    std::cout << str << std::endl;
}

// ✅ 好：传 const 引用，不复制，不修改
void printGood(const std::string& str) {
    std::cout << str << std::endl;
    // str += "!";  // ❌ 编译器阻止修改
}

int main() {
    std::string longText = "这是一个很长的字符串...";
    
    printBad(longText);   // 复制整个字符串，慢！
    printGood(longText);  // 不复制，快！
    
    return 0;
}
```

**最佳实践：**
> 如果函数不需要修改参数，就用 `const 引用`，既快又安全！

---

## 5. const 成员函数

### 5.1 什么是 const 成员函数？

**定义：** const 成员函数是承诺"不会修改对象成员变量"的成员函数。

### 5.2 基本语法拆解

```cpp
class MyClass {
    int value;
    
public:
    int getValue() const {
        return value;
    }
};
```

**语法拆解：**
```cpp
int getValue() const
```

- `int` ：返回类型
- `getValue` ：函数名
- `()` ：参数列表（这里为空）
- `const` ：**const 修饰符，放在参数列表后面**

**完整含义：**
> 这是一个成员函数 `getValue`，它承诺**不会修改对象的成员变量**。

### 5.3 为什么需要 const 成员函数？

```cpp
#include <iostream>

class Person {
    std::string name;
    int age;
    
public:
    Person(const std::string& n, int a) : name(n), age(a) {}
    
    // const 成员函数：只读取，不修改
    std::string getName() const {
        return name;
    }
    
    int getAge() const {
        return age;
    }
    
    // 非 const 成员函数：可以修改
    void setAge(int a) {
        age = a;
    }
};

int main() {
    const Person p("Alice", 25);  // const 对象
    
    std::cout << p.getName() << std::endl;  // ✅ 可以调用 const 成员函数
    std::cout << p.getAge() << std::endl;   // ✅ 可以调用 const 成员函数
    
    // p.setAge(26);  // ❌ 错误：const 对象不能调用非 const 成员函数
    
    return 0;
}
```

**规则：**
- **const 对象只能调用 const 成员函数**
- 非 const 对象可以调用任何成员函数

### 5.4 const 成员函数的限制

```cpp
class Counter {
    int count;
    
public:
    Counter() : count(0) {}
    
    int getCount() const {
        return count;  // ✅ 可以读取成员变量
    }
    
    void increment() const {
        // count++;  // ❌ 错误：const 成员函数不能修改成员变量
    }
    
    void reset() const {
        // count = 0;  // ❌ 错误：const 成员函数不能修改成员变量
    }
};
```

**限制：**
- const 成员函数内部不能修改成员变量
- const 成员函数内部不能调用非 const 成员函数

### 5.5 完整示例：const 的正确用法

```cpp
#include <iostream>
#include <string>

class Book {
    std::string title;
    int pages;
    
public:
    Book(const std::string& t, int p) : title(t), pages(p) {}
    
    // const 成员函数：只读取
    std::string getTitle() const {
        return title;
    }
    
    int getPages() const {
        return pages;
    }
    
    void printInfo() const {  // ✅ 不修改成员变量，应该是 const
        std::cout << "《" << title << "》, " << pages << " 页" << std::endl;
    }
    
    // 非 const 成员函数：修改成员变量
    void setTitle(const std::string& t) {
        title = t;
    }
    
    void setPages(int p) {
        pages = p;
    }
};

void displayBook(const Book& book) {  // 参数是 const 引用
    book.printInfo();  // ✅ 只能调用 const 成员函数
    // book.setTitle("New Title");  // ❌ 错误：不能调用非 const 成员函数
}

int main() {
    Book book("C++ Primer", 800);
    
    // 非 const 对象：可以调用任何函数
    book.printInfo();
    book.setTitle("Effective C++");
    
    // const 对象：只能调用 const 函数
    const Book constBook("The C++ Programming Language", 1000);
    constBook.printInfo();  // ✅ 可以
    // constBook.setTitle("xxx");  // ❌ 不能
    
    displayBook(book);
    displayBook(constBook);
    
    return 0;
}
```

### 5.6 const 成员函数的底层原理

```cpp
class MyClass {
    int value;
    
public:
    int getValue() const {
        return value;
    }
};
```

**编译器实际上会这样理解：**
```cpp
int getValue(const MyClass* this) {  // this 指针是 const 的
    return this->value;
}
```

**关键理解：**
- 每个成员函数都有一个隐藏的 `this` 指针
- const 成员函数的 `this` 指针是 `const MyClass*` 类型
- 所以不能通过 `this` 修改成员变量

---

## 6. mutable 关键字

### 6.1 什么是 mutable？

**问题场景：** 有时候我们需要在 const 成员函数中修改某些成员变量（比如缓存、计数器）。

`mutable` 关键字可以让成员变量在 const 成员函数中被修改。

### 6.2 基本语法拆解

```cpp
class MyClass {
    mutable int accessCount;
};
```

- `mutable` ：关键字，表示"可变的"
- `int` ：类型
- `accessCount` ：变量名

**完整含义：**
> `accessCount` 是一个可变的成员变量，即使在 const 成员函数中也可以修改。

### 6.3 实际应用：缓存

```cpp
#include <iostream>
#include <string>

class Person {
    std::string firstName;
    std::string lastName;
    
    mutable std::string cachedFullName;  // 缓存的完整姓名
    mutable bool cacheValid;              // 缓存是否有效
    
public:
    Person(const std::string& first, const std::string& last)
        : firstName(first), lastName(last), cacheValid(false) {}
    
    // const 成员函数，但可以修改 mutable 成员变量
    std::string getFullName() const {
        if (!cacheValid) {  // 缓存无效，重新计算
            cachedFullName = firstName + " " + lastName;
            cacheValid = true;
            std::cout << "计算完整姓名..." << std::endl;
        } else {
            std::cout << "使用缓存的姓名..." << std::endl;
        }
        return cachedFullName;
    }
    
    void setFirstName(const std::string& name) {
        firstName = name;
        cacheValid = false;  // 名字改变，缓存失效
    }
};

int main() {
    const Person p("Zhang", "San");
    
    std::cout << p.getFullName() << std::endl;  // 输出: 计算完整姓名... Zhang San
    std::cout << p.getFullName() << std::endl;  // 输出: 使用缓存的姓名... Zhang San
    
    return 0;
}
```

**为什么需要 mutable？**
> `getFullName()` 在逻辑上是 const 的（不改变对象的"逻辑状态"），但在实现上需要修改缓存（改变对象的"物理状态"）。`mutable` 允许这种情况。

### 6.4 实际应用：访问计数

```cpp
#include <iostream>

class Data {
    int value;
    mutable int accessCount;  // 记录被访问的次数
    
public:
    Data(int v) : value(v), accessCount(0) {}
    
    int getValue() const {
        accessCount++;  // ✅ 可以修改 mutable 成员
        return value;
    }
    
    int getAccessCount() const {
        return accessCount;
    }
};

int main() {
    const Data d(100);
    
    std::cout << d.getValue() << std::endl;  // 第1次访问
    std::cout << d.getValue() << std::endl;  // 第2次访问
    std::cout << d.getValue() << std::endl;  // 第3次访问
    
    std::cout << "访问次数: " << d.getAccessCount() << std::endl;  // 输出: 3
    
    return 0;
}
```

### 6.5 逻辑常量 vs 物理常量

**逻辑常量（Logical Constness）：**
- 对象的"逻辑状态"不变
- 比如：Person 的姓名没变，只是缓存了完整姓名

**物理常量（Physical Constness）：**
- 对象的"物理状态"（每个字节）都不变
- 比如：没有任何成员变量被修改

**mutable 的作用：**
> 允许"物理状态"改变（修改 mutable 成员变量），但保持"逻辑状态"不变。

---

## 7. constexpr 是什么？

### 7.1 constexpr 的基本概念

**`constexpr` 是 C++11 引入的关键字，表示"编译期常量表达式"（compile-time constant expression）。**

**关键理解：**
- `const`：运行时常量（值在运行时确定，但不能修改）
- `constexpr`：编译期常量（值在编译时就确定了）

### 7.2 为什么需要 constexpr？

#### 场景1：数组大小必须是编译期常量
```cpp
const int size1 = 10;
int arr1[size1];  // ✅ 正确（编译器可能优化）

int n;
std::cin >> n;
const int size2 = n;  // const，但值在运行时确定
// int arr2[size2];  // ❌ 错误：数组大小必须是编译期常量
```

#### 使用 constexpr
```cpp
constexpr int size = 10;  // 编译期常量
int arr[size];  // ✅ 正确：size 在编译时就确定了
```

### 7.3 constexpr 变量

#### 语法拆解
```cpp
constexpr int x = 10;
```

- `constexpr` ：关键字，表示"编译期常量"
- `int` ：类型
- `x` ：变量名
- `=` ：赋值符号
- `10` ：初始值（必须是编译期可确定的）

**完整含义：**
> `x` 是一个编译期常量，值在编译时就确定为 `10`。

#### constexpr 的规则

**规则1：constexpr 变量必须用常量表达式初始化**
```cpp
constexpr int a = 10;  // ✅ 正确：10 是字面量

constexpr int b = a + 5;  // ✅ 正确：a + 5 在编译时可计算

int x = 10;
constexpr int c = x;  // ❌ 错误：x 不是编译期常量
```

**规则2：constexpr 变量隐含 const**
```cpp
constexpr int x = 10;
// 等价于：const int x = 10; （并且值在编译时确定）

// x = 20;  // ❌ 错误：constexpr 变量是 const 的
```

### 7.4 constexpr 函数

#### 基本语法
```cpp
constexpr int square(int x) {
    return x * x;
}
```

**含义：**
> 如果参数在编译时已知，`square` 可以在编译时计算结果。

#### 使用示例
```cpp
#include <iostream>

constexpr int square(int x) {
    return x * x;
}

int main() {
    // 编译期计算
    constexpr int result1 = square(5);  // 在编译时计算：25
    int arr[square(3)];  // 数组大小 = 9（编译时确定）
    
    // 运行时计算（如果参数不是编译期常量）
    int n = 10;
    int result2 = square(n);  // 在运行时计算
    
    std::cout << "result1 = " << result1 << std::endl;
    std::cout << "result2 = " << result2 << std::endl;
    
    return 0;
}
```

**关键理解：**
- `constexpr` 函数**可以**在编译时执行（如果参数是编译期常量）
- `constexpr` 函数**也可以**在运行时执行（如果参数不是编译期常量）

### 7.5 constexpr 函数的限制（C++11）

C++11 中，`constexpr` 函数有严格的限制：
```cpp
// ✅ 允许：简单的返回语句
constexpr int add(int a, int b) {
    return a + b;
}

// ❌ 不允许：多条语句（C++11）
constexpr int max(int a, int b) {
    if (a > b) return a;  // C++11 不允许 if
    else return b;
}

// ✅ C++11 的变通方法：使用三元运算符
constexpr int max(int a, int b) {
    return a > b ? a : b;
}
```

**C++14 放宽了限制：**
- C++14 允许 `if`、`for`、`while` 等语句
- C++14 允许局部变量

```cpp
// C++14 及以后
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}
```

---

## 8. const vs constexpr

### 8.1 核心区别

| 特性 | const | constexpr |
|------|-------|----------|
| 含义 | 不可修改的变量 | 编译期常量表达式 |
| 值确定时间 | 编译时或运行时 | **必须在编译时** |
| 是否可修改 | ❌ 否 | ❌ 否 |
| 可否用作数组大小 | 取决于初始化方式 | ✅ 是 |
| 可否用作 case 标签 | 取决于初始化方式 | ✅ 是 |
| 隐含 const | ❌ 否 | ✅ 是 |

### 8.2 对比示例

```cpp
#include <iostream>

int getValue() {
    return 42;
}

constexpr int getConstValue() {
    return 42;
}

int main() {
    // const：可以用运行时值初始化
    const int a = getValue();  // ✅ 正确：在运行时确定
    
    // constexpr：必须用编译期值初始化
    // constexpr int b = getValue();  // ❌ 错误：getValue() 不是编译期常量
    constexpr int c = getConstValue();  // ✅ 正确：getConstValue() 是 constexpr
    
    // 数组大小
    // int arr1[a];  // ❌ 可能错误：a 是运行时常量
    int arr2[c];  // ✅ 正确：c 是编译期常量
    
    // switch-case
    int x = 1;
    switch (x) {
        // case a:  // ❌ 可能错误：a 是运行时常量
        case c:  // ✅ 正确：c 是编译期常量
            break;
    }
    
    return 0;
}
```

### 8.3 何时使用 const，何时使用 constexpr？

#### 使用 const：
- 函数参数（避免修改）
- 成员函数（不修改对象）
- 指针和引用（避免修改内容）
- 运行时确定的常量

```cpp
void process(const std::string& str);  // 函数参数

class MyClass {
    int getValue() const;  // 成员函数
};

int n;
std::cin >> n;
const int size = n;  // 运行时常量
```

#### 使用 constexpr：
- 编译期常量（数组大小、case 标签）
- 编译期计算（性能优化）
- 需要在编译时确定的值

```cpp
constexpr int MAX_SIZE = 100;  // 编译期常量
int buffer[MAX_SIZE];

constexpr int square(int x) { return x * x; }
int data[square(10)];  // 数组大小 = 100
```

---

## 9. 实际使用场景

### 9.1 场景1：const 保护函数参数

```cpp
#include <iostream>
#include <vector>

// 好的实践：const 引用
void printVector(const std::vector<int>& vec) {
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    // vec.push_back(100);  // ❌ 编译器阻止意外修改
}

// 坏的实践：传值（复制）
void printVectorBad(std::vector<int> vec) {  // 复制整个 vector！
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5};
    printVector(data);  // 快速且安全
    return 0;
}
```

### 9.2 场景2：const 成员函数提供只读接口

```cpp
#include <iostream>
#include <string>
#include <vector>

class StudentRegistry {
    std::vector<std::string> students;
    
public:
    // 修改数据：非 const
    void addStudent(const std::string& name) {
        students.push_back(name);
    }
    
    // 读取数据：const
    int getStudentCount() const {
        return students.size();
    }
    
    std::string getStudent(int index) const {
        if (index >= 0 && index < students.size()) {
            return students[index];
        }
        return "";
    }
    
    void printAll() const {
        for (const auto& student : students) {
            std::cout << student << std::endl;
        }
    }
};

// 只读函数：参数是 const 引用
void displayRegistry(const StudentRegistry& registry) {
    std::cout << "学生总数: " << registry.getStudentCount() << std::endl;
    registry.printAll();
    // registry.addStudent("New Student");  // ❌ 编译器阻止修改
}

int main() {
    StudentRegistry registry;
    registry.addStudent("Alice");
    registry.addStudent("Bob");
    registry.addStudent("Charlie");
    
    displayRegistry(registry);
    
    return 0;
}
```

### 9.3 场景3：constexpr 编译期计算

```cpp
#include <iostream>

// 计算阶乘（编译期）
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

// 计算幂（编译期）
constexpr int power(int base, int exp) {
    return exp == 0 ? 1 : base * power(base, exp - 1);
}

int main() {
    // 编译时计算
    constexpr int fact5 = factorial(5);  // 120（编译时确定）
    constexpr int pow2_10 = power(2, 10);  // 1024（编译时确定）
    
    // 数组大小使用编译期常量
    int buffer[factorial(4)];  // 大小 = 24
    
    std::cout << "5! = " << fact5 << std::endl;
    std::cout << "2^10 = " << pow2_10 << std::endl;
    std::cout << "buffer 大小: " << sizeof(buffer) / sizeof(int) << std::endl;
    
    return 0;
}
```

### 9.4 场景4：const 指针保护数据

```cpp
#include <iostream>
#include <cstring>

// 字符串长度（不修改字符串）
int stringLength(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    // str[0] = 'X';  // ❌ 编译器阻止修改
    return len;
}

// 字符串拷贝
void copyString(char* dest, const char* src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    // *src = 'X';  // ❌ 编译器阻止修改 src
}

int main() {
    const char* message = "Hello, World!";
    char buffer[50];
    
    int len = stringLength(message);
    std::cout << "长度: " << len << std::endl;
    
    copyString(buffer, message);
    std::cout << "复制: " << buffer << std::endl;
    
    return 0;
}
```

### 9.5 场景5：mutable 实现缓存

```cpp
#include <iostream>
#include <vector>
#include <numeric>

class Statistics {
    std::vector<int> data;
    
    mutable double cachedAverage;
    mutable bool averageValid;
    
public:
    Statistics() : averageValid(false) {}
    
    void addData(int value) {
        data.push_back(value);
        averageValid = false;  // 数据改变，缓存失效
    }
    
    double getAverage() const {
        if (!averageValid) {
            cachedAverage = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
            averageValid = true;
            std::cout << "[计算平均值]" << std::endl;
        } else {
            std::cout << "[使用缓存]" << std::endl;
        }
        return cachedAverage;
    }
};

int main() {
    Statistics stats;
    stats.addData(10);
    stats.addData(20);
    stats.addData(30);
    
    std::cout << "平均值: " << stats.getAverage() << std::endl;  // 计算
    std::cout << "平均值: " << stats.getAverage() << std::endl;  // 使用缓存
    std::cout << "平均值: " << stats.getAverage() << std::endl;  // 使用缓存
    
    return 0;
}
```

---

## 10. 常见错误和陷阱

### 10.1 错误1：const 变量未初始化

```cpp
const int x;  // ❌ 错误：const 变量必须初始化
x = 10;       // 即使后面赋值也不行
```

**正确写法：**
```cpp
const int x = 10;  // ✅ 正确
```

### 10.2 错误2：混淆 const 指针的类型

```cpp
int a = 10, b = 20;

const int* ptr1 = &a;
*ptr1 = 30;  // ❌ 错误：不能修改内容
ptr1 = &b;   // ✅ 正确：可以改变指向

int* const ptr2 = &a;
*ptr2 = 30;  // ✅ 正确：可以修改内容
ptr2 = &b;   // ❌ 错误：不能改变指向
```

**助记：** 从右往左读！

### 10.3 错误3：const 对象调用非 const 函数

```cpp
class MyClass {
public:
    void modify() {  // 非 const 函数
        // ...
    }
    
    void display() const {  // const 函数
        // ...
    }
};

int main() {
    const MyClass obj;
    obj.display();  // ✅ 正确
    // obj.modify();  // ❌ 错误：const 对象不能调用非 const 函数
    return 0;
}
```

### 10.4 错误4：忘记 const 成员函数的 const

```cpp
class Person {
    std::string name;
    
public:
    // ❌ 错误：应该是 const，但忘记写了
    std::string getName() {
        return name;
    }
};

void printPerson(const Person& p) {
    // std::cout << p.getName() << std::endl;  // ❌ 错误：getName 不是 const
}
```

**正确写法：**
```cpp
std::string getName() const {  // ✅ 添加 const
    return name;
}
```

### 10.5 错误5：constexpr 使用运行时值

```cpp
int getValue() {
    return 42;
}

int main() {
    constexpr int x = getValue();  // ❌ 错误：getValue() 不是编译期常量
    return 0;
}
```

**正确写法：**
```cpp
constexpr int getValue() {  // ✅ constexpr 函数
    return 42;
}

int main() {
    constexpr int x = getValue();  // ✅ 正确
    return 0;
}
```

### 10.6 错误6：const 引用延长临时对象，但返回局部引用

```cpp
// ✅ 安全：const 引用延长临时对象
const std::string& ref = std::string("Hello");
std::cout << ref << std::endl;  // 正确

// ❌ 危险：返回局部变量的引用
const std::string& getDangerous() {
    std::string local = "Hello";
    return local;  // 局部变量，函数结束后销毁
}

int main() {
    const std::string& ref = getDangerous();
    // std::cout << ref << std::endl;  // 未定义行为！
    return 0;
}
```

---

## 11. 总结对比表

### 11.1 const 在不同位置的含义

| 位置 | 语法 | 含义 |
|------|------|------|
| 变量 | `const int x = 10;` | x 是常量，不可修改 |
| 指针内容 | `const int* ptr` | 不能通过 ptr 修改内容 |
| 指针本身 | `int* const ptr` | ptr 不能改变指向 |
| 指针两者 | `const int* const ptr` | 内容和指向都不能变 |
| 引用 | `const int& ref` | 不能通过 ref 修改 |
| 成员函数 | `int getValue() const` | 不修改成员变量 |
| 成员变量 | `mutable int count` | 在 const 函数中可修改 |

### 11.2 const vs constexpr

| 特性 | const | constexpr |
|------|-------|----------|
| 主要用途 | 防止修改 | 编译期计算 |
| 值确定时间 | 编译时或运行时 | 必须编译时 |
| 函数参数 | ✅ 常用 | ❌ 不能用 |
| 成员函数 | ✅ 常用 | ❌ 很少用 |
| 数组大小 | 有时可以 | ✅ 总是可以 |
| case 标签 | 有时可以 | ✅ 总是可以 |
| 运行时初始化 | ✅ 可以 | ❌ 不可以 |

### 11.3 const 指针快速记忆表

| 语法 | 记忆方法 | 内容可变 | 指向可变 |
|------|---------|---------|---------|
| `const int* p` | const 在前，内容不变 | ❌ 否 | ✅ 是 |
| `int* const p` | const 在后，指向不变 | ✅ 是 | ❌ 否 |
| `const int* const p` | 两个 const，都不变 | ❌ 否 | ❌ 否 |
| `int* p` | 无 const，都可变 | ✅ 是 | ✅ 是 |

### 11.4 最佳实践总结

#### 1. 函数参数
```cpp
// ✅ 大对象：用 const 引用
void process(const std::string& str);

// ✅ 小类型：传值
void process(int x);

// ✅ 需要修改：用引用（不加 const）
void modify(std::string& str);

// ✅ 可能为空：用指针
void process(const int* ptr);
```

#### 2. 成员函数
```cpp
class MyClass {
public:
    // ✅ 只读取：加 const
    int getValue() const;
    
    // ✅ 修改数据：不加 const
    void setValue(int v);
    
    // ✅ 缓存等特殊情况：用 mutable
    mutable int cacheData;
};
```

#### 3. 变量
```cpp
// ✅ 不会修改的值：用 const
const double PI = 3.14159;

// ✅ 编译期常量：用 constexpr
constexpr int BUFFER_SIZE = 1024;

// ✅ 运行时确定的常量：用 const
int size;
std::cin >> size;
const int arraySize = size;
```

---

## 12. 进阶主题（简介）

### 12.1 consteval（C++20）

C++20 引入了 `consteval`，表示"必须在编译期执行"。

```cpp
consteval int square(int x) {
    return x * x;
}

int main() {
    constexpr int a = square(5);  // ✅ 编译期执行
    
    int n = 10;
    // int b = square(n);  // ❌ 错误：n 不是编译期常量
    
    return 0;
}
```

**区别：**
- `constexpr` 函数：可以在编译期或运行期执行
- `consteval` 函数：必须在编译期执行

### 12.2 constinit（C++20）

C++20 引入了 `constinit`，用于保证静态/全局变量在编译期初始化。

```cpp
constexpr int getValue() {
    return 42;
}

constinit int globalVar = getValue();  // 编译期初始化
```

### 12.3 const 与模板

```cpp
template<typename T>
void process(const T& value) {
    // T 可能是 int, std::string, 甚至 const int
}
```

**注意：** 模板中的 const 处理比较复杂，涉及类型推导规则。

---

## 结语

`const` 和 `constexpr` 是C++中非常重要的特性：
- **const**：保护数据不被意外修改，让代码更安全、更易读
- **constexpr**：在编译期计算，让代码更高效

**核心要点：**
1. 优先使用 `const 引用` 作为函数参数（大对象）
2. 不修改对象的成员函数应该标记为 `const`
3. `const` 指针：记住"从右往左读"规则
4. 需要编译期常量时使用 `constexpr`
5. 特殊情况（缓存等）使用 `mutable`

**学习建议：**
- 养成习惯：能加 `const` 就加 `const`
- 多写代码，体会 `const` 的好处
- 理解 `const` 指针是重点难点，需要多练习

祝你学习愉快！🎉

