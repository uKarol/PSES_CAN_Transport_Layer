/**===================================================================================================================*\
  @file CanTp.c

  @brief Can Transport Layer
  
  Implementation of Can Transport Layer

  @see CanTp.pdf
\*====================================================================================================================*/

/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "CanTp.h"
#include "CanIf.h"
#include "PdurCanTp.h"
/*====================================================================================================================*\
    Makra lokalne
\*====================================================================================================================*/
#define NE_NULL_PTR( ptr ) (ptr != NULL)

//#define GET_PCI_ID(  )

#define SF_ID 0x0 // single frame
#define FF_ID 0x1 // first frame
#define CF_ID 0x2 // consecutive frame
#define FC_ID 0x3 // flow control frame

#define DEFAULT_ST 0


/*====================================================================================================================*\
    Typy lokalne
\*====================================================================================================================*/
typedef enum {
    CAN_TP_ON, 
    CAN_TP_OFF
}CanTpState_Type;

typedef enum {
    CANTP_RX_WAIT,
    CANTP_RX_PROCESSING,
    CANTP_END_OF_BLOCK
} CanTp_RxState_Type;

typedef enum {
    CANTP_TX_WAIT,                      // wait for first frame or single frame 
    CANTP_TX_PROCESSING,                // wait for consecutive frame 
    CANTP_TX_PROCESSING_SUSPENDED       // wait for buffer - UWAGA STAN MOZE OKAZAC SIE NIEPOTRZEBNY
} CanTp_TxState_Type;

typedef enum{
    FC_OVFLW = 0,
    FC_WAIT = 1,
    FC_CTS = 2
} FlowControlStatus_type;
typedef enum {
    SF = 0, // signel frame
    FF = 1, // first frame
    CF = 2, // consecutive frame
    FC = 3, // flow control frame
    UNKNOWN = 4
    } frame_type_t;

typedef struct{
    frame_type_t frame_type;
    uint32 frame_lenght; 
    uint8 SN; // Sequence Nubmer - only consecutive frame
    uint8 BS; // Block Size - only flow control frame 
    uint8 FS; // Flow Status -only flow control frame 
    uint8 ST; // Sepsration Time - only flow control frame
} CanPCI_Type;

// struc contains all rx state variables 
typedef struct{
    uint16 message_length;              //length of message
    uint16 expected_CF_SN;              //expected number of CF
    uint16 sended_bytes;                // number of sended bytes 
    uint16 last_CF_length;              // length of last CF
    CanTp_RxState_Type CanTp_RxState;   // state (unused)
    PduIdType CanTp_Current_RxId;       // id of currently processed message
} CanTp_StateVariables_type;


/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/
CanTpState_Type CanTp_State; 
CanTp_TxState_Type CanTp_TxState;
CanTp_RxState_Type CanTp_RxState;

CanTp_StateVariables_type CanTp_StateVariables;

/*====================================================================================================================*\
    Zmienne lokalne (statyczne)
\*====================================================================================================================*/

/*====================================================================================================================*\
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/
static Std_ReturnType CanTp_GetPCI( const PduInfoType* can_data, CanPCI_Type* CanFrameInfo);
static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload);

static void CanTp_Reset_Rx_State_Variables();
static void CanTp_Resume();

//TODO
static Std_ReturnType CanTp_SendFlowControl( uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime );
static void CanTp_N_Br_Start();
static void CanTp_N_Br_Stop();
static void CanTp_N_Cr_Start();
static void CanTp_N_Cr_Stop();


//static Std_ReturnType Can_Tp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload);
/*====================================================================================================================*\
    Kod globalnych funkcji inline i makr funkcyjnych
\*====================================================================================================================*/

/*====================================================================================================================*\
    Kod funkcji
\*====================================================================================================================*/

/**
  @brief CanTp_Init

  This function initializes the CanTp module.  

  Parameters (in) 
        CfgPtr - Pointer to the CanTp post-build configuration data.

  Parameters (inout): None

  Parameters (out): None

  Return value None

*/
void CanTp_Init ( const CanTp_ConfigType* CfgPtr );


/**
  @brief CanTp_ GetVersionInfo

  This function returns the version information of the CanTp module. 

  Parameters (in) 
        versioninfo - Indicator as to where to store the version information of this module.

  Parameters (inout): None

  Parameters (out): None

  Return value None

*/
void CanTp_GetVersionInfo ( Std_VersionInfoType* versioninfo );

/**
  @brief CanTp_Shutdown

  This function is called to shutdown the CanTp module. 

  Parameters (in): None

  Parameters (inout): None

  Parameters (out): None

  Return value None

*/
void CanTp_Shutdown ( void );

/**
  @brief CanTp_Transmit

  Requests transmission of a PDU.

  Parameters (in): 
  TxPduId - Identifier of the PDU to be transmitted
  PduInfoPtr - Length of and pointer to the PDU data and pointer to MetaData.

  Parameters (inout): None

  Parameters (out): None

  Return value: Std_ReturnType
    E_OK: Transmit request has been accepted. 
    E_NOT_OK: Transmit request has not been accepted.

*/
Std_ReturnType CanTp_Transmit ( PduIdType TxPduId, const PduInfoType* PduInfoPtr );

/**
  @brief CanTp_CancelReceive

  Requests cancellation of an ongoing reception of a PDU in a lower layer transport protocol module.
  Parameters (in): 
   TxPduId - Identification of the PDU to be cancelled.

  Parameters (inout): None

  Parameters (out): None

  Return value: Std_ReturnType
    E_OK: Cancellation was executed successfully by the destination module. 
    E_NOT_OK: Cancellation was rejected by the destination module.

*/
Std_ReturnType CanTp_CancelTransmit ( PduIdType TxPduId );

/**
  @brief CanTp_CancelReceive

Requests cancellation of an ongoing reception of a PDU in a lower layer transport protocol module.  Parameters (in): 
   RxPduId - Identification of the PDU to be cancelled.

  Parameters (inout): None

  Parameters (out): None

  Return value: Std_ReturnType
    E_OK: Cancellation was executed successfully by the destination module. 
    E_NOT_OK: Cancellation was rejected by the destination module.

*/
Std_ReturnType CanTp_CancelReceive ( PduIdType RxPduId );

Std_ReturnType CanTp_ChangeParameter ( PduIdType id, TPParameterType parameter, uint16 value );

Std_ReturnType CanTp_ReadParameter ( PduIdType id, TPParameterType parameter, uint16* value );

void CanTp_MainFunction ( void ){


    /* TO DO:

    funkcja ma odpowiadać za timeouty oraz za obslugę eventów od timera 

    mamy łącznie 3 timery w przypadku odbierania:

    N_Br - odmierza czas między ramkami FlowControl 
    N_Cr - odmierza czas między ramkami CF

    timery N_Br i N_Cr mają dwa stany:
    - <timer>_active 
    - <timer>_not_active

    jeżeli timer N_Br jest w stanie N_Br_ACTIVE to wartość licznika ma być inkrementowana z każdym wywołaniem funkcji CanTp_MainFunction 
    jeżeli timer N_Br jest w stanie N_Br_NOT_ACVITE to wartość licznika ma się nie zmieniać 

    dokładnie tak samo ma być w przypadku timera N_Cr 

    Generowane Eventy:

    Jeżeli N_Br jest aktywny to:

    -z każdym wywołaniem funkcji CanTp_MainFunction nalezy wywołać funkcję PduR_CanTpCopyRxData() 
    oraz sprawdzać jej parametry, wymaganie: [SWS_CanTp_00222] 

    -jeżeli funkcja PduR_CanTpCopyRxData() zwróci bufor równy 7 lub więcej, ewentualnie mniejszy niż 7,
    ale większy lub równy liczbie bajtów pozostałych do wysłania,
     to należy wysłać ramkę FlowControl z parametrem CTS [SWS_CanTp_00224], w tym wypadku 
     nalezy takze zatrzymac timer (przejsc do stanu not_active) oraz wyzerowac jego licznik, a także aktywować 
     timer N_Cr

    -jeżeli timer doliczy do wartości N_Br timeout, należy inkrementować to należy wyslać ramkę FlowControl 
    z argumentem WAIT oraz inkrementować licznik wysłanych ramek WAIT [SWS_CanTp_00341]

    -jeżeli licznik wysłanych ramek FlowControl WAIT się przepełni to mamy problem, musimy zgłosić problem
    wywołujemy PduR_CanTpRxIndication ( RxPduId, E_NOT_OK) i na tym koniec transmisji [SWS_CanTp_00223], nalezy
    zatrzymac timer oraz zresetowac jego licznik 

    Jeżeli N_Cr jest aktywny to:

    -inkrementujemy go po każdym wywołaniu funkcji cantp_main

    - jeżeli wystąpi N_Cr_timeout, to wywołujemy PduR_CanTpRxIndication ( RxPduId, E_NOT_OK) i na tym koniec transmisji [SWS_CanTp_00223], nalezy
    zatrzymac timer oraz zresetowac jego licznik [SWS_CanTp_00313]

    To oczywiście nie wszystko, ale jest to na razie priorytetem, kolejne elementy zostaną dołożone później 

    poza tym do napisania funkcje (wszystkie typu void):

    -startująca N_Br
    -zatrzymująca i resetująca N_Br 

    -startująca C_Br
    -zatrzymująca i resetująca C_Br 


    */

} 

// callbacks

void CanTp_TxConfirmation ( PduIdType TxPduId, Std_ReturnType result );

void CanTp_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr ){

 /*  
    DZIADOSTWO, NIE ZWRACAC NA TO UWAGI
    */
    CanPCI_Type Can_PCI;            // PCI extracted from frame
    PduLengthType buffer_size;      // use when calling PduR callbacks
    BufReq_ReturnType Buf_Status;   // use when calling PduR callbacks
    Std_ReturnType retval = E_OK;   // return 
    PduInfoType Extracted_Data;     // Payload extracted from PDU
    PduIdType SegmentedFrameId = 0; // Id of frame 
    uint8 temp_data[8];             // array for temp payload
    uint16 current_block_size;      // currently processes block size (will be global)
    uint16 message_size;            // size of unsegmented message
    uint16 last_CF_size;            // size of last CF frame
    uint16 bytes_sent;              // number of sent bytes

    if( CanTp_RxState == CANTP_RX_WAIT){
        
        SegmentedFrameId = RxPduId; // setting of segmented frame id possible only in waiting state

        CanTp_GetPCI( PduInfoPtr, &Can_PCI );

        //CanTp_AssertPCI( &Can_PCI ); conceptual

        // transmisja danych jeszcze nie wystartowała 
        // pierwsza ramka musi być SF albo FF, w przeciwnym wypadku mamy błąd
         
        if( Can_PCI.frame_type == FF ){

            /*
            [SWS_CanTp_00079]
            ⌈When receiving an SF or an FF N-PDU, the CanTp module shall notify the upper layer (PDU Router)
            about this reception using the PduR_CanTpStartOfReception function. 

            [SWS_CanTp_00329] 
            CanTp shall provide the content of the FF/SF to PduR
            */
            Buf_Status = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI.frame_lenght, &buffer_size);
            /*

                [SWS_CanTp_00333] ⌈ When CanTp_RxIndication is called for a CF on a generic connection (N-PDU with MetaData), 
                the CanTp module shall check the addressing information contained in the MetaData of the N-PDU against the stored values from the FF. ⌋ ()
                [SWS_CanTp_00080] ⌈The available Rx buffer size is reported to the CanTp in the output pointer parameter of the PduR_CanTpStartOfReception() service. 
                The available Rx buffer can be smaller than the expected N-SDU data length. 
            */
            if( Buf_Status == BUFREQ_OK ) {
                
                // calculate block size to be send by FLOW_CONTROL
                CanTp_StateVariables.message_length = Can_PCI.frame_lenght;
                if( Can_PCI.frame_lenght <= 4095 ){
                    CanTp_StateVariables.last_CF_length = (Can_PCI.frame_lenght - 6) % 7; 
                } 
                // longer frames
                else{
                    CanTp_StateVariables.last_CF_length = (Can_PCI.frame_lenght - 2) % 7; 
                }
                
                current_block_size = Can_PCI.frame_lenght / 7; // czy musie byc wystarczajaco duzo miejsca w buforze na FF?

                // send payload of FF
                if( current_block_size > 0){    
                    CanTp_SendFlowControl( current_block_size, FC_CTS, DEFAULT_ST );
                    Extracted_Data.SduLength = Can_PCI.frame_lenght;
                    // shorter frames
                    if( Can_PCI.frame_lenght <= 4095 ){
                        Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+2);
                    } 
                    // longer frames
                    else{
                        Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+6);
                    } 
                    Buf_Status = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);                   
                }
                else{
                    CanTp_SendFlowControl( current_block_size, FC_WAIT, DEFAULT_ST );              
                }

                CanTp_RxState = CANTP_RX_PROCESSING; // After Successfull Reception of FF, wait for Consecutive Frames
            }
            else if ( Buf_Status == BUFREQ_OVFL ){
                /*
                [SWS_CanTp_00318] ⌈After the reception of a First Frame, if the function PduR_CanTpStartOfReception()returns BUFREQ_E_OVFL to the CanTp module, 
                the CanTp module shall send a Flow Control N-PDU with overflow status (FC(OVFLW)) and abort the N-SDU reception. */
                CanTp_SendFlowControl( current_block_size, FC_OVFLW, DEFAULT_ST );
                CanTp_Reset_Rx_State_Variables();
                CanTp_RxState = CANTP_RX_WAIT;
            }
            else {

            }

        }
        else if( Can_PCI.frame_type == SF ){
            CanTp_RxState = CANTP_RX_WAIT;
            Buf_Status = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI.frame_lenght, &buffer_size);

            if( (Buf_Status == BUFREQ_OK)  ){
                
                if(buffer_size >= Can_PCI.frame_lenght){

                    Extracted_Data.SduLength = Can_PCI.frame_lenght;
                    Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
                    Buf_Status = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);

                    if(Buf_Status == BUFREQ_OK ){
                        PduR_CanTpRxIndication ( RxPduId, E_OK);
                    }
                    else{

                        /*
                            UWAGA TEGO SAM NIE JESTEM PEWIEN ALE WYDAJE MI SIE TO LOGICZNE
                            AUTOSAR PRECYZUJE CO SIE STANIE GDY CopyData ZWROCI NOT_OK ALE
                            DLA CONSECUTIVE FRAME, BRAK JASNOSCI CO SIE MOZE STAC DLA SF
                        */
                        PduR_CanTpRxIndication ( RxPduId, E_NOT_OK);
                    }
                }
                else{

                /*
                    [SWS_CanTp_00339] ⌈After the reception of a First Frame or Single Frame, 
                    if the function PduR_CanTpStartOfReception() returns BUFREQ_OK with a 
                    smaller available buffer size than needed for the already received data, 
                    the CanTp module shall abort the reception of the N-SDU and call 
                    PduR_CanTpRxIndication() with the result E_NOT_OK. ⌋ ( )
                */
                    PduR_CanTpRxIndication ( RxPduId, E_NOT_OK);
                }
            }
            else{
            /*
                [SWS_CanTp_00081] ⌈After the reception of a First Frame or Single Frame, 
                if the function PduR_CanTpStartOfReception()returns BUFREQ_E_NOT_OK to 
                the CanTp module, the CanTp module shall abort the reception of this N-SDU. 
                No Flow Control will be sent and PduR_CanTpRxIndication() will not be called 
                in this case. ⌋ ( ) 
            */

            /*
                [SWS_CanTp_00353]⌈After the reception of a Single Frame, if the function 
                PduR_CanTpStartOfReception()returns BUFREQ_E_OVFL to the CanTp module,
                the CanTp module shall abort the N-SDU reception.⌋()
            */
            }           

        } 
        else if( Can_PCI.frame_type == FF ){
            /* accept flow control if Tranmitter is waiting for it
            
             */

        }
        else
        {
            // in this state, CanTP expect only SF or FF, otherwise it is an error
            CanTp_RxState = CANTP_RX_WAIT; 
            retval = E_NOT_OK;
        } 
    }

    else if( CanTp_RxState == CANTP_RX_PROCESSING){
        
        /*
        in this state process only CF and FF, other frames are errors

        */
       if( Can_PCI.frame_type == CF ){

       }
       else if( Can_PCI.frame_type == FF ){

       }
       else {
           // uwaga na razie niejasne 
       }
    }
    /* uwaga na razie nie jestem pewny czy ten stan ma sens, mozliwe ze zostanie skasowany
        jest to stan w ktorym CanTp czeka aż zwolni się bufor 
        zasadniczo nie powinny przychodzić wtedy żadne ramki z wyjątkie flow control 
        nadejscie jakiejkolwiek innej ramki traktowane jest jako blad, ale uwaga 
        nie jest jasne jak zachowac sie na wypadek bledu (niestety)
    */
    else if( CanTp_RxState == CANTP_TX_PROCESSING_SUSPENDED){
            
        if( Can_PCI.frame_type == FF ){

       }
       else {
           // uwaga na razie niejasne 
       }


    }


}

static Std_ReturnType CanTp_GetPCI( const PduInfoType* can_data, CanPCI_Type* CanFrameInfo){
    Std_ReturnType ret = E_OK;

    if( NE_NULL_PTR(can_data) && NE_NULL_PTR(CanFrameInfo) && ( NE_NULL_PTR(can_data->SduDataPtr) ) ){

        CanFrameInfo->frame_type = UNKNOWN;
        CanFrameInfo->frame_lenght = 0;
        CanFrameInfo->BS = 0;
        CanFrameInfo->FS = 0;
        CanFrameInfo->ST = 0;
        CanFrameInfo->SN = 0;

        switch( (can_data->SduDataPtr[0]) >> 4 ){
            case SF_ID:
                CanFrameInfo->frame_type = SF;
                CanFrameInfo->frame_lenght = can_data->SduDataPtr[0];
            break;

            case FF_ID:
                CanFrameInfo->frame_type = FF;
                if( (can_data->SduDataPtr[0] & 0x0F) | can_data->SduDataPtr[1] ) {
                    CanFrameInfo->frame_lenght =  can_data->SduDataPtr[0] & 0x0F;
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[1]; 
                }
                else{
                    CanFrameInfo->frame_lenght =  can_data->SduDataPtr[2];
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[3]; 
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[4];
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[5];
                }
            break;

            case CF_ID:
                CanFrameInfo->frame_type = CF;
                CanFrameInfo->SN= (can_data->SduDataPtr[0] & 0x0F );
            break;

            case FC_ID:
                CanFrameInfo->frame_type = FC;
                CanFrameInfo->FS = can_data->SduDataPtr[0] & 0x0F; 
                CanFrameInfo->BS = can_data->SduDataPtr[1]; 
                CanFrameInfo->ST = can_data->SduDataPtr[2]; 
            break;

            default:
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }
    return ret;
}


static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload){

    /* TODO
        Przygotować funkcję i napisać do niej UT

        Funkcja ma przyjmować strukturę PCI, Wskaźnik na ramkę, i tablice z zawartością (payload)    

        Wejscie:
        CanPCI - strukturka z metadanymi 
        CanPdu_Info - docelowa ramka
        Can_payload - treść któą włożymy do ramki (tylko Consecutive i Single Frame) - może mieć max 7 bajtów 

        Składanie ramki ma być zgodne ze specyfikacją autosarową:
        https://drive.google.com/drive/folders/1lAoLr9j1brjr8pam5zddvbTAf297gXCo?fbclid=IwAR01armEu1UxoFRvtUuRi9BI2D4Ep8igP4bWNgwrwoyXUae7FVnl84VJ41I

        CanTransporLayer, str 28

    */

    Std_ReturnType ret = E_OK;

    if( NE_NULL_PTR(CanPCI) && NE_NULL_PTR(CanPdu_Info) && NE_NULL_PTR(Can_payload) ){  
        switch(CanPCI->frame_type){
            case SF:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = SF_ID << 4;
        
                if(CanPCI->frame_lenght <= 7){
                 //   *(CanPdu_Info->SduDataPtr) = 0x0F | CanPCI->frame_lenght; // panie kto tu tak spierdolil
                    *(CanPdu_Info->SduDataPtr) = 0x0F & CanPCI->frame_lenght; 
                    for(uint8_t i = 0; i < CanPCI->frame_lenght; i++){
                        *(CanPdu_Info->SduDataPtr + (i + 1)) = *(Can_payload + i);
                    }  
                }
                else{
                    ret = E_NOT_OK; //Za duża ilość danych
                }
            break;
            case CF: 
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CF_ID << 4;
                if(CanPCI->SN < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->SN);
                }
                else{
                    ret = E_NOT_OK; 
                }

            break;
            case FF:    
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = FF_ID << 4;

                if(CanPCI->frame_lenght <= 4095){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & (CanPCI->frame_lenght >> 8));
                    *(CanPdu_Info->SduDataPtr + 1) = (0xFF & (CanPCI->frame_lenght));

                    *(CanPdu_Info->SduDataPtr + 2) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 3) = *(Can_payload + 1);
                    *(CanPdu_Info->SduDataPtr + 4) = *(Can_payload + 2);
                    *(CanPdu_Info->SduDataPtr + 5) = *(Can_payload + 3);
                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload + 4);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 5);
                }
                else{
                    *(CanPdu_Info->SduDataPtr + 1) = 0;

                    *(CanPdu_Info->SduDataPtr + 2) = (CanPCI->frame_lenght >> 24) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 3) = (CanPCI->frame_lenght >> 16) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 4) = (CanPCI->frame_lenght >> 8) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 5) = (CanPCI->frame_lenght >> 0) & 0xFF;

                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 1);
                }
            break;
            case FC:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = FC_ID << 4;

                if(CanPCI->FS < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->FS);
                    *(CanPdu_Info->SduDataPtr + 1) = CanPCI->BS;
                    *(CanPdu_Info->SduDataPtr + 2) = CanPCI->ST;
                }
                else{
                    ret = E_NOT_OK;
                }
            break;
            default:
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }

    return ret;
}

static Std_ReturnType CanTp_SendFlowControl( uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime ){

    /*
        TO DO
        Napisac funkcje, ktora wysyla ramke FlowControl o zadanych parametrach.

        -funkcja przyjmuje BlockSize
        -FC_Status
        -Separation Time (na razie nieistotna wartosc)

        Funkcja ma za zadanie poskladać ramkę przy użyciu 
        CanTp_PrepareSegmenetedFrame

        a następnie ją wysłać przy użyciu 
        CanIf_Transmit()

        Jeżeli którakolwiek z wyżej wymienionych funkcji zwróci E_NOT_OK, należy przerwać działanie funkcji i też zwrócić E_NOT_OK
        Jeżeli wszystko przejdzie bez bledow, należy zwrocick E_OK




    */


    return E_OK;
}

// reset all state variables 

static void CanTp_Reset_Rx_State_Variables(){
    CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
    CanTp_StateVariables.expected_CF_SN = 0;
    CanTp_StateVariables.last_CF_length = 0;
    CanTp_StateVariables.message_length = 0;
    CanTp_StateVariables.sended_bytes = 0;
}

// resume CanTp

static void CanTp_Resume(){
    CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
}