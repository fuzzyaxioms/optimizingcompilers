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
extern void main();
extern int exit();

extern void main()
  {
    int i;
    int c1;
    int c2;
    int x[2000];
    struct _IO_FILE *f;
    int *_preg123;

    i = 0;
  L7:
    _preg123 = (int *)((char *)x + i * 4);
    *_preg123 = 15;
  L3:
    *_preg123 = 20;
  L4:
  L1:
    i = i + 1;
    if (!(2000 <= i))
        goto L7;
  L2:
    f = fopen("cond.out", "w");
    i = 0;
  L8:
    fprintf(f, "%d\n", *(int *)((char *)x + i * 4));
  L5:
    i = i + 1;
    if (!(20 <= i))
        goto L8;
  L6:
    fclose(f);
    exit(0);
    return;
  }
