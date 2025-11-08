#include <stdio.h>

// הגדרת משתנים גלובליים
int x = 5;    // הצהרה עם אתחול
float f = 5.7; // הגדרה תקינה
char a = 'a';  // הגדרה נכונה לתו

int main() {
    // השמה בתוך פונקציה - תקין
    x = 10;   
    int y = 20;
    int z = x + y;

    // הדפסה
    if (x == 10) {
        printf("x is 10\n");
    } else {
        printf("x is not 10\n");
    }

    printf("z = %d\n", z);
    return 0;
}
