/*
 * This file was created automatically from SUIF
 *   on Sun Apr 14 23:38:45 2013.
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
extern void main();
extern int exit();

extern void main()
  {
    int i;
    int j;
    int k;
    int (A[50])[25];
    int (B[25])[50];
    int (C[50])[50];
    struct _IO_FILE *f;
    int _preg270;
    int _preg116;
    int _preg167;
    int _preg166;
    int _preg271;
    int *_preg151;

    i = 0;
  L16:
    j = 0;
    _preg270 = i * 4;
    _preg116 = i * 100;
  L15:
    *(int *)((char *)A[0] + _preg116 + j * 4) = i * j;
    if (!(j != 0))
        goto L5;
    *(int *)((char *)B[0] + j * 200 + _preg270) = i / j;
    goto __done14;
  L5:
    *(int *)((char *)B[0] + j * 200 + _preg270) = 0;
  __done14:
  L3:
    j = j + 1;
    if (!(25 <= j))
        goto L15;
  L4:
  L1:
    i = i + 1;
    if (!(50 <= i))
        goto L16;
  L2:
    i = 0;
  L19:
    j = 0;
    _preg167 = i * 200;
    _preg166 = i * 100;
  L18:
    k = 0;
    _preg271 = j * 4;
    _preg151 = (int *)((char *)C[0] + _preg167 + _preg271);
  L17:
    *_preg151 = i * *(int *)((char *)A[0] + _preg166 + k * 4) - *(int *)((char *)B[0] + k * 200 + _preg271);
  L10:
    k = k + 1;
    if (!(25 <= k))
        goto L17;
  L11:
  L8:
    j = j + 1;
    if (!(50 <= j))
        goto L18;
  L9:
  L6:
    i = i + 1;
    if (!(50 <= i))
        goto L19;
  L7:
    f = fopen("arrays.out", "w");
    i = 0;
  L20:
    fprintf(f, "C[%d][%d] = %d\n", i, i, *(int *)((char *)C[0] + i * 200 + i * 4));
  L12:
    i = i + 1;
    if (!(50 <= i))
        goto L20;
  L13:
    fclose(f);
    exit(0);
    return;
  }
