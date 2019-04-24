#ifndef _GSM_322_H_
#define _GSM_322_H_

/* List of state machines for event notification towards UI */
#define GSM322_STATE_MACHINE_PLMN_AUTOMATIC     0
#define GSM322_STATE_MACHINE_PLMN_MANUAL        1
#define GSM322_STATE_MACHINE_CELL_SELECTION     2

/* 4.3.1.1 List of states for PLMN slection process (automatic mode) */
#define GSM322_A0_NULL          0
#define GSM322_A1_TRYING_RPLMN      1
#define GSM322_A2_ON_PLMN       2
#define GSM322_A3_TRYING_PLMN       3
#define GSM322_A4_WAIT_FOR_PLMN     4
#define GSM322_A5_HPLMN_SEARCH      5
#define GSM322_A6_NO_SIM        6

/* 4.3.1.2 List of states for PLMN slection process (manual mode) */
#define GSM322_M0_NULL          0
#define GSM322_M1_TRYING_RPLMN      1
#define GSM322_M2_ON_PLMN       2
#define GSM322_M3_NOT_ON_PLMN       3
#define GSM322_M4_TRYING_PLMN       4
#define GSM322_M5_NO_SIM        5

/* 4.3.2 List of states for cell selection process */
#define GSM322_C0_NULL          0
#define GSM322_C1_NORMAL_CELL_SEL   1
#define GSM322_C2_STORED_CELL_SEL   2
#define GSM322_C3_CAMPED_NORMALLY   3
#define GSM322_C4_NORMAL_CELL_RESEL 4
#define GSM322_C5_CHOOSE_CELL       5
#define GSM322_C6_ANY_CELL_SEL      6
#define GSM322_C7_CAMPED_ANY_CELL   7
#define GSM322_C8_ANY_CELL_RESEL    8
#define GSM322_C9_CHOOSE_ANY_CELL   9
#define GSM322_CONNECTED_MODE_1     10
#define GSM322_CONNECTED_MODE_2     11
#define GSM322_PLMN_SEARCH      12
#define GSM322_HPLMN_SEARCH     13
#define GSM322_ANY_SEARCH       14

#endif
