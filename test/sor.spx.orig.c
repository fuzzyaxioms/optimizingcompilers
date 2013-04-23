/*
 * This file was created automatically from SUIF
 *   on Sat Apr 13 12:01:27 2013.
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

extern double (x[300])[300];

extern int main();
extern int exit();

double (x[300])[300];

extern int main()
  {
    int i;
    int j;
    int k;
    int t;
    double ctr;
    struct _IO_FILE *f;
    unsigned int _preg219;
    int _preg227;
    int _preg817;
    int _preg326;
    int _preg325;
    int _preg822;
    int _preg280;
    int _preg279;
    int _preg1121;
    int _preg286;
    int _preg285;
    int _preg284;
    int _preg283;
    double *_preg1411;
    double *_preg290;
    double *_preg289;
    double *_preg288;
    double *_preg287;
    double *_preg553;
    double *_preg552;
    int _preg827;
    int _preg480;
    int _preg479;
    int _preg832;
    int _preg434;
    int _preg433;
    int _preg1122;
    int _preg440;
    int _preg439;
    int _preg438;
    int _preg437;
    double *_preg1412;
    double *_preg444;
    double *_preg443;
    double *_preg442;
    double *_preg441;

    ctr = 0.0;
    i = 0;
  L23:
    j = 0;
    _preg219 = i;
    _preg227 = i * 2400;
  L22:
    ctr = ctr + (double)(int)(_preg219 | (unsigned int)j);
    if (!(200.0 < ctr))
        goto L5;
    ctr = ctr - 300.0;
  L5:
    *(double *)((char *)x[0] + _preg227 + j * 8) = ctr;
  L3:
    j = j + 1;
    if (!(300 <= j))
        goto L22;
  L4:
  L1:
    i = i + 1;
    if (!(300 <= i))
        goto L23;
  L2:
    t = 0;
  L27:
    i = 1;
  L26:
    j = 1;
    _preg817 = i * 2400;
    _preg326 = _preg817 - 2400;
    _preg325 = _preg817 + 2400;
  L25:
    k = 1;
    _preg822 = j * 8;
    _preg280 = _preg822 + 8;
    _preg279 = _preg822 - 8;
    _preg1121 = _preg817 + _preg822;
    _preg286 = _preg817 + _preg280;
    _preg285 = _preg817 + _preg279;
    _preg284 = _preg325 + _preg822;
    _preg283 = _preg326 + _preg822;
    _preg1411 = (double *)((char *)x[0] + _preg1121);
    _preg290 = (double *)((char *)x[0] + _preg286);
    _preg289 = (double *)((char *)x[0] + _preg285);
    _preg288 = (double *)((char *)x[0] + _preg284);
    _preg287 = (double *)((char *)x[0] + _preg283);
  L24:
    *_preg1411 = (*_preg1411 + *_preg287 + *_preg288 + *_preg289 + *_preg290) / 5.0 + (double)k;
  L12:
    k = k + 1;
    if (!(5 <= k))
        goto L24;
  L13:
  L10:
    j = j + 1;
    if (!(299 <= j))
        goto L25;
  L11:
  L8:
    i = i + 1;
    if (!(299 <= i))
        goto L26;
  L9:
  L6:
    t = t + 1;
    if (!(5 <= t))
        goto L27;
  L7:
    f = fopen("sor.out", "w");
    t = 0;
    _preg553 = x[0] + 44800;
    _preg552 = x[0] + 6005;
  L31:
    i = 1;
  L30:
    j = 1;
    _preg827 = i * 2400;
    _preg480 = _preg827 - 2400;
    _preg479 = _preg827 + 2400;
  L29:
    k = 1;
    _preg832 = j * 8;
    _preg434 = _preg832 + 8;
    _preg433 = _preg832 - 8;
    _preg1122 = _preg827 + _preg832;
    _preg440 = _preg827 + _preg434;
    _preg439 = _preg827 + _preg433;
    _preg438 = _preg479 + _preg832;
    _preg437 = _preg480 + _preg832;
    _preg1412 = (double *)((char *)x[0] + _preg1122);
    _preg444 = (double *)((char *)x[0] + _preg440);
    _preg443 = (double *)((char *)x[0] + _preg439);
    _preg442 = (double *)((char *)x[0] + _preg438);
    _preg441 = (double *)((char *)x[0] + _preg437);
  L28:
    *_preg1412 = (*_preg1412 + *_preg441 + *_preg442 + *_preg443 + *_preg444) / 5.0 + (double)k;
  L20:
    k = k + 1;
    if (!(5 <= k))
        goto L28;
  L21:
  L18:
    j = j + 1;
    if (!(299 <= j))
        goto L29;
  L19:
  L16:
    i = i + 1;
    if (!(299 <= i))
        goto L30;
  L17:
    fprintf(f, "x[20][5] = %.3f, x[149][100] = %.3f\n", *_preg552, *_preg553);
  L14:
    t = t + 1;
    if (!(5 <= t))
        goto L31;
  L15:
    fclose(f);
    exit(0);
    return 0;
  }
