/**@file omfy.c
 * @author 何凱帆
 * @brief omf程序目標文件分析
 * @details
 * 實現omf程序目標文件的全局符號讀取和程序段提取
*/
#include <string.h>
#include "Misc\stdafx.h"

#include "pofunc.h"

/**
 * @class omf_hdl_t
 * @author 何凱帆
 * @date 2016/10/18
 * @file omfy.c
 * @brief POF_HDL在omfy.c的實現
 */
struct omf_hdl_t {
  struct pofunc *func;
  FILE          *fp;
};
typedef struct omf_hdl_t *omf_hdl;
#define omf_get_hdl(h) (omf_hdl) (h)

static POF_HDL probe (char *name, FILE *fp)
{
  int len = strlen( name );

  if( len > 4 && !_stricmp( name+len-4, ".omf" ))
    fprintf( stderr, "Omf file but not supported yet.\r\n" );
  else
    fprintf( stderr, "Omf file is not supported yet.\r\n" );
  return NULL;
}

/*
struct pofunc pof_omf = {
  .name   = "omf",

  .probe  = probe
};
*/

struct pofunc pof_omf = {
  "omf",

  probe,
};