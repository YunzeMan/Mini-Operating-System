#ifndef _LS_H
#define _LS_H


char *cut_front_blank(char *str);
unsigned int strlen(unsigned char *str);
unsigned int each_param(char *para, char *word, unsigned int off, char ch);
int ls_help();
int ls(char *para);
int ls_l(char *para);

#endif
