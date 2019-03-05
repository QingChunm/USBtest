#ifndef _POFUNC_H_
#define _POFUNC_H_

extern "C" {
#include <stdio.h>
//#include <stdint.h>

typedef unsigned   uint32_t;
/**
 * @class POF_SYM
 * @author 何凱帆
 * @date 2016/10/18
 * @file pofunc.h
 * @brief 用於提取全局符號的表格
 * 
 * 目前，這個結構體比較彆扭，必須與pof.h中的相同結構體定義一致
 */

typedef struct POF_SYM
{
	unsigned long *val;
	unsigned long  type;
	char          *name;
} POF_SYM, *pPOF_SYM;

#define SYM_TYPE_VMA  0
#define SYM_TYPE_LONG 1
#define SYM_TYPE_STR  2

#ifndef POF_HDL
# define POF_HDL void *
#endif

#ifndef SYM_HDL
# define SYM_HDL void *
#endif

#ifndef CTX_HDL
# define CTX_HDL void *
#endif

struct pofunc {
  char *name;

  POF_HDL (*probe) (char *name, FILE *fp);  /**< probe and allocate */
  void (*close) (POF_HDL);

  SYM_HDL (*lookup_symbol) (POF_HDL, char*);
  uint32_t (*symbol_vma) (POF_HDL, SYM_HDL);
  char* (*symbol_str) (POF_HDL, SYM_HDL);
  uint32_t (*symbol_long) (POF_HDL, SYM_HDL);

  CTX_HDL (*context_init) (POF_HDL);
  uint32_t (*get_next_seg) (POF_HDL, CTX_HDL);
};

void elf_close (POF_HDL h);
POF_HDL elf_probe (char *name, FILE *fp);
SYM_HDL elf_lookup_symbol (POF_HDL h, char* name);
uint32_t elf_symbol_vma (POF_HDL h, SYM_HDL s);
uint32_t elf_symbol_long (POF_HDL h, SYM_HDL s);
char* elf_symbol_str (POF_HDL h, SYM_HDL s);
CTX_HDL elf_context_init (POF_HDL h);
uint32_t elf_get_next_seg (POF_HDL h, CTX_HDL c);

//#define pof_context_free(ctx) free(ctx);
//int __cdecl pof_read_symbol (POF_HDL h, pPOF_SYM sym);
//CTX_HDL __cdecl pof_context_init (POF_HDL h);
//uint32_t __cdecl pof_get_next_seg (POF_HDL h, CTX_HDL c);
//void __cdecl pof_close( POF_HDL h );

//POF_HDL __cdecl pof_open_file (char *name, FILE *fp);
POF_HDL __cdecl pof_open_file (char *name);
void    __cdecl pof_close     (POF_HDL);

int __cdecl pof_read_symbol (POF_HDL, pPOF_SYM);
CTX_HDL __cdecl pof_context_init (POF_HDL);
uint32_t __cdecl pof_get_next_seg (POF_HDL, CTX_HDL);

}
#endif  /* _POFUNC_H_ */
