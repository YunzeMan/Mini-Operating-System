#include <driver/vga.h>
#include <zjunix/utils.h>

void* kernel_memcpy(void* dest, void* src, int len) {
    char* deststr = dest;
    char* srcstr = src;
    while (len--) {
        *deststr = *srcstr;
        deststr++;
        srcstr++;
    }
    return dest;
}

#pragma GCC push_options
#pragma GCC optimize("O2")
void* kernel_memset(void* dest, int b, int len) {
#ifdef MEMSET_DEBUG
    kernel_printf("  memset:%x,%x,len%x,", (int)dest, b, len);
#endif  // ! MEMSET_DEBUG
    char content = b ? -1 : 0;
    char* deststr = dest;
    while (len--) {
        *deststr = content;
        deststr++;
    }
#ifdef MEMSET_DEBUG
    kernel_printf("  %x\n", (int)deststr);
#endif  // ! MEMSET_DEBUG
    return dest;
}
#pragma GCC pop_options

unsigned int* kernel_memset_word(unsigned int* dest, unsigned int w, int len) {
    while (len--)
        *dest++ = w;

    return dest;
}

/* realize the memmove function in string.h */
void* kernel_memmove(void* to, const void* from, int count)
{
    void * result = to;
    char * pto = (char *)to;
    char * pfrom = (char *)from;
    //to与from没有重叠
    if(pto < pfrom || pto > pfrom + count -1)
    {
       while(count--)
       {
           *pto++ = *pfrom++;
       }
    }
    //to与from有重叠，从后向前move
    else
    {
       pto = pto + count -1;
       pfrom = pfrom + count -1;
       while(count--)
       {
          *pto-- = *pfrom--;
       }
    }
    return result;
}

/* realize the strlen function in string.h */
int kernel_strlen(char * str)
{
    int index = 0;
    while(str[index] != '0')
    {
        ++index;
    }
    return index;
}

/* realize the strcat function in string.h */
char * kernel_strcat(char * dest, char * src)
{
    char * address = dest;
    while(*dest)
    {
        dest++;
    }
    while(*dest++ = *src++)
    {
        
    }
    return address;
}

/* realize the strncat function in string.h */
char * kernel_strncat(char *dest, char *src, int n)
{
    char *cp = dest;  
    while(*cp != '\0')
    {
        ++cp;
    }
    while(n && (*cp++=*src++)!='\0')  
    {  
        --n;  
    }  
    return dest;  
}

/* realize the strncpy function is string.h */
char * kernel_strncpy(char *dest, char *src, int n)
{
    char *cp = dest;  
    while(n && (*cp++ = *src++) != '\0')  
    {  
        n--;  
    }  
    if(n)  
    {  
        while(--n)  
        *cp++='\0';  
    }  
    return dest; 
}

/* realize the strchr function in string.h */
char * kernel_strchr(const char *s, int c)
{
    if(s == NULL)
    {
        return NULL;
    }
    while(*s != '\0')
    {
        if(*s == (char)c )
        {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

/* realize the strrchr function in string.h */
char * kernel_strrchr(const char *s, int c)
{
    if(s == NULL)
    {
        return NULL;
    }
    char *p_char = NULL;
    while(*s != '\0')
    {
        if(*s == (char)c)
        {
            p_char = (char *)s;
        }
        s++;
    }
    return p_char;
}

/* realize the strstr function in string.h */
char * kernel_strstr(const char *str1, const char *str2)
{
    char *cp = (char*)str1;
    char *s1, *s2;
    if (!*str2)
        return((char *)str1);
    while (*cp)
    {
        s1 = cp;
        s2 = (char *)str2;
        while (*s1 && *s2 && !(*s1 - *s2))
            s1++, s2++;
        if (!*s2)
            return cp;
        cp++;
    }
    return NULL;
}

int kernel_strspn(const char *str1, const char *str2)
{
    const char *p = str1;
    const char *a;
    int count = 0;
    for ( ; *p != '\0'; ++p) {
        for (a = str2; *a != '\0'; ++a) {
            if (*p == *a)
            {
                break;
            }
        }
        if (*a == '\0')
        {
            return count;
        }
        ++count;
    }
    return count;
}

char * kernel_strpbrk(const char *str1, const char *str2)
{
    const char *sc1,*sc2;
    for ( sc1 = str1; *sc1!='\0'; sc1++)
    {
        for ( sc2 = str2; *sc2!='\0'; sc2++)
        {
            if (*sc1 == *sc2)
            {
                return (char *)sc1;
            }
        }
    }
    return NULL;
}

char * kernel_strdup(char * src)
{
    if(src != NULL)     
    {     
        char *start=src;     
        int len = 0;     
        while(*src++ != '\0')
        {
            len++;
        }          
        char *address = (char *)kmalloc(len+1);
        while((*address++=*start++)!='\0');      
        return address-(len+1);      
    }     
    return NULL;   
}

int kernel_strcmp(const char* dest, const char* src) {
    while ((*dest == *src) && (*dest != 0)) {
        dest++;
        src++;
    }
    return *dest - *src;
}

char* kernel_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++))
        ;
    return dest;
}

int pow(int x, int z) {
    int ret = 1;
    if (z < 0)
        return -1;
    while (z--) {
        ret *= x;
    }
    return ret;
}

#pragma GCC push_options
#pragma GCC optimize("O0")

void kernel_cache(unsigned int block_index) {
    block_index = block_index | 0x80000000;
    asm volatile(
        "li $t0, 233\n\t"
        "mtc0 $t0, $8\n\t"
        "move $t0, %0\n\t"
        "cache 0, 0($t0)\n\t"
        "nop\n\t"
        "cache 1, 0($t0)\n\t"
        : "=r"(block_index));
}

#pragma GCC pop_options

void kernel_serial_puts(char* str) {
    while (*str)
        *((unsigned int*)0xbfc09018) = *str++;
}

void kernel_serial_putc(char c) {
    *((unsigned int*)0xbfc09018) = c;
}

unsigned int is_bound(unsigned int val, unsigned int bound) {
    return !(val & (bound - 1));
}
