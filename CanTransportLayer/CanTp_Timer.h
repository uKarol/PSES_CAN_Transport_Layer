
#ifndef CAN_TP_TIMER_H
#define CAN_TP_TIMER_H

/**===================================================================================================================*\
  @file CanTp.h

  @brief Can Transport Layer Timer
  
  Implementation of Timers used in Can Transport Layer

\*====================================================================================================================*/

/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "Std_Types.h"
/*====================================================================================================================*\
    Makra globalne
\*====================================================================================================================*/
//Wartości timoutów timerów 
#define N_AR_TIMEOUT_VAL 100
#define N_BR_TIMEOUT_VAL 100
#define N_CR_TIMEOUT_VAL 100

#define N_AS_TIMEOUT_VAL 100
#define N_BS_TIMEOUT_VAL 100
#define N_CS_TIMEOUT_VAL 100

/*====================================================================================================================*\
    Typy globalne
\*====================================================================================================================*/
/*
    \enum timer_state_t
    \brief Stan timera
    Typ wyliczeniowy zawierający dostępne stany timera
*/
typedef enum{
    TIMER_ACTIVE,
    TIMER_NOT_ACTIVE
} timer_state_t;

/*
    \enum CanTp_Timer_type
    \brief Struktura timera
    Struktura timera zawierająca jego stan, aktualną wartość licznika oraz wartość timeoutu 
*/
typedef struct{
    timer_state_t state;
    uint32        counter; 
    const uint32   timeout; 
} CanTp_Timer_type;


/*====================================================================================================================*\
    Deklaracje funkcji globalnych
\*====================================================================================================================*/
void CanTp_TimerStart(CanTp_Timer_type *timer);
void CanTp_TimerReset(CanTp_Timer_type *timer);
Std_ReturnType CanTp_TimerTick(CanTp_Timer_type *timer);
Std_ReturnType CanTp_TimerTimeout(const CanTp_Timer_type *timer);



/*====================================================================================================================*\
    Kod globalnych funkcji inline i makr funkcyjnych
\*====================================================================================================================*/

#endif /*CAN_TP_TIMER_H*/