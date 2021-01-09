/** ==================================================================================================================*\
  @file CanTp.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"   /* To nie pomyłka - taki include eksponuje zmienne statyczne dla testów */

/**
  @brief Test dodawania

  Funkcja testująca dodawanie w bibliotece. Funkcje testowe acutest nie mogą zwracać ani przyjmować danych!
*/
void Test_Of_Lib_Calc_Add(void)
{
    sint32 result;
    Std_ReturnType retv;

    uint8 can_data[] = "12345678";
    CanFrameInfo_Type CanFrameInfo;

    retv = CanTp_FrameCheckType(can_data, &CanFrameInfo);

    TEST_CHECK(retv == E_OK);

}

/*
  Lista testów - wpisz tutaj wszystkie funkcje które mają być wykonane jako testy.
*/
TEST_LIST = {
    { "Test of Lib_Calc_Add", Test_Of_Lib_Calc_Add },   /* Format to { "nazwa testu", nazwa_funkcji } */
   // { "Test of Lib_Calc_Sub", Test_Of_Lib_Calc_Sub },
    { NULL, NULL }                                      /* To musi być na końcu */
};

