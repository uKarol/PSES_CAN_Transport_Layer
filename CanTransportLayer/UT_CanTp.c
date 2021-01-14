/** ==================================================================================================================*\
  @file CanTp.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"  
#include <stdio.h>

#include "fff.h"

DEFINE_FFF_GLOBALS; 

FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType*);
 
PduLengthType *buffSize_array;//[10] = {1,2,3,4,5,6,7,8,9,0};

BufReq_ReturnType PduR_CanTpStartOfReception_FF (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr ){
    static int i = 0;
    i = PduR_CanTpStartOfReception_fake.call_count - 1;
    *bufferSizePtr = buffSize_array[i];
   return PduR_CanTpStartOfReception_fake.return_val_seq[i];
}

void Test_Of_PduR_CanTpStartOfReception(void){

    PduIdType id;
    PduInfoType info; 
    PduLengthType TpSduLength;
    PduLengthType bufferSizePtr;

    BufReq_ReturnType retv;
    
    PduLengthType buffSize_array_local[10] = {1,2,3,4,5,6,7,8,9,0};

    buffSize_array = buffSize_array_local;

    
    BufReq_ReturnType BufferReturnVals[4] = { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OVFL };
    SET_RETURN_SEQ(PduR_CanTpStartOfReception, BufferReturnVals, 4);
    PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;
    

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 1);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 1);
    TEST_CHECK(retv == BUFREQ_OK);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 2);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 2);
    TEST_CHECK(retv == BUFREQ_E_NOT_OK);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 3);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 3);
    TEST_CHECK(retv == BUFREQ_BUSY);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 4);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 4);
    TEST_CHECK(retv == BUFREQ_OVFL);

    RESET_FAKE(PduR_CanTpStartOfReception);
    SET_RETURN_SEQ(PduR_CanTpStartOfReception, BufferReturnVals, 4);
    PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 1);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 1);
    TEST_CHECK(retv == BUFREQ_OK);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 2);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 2);
    TEST_CHECK(retv == BUFREQ_E_NOT_OK);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 3);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 3);
    TEST_CHECK(retv == BUFREQ_BUSY);

    retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
    TEST_CHECK(bufferSizePtr == 4);
    TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 4);
    TEST_CHECK(retv == BUFREQ_OVFL);

}


#include "stdio.h"
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
  static const uint32_t ff_short_frame_lenght[] = {0x01, 0xFFF, 0xFFE, 0x111};
  static const uint32_t ff_long_frame_lenght[] = {0x00001000, 0xFFFFFFFF, 0xFFFFFFFE, 0x11111111};

  CanPDU.SduDataPtr = sdu_data;


  //Signle frame, lenght = 7
  CanPCI.frame_type = SF;
  CanPCI.frame_lenght = sizeof(Can_payload_example);
  memcpy(Can_payload, Can_payload_example, sizeof(Can_payload_example));
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (SF_ID << 4) | sizeof(Can_payload_example));
  TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr + 1, CanPCI.frame_lenght) == 0);

  //Signle frame, lenght = short
  CanPCI.frame_type = SF;
  CanPCI.frame_lenght = sizeof(Can_payload_example_short);
  memcpy(Can_payload, Can_payload_example_short, sizeof(Can_payload_example_short));
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (SF_ID << 4) | sizeof(Can_payload_example_short));
  TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr+1, CanPCI.frame_lenght) == 0);

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
  TEST_CHECK(CanPDU.SduDataPtr[0] == (CF_ID << 4) | CanPCI.SN);

  //Consecutive frame, SN < 8; 
  CanPCI.frame_type = CF;
  CanPCI.SN = 1;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (CF_ID << 4) | CanPCI.SN);

  //Consecutive frame, SN  >= 8; 
  CanPCI.frame_type = CF;
  CanPCI.SN = 0x10;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);

  //Flow control, FS  < 7
  CanPCI.frame_type = FC;
  CanPCI.FS = 0x01;
  CanPCI.BS = 0xF1;
  CanPCI.ST = 0x1F;

  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_OK);
  TEST_CHECK(CanPDU.SduDataPtr[0] == (FC_ID << 4) | CanPCI.FS);
  TEST_CHECK(CanPDU.SduDataPtr[1] == CanPCI.BS);
  TEST_CHECK(CanPDU.SduDataPtr[2] = CanPCI.ST);

  //Flow control, FS  >= 8
  CanPCI.frame_type = FC;
  CanPCI.FS = 0x10;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
  TEST_CHECK(retv == E_NOT_OK);



  //FF, DL <= 4095
  for(uint8_t i = 0; i < 4; i++){
    CanPCI.frame_type = FF;
    CanPCI.frame_lenght = ff_short_frame_lenght[i];
    memcpy(Can_payload, Can_payload_example, sizeof(Can_payload_example));
    retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(((CanPDU.SduDataPtr[0] >> 4) & 0x0F) == FF_ID);            //frame type
    TEST_CHECK((CanPDU.SduDataPtr[0] & 0x0F) == ((CanPCI.frame_lenght >> 8) & 0x0F)); //4 highest bits of frame lenght
    TEST_CHECK(CanPDU.SduDataPtr[1] == (CanPCI.frame_lenght & 0xFF));     //8 first bits of frame lenght
    TEST_CHECK(CanPDU.SduDataPtr[1] == (CanPCI.frame_lenght & 0xFF));
    TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr+2, 6) == 0);
  }

  //FF, DL > 4095
  for(uint8_t i = 0; i < 4; i++){
    CanPCI.frame_type = FF;
    CanPCI.frame_lenght = ff_long_frame_lenght[i];
    memcpy(Can_payload, Can_payload_example, sizeof(Can_payload_example));
    retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, Can_payload);
    TEST_CHECK(retv == E_OK);
    TEST_CHECK(((CanPDU.SduDataPtr[0] >> 4) & 0x0F) == FF_ID);            //frame type
    TEST_CHECK((CanPDU.SduDataPtr[0] & 0x0F) == 0 ); //4 highest bits of frame lenght
    TEST_CHECK(CanPDU.SduDataPtr[1] == 0);     //8 first bits of frame lenght
    TEST_CHECK(CanPDU.SduDataPtr[2] == ((CanPCI.frame_lenght >> 24) & 0xFF));
    TEST_CHECK(CanPDU.SduDataPtr[3] == ((CanPCI.frame_lenght >> 16) & 0xFF));
    TEST_CHECK(CanPDU.SduDataPtr[4] == ((CanPCI.frame_lenght >> 8) & 0xFF));
    TEST_CHECK(CanPDU.SduDataPtr[5] == ((CanPCI.frame_lenght >> 0) & 0xFF));
    TEST_CHECK(memcmp(Can_payload_example, CanPDU.SduDataPtr+6, 2) == 0);
  }


  //Uknown frame:
  CanPCI.frame_type = UNKNOWN;
  retv = CanTp_PrepareSegmenetedFrame(&CanPCI, &CanPDU, NULL);
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
    //{ "Test_Of_PduR_CanTpStartOfReception" , Test_Of_PduR_CanTpStartOfReception},
   // { "Test of Lib_Calc_Sub", Test_Of_Lib_Calc_Sub },
    { NULL, NULL }                                      /* To musi być na końcu */
};

