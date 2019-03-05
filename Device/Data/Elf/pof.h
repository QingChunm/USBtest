/**@file pof.h
 * @author 何凱帆
 * @brief 程序目標文件分析
 * @details 實現程序目標文件的全局符號讀取和程序段提取
*/

#ifndef _POF_H_
#define _POF_H_
extern "C" {
//#include <stdint.h>


typedef unsigned   uint32_t;

typedef struct POF_SYM
{
	uint32_t *val;
	uint32_t  type;
	char     *name;
} POF_SYM, *pPOF_SYM;
#define SYM_TYPE_VMA  0
#define SYM_TYPE_LONG 1
#define SYM_TYPE_STR  2

#ifndef POF_HDL
# define POF_HDL void *
#endif

typedef struct CTX_HDL_t {
  uint32_t addr;
  char     *buf;
} *CTX_HDL;

//POF_HDL __cdecl pof_open_file (char *name, FILE *fp);
POF_HDL __cdecl pof_open_file (char *name);
void    __cdecl pof_close     (POF_HDL);

int __cdecl pof_read_symbol (POF_HDL, pPOF_SYM);

CTX_HDL __cdecl pof_context_init (POF_HDL);
#define pof_context_free(ctx) free(ctx);
uint32_t __cdecl pof_get_next_seg (POF_HDL, CTX_HDL);

}
#endif  /* _POF_H_ */
