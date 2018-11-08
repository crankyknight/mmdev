/* Code to test read/write, ioctl and mmap.
 * Blocking read is not tested here (tested separately
 * using command line tools).
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>

#include "test.h"

unsigned int cur_size, tot_size;
char print_flag = 0;

int rw_seek_test(int fd)
{
    int n;
    int fail=0;
    char wbuf[DEV_SIZE_DEFAULT] = "This is a read/write test";
    char rbuf[DEV_SIZE_DEFAULT]; 
    VERBOSE_PRINT("Testing simple read/write :\n");
    n = write(fd, wbuf, 50);
    if(n != 50) { fail++; VERBOSE_PRINT("FAIL :"); }
    cur_size += n;
    VERBOSE_PRINT("- %d bytes written\n", n);

    /*Go to start*/
    lseek(fd, 0, SEEK_SET);
    n = read(fd, rbuf, 100);
    if(n != 100) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- %d bytes read from SEEK_SET (%s)\n", n, rbuf);

    /*Testing lseek*/
    lseek(fd, 2000, SEEK_CUR);
    n = read(fd, rbuf, 100);
    if(n != 100) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- %d bytes read from SEEK_CUR (%s)\n", n, rbuf);

    lseek(fd, -100, SEEK_END);
    n = read(fd, rbuf, 100);
    if(n != 100) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- %d bytes read from SEEK_END (%s)\n", n, rbuf);

    /*This read should fail */
    lseek(fd, 100, SEEK_END);
    n = read(fd, rbuf, 100);
    if(n != -1) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- %d bytes read from SEEK_END (%s)\n", n, rbuf);

    /*Go to start before leaving */
    lseek(fd, 0, SEEK_SET);
    return fail;
}

int ioctl_test(int fd)
{
    char filler;
    int n, arg, fail=0;
    VERBOSE_PRINT("IOCTL testing :\n");

    n = ioctl(fd, MMDEV_IOCGTOTSIZE, &arg);
    if(n) fail++;
    if(arg != tot_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Total size = %d\n", arg);
    tot_size = 2048;
    n = ioctl(fd, MMDEV_IOCSTOTSIZE, &tot_size);
    if(n) fail++;
    n = ioctl(fd, MMDEV_IOCGTOTSIZE, &arg);
    if(n) fail++;
    if(arg != tot_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Total size = %d\n", arg);

    n = ioctl(fd, MMDEV_IOCGCURSIZE, &arg);
    if(n) fail++;
    if(arg != cur_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Current size = %d\n", arg);
    cur_size = tot_size + 100; /* Return value should be tot size */
    n = ioctl(fd, MMDEV_IOCSCURSIZE, &cur_size);
    if(n) fail++;
    n = ioctl(fd, MMDEV_IOCGCURSIZE, &arg);
    if(n) fail++;
    if(arg != tot_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Current size = %d\n", arg);

    arg = tot_size - 100;
    n = ioctl(fd, MMDEV_IOCXCURSIZE, &arg);
    if(arg != tot_size - 100) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Current size = %d\n", arg);

    /*Reset device */
    VERBOSE_PRINT("***************Resetting device****************\n");
    n = ioctl(fd, MMDEV_IOCRESET);
    if(n) fail++;
    tot_size = DEV_SIZE_DEFAULT;
    cur_size = 0;
    /*Check sizes again */
    n = ioctl(fd, MMDEV_IOCGTOTSIZE, &arg);
    if(n) fail++;
    if(arg != tot_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Total size = %d\n", arg);
    n = ioctl(fd, MMDEV_IOCGCURSIZE, &arg);
    if(n) fail++;
    if(arg != cur_size) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Current size = %d\n", arg);

    filler = DEV_FILLER_DEFAULT-2; /*'X'*/
    n = ioctl(fd, MMDEV_IOCXFILLER, &filler);
    if(filler != DEV_FILLER_DEFAULT) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- Current filler = %c\n", DEV_FILLER_DEFAULT-2);

    return fail;
}

static int mem_map(int fd)
{
    char *dev;
    char rbuf[DEV_SIZE_DEFAULT] = "Write from mmap";
    int i, n, fail=0;
    /*Map entire device */
    dev = (char*)mmap(NULL, DEV_SIZE_DEFAULT, PROT_READ|PROT_WRITE, 
                            MAP_SHARED, fd, 0);
    if(dev == MAP_FAILED){
        perror("Mmap Failed!!\n");
        return 1;
    }

    /*write and check dev */
    n = write(fd, rbuf, 50);
    VERBOSE_PRINT("Mmaped read is - %s\n", dev);

    /*Write */
    for(i=0; i<DEV_SIZE_DEFAULT - 100; i++){
        dev[i] = i & 0xff;
        /*Remove Nulls for print */
        dev[i] = dev[i] == 0 ? 0xff : dev[i];
    }
    
    /*Try IOCTL, should fail */
    n = ioctl(fd, MMDEV_IOCRESET);
    if(!n) fail++;

    /*Read device */
    lseek(fd, 0, SEEK_SET);
    n = read(fd, rbuf, DEV_SIZE_DEFAULT);
    if(n != DEV_SIZE_DEFAULT) { fail++; VERBOSE_PRINT("FAIL :"); }
    VERBOSE_PRINT("- %d bytes read from SEEK_SET (%s)\n", n, rbuf);

    /*Unmap */
    n = munmap(dev, DEV_SIZE_DEFAULT);
    if(n){
        perror("Unmap failed!");
        fail++;
    }
    lseek(fd, 0, SEEK_SET);

    return fail; 
}

void print_help()
{
    char help_msg[] = 
	"Usage : ./a.out [optargs]\n\
	Optargs :\n\
    	-v : Run in verbose mode. Output printed at each stage.\n\
    	-h : Print this message and exit\n";
    printf("%s", help_msg);
}

int main(int argc, char *argv[])
{
    int fd;
    int opt;
    while((opt = getopt(argc, argv, "vh")) != -1)
    {
        switch(opt){
            case 'v':
                print_flag = 1;
                break;
            case 'h' :
                print_help();
                exit(1);
                break;
            default :
                printf("Invalid option!!!\n");
                print_help();
                exit(1);
                break;
        }
    }

    int summary[10];
    fd = open("/dev/mmdev0", O_RDWR);
    if(fd == -1) {
        printf("File open failed!!\n");
        exit(1);
    }
    cur_size = 0;
    tot_size = DEV_SIZE_DEFAULT;
    summary[0] = rw_seek_test(fd);
    VERBOSE_PRINT("\n\n");
    summary[1] = ioctl_test(fd);
    VERBOSE_PRINT("\n\n");
    summary[2] = rw_seek_test(fd);
    VERBOSE_PRINT("\n\n");
    summary[3] = mem_map(fd);
    VERBOSE_PRINT("\n\n");
    summary[4] = rw_seek_test(fd);

    VERBOSE_PRINT("\n\n\n\n\n");
    printf("******* TESTS SUMMARY **********\n");
    printf("0 -> Pass    >0 -> Fail\n");
    printf("1. Read write test = %d\n", summary[0]);
    printf("2. Ioctl test      = %d\n", summary[1]);
    printf("3. Read write test = %d\n", summary[2]);
    printf("4. Mmap test       = %d\n", summary[3]);
    printf("5. Read write test = %d\n", summary[4]);

    close(fd);
}
