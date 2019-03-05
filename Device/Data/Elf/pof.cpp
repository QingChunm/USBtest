/**@file pof.c
 * @author 何凱帆
 * @brief 程序目標文件分析
 * @details
 * 實現程序目標文件的全局符號讀取和程序段提取
*/

#include "Misc\stdafx.h"

#include <malloc.h>

#include "pofunc.h"

/**
 * @class pof_hdl_t
 * @author 何凱帆
 * @date 2016/10/18
 * @file pof.c
 * @brief POF_HDL在pof.c的實現
 * @details POF_HDL在pof.c中實現為pof_hdl_t，其成員祇有一個。
 * 要注意，所有具體實現中，第一個成員都必須是struct pofunc *。
 */
struct pof_hdl_t {
  struct pofunc *func;
};
typedef struct pof_hdl_t *pof_hdl;
#define pof_get_hdl(h) (pof_hdl) (h)

/* 所支持的文件格式，在此註冊 */
extern struct pofunc pof_elf;
extern struct pofunc pof_omf;
struct pofunc *pofuncs[] = {
  &pof_elf,
  &pof_omf,
  NULL
};

//POF_HDL pof_open_file( char *name, FILE *fp )
POF_HDL pof_open_file( char *name)
{

#if 1
  FILE *fp = nullptr;
  struct pofunc **pof;
  pof_hdl hdl = NULL;

  fopen_s(&fp, name, "rb");
  if( fp == NULL ) {
    fprintf( stderr, "Can't open file '%s'\r\n", name );
    return NULL;
  }

  for( pof = pofuncs; *pof; ++pof ) {
    if( (*pof)->probe == NULL ) {
      fprintf( stderr, "Error '%s': the probe() should not be NULL\r\n", (*pof)->name );
      continue;
    }
    hdl = (pof_hdl)(*(*pof)->probe)( name, fp );
    if( hdl != NULL ) break;
  }

  if( hdl == NULL ) {
    fclose( fp );
    return NULL;
  }

  hdl->func = *pof;
#endif
#if 0
  POF_HDL  hdl = elf_probe(name, fp);
#endif
  return hdl;
}

void pof_close( POF_HDL h )
{
  pof_hdl hdl = pof_get_hdl(h);
  //(*hdl->func->close) ( h );
  elf_close (h);
  free( h );
}


/*******************************************************************************
 * 定義全局符號表
 ******************************************************************************/
/*uint32_t Code2xDataOffset;
uint32_t func_MemReadWrite;
uint32_t func_probe;
uint32_t func_tgtrw;
uint32_t func_Fini;
uint32_t func_Init;

POF_SYM pubsym[] = {
  {(unsigned  long*)&Code2xDataOffset,  SYM_TYPE_VMA,  "Code2xDataOffset"},
  {(unsigned  long*)&func_MemReadWrite, SYM_TYPE_VMA,  "MemReadWrite"},
  {(unsigned  long*)&func_probe,        SYM_TYPE_VMA,  "probe"},
  {(unsigned  long*)&func_tgtrw,        SYM_TYPE_VMA,  "tgt_rw"},

  {(unsigned  long*)&func_Init,         SYM_TYPE_VMA,  "Init"},
  {NULL,0,NULL}
};*/
extern struct pofunc pof_elf;
int pof_read_symbol (POF_HDL h, pPOF_SYM sym)
{
#if 0
  pPOF_SYM p;
  pof_hdl hdl = pof_get_hdl(h);

  for( p=sym; p->name; ++p ) {
    SYM_HDL s = (*hdl->func->lookup_symbol) ( h, p->name );
    if( s == NULL ) continue;

    switch( p->type ) {
    case SYM_TYPE_STR:
      *p->val = (uint32_t) (*hdl->func->symbol_str) (h, s);
      break;
    case SYM_TYPE_LONG:
      *p->val = (*hdl->func->symbol_long) (h, s);
      break;
    case SYM_TYPE_VMA:
    default:
      *p->val = (*hdl->func->symbol_vma) (h, s);
      break;
    }
  }
#endif

#if 0 
  	pPOF_SYM p;
	pof_hdl hdl = pof_get_hdl(h);

	for( p = pubsym; p->name; ++p ) {

		SYM_HDL s = elf_lookup_symbol(h, p->name );
		if( s == NULL ) continue;

		*p->val = elf_symbol_vma (h, s);
	}
    return 0;
#endif

	pPOF_SYM p;
	pof_hdl hdl = pof_get_hdl(h);

	for( p=sym; p->name; ++p ) {
		SYM_HDL s = elf_lookup_symbol(h, p->name );
		if( s == NULL ) continue;

		switch( p->type ) {
		case SYM_TYPE_STR:
		  //*p->val = (uint32_t) (*hdl->func->symbol_str) (h, s);
		  break;
		case SYM_TYPE_LONG:
		  //*p->val = (*hdl->func->symbol_long) (h, s);
		  break;
		case SYM_TYPE_VMA:
		default:
		  //*p->val = (*hdl->func->symbol_vma) (h, s);
		  *p->val = elf_symbol_vma (h, s);
		  break;
		}
	}
	return 0;
}
CTX_HDL pof_context_init (POF_HDL h)
{
  pof_hdl hdl = pof_get_hdl(h);
  //CTX_HDL ctx = (*hdl->func->context_init) (h);
  CTX_HDL ctx = elf_context_init (h);

  return ctx;
}

uint32_t pof_get_next_seg (POF_HDL h, CTX_HDL c)
{
  pof_hdl hdl = pof_get_hdl(h);

  //return (*hdl->func->get_next_seg) (h, c);
  return elf_get_next_seg (h, c);
}

