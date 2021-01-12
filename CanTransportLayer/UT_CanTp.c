/** ==================================================================================================================*\
  @file CanTp.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"  
#include <stdio.h>

/**
  @brief Test dodawania

  Funkcja testująca dodawanie w bibliotece. Funkcje testowe acutest nie mogą zwracać ani przyjmować danych!
*/
void Test_Of_CanTp_FrameCheckType(void)
{
    sint32 result;
    Std_ReturnType retv;

    uint8 sdu_data[8];
    CanPCI_Type CanFrameInfo;

    PduInfoType can_data;
    can_data.SduDataPtr = sdu_data;
    can_data.SduLength = 8;

    // single frame test
    can_data.SduDataPtr[0] = 0x0F; 
    can_data.SduDataPtr[1] = 0;
    can_data.SduDataPtr[2] = 0;

    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == SF);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0xF);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.ST == 0);

    // FirstFrame short
    can_data.SduDataPtr[0] = 0x1F; 
    can_data.SduDataPtr[1] = 0xF;
    can_data.SduDataPtr[2] = 0;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == FF);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0xF0F);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.ST == 0);

    // FirstFrame long
    can_data.SduDataPtr[0] = 0x10; 
    can_data.SduDataPtr[1] = 0x0;
    can_data.SduDataPtr[2] = 0x11;
    can_data.SduDataPtr[3] = 0x22;
    can_data.SduDataPtr[4] = 0x33;
    can_data.SduDataPtr[5] = 0x44;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == FF);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0x11223344);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.ST == 0);

    // Consecutive Frame
    can_data.SduDataPtr[0] = 0x2F; 
    can_data.SduDataPtr[1] = 0x0;
    can_data.SduDataPtr[2] = 0;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == CF);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0x0);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.SN == 0xF);
    TEST_CHECK(CanFrameInfo.ST == 0);

    // Flow Control Frame
    can_data.SduDataPtr[0] = 0x31; 
    can_data.SduDataPtr[1] = 0x2;
    can_data.SduDataPtr[2] = 0x3;
    can_data.SduDataPtr[3] = 0x3;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == FC);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0x0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.FS == 1);
    TEST_CHECK(CanFrameInfo.BS == 2);
    TEST_CHECK(CanFrameInfo.ST == 3);

    // invalid frame
    can_data.SduDataPtr[0] = 0xF1; 
    can_data.SduDataPtr[1] = 0x2;
    can_data.SduDataPtr[2] = 0x3;
    can_data.SduDataPtr[3] = 0x3;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_NOT_OK);
    TEST_CHECK(CanFrameInfo.frame_type == UNKNOWN);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0x0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.ST == 0);

    // pduinfotype as NULL_PTR
    retv = CanTp_GetPCI(NULL, &CanFrameInfo);
    TEST_CHECK(retv == E_NOT_OK);

    // CanPCI_info as NULL_PTR
    retv = CanTp_GetPCI(&can_data, NULL);
    TEST_CHECK(retv == E_NOT_OK);

    // both null pointers
    retv = CanTp_GetPCI(NULL, NULL);
    TEST_CHECK(retv == E_NOT_OK);

    // payload is NULL ptr
    can_data.SduDataPtr = NULL;
    retv = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(retv == E_NOT_OK);
}


void Test_Of_CanTp_PrepareSegmenetedFrame(void)
{
  Std_ReturnType retv;
  CanPCI_Type CanPCI;
  PduInfoType CanPDU;
  
  uint8_t Can_payload[7];
  uint8_t sdu_data[8];

  static const uint8_t Can_payload_example[] = {0x00, 0xFF, 0x01, 0xFE, 0x02, 0x03, 0x04};
  static const uint8_t Can_payload_example_short[] = {0x00, 0xFF};


  CanPDU.SduDataPtr = sdu_data;


  //Signle frame, lenght = 7
  CanPCI.frame_type = SF;
  CanPCI.frame_lenght;
  memcpy(Can_payload, Can_payload_example, sizeof(Can_payload_example));
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (SF_ID << 4) | sizeof(Can_payload_example));
  TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr+1, sizeof(Can_payload_example)));

  //Signle frame, lenght = short
  CanPCI.frame_type = SF;
  CanPCI.frame_lenght;
  memcpy(Can_payload, Can_payload_example_short, sizeof(Can_payload_example_short));
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (SF_ID << 4) | sizeof(Can_payload_example_short));
  TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr+1, sizeof(Can_payload_example_short)));

  //Signle frame, lenght >= 8
  CanPCI.frame_type = SF;
  CanPCI.frame_lenght = 8;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] << 4 == (SF_ID << 4));

  //Consecutive frame, SN = 0; 
  CanPCI.frame_type = CF;
  CanPCI.SN = 0;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (CF_ID << 4) | CanPCI.SN));

  //Consecutive frame, SN != 0; 
  CanPCI.frame_type = CF;
  CanPCI.SN = 1;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (CF_ID << 4) | CanPCI.SN));

  //FirstFrame
  CanPCI.frame_type = FF;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);
  
  //FlowControl
  CanPCI.frame_type = FC;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);

  //Payload is NULL:
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, NULL);
  TEST_CHECK(retv == E_NOT_OK);

  //PCI is NULL:
  retv = CanTp_PrepareSegmenetedFrame(NULL, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);

  //PDU is NULL:
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, NULL, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);
}

/*
  Lista testów - wpisz tutaj wszystkie funkcje które mają być wykonane jako testy.
*/
TEST_LIST = {
    //{ "Test_Of_CanTp_FrameCheckType", Test_Of_CanTp_FrameCheckType },   /* Format to { "nazwa testu", nazwa_funkcji } */
    { "Test_Of_CanTp_PrepareSegmenetedFrame", Test_Of_CanTp_PrepareSegmenetedFrame },
   // { "Test of Lib_Calc_Sub", Test_Of_Lib_Calc_Sub },
    { NULL, NULL }                                      /* To musi być na końcu */
};

