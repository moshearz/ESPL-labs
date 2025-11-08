#include <stdio.h>
int main()
{
double weight, price = 0.2;
printf("Enter the weight of the apple: ");
scanf_s("%lf", &weight);
printf("Sider ");
if (weight > 10)
{ printf("Premium ");
price = price + 0.1;
} // end if
printf("Apple. $%.2lf.\n", price);
return 0;
} // end main