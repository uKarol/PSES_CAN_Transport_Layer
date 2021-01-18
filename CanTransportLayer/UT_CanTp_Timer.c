/** ==================================================================================================================*\
  @file CanTp.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"
 
#include "CanTp_Timer.c"

#include <stdio.h>



void Test_Of_CanTp_TimerReset(void){
    uint8 timeout = 10;

    CanTp_Timer_type timer = {TIMER_ACTIVE, 1, timeout};

    // Reset Active & ctr != 0 timer
    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(timer.counter == 0);
    TEST_CHECK(timer.timeout == timeout);

    // Reset inactive timer
    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(timer.counter == 0);
    TEST_CHECK(timer.timeout == timeout);
}

void Test_Of_CanTp_TimerStart(void){
    uint8 timeout = 10;
    CanTp_Timer_type timer = {TIMER_NOT_ACTIVE, 0, timeout};

    // Start  inactive & ctr == 0 timer
    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.state == TIMER_ACTIVE);
    TEST_CHECK(timer.counter == 0);
    TEST_CHECK(timer.timeout == timeout);

    // Start active timer
    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.state == TIMER_ACTIVE);
    TEST_CHECK(timer.counter == 0);
    TEST_CHECK(timer.timeout == timeout);
}


void Test_Of_CanTp_TimerTimeout(void){
    uint8 timeout = 10;
    CanTp_Timer_type timer = {TIMER_ACTIVE, 0, timeout};
    Std_ReturnType ret; 
    // 
    ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_OK);

    timer.counter = timer.timeout;
    ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_NOT_OK);

    timer.counter = timer.timeout + 1;
    ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_NOT_OK);
}

void Test_Of_CanTp_TimerTick(void){
    uint8 timeout = 10;
    CanTp_Timer_type timer = {TIMER_ACTIVE, 0, timeout};
    Std_ReturnType ret; 
    uint8 i;

    for(i = 0 ; i <= timeout + 1; i++){
      ret = CanTp_TimerTick(&timer);  
      TEST_CHECK(ret == E_OK);
      TEST_CHECK(timer.counter == i + 1);
    }

    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(timer.counter == 0);

    for(i = 0 ; i <= timeout + 1; i++){
      ret = CanTp_TimerTick(&timer);  
      TEST_CHECK(ret == E_OK);
      TEST_CHECK(timer.counter == 0);
    }

    CanTp_TimerStart(&timer);
    timer.counter = UINT32_MAX;
    ret = CanTp_TimerTick(&timer);  
    TEST_CHECK(ret == E_NOT_OK);
}


/*
  Lista testów - wpisz tutaj wszystkie funkcje które mają być wykonane jako testy.
*/
TEST_LIST = {
    { "Test_Of_CanTp_TimerReset", Test_Of_CanTp_TimerReset },   /* Format to { "nazwa testu", nazwa_funkcji } */
    { "Test_Of_CanTp_TimerStart", Test_Of_CanTp_TimerStart },
    { "Test_Of_CanTp_TimerTimeout", Test_Of_CanTp_TimerTimeout},
     { "Test_Of_CanTp_TimerTick", Test_Of_CanTp_TimerTick},
    { NULL, NULL }                                      /* To musi być na końcu */
};

