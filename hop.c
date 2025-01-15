

#include "hop.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>


#define PATH_MAX 4096

char* hop(char* olddir,char *token) {
    
    
    if(strcmp(token,"..")==0)
    {
       olddir=parentdir(olddir);
       return olddir;
    }

    else if(strcmp(token,"-")==0)
    {
        olddir=prevdir(olddir);
        return olddir;
    }

    else if(strcmp(token,"~")==0)
    {
        olddir=homedir(olddir);
        return olddir;
    }

    else
    {
        olddir=nameddir(olddir,token);
        return olddir;
    }
    // Get the current working directory
    
        
    
}

void home() {
    char home_dir[PATH_MAX];
    struct passwd *pw = getpwuid(getuid()); 

    if (pw == NULL) {
        printf("Failed to get home directory");
        exit(1);
    }

    strcpy(home_dir, pw->pw_dir); 
    chdir(home_dir); 
    
}

char* homedir(char* olddir)
{
    char home_dir[PATH_MAX];
    char temp[PATH_MAX];
    
    olddir=getcwd(temp,sizeof(temp));
    struct passwd *pw = getpwuid(getuid()); 
    if (pw == NULL) {
        printf("Failed to get home directory");
        exit(1);
    }

    strcpy(home_dir, pw->pw_dir); 
    chdir(home_dir); 
    printf("%s\n", home_dir);
    return olddir;
}
char* parentdir(char* olddir)
{
    
    char curr_dir[PATH_MAX];
    char temp[PATH_MAX];

    olddir=getcwd(curr_dir,sizeof(curr_dir));
    chdir("..");
    printf("%s\n",getcwd(temp, sizeof(temp)));
    return olddir;

    
}

char* prevdir(char* olddir)
{
        
        char temp[PATH_MAX];
        
        char temp1[PATH_MAX];
        char temp2[PATH_MAX];
        strcpy(temp1,olddir);
        
        olddir=getcwd(temp,sizeof(temp));
        chdir(temp1);
        
        char curr_dir[PATH_MAX];
        printf("%s\n",getcwd(temp2, sizeof(temp2)));
        
        return olddir;

        
}

char* nameddir(char* olddir,char* target)
{
    char temp[PATH_MAX];
    char* temp1;
    temp1=getcwd(temp,sizeof(temp));
    
    if(chdir(target)==0)
    {
       
        strcpy(olddir,temp1);
        char temp2[PATH_MAX];
        char* temp3;
        temp3=getcwd(temp2,sizeof(temp2));
        printf("%s\n",temp3);
    }
    else
    {
        printf("No directory named %s in current directory\n",target);
    }

    return olddir;
}