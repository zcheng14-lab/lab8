
#include <stdio.h>      // 标准输入输出库，用于printf、fgets等函数
#include <stdlib.h>     // 标准库，用于EXIT_FAILURE等常量
#include <stdbool.h>    // 布尔类型库，提供bool、true、false
#include <string.h>     // 字符串处理库，用于strlen、strcpy等函数

#define MAX_LEN 257     // 定义字符串最大长度为257（包括结束符'\0'）


// 函数声明：判断正则表达式reg ex是否匹配文本text
bool matches(const char* regex, const char* text);


// 函数声明：获取正则表达式中最后一个子表达式的起始位置
int get_last_sub_expr_start(const char* regex, int len);


// 函数功能：移除字符串末尾的换行符'\n'
// 参数：str - 需要处理的字符串
void remove_newline(char* str) {
    int len = strlen(str);  // 获取字符串长度
    
    // 如果字符串长度大于0且最后一个字符是换行符
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';  // 将换行符替换为字符串结束符
    }
}


// 函数功能：找到正则表达式中最后一个子表达式的起始位置
// 这个函数用于处理连接运算（concatenation）时分割正则表达式
// 参数：regex - 正则表达式字符串
//       len - 正则表达式的长度
// 返回值：最后一个子表达式的起始索引
int get_last_sub_expr_start(const char* regex, int len) {
    
    // 情况1：如果最后一个字符是右括号')'
    // 例如：regex = "abc(def)"，需要找到对应的左括号'('
    if (regex[len - 1] == ')') {
        int level = -1;  // level用来追踪括号的嵌套层级，从-1开始
        // 从倒数第二个字符开始向前遍历
        for (int i = len - 2; i >= 0; i--) {
            if (regex[i] == ')') level--;  // 遇到右括号，层级减1
            if (regex[i] == '(') level++;  // 遇到左括号，层级加1
            if (level == 0) {              // 当层级为0时，找到了匹配的左括号
                return i;                   // 返回左括号的位置
            }
        }
    } 
   

    // 情况2：如果最后一个字符是星号'*'（表示重复运算）
    // 例如：regex = "abc*" 或 "ab(cd)*"
    else if (regex[len - 1] == '*') {
        // 情况2.1：星号前面是右括号，例如："(abc)*"
        if (regex[len - 2] == ')') {
            int level = -1;  // 同样需要找到匹配的左括号
            // 从倒数第三个字符开始向前遍历
            for (int i = len - 3; i >= 0; i--) {
                if (regex[i] == ')') level--;
                if (regex[i] == '(') level++;
                if (level == 0) {
                    return i;  // 返回左括号的位置
                }
            }
        } else {
            // 情况2.2：星号前面是单个字符，例如："abc*"
            return len - 2;  // 返回星号前面那个字符的位置
        }
    }
    
    // 情况3：最后一个字符是普通字符（字母、数字等）
    else {
        return len - 1;  // 直接返回最后一个字符的位置
    }
    
    return 0;  // 默认返回0（理论上不应该到达这里）
}


// 核心函数：递归判断正则表达式regex是否匹配文本text
// 参数：regex - 正则表达式字符串
//       text - 待匹配的文本字符串
// 返回值：如果匹配返回true，否则返回false

bool matches(const char* regex, const char* text) {
    
    int regex_len = strlen(regex);  // 获取正则表达式的长度
    int text_len = strlen(text);    // 获取文本的长度
    
    // 基础情况：如果正则表达式为空
    if (regex_len == 0) {
        return (text_len == 0);  // 只有当文本也为空时才匹配
    }
    

// ========== 第一步：处理联合运算符'|'（优先级最低） ==========
    // 例如：regex = "a|b"，需要检查text是否匹配'a'或'b'
    int level = 0;  // 用于跟踪括号的嵌套层级
    // 从右往左遍历正则表达式，寻找不在括号内的'|'
    for (int i = regex_len - 1; i >= 0; i--) {
        if (regex[i] == ')') level++;  // 遇到右括号，进入更深一层
        if (regex[i] == '(') level--;  // 遇到左括号，退出一层
        
        // 如果在最外层（level==0）找到了'|'
        if (level == 0 && regex[i] == '|') {
            char S[MAX_LEN];  // 用于存储'|'左边的子表达式
            char T[MAX_LEN];  // 用于存储'|'右边的子表达式
            
            // 复制'|'左边的部分到S
            strncpy(S, regex, i);
            S[i] = '\0';  // 添加字符串结束符
            
            // 复制'|'右边的部分到T
            strcpy(T, regex + i + 1);
            
            // 递归检查：text匹配S 或 text匹配T
            // 只要有一个匹配就返回true
            return matches(S, text) || matches(T, text);
        }
    }


// ========== 第三步：处理星号运算'*'（优先级最高） ==========
    // 例如：regex = "a*"，匹配0个或多个'a'
        //

        if (regex[regex_len - 1] == '*') {
        char S[MAX_LEN];  // 存储星号前面的表达式
        strncpy(S, regex, regex_len - 1);
        S[regex_len - 1] = '\0';
        
  
        // 情况1：空字符串总是能被"任何东西*"匹配
        if (text_len == 0) {
            return true;
        }
        

        // 情况2：尝试将text分割为x和y
        // 让S匹配x，regex（包含星号）匹配y
        // 这样可以实现"重复0次、1次、2次...n次"的效果
        //
        for (int i = 1; i <= text_len; i++) {
            char x[MAX_LEN];  // text的前i个字符
            char y[MAX_LEN];  // text的剩余字符
            
            strncpy(x, text, i);
            x[i] = '\0';
            
            strcpy(y, text + i);
            
            // 如果S匹配x（一次重复）且regex匹配y（剩余的重复）
            if (matches(S, x) && matches(regex, y)) {
                return true;
            }
        }
        
        return false;  // 所有分割方式都不匹配
    }


// 主函数：程序的入口点
int main() {
    char regex_line[MAX_LEN];  // 存储从输入读取的正则表达式
    char text_line[MAX_LEN];   // 存储从输入读取的文本行
    

    // 读取第一行：正则表达式file get string(从文件获取字符串)遇到换行符停止，失败返回null
    if (fgets(regex_line, MAX_LEN, stdin) == NULL) {
        return EXIT_FAILURE;  // 读取失败，返回错误代码
    }
    remove_newline(regex_line);  // 移除末尾的换行符
    
 
    // 循环读取剩余的每一行文本
    while (fgets(text_line, MAX_LEN, stdin) != NULL) {
        remove_newline(text_line);  // 移除换行符
        
        char* text_to_match;  // 指向实际要匹配的文本
        
 
        // 特殊处理：如果输入是"_"，表示空字符串string compare (字符串比较)用来比较两个字符串是否相同
        if (strcmp(text_line, "_") == 0) {
            text_to_match = "";  // 将text_to_match指向空字符串
        } else {
            text_to_match = text_line;  // 否则就是普通文本
        }
        
 
        // 调用matches函数检查是否匹配
        if (matches(regex_line, text_to_match)) {
            printf("1\n");  // 匹配成功，输出1
        } else {
            printf("0\n");  // 匹配失败，输出0
        }
    }

    return 0;  // 程序正常结束
}
