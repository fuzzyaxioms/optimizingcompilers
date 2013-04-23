/*
 * This file was created automatically from SUIF
 *   on Sun Apr 14 23:38:47 2013.
 *
 * Created by:
 * s2c 5.138 (plus local changes) compiled Thu Oct 9 05:14:25 EDT 2008 by kingyen on ug153
 *     Based on SUIF distribution 1.1.2
 *     Linked with:
 *   libsuif 5.228 (plus local changes) compiled Thu Oct 9 05:12:27 EDT 2008 by kingyen on ug153
 *   libuseful 1.243 (plus local changes) compiled Thu Oct 9 05:12:38 EDT 2008 by kingyen on ug153
 */


struct _IO_marker;

struct __tmp_struct1 { int __val[2]; };
struct _IO_FILE { int _flags;
                  char *_IO_read_ptr;
                  char *_IO_read_end;
                  char *_IO_read_base;
                  char *_IO_write_base;
                  char *_IO_write_ptr;
                  char *_IO_write_end;
                  char *_IO_buf_base;
                  char *_IO_buf_end;
                  char *_IO_save_base;
                  char *_IO_backup_base;
                  char *_IO_save_end;
                  struct _IO_marker *_markers;
                  struct _IO_FILE *_chain;
                  int _fileno;
                  int _flags2;
                  int _old_offset;
                  unsigned short _cur_column;
                  char _vtable_offset;
                  char _shortbuf[1];
                  void *_lock;
                  struct __tmp_struct1 _offset;
                  void *__pad1;
                  void *__pad2;
                  void *__pad3;
                  void *__pad4;
                  unsigned int __pad5;
                  int _mode;
                  char _unused2[40]; };
struct _IO_marker { struct _IO_marker *_next;
                    struct _IO_FILE *_sbuf;
                    int _pos; };

extern int fclose(struct _IO_FILE *);
extern struct _IO_FILE *fopen(const char *, const char *);
extern int fprintf(struct _IO_FILE *, const char *, ...);
extern void vector_dotprod(int *, int *, int *, int);
extern void main();
extern int exit();

extern void vector_dotprod(int *C, int *A, int *B, int n)
  {
    int i;
    int j;
    int t1;
    int t2;
    int t3;
    int *_preg77;
    int *_preg75;
    int *_preg73;
    int _preg102;
    int *_preg68;
    int *_preg67;
    int *_preg66;

    if (!(0 < n))
        goto L6;
    i = 0;
    _preg77 = A;
    _preg75 = B;
    _preg73 = C;
  L7:
    j = 0;
    _preg102 = i * 4;
    _preg68 = (int *)((char *)_preg73 + _preg102);
    _preg67 = (int *)((char *)_preg75 + _preg102);
    _preg66 = (int *)((char *)_preg77 + _preg102);
  L5:
    *_preg68 = *_preg66 * *_preg67;
  L3:
    j = j + 1;
    if (!(5 <= j))
        goto L5;
  L4:
  L1:
    i = i + 1;
    if (!(n <= i))
        goto L7;
  L2:
    goto __done8;
  L6:
  __done8:
    return;
  }

extern void main()
  {
    int i;
    int A[1000];
    int B[1000];
    int C[1000];
    struct _IO_FILE *f;
    int _preg65;

    i = 0;
  L5:
    _preg65 = i * 4;
    *(int *)((char *)A + _preg65) = i + i;
    *(int *)((char *)B + _preg65) = i;
  L1:
    i = i + 1;
    if (!(1000 <= i))
        goto L5;
  L2:
    vector_dotprod(C, A, B, 1000);
    f = fopen("dotprod.out", "w");
    i = 0;
  L6:
    fprintf(f, "C[%d] = %d\n", i, *(int *)((char *)C + i * 4));
  L3:
    i = i + 20;
    if (!(1000 <= i))
        goto L6;
  L4:
    fclose(f);
    exit(0);
    return;
  }
