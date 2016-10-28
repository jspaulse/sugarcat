/* libstr.c - Provides some necessary char[] functionality */

/* Copyright (C) 2014 Jacob Paulsen <jspaulse@ius.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdbool.h>
#include <stddef.h>

size_t strlen(const char *);
char *strrev(char *);
char *itox(unsigned int, char *);
char *itoa(int, char *);
int atoi(const char *);
int strcmp(const char *, const char *);
/**
 * strcmp
 * 
 * Compares two character arrays for differences 
 * 
 * @str1	- char[] to compare
 * @str2	- char[] to compare against str1
 * 
 * @return - 
 * The difference between the two differing character (or size)
 * or zero if equal.
 * 
 * NOTE:  
 * This function will also return 0 if either two character arrays
 * are null.
 **/
int strcmp(const char *str1, const char *str2) {
    int ret = 0;

    if (str1 != NULL && str2 != NULL) {
        if (strlen(str1) == strlen(str2)) {
            for (unsigned int i = 0; i < strlen(str1); i++) {
                if (str1[i] > str2[i] || str1[i] < str2[i]) {
                    ret = str1[i] - str2[i];
                    break;
                }
            }
        } else {
            ret = strlen(str1) - strlen(str2);
        }
    }

    return ret;
}


/**
 * atoi
 * 
 * Array to Integer
 * Returns the int representation of a string.
 * 
 * @str - char[] to convert
 * 
 * @return -
 * The int representation of a string
 * 
 * NOTE
 * If the char[] contains a non-integer or str is null, this returns zero.
 **/
int atoi(const char *str) {
    int ret     = 0;
    bool sign   = false;

    if (str != NULL) {

        if (str[0] == '-') {
            sign = true;
            str++;
        }

        while (*str != '\0') {
            if (*str >= '0' && *str <= '9') {
                ret = (ret * 10) + (*str - '0');
                str++;
            } else {
                /* invalid character; break */
                ret = 0;
                break;
            }
        }

        if (sign)
            ret *= -1;
    }

    return ret;
}


/**
 * itoa
 * Integer to Array
 * 
 * Creates a char[] representation of integer inside a char[].
 * 
 * @val	- int value to represent in str
 * @str - char[] representation of val
 * 
 * @return -
 * The reference to str or NULL on error.
 **/
char *itoa(int val, char *str) {
    unsigned int offset = 0;
    bool sign           = false;
    char *ret           = NULL;

    if (str != NULL) {
		ret = str;
		
        if (val != 0) {

            if (val < 0) {
                val *= -1;
                sign = true;
           }

           while (val > 0) {
                str[offset++] = (48 + (val % 10));
                val /= 10;
           }

           if (sign == true) {
                str[offset++] = '-';
           }

           str[offset] = '\0';
           strrev(str);
        } else {
            str[offset++]   = '0';
            str[offset]     = '\0';
        }
    }

    return ret;
}

/**
 * itox
 * Integer to Hexadecimal
 * 
 * Creates a hexidecimal representation of val inside a char[].
 * 
 * @val - int value representation
 * @str - char[] representation of hexadecimal.
 * 
 * @return reference to str or null on error
 **/
char *itox(unsigned int val, char *str) {
    unsigned char c     = 0;
    unsigned int offset = 0;
    char *ret           = NULL;

    if (str != NULL) {
		ret = str;
		
        if (val != 0) {

            while (val > 0) {
                c = (val & 15);

                if (c <= 9) {
                    c += 48;
                } else {
                    c += 55;
                }

                str[offset++] = c;
                val >>= 4;
            }

            str[offset] = '\0';
            strrev(str);
        } else {
            str[offset++]   = '0';
            str[offset]     = '\0';
        }
    }

    return ret;
}


/**
 * strrev
 * String Reverse
 * 
 * Reverses the characters in a char[].
 * 
 * @str - char[] to reverse.
 * 
 * @return reference to the char[] or null on error
 **/
char *strrev(char *str) {
    int len = 0;
    char *ret = NULL;

    if (str != NULL) {
        ret = str;
        if ((len = strlen(str) - 1) != 0) {
            for (int i = 0; i <= len/2; i++) {
                char tmp = str[i];
                str[i] = str[len - i];
                str[len - i] = tmp;
            }
        }
    }

    return ret;
}


/**
 * strlen
 * 
 * returns the length of a char[]
 * 
 * @str - char[] to size
 * 
 * @return size of char[]
 * 
 * NOTE:
 * If str is null, value returned is -1;
 **/
size_t strlen(const char *str) {
    size_t ret = -1;

    if (str != NULL) {
		ret = 0;
		
        while (*str != '\0') {
            str++;
            ret++;
        }
    }

    return ret;
}
