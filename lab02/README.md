# CS61C Lab 02 中文复习笔记

本笔记结合本目录的作业、测试代码及本次学习中的问题整理。记录日期：2026-11-08。代码块中的“参考实现”用于复习；本文没有自动替换你的源码。

课程入口：[Lab02 课程页面](https://www.learncs.site/docs/curriculum-resource/cs61c/labs/lab02)。课程分为 Makefile、位运算、LFSR 和动态内存管理四部分。特别注意：Exercise 1 要求仅使用位运算，禁止条件语句、循环及算术运算；Exercise 3 还要求补全头文件声明、构建规则，并检查内存管理。

## 1. 当前作业状态：先知道哪些地方还需要注意

| 文件或项目 | 本次检查结果 |
| --- | --- |
| `make.txt` | 前六题答案正确；第七题应精确写 **32**，第 31 行是依赖规则，第 32 行才是链接命令 |
| `bit_ops.c` | 自带测试全部输出 `correct`；但 `set_bit` 使用 `if`，不符合课程限制 |
| 位移掩码 | 当前 `set_bit`、`flip_bit` 使用 `1 << n`，建议改为 `1u << n`，覆盖第 31 位的情况 |
| `lfsr.c` | 当前反馈计算和更新顺序正确，运行得到 65535 个状态 |
| `vector.c`、`vector.h` | 当前常规测试结果匹配，Vector 的 Valgrind 检查无错误、无未释放内存 |
| `vector_set` 的错误提示 | 当前空指针分支写成了 `vector_get`，应标明 `vector_set`，便于定位问题 |
| `Makefile` | 已添加不使用变量的 `vector-test` 规则，可完成构建 |

之前讨论中认可了分支版 `set_bit` 的思路，但遗漏了题目禁止条件语句的限制。提交时应使用本文的纯位运算版本。测试通过不等于满足所有题目限制，也不等于覆盖所有边界。

## 2. Exercise 0：Makefile 七题答案

行号以本次检查的 [Makefile](Makefile) 为准，后续插入代码可能改变行号。

| 题号 | 问题含义 | 答案 | 对应说明 |
| --- | --- | --- | --- |
| 1 | 哪个目标删除所有已编译程序？ | `clean` | 第 46 行声明目标，第 47 行执行删除 |
| 2 | 哪个目标生成全部程序？ | `all` | 第 26 行，依赖 `$(BINARIES)` |
| 3 | 使用哪个编译器？ | `gcc` | 第 2 行 `CC=gcc` |
| 4 | 使用哪个 C 标准？ | C99 | 第 4 行的 `-std=c99` |
| 5 | 如何引用变量 FOO？ | `$(FOO)` | `${FOO}` 也可使用 |
| 6 | Darwin 在这里表示哪个系统？ | macOS | macOS 上 `uname -s` 输出 Darwin |
| 7 | 哪一行把目标文件链接成 lfsr？ | **第 32 行** | 第 31 行仅声明目标和依赖 |

### 2.1 规则的基本结构

```makefile
目标: 依赖文件
	执行的命令
```

命令前的缩进必须是实际的 Tab。目标可以是生成的文件，也可以是 `clean` 这样的操作名称。

当前的 Vector 规则不使用变量，容易直接理解：

```makefile
vector-test: vector-test.c vector.c vector.h
	gcc -ggdb -Wall -std=c99 vector-test.c vector.c -o vector-test
```

含义：要生成 `vector-test`，需要这三个文件。目标不存在，或依赖比目标更新时，Make 执行下一行命令。两个 `.c` 文件提供测试和实现；`.h` 文件提供接口声明，其变化也应触发重新构建。

### 2.2 变量、系统判断和编译选项

```makefile
UNAME_S := $(shell uname -s)
CC=gcc
CFLAGS=-ggdb -Wall -std=c99
```

- `$(shell uname -s)`：执行命令并取得输出，例如 `Linux` 或 `Darwin`。
- `:=`：立即计算右侧并保存结果。普通 `=` 是递归展开赋值，通常在引用时展开右侧。
- `CC`、`CFLAGS` 是 Make 变量；引用时写 `$(CC)`、`$(CFLAGS)`。
- `-ggdb`：生成便于 GDB 调试的信息；不加通常仍能运行，但源码和变量调试能力受限。
- `-Wall`：开启一组常用警告，不是全部警告；有助于发现可疑赋值等问题。
- `-std=c99`：选择 C99 语言标准。
- `-c`：生成 `.o` 目标文件，不执行最终链接。
- `-o 文件名`：指定输出文件名。

`.c.o:` 是 Makefile 中的后缀规则，用来从 `.c` 生成 `.o`；配方里的 `$<` 表示第一个依赖文件。

### 2.3 编译和链接有什么区别？

```text
lfsr.c       → lfsr.o       ┐
                           ├→ 链接 → lfsr
 test_lfsr.c → test_lfsr.o  ┘
```

第 32 行展开后大致为：

```bash
gcc -ggdb -Wall -std=c99 -g -o lfsr lfsr.o test_lfsr.o
```

编译器读取头文件后知道函数的参数和返回类型；链接器还需要找到函数的实际实现。

你遇到的 `undefined reference to vector_new` 等错误，是因为原来的构建命令只有 `vector-test.c`，遗漏了 `vector.c`。`#include "vector.h"` 不会自动把 `vector.c` 编译进来。

### 2.4 常用命令

```bash
make all             # 按需构建全部程序
make bit_ops
./bit_ops
make lfsr
./lfsr
make vector-test
./vector-test
make vector-memcheck
make clean           # 删除构建产物，下次需要重新编译
```

修改 `.c` 后应先 `make` 再运行；单独执行 `./程序名` 不会重新编译。当前 `all` 是首个普通目标，所以直接运行 `make` 默认构建它。

## 3. Exercise 1：位运算

### 3.1 位编号、掩码与类型

位编号从右向左，从 0 开始。以十进制 10 为例，仅展示低四位：

```text
位编号：3 2 1 0
数值：  1 0 1 0
```

| 运算符 | 含义 | 常用用途 |
| --- | --- | --- |
| `&` | 按位与 | 取位、清零 |
| `\|` | 按位或 | 设置某位为 1 |
| `^` | 按位异或 | 翻转某位 |
| `~` | 按位取反 | 生成清零掩码 |
| `<<` | 左移 | 把掩码移到目标位置 |
| `>>` | 右移 | 把目标位移到最低位 |

`&` 与 `&&`、`|` 与 `||` 不一样：前者逐位计算，后者是逻辑判断。

`1u` 是 `unsigned int` 类型的 1；普通 `1` 是 `int`。本实验按 32 位 `unsigned` 使用，`1u << n` 生成仅第 n 位为 1 的掩码。32 位 `int` 环境中，`1 << 31` 的结果不能由正的 `int` 表示，会产生未定义行为；`1u << 31` 是无符号移位，没有这个问题。移位数量仍必须在类型位宽范围内。

### 3.2 get_bit：读取某一位

```c
unsigned get_bit(unsigned x, unsigned n) {
    return (x >> n) & 1u;
}
```

右移 n 位后，原第 n 位来到最低位；与 `1u` 按位与，只留下这一位。

```text
x = 10： 1010
n = 1：  0101  （右移后）
& 1：    0001  → 返回 1
```

你写的 `x >> n & 1` 按运算符优先级可以正确解析为 `(x >> n) & 1`，但加括号更易读。占位代码 `return -1` 在无符号返回类型下会转换为很大的无符号值，不符合返回 0 或 1 的约定。

### 3.3 set_bit：设置某一位，不使用 if

```c
void set_bit(unsigned *x, unsigned n, unsigned v) {
    *x = (*x & ~(1u << n)) | (v << n);
}
```

先清零，再写入：

1. `1u << n`：只有目标位为 1。
2. `~(1u << n)`：只有目标位为 0，其余位为 1。
3. `*x & ...`：清除目标位，保留其他位。
4. `v << n`：把 0 或 1 放到目标位置。
5. 按位或合并，存回 `*x`。

为什么不能只写 `*x |= v << n`？因为当 v 为 0 时，和 0 按位或不会把原来的 1 清掉。

分支版也能表达设置/清除的算法，但不符合本题禁止条件语句的限制。

### 3.4 flip_bit：翻转某一位

```c
void flip_bit(unsigned *x, unsigned n) {
    *x ^= (1u << n);
}
```

某位与 1 异或会翻转，与 0 异或保持不变。`a ^= b` 相当于 `a = a ^ b`，这里的 a 是 `*x`。

### 3.5 为什么后两个函数使用指针？

```c
unsigned a = 10;
unsigned b = get_bit(a, 1);  // b = 1，a 不变
set_bit(&a, 2, 1);          // a = 14
flip_bit(&a, 1);            // a = 12
```

`&a` 获取地址；函数参数 `x` 保存地址；`*x` 访问地址上的整数。C 的参数按值传递，传入地址这个值后，就可以通过它修改调用者的对象。

当前自带测试只覆盖一些低位，没有覆盖 `n = 31`，因此不能靠它排除有符号移位问题。

## 4. Exercise 2：LFSR

### 4.1 uint16_t 不是“每个格子的类型”

`uint16_t` 来自 `<stdint.h>`，是恰好 16 位的无符号整数类型，可表示 0 到 65535。整个整数占 16 位，每个二进制位仍只存 0 或 1。

```c
uint16_t state = 10;
// 二进制：0000 0000 0000 1010
lfsr_calculate(&state);
```

`uint16_t *reg` 在本题中指向一个整数，不是要求传入数组。`reg` 是地址，`*reg` 是这个 16 位整数。

### 4.2 一次更新的三个步骤

本题使用原状态第 0、2、3、5 位的异或作为反馈，整体右移一位，并把反馈补到第 15 位。LSB 是最低位，MSB 是最高位。

参考实现与当前作业采用同一算法：

```c
void lfsr_calculate(uint16_t *reg) {
    unsigned bit0 = (*reg >> 0) & 1u;
    unsigned bit2 = (*reg >> 2) & 1u;
    unsigned bit3 = (*reg >> 3) & 1u;
    unsigned bit5 = (*reg >> 5) & 1u;
    unsigned feedback = bit0 ^ bit2 ^ bit3 ^ bit5;
    *reg = (*reg >> 1) | (feedback << 15);
}
```

必须先用原状态计算反馈，再更新状态。四个输入中有奇数个 1 时反馈为 1，否则为 0。

```text
旧值：[b15 b14 ... b2 b1 b0]
新值：[反馈 b15 ... b3 b2 b1]
```

例如原值为 1：反馈为 `1 ^ 0 ^ 0 ^ 0 = 1`，右移后为 0，最高位补 1，得到 32768。

### 4.3 本次容易混淆的写法

```c
*reg >> 1;                     // 只计算，结果被丢弃
*reg >> 15 = 1;                // 错误：计算结果不能放在赋值号左边
*reg = (*reg >> 1) | (temp << 15); // 正确：计算后存回
```

`*reg >> 15` 并不是“选中第 15 位，可以直接赋值”。位运算返回一个数值，不返回可以修改的某一位的引用。

### 4.4 测试输出到底表示什么？

[test_lfsr.c](test_lfsr.c) 从 1 开始，用数组记录状态是否出现过，每轮连续调用 **32 次**更新函数，再检查新状态。

因此输出的 `1` 和 `5185` 相隔 32 次更新，不是一次更新。代码只打印前 23 个状态，然后省略中间输出。

`Got 65535 numbers before cycling!` 表示记录到了 65535 个不同的状态后遇到重复。16 位共有 65536 种取值，但全零状态更新后仍是零，构成独立的固定状态；最大非零周期为 65535。32 和 65535 互质，因此对于该最大周期序列，每次跨 32 步仍能遍历全部非零状态。

**测试代码本身也有一个边界问题：** 当前 `numbers` 只分配 65535 个元素，合法下标为 0 到 65534，但 `reg` 可以等于 65535，因此 `numbers[reg]` 存在越界访问。若修正测试，应分配并初始化 **65536** 个元素。正常打印成功不能证明这段测试自身内存安全；本笔记仅指出问题，未修改测试文件。

## 5. Exercise 3：Vector 与动态内存管理

### 5.1 结构体、成员和头文件

当前结构体在 [vector.c](vector.c) 中定义：

```c
struct vector_t {
    size_t size;
    int *data;
};
```

- `size`：当前数组拥有的元素数量，合法下标为 `[0, size)`。
- `data`：指向保存整数的连续内存。
- `size_t`：用于表示对象大小、数量等的无符号整数类型；不是固定的 16 位或 32 位类型。
- `v->size` 等价于 `(*v).size`。结构体变量用 `.`，结构体指针常用 `->`。

[vector.h](vector.h) 中的接口声明应包括：

```c
typedef struct vector_t vector_t;

vector_t *vector_new();
void vector_delete(vector_t *v);
int vector_get(vector_t *v, size_t loc);
void vector_set(vector_t *v, size_t loc, int value);
```

`typedef` 给结构体类型取别名。头文件只声明结构体类型，没有公开成员布局，这叫不完整类型；调用者可以保存它的指针，但不能在不知道布局时创建该结构体对象、计算其大小或访问成员。这也是通过函数接口操作 Vector 的原因。

头文件保护宏 `#ifndef / #define / #endif` 防止内容重复展开。头文件提供声明，`.c` 提供定义，两者不能代替彼此。严格的 C99 无参数原型通常写成 `vector_new(void)`，这里展示的是作业现有接口。

### 5.2 栈上的结构体与堆上的数组是两个对象

```text
vector_t *v
    │
    ▼
结构体：size、data ──→ 整数数组 [0][0][0]...
```

结构体放在哪里，和 `data` 指向的数组放在哪里，是两个独立的问题。给数组调用 `malloc`，不会自动让结构体也进入堆内存。

### 5.3 为什么 bad_vector_new 错误？

```c
vector_t *retval, v;
retval = &v;
return retval;
```

`v` 是自动存储期的局部对象，通常放在栈上。函数返回后，它的生命周期结束；返回的地址不能再用于访问它。即使数组仍在堆上，保存数组地址的结构体已经失效。通过返回的指针访问成员属于未定义行为。

### 5.4 also_bad_vector_new 为什么不是同一种错误？

```c
vector_t v;
/* 初始化 v */
return v;
```

返回结构体的值是合法的，就像 `int x = 10; return x;`。调用者得到结构体值，不必继续访问已经结束生命周期的局部对象。

```text
局部 v：      size = 1，data = 地址 A
返回后 result：size = 1，data = 地址 A ──→ 仍有效的堆内存
```

它不够理想的地方要结合本实验设计理解：

- 按值传递结构体在语义上复制全部成员，较大的结构体可能有更多复制和存储开销；编译器可能优化，不能断言一定复制或一定更慢。本例结构体很小。
- 复制指针成员只是浅拷贝，不复制数组，多个结构体副本会共享同一块数据，需要明确所有权。
- 本实验的接口返回 `vector_t *`，销毁函数释放结构体和数组；按值返回的结构体不能不加区分地交给该销毁函数。

不要把答案写成“返回局部变量一定不合法”。准确区分 **返回地址** 和 **返回值**。

### 5.5 vector_new：两次申请、两次检查

```c
vector_t *vector_new() {
    vector_t *retval = malloc(sizeof(vector_t));
    if (retval == NULL) {
        allocation_failed();
    }

    retval->size = 1;
    retval->data = malloc(sizeof(int));
    if (retval->data == NULL) {
        free(retval);
        allocation_failed();
    }

    retval->data[0] = 0;
    return retval;
}
```

先分配结构体并确认成功，才能访问 `retval->...`。第二次分配一个整数的空间；如果失败，先释放已经分配的结构体，避免留下无法使用的对象。

本文件的 `allocation_failed()` 调用 `exit(1)` 结束程序，所以失败后不会继续解引用空指针。`malloc` 不初始化内容，必须显式把初始元素设为 0。在 C 中，包含 `<stdlib.h>` 后不需要强制转换 `malloc` 的返回值。

特别注意：`if (retval = NULL)` 是赋值，会覆盖指针且条件为假；检查应写 `==`。

### 5.6 vector_get：读越界返回 0

```c
int vector_get(vector_t *v, size_t loc) {
    if (v == NULL) {
        fprintf(stderr, "vector_get: passed a NULL vector.\n");
        abort();
    }
    if (loc < v->size) {
        return v->data[loc];
    }
    return 0;
}
```

`size = 3` 时只有 0、1、2 合法。越界返回 0 是本实验的接口约定，不代表 C 数组越界读取会自动得到 0，也不代表读操作会扩容。

`fprintf(stderr, ...)` 把错误信息输出到标准错误；`abort()` 异常终止程序。

### 5.7 vector_delete：按分配块释放，不按元素释放

```c
void vector_delete(vector_t *v) {
    free(v->data);
    free(v);
}
```

先释放数组，再释放结构体。反过来会在结构体已经释放后读取它的 `data` 成员。

`v->data[i]` 是整数，不是动态分配得到的指针，因此不能 `free(v->data[i])`。`&v->data[i]` 通常是内存块内部的地址，也不能逐个释放。一整块连续数组只需一次 `free(v->data)`。

只有元素本身是指针，并且分别进行过独立分配时，才可能需要循环释放各元素指向的块。

当前函数要求 v 非空。如需允许删除空指针，应先判断 `if (v == NULL) return;`。虽然 `free(NULL)` 安全，但 `NULL->data` 不安全。函数也不会自动把调用者的指针设为 NULL，释放后不可继续使用或重复释放。

### 5.8 vector_set：扩容、补零、写入

以下是适用于本实验普通下标的参考实现：

```c
void vector_set(vector_t *v, size_t loc, int value) {
    if (v == NULL) {
        fprintf(stderr, "vector_set: passed a NULL vector.\n");
        abort();
    }

    if (loc >= v->size) {
        size_t new_size = loc + 1;
        int *new_data = realloc(v->data, new_size * sizeof(int));
        if (new_data == NULL) {
            allocation_failed();
        }
        v->data = new_data;

        for (size_t i = v->size; i < new_size; i++) {
            v->data[i] = 0;
        }
        v->size = new_size;
    }

    v->data[loc] = value;
}
```

例如原数组 `[10, 20]`，执行 `vector_set(v, 4, 99)` 后得到 `[10, 20, 0, 0, 99]`，size 为 5。

关键顺序：

1. 当 `loc >= size` 时扩容；新数量为 `loc + 1`。
2. 分配的是字节数，因此乘以 `sizeof(int)`。
3. `realloc` 可能在原地址扩展，也可能移动内存；成功后必须保存返回的新指针。
4. 新增部分不保证为零，必须初始化。
5. 用旧 size 作为补零起点，完成后再更新 size。
6. 写入放在条件外，使不需要扩容的情况也能更新元素。

不要直接 `v->data = realloc(...)`：当请求大小非零且分配失败时，原块仍有效，但返回 NULL；直接赋值会丢失原块地址。临时指针避免这个问题。成功后也不要再释放旧地址，内存迁移由 `realloc` 处理。

边界补充：以上和当前作业版本都没有处理极大 `loc` 导致的整数溢出。通用实现还应在 `loc + 1` 和乘以 `sizeof(int)` 前检查是否超过 `SIZE_MAX` 的范围；普通测试通过不能证明任意大小的请求都安全。

## 6. 测试与 Memcheck

### 6.1 功能测试覆盖了什么？

`vector-test.c` 检查创建、删除、再次创建，读取初始和越界位置，写入远处位置、覆盖已有值，及新增空隙为零。它打印预期值和实际值，需要观察两者是否一致；`Test complete.` 本身不是断言全部正确。

`test_bit_ops.c` 同样主要通过打印结果反馈，不应仅凭进程退出码为 0 判断所有比较都成功。

### 6.2 make vector-memcheck 不是寻找同名文件

当前 Linux 环境中，这个目标最终执行：

```bash
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./vector-test
```

| 名称或参数 | 意义 |
| --- | --- |
| `vector-memcheck` | Makefile 中执行检查的目标 |
| `valgrind` | 启动检查工具的程序 |
| `--tool=memcheck` | 选择内存访问与分配检查工具 |
| `--leak-check=full` | 输出详细的内存泄漏信息 |
| `--track-origins=yes` | 帮助追踪未初始化值的来源 |
| `./vector-test` | 在工具下运行的被检查程序 |

本 Makefile 的 Darwin 分支还带有 `--dsymutil=yes`，属于其历史 macOS 调试信息处理配置；WSL 使用 Linux 分支，不需要该参数。

关注非法读写、使用未初始化值、重复释放、释放后使用、内存泄漏等报告。除了错误数量，也应查看 HEAP/LEAK SUMMARY，确认退出时没有遗漏的分配。检查只能覆盖本次实际执行的路径。

### 6.3 本次实际运行结果

运行了 `make all`、三个功能测试，以及 `make vector-memcheck`：

- `make all`：目标已是最新，无需重新构建。
- `./bit_ops`：现有测试项全部 `correct`。
- `./lfsr`：输出周期计数 65535。
- `./vector-test`：打印的预期值和实际值匹配。
- Vector 的 Memcheck：退出时 0 bytes / 0 blocks，9 次分配、9 次释放，0 errors。

这些结果不消除第 1 节的题目限制问题和第 4 节的 LFSR 测试越界问题。本次没有对 LFSR 测试运行内存检查，也没有修改作业源码。

## 7. 本次疑问速查

| 疑问或错误写法 | 复习答案 |
| --- | --- |
| `1u` 是什么？ | 无符号整数常量 1，适合构造位掩码 |
| `uint16_t` 是数组吗？ | 不是，是一个 16 位无符号整数 |
| `*reg >> 1;` 会改变原数吗？ | 不会，必须把结果赋回 |
| 可以给 `*reg >> 15` 赋值吗？ | 不行，它是计算结果，不是可修改对象 |
| 局部变量可以 return 吗？ | 可以按值返回，不能返回地址后继续访问已失效对象 |
| `malloc` 会自动清零吗？ | 不会；本实验需要显式初始化 |
| 为什么 Vector 两次 malloc？ | 结构体与数组是两个独立对象 |
| 为什么不能逐个 free 整数？ | free 释放动态分配块，整数元素不是独立分配块 |
| `loc == size` 要扩容吗？ | 要，最后合法下标是 size - 1 |
| 只 include 头文件就能调用实现吗？ | 编译时知道声明，最终还需要链接实现 |
| 没有 memcheck 文件怎么办？ | 它是 Valgrind 工具名称，不是作业文件 |
| 测试通过就可以提交了吗？ | 还要检查题目约束、边界与内存报告 |

## 8. 复习自测

尝试不看答案口述：

1. 第 n 位为什么能通过右移再 `& 1u` 取得？
2. 为什么设置某位之前需要先清零？
3. 为什么异或可以只翻转选中的一位？
4. LFSR 为什么必须在更新状态前计算反馈？
5. `return v` 和 `return &v` 的生命周期区别是什么？
6. Vector 的结构体和数组分别由谁分配、谁释放？
7. `realloc` 为什么需要临时指针，扩容后为什么需要补零？
8. `vector_get` 越界时为何不能直接读取 `data[loc]`？
9. 编译通过但出现 `undefined reference`，应检查哪一步？
10. 为什么有限测试成功仍可能遗漏第 31 位和数组边界问题？

复习时优先画出“指针 → 对象”的关系，再判断每一步是在修改地址、修改对象，还是只计算一个临时结果。
