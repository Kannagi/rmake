#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


void folderRCreate(char *buffer,char *binit);
#define MAX_ENTRIES 4000

typedef struct
{
    char target[64];

    char exe_name[64];
    char compilerc[64];
    char compilercpp[64];
    char linker[64];

    char options[512];
    char libs[1024];

    char libdirs[2048];
    char includes[2048];

    char cmd_precompiler[1024];
    char cmd_postcompiler[1024];
    char cmd_linker[512];
    char cmd_compiler[512];
    
    char cmd_execute[256];

    char folder[2048];
    char file[2048];

    char *obj;

}RMAKE;

void rmakeCommand(FILE *file,RMAKE *rmake,char *buffer,char *filename,int compiler_type);
RMAKE *_rmake_rmake;
FILE *_rmake_file;

void listFilesRecursive(const char *path, char *entries[], int *entryCount)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path)) == NULL)
	{
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
	{
        // Ignore "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
    
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        //printf("%s\n", fullPath);
        entries[*entryCount] = strdup(fullPath);
        (*entryCount)++;

        if (entry->d_type == DT_DIR)
            listFilesRecursive(fullPath, entries, entryCount);
        else
        {
            int l = strlen(fullPath);
            if( (fullPath[l-2] == '.') && (fullPath[l-1] == 'c') )
            {
                fullPath[l-2] = 0;
                rmakeCommand(_rmake_file,_rmake_rmake,_rmake_rmake->cmd_compiler,fullPath,0);
                fputs("\n",_rmake_file);
            }else
            if( (fullPath[l-4] == '.') && (fullPath[l-3] == 'c') && (fullPath[l-2] == 'p') && (fullPath[l-1] == 'p') )
            {
                fullPath[l-4] = 0;
                rmakeCommand(_rmake_file,_rmake_rmake,_rmake_rmake->cmd_compiler,fullPath,1);
                fputs("\n",_rmake_file);
            }else
                printf("unkonw format %s\n",fullPath);

        }
    }

    closedir(dir);
}

void listFiles(RMAKE *rmake,FILE *file,const char *path)
{
    _rmake_rmake = rmake;
    _rmake_file = file;
    char *entries[MAX_ENTRIES];
    int entryCount = 0;

    listFilesRecursive(path, entries, &entryCount);

    for (int i = 0; i < entryCount; i++)
        free(entries[i]);
}

void* loadFile(const char* filename,int *sz)
{
	int fileSize;
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Impossible to open the file : %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0)
    {
        fprintf(stderr, "Empty file: %s\n", filename);
        fclose(file);
        return NULL;
    }


    char* buffer = (char*)malloc(fileSize+1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation error :%s /%d\n",filename,fileSize);
        fclose(file);
        return NULL;
    }

    int bytesRead = fread(buffer, 1, fileSize, file);
    if(bytesRead != fileSize)
    {
        fprintf(stderr, "Error reading file : %s\n", filename);
        fclose(file);
        free(buffer);
        return NULL;
    }

    fclose(file);

    *sz = fileSize;

    buffer[fileSize] = 0;

    return buffer;
}

void *rmake_keyword(RMAKE *rmake,char *keywords)
{
    if(strcmp(keywords,"exe_name") == 0)
        return rmake->exe_name;

    if(strcmp(keywords,"compilerc") == 0)
        return rmake->compilerc;

    if(strcmp(keywords,"compilercpp") == 0)
        return rmake->compilercpp;

    if(strcmp(keywords,"linker") == 0)
        return rmake->linker;


    if(strcmp(keywords,"options") == 0)
        return rmake->options;

    if(strcmp(keywords,"libs") == 0)
        return rmake->libs;


    if(strcmp(keywords,"libdirs") == 0)
        return rmake->libdirs;

    if(strcmp(keywords,"includes") == 0)
        return rmake->includes;

    if(strcmp(keywords,"target") == 0)
        return rmake->target;

    if(strcmp(keywords,"cmd_precompiler") == 0)
        return rmake->cmd_precompiler;

    if(strcmp(keywords,"cmd_postcompiler") == 0)
        return rmake->cmd_postcompiler;

    if(strcmp(keywords,"cmd_linker") == 0)
        return rmake->cmd_linker;

    if(strcmp(keywords,"cmd_compiler") == 0)
        return rmake->cmd_compiler;

    if(strcmp(keywords,"cmd_execute") == 0)
        return rmake->cmd_execute;

    if(strcmp(keywords,"add_folder") == 0)
        return rmake->folder;

    if(strcmp(keywords,"add_file") == 0)
        return rmake->file;

    return NULL;
}


void rmakeInit(RMAKE *rmake,char *buffer,int n)
{
    int l = 0;
    char keywords[32];
    char *buf = NULL;
    int lec = 0;

    rmake->exe_name[0] = 0;
    rmake->compilerc[0] = 0;
    rmake->compilercpp[0] = 0;
    rmake->linker[0] = 0;

    rmake->options[0] = 0;
    rmake->libs[0] = 0;

    rmake->libdirs[0] = 0;
    rmake->includes[0] = 0;

    rmake->target[0] = 0;

    rmake->cmd_precompiler[0] = 0;
    rmake->cmd_postcompiler[0] = 0;
    rmake->cmd_linker[0] = 0;
    rmake->cmd_compiler[0] = 0;

    rmake->obj = malloc(0x100000);
    memset(rmake->obj,0,0x100000);

    memset(rmake->file,0,2048);
    memset(rmake->folder,0,2048);

    for(int i = 0;i < n ;i++)
    {
        if(buffer[i] == '#')
        {
            while((buffer[i] != '\n') && (i < n)) i++;
        }

        if(buffer[i] >= ' ')
        {
            if(lec == 0)
            {
                l = 0;
                while((buffer[i] != ':') && (i < n))
                {
                    keywords[l] = buffer[i];
                    i++;
                    l++;
                }

                if(l > 0)
                {
                    keywords[l] = 0;
                    l = 0;
                    //printf("key:%s\n",keywords);

                    buf = rmake_keyword(rmake,keywords);

                    lec = 1;
                }

                if(buffer[i+1] == '\n')
                    lec = 0;
            }
            else
            {
                if(buf != NULL)
                {
                    l = 0;
                    while((buffer[i] != '\n') && (i < n))
                    {
                        buf[l] = buffer[i];
                        i++;
                        l++;
                    }

                    if(l > 0)
                    {
                        buf[l] = 0;
                        l = 0;
                        //printf("cmd:%s\n",buf);
                        buf = NULL;
                    }
                }else
                {
                    while((buffer[i] != '\n') && (i < n)) i++;
                }

                lec = 0;
            }

            
        }
    }
}

void rmakeCommand(FILE *file,RMAKE *rmake,char *buffer,char *filename,int compiler_type)
{
    char keywords[32];
    int l;

    int n = strlen(buffer);

    for(int i = 0;i < n;i++)
    {
        if(buffer[i] == '$')
        {
            l = 0;
            i++;
            while((buffer[i] != ' ') && (i < n))
            {
                keywords[l] = buffer[i];
                i++;
                l++;
            }

            keywords[l] = 0;

            if(strcmp(keywords,"compiler") == 0)
            {
                if(compiler_type == 0)
                    fputs(rmake->compilerc,file);
                else
                    fputs(rmake->compilercpp,file);
            }
                

            if(strcmp(keywords,"options") == 0)
                fputs(rmake->options,file);

            if(strcmp(keywords,"includes") == 0)
                fputs(rmake->includes,file);

            if(strcmp(keywords,"file") == 0)
            {
                fputs(filename,file);

                if(compiler_type == 0)
                    fputs(".c",file);
                else
                    fputs(".cpp",file);

                strcat(rmake->obj,"obj/");
                strcat(rmake->obj,filename);
                strcat(rmake->obj,".o ");

                folderRCreate(filename,"obj/");
            }
                
            if(strcmp(keywords,"object") == 0)
            {
                if(compiler_type < 16)
                {
                    fputs("obj/",file);
                    fputs(filename,file);
                    fputs(".o",file);
                }
                else
                {
                    fputs(rmake->obj,file);
                }
                
            }

            if(strcmp(keywords,"linker") == 0)
                fputs(rmake->linker,file);

            if(strcmp(keywords,"libdirs") == 0)
                fputs(rmake->libdirs,file);

            if(strcmp(keywords,"exe_name") == 0)
                fputs(rmake->exe_name,file);

            if(strcmp(keywords,"libs") == 0)
                fputs(rmake->libs,file);

        }
        
        if(i < n)
            fputc(buffer[i],file);

    }
}

#ifdef WINDOWS
const char *bin_sh = "";
const char *compile_sh = ".vscode/compile.bat";
const char *execute_sh = ".vscode/execute.bat";
#else
const char *bin_sh = "#!/bin/sh\n";
const char *compile_sh = ".vscode/compile.sh";
const char *execute_sh = ".vscode/execute.sh";
const char *cmddir = "mkdir";
#endif

void dirCreate(char *pathdir)
{
    DIR *dir;

    char out[256];

    sprintf(out,"%s %s\n",cmddir,pathdir);

    if ((dir = opendir(pathdir)) == NULL)
        system(out);
    else
        closedir(dir);
}

void folderRCreate(char *buffer,char *binit)
{
    char folder[256];
    
    int l = 4;
    int bfile = 0;


    strcpy(folder,binit);
    int n = strlen(buffer);

    for(int i = 0;i < n;i++)
    {
        
        while((buffer[i] != '/') && (i < n))
        {
            folder[l] = buffer[i];
            i++;
            l++;
        }

        if(buffer[i] == '/')
            bfile = 1;



        if( (l > 0) && (bfile == 1) )
        {
            folder[l] = 0;
            dirCreate(folder);
            folder[l] = '/';
            l++;

            bfile = 0;
            
        }
    }
}


void rmakeCompileFile(FILE *file,RMAKE *rmake,char *buffer)
{
    char filename[256];
    int n = strlen(buffer);
    for(int i = 0;i < n;i++)
    {
        int l = 0;
        while((buffer[i] != ';') && (i < n))
        {
            filename[l] = buffer[i];
            i++;
            l++;
        }
        filename[l] = 0;

        if(l > 0)
        {
            filename[l] = 0;

            if( (filename[l-2] == '.') && (filename[l-1] == 'c') )
            {
                filename[l-2] = 0;
                rmakeCommand(file,rmake,rmake->cmd_compiler,filename,0);
                fputs("\n",file);
            }else
            if( (filename[l-4] == '.') && (filename[l-3] == 'c') && (filename[l-2] == 'p') && (filename[l-1] == 'p') )
            {
                filename[l-4] = 0;
                rmakeCommand(file,rmake,rmake->cmd_compiler,filename,1);
                fputs("\n",file);
            }else
                printf("unkonw format %s\n",filename);
        }
    }
}


void rmakeCreate(RMAKE *rmake)
{
    FILE *file = fopen(".vscode/compile.sh","w");

    if(file == NULL)
    {
        printf("Error write file compile.sh\n");
        return;
    }
        
    fputs(bin_sh,file);

    //pre compiler
    fputs(rmake->cmd_precompiler,file);
    fputs("\n",file);

    //compiler file
    rmakeCompileFile(file,rmake,rmake->file);

    char filename[256];
    int n = strlen(rmake->folder);
    for(int i = 0;i < n;i++)
    {
        int l = 0;
        while((rmake->folder[i] != ';') && (i < n))
        {
            filename[l] = rmake->folder[i];
            i++;
            l++;
        }
        filename[l] = 0;

        if(l > 0)
        {
            filename[l] = 0;

            listFiles(rmake,file,filename);
        }
    }

    fputs("\n",file);


    //linker
    rmakeCommand(file,rmake,rmake->cmd_linker,"",16);
    fputs("\n\n",file);

    //post compiler
    fputs(rmake->cmd_postcompiler,file);
    fputs("\n",file);

    fclose(file);

    //---------------
    file = fopen(".vscode/execute.sh","w");

    if(file == NULL)
    {
        printf("Error write file execute.sh\n");
        return;
    }

    fputs(bin_sh,file);
    fputs(rmake->cmd_execute,file);
    fputs("\n",file);


    fclose(file);
}

int main(int argc, char *argv[])
{
    char *buffer = NULL;
    int fsize;

    char filename[256];
    strcpy(filename,"rmake.txt");

    if(argc == 2)
        strcpy(filename,argv[1]);

    if(argc > 2)
        printf("additional arguments are ignored\n");

    buffer = loadFile(filename,&fsize);
    if (buffer == NULL)
        return 0;

    RMAKE rmake;

    rmakeInit(&rmake,buffer,fsize);

    rmakeCreate(&rmake);

    return 0;
}


//C:\Windows\System32
// /usr/bin
// .bat