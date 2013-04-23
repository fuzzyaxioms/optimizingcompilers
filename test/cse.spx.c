/*
 * This file was created automatically from SUIF
 *   on Sun Apr 14 23:38:46 2013.
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

extern double x[10000];

extern int main();
extern int exit();

double x[10000];

extern int main()
  {
    int i;
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    double ctr;
    struct _IO_FILE *file;
    int _preg141;
    int _preg140;
    int _preg186;
    int _preg230;
    int _preg273;

    ctr = 0.0;
    i = 0;
  L8:
    ctr = ctr + (double)i;
    if (!(200.0 < ctr))
        goto L3;
    ctr = ctr - 300.0;
  L3:
    *(double *)((char *)x + i * 8) = ctr;
  L1:
    i = i + 1;
    if (!(10000 <= i))
        goto L8;
  L2:
    i = 1;
  L9:
    _preg141 = i * 8;
    _preg140 = (int)*(double *)((char *)x + _preg141 + 8) + (int)*(double *)((char *)x + (_preg141 - 8));
    _preg186 = _preg140 + _preg140;
    _preg230 = _preg186 + _preg186;
    _preg273 = _preg230 + _preg230;
    *(double *)((char *)x + _preg141) = (_preg273 + _preg273) % 200;
  L4:
    i = i + 1;
    if (!(9999 <= i))
        goto L9;
  L5:
    file = fopen("cse.out", "w");
    i = 10;
  L10:
    fprintf(file, "x[%d] = %.3f\n", i, *(double *)((char *)x + i * 8));
  L6:
    i = i + 1;
    if (!(20 <= i))
        goto L10;
  L7:
    fclose(file);
    exit(0);
    return 0;
  }
