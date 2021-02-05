/** ==================================================================================================================*\
  @file CanTp.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"  

#include <stdio.h>

#include <string.h>

#include "fff.h"

DEFINE_FFF_GLOBALS; 

// fake functions from PDU Router

FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyRxData, PduIdType, const PduInfoType*, PduLengthType*);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyTxData, PduIdType, const PduInfoType*, const RetryInfoType*, PduLengthType*);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType* );
FAKE_VOID_FUNC(PduR_CanTpRxIndication, PduIdType, Std_ReturnType );
FAKE_VOID_FUNC(PduR_CanTpTxConfirmation, PduIdType, Std_ReturnType );

// fake functions from Can Interface

FAKE_VALUE_FUNC( Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType* );

// custom fake functions
PduLengthType* PduR_CanTpCopyRxData_buffSize_array;

BufReq_ReturnType PduR_CanTpCopyRxData_FF ( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr ){
    static int i = 0;
    i = PduR_CanTpCopyRxData_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpCopyRxData_buffSize_array[i];
    return PduR_CanTpCopyRxData_fake.return_val_seq[i];
}

uint8 PduR_CanTpCopyTxData_sdu_data[20][7];
PduLengthType *PduR_CanTpCopyTxData_availableDataPtr; 

BufReq_ReturnType PduR_CanTpCopyTxData_FF ( PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr ){
    static int i = 0;
    int loop_ctr;
    i = PduR_CanTpCopyTxData_fake.call_count - 1;
    for( loop_ctr = 0; loop_ctr < info->SduLength; loop_ctr++ ){
      info->SduDataPtr[loop_ctr] = PduR_CanTpCopyTxData_sdu_data[i][loop_ctr];
    }
    *availableDataPtr = PduR_CanTpCopyTxData_availableDataPtr[i];
    return PduR_CanTpCopyTxData_fake.return_val_seq[i];
}

PduLengthType *PduR_CanTpStartOfReception_buffSize_array;

BufReq_ReturnType PduR_CanTpStartOfReception_FF (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr ){
    static int i = 0;
    i = PduR_CanTpStartOfReception_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpStartOfReception_buffSize_array[i];
   return PduR_CanTpStartOfReception_fake.return_val_seq[i];
}

// end of fake functions

// start of unit tests


// tests of custom FF
void Test_Of_PduR_CanTpCopyTxData_FF(void){
  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[3][7] = { "marszal", "ek prog", "ramuje " };
   BufReq_ReturnType BufferReturnVals[10] = { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY};

  uint8_t sdu[8];

  PduInfoType info;
  PduIdType id = 0;
  PduLengthType availableData;
  info.SduDataPtr = sdu;
  info.MetaDataPtr = NULL;
  info.SduLength = 7; 
  int i;

  for( i = 0; i < 3; i++){
   memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, BufferReturnVals, 3 );

  PduR_CanTpCopyTxData(id, &info, NULL, &availableData);
  TEST_CHECK(availableData == 1);
 
  for( i = 0; i<7; i++){
    TEST_CHECK(info.SduDataPtr[i] == sdu_data_ptr_array[0][i]);
  }

  PduR_CanTpCopyTxData(id, &info, NULL, &availableData);
  TEST_CHECK(availableData == 2);
 
  for( i = 0; i<7; i++){
    TEST_CHECK(info.SduDataPtr[i] == sdu_data_ptr_array[1][i]);
  }

  PduR_CanTpCopyTxData(id, &info, NULL, &availableData);
  TEST_CHECK(availableData == 3);
 
  for( i = 0; i<7; i++){
    TEST_CHECK(info.SduDataPtr[i] == sdu_data_ptr_array[2][i]);
  }

}

void Test_Of_Custom_FFs(void){

    PduIdType id;
    PduInfoType info; 
    PduLengthType TpSduLength;
    PduLengthType bufferSizePtr;

    BufReq_ReturnType retv;

    int i;
    
    PduLengthType buffSize_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
    PduR_CanTpStartOfReception_buffSize_array = buffSize_array_local;
    PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    
    BufReq_ReturnType BufferReturnVals[10] = { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OVFL, BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OVFL, BUFREQ_OK, BUFREQ_OK };
    SET_RETURN_SEQ(PduR_CanTpStartOfReception, BufferReturnVals, 10);
    PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;

  
    SET_RETURN_SEQ(PduR_CanTpCopyRxData, BufferReturnVals, 10);
    PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;
    
    for( i = 0; i<10; i++){
      retv = PduR_CanTpStartOfReception (id, &info, TpSduLength, &bufferSizePtr );
      TEST_CHECK(bufferSizePtr == buffSize_array_local[i]);
      TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == i+1);
      TEST_CHECK(retv == BufferReturnVals[i] );
    }

    for( i = 0; i<10; i++){
      retv = PduR_CanTpCopyRxData (id, &info, &bufferSizePtr );
      TEST_CHECK(bufferSizePtr == buffSize_array_local[i]);
      TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == i+1);
      TEST_CHECK(retv == BufferReturnVals[i] );
    }




}
void Test_Of_CanTp_Calculate_Available_Blocks(void){

  uint16 retval;

  CanTp_StateVariables.message_length = 8;
  CanTp_StateVariables.sended_bytes = 0;

  retval = CanTp_Calculate_Available_Blocks(18);
  TEST_CHECK(retval == 2);
  
  CanTp_StateVariables.message_length = 14;
  CanTp_StateVariables.sended_bytes = 0;

  retval = CanTp_Calculate_Available_Blocks(18);
  TEST_CHECK(retval == 2);

  CanTp_StateVariables.message_length = 13;
  CanTp_StateVariables.sended_bytes = 0;

  retval = CanTp_Calculate_Available_Blocks(10);
  TEST_CHECK(retval == 1);
}
// tests of static CanTp functions

void Test_Of_CanTp_set_blocks_to_next_cts( void ){

  CanTp_set_blocks_to_next_cts( 100 );
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts = 100);
}

void Test_Of_CanTp_Resume(void){
    
    CanTp_Resume();
    CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING;

}

void Test_Of_CanTp_Reset_Rx_State_Variables(){     

    CanTp_Reset_Rx_State_Variables();
    TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
    TEST_CHECK(CanTp_StateVariables.message_length == 0);
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);

}

void Test_Of_CanTp_SendFlowControl(){

    PduIdType ID = 1;
    uint8 BS = 1;
    Std_ReturnType ret;

    RESET_FAKE(CanIf_Transmit);
    RESET_FAKE(PduR_CanTpRxIndication);
    
    Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_NOT_OK, E_OK, E_OK};
    SET_RETURN_SEQ( CanIf_Transmit, RetVal, 10);

    // reset all timers
    CanTp_TimerReset(&N_Cr_timer);
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);


    /*
      TEST CASE 1
      WYSYLAMY CTS, ZAKLADAMY ZE WSZYSTKO JEST OK
    */

    ret = CanTp_SendFlowControl( ID, BS, FC_CTS, DEFAULT_ST );

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);

    /*
      TEST CASE 2
      WYSYLAMY WAIT, ZAKLADAMY ZE WSZYSTKO JEST OK
    */
        // reset all timers
    CanTp_TimerReset(&N_Cr_timer);
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);

    ret = CanTp_SendFlowControl( ID, BS, FC_WAIT, DEFAULT_ST );

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(CanIf_Transmit_fake.call_count == 2);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_ACTIVE);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);

    /*
      TEST CASE 3
      WYSYLAMY WAIT, ZAKLADAMY ZE WSZYSTKO JEST OK
    */
        // reset all timers
    CanTp_TimerReset(&N_Cr_timer);
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);

    ret = CanTp_SendFlowControl( ID, BS, FC_OVFLW, DEFAULT_ST );

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(CanIf_Transmit_fake.call_count == 3);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);

    /*
      TEST CASE 4
      WYSYLAMY NIEISTNIEJACE FS
    */
        // reset all timers
    CanTp_TimerReset(&N_Cr_timer);
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);

    ret = CanTp_SendFlowControl( ID, BS, 6, DEFAULT_ST );

    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(CanIf_Transmit_fake.call_count == 3);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);

    /*
      TEST CASE 5
      CanIf zwrocil NOT_OK
    */
        // reset all timers
    CanTp_TimerReset(&N_Cr_timer);
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);

    ret = CanTp_SendFlowControl( ID, BS, FC_WAIT, DEFAULT_ST );

    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(CanIf_Transmit_fake.call_count == 4);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
    TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);


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

void Test_Of_Main(){


   // przypominam że funcja main używa funkcji z pdu routera, dla których musimy zrobić FF
   // zasadniczo interesuje nas funkcja PduR_CanTpCopyRxData
   // przygotujmey tablice z wartosciami zwracanymi przez wskaznik
    RESET_FAKE(PduR_CanTpRxIndication);
     RESET_FAKE(CanIf_Transmit);
      
    Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
    Std_ReturnType RetVal_transmit[10] = {E_OK, E_OK, E_NOT_OK, E_NOT_OK, E_OK, E_OK};
    SET_RETURN_SEQ( CanIf_Transmit, RetVal_transmit, 10);
    PduLengthType buffSize_array_local[11] = {0,0,0,0,10,0,0,0,9,0, 10};
    PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    // teraz pora przygotowac wartosci zwrocone przez te funkcje
    BufReq_ReturnType BufferReturnVals[11] = { BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OVFL, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK};
    SET_RETURN_SEQ(PduR_CanTpCopyRxData, BufferReturnVals, 11);
    PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;


    //Fake do przygotowania to RxIndication, tutaj prosta sprawa bo nic nie zwraca wiec nie nie trzeba kombinowac


    // no i nasz fake gotowy ;)
    // jedziemy dalej oto nasze testy do wykonania

    // resetujemy wszystkie timery
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Cr_timer);
    // TEST CASE 1 

    /*
      puszczamy maina dla nieaktywnych timerów, ma nie byc zadnej akcji
    */

    // nie uruchamiamy żadnego timera 
    CanTp_MainFunction ();
    CanTp_MainFunction ();
    CanTp_MainFunction ();
    CanTp_MainFunction ();

    // stan timerow ma pozostac bez zmian 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    // te funkcje nie powinny byc wywolane
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 0); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    // TEST CASE 2

    /*
    normalne dzialanie wszystkich timerow 
    kazdy powinien cos policzyc
    zaden nie powinien wyrzucic timeoutu

    ze wzgledu na to ze nie mamy funkcje sendlowcontrol, 
    nie musimy sprawdzac jej wywolania

    chcemy aby tym razem funkcja calculate available blocks zwrocila 0

    */
    // parametry zostaly tak ustawione abysmy otrzymali ilosc blokow rowna 0
    CanTp_StateVariables.message_length = 10;
    CanTp_StateVariables.sended_bytes = 0;


    // wszystkie timery aktywne 
    CanTp_TimerStart(&N_Ar_timer);
    CanTp_TimerStart(&N_Br_timer);
    CanTp_TimerStart(&N_Cr_timer);
    
    CanTp_MainFunction ();
    CanTp_MainFunction ();
    CanTp_MainFunction ();
    CanTp_MainFunction ();

    // po czterokrotnym wywolaniu funkcji timery powinny wskazywac 4

    TEST_CHECK(N_Ar_timer.counter == 4);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE );
    TEST_CHECK(N_Br_timer.counter == 4);
    TEST_CHECK(N_Br_timer.state == TIMER_ACTIVE );
    TEST_CHECK(N_Cr_timer.counter == 4);
    TEST_CHECK(N_Cr_timer.state == TIMER_ACTIVE );

    // ze wzgledu na to ze N_Br jest aktywny, opowinien wywolac funkcje copy_data    
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4); 
    //  nie ma timeoutu wiec ta funkcja nie powinna byc ruszana
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    // TEST CASE 3 
    /*

      dokonczymy teraz testowanie N_Cr i N_Ar 
      ze wzgledu na to ze n_Br jest bardziej skomplikowany nim zajmiemy sie potem


    */ 

   //Sprawdzienie timeoutu od timera N_Cr
   // blokujemy N_br i N_ar
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Ar_timer);
    //Ustawienie niezerowej wartości do sprawdzenie czy zostanie poprawnie wyzerowana
    CanTp_StateVariables.message_length = 1;
   // ustawiamy wartosc tak aby przy kolejnym wywolaniu poszedl timeout 
   N_Cr_timer.counter = 99;
   CanTp_MainFunction ();
    // ze wzgledu na to ze N_Br jest nieaktywny, ilosc wywolan powinna zostac bez zmian  
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4); 
    //  mamy 1 timeouty wiec funkcje powinna by wywolana 1 raz
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1); 
    // N_Br i N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    // te timery powinny zostac zresetowane, bo osiagnely timeout 
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    //Sprawdzenie czy wartość stanu została poprawnie wyzerowana na timeout od N_Cr
    TEST_CHECK(CanTp_StateVariables.message_length == 0);

   //Sprawdzienie timeoutu od timera N_Ar
   // blokujemy N_br i N_Cr
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Cr_timer);
    // Startujemy N_Ar
    CanTp_TimerStart(&N_Ar_timer);
    //Ustawienie niezerowej wartości do sprawdzenie czy zostanie poprawnie wyzerowana
    CanTp_StateVariables.message_length = 1;
   // ustawiamy wartosc tak aby przy kolejnym wywolaniu poszedl timeout 
    N_Ar_timer.counter = 99;
    CanTp_MainFunction ();
    // ze wzgledu na to ze N_Br jest nieaktywny, ilosc wywolan powinna zostac bez zmian  
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4); 
    //  Timeout od drugiego timera powinien ponownie wywołac funkcję, licznik wywołań powinien zostac zainkrementowany
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); 
    // N_Br i N_Cr powinien byc nieruszony, bo zostal wczesniej zresetowany 
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    // te timery powinny zostac zresetowane, bo osiagnely timeout 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    //Sprawdzenie czy wartość stanu została poprawnie wyzerowana na timeout od N_Ar
    TEST_CHECK(CanTp_StateVariables.message_length == 0);



  /*
    TEST CASE 4 

    tym razem timer N_Br jest aktywny i mamy dobry bufor 
    powinien wyslac CTS i aktywowac N_CR

  */
  // uruchamiamy N_Br
  CanTp_TimerStart(&N_Br_timer);
  // parametry zostaly tak ustawione abysmy otrzymali ilosc blokow rowna 0
    CanTp_StateVariables.message_length = 7;
    CanTp_StateVariables.sended_bytes = 0;

  // wolamy maina
   CanTp_MainFunction ();
    // copy data powinna byc wykonana 
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 5); 
    // ze wzgledu na to ze nie bylo przepelnienia wartosc ma byc bez zmian  
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); 


/* TODO   W zależności od wartości jaką zwróci CanTp_SendFlowControl powinien zostać zatrzymany timer N_Br, 
  Albo zresetowana zmienna stanu recievera. 

  Narazie CanTp_SendControl zwraca zawsze E_OK, dlatego oczekujemy zatrzymania licznika
*/
    // Została ustawiona wartość block_size w zmiennej stanu recievera
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1);
    // Stan recievera powinien zostać ustawiony na PROCESSING
    TEST_CHECK(CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING);
       // N_Br powinien byc zresetowany 
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    // poszedł FLowControl, timery Ar i Cr powinny ruszyć 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE );

    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_ACTIVE );


  /*
    TEST CASE 6

    timer N_Br jest aktywny chcemy zrobić timeouta 
    poeinna inkrementowac sie ilosc wysylanych ramek WAIT

  */
   // uruchamiamy N_Br
  CanTp_TimerReset(&N_Cr_timer);
  CanTp_TimerReset(&N_Ar_timer);
  CanTp_TimerStart(&N_Br_timer);
  // parametry zostaly tak ustawione abysmy otrzymali ilosc blokow rowna 0
    CanTp_StateVariables.message_length = 10;
    CanTp_StateVariables.sended_bytes = 0;
    N_Br_timer.counter = 99;
    FC_Wait_frame_ctr = 0;
  // wolamy maina
   CanTp_MainFunction ();
    // copy data powinna byc wykonana 
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 6); 
    //  ilosc wywolan powinna zostac bez zmian  
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); 
       // N_Br powinien byc dalej aktywny, licznik powinien sie wyzerowac
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_ACTIVE );
    // N_Ar powinien byc ruszony, bo wysyłamy flow controla
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE );
    //// N_Cr powinien byc nieaktywny
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(FC_Wait_frame_ctr == 1);
    /*
    TEST CASE 7 
    sprawdzamy działanie timera po wystapieniu timeouta 
    timer ma normalnie liczyc 
    wszystkie parametry maja byc tak jak poprzenio 
    jedynie wartosc licznika inna
    */

    CanTp_TimerReset(&N_Ar_timer);
    CanTp_MainFunction ();
    // copy data powinna byc wykonana 
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 7); 
    //  ilosc wywolan powinna zostac bez zmian  
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); 
       // N_Br powinien byc dalej aktywny
    TEST_CHECK(N_Br_timer.counter == 1);
    TEST_CHECK(N_Br_timer.state == TIMER_ACTIVE );
    // N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    //// N_Cr powinien byc nieaktywny
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(FC_Wait_frame_ctr == 1);

        /*
    TEST CASE 8 
    sprawdzamy działanie timera po wystapieniu timeouta 
    wyslano rowniez maksymalna ilosc ramek fc wait
    powinien wystapic blad
    odbiranie ma zostac przerwane
    */

    N_Br_timer.counter = 99;
    FC_Wait_frame_ctr = 9;

    // Niezerowa wartość sended_bytes do sprawdzenia, czy reciever jest resetowany (wszystki pola powiny zostać wyzerowane)
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_MainFunction ();
    // copy data powinna byc wykonana 
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 8); 
    //  ilosc wywolan powinna zostac bez zmian  
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 3); 
       // N_Br powinien zostać zresetowany
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    // N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    //// N_Cr powinien byc nieaktywny
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(FC_Wait_frame_ctr == 0);

    // Reciever powinien zostać zresetowany. 
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);



        /*
    TEST CASE 9
      Sprawdzamy działanie po otrzymaniu wartości BUFREQ_E_NOT_OK od PduR
    */

    //Aktywacja timera
    CanTp_TimerStart(&N_Br_timer);
    N_Br_timer.counter = 0;

    // Niezerowa wartość sended_bytes do sprawdzenia, czy reciever jest resetowany (wszystki pola powiny zostać wyzerowane)
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_MainFunction ();
    // copy data powinna byc wykonana 
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 9); 
    //TEST_CHECK(PduR_CanTpCopyRxData_fake.return_val == BUFREQ_E_NOT_OK); 
    
    //  Funkcja powinna być wywołana z argumentem E_NOT_OK
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 4); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
       // N_Br powinien byc dalej aktywny
    TEST_CHECK(N_Br_timer.counter == 1);
    TEST_CHECK(N_Br_timer.state == TIMER_ACTIVE );
    // N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    //// N_Cr powinien byc nieaktywny
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
    TEST_CHECK(FC_Wait_frame_ctr == 0);


    /*
    TEST CASE 8 
    WYSLANIE FLOW CONTROL WAIT ZAKONCZONE NIEPOWODZENIEM
    */
    N_Br_timer.state = TIMER_ACTIVE; 
    N_Br_timer.counter = 99;
    FC_Wait_frame_ctr = 0;

    // Niezerowa wartość sended_bytes do sprawdzenia, czy reciever jest resetowany (wszystki pola powiny zostać wyzerowane)
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_MainFunction ();
    // // copy data powinna byc wykonana 
     TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 10); 
     TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 5); 
    //    // N_Br powinien zostać zresetowany
     TEST_CHECK(N_Br_timer.counter == 0);
     TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    // // N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
     TEST_CHECK(N_Ar_timer.counter == 0);
     TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    // //// N_Cr powinien byc nieaktywny
     TEST_CHECK(N_Cr_timer.counter == 0);
     TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
     TEST_CHECK(FC_Wait_frame_ctr == 1);

    // // Reciever powinien zostać zresetowany. 
    // TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
    N_Br_timer.state = TIMER_ACTIVE; 
    N_Br_timer.counter = 90;
    FC_Wait_frame_ctr = 0;

    // Niezerowa wartość sended_bytes do sprawdzenia, czy reciever jest resetowany (wszystki pola powiny zostać wyzerowane)
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_MainFunction ();

        // // copy data powinna byc wykonana 
     TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count ==11 ); 
     TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 6); 
    //    // N_Br powinien zostać zresetowany
     TEST_CHECK(N_Br_timer.counter == 0);
     TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE );
    // // N_Ar powinien byc nieruszony, bo zostal wczesniej zresetowany 
     TEST_CHECK(N_Ar_timer.counter == 0);
     TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE );
    // //// N_Cr powinien byc nieaktywny
     TEST_CHECK(N_Cr_timer.counter == 0);
     TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE );
     TEST_CHECK(FC_Wait_frame_ctr == 0);

// TESTS OF TX TIMERS

// activate all tx timers and check status after callind main
CanTp_Reset_Rx_State_Variables();
CanTp_ResetTxStateVariables();


CanTp_TimerStart(&N_Bs_timer);
CanTp_TimerStart(&N_Cs_timer);
CanTp_TimerStart(&N_As_timer);

CanTp_MainFunction ();
CanTp_MainFunction ();
CanTp_MainFunction ();

TEST_CHECK(N_As_timer.counter == 3);
TEST_CHECK(N_As_timer.state == TIMER_ACTIVE );

TEST_CHECK(N_Bs_timer.counter == 3);
TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE );

TEST_CHECK(N_Cs_timer.counter == 3);
TEST_CHECK(N_Cs_timer.state == TIMER_ACTIVE );


// TX TIMERS TIMEOUT OF AS

CanTp_Reset_Rx_State_Variables();
CanTp_ResetTxStateVariables();

N_As_timer.counter = 99;

CanTp_TimerStart(&N_As_timer);


CanTp_MainFunction ();

TEST_CHECK(N_As_timer.counter == 0);
TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Bs_timer.counter == 0);
TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Cs_timer.counter == 0);
TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);

// TX TIMERS TIMEOUT OF BS

CanTp_Reset_Rx_State_Variables();
CanTp_ResetTxStateVariables();

N_Bs_timer.counter = 99;

CanTp_TimerStart(&N_Bs_timer);


CanTp_MainFunction ();

TEST_CHECK(N_As_timer.counter == 0);
TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Bs_timer.counter == 0);
TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Cs_timer.counter == 0);
TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 2);

// TX TIMERS TIMEOUT OF CS

CanTp_Reset_Rx_State_Variables();
CanTp_ResetTxStateVariables();

N_Cs_timer.counter = 99;

CanTp_TimerStart(&N_Cs_timer);


CanTp_MainFunction ();

TEST_CHECK(N_As_timer.counter == 0);
TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Bs_timer.counter == 0);
TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(N_Cs_timer.counter == 0);
TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE );

TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 3);




}

//test of CanTp_RxIndication

///< ONLY SF>

void Test_Of_CanTp_RxIndication_SF(void){

// declare test variables
  int i;
  PduIdType  RxPduId = 1;

  PduInfoType PduInfoPtr;
  CanPCI_Type CanPCI;

// configure FF
  CanTp_State = CAN_TP_ON;
  RESET_FAKE(PduR_CanTpStartOfReception);
  RESET_FAKE(PduR_CanTpCopyRxData); 
  RESET_FAKE(PduR_CanTpRxIndication);


  RESET_FAKE(CanIf_Transmit);
      
  Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
  SET_RETURN_SEQ( CanIf_Transmit, RetVal, 10);

// PDU Router FF
  PduLengthType buffSize_array_local[10] = {7,1,7,7,7,7,7,7,9,0};
  PduR_CanTpStartOfReception_buffSize_array = buffSize_array_local;
  PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    
  BufReq_ReturnType PduR_CanTpStartOfReception_ReturnVals[10] = { BUFREQ_OK, BUFREQ_OK, BUFREQ_OVFL, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK}; 
  BufReq_ReturnType PduR_CanTpCopyRxData_ReturnVals[10] =       { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK}; 
  SET_RETURN_SEQ(PduR_CanTpStartOfReception, PduR_CanTpStartOfReception_ReturnVals, 6);
  PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;

  SET_RETURN_SEQ(PduR_CanTpCopyRxData, PduR_CanTpCopyRxData_ReturnVals, 4);
  PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;

// set CanTp internal variables

// testujemy scenariusz ze strony 62
/*

  TEST CASE 1

  testujemy sytuacje w której odbieramy ramkę typu SINGLE FRAME
  zakładamy że rozmiar bufora jest wystarczający na odebranie ramki 
  wszystko pozimmo pójść zgodnie z planem

*/

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;

  CanPCI.frame_lenght = 7;
  CanPCI.frame_type = SF;
  uint8 payload[8] = "kasztan"; 

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduLength == 7);

  //TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[i] == payload[i]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);
  

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 1); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 1); // this functio shlud be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_OK);

/*

  TEST CASE 2

  testujemy sytuacje w której odbieramy ramkę typu SINGLE FRAME dokładnie 
  taka samą jak w poprzedim tescie
  zakładamy że rozmiar bufora jest za mały na odebranie ramki 
  odbieranie powinno zakońćzyć się błędem 
  w efekcie powinna zostać wysłana wartośc E_NOT_OK do PDU Routera

*/

  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 2); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 1); // this functio shlud not be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); // 
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
  
  
  /*
  TEST CASE 3

  testujemy sytuacje w której odbieramy ramkę typu SINGLE FRAME dokładnie 
  taka samą jak w poprzedim tescie
  zakłądamy że PduR_CanTpStartOfReception nie zwróciła wartości BUFFER_OK 
  odbieranie powinno zakońćzyć się błędem, ale nie należy wołać funkcji PDUR_CanTpRxIndication
*/
  
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 3); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 1); // this functio shlud not be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2); 
 
  /*
  TEST CASE 4

  testujemy sytuacje w której odbieramy ramkę typu SINGLE FRAME dokładnie 
  taka samą jak w poprzedim tescie
  zakłądamy że PduR_CanTpStartOfReception zwróciła wartości BUFFER_OK
  natomiast funkcja kopiująca dane zwróciłą bląd 
  odbieranie powinno zakońćzyć się błędem, należy wywołać funkcję PDUR_CanTpRxIndication
  */
  // PduR_CopyData returns error
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduLength == 7);

  //TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[i] == payload[i]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);
  

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);                    // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 4);   // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 2);         // this function should be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 3);       //
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

  /*
    TEST CASE 5

    testujaemy poprawną ramkę w momencie kiedy jest nieoczekiwana 
    receiver jest w stanie PROCESSING 
    trawjąca transmisja powinna zostać przerwana 
    ramka SF powinna zostać wysłana poprawnie

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduLength == 7);

  //TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[i] == payload[i]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);
  

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 5); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 3); // this functio shlud be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 5);

  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_history[3] == E_NOT_OK);
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_OK);

  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0); // to powinno zostać wyzerowane 
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); // to też

  /*
    TEST CASE 6

    testujaemy poprawną ramkę w momencie kiedy jest nieoczekiwana 
    receiver jest w stanie PROCESSING_SUSPENDED 9oczekiwanie na bufor)
    trawjąca transmisja powinna zostać przerwana 
    ramka SF powinna zostać wysłana poprawnie

  */
  CanTp_Reset_Rx_State_Variables();
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduLength == 7);

  //TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[i] == payload[i]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);
  

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 6); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4); // this functio shlud be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 7);

  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_history[5] == E_NOT_OK);
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_OK);

  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0); // to powinno zostać wyzerowane 
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); // to też
       

}

void Test_Of_CanTp_RxIndication_FF(void){

// declare test variables
  int i;
  PduIdType  RxPduId = 1;

  PduInfoType PduInfoPtr;
  CanPCI_Type CanPCI;
  CanTp_State = CAN_TP_ON;
// configure FF
  RESET_FAKE(PduR_CanTpRxIndication);
  RESET_FAKE(PduR_CanTpStartOfReception);
  RESET_FAKE(PduR_CanTpCopyRxData); 
    RESET_FAKE(CanIf_Transmit);
      
  Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
  SET_RETURN_SEQ( CanIf_Transmit, RetVal, 10);
// PDU Router FF
  PduLengthType buffSize_array_local[10] = {7,1,7,7,7,7,7,8,9,0};
  PduR_CanTpStartOfReception_buffSize_array = buffSize_array_local;
  PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    
  BufReq_ReturnType PduR_CanTpStartOfReception_ReturnVals[10] = { BUFREQ_OK, BUFREQ_OK, BUFREQ_OVFL, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK,}; 
  BufReq_ReturnType PduR_CanTpCopyRxData_ReturnVals[10] =       { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK,}; 
  SET_RETURN_SEQ(PduR_CanTpStartOfReception, PduR_CanTpStartOfReception_ReturnVals, 6);
  PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;

  SET_RETURN_SEQ(PduR_CanTpCopyRxData, PduR_CanTpCopyRxData_ReturnVals, 4);
  PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;

// wszystk idzie ok

/*
  TEST CASE 1
  
  odbieramy ramkę typu FF i zakładamy, że wszystko idzie zgodnie z planem
  zakłądamy że funkcja PduR_CanTpStartOfReception zwróciła BUFREQ_OK
  zakłądamy że bufor jest w stanie pomiescić co najmniej jedną ramkę CF (co najmniej 7B)
  funkcja powinna wysłać ramkę FlowControl typu CTS 
  uzupełnić zmienne stanu
  przejść do stanu RX_PROCESSINS 

*/

  CanTp_Reset_Rx_State_Variables();

  CanPCI.frame_lenght = 100;  // długość odbieranych danych
  CanPCI.frame_type = FF;
  uint8 payload[8] = "xxx"; // w wypadku FF payload nie ma znaczenia

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 100); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 1);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING); // funkcja powinna przejsc do stanu proceeing
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1);  // bufor zostal tak dobrany aby ilosc blokow byla rowna 1 
  TEST_CHECK(CanTp_StateVariables.message_length == 100);   // do zmiennych stanu powinna dolaczyc dlugosc wiadomosci
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1); // do zmiennych stanu powinno dalczyc ID
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);       //PduR_CanTpRxIndication nie powinna zostac wywalona bo nie ma bledu
 // TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

// za maly bufor

/*
  TEST CASE 2
  
  odbieramy ramkę typu FF 
  zakłądamy że funkcja PduR_CanTpStartOfReception zwróciła BUFREQ_OK
  bufor jest jednak za mały na pomieszczenie CF
  funkcja powinna wysłać ramkę FlowControl typu WAIT 
  uzupełnić zmienne stanu
  przejść do stanu RX_PROCESSING_SUSPENDED 

*/

  CanTp_Reset_Rx_State_Variables();

  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 100); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 1);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);       //PduR_CanTpRxIndication nie powinna zostac wywalona bo nie ma bledu
 // TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);


/*
  TEST CASE 3
  
  odbieramy ramkę typu FF i zakładamy
  zakłądamy że funkcja PduR_CanTpStartOfReception zwróciła BUFREQ_OVFL
  funkcja powinna wysłać ramkę FlowControl typu OVLF
  funkcja powinna zresetować zmienne stanu 
  funkcja powinna przejsc do stanu WAIT

*/

  CanTp_Reset_Rx_State_Variables();

  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 100); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); // ta funkcja nie powinna zostac wywolana 

  /*
  TEST CASE 4
  
  odbieramy ramkę typu FF i zakładamy
  zakłądamy że funkcja PduR_CanTpStartOfReception zwróciła BUFREQ_NOT_OK
  funkcja powinna zresetować zmienne stanu 
  funkcja powinna przejsc do stanu WAIT

*/

  CanTp_Reset_Rx_State_Variables();

  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 100); // sprawdz argumenty
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0); // ta funkcja nie powinna zostac wywolana 
  /*
  TEST CASE 5
  
  UNEXPECTED FRAME DETECTION 

  wysyłamy ramkę FF, kuedy receiver oczekuje CF

  poprzednia transmisja powinna zostać przerwana ze 
  statusem NOT_OK i rozpoaczęta powinna zostac nowa

*/

  // send FF when is unexpected
  CanTp_Reset_Rx_State_Variables();
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 1);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

/*
  TEST CASE 6
  
  UNEXPECTED FRAME DETECTION 

  wysyłamy ramkę FF, kuedy receiver oczekuje CF

  poprzednia transmisja powinna zostać przerwana ze 
  statusem NOT_OK i rozpoaczęta powinna zostac nowa

*/

  // send FF when is unexpected
  CanTp_Reset_Rx_State_Variables();
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 1);
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
}

void Test_Of_CanTp_RxIndication_CF(void){

// declare test variables
  int i;
  PduIdType  RxPduId = 1;

  PduInfoType PduInfoPtr;
  CanPCI_Type CanPCI;
  CanTp_State = CAN_TP_ON;
// configure FF
  RESET_FAKE(PduR_CanTpRxIndication);
  RESET_FAKE(PduR_CanTpCopyRxData); 
  RESET_FAKE(CanIf_Transmit);
      
  Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
  SET_RETURN_SEQ( CanIf_Transmit, RetVal, 10);
// PDU Router FF
  PduLengthType buffSize_array_local[10] = {7,7,1,7,7,7,7,8,9,0};
  PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

  BufReq_ReturnType PduR_CanTpCopyRxData_ReturnVals[10] =       { BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_OK}; 

  SET_RETURN_SEQ(PduR_CanTpCopyRxData, PduR_CanTpCopyRxData_ReturnVals, 5);
  PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;

  CanTp_Reset_Rx_State_Variables();

  // to jest do przygotowanie ramki
  CanPCI.SN = 4;  // długość odbieranych danych
  CanPCI.frame_type = CF;
  uint8 payload[8] = "Marshal"; // w wypadku FF payload nie ma znaczenia

  /*
    TEST CASE 1 
    Frame should be recieved without any problem
    No Flow Control
*/
  
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 2; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  PduInfoPtr.SduLength = 7;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1); 
  TEST_CHECK(CanTp_StateVariables.message_length == 20);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 5);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 7);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);

  // check timer state - todo
  /*
    TEST CASE 2 
    Frame should be recieved, but buffer it is time to send CTS 
    Flow Control with CTS shuld be send
*/
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 2);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1); 
  TEST_CHECK(CanTp_StateVariables.message_length == 20);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 5);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 7);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);


  /*
    TEST CASE 3 
    Frame should be recieved, but buffer is too small for further frames
    Flow Control with WAIT shuld be send

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 3);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 20);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 5);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 7);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);

    /*
    TEST CASE 4 
    buffer failure - reception abort

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);

  /*
    TEST CASE 5 
    error - unexpected SN

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 5; // here
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);


  /*
    TEST CASE 6 
    error - unexpected ID

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 5; // here
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 2; //here

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 3);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);

  /*
    TEST CASE 7 
    FINISH SENDING MESSAGE
  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.blocks_to_next_cts = 1; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4; 
  CanTp_StateVariables.sended_bytes = 13;
  CanTp_StateVariables.CanTp_Current_RxId = 1; 

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 5);
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 4);       
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_OK);

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);

  // check state variables
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);

  /*
    TEST CASE 8
    CALLING SEND CONSECUTIVE FRAME WHEN IT IS UNEXPECTED
    FRAME SHOULD BE IGNORED

  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
  CanTp_StateVariables.blocks_to_next_cts = 2; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  PduInfoPtr.SduLength = 7;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 5);   // should not be changed, function shouldnt be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 4); // should not be changed,

  // check state variables - should not be changed
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 2); 
  TEST_CHECK(CanTp_StateVariables.message_length == 20);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 4);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);

  /*
    TEST CASE 9
    CALLING SEND CONSECUTIVE FRAME WHEN IT IS WAITING FOR BUFFER 
    ERROR SHOULD OCCURE
    COMMUNICATION SHOULD BE RESET
  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_StateVariables.blocks_to_next_cts = 2; 
  CanTp_StateVariables.message_length = 20;
  CanTp_StateVariables.expected_CF_SN = 4;
  CanTp_StateVariables.sended_bytes = 0;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  PduInfoPtr.SduLength = 7;
  CanTp_RxIndication (RxPduId, &PduInfoPtr );
  
  // function calls
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 5);   // should not be changed, function shouldnt be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 5); // should not be changed,
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK);
  // check state variables - should not be changed
  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0); 
  TEST_CHECK(CanTp_StateVariables.message_length == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);


}




void Test_Of_CanTp_SendConsecutiveFrame(void){
  Std_ReturnType ret; 
  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK};
  PduIdType PduId = 0xF1A7;
  uint8 payload[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  uint32 size = 7;
  uint8 SN = 0x1;
  //Create and Fill PduInfo with PrepareSegmentedFrame function
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  CanPCI_Type CanPCI = {CF, size, SN, 0, 0, 0};
  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);

  //Ustawienie sekwencji wartości zwracanych przez CanIF_Transmitt
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 3);

  //TEST1: Sprawdzenie czy dane wysyłane przez CanIf pokrywają się 
  // Z danymi dostarczonymi do funkcji
  ret = CanTp_SendConsecutiveFrame(PduId, SN,  payload, size);
  // Sprawdzenie czy funcja została wywołana
  TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Porównujemy pierwszy bajt z SduDataPtr. W przypadku CF pozostałe bajty są bez znaczenia
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == PduInfo.SduDataPtr[0]);

  //TEST 2: Transmisja CanIf przebiega pomyślnie
  //Wyzerowanie licznika N_As_timer
  CanTp_TimerReset(&N_As_timer);
  //Wywołanie funkcji
  ret = CanTp_SendConsecutiveFrame(PduId, SN,  payload, size);
  TEST_CHECK(ret == E_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 2);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE);
  //CanTpTxConfirmation nie powinna być wywołana
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);

  //TEST 3: Transmisja CanIf zwraca E_NOT_OK
  //Zmiana ID
  PduId = 0x0101;
  //Wyzerowanie licznika N_As_timer
  CanTp_TimerReset(&N_As_timer);
  //Wywołanie funkcji, powinna zwrócić E_NOT_OK
  ret = CanTp_SendConsecutiveFrame(PduId, SN, payload, size);
  TEST_CHECK(ret == E_NOT_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_NOT_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 3);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS nie powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE);
  //CanTpTxConfirmation powinna być wywołana z aktualny id oraz E_NOT_OK
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == PduId);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
}

void Test_Of_CanTp_SendSingleFrame(void){
  Std_ReturnType ret; 
  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK};
  PduIdType PduId = 0xF1A7;
  uint8 payload[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  uint32 size = 7;
  //Create and Fill PduInfo with PrepareSegmentedFrame function
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  CanPCI_Type CanPCI = {SF, size, 0, 0, 0, 0};
  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
  //Ustawienie sekwencji wartości zwracanych przez CanIF_Transmitt
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 3);
  
  //TEST1: Sprawdzenie czy dane wysyłane przez CanIf pokrywają się 
  // Z danymi dostarczonymi do funkcji
  ret = CanTp_SendSingleFrame(PduId, payload, size);
  // Sprawdzenie czy funcja została wywołana
  TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  TEST_CHECK(memcmp(CanIf_Transmit_fake.arg1_val->SduDataPtr, PduInfo.SduDataPtr, 8) == 0);
  
  //TEST 2: Transmisja CanIf przebiega pomyślnie
  //Wyzerowanie licznika N_As_timer
  CanTp_TimerReset(&N_As_timer);
  //Wywołanie funkcji
  ret = CanTp_SendSingleFrame(PduId, payload, size);
  TEST_CHECK(ret == E_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 2);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE);
  //CanTpTxConfirmation nie powinna być wywołana
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);

  //TEST 3: Transmisja CanIf zwraca E_NOT_OK
  //Zmiana ID
  PduId = 0x0101;
  //Wyzerowanie licznika N_As_timer
  CanTp_TimerReset(&N_As_timer);
  //Wywołanie funkcji, powinna zwrócić E_NOT_OK
  ret = CanTp_SendSingleFrame(PduId, payload, size);
  TEST_CHECK(ret == E_NOT_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_NOT_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 3);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS nie powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE);
  //CanTpTxConfirmation powinna być wywołana z aktualny id oraz E_NOT_OK
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == PduId);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
}

void Test_Of_CanTp_SendFirstFrame(void){
  Std_ReturnType ret; 
  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK};
  PduIdType PduId = 0xF1A7;
  uint32 size = 10000;
  uint8 payload[8]; 
  //Create and Fill PduInfo with PrepareSegmentedFrame function
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  CanPCI_Type CanPCI = {FF, size, 0, 0, 0, 0};
  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
  //Ustawienie sekwencji wartości zwracanych przez CanIF_Transmitt
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 3);
  
  //TEST 1: Sprawdzenie czy CanIf jest wywoływany z odpowiednimi parametrami
  ret = CanTp_SendFirstFrame(PduId, size);
  // Sprawdzenie czy funcja została wywołana
  TEST_CHECK(CanIf_Transmit_fake.call_count == 1);
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  // Size > 5095 wiec nalezy porównać sześć pierwszych bajtów danych 
  TEST_CHECK(memcmp(CanIf_Transmit_fake.arg1_val->SduDataPtr, PduInfo.SduDataPtr, 6) == 0);

  //TEST 2: Transmisja CanIf przebiega pomyślnie
  //Zmiana ID
  PduId = 0x0101;
  //Wyzerowanie licznika N_As_timer oraz N_Bs_timer
  CanTp_TimerReset(&N_As_timer);
  CanTp_TimerReset(&N_Bs_timer);
  //Wywołanie funkcji
  ret = CanTp_SendFirstFrame(PduId, size);
  TEST_CHECK(ret == E_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 2);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE);
  TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE);
  //CanTpTxConfirmation nie powinna być wywołana
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);

  //TEST 3: Transmisja CanIf zwraca E_NOT_OK
  //Zmiana ID
  PduId = 0x0101;
  //Wyzerowanie licznika N_As_timer i N_Bs
  CanTp_TimerReset(&N_As_timer);
  CanTp_TimerReset(&N_Bs_timer);
  //Wywołanie funkcji, powinna zwrócić E_NOT_OK
  ret = CanTp_SendFirstFrame(PduId, size);
  TEST_CHECK(ret == E_NOT_OK);
  //Funkcja CanIf_Transmit powinna zostać wywołana i powinna zwrócić E_NOT_OK
  TEST_CHECK(CanIf_Transmit_fake.call_count == 3);
  //Sprawdzienie argumentów wywołania
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == PduId);
  //Timer N_AS, NBSnie powinien zostać aktywowany:
  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  //CanTpTxConfirmation powinna być wywołana z aktualny id oraz E_NOT_OK
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == PduId);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
}




// TODO
void TestOf_CanTp_ResetTxStateVariables(void){
  /*Test funkcji do resetowania zmiennej stanu nadajnika */

    //Set TxStateVariable
    CanTp_Tx_StateVariables.blocks_to_fc = 1;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;
    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
    CanTp_Tx_StateVariables.message_legth = 1;
    CanTp_Tx_StateVariables.sent_bytes = 1;

    //Call function
    CanTp_ResetTxStateVariables();
    
    TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);
}

// TODO Test funkcji CanTp_Transmit
void TestOf_CanTp_Transmit(void){

  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 
 

  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[4][7] = { "marszal", "ek prog", "ramuje ","1234567" };


  PduLengthType availableData;
  int i;

  for( i = 0; i < 4; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);
  RESET_FAKE( CanIf_Transmit );
  RESET_FAKE( PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;





  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OK};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 4);

  
  // TEST 1 = CAN TP W STANIE OFF
   CanTp_State = CAN_TP_OFF;
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 7;
  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  ret = CanTp_Transmit(PduId, &PduInfo);
  
  TEST_CHECK( ret == E_NOT_OK );
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
 
 CanTp_State = CAN_TP_ON;
  
/*
  TEST1: 
  Tx w stanei WAIT
  SduLenght < 8 - bedziemy wysylac SF
  CanTpCopyTx zwraca BUFREQ_OK
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 7;
  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  ret = CanTp_Transmit(PduId, &PduInfo);
  //Zwrócona wartość powinna wynosic E_OK
  TEST_CHECK(ret == E_OK);
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == PduId);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x7 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'm' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'r' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 's' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == 'z' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[6] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[7] == 'l' );

  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE); // sprawdzamy czy timer dziala


  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  
  CanTp_Reset_Rx_State_Variables();
/*
  TEST2: 
  Tx w stanei WAIT, SduLenght < 8, CanTpCopyTx zwraca BUFREQ_E_NOT_OK
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 7;
  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  ret = CanTp_Transmit(PduId, &PduInfo);
  //Zwrócona wartość powinna wynosic E_OK
  TEST_CHECK(ret == E_NOT_OK);
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 2);
  //Zmienna stanu powinna zostac zresetowana 
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0);
  //Powinien zostać wysłany TpTxConfirmation z E_NOT_OK
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);

  /*
    TEST3: 
    Tx w stanie WAIT, 
    SduLenght < 8, 
    CanTpCopyTx zwraca BUFREQ_BUSY
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 7;
  CanTp_TimerReset(&N_Cs_timer); //Zresetowanie timera
  ret = CanTp_Transmit(PduId, &PduInfo);
  //Zwrócona wartość powinna wynosic E_OK
  TEST_CHECK(ret == E_OK);
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 3); 
  //Timer N_Cs powinien zostac aktywowany
  TEST_CHECK(N_Cs_timer.state == TIMER_ACTIVE);

  CanTp_Reset_Rx_State_Variables();
/*
  TEST4: 
  Tx w stanei WAIT, 
  SduLenght >= 8, 
  CanTpSendFirst Frame zwraca E_OK
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 1000;
  ret = CanTp_Transmit(PduId, &PduInfo);
  //Zwrócona wartość powinna wynosic E_OK
  TEST_CHECK(ret == E_OK);
  //Funcja CanTpCopyTxData nie powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 3);
  //Trasniter powinien przejsć w trym PROCESSING_SUSPEBDEDB
  TEST_CHECK(CanIf_Transmit_fake.return_val == E_OK);
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == PduInfo.SduLength);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 2 ); 
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x13 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 0xE8 );
  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE);
  TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE);

  //TEST5: 
  //Tx w stanei WAIT, SduLenght >= 8, CanTpSenFirst Frame zwraca E_NOT_OK
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  PduInfo.SduLength = 1000;
  ret = CanTp_Transmit(PduId, &PduInfo);
  //Zwrócona wartość powinna wynosic E_OK
  TEST_CHECK(ret == E_NOT_OK);
  //Funcja CanTpCopyTxData nie powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 3);
  //Trasniter powinien ppozostać w stanie CANTP_TX_WAIT
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
}

// TODO Test funkcji CanTp_Transmit
void TestOf_CanTp_SendNextCF(void){

  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 


  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[5][7] = { "marszal", "ek prog", "ramuje ","1234567", "1234567" };


  PduLengthType availableData;
  int i;

  for( i = 0; i < 5; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);
  RESET_FAKE( CanIf_Transmit );
  RESET_FAKE( PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK , BUFREQ_BUSY};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 5);

  
/*
  TEST1: 
  Sending normal CF 
  all buffers are OK
  CanIf transmit finished with success
  Tx should stay at TX_PROCESSING
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 2;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 0;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x20 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'm' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'r' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 's' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == 'z' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[6] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[7] == 'l' );

  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE); // sprawdzamy czy timer dziala


  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 7);


  CanTp_Reset_Rx_State_Variables();

  /*
    TEST 2
    buffer is OK
    message parametrs of
    blocks to next cts is 1 so tx sohuld switch to processing suspended
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 7;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 2);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 2 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x20 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'e' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'k' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == ' ' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 'p' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == 'r' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[6] == 'o' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[7] == 'g' );

  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE); // sprawdzamy czy timer dziala

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 14);

  CanTp_Reset_Rx_State_Variables();

  /*
    TEST 3
    buffer is OK
    message parametrs of
    blocks to next cts is 1 so tx sohuld switch to processing suspended
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 7;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 3);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 3 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana


  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();

/*

  TEST 4
  copy tx data returns e_not_ok - communication should be aborted

*/

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 7;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 4);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 3 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 2);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();


  /*

  TEST 5
  copy tx data returns e_busy - communication should wait for free buffer

*/

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 7;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 5);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 3 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_Cs_timer.state == TIMER_ACTIVE); 

  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 2);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 7);

  CanTp_Reset_Rx_State_Variables();

  /*
  TEST 6 - finish  of transmission

  */

   CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 100;
  CanTp_Tx_StateVariables.next_SN = 0;

  //Ustawienie niezerowej zmiennej środowiskowej
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_SendNextCF();
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 5);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);

 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 3 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 3);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_OK);
  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();

}

void Test_Of_CanTp_FlowControlReception(void){
  
  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 

  PduInfoType PduInfoPtr;
  CanPCI_Type CanPCI;

  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[5][7] = { "marszal", "ek prog", "ramuje ","1234567", "1234567" };


  PduLengthType availableData;
  int i;

  for( i = 0; i < 5; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);
  RESET_FAKE( CanIf_Transmit );
  RESET_FAKE( PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK , BUFREQ_BUSY};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 5);

  
/*
  TEST1: 
  Sending normal CF 
  all buffers are OK
  CanIf transmit finished with success
  Tx should stay at TX_PROCESSING
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;



  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_CTS;
  CanPCI.BS = 1;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(1, &CanPCI);
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 5);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x20 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'm' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'r' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 's' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == 'z' );

  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE); // sprawdzamy czy timer dziala


  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 100);

  CanTp_Reset_Rx_State_Variables();


  /*
  TEST 2
  ramka flow control z argumentem OVFL

*/

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_OVFLW;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(1, &CanPCI);
 

  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg0_val == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK );

  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();


  /*
  TEST 3 - frame with FC_WAIT (UNFINISHED)

  */

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(1, &CanPCI);
 

  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);

  CanTp_Reset_Rx_State_Variables();

  /*
  TEST 3 - UNEXPECTED FRAME - should be ignored

  */

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(1, &CanPCI);
 

  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);

  CanTp_Reset_Rx_State_Variables();

  /*
  TEST 5 - frame with wrong ID

  */


  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(2, &CanPCI); // wrong ID - 2 but should be 1
 

  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Bs_timer.state == TIMER_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);

  CanTp_Reset_Rx_State_Variables();

    /*
  TEST 6 - frame with unknown frame type - ERRPR = transmit should be aborted

  */


  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
  CanTp_Tx_StateVariables.blocks_to_fc = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanPCI.frame_type = FC;
  CanPCI.FS = 5; // unknown type
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_FlowControlReception(1, &CanPCI); 
 

  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 2 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg0_val == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK );

  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE); 
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE); 

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();
}

void Test_Of_CanTp_TxConfirmation(){

  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 


  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[5][7] = { "marszal", "ek prog", "ramuje ","1234567", "1234567" };


  PduLengthType availableData;
  int i;

  for( i = 0; i < 5; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);
  RESET_FAKE( CanIf_Transmit );
  RESET_FAKE( PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK , BUFREQ_BUSY};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 5);

  CanTp_State = CAN_TP_ON;
/*
  TEST1: 
  wszystko idzie ok
*/
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;



  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation (1, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami

  //Sprawdzenie argumentów funckji CanTpCopyData
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 5);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); // sprawdzaym z jakimi argumentami funkcja wysylajaca zostala wykonana
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x20 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'm' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'r' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 's' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == 'z' );

  TEST_CHECK(N_As_timer.state == TIMER_ACTIVE); // sprawdzamy czy timer dziala


  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 100);

  CanTp_Reset_Rx_State_Variables();
   CanTp_ResetTxStateVariables();

  /*
    TEST 2 - RETURN NOT OK
  */

    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;



  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation (1, E_NOT_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg0_val == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK );
  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

  CanTp_Reset_Rx_State_Variables();
   CanTp_ResetTxStateVariables();

  /*
    TEST 3 - UNKNOWN ID
  */

    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;



  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation (2, E_NOT_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();


  /*
    TEST 4 - NOT IN PROCESSING STATE
  */

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;



  //Ustawienie niezerowej zmiennej stanu
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation (1, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 1); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();


  /* 
    TEST 5 
    RECEIVER - FLOW CONTROL RECEPTION CONFIRMATION 
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (1, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();


    /* 
    TEST 6
    RECEIVER - FLOW CONTROL RECEPTION CONFIRMATION 
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (1, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();

  /* 
    TEST 7
    RECEIVER - FLOW CONTROL RECEPTION CONFIRMATION 
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (1, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();


    /* 
    TEST 8
    RECEIVER - UNKNOWN ID
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (2, E_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();


  /* 
    TEST 9
    RECEIVER - E_NOT_OK
  */
  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (1, E_NOT_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Br_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cr_timer.state == TIMER_NOT_ACTIVE);

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
  TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);

  TEST_CHECK( PduR_CanTpRxIndication_fake.call_count == 1 ); 
  TEST_CHECK( PduR_CanTpRxIndication_fake.arg0_val == 1 ); 
  TEST_CHECK( PduR_CanTpRxIndication_fake.arg1_val == E_NOT_OK ); 

  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();

    /* 
    TEST 10
    CAN_TP_OFF
  */
  CanTp_State = CAN_TP_OFF;

  CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
  CanTp_Tx_StateVariables.blocks_to_fc = 1;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
  CanTp_Tx_StateVariables.message_legth = 100;
  CanTp_Tx_StateVariables.sent_bytes = 95;
  CanTp_Tx_StateVariables.next_SN = 0;
  CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
  CanTp_StateVariables.CanTp_Current_RxId = 1;

  N_Ar_timer.state = TIMER_ACTIVE;


  //Ustawienie niezerowej zmiennej stanu
  

  CanTp_TxConfirmation (1, E_NOT_OK );
 
  //Funcja CanTpCopyTxData powinna zostac wywołana
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  // sprawdzamy czy CanTpCopyData zostal wywolany z poprawnymi argumentami


 
  // sprawdzamy czy sen single frame wykonal sie poprawnie 

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 ); // sprawdzamy czy can tp cos wyslal

  TEST_CHECK(N_As_timer.state == TIMER_NOT_ACTIVE); // sprawdzamy czy timer dziala
  TEST_CHECK(N_Bs_timer.state == TIMER_NOT_ACTIVE);
  TEST_CHECK(N_Cs_timer.state == TIMER_NOT_ACTIVE);

  //Zmienna stanu nie powinna się zmienic
  TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 2); //sprawdzenie czy ID sie nie zmieniclo
  TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
  TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
  TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);


  // n ar timer shuol be in reset state

  TEST_CHECK(N_Ar_timer.state == TIMER_ACTIVE);


  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);

  TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);


  CanTp_Reset_Rx_State_Variables();
  CanTp_ResetTxStateVariables();

}

void Test_Of_CanTp_Init(void){
    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
    CanTp_Tx_StateVariables.blocks_to_fc = 1;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
    CanTp_Tx_StateVariables.message_legth = 100;
    CanTp_Tx_StateVariables.sent_bytes = 95;
    CanTp_Tx_StateVariables.next_SN = 0;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

    CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING;
    CanTp_StateVariables.blocks_to_next_cts = 1;
    CanTp_StateVariables.CanTp_Current_RxId = 1;
    CanTp_StateVariables.expected_CF_SN = 1;
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_State = CAN_TP_OFF;
  // shult reset all variables and set state to ON
    CanTp_Init();
    
    TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

    TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);

    TEST_CHECK(CanTp_State == CAN_TP_ON);
}

void Test_Of_CanTp_Shutdown(void){
    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
    CanTp_Tx_StateVariables.blocks_to_fc = 1;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
    CanTp_Tx_StateVariables.message_legth = 100;
    CanTp_Tx_StateVariables.sent_bytes = 95;
    CanTp_Tx_StateVariables.next_SN = 0;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 2;

    CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING;
    CanTp_StateVariables.blocks_to_next_cts = 1;
    CanTp_StateVariables.CanTp_Current_RxId = 1;
    CanTp_StateVariables.expected_CF_SN = 1;
    CanTp_StateVariables.sended_bytes = 1;

    CanTp_State = CAN_TP_ON;
    // shult reset all variables and set state to OFF
    CanTp_Shutdown();
    
    TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);

    TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);

    TEST_CHECK(CanTp_State == CAN_TP_OFF);
}

void Test_Of_CanTp_CancelTransmit(void){

  /*
    TEST 1 

    Correct_Id - ret E_OK

  */
    Std_ReturnType ret;
    
    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
    CanTp_Tx_StateVariables.blocks_to_fc = 1;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
    CanTp_Tx_StateVariables.message_legth = 100;
    CanTp_Tx_StateVariables.sent_bytes = 95;
    CanTp_Tx_StateVariables.next_SN = 0;

    ret = CanTp_CancelTransmit ( 0x1 );

   TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 0);
    TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 0);
    TEST_CHECK( ret == E_OK );

/*
  TEST 2
  wrong id - relut E_NOT_OK

*/

    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
    CanTp_Tx_StateVariables.blocks_to_fc = 1;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 0x1;
    CanTp_Tx_StateVariables.message_legth = 100;
    CanTp_Tx_StateVariables.sent_bytes = 95;
    CanTp_Tx_StateVariables.next_SN = 0;

    ret = CanTp_CancelTransmit ( 0 );

   TEST_CHECK(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_Tx_StateVariables.CanTp_Current_TxId == 0x1); 
    TEST_CHECK(CanTp_Tx_StateVariables.blocks_to_fc == 1);
    TEST_CHECK(CanTp_Tx_StateVariables.message_legth == 100);
    TEST_CHECK(CanTp_Tx_StateVariables.sent_bytes == 95);
    TEST_CHECK( ret == E_NOT_OK );

}

void Test_Of_CanTp_CancelReceive(void){

    Std_ReturnType ret;

    /* TEST 1 - ID OK */

    CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING;
    CanTp_StateVariables.blocks_to_next_cts = 1;
    CanTp_StateVariables.CanTp_Current_RxId = 1;
    CanTp_StateVariables.expected_CF_SN = 1;
    CanTp_StateVariables.sended_bytes = 1;

    ret = CanTp_CancelReceive ( 1 );

    TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 0);
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 0);
    TEST_CHECK( ret == E_OK );

    /* TEST 2 - WRONG_ID */
    CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
    CanTp_StateVariables.blocks_to_next_cts = 1;
    CanTp_StateVariables.CanTp_Current_RxId = 1;
    CanTp_StateVariables.expected_CF_SN = 1;
    CanTp_StateVariables.sended_bytes = 1;

    ret = CanTp_CancelReceive ( 0 );

    TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING);
    TEST_CHECK(CanTp_StateVariables.blocks_to_next_cts == 1);
    TEST_CHECK(CanTp_StateVariables.CanTp_Current_RxId == 1);
    TEST_CHECK(CanTp_StateVariables.expected_CF_SN == 1);
    TEST_CHECK(CanTp_StateVariables.sended_bytes == 1);
    TEST_CHECK( ret == E_NOT_OK );

}

void Test_Of_CanTp_RxIndication_FC(void){

// declare test variables
  int i;
  PduIdType  RxPduId = 1;

  PduInfoType PduInfoPtr;
  CanPCI_Type CanPCI;

// configure FF
  CanTp_State = CAN_TP_ON;
  RESET_FAKE(PduR_CanTpStartOfReception);
  RESET_FAKE(PduR_CanTpCopyRxData); 
  RESET_FAKE(PduR_CanTpRxIndication);


  RESET_FAKE(CanIf_Transmit);
      
  Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
  SET_RETURN_SEQ( CanIf_Transmit, RetVal, 10);

// PDU Router FF
  PduLengthType buffSize_array_local[10] = {7,1,7,7,7,7,7,7,9,0};
  PduR_CanTpStartOfReception_buffSize_array = buffSize_array_local;
  PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    
  BufReq_ReturnType PduR_CanTpStartOfReception_ReturnVals[10] = { BUFREQ_OK, BUFREQ_OK, BUFREQ_OVFL, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK}; 
  BufReq_ReturnType PduR_CanTpCopyRxData_ReturnVals[10] =       { BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK}; 
  SET_RETURN_SEQ(PduR_CanTpStartOfReception, PduR_CanTpStartOfReception_ReturnVals, 6);
  PduR_CanTpStartOfReception_fake.custom_fake =  PduR_CanTpStartOfReception_FF;

  SET_RETURN_SEQ(PduR_CanTpCopyRxData, PduR_CanTpCopyRxData_ReturnVals, 4);
  PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;

// set CanTp internal variables

// testujemy scenariusz ze strony 62
/*

  TEST CASE 1

  Reception of FLOW CONTROL when wait

*/

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;
  uint8 payload[8] = "kasztan"; 

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

  /*TEST_CHECK(PduR_CanTpStartOfReception_fake.arg2_val == 7); // sprawdz argumenty

  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduLength == 7);

  //TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[i] == payload[i]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[0] == payload[0]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[1] == payload[1]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[2] == payload[2]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[3] == payload[3]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[4] == payload[4]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[5] == payload[5]);
  TEST_CHECK(PduR_CanTpCopyRxData_fake.arg1_val->SduDataPtr[6] == payload[6]);
  

  TEST_CHECK(CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT); // calling function with this argument shouldnt change internal state
  TEST_CHECK(PduR_CanTpStartOfReception_fake.call_count == 1); // this funtions shulod be called
  TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 1); // this functio shlud be called
  TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpRxIndication_fake.arg1_val == E_OK);
*/

/*

  TEST CASE 2

  Reception of FLOW CONTROL when processing suspended
*/

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );

/*

  TEST CASE 3

Reception of FLOW CONTROL when processing suspended

*/

  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;

  CanPCI.frame_type = FC;
  CanPCI.FS = FC_WAIT;
  CanPCI.BS = 0;
  CanPCI.ST = DEFAULT_ST;

  CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payload); // ta funkcja była wczesniej przetestowana to wolna nam jej uzyc
  CanTp_RxIndication (RxPduId, &PduInfoPtr );



  /*
    TEST CASE 4 
    INVALID FRAME RECEPTION
  */
   CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;

  CanTp_RxIndication (RxPduId, NULL );

  /*
    TEST CASE 5 
    INVALID FRAME RECEPTION
  */
   CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;

  CanTp_RxIndication (RxPduId, NULL );

    /*
    TEST CASE 6 
    INVALID FRAME RECEPTION
  */
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_RxIndication (RxPduId, NULL );

  /*
    TEST CASE 7 
    RECEPTION OF FRAME WHEN TP IF OFF
  */
 CanTp_State = CAN_TP_OFF;
  CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
  CanTp_RxIndication (RxPduId, NULL );

}
/*
  Lista testów - wpisz tutaj wszystkie funkcje które mają być wykonane jako testy.
*/
TEST_LIST = {
    { "Test_Of_PduR_CanTpCopyTxData_FF",Test_Of_PduR_CanTpCopyTxData_FF},
    { "Test_Of_CanTp_FrameCheckType", Test_Of_CanTp_FrameCheckType },   /* Format to { "nazwa testu", nazwa_funkcji } */
    { "Test_Of_CanTp_PrepareSegmenetedFrame", Test_Of_CanTp_PrepareSegmenetedFrame },
    { "Test_Of_Custom_FFs", Test_Of_Custom_FFs},
    { "Test_Of_CanTp_Calculate_Available_Blocks", Test_Of_CanTp_Calculate_Available_Blocks},
    { "Test_Of_CanTp_SendFlowControl", Test_Of_CanTp_SendFlowControl},
    { "Test_Of_Main", Test_Of_Main},
    { "Test_Of_CanTp_set_blocks_to_next_cts", Test_Of_CanTp_set_blocks_to_next_cts},
    { "Test_Of_CanTp_Resume", Test_Of_CanTp_Resume},
    { "Test_Of_CanTp_Reset_Rx_State_Variables", Test_Of_CanTp_Reset_Rx_State_Variables},
    { "Test_Of_CanTp_RxIndication_SF", Test_Of_CanTp_RxIndication_SF},
    { "Test_Of_CanTp_RxIndication_FF", Test_Of_CanTp_RxIndication_FF},
    {"Test_Of_CanTp_RxIndication_FC", Test_Of_CanTp_RxIndication_FC},
    { "Test_Of_CanTp_SendSingleFrame", Test_Of_CanTp_SendSingleFrame},
    { "Test_Of_CanTp_ConsecutiveFrame", Test_Of_CanTp_SendConsecutiveFrame},
    { "Test_Of_CanTp_FirstFrame", Test_Of_CanTp_SendFirstFrame},
    { "Test_Of_CanTp_RxIndication_CF", Test_Of_CanTp_RxIndication_CF},
    {"TestOf_CanTp_ResetTxStateVariables", TestOf_CanTp_ResetTxStateVariables},
    {"TestOf_CanTp_Transmit", TestOf_CanTp_Transmit},
    {"TestOf_CanTp_SendNextCF", TestOf_CanTp_SendNextCF},
    {"Test_Of_CanTp_FlowControlReception", Test_Of_CanTp_FlowControlReception},
    {"Test_Of_CanTp_TxConfirmation",Test_Of_CanTp_TxConfirmation},
    {"Test_Of_CanTp_Init", Test_Of_CanTp_Init},
    {" Test_Of_CanTp_Shutdown",  Test_Of_CanTp_Shutdown},
    {" Test_Of_CanTp_CancelTransmit",  Test_Of_CanTp_CancelTransmit},
    {" Test_Of_CanTp_CancelReceive", Test_Of_CanTp_CancelReceive},
    //{ "Test_Of_PduR_CanTpStartOfReception" , Test_Of_PduR_CanTpStartOfReception},
    { NULL, NULL }                                      /* To musi być na końcu */
};

