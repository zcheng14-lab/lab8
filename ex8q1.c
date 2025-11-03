#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LEN 257

bool matches(const char* regex, const char* text);
int get_last_sub_expr_start(const char* regex, int len);

void remove_newline(char* str) {
    int len = strlen(str);
    
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int get_last_sub_expr_start(const char* regex, int len) {
    if (regex[len - 1] == ')') {
        int level = -1;
        for (int i = len - 2; i >= 0; i--) {
            if (regex[i] == ')') level--;
            if (regex[i] == '(') level++;
            if (level == 0) {
                return i;
            }
        }
    } 
    else if (regex[len - 1] == '*') {
        if (regex[len - 2] == ')') {
            int level = -1;
            for (int i = len - 3; i >= 0; i--) {
                if (regex[i] == ')') level--;
                if (regex[i] == '(') level++;
                if (level == 0) {
                    return i;
                }
            }
        } else {
            return len - 2;
        }
    }
    else {
        return len - 1;
    }
    
    return 0;
}

bool matches(const char* regex, const char* text) {
    
    int regex_len = strlen(regex);
    int text_len = strlen(text);
    
    if (regex_len == 0) {
        return (text_len == 0);
    }
    
    int level = 0;
    for (int i = regex_len - 1; i >= 0; i--) {
        if (regex[i] == ')') level++;
        if (regex[i] == '(') level--;
        
        if (level == 0 && regex[i] == '|') {
            char S[MAX_LEN];
            char T[MAX_LEN];
            
            strncpy(S, regex, i);
            S[i] = '\0'; 
            
            strcpy(T, regex + i + 1);
            
            return matches(S, text) || matches(T, text);
        }
    }
    
    int split_point = get_last_sub_expr_start(regex, regex_len);
    
    if (split_point > 0) {
        char S[MAX_LEN];
        char T[MAX_LEN];
        
        strncpy(S, regex, split_point);
        S[split_point] = '\0';
        
        strcpy(T, regex + split_point);
        
        for (int i = 0; i <= text_len; i++) {
            char x[MAX_LEN];
            char y[MAX_LEN];
            
            strncpy(x, text, i);
            x[i] = '\0';
            
            strcpy(y, text + i);
            
            if (matches(S, x) && matches(T, y)) {
                return true;
            }
        }
        return false;
    }

    if (regex[regex_len - 1] == '*') {
        char S[MAX_LEN];
        strncpy(S, regex, regex_len - 1);
        S[regex_len - 1] = '\0';
        
        if (text_len == 0) {
            return true;
        }
        
        for (int i = 1; i <= text_len; i++) {
            char x[MAX_LEN];
            char y[MAX_LEN];
            
            strncpy(x, text, i);
            x[i] = '\0';
            
            strcpy(y, text + i);
            
            if (matches(S, x) && matches(regex, y)) {
                return true;
            }
        }
        
        return false;
    }
    
    if (regex[0] == '(' && regex[regex_len - 1] == ')') {
        char S[MAX_LEN];
        strncpy(S, regex + 1, regex_len - 2);
        S[regex_len - 2] = '\0';
        
        return matches(S, text);
    }
    
    if (regex_len == 1) {
        if (regex[0] == '_') {
            return (text_len == 0);
        }
        else {
            return (text_len == 1 && regex[0] == text[0]);
        }
    }
    
    return false;
}

int main() {
    char regex_line[MAX_LEN];
    char text_line[MAX_LEN];
    
    if (fgets(regex_line, MAX_LEN, stdin) == NULL) {
        return EXIT_FAILURE;
    }
    remove_newline(regex_line);
    
    while (fgets(text_line, MAX_LEN, stdin) != NULL) {
        remove_newline(text_line);
        
        char* text_to_match;
        
        if (strcmp(text_line, "_") == 0) {
            text_to_match = "";
        } else {
            text_to_match = text_line;
        }
        
        if (matches(regex_line, text_to_match)) {
            printf("1\n");
        } else {
            printf("0\n");
        }
    }

    return 0;
}
