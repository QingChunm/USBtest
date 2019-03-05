#pragma once

#include <vector>
#include <map>
using namespace std;


#define _ISP_BIT_WIDTH_ 2
#define _ISP_MODE_BW_ 0x03
#define _AE_POS_          (0*_ISP_BIT_WIDTH_)
#define _BLC_POS_         (1*_ISP_BIT_WIDTH_)
#define _LSC_POS_    	  (2*_ISP_BIT_WIDTH_)
#define _DDC_POS_         (3*_ISP_BIT_WIDTH_)
#define _AWB_POS_         (4*_ISP_BIT_WIDTH_)
#define _CCM_POS_         (5*_ISP_BIT_WIDTH_)
#define _DGAIN_POS_       (6*_ISP_BIT_WIDTH_)
#define _YGAMA_POS_       (7*_ISP_BIT_WIDTH_)
#define _RGB_GAMA_POS_    (8*_ISP_BIT_WIDTH_)
#define _CH_POS_          (9*_ISP_BIT_WIDTH_)
#define _VDE_POS_         (10*_ISP_BIT_WIDTH_)
#define _EE_POS_          (11*_ISP_BIT_WIDTH_)
#define _CFD_POS_         (12*_ISP_BIT_WIDTH_)
#define _SAJ_POS_         (13*_ISP_BIT_WIDTH_)
#define _OL_DB_POS        (15*_ISP_BIT_WIDTH_)

struct Blc
{
    int blkl_r;
    int blkl_gr;
    int blkl_gb;
    int blkl_b;
};

struct Awb
{
    int seg_mode;
    int rg_start;
    int rgmin;
    int rgmax;
    int weight_in;
    int weight_mid;
    int ymin;
    int ymax;
    int hb_rate;
    int hb_class;
    int hr_rate;
    int hr_class;
    int awb_scene_mod; //NotUse
    int manu_awb_gain[5];//= new int[5]; // NotUse
    int yuv_mod_en;
    int cb_th[8]; //= new int[8];
    int cr_th[8]; //= new int[8];
    int cbcr_th[8]; //= new int[8];
    int ycbcr_th;
    int manu_rgain;
    int manu_ggain;
    int manu_bgain;
    int rgain;
    int ggain;
    int bgain;
    short seg_gain[8][3];
    unsigned char awb_tab[128]; //= new byte[128];
};

struct Gamma
{
    int br_mod;
    int gma_num[8]; //= new int[8];
    int contra_num;
    int bofst;
    int lofst;
    int lcpr_low;
    int lcpr_high;
    int lcpr_llimt;
    int lcpr_hlimt;
    int pad_num;
    short using_ygama[512];//= new short[512];
};

typedef	struct
{
	int ylog_cal_fnum;
	int exp_tag[8];
	int exp_ext_mod;
	int exp_th;
	int gain_th;
	int exp_gain;
	int k_br;
	int exp_min;
	int gain_max;
	int isp_gain[8];
	int exp_nums;
}EXP;

typedef	struct
{
	int allow_miss_dots;
	int ae_win_x0;
	int ae_win_x1;
	int ae_win_x2;
	int ae_win_x3;
	int ae_win_y0;
	int ae_win_y1;
	int ae_win_y2;
	int ae_win_y3;
	int weight_0_7;
	int weight_8_15;
	int weight_16_23;
	int weight_24;
	int hgrm_centre_weight[8];
	int hgrm_gray_weight[8];
}HGRM;

typedef	struct
{
	EXP exp_adapt;
	HGRM hgrm_adapt;
}AE;

typedef	struct
{
	int contra;
	int bright_k; // 80 -> 1 gain
	int bright_oft; // bright_oft * bright_K 
	int hue;
	int sat[9];
	int sat_rate[8];
	int vde_step;
}VDE;

typedef	struct
{

	int hot_num;
	int dead_num;
	int hot_th;
	int dead_th;
	int avg_th;
	int d_th_rate[8];
	int h_th_rate[8];
	int dpc_dn_en;
	int indx_table[8];
	int indx_adapt[8];
	int std_th[7];
	int std_th_rate;
	int ddc_step;
}DDC;


typedef	struct
{
	char ee_class;
	char ee_dn_slope[8];
	char ee_sharp_slope[8];
	char ee_th_adp[8];
	char ee_dn_th[8];
	char sharp_class[8];
	char dn_class[8];
}EE;

typedef	struct
{
	char sat[17];
	char sat_rate[8];
	char saj_en;
}SAJ;

typedef	struct
{
	short ccm[9];
	short s41;
	short s42;
	short s43;
}CCM;


typedef	struct
{
	int stage0_en;//enable r g b
	int stage1_en;//enable y c m
	int enhence[6];//enhance channel  r b g y c m
	int th1[6];//you can set hue width
	int th0[6];
	//m_x r_x y_x b_x g_r r_x
	int r_rate[6];//combining with sat[],you can enhance or weaken
	int g_rate[6];
	int b_rate[6];
	int sat[17];//16为1
	int rate[8];
}CH;

typedef struct {
    int pclk;
    int v_len;
    int step_val;
    int step_max;
    int down_fps_mode;//0,1,hvb down_fps; 2: exp down_fps,0xff: turn off down_fps
    char fps;
    char frequency;
} Hvb_Adapt;

typedef struct
{
	int exp_gain;
	int gain_max;
	int id;
	int mclk;
	Hvb_Adapt hvb;
	short pixelw;
	short pixelh;
	char type;//CSI_TYPE_RAW10,CSI_TYPE_RAW8
	char hsyn;
	char vsyn;
	char rduline;
	char colrarray;//0:_RGRG_ 1:_GRGR_,2:_BGBG_,3:_GBGB_
	char pclk_fir_en;
	char pclk_inv_en;
	char csi_tun;
	char name[20];//最后一个字母是 ^
	char exp_gain_en;
	char blk_en;
	char lsc_en;
	char ddc_en;
	char awb_en;
	char ccm_en;
	char dgain_en;
	char ygama_en;
	char rgb_gama_en;
	char ch_en;
	char vde_en;
	char ee_en;
	char cfd_en;
	char saj_en;
	char pclk_fir_class;
	char AVDD;
	char DVDD;
	char VDDIO;
	char rotate;
}COMMON;

struct lcd_lsawtooth_t 
{
	int anti_lsawtooth[3][24];//0: all lcd  1:half lcd 2:small window
};

struct lcd_common_t{
	char name[32];
	short screen_w;
	short screen_h;
};

struct lcd_vde_t{
	int contrast;
	int brightness;
	int saturation;
};

struct lcd_gamma_t{
	int contra_index;
	int gamma_red;
	int gamma_green;
	int gamma_blue;
};


struct usb_lcddev_t{
	lcd_common_t lcd_common;
	lcd_vde_t lcd_vde;
	lcd_gamma_t lcd_gamma;
	int de_ccm[12];
	int de_saj[5];
	lcd_lsawtooth_t  lcd_lsawtooth;
};


enum ISP_order{
	INDEX_BLC   =0,
	INDEX_CFA     ,
	INDEX_DDC     ,
	INDEX_AWB     ,
	//INDEX_GAIN    ,
	INDEX_CCM     ,
	INDEX_YGAMA   ,
	INDEX_RGBGAMA ,
	INDEX_EE      ,
	INDEX_CCF     ,
	INDEX_LSC     ,
	INDEX_CBHS    ,
	INDEX_MD      ,
	INDEX_EXP     ,
	INDEX_HGRM    ,
	MODULE_NUM,
};