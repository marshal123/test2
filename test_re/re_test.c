#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pcre.h>
#include "cre2.h"

void usage()
{
    printf("re_test $option\noption:\n");    
    printf("\t-r $regex_string\n");    
    printf("\t-R $regex_file_name\n");    
    printf("\t-t $test_string\n");    
    printf("\t-T $test_file_name\n");    
    printf("\t-p : set pcre match, default cre2 match.\n"); 
    printf("\t-V : print version.\n");  
    printf("\t-h : print usage.\n"); 
}

void version()
{
    printf("Version: 1.0\n");
    printf("Author: pengyuan@nsfocus.com\n");
    printf("Date: Mon Oct 13 11:30:07 CST 2014\n");
}

#define OVECCOUNT 30 /* should be a multiple of 3 */  
int pcre_match(char *re_str, size_t re_len, char *text_str, size_t text_len)
{
    int ret = 0;
    pcre  *re;  
    const char *error;  
    int  erroffset;  
    int  ovector[OVECCOUNT];  
    int  rc, i;   
    
    printf("[regex lib]\npcre\n\n[regex string]\n%.*s\n\n[text string]\n%.*s\n\n[match string]\n",
        (int)re_len,re_str,(int)text_len,text_str);
    
    re = pcre_compile(re_str,0,&error,&erroffset,NULL); 
    if (re == NULL) { 
        printf("\n[warning]\nPCRE compilation failed at offset %d: %s/n", erroffset, error);    
        goto out;  
    }  
    rc = pcre_exec(re,NULL,text_str,text_len,0,0,ovector,OVECCOUNT);  
    
    
    if (rc < 0) {
        if (rc == PCRE_ERROR_NOMATCH) printf("\n");  
        else printf("\n[warning]\nMatching error %d\n", rc);  
        pcre_free(re);  
        goto out;  
    }  
    for (i = 0; i < rc; i++) {
        char *substring_start = text_str + ovector[2*i];  
        int substring_length = ovector[2*i+1] - ovector[2*i];  
        printf("%.*s\n\n",substring_length, substring_start);  
    }  
    pcre_free(re);
    
out:    
    
    return 0;
}

int re2_match(char *re_str, size_t re_len, char *text_str, size_t text_len)
{
    int ret = 0;
    cre2_options_t *opt = cre2_opt_new();
    cre2_regexp_t *rexexp = cre2_new(re_str,re_len,opt);
    cre2_string_t match_string = {.length =0, .data = NULL};    
    cre2_opt_set_encoding (opt, CRE2_Latin1);
    
    ret = cre2_match(rexexp, text_str, text_len, 0, text_len,CRE2_UNANCHORED, &match_string, 1);       
    printf("[regex lib]\ncre2\n\n[regex string]\n%.*s\n\n[text string]\n%.*s\n\n[match string]\n%.*s\n",
        (int)re_len,re_str,(int)text_len,text_str,match_string.length,match_string.data);

    if (re_str)
        free(re_str);
    if (text_str)
        free(text_str);
    cre2_delete(rexexp);
    cre2_opt_delete(opt);
    
    return 0;
}

int read_cmd(char* cmd, char **p, size_t *len)
{
    *len = strlen(cmd);
    *p =(char*)malloc(*len + 1);
    memcpy(*p,cmd,*len);
    (*p)[*len] = '\0';   
    
    return 0;
}
int read_file(char* filename, char **p, size_t *len)
{
    int num = 0;
    FILE * fp = fopen(filename,"r");
    if(NULL == fp) {
        printf("open %s error.\n",filename);
        return -1;
    }
    
    fseek(fp,0,SEEK_END);;
    *len = (size_t)ftell(fp);
    *p =(char*)malloc(*len + 1);
    fseek(fp,0,SEEK_SET);
    num = fread(*p,sizeof(char),*len,fp);
    if (num != *len) {
        printf("read %s error.\n",filename);
        return -1;
    }
    (*p)[*len] = '\0';      
    
    fclose(fp);      
    
    return 0;
}

int main(int argc, char **argv)
{
    int     opt = 0;
    int     match_pcre = 0;
    char    *re_str = NULL;
    char    *text_str = NULL;
    size_t  re_str_len = 0;
    size_t  text_str_len = 0;
    
    while ((opt=getopt(argc,argv,"Vhpr:R:t:T:"))!=-1) {
        switch (opt) {
            case 'p':
                match_pcre = 1;
                break;                
            case 'r':
                read_cmd(optarg,&re_str,&re_str_len);
                break;                
            case 'R':
                read_file(optarg,&re_str,&re_str_len);
                break;                
            case 't':
                read_cmd(optarg,&text_str,&text_str_len);
                break;                
            case 'T':
                read_file(optarg,&text_str,&text_str_len);
                break;                  
            case 'V':
                version();
                return 0; 
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    if (!re_str_len || !text_str_len) {
        usage();
        return 0;
    }

    if (match_pcre) {
        pcre_match(re_str,re_str_len,text_str,text_str_len);    
    } else {
        re2_match(re_str,re_str_len,text_str,text_str_len);
    }
    
    return 0;    
}

