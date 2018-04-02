#include <stdio.h>
#include <stdlib.h>

struct msg1
{
    long type;
    int data;
};

struct msg2
{
    long type;
    int data;
    char ch;
};

int main(void)
{
    struct msg1 *test = malloc(sizeof(struct msg1));
    struct msg2 *test2 = malloc(sizeof(struct msg2));

    test2->type = 10;
    test2->data = 100;
    test2->ch = 'a';

    test->type = ((struct msg1*)test2)->type;
    test->data = 1234;

    printf("%ld\n", test->type);

    free(test);
    free(test2);
    return 0;
}
