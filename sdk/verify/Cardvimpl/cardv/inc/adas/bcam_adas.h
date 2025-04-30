/*
 * bcam_adas.h
 *
 *  Created on: 2015/3/16
 *      Author: chris
 */

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

#ifndef BCCM_ADAS_H_
#define BCCM_ADAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ldws.h"
#include "fcws.h"
#include "SaG.h"
#include "adas.h"

extern int BCam_ADAS_get_buffer_info(unsigned short width, unsigned short height);
extern ADAS_init_error_type BCam_ADAS_init(unsigned short width, unsigned short height, unsigned char* working_buf_ptr, int working_buf_len, ADAS_init_params* params);
extern ADAS_error_type BCam_ADAS_process(const unsigned char *src, ADAS_set_info* params);
extern int BCam_ADAS_enable(ADAS_enable_params* ADAS_en);
extern int BCam_ADAS_get_result(ADAS_results* result);
extern int BCam_ADAS_set_params(ADAS_process_params* params);
extern int BCam_ADAS_wait_process_done(void);
extern int BCam_ADAS_abort_process(void);
extern unsigned int BCam_ADAS_GetLibVersion(unsigned int* ver);

extern void BCam_ADAS_set_calibration(int calibration_status);
extern void BCam_ADAS_set_symmetry(int symmetry_status);

#ifdef __cplusplus
}
#endif

#endif /* BCCM_ADAS_H_ */
