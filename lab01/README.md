# Lab 01 复习整理：C 基础、调试与指针

本实验把 C 程序的基本工作流程串起来：编译运行 → 理解控制流 → 用 GDB 观察执行过程 → 给交互程序提供输入 → 排查内存错误 → 用指针实现链表判环。

## 1. 文件与练习对应关系

| 练习 | 文件 | 本次做了什么 |
| --- | --- | --- |
| Exercise 1：控制流 | [eccentric.c](eccentric.c) | 把四个宏都设为 `3`，得到指定输出 |
| Exercise 2：GDB | [hello.c](hello.c)、[gdb.txt](gdb.txt) | 观察指针自增，整理调试命令 |
| Exercise 3：输入重定向 | [interactive_hello.c](interactive_hello.c)、[input.txt](input.txt) | 从文件读取名字，方便重复调试 |
| Exercise 4：内存错误 | [segfault_ex.c](segfault_ex.c)、[no_segfault_ex.c](no_segfault_ex.c) | 分析越界写、越界读，在注释中记录正确循环边界 |
| Exercise 5：链表判环 | [ll_cycle.c](ll_cycle.c)、[ll_cycle.h](ll_cycle.h)、[test_ll_cycle.c](test_ll_cycle.c) | 实现快慢指针，检查空链表、无环链表和有环链表 |

**当前实现需要注意的两点：** `gdb.txt` 第 3、4 题的 `step` / `next` 写反了；`no_segfault_ex.c` 的正确循环仍只是注释，实际执行的代码保留了越界问题。本文给出正确理解，未修改这些文件。

## 2. 编译与运行：源码不等于可执行文件

以下命令都在 `labs_hive/lab01` 目录运行：

```bash
gcc -g -o hello hello.c
./hello
gdb ./hello
```

- `gcc`：把 C 源码编译、链接为可执行文件。
- `-o hello`：指定输出文件名；省略时通常生成 `a.out`。
- `-g`：生成调试信息，方便 GDB 对应源代码行和变量。
- `./hello`：运行当前目录中的程序。
- `gdb ./hello`：调试编译后的程序。

**每次修改源码后，都要重新编译。** 直接运行旧的可执行文件不会体现源码变化。调试时也可加 `-O0` 关闭优化，让执行过程更容易对应源码。

## 3. Exercise 1：四个宏如何控制输出

题面要求只改四个宏的值，得到指定文本，并思考最少需要几种不同的值。当前实现是：

```c
#define V0 3
#define V1 3
#define V2 3
#define V3 3
```

| 宏 | 对应代码 | 为什么设成 `3` |
| --- | --- | --- |
| `V0` | `for (a = 0; a < V0; a++)` | `a` 依次为 0、1、2，打印三次 `Happy` |
| `V1` | `switch (V1)` | 进入 `case 3`，打印 `Yoshua` 后 `break` |
| `V2` | `if (V2)` | C 中非零为真，选择 `BEARS` 分支 |
| `V3` | `(V3 == 3) ? "Go" : "Boo"` | 条件成立，选择 `Go` |

因此四个宏只需要 **1 种不同的值**。`V2` 可以是任意非零整数，不局限于 `1`。

运行方式：

```bash
gcc -g -o eccentric eccentric.c
./eccentric
```

输出为：

```text
Berkeley eccentrics:
====================
Happy Happy Happy
Yoshua
Go BEARS!
```

上面的展示省略了第三行末尾的一个空格；实际代码每次打印的是 `"Happy "`。

复习重点：`#define` 是预处理替换，不是普通变量赋值；`switch` 匹配后会继续向下执行，直到遇到 `break` 或结束。例如 `case 0` 虽然也打印 `Yoshua`，却会继续打印 `Triangle Man`，所以不符合目标。

## 4. Exercise 2：用 GDB 看清指针与执行过程

`hello.c` 的核心代码是：

```c
int count = 0;
int *p = &count;
for (i = 0; i < 10; i++) {
    (*p)++;
}
```

`&count` 取得地址，`p` 保存地址，`*p` 访问这个地址对应的整数。因此 `(*p)++` 修改的是 `count`，循环结束后 `count == 10`，`p` 仍指向 `count`。

### 常用命令与九道题的正确对应

| 题号 / 用途 | 命令示例 | 说明 |
| --- | --- | --- |
| 1. 设置程序参数 | `set args arg1 arg2` | 设置之后运行时传给程序的参数 |
| 2. 设置断点 | `break main` / `b hello.c:12` | 到指定函数或源码行时暂停 |
| 3. 执行下一行，不进入被调用函数 | `next` / `n` | 把函数调用作为一步执行 |
| 4. 进入被调用函数 | `step` / `s` | 进入函数内部逐步调试 |
| 5. 继续运行 | `continue` / `c` | 继续到下一个断点或程序结束 |
| 6. 查看值 | `print count` / `p *p` | 也可以求值表达式，如 `p 1+2` |
| 7. 自动显示 | `display count` | 后续每次停下时显示 |
| 8. 查看局部变量 | `info locals` | 函数参数可另用 `info args` 查看 |
| 9. 退出 | `quit` / `q` | 退出调试器 |
| 启动程序 | `run` / `r` | 开始执行，也可以重新运行 |
| 从当前函数返回 | `finish` | 误入 `printf` 等函数时很有用 |

**纠错：当前 `gdb.txt` 的第 3 题应是 `next`，第 4 题应是 `step`。** 网站还提醒自动评分只接受命令本身，不要照抄表格里的示例参数；提交前需要单独检查答案格式。参见[题面的 Checkoff 部分](https://www.learncs.site/docs/curriculum-resource/cs61c/labs/lab01#checkoff)。

可以这样复现观察：

```text
gdb ./hello
(gdb) break main
(gdb) run
(gdb) next
... 用 next 执行到 count 和 p 初始化完成 ...
(gdb) print p
(gdb) print *p
(gdb) display count
(gdb) break hello.c:12
(gdb) continue
(gdb) print count
```

最后应看到 `10`。这里的第 12 行是当前文件中的 `printf`；代码行数变化后应调整断点。CGDB 在 GDB 外增加源码界面，`Esc` 切到源码窗口，`i` 返回命令窗口。

### `(*p)++` 与 `*p++` 的区别

| 表达式 | 修改谁 | 表达式的值 |
| --- | --- | --- |
| `(*p)++` | `p` 指向的整数 | 自增前的整数值 |
| `++(*p)` | `p` 指向的整数 | 自增后的整数值 |
| `*p++`，即 `*(p++)` | 指针 `p` | 原位置的整数值 |
| `*++p`，即 `*(++p)` | 指针 `p` | 移动后位置的整数值 |

指针加一按所指类型的大小移动，不一定只移动一个字节。这里 `p` 指向单个 `count`，不能随意把它后面的内存当作另一个合法整数解引用。

## 5. Exercise 3：把输入文件接到标准输入

`interactive_hello.c` 用 `fgets(a_word, MAX_LEN, stdin)` 读取名字。`MAX_LEN` 是 80，成功读取时最多存入 79 个字符，再补字符串结束符 `\0`；如果读到了换行符，也会保留它。与按空白拆分输入不同，名字中的空格可以被一起读入。

当前 [input.txt](input.txt) 内容为 `reluvieir CHLOE`，可重复使用：

```bash
gcc -g -o int_hello interactive_hello.c
./int_hello < input.txt
gdb ./int_hello
```

在 GDB 内输入：

```text
(gdb) break main
(gdb) run < input.txt
```

`< input.txt` 把文件作为程序的标准输入。`set args input.txt` 只是传递一个命令行参数，并不会让 `fgets` 读取该文件。

当前输入文件末尾没有换行，因此实际输出会出现 `Hey, reluvieir CHLOEI just really wanted...`，名字与下一句连在一起。这是因为格式串 `%s` 后没有额外写 `\n`；文件末尾若带换行，显示效果就会不同。

## 6. Exercise 4：不崩溃也可能发生越界

### `segfault_ex.c`：无限循环导致越界写

```c
int a[20];
for (int i = 0; ; i++) {
    a[i] = i;
}
```

数组合法下标是 `0` 到 `19`。循环没有终止条件，从 `i == 20` 开始发生越界写，属于未定义行为，可能破坏栈上其他数据并最终崩溃。不能依赖它一定在某个下标触发段错误。

### `no_segfault_ex.c`：把字节数当成元素个数

当前执行代码中：

```c
int a[5] = {1, 2, 3, 4, 5};
unsigned total = 0;
for (int j = 0; j < sizeof(a); j++) {
    total += a[j];
}
```

`sizeof(a)` 返回整个数组的**字节数**。当 `sizeof(int) == 4` 时，它是 20，循环却把 20 当成了元素数，于是从 `a[5]` 开始越界读。`total` 已初始化为零，问题来自后续累加了不属于该数组的数据。

源码注释已经指出应使用 `sizeof(a) / sizeof(a[0])`。更完整的修正写法如下（仅供复习，当前源文件尚未采用）：

```c
for (size_t j = 0; j < sizeof(a) / sizeof(a[0]); j++) {
    total += a[j];
}
printf("sum of array is %u\n", total);
```

正确求和结果为 `15`。`size_t` 与 `sizeof` 的结果类型一致；`total` 是 `unsigned int`，输出应使用 `%u`。数组元素个数公式只适用于这里仍是数组的 `a`：函数形参中的 `int a[]` 会调整为指针，不能用同一方法求原数组长度。

### 如何借助 Valgrind

```bash
gcc -g -O0 -o segfault_ex segfault_ex.c
gcc -g -O0 -o no_segfault_ex no_segfault_ex.c
valgrind ./segfault_ex
valgrind --track-origins=yes ./no_segfault_ex
```

读报告时关注非法读写的位置、未初始化值的使用位置及调用栈。**报告中暴露错误的位置不一定是错误的源头**：问题可能在求和时产生，到输出时才显现。Valgrind 也不能保证识别每一次栈数组越界。

程序不崩溃，只能说明这次运行没有以崩溃结束。越界地址可能仍处于可访问区域，所以不会立刻触发操作系统保护，但 C 语言层面已经是未定义行为；输出可能随编译方式、运行环境变化，也可能暂时看起来正常。这与“内存泄漏”是不同的问题。

## 7. Exercise 5：快慢指针判断链表是否有环

[ll_cycle.h](ll_cycle.h) 定义了节点：

```c
typedef struct node {
    int value;
    struct node *next;
} node;
```

`node *` 指向一个节点，`p->next` 等价于 `(*p).next`。判环要比较**节点地址**；不同节点的 `value` 相同，并不代表链表有环。

当前 [ll_cycle.c](ll_cycle.c) 实现的逻辑为：

```c
int ll_has_cycle(node *head) {
    if (!head) return 0;
    node *slow = head;
    node *fast = head;
    while (slow && fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return 1;
        }
    }
    return 0;
}
```

返回 `1` 表示有环，返回 `0` 表示无环。每轮慢指针走一步，快指针走两步。

- **空链表**：`head == NULL` 时直接返回 `0`。
- **安全地走两步**：先检查 `fast`，再检查 `fast->next`；`&&` 从左到右短路求值，避免解引用空指针。
- **允许走到空指针**：读取 `fast->next->next` 可以得到 `NULL`，这本身合法；下一轮停止即可。
- **先移动，再比较**：两个指针初始都在 `head`，此时直接比较会把所有非空链表都判成有环。
- **`slow` 检查是冗余保护**：在合法、结构不变的链表中，只要快指针还能前进两步，慢指针就能安全前进一步；保留它不影响正确性。

### 为什么一定能判定

无环链表中，快指针最终到达 `NULL`，或到达 `next == NULL` 的尾节点，循环结束。

有环链表中，两个指针最终都会进入环。若环长为 `L`，每轮快指针相对慢指针多走一步，它们的相对位置按模 `L` 不断变化，最终会相遇。若入环前有 `μ` 个节点，时间复杂度为 `O(μ + L)`；无环时为 `O(n)`。算法只保存两个指针，额外空间为 `O(1)`，不会修改链表。

### 测试对应哪些边界

```bash
gcc -g -o test_ll_cycle test_ll_cycle.c ll_cycle.c
./test_ll_cycle
echo $?
```

需要同时编译测试文件和实现文件：前者调用函数，后者提供函数定义。

| 测试 | 链表形状 | 期望 |
| --- | --- | --- |
| first | `0 → 1 → 2 → 3 → NULL` | 无环 |
| second | `4 → … → 10 → 4` | 有环，回到头节点 |
| third | `11 → … → 17 → 14` | 有环，环前有一段路径 |
| fourth | `18 → 18` | 有环，单节点自环 |
| fifth | `19 → … → 23 → NULL` | 无环 |
| length-zero | `NULL` | 无环 |

表中数字是 `nodes` 数组的下标，不是节点值；测试实际把节点值都初始化成了零。这也说明必须比较地址。全部通过时，测试程序退出码为 `0`。

## 8. 本次整理时的验证

从当前源码重新编译到临时目录后，实际核对了：

- `eccentric` 输出符合上述结果。
- GDB 在 `hello.c` 的输出行停下时，`count == 10`。
- `int_hello < input.txt` 能读取完整名字，并复现缺少换行导致的文本相连。
- `test_ll_cycle` 的六组测试全部通过，退出码为 `0`。

内存错误部分是源码分析和复习命令，本次没有执行 Valgrind，因此没有把示例诊断描述当作本机实测报告。上述验证也不代表已经通过课程线上自动评分。

## 9. 复习自测

1. 为什么 `V1 = 0` 不能得到目标输出，而 `V2 = 3` 可以当作真？
2. `next` 与 `step` 有什么区别？误入库函数后怎样返回？
3. `(*p)++` 与 `*p++` 分别改变什么？
4. `run < input.txt` 与 `set args input.txt` 有什么区别？
5. 为什么 `sizeof(a)` 不等于数组元素数？什么时候元素数公式也会失效？
6. 为什么数组越界不一定触发段错误？
7. 为什么必须先检查 `fast` 再访问 `fast->next`？
8. 为什么判环时先移动指针、再比较地址？为什么不能比较 `value`？
9. 为什么快慢指针最终会相遇，且只需要常数额外空间？
