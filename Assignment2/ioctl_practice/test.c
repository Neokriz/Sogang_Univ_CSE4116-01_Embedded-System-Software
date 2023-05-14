#include <stdio.h>
#include <stdlib.h>
#include <linux/string.h>

int main()
{
    char str[13] = {0x00};
    char init[5] = {'0', '0', '0', '5', '\0'};
    int interval=7, cnt=1;
    char *in_str = str;
    char *msg;
    
    char value[5];

    int a, b;
    int arg = 0;
    
    int i, j;
    
    sprintf(str, "%3d %3d %s", interval, cnt, init);
    printf("[--%s--]\n", str);
    
    msg = in_str;
    while(msg != NULL) {
        msg = strsep(&in_str, " ");
        if((msg != NULL) && strcmp("", msg)) {
            arg++;
            switch(arg) {
            case 1:
            case 2:
                printf("%d\n", atoi(msg));
                break;
            case 3:
                strcpy(value, msg);
                printf("[%s]\n", msg);
            }
        }
    }
    
    for(i=0,j=10;i<10;++i,++j) {
        printf("i:%d, j:%d\n", i, j);
    }
    
    return 0;
}

