/**@file elfy.c
 * @author 何凱帆
 * @brief elf程序目標文件分析
 * @details
 * 實現elf程序目標文件的全局符號讀取和程序段提取
*/

#include "Misc\stdafx.h"

#include <malloc.h>
#include <string.h>

#include "pofunc.h"

#define NOT_ELF_FILE "Not an ELF file\n"
#define MALLOC_ERROR "malloc error\n"

typedef unsigned short Elf32_Half;
typedef unsigned long  Elf32_Word;
typedef unsigned long  Elf32_Off;
typedef unsigned long  Elf32_Addr;

#define PT_LOAD    1
#define phdr_is_load(elf,i) (elf->phdr[i].p_type == PT_LOAD)
#define SHT_SYMTAB 2
#define sym_is_global(elf,i) ((elf->sym[i].st_info>>4) == 1)

struct ELF_Header {
	unsigned char  e_ident[16];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off  e_phoff;
	Elf32_Off  e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
};

struct ELF_phdr {
  Elf32_Word p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
};

struct ELF_shdr {
  Elf32_Word sh_name;
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off  sh_offset;
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
};
#define elf_sec_off(elf, i) elf->shdr[i].sh_offset
#define elf_sec_vma(elf, i) elf->shdr[i].sh_addr

struct ELF_sym {
  Elf32_Word st_name;
  Elf32_Addr st_value;
  Elf32_Word st_size;
  unsigned char st_info;
  unsigned char st_other;
  Elf32_Half st_shndx;
};
#define elf_get_sym_hdl(s) (struct ELF_sym*) (s)
#define elf_sym_vma(s)  s->st_value
#define elf_sym_size(s) s->st_size
#define elf_sym_shndx(s) s->st_shndx

/**
 * @class elf_hdl_t
 * @author 何凱帆
 * @date 2016/10/18
 * @file elfy.c
 * @brief POF_HDL在elfy.c的實現
 */
struct elf_hdl_t {
  struct pofunc *func;
  FILE          *fp;

  struct ELF_Header hdr;
  struct ELF_phdr  *phdr;
  struct ELF_shdr  *shdr;
  struct ELF_sym   *sym;
  unsigned long    sym_num;
  char             *shstr;
  char             *symstr;
};
typedef struct elf_hdl_t *elf_hdl;
#define elf_get_hdl(h) (elf_hdl) (h)

static int elf_read_table( elf_hdl elf )
{
  unsigned char *id;
  int i;
  FILE *fp = elf->fp;

  //printf ("****Read Header\n");
  fseek (fp, 0, SEEK_SET);
  if (fread(&elf->hdr, 1, sizeof(struct ELF_Header), fp) != sizeof(struct ELF_Header)) {
    printf (NOT_ELF_FILE);
    return 0;
  }

  id = elf->hdr.e_ident;
  if (id[0]!=0x7F || id[1]!='E' || id[2]!='L' || id[3]!='F') {
    printf (NOT_ELF_FILE);
    return 0;
  }

  elf->phdr = (struct ELF_phdr*) malloc (sizeof(struct ELF_phdr) * elf->hdr.e_phnum);
  if (elf->phdr == NULL) {
    fprintf( stderr, MALLOC_ERROR );
    return 0;
  }
  elf->shdr = (struct ELF_shdr*) malloc (sizeof(struct ELF_shdr) * elf->hdr.e_shnum);
  if (elf->shdr == NULL) {
    free (elf->phdr);
    fprintf( stderr, MALLOC_ERROR );
    return 0;
  }

  //printf ("****Read Program Header\n");
  fseek (fp, elf->hdr.e_phoff, SEEK_SET);
  fread (elf->phdr, 1, sizeof(struct ELF_phdr) * elf->hdr.e_phnum, fp);
  //printf ("****Read Section Header\n");
  fseek (fp, elf->hdr.e_shoff, SEEK_SET);
  fread (elf->shdr, 1, sizeof(struct ELF_shdr) * elf->hdr.e_shnum, fp);

  elf->shstr = (char*) malloc (elf->shdr[elf->hdr.e_shstrndx].sh_size);
  if (elf->shstr == NULL) {
    free (elf->shdr);
    free (elf->phdr);
    fprintf( stderr, MALLOC_ERROR );
    return 0;
  }
  //printf ("****Read Section String\n");
  fseek (fp, elf->shdr[elf->hdr.e_shstrndx].sh_offset, SEEK_SET);
  fread (elf->shstr, 1, elf->shdr[elf->hdr.e_shstrndx].sh_size, fp);

  for (i=0; i<elf->hdr.e_shnum; ++i)
    if (elf->shdr[i].sh_type == SHT_SYMTAB) break;
  if (i == elf->hdr.e_shnum)
  	printf ("No Symbol Table found!\n");
  elf->sym = (struct ELF_sym*) malloc (elf->shdr[i].sh_size);
  if (elf->sym == NULL) {
    free (elf->shstr);
    free (elf->shdr);
    free (elf->phdr);
    fprintf( stderr, MALLOC_ERROR );
    return 0;
  }
  elf->symstr = (char*) malloc (elf->shdr[elf->shdr[i].sh_link].sh_size);
  if (elf->symstr == NULL) {
    free (elf->sym );
    free (elf->shstr);
    free (elf->shdr);
    free (elf->phdr);
    fprintf( stderr, MALLOC_ERROR );
    return 0;
  }
printf ("****Read Symbol Table\n");
  fseek (fp, elf->shdr[i].sh_offset, SEEK_SET);
  fread (elf->sym, 1, elf->shdr[i].sh_size, fp);
printf ("****Read Symbol String %d\n", elf->shdr[elf->shdr[i].sh_link].sh_size);
  fseek (fp, elf->shdr[elf->shdr[i].sh_link].sh_offset, SEEK_SET);
  fread (elf->symstr, 1, elf->shdr[elf->shdr[i].sh_link].sh_size, fp);
printf ("****Read Table End\n");
  elf->sym_num = elf->shdr[i].sh_size / elf->shdr[i].sh_entsize;
  return 1;
}

static elf_hdl elf_new_hdl (FILE *fp)
{
  elf_hdl hdl = (elf_hdl) malloc( sizeof( struct elf_hdl_t ) );

  if( hdl == NULL ) {
    fprintf( stderr, MALLOC_ERROR );
    return NULL;
  }

  hdl->fp = fp;
  if( elf_read_table( hdl ))
    return hdl;

  free( hdl );
  return NULL;
}

void elf_close (POF_HDL h)
{
  elf_hdl hdl = elf_get_hdl(h);

  free( hdl->symstr );
  free( hdl->sym );
  free( hdl->shstr );
  free( hdl->shdr );
  free( hdl->phdr );

  fclose( hdl->fp );
}

POF_HDL elf_probe (char *name, FILE *fp)
{
  uint32_t magic;
  elf_hdl hdl;

  fseek( fp, 0, SEEK_SET );
  fread( &magic, sizeof magic, 1, fp );

  if( magic != 0x464C457F )
    return NULL;

  hdl = elf_new_hdl( fp );

  return hdl;
}

SYM_HDL elf_lookup_symbol (POF_HDL h, char* name)
{
  int i;
  elf_hdl hdl = elf_get_hdl(h);

  for( i=0; i<hdl->sym_num; ++i ) {
    if( sym_is_global( hdl, i ) && !strcmp( name, hdl->symstr+hdl->sym[i].st_name) )
      return &hdl->sym[i];
  }
  return NULL;
}

uint32_t elf_symbol_vma (POF_HDL h, SYM_HDL s)
{
  struct ELF_sym *sym = elf_get_sym_hdl(s);
  return elf_sym_vma( sym );
}

uint32_t elf_symbol_long (POF_HDL h, SYM_HDL s)
{
  elf_hdl hdl = elf_get_hdl(h);
  struct ELF_sym *sym = elf_get_sym_hdl(s);
  int sn = elf_sym_shndx(sym);
  uint32_t val;

  fseek( hdl->fp, elf_sec_off(hdl,sn)+elf_sym_vma(sym)-elf_sec_vma(hdl,sn), SEEK_SET );
  fread( &val, sizeof val, 1, hdl->fp );

  return val;
}

char* elf_symbol_str (POF_HDL h, SYM_HDL s)
{
  elf_hdl hdl = elf_get_hdl(h);
  struct ELF_sym *sym = elf_get_sym_hdl(s);
  int sn = elf_sym_shndx(sym);
  int sz = elf_sym_size (sym);
  char *str = (char*) malloc( sz );

  if( str == NULL ) {
    fprintf( stderr, MALLOC_ERROR );
    return NULL;
  }

  fseek( hdl->fp, elf_sec_off(hdl,sn)+elf_sym_vma(sym)-elf_sec_vma(hdl,sn), SEEK_SET );
  fread( str, 1, sz, hdl->fp );

  return str;
}

#define TRANS_SIZE 0x8000
/**
 * @class elf_ctx
 * @author 何凱帆
 * @date 2016/10/19
 * @file elfy.c
 * @brief CTX_HDL的elf實現
 * @details 注意保持CTX_HDL定義的一致性
 */
struct elf_ctx {
  uint32_t addr;
  char buf[TRANS_SIZE];
  uint32_t i;
  uint32_t o;
};

CTX_HDL elf_context_init (POF_HDL h)
{
  struct elf_ctx *ctx = (struct elf_ctx*) malloc (sizeof(struct elf_ctx));

  if( ctx == NULL ) {
    fprintf( stderr, MALLOC_ERROR );
    return NULL;
  }

  ctx->i = ~0;
  ctx->o = 0-TRANS_SIZE;

  return ctx;
}

uint32_t elf_get_next_seg (POF_HDL h, CTX_HDL c)
{
  elf_hdl hdl = elf_get_hdl(h);
  struct elf_ctx *ctx = (struct elf_ctx*) c;
  uint32_t s;
  uint32_t i, o, on;
  uint32_t fs;
  uint32_t ms;

  i = ctx->i;
  o = ctx->o;
  o += TRANS_SIZE;
  if( o==0 ) {
    for (++i; i < hdl->hdr.e_phnum; ++i) {
      if( phdr_is_load( hdl, i ) && hdl->phdr[i].p_memsz ) {
        fseek( hdl->fp, hdl->phdr[i].p_offset, SEEK_SET);
        goto do_it;
      }
    }
    ctx->i = i;
    ctx->o = o;
    return 0;
  }

do_it:
  fs = hdl->phdr[i].p_filesz & ~(TRANS_SIZE-1);
  ms = hdl->phdr[i].p_memsz;

  on = o + TRANS_SIZE;
  /* 清0 */
  if( on > fs )
    memset( ctx->buf, 0, TRANS_SIZE );

  if( o < hdl->phdr[i].p_filesz ) {
    /* 需要讀 */
    if( on < hdl->phdr[i].p_filesz )
      fread( ctx->buf, 1, TRANS_SIZE, hdl->fp );
    else
      fread( ctx->buf, 1, hdl->phdr[i].p_filesz - o, hdl->fp );
  }

  ctx->addr = o + hdl->phdr[i].p_vaddr;
  if( on > ms ) {
    s = ms - o;
    o = 0-TRANS_SIZE;
  } else
    s = TRANS_SIZE;

  ctx->i = i;
  ctx->o = o;

  return s;
}

struct pofunc pof_elf = {
	"elf",
	elf_probe,
	elf_close,

	elf_lookup_symbol,
	elf_symbol_vma,
	elf_symbol_str,
	elf_symbol_long,

	elf_context_init,
	elf_get_next_seg,

};