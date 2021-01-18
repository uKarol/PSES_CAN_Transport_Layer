/**===================================================================================================================*\
  @file CanTp_Timer.c

  @brief Can Transport Layer Timer
  
  Implementation of Timers used in Can Transport Layer
\*====================================================================================================================*/



/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "CanTp_Timer.h"


/*====================================================================================================================*\
    Makra lokalne
\*====================================================================================================================*/

/*====================================================================================================================*\
    Typy lokalne
\*====================================================================================================================*/


/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/


/*====================================================================================================================*\
    Zmienne lokalne (statyczne)
\*====================================================================================================================*/

/*====================================================================================================================*\
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/

/*====================================================================================================================*\
    Kod globalnych funkcji inline i makr funkcyjnych
\*====================================================================================================================*/

/*====================================================================================================================*\
    Kod funkcji
\*====================================================================================================================*/

/**
  @brief CanTp_TimerStart

  Funkcja jest używana do wystartowania działania timera. 

  Parameters (in): 

  Parameters (inout): CanTp_Timer_type* - wskźnik na timer, który ma być wystartowany 

  Parameters (out): None

  Return value None
**/
void CanTp_TimerStart(CanTp_Timer_type *timer){
    
    timer->state = TIMER_ACTIVE;
}

/**
  @brief CanTp_TimerReset

  Funkcja jest używana zatrzymania i wyzerowania licznika timera. 

  Parameters (in): 

  Parameters (inout): CanTp_Timer_type* - wskźnik na timer, który ma być zresetowany

  Parameters (out): None

  Return value None
**/

void CanTp_TimerReset(CanTp_Timer_type *timer){
    timer->state = TIMER_NOT_ACTIVE;
    timer->counter = 0;
}

/**
  @brief CanTp_TimerTick

  Funkcja jest używana do inkrementacji timera. Licznik jest inkrementowany w przypadku kiedy timer jest w stanie aktywnym. 

  Parameters (in): 

  Parameters (inout): CanTp_Timer_type* - wskźnik na timer, który ma być zainkrementowany

  Parameters (out): None

  Return value: Std_returnType - Zwracana jest wartość E_NOT_OK jeżeli licznik osiągnął maksymalną wartość i nie może być inkrementowany. 
                W przeciwnym wapadku zwracana jest wartość E_OK
**/
Std_ReturnType CanTp_TimerTick(CanTp_Timer_type *timer){
    Std_ReturnType ret = E_OK;   
    if(timer->state == TIMER_ACTIVE){
        if(timer->counter < UINT32_MAX){
            timer->counter ++;
            //if(timer->counter >= timer->timeout){
            //    ret = E_NOT_OK;
            //}
        }
        else{
            ret = E_NOT_OK;
        }
    }
    return ret;
}


/**
  @brief CanTp_TimerTimeout

  Funkcja jest używana do sprawdzenia timeoutu danego timera.

  Parameters (in): 

  Parameters (inout): CanTp_Timer_type* - wskźnik na timer na którym ma być sprawdzony status timeoutu. 
  Parameters (out): None

  Return value : Std_ReturnType, Zwracana jest wartość E_OK jeżeli timer nie zwraca timeoutu. E_NOT_OK jest zwracany jeżeli licznik timera 
                przekroczył zadaną watyość timeoutu. 
**/
Std_ReturnType CanTp_TimerTimeout(CanTp_Timer_type *timer){
    if(timer->counter >= timer->timeout){
        return E_NOT_OK;
    }
    else{
        return E_OK;
    }
}

