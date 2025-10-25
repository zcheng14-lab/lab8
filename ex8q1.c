/*
 * src/ex8q1.c
 *
 * 这是 Lab 8 的解决方案。
 * 这个程序读取一个正则表达式和多个测试字符串。
 * 它会判断每个字符串是否与正则表达式匹配。
 *
 * 编译: gcc -Wall -Werror -std=c99 src/ex8q1.c -o ex8q1
 * 运行: ./ex8q1 < Testcases/1-1-input.txt
 */

[cite_start]// 1. 包含允许的库文件 [cite: 7]
// stdio.h: 用于标准输入输出, 比如 printf (打印) 和 fgets (读取一行)
#include <stdio.h>
// stdlib.h: 用于标准库, 比如 exit (退出程序), 但我们这里主要用它提供的 EXIT_FAILURE
#include <stdlib.h>
// stdbool.h: 让我们能使用 true 和 false 这样的布尔值
#include <stdbool.h>
// string.h: 用于字符串操作, 比如 strlen (获取长度), strcmp (比较), strncpy (复制)
#include <string.h>

// 定义一个常量, 用于设置缓冲区(buffer)的最大长度
[cite_start]// 正则表达式和字符串最大都是 255[cite: 1], 我们加 1 为 '\0' (字符串结束符)
// 再加 1 以防万一 (比如行尾的换行符)
#define MAX_LEN 257

// 声明 (Declare) 我们的核心函数, 这样 main 函数在它之前也能"认识"它
// C 语言需要在使用函数之前知道函数长什么样
bool matches(const char* regex, const char* text);
int get_last_sub_expr_start(const char* regex, int len);

/*
 * 这是一个辅助函数, 用于删除从 fgets 读入的字符串末尾的换行符 '\n'
 * * char* str: 这是一个 "C 字符串" (一个指向字符数组的指针)
 * * `fgets` 会连同行尾的换行符一起读进来, 这会干扰我们的匹配
 * 所以我们需要找到它并用字符串结束符 '\0' 替换掉
 */
void remove_newline(char* str) {
    // `strlen` (string length) 返回字符串的长度, 不包括最后的 '\0'
    int len = strlen(str);
    
    // 检查最后 G 个字符是不是换行符
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // 是的话, 就替换成字符串结束符
    }
}

/*
 * 这是一个辅助函数, 用于在"连接"(Concatenation)操作中找到分割点
 *
 * 它会找到 R = ST 中的 S 和 T 的分界线
 * 它通过从右到左扫描, 找到最后一个完整的子表达式 (T) 的起始索引
 * * 比如 (a|b)c*
 * T 是 "c*", 它从索引 5 开始 (c 是 5, * 是 6) 
 * 不对, T 是 "c*", 它从索引 5 开始, 它的起始索引是 5
 * (a|b) 是 S
 *
 * 比如 (a*b)(c|d)
 * T 是 "(c|d)", 它的起始索引是 5
 *
 * 比如 a*b*
 * T 是 "b*", 它的起始索引是 2
 */
int get_last_sub_expr_start(const char* regex, int len) {
    // 1. 如果最后一个字符是 ')', T 是一个 (...) 表达式
    if (regex[len - 1] == ')') {
        int level = -1; // 从 -1 开始, 因为我们从 ')' 开始
        for (int i = len - 2; i >= 0; i--) { // 从右往左找
            if (regex[i] == ')') level--; // 遇到 ')' 嵌套加深
            if (regex[i] == '(') level++; // 遇到 '(' 嵌套变浅
            if (level == 0) {
                // 找到了匹配的 '(', 这个 (...) 表达式从 i 开始
                return i;
            }
        }
    } 
    // 2. 如果最后一个字符是 '*', T 是一个 S* 表达式
    else if (regex[len - 1] == '*') {
        // T 的主体 S 可能是 (...) 或单个字符
        if (regex[len - 2] == ')') { // T 是 (S)*
            int level = -1;
            for (int i = len - 3; i >= 0; i--) { // 从 ')' 往前找
                if (regex[i] == ')') level--;
                if (regex[i] == '(') level++;
                if (level == 0) {
                    // 找到了 (S)* 的起始 '(', 索引是 i
                    return i;
                }
            }
        } else { // T 是 s* (s 是单个字符)
            // s* 从 s 开始, 索引是 len - 2
            return len - 2;
        }
    }
    // 3. T 是一个单独的字符 (s 或 _)
    else {
        return len - 1; // 单个字符从 len - 1 开始
    }
    
    // 如果 R 本身就是一个完整的表达式 (比如 (a|b) 或 a*), 那么分割点就是 0
    return 0;
}


/*
 * 核心递归函数: 检查正则表达式 regex 是否匹配字符串 text
 *
 * const char* regex: 指向当前要匹配的正则表达式字符串
 * const char* text: 指向当前要匹配的文本字符串
 * * 返回: true (匹配) 或 false (不匹配)
 *
 * [cite_start]这个函数严格按照 README 中的优先级来解析 [cite: 1]
 * 1. Union (并集 |)
 * 2. Concatenation (连接 ST)
 * 3. Star (星号 *)
 * 4. Parentheses (括号 (S)) / Base Case (S)
 *
 * 它通过从右到左查找最低优先级的操作符来工作
 */
bool matches(const char* regex, const char* text) {
    
    int regex_len = strlen(regex);
    int text_len = strlen(text);
    
    // --- 0. 递归的真正基础: 如果正则表达式是空的
    // 只有当文本也是空的时候, 才算匹配
    if (regex_len == 0) {
        return (text_len == 0);
    }
    
    [cite_start]// --- 1. 检查并集 (Union, |) [cite: 1] ---
    // 这是最低的优先级, 所以我们先找它
    // 我们需要从右到左找, 确保它不在括号里
    int level = 0; // 括号嵌套层级
    for (int i = regex_len - 1; i >= 0; i--) {
        if (regex[i] == ')') level++; // 进入一层括号
        if (regex[i] == '(') level--; // 退出一层括号
        
        // 如果我们不在括号里 (level == 0) 并且找到了 '|'
        if (level == 0 && regex[i] == '|') {
            // 我们找到了一个 S|T 结构
            // S 是 | 左边的所有内容
            // T 是 | 右边的所有内容
            
            // `char S[MAX_LEN];` 在栈 (stack) 上分配 257 字节的内存
            // 这是一种静态内存分配, 不像 `malloc` (动态分配)
            // 它会在函数返回时自动释放, 不会造成内存泄漏
            char S[MAX_LEN];
            char T[MAX_LEN];
            
            // `strncpy` (string N copy) 复制最多 i 个字符
            // (从 regex 复制到 S)
            strncpy(S, regex, i);
            S[i] = '\0'; // `strncpy` 不一定会加 '\0', 我们必须手动添加!
            
            // `strcpy` (string copy) 复制 | 后面的所有内容
            // (从 regex + i + 1 复制到 T)
            // `regex + i + 1` 是一个指针运算, 指向 | 符号的下一个字符
            strcpy(T, regex + i + 1);
            
            [cite_start]// 递归检查: S 匹配 text 或者 T 匹配 text [cite: 1]
            return matches(S, text) || matches(T, text);
        }
    }
    
    [cite_start]// --- 2. 检查连接 (Concatenation, ST) [cite: 1] ---
    // 如果没有找到顶层的 '|', 那么它可能是个连接
    // 比如 (a|b)c* 或者 a*b*
    // 我们找到最后一个子表达式 T, 它之前的所有内容就是 S
    int split_point = get_last_sub_expr_start(regex, regex_len);
    
    // 如果分割点不是 0, 说明 regex 是 S 和 T 的连接
    if (split_point > 0) {
        char S[MAX_LEN];
        char T[MAX_LEN];
        
        strncpy(S, regex, split_point);
        S[split_point] = '\0';
        
        strcpy(T, regex + split_point);
        
        [cite_start]// 我们必须尝试 text 的所有分割 (w = xy) [cite: 1]
        // i 是分割点, 代表 x 的长度
        for (int i = 0; i <= text_len; i++) {
            char x[MAX_LEN];
            char y[MAX_LEN];
            
            strncpy(x, text, i);
            x[i] = '\0'; // x 是 text 的前 i 个字符
            
            strcpy(y, text + i); // y 是 text 剩下的字符
            
            // 递归检查: S 匹配 x 并且 T 匹配 y
            if (matches(S, x) && matches(T, y)) {
                return true; // 找到一个成功的分割
            }
        }
        return false; // 尝试了所有分割, 都失败了
    }

    [cite_start]// --- 3. 检查星号 (Star, S*) [cite: 1] ---
    // 如果它不是 | 也不是 ST, 它可能是 S*
    if (regex[regex_len - 1] == '*') {
        // S 是 * 之前的所有内容
        char S[MAX_LEN];
        strncpy(S, regex, regex_len - 1);
        S[regex_len - 1] = '\0';
        
        // S* 的匹配逻辑:
        // 1. 匹配 0 次 (即 S* 匹配空字符串)
        // 注意: "text" 是我们当前正在看的子字符串
        if (text_len == 0) {
            return true; [cite_start]// [cite: 1]
        }
        
        [cite_start]// 2. 匹配 1 次或多次 [cite: 1]
        // 尝试将 text 分割成 x 和 y, 其中 x 非空 (i=1 开始)
        // x: S 匹配的部分
        // y: S* (即整个 regex) 匹配的剩余部分
        for (int i = 1; i <= text_len; i++) {
            char x[MAX_LEN];
            char y[MAX_LEN];
            
            strncpy(x, text, i);
            x[i] = '\0';
            
            strcpy(y, text + i);
            
            // 递归: S 匹配 x 吗? 并且 S* (即 regex) 匹配 y 吗?
            if (matches(S, x) && matches(regex, y)) {
                return true; // 找到一个成功的 "1次" 匹配
            }
        }
        
        // 0 次 和 1+ 次都失败了
        return false;
    }
    
    [cite_start]// --- 4. 检查括号 (Parentheses, (S)) [cite: 1] ---
    // 如果它被一对括号包裹
    if (regex[0] == '(' && regex[regex_len - 1] == ')') {
        // S 是括号里的内容
        char S[MAX_LEN];
        strncpy(S, regex + 1, regex_len - 2);
        S[regex_len - 2] = '\0';
        
        // 递归: 检查 S 是否匹配 text
        return matches(S, text);
    }
    
    [cite_start]// --- 5. 基础情况 (Base Case, s 或 _) [cite: 1] ---
    // 如果它不是以上任何一种, 它必须是一个单独的字符
    if (regex_len == 1) {
        // 1. 如果 regex 是 '_' (空串符号)
        if (regex[0] == '_') {
            // 它只匹配空文本
            return (text_len == 0); [cite_start]// [cite: 1]
        }
        // 2. 如果 regex 是普通字符 's'
        else {
            // 它只匹配包含 G 个相同字符的文本
            return (text_len == 1 && regex[0] == text[0]);
        }
    }
    
    // 如果所有检查都失败了 (比如一个无效的 regex 子串)
    return false;
}

// --- 主函数 ---
int main() {
    // 为正则表达式和文本行分配缓冲区
    char regex_line[MAX_LEN];
    char text_line[MAX_LEN];
    
    [cite_start]// 1. 读取第一行: 正则表达式 [cite: 1]
    // `fgets` (file get string) 从标准输入 (stdin) 读取最多 MAX_LEN-1 个字符
    // 它会停止在换行符 '\n' 或文件结尾
    if (fgets(regex_line, MAX_LEN, stdin) == NULL) {
        // 如果第一行就读不到东西, 说明输入为空
        return EXIT_FAILURE; // 异常退出
    }
    remove_newline(regex_line); // 删除末尾的 '\n'
    
    [cite_start]// 2. 循环读取后续的文本行 [cite: 1]
    // `while` 循环会一直执行, 直到 `fgets` 返回 NULL (表示文件结束)
    while (fgets(text_line, MAX_LEN, stdin) != NULL) {
        remove_newline(text_line); // 删除末尾的 '\n'
        
        char* text_to_match; // 用一个指针来指向我们要匹配的文本
        
        [cite_start]// 检查输入的文本是不是 "_" (代表空字符串) [cite: 1]
        // `strcmp` (string compare) 比较两个字符串
        // 如果它们完全 G 样, 返回 0
        if (strcmp(text_line, "_") == 0) {
            // 如果是, 我们让指针指向一个空字符串 ""
            text_to_match = "";
        } else {
            // 否则, 指针就指向我们刚读入的 `text_line`
            text_to_match = text_line;
        }
        
        [cite_start]// 3. 调用核心函数并打印结果 [cite: 1]
        if (matches(regex_line, text_to_match)) {
            printf("1\n"); // 匹配成功, 打印 1
        } else {
            printf("0\n"); // 匹配失败, 打印 0
        }
    }

    // 正常退出
    return 0;
}