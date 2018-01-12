#include "cd.h"

#include <zjunix/type.h>
#include <zjunix/fs/fat.h>


void do_cd(char * param, char * pwd)
{
	int i;
	int j;
	int len;
	i = strlen(param);
    //printf("  %s\n",param);
    len = strlen(pwd);
    //printf("  %d\n",len);
    if(param[0] == '.')
    {
        if(param[1] == '.')
        {
        	j = len;
        	while(j != 1 && pwd[j-1] != '/')
        	{
        		pwd[j-1] = 0;
        		j--;
			}
			if(j != 1)
			{
				pwd[j-1] = 0;
			}
		}
		else
		{
			//printf("we don't accept this instruction");
		}
	}
	else
	{
		if(len!=1)
		{
			pwd[len] = '/';
			for(j = 0; j < i; j++)
        	{
            	pwd[len+j+1] = param[j];
        	}
        	pwd[len+j+1] = 0;
		}
		else
		{
			for(j = 0; j < i; j++)
        	{
            	pwd[len+j] = param[j];
        	}
        	pwd[len+j] = 0;
		}
	}
	//printf("%s\n",pwd);
    //gets(ps_buffer);
}