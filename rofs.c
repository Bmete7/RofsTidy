

#define FUSE_USE_VERSION 26

static const char* rofsVersion = "2008.09.24";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/xattr.h>
#include <dirent.h>
#include <unistd.h>
#include <fuse.h>
#include <tidy.h>
#include <tidybuffio.h>


// Global to store our read-write path
char *rw_path;

// Translate an rofs path into it's underlying filesystem path
static char* translate_path(const char* path)
{

    char *rPath= malloc(sizeof(char)*(strlen(path)+strlen(rw_path)+1));

    strcpy(rPath,rw_path);
    if (rPath[strlen(rPath)-1]=='/') {
        rPath[strlen(rPath)-1]='\0';
    }
    strcat(rPath,path);

    return rPath;
}


/******************************
*
* Callbacks for FUSE
*
*
*
******************************/

static int rofs_getattr(const char *path, struct stat *st_data)
{
    int res;
    char *upath=translate_path(path);
    int i = 0;
    int fd;
    int index = 0;
    int offset = 0;
    int size = st_data->st_size;
    TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;
    
	for(i = 0 ; i < strlen(path); ++i){
		if(path[i] == '.'){
			index = i;
		}
	}
	if(!strcmp(path+index+1, "html")){
		
		printf("This is an html file\n");
		fd = open(upath, O_RDONLY);
		char *target = malloc(sizeof(char)*size); // intermediate buffer, to manipulate html
		close(fd);
		res = pread(fd, target, size, offset);
		TidyDoc tdoc = tidyCreate();                
	
		tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  
		tidyOptSetBool( tdoc, TidyIndentContent, yes );
		
		if(tidyFileExists(tdoc,"/home/meteb/Desktop/projectFuse/config.txt")){
			tidyLoadConfig(tdoc,"/home/meteb/Desktop/projectFuse/config.txt");
		}
		printf( "%d\n",tidyConfigErrorCount(tdoc));
		rc = tidySetErrorBuffer( tdoc, &errbuf );    
		rc = tidyParseString( tdoc, target );           
		rc = tidyCleanAndRepair( tdoc );               
		rc = tidyRunDiagnostics( tdoc );               
		rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
		rc = tidySaveBuffer( tdoc, &output ); 
		
		
	}

    res = lstat(upath, st_data);

    if(!strcmp(path+index+1, "html")){
		st_data->st_size = strlen(output.bp) + 2;
	}
    
    free(upath);
    if(res == -1) {
        return -errno;
    }
    return 0;
}

static int rofs_readlink(const char *path, char *buf, size_t size)
{
    int res;
    char *upath=translate_path(path);

    res = readlink(upath, buf, size - 1);
    free(upath);
    if(res == -1) {
        return -errno;
    }
    buf[res] = '\0';
    return 0;
}

static int rofs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;
    int res;

    (void) offset;
    (void) fi;

    char *upath=translate_path(path);

    dp = opendir(upath);
    free(upath);
    if(dp == NULL) {
        res = -errno;
        return res;
    }

    while((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    printf("\n%s\n", path);
    return 0;
}


static int rofs_open(const char *path, struct fuse_file_info *finfo)
{
    int res;

    /* We allow opens, unless they're tring to write, sneaky
     * people.
     */
    int flags = finfo->flags;

    if ((flags & O_WRONLY) || (flags & O_RDWR) || (flags & O_CREAT) || (flags & O_EXCL) || (flags & O_TRUNC) || (flags & O_APPEND)) {
        return -EROFS;
    }

    char *upath=translate_path(path);

    res = open(upath, flags);

    free(upath);
    if(res == -1) {
        return -errno;
    }
    close(res);
    return 0;
}

static int rofs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    int fd;
    int res;
   (void)finfo;
    char *upath=translate_path(path);
     

    fd = open(upath, O_RDONLY);
    free(upath);
     
    if(fd == -1) {
        res = -errno;
        return res;
    }
    
    TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	int rc = -1;
  

    

    
    ////// sondan bir önceki karakter \n
	int i = 0,index = 0;
	for(i = 0 ; i < strlen(path); ++i){
		if(path[i] == '.'){
			index = i;
		}
	}
	if(!strcmp(path+index+1, "html")){
		char *target = malloc(sizeof(char)*size); // intermediate buffer, to manipulate html
		res = pread(fd, target, size, offset);
		close(fd);
		TidyDoc tdoc = tidyCreate();                
		
		tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  
		tidyOptSetBool( tdoc, TidyIndentContent, yes );
		if(tidyFileExists(tdoc,"/home/meteb/Desktop/projectFuse/config.txt")){
			tidyLoadConfig(tdoc,"/home/meteb/Desktop/projectFuse/config.txt");
		}
		
		rc = tidySetErrorBuffer( tdoc, &errbuf );     
		rc = tidyParseString( tdoc, target );           
		rc = tidyCleanAndRepair( tdoc );               
		rc = tidyRunDiagnostics( tdoc );               
		rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
		
		rc = tidySaveBuffer( tdoc, &output ); 
			printf("%s\n",output.bp);
		memcpy(buf,output.bp + offset,strlen(output.bp)+2);
		res = strlen(output.bp) - offset;
		buf[res] = '\n';
		buf[res+1] = '\0';
		free(target);
	}
	else {
		res = pread(fd, buf, size, offset);
		close(fd);
	}

    if(res == -1) {
        res = -errno;
    }
  
	
    return res;
}

static int rofs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    (void)path;
    (void)buf;
    (void)size;
    (void)offset;
    (void)finfo;
    return -EROFS;
}

static int rofs_statfs(const char *path, struct statvfs *st_buf)
{
    int res;
    char *upath=translate_path(path);

    res = statvfs(upath, st_buf);
    free(upath);
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int rofs_access(const char *path, int mode)
{
    int res;
    char *upath=translate_path(path);

    /* Don't pretend that we allow writing
     * Chris AtLee <chris@atlee.ca>
     */
    if (mode & W_OK)
        return -EROFS;

    res = access(upath, mode);
    free(upath);
    if (res == -1) {
        return -errno;
    }
    return res;
}


struct fuse_operations rofs_oper = {
    .getattr     = rofs_getattr,
    .readlink    = rofs_readlink,
    .readdir     = rofs_readdir,
    .open        = rofs_open,
    .read        = rofs_read,
    .write       = rofs_write,
    .statfs      = rofs_statfs,
    .access      = rofs_access,

};
enum {
    KEY_HELP,
    KEY_VERSION,
};

static void usage(const char* progname)
{
    fprintf(stdout,
            "usage: %s readwritepath mountpoint [options]\n"
            "\n"
            "   Mounts readwritepath as a read-only mount at mountpoint\n"
            "\n"
            "general options:\n"
            "   -o opt,[opt...]     mount options\n"
            "   -h  --help          print help\n"
            "   -V  --version       print version\n"
            "\n", progname);
}

static int rofs_parse_opt(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
    (void) data;

    switch (key)
    {
    case FUSE_OPT_KEY_NONOPT:
        if (rw_path == 0)
        {
            rw_path = strdup(arg);
            return 0;
        }
        else
        {
            return 1;
        }
    case FUSE_OPT_KEY_OPT:
        return 1;
    case KEY_HELP:
        usage(outargs->argv[0]);
        exit(0);
    case KEY_VERSION:
        fprintf(stdout, "ROFS version %s\n", rofsVersion);
        exit(0);
    default:
        fprintf(stderr, "see `%s -h' for usage\n", outargs->argv[0]);
        exit(1);
    }
    return 1;
}

static struct fuse_opt rofs_opts[] = {
    FUSE_OPT_KEY("-h",          KEY_HELP),
    FUSE_OPT_KEY("--help",      KEY_HELP),
    FUSE_OPT_KEY("-V",          KEY_VERSION),
    FUSE_OPT_KEY("--version",   KEY_VERSION),
    FUSE_OPT_END
};

int main(int argc, char *argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int res;

    res = fuse_opt_parse(&args, &rw_path, rofs_opts, rofs_parse_opt);
    if (res != 0)
    {
        fprintf(stderr, "Invalid arguments\n");
        fprintf(stderr, "see `%s -h' for usage\n", argv[0]);
        exit(1);
    }
    if (rw_path == 0)
    {
        fprintf(stderr, "Missing readwritepath\n");
        fprintf(stderr, "see `%s -h' for usage\n", argv[0]);
        exit(1);
    }

    fuse_main(args.argc, args.argv, &rofs_oper, NULL);

    return 0;
}
