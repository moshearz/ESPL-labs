#include <stdio.h>

int count_digits(const char *str){
    int count = 0;
    while (*str){
        if(*str >= '0' && *str <= '9') count ++;
        str += 1;
    }
    return count;
}

int main() { return 0;}