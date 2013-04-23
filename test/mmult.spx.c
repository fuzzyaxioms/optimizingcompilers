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
extern void matrix_multiply(int (*)[100], int (*)[100], int (*)[100], int);
extern void main();
extern int exit();

extern void matrix_multiply(int (*C)[100], int (*A)[100], int (*B)[100], int n)
  {
    int i;
    int j;
    int k;
    int *suif_tmp1;
    int _preg166;
    int *_preg122;
    int *_preg112;
    int *_preg109;
    int *_preg106;
    int _preg168;
    int _preg170;
    int _preg211;
    int *_preg79;

    _preg166 = 0 < n;
    if (!_preg166)
        goto L12;
    i = 0;
    _preg122 = (int *)C;
    _preg112 = (int *)B;
    _preg109 = (int *)A;
    _preg106 = (int *)C;
  L13:
    if (!_preg166)
        goto L9;
    j = 0;
    _preg168 = i * 400;
  L10:
    _preg170 = j * 4;
    _preg211 = _preg168 + _preg170;
    *(int *)((char *)_preg122 + _preg211) = 0;
    if (!_preg166)
        goto L7;
    k = 0;
    _preg79 = (int *)((char *)_preg106 + _preg211);
  L8:
    *_preg79 = *_preg79 + *(int *)((char *)_preg109 + _preg168 + k * 4) * *(int *)((char *)_preg112 + k * 400 + _preg170);
  L5:
    k = k + 1;
    if (!(n <= k))
        goto L8;
  L6:
    goto __done9;
  L7:
  __done9:
  L3:
    j = j + 1;
    if (!(n <= j))
        goto L10;
  L4:
    goto __done11;
  L9:
  __done11:
  L1:
    i = i + 1;
    if (!(n <= i))
        goto L13;
  L2:
    goto __done14;
  L12:
  __done14:
    return;
  }

extern void main()
  {
    int i;
    int j;
    int (A[100])[100];
    int (B[100])[100];
    int (C[100])[100];
    struct _IO_FILE *f;
    int _preg143;
    int _preg201;

    i = 0;
  L10:
    j = 0;
    _preg143 = i * 400;
  L9:
    _preg201 = _preg143 + j * 4;
    *(int *)((char *)A[0] + _preg201) = i * j;
    if (!(j != 0))
        goto L5;
    *(int *)((char *)B[0] + _preg201) = i / j;
    goto __done8;
  L5:
    *(int *)((char *)B[0] + _preg201) = 0;
  __done8:
  L3:
    j = j + 1;
    if (!(100 <= j))
        goto L9;
  L4:
  L1:
    i = i + 1;
    if (!(100 <= i))
        goto L10;
  L2:
    matrix_multiply((int (*)[100])C[0], (int (*)[100])A[0], (int (*)[100])B[0], 100);
    f = fopen("mmult.out", "w");
    i = 0;
  L11:
    fprintf(f, "C[%d][%d] = %d\n", i, i, *(int *)((char *)C[0] + i * 400 + i * 4));
  L6:
    i = i + 1;
    if (!(100 <= i))
        goto L11;
  L7:
    fclose(f);
    exit(0);
    return;
  }
