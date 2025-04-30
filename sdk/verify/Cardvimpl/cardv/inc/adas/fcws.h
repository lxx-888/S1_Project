
/* -- DO NOT MODIFY ---- DO NOT MODIFY ----------- O NOT MODIFY ---------- DO NOT MODIFY DIFY -------------- */
/* -- DO NOT MODIFY ----- DO NOT MODIFY --------- O NOT MODIFY ---------- DO NOT MODIFY MODIFY ------------- */
/* -- DO NOT MODIFY ------ DO NOT MODIFY ------- O NOT MODIFY ---------- DO NOT MODIFY T MODIFY ------------ */
/* -- DO NOT MODIFY ------- DO NOT MODIFY ----- O NOT MODIFY ---------- DO NOT MODIFY NOT MODIFY ----------- */
/* -- DO NOT MODIFY -------- DO NOT MODIFY --- O NOT MODIFY ---------- DO NOT MODIFY O NOT MODIFY ---------- */
/* -- DO NOT MODIFY --------- DO NOT MODIFY - O NOT MODIFY ---------- DO NOT MODIFY - DO NOT MODIFY -------- */
/* -- DO NOT MODIFY ---------- DO NOT MODIFY O NOT MODIFY ---------- DO NOT MODIFY --- DO NOT MODIFY ------- */
/* -- DO NOT MODIFY ----------- DO NOT MODIFY NOT MODIFY ---------- DO NOT MODIFY ----- DO NOT MODIFY ------ */
/* -- DO NOT MODIFY ------------ DO NOT MODIFY T MODIFY ---------- DO NOT MODIFY --DONT- DO NOT MODIFY ----- */
/* -- DO NOT MODIFY ------------- DO NOT MODIFY MODIFY ---------- DO NOT MODIFY --------- DO NOT MODIFY ---- */
/* -- DO NOT MODIFY -------------- DO NOT MODIFY DIFY ---------- DO NOT MODIFY ----------- DO NOT MODIFY --- */
/* -- DO NOT MODIFY --------------- DO NOT MODIFY ------------- DO NOT MODIFY ------------- DO NOT MODIFY -- */

#ifndef FCWS_H
#define FCWS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum fcws_err_e
{
	FCWS_ERR_NONE = 0x200,
	FCWS_ERR_NULL_PARAMETER,
	FCWS_ERR_NOT_READY,
	FCWS_ERR_NOT_INIT,
	FCWS_ERR_NOT_SUPPROT,
	FCWS_ERR_INVALID_PARAMETER,
	FCWS_ERR_OUT_OF_MEMORY,
	FCWS_ERR_IC_CHECK,
	FCWS_ERR_MVE_INIT_FAIL
}fcws_err;

typedef enum _fcws_state_e
{
	FCWS_STATE_LOSE = 0,
	FCWS_STATE_FIND
}fcws_state_e;

typedef struct _fcws_info_t {
	fcws_state_e        state;
	int car_x;
	int car_y;
	int car_width;
	int car_height;
	int distance;
} fcws_info_t;

typedef struct _fcws_debug_info_t
{
	int		iCarVld;
	int		iCarDist;
	int 	iMyCarX;
	int 	iMyCarY;
	int 	iMyCarW;
	int 	iMyCarLife;
	int		iLTX;
	int		iLBX;
	int 	iRTX;
	int		iRBX;
	int 	iTY;
	int		iBY;
} fcws_debug_info;

extern unsigned int fcws_GetLibVersion(unsigned int* ver);

#ifdef __cplusplus
}
#endif

#endif
