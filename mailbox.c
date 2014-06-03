#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>		/* ioctl */
#include "vcio.h"

#define PAGE_SIZE (4*1024)

static void *mapmem(unsigned base, unsigned size)
{
   int mem_fd;
   unsigned offset = base % PAGE_SIZE; 
   base = base - offset;
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit (-1);
   }
   void *mem = mmap(
      0,
      size,
      PROT_READ|PROT_WRITE,
      MAP_SHARED/*|MAP_FIXED*/,
      mem_fd, 
      base);
#ifdef DEBUG
   printf("base=0x%x, mem=%p\n", base, mem);
#endif
   if (mem == MAP_FAILED) {
      printf("mmap error %d\n", (int)mem);
      exit (-1);
   }
   close(mem_fd);
   return (char *)mem + offset;
}

static void *unmapmem(void *addr, unsigned size)
{
   int s = munmap(addr, size);
   if (s != 0) {
      printf("munmap error %d\n", s);
      exit (-1);
   }
}

/* 
 * use ioctl to send mbox property message
 */

static int mbox_property(int file_desc, void *buf)
{
   int ret_val = ioctl(file_desc, IOCTL_MBOX_PROPERTY, buf);

   if (ret_val < 0) {
      printf("ioctl_set_msg failed:%d\n", ret_val);
   }

#ifdef DEBUG
   unsigned *p = message; int i; unsigned size = *(unsigned *)buf;
   for (i=0; i<size/4; i++)
      printf("%04x: 0x%08x\n", i*sizeof *p, p[i]);
#endif
   return ret_val;
}

static unsigned get_version(int file_desc)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x00000001; // get firmware version
   p[i++] = 0x00000004; // buffer size
   p[i++] = 0x00000000; // request size
   p[i++] = 0x00000000; // value buffer

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

unsigned mem_alloc(int file_desc, unsigned size, unsigned align, unsigned flags)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000c; // (the tag id)
   p[i++] = 12; // (size of the buffer)
   p[i++] = 12; // (size of the data)
   p[i++] = size; // (num bytes? or pages?)
   p[i++] = align; // (alignment)
   p[i++] = flags; // (MEM_FLAG_L1_NONALLOCATING)

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

unsigned mem_free(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000f; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

unsigned mem_lock(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000d; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

unsigned mem_unlock(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000e; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

unsigned execute_code(int file_desc, unsigned code, unsigned r0, unsigned r1, unsigned r2, unsigned r3, unsigned r4, unsigned r5)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x30010; // (the tag id)
   p[i++] = 28; // (size of the buffer)
   p[i++] = 28; // (size of the data)
   p[i++] = code;
   p[i++] = r0;
   p[i++] = r1;
   p[i++] = r2;
   p[i++] = r3;
   p[i++] = r4;
   p[i++] = r5;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mbox_property(file_desc, p);
   return p[5];
}

// test code for peeking and poking with mmap
int xmain(int argc, char **argv)
{
   int i;
   unsigned base;
   if (argc > 2) {
      base = strtoul(argv[1], NULL, 0);
      volatile unsigned *p = mapmem(base, PAGE_SIZE);
      p[0] = strtoul(argv[2], NULL, 0);
      for (i=0; i<1; i++)
         printf("%04x: 0x%08x\n", i*sizeof *p, p[i]);
      return 0;
   }
   if (argc > 1) {
      base = strtoul(argv[1], NULL, 0);
      volatile unsigned *p = mapmem(base, PAGE_SIZE);
      for (i=0; i<8; i++)
         printf("%04x: 0x%08x\n", i*sizeof *p, p[i]);
      return 0;
   }
   return 0;
}
void *printroutine(void *pointer)
{
	char* ptr = (char*)pointer;
	puts("Thread Runned !");
	while ( 1 == 1) {
		while (ptr[0] == 0) {
			/* puts("Still not anything useful. */
		}
		putc(ptr[1],stdout);
		ptr[0] = 0;
	}
   pthread_exit(NULL);
}
void *flushroutine(void* pointer) {
	while (1 == 1) {
		fflush(stdout);
		sleep(1);
	}
}
/* 
 * Main - Call the ioctl functions 
 */
int main(int argc, char *argv[])
{
   int file_desc, i;
	pthread_t thread,dth;
   // open a char device file used for communicating with kernel mbox driver
   file_desc = open(DEVICE_FILE_NAME, 0);
   if (file_desc < 0) {
      printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
      printf("Try creating a device file with: mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
      exit(-1);
   }
   /* sanity check we care talking to GPU - should match vcgencmd version */
   printf("Version: %d\n", get_version(file_desc));
   int size = 1024*1024; /* 128 KBytes */
   unsigned handle[3], buffer[3]; void *user[3];
   /* allocate memory on GPU, and use accessible mmapped versions */
   for (i=0; i<sizeof handle/sizeof *handle; i++) {
      handle[i] = mem_alloc(file_desc, size, 4096, 0x4);
      buffer[i] = mem_lock(file_desc, handle[i]);
      printf("handle=%x, buffer=%x\n", handle[i], buffer[i]);
      user[i] = mapmem(buffer[i]+0x20000000, size);
      memset(user[i], 0x00, size);
   }
   /* load GPU code into one of the buffers. */
   FILE *fp = fopen("program.bin", "rb");
   assert(fp);
   fread(user[0], 1, size, fp);
   fclose(fp);
	 pthread_create(&thread,NULL,printroutine,user[1]);  /* Just A Thread */
	 pthread_create(&dth,NULL,flushroutine,NULL);
   // execute the GPU code:
	sleep(1);
   int s = execute_code(file_desc, buffer[0], 0,0, buffer[1], buffer[2], size, 0);
	puts("GPU Task Finished");
/*
	char* ptr = (char*)user[1];
	putc(ptr[1],stdout);
*/
	 pthread_cancel(thread);  /* Halting It */
	 pthread_cancel(dth);
   // free up (important - if you don't call mem_free that memory will be lost for good)
   for (i=0; i<sizeof handle/sizeof *handle; i++) {
      munmap(user[i], size);
      mem_unlock(file_desc, handle[i]);
      mem_free(file_desc, handle[i]);
   }
   close(file_desc);
   return 0;
}
