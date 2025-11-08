#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
//T2b
//* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c){
  return fgetc(stdin);
}

//* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line.
//Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  (c >= 0x20 && c <= 0x7E) ? printf("%c\n", c) : printf(".\n"); //%c == takse int and translate it to ASCII code of this character
  return c;
}

//* Gets a char c and returns its encrypted form by subtracting 1 from its value.
// If c is not between 0x21 and 0x7F it is returned unchanged */
char encrypt(char c){
  return (c >= 0x21 && c <= 0x7F) ? c - 1 : c;
}

//* Gets a char c and returns its decrypted form by adding 1 to its value. 
//If c is not between 0x1F and 0x7E it is returned unchanged */
char decrypt(char c){
  return (c >= 0x1F && c <= 0x7E) ? c + 1 : c;
}

///* oprt prints the value of c in octal representation followed by a new line, and returns c unchanged. */
char oprt(char c){
  //we use in this limit (c >= 0x20 && c <= 0x7E) because those are all the signs you can show up on he screen! (outside from this spec is control characters). Will result "."
  //(c >= 0x20 && c <= 0x7E) ? printf("%o\n", c) : printf(".\n"); // %o == print the num in octal base
   //If we just wabt to print octal num we will use this num . This will result 12 for the example 
  printf("%o\n", c);
  return c;
}




char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for (int i = 0; i < array_length; i++){
    mapped_array[i] = f(array[i]);
  }

  return mapped_array;
}
 
//int main(int argc, char **argv){
  /* TODO: Test your code */
  //char arr1[] = {'H','e','y','!'};
  //char* arr2 = map(arr1, 4, oprt);
  //printf("%s\n", arr2);
  //free(arr2);

  //int base_len = 5;
  //char arr1[base_len];
  //char* arr2 = map(arr1, base_len, my_get);
  //char* arr3 = map(arr2, base_len, cprt);
  //char* arr4 = map(arr3, base_len, oprt);
  //char* arr5 = map(arr4, base_len, encrypt);
  //free(arr2);
  //free(arr3);
  //free(arr4);
  //free(arr5);
//}

