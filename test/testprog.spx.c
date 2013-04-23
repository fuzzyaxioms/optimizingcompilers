/*
 * This file was created automatically from SUIF
 *   on Sun Apr 14 23:41:08 2013.
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
extern float f1(float, float);
extern int f2(int);
extern int f3(int, int);
extern int f4(int, int);
extern int f5(int, int);
extern int f6(int);
extern int f7(int, int);
extern void main();
extern int exit();

extern float f1(float b, float c)
  {
    int i;
    float j;
    float k;

    i = 0;
  L3:
    k = b * (float)i;
  L1:
    i = i + 1;
    if (!(300 <= i))
        goto L3;
  L2:
    return k;
  }

extern int f2(int b)
  {
    int i;
    int j;
    int k;

    return 900 + 1 * b;
  }

extern int f3(int b, int c)
  {
    int i;
    int j;
    int k;

    return 0 + 299 * b;
  }

extern int f4(int b, int c)
  {
    int i;
    int j;
    int k;
    int _preg14;

    j = 0;
    i = 0;
    _preg14 = b + c;
  L3:
    j = j + _preg14;
  L1:
    i = i + 1;
    if (!(300 <= i))
        goto L3;
  L2:
    return j;
  }

extern int f5(int b, int c)
  {
    int i;
    int j;
    int k;

    return 0 + 299 * c;
  }

extern int f6(int c)
  {
    int a[300];
    int b[300];
    int i;
    int k;
    int _preg68;

    k = 0;
    i = 0;
  L7:
    *(int *)((char *)a + i * 4) = i;
  L1:
    i = i + 1;
    if (!(300 <= i))
        goto L7;
  L2:
    i = 0;
  L8:
    _preg68 = i * 4;
    *(int *)((char *)b + _preg68) = c + *(int *)((char *)a + _preg68);
  L3:
    i = i + 1;
    if (!(300 <= i))
        goto L8;
  L4:
    i = 0;
  L9:
    k = k + *(int *)((char *)b + i * 4);
  L5:
    i = i + 1;
    if (!(300 <= i))
        goto L9;
  L6:
    return k;
  }

extern int f7(int b, int c)
  {
    int i;
    int j;

    return 0 + 1 * c;
  }

extern void main()
  {
    int t;
    struct _IO_FILE *f;
    float suif_tmp0;
    int suif_tmp1;
    int suif_tmp2;
    int suif_tmp3;
    int suif_tmp4;
    int suif_tmp5;
    int suif_tmp6;

    t = (float)3 + f1(4.0F, 5.0F);
    t = t + f2(3);
    t = t + f3(4, 5);
    t = t + f4(4, 5);
    t = t + f5(4, 5);
    t = t + f6(3);
    t = t + f7(3, 4);
    f = fopen("testprog.out", "w");
    fprintf(f, "finished: %d\n", t);
    fclose(f);
    exit(0);
    return;
  }
