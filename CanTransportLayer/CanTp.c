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
#include "CanTp_Timer.h"
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

#define FC_WAIT_FRAME_CTR_MAX 10

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
    CANTP_RX_PROCESSING_SUSPENDED
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
    CanTp_RxState_Type CanTp_RxState;   // state
    PduIdType CanTp_Current_RxId;       // id of currently processed message
    uint8 blocks_to_next_cts; 
} CanTp_StateVariables_type;

typedef struct{
    CanTp_TxState_Type Cantp_TxState;   // stan transmitera
    PduIdType CanTp_Current_TxId;       // id procesowanej wiadomosci 
    uint16 sent_bytes;                  // nuber of sent bytes 
    uint16 message_legth;               // length of message 
    uint8_t next_SN;
    uint16 blocks_to_fc;                // number of block between flow control
    /*
        uwaga a ostatnie pole, może być nie do końca oczywiste 
        o co tu chodzi?
        1 receiver ma ograniczony bufor 
        2 ramka FlowControl zawiera informacje ile bloków może pomieścić receiver 
        3 jeżeli otrzymamy FlowControl , to dobieramy się do informacji o blokach, które
        mogą być odebrane i umieszczamy je w ostatnim polu
        4 po wysłaniu ramki ilość bloków (ramek) dekrementujemy o 1 
        5 jeżeli wartość wynosi 0, to oczekujemy kolenej FlowControl, no chyba że wszystkie 
        bajty dlugiej wiadomości zostaly wysłane 
        6 jeżeli nie, to wróć do punktu 3
        

    */
} CanTp_Tx_StateVariables_type;

            


/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/
CanTpState_Type CanTp_State; 
// transmitter state variables
CanTp_Tx_StateVariables_type CanTp_Tx_StateVariables;
//CanTp_RxState_Type CanTp_RxState;

CanTp_StateVariables_type CanTp_StateVariables;

CanTp_Timer_type N_Ar_timer = {TIMER_NOT_ACTIVE, 0, N_AR_TIMEOUT_VAL};
CanTp_Timer_type N_Br_timer = {TIMER_NOT_ACTIVE, 0, N_BR_TIMEOUT_VAL};
CanTp_Timer_type N_Cr_timer = {TIMER_NOT_ACTIVE, 0, N_CR_TIMEOUT_VAL};

CanTp_Timer_type N_As_timer = {TIMER_NOT_ACTIVE, 0, N_AS_TIMEOUT_VAL};
CanTp_Timer_type N_Bs_timer = {TIMER_NOT_ACTIVE, 0, N_BS_TIMEOUT_VAL};
CanTp_Timer_type N_Cs_timer = {TIMER_NOT_ACTIVE, 0, N_CS_TIMEOUT_VAL};

uint32 FC_Wait_frame_ctr;

/*====================================================================================================================*\
    Zmienne lokalne (statyczne)
\*====================================================================================================================*/

/*====================================================================================================================*\
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/
static Std_ReturnType CanTp_GetPCI( const PduInfoType* can_data, CanPCI_Type* CanFrameInfo);
static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload);

static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *Can_PCI);
static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr);
static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr);
static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *Can_PCI);

static void CanTp_Reset_Rx_State_Variables();
static void CanTp_Resume();
static uint16 CanTp_Calculate_Available_Blocks( uint16 buffer_size );

static void CanTp_set_blocks_to_next_cts( uint8 blocks );

static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 SN, uint8* payload, uint32 size);
static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 message_lenght);
static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* payload, uint32 size);

static void CanTp_ResetTxStateVariables(void);

//TODO
static Std_ReturnType CanTp_SendFlowControl( PduIdType ID, uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime );

static void CanTp_SendNextCF();



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
Std_ReturnType CanTp_Transmit ( PduIdType TxPduId, const PduInfoType* PduInfoPtr ){

    BufReq_ReturnType BufReq_State;
    PduLengthType Pdu_Len;
    Std_ReturnType ret = E_OK;
    
    PduInfoType Temp_Pdu;
    uint8_t payload[8];
    Temp_Pdu.SduDataPtr = payload;
    Temp_Pdu.MetaDataPtr = NULL;
    

    if( CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_WAIT){
        if(PduInfoPtr->SduLength < 8){
            //Send signle frame
            Temp_Pdu.SduLength = PduInfoPtr->SduLength;
            BufReq_State = PduR_CanTpCopyTxData(TxPduId, &Temp_Pdu, NULL, &Pdu_Len);
            if(BufReq_State == BUFREQ_OK){

               ret = CanTp_SendSingleFrame(TxPduId, Temp_Pdu.SduDataPtr, PduInfoPtr->SduLength );

            }
            else if(BufReq_State == BUFREQ_E_NOT_OK){
                CanTp_ResetTxStateVariables();  
                PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
                ret = E_NOT_OK;
            }
            else {
                //Start N_Cs timer
                CanTp_TimerStart(&N_Cs_timer);
                ret = E_OK;
            }
        }
        else{
            //Send First Frame
            if(CanTp_SendFirstFrame(TxPduId, PduInfoPtr->SduLength) == E_OK){
                //Transmiter przechodzi w stan Processing_suspended
                CanTp_Tx_StateVariables.Cantp_TxState = TxPduId;
                CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
                CanTp_Tx_StateVariables.message_legth = PduInfoPtr->SduLength;
                CanTp_Tx_StateVariables.sent_bytes = 0;
                ret = E_OK;
            }
            else{
                //TODO: Obsługa błedu funkcji SendFirstFrame 
                /*  Przrwać transmisję: 
                CanTp_ResetTxStateVariables();  
                PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
                 Czy wystarczy tylko zwrócić E_NOT_OK?  */ 
                ret = E_NOT_OK;
            }
        }
    }
    else{
        /*[SWS_CanTp_00123] ⌈If the configured transmit connection channel is in use 
        (state CANTP_TX_PROCESSING), the CanTp module shall reject new transmission 
        requests linked to this channel. To reject a transmission, CanTp returns 
        E_NOT_OK when the upper layer asks for a transmission with the 
        CanTp_Transmit() function.⌋ (SRS_Can_01066) */
        ret = E_NOT_OK;
    }
    return ret;
}

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
    static boolean N_Ar_timeout, N_Br_timeout, N_Cr_timeout;
    static PduLengthType PduLenght;
    static const PduInfoType   PduInfoConst = {NULL, NULL, 0};
  //  static PduIdType RxPduId;
    uint16 block_size;
    uint8 separation_time;
    BufReq_ReturnType BufReq_State; 
    


    /* TO DO:

    -zadeklarować zmienne stanu dla timerów
    -zadeklarować timeouty za pomocą definów
    -zadeklarować liczniki jako zmienne globalne 

    funkcja ma odpowiadać za timeouty oraz za obslugę eventów od timera 

    mamy łącznie 3 timery w przypadku odbierania:

    N_Br - odmierza czas między ramkami FlowControl 
    N_Cr - odmierza czas między ramkami CF
    N_Ar - odmierza czas od wyslania ramki do potwierdzenia wyslania przez Driver

    timery N_Br i N_Cr mają dwa stany:
    - <timer>_active 
    - <timer>_not_active

    jeżeli timer N_Br jest w stanie N_Br_ACTIVE to wartość licznika ma być inkrementowana z każdym wywołaniem funkcji CanTp_MainFunction 
    jeżeli timer N_Br jest w stanie N_Br_NOT_ACVITE to wartość licznika ma się nie zmieniać 

    dokładnie tak samo ma być w przypadku timera N_Cr 

    Generowane Eventy:

    Jeżeli N_Br jest aktywny to:

    -z każdym wywołaniem funkcji CanTp_MainFunction nalezy wywołać funkcję PduR_CanTpCopyRxData() 
    oraz sprawdzać jej parametry, oraz wysyłać funkcję CanTp_Calculate_Available_Blocks( uint16 buffer_size ), która obliczy ilość bloków wymaganie: [SWS_CanTp_00222] 

    -jeżeli funkcja CanTp_Calculate_Available_Blocks() zwroci wartość większą od 0 
     to należy wysłać ramkę FlowControl z parametrem CTS [SWS_CanTp_00224], w tym wypadku 
     nalezy takze zatrzymac timer (przejsc do stanu not_active) oraz wyzerowac jego licznik, a także aktywować 
     timer N_Cr

    -jeżeli timer doliczy do wartości N_Br timeout, należy inkrementować to należy wyslać ramkę FlowControl 
    z argumentem WAIT oraz inkrementować licznik wysłanych ramek WAIT [SWS_CanTp_00341]

    -jeżeli licznik wysłanych ramek FlowControl WAIT się przepełni to mamy problem, musimy zgłosić problem
    wywołujemy PduR_CanTpRxIndication ( RxPduId, E_NOT_OK) i na tym koniec transmisji [SWS_CanTp_00223], nalezy
    zatrzymac timer oraz zresetowac jego licznik 

    */

    // Inkrementacja wszystkich aktywnych timerów
    CanTp_TimerTick(&N_Ar_timer);
    CanTp_TimerTick(&N_Br_timer);
    CanTp_TimerTick(&N_Cr_timer);


   if(N_Br_timer.state == TIMER_ACTIVE){

// 1   - Sprawdzenie i obsługa wartości zwronej PduR_CanTpCopyRxData
        //[SWS_CanTp_00222] 
       BufReq_State = PduR_CanTpCopyRxData(CanTp_StateVariables.CanTp_Current_RxId, &PduInfoConst, &PduLenght);

       if(BufReq_State == BUFREQ_E_NOT_OK){
           // [SWS_CanTp_00271]
           PduR_CanTpRxIndication(CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
       }
       else{
            block_size = CanTp_Calculate_Available_Blocks(PduLenght);
            if(block_size > 0){

// 3            Wywolanie funkcji CanTp_Set_blocks... i CanTp_Resume jeżeli block_size > 0
                CanTp_set_blocks_to_next_cts(block_size);
                CanTp_Resume();


                //[SWS_CanTp_00224]
                if(CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, block_size, FC_CTS, separation_time) == E_NOT_OK){
 // 4   - resetowanie recievera jeżeli SendFlowControl zwróci błąd                   
                    CanTp_Reset_Rx_State_Variables();
                }
                else{
                    //Zresetowanie Timera N_Br:
                    CanTp_TimerReset(&N_Br_timer); 
                }

// 5            - Usuniecie startowania timera
                //Start timera N_Cr:
                //CanTp_TimerStart(&N_Cr_timer);
           
            }
            //Obsługa timeouta
            if(CanTp_TimerTimeout(&N_Br_timer)){
                FC_Wait_frame_ctr ++;  //Inkrementacja licznika ramek WAIT 
                N_Br_timer.counter = 0;
                if(FC_Wait_frame_ctr >= FC_WAIT_FRAME_CTR_MAX){
                    // [SWS_CanTp_00223]
// 2- reset recievera  --Zbyt duza ilość ramek FC_WAIT, 
                    CanTp_Reset_Rx_State_Variables();
                    PduR_CanTpRxIndication (CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
                    //Zresetowanie Timera N_Br:
                    CanTp_TimerReset(&N_Br_timer); 
                    FC_Wait_frame_ctr = 0;
                }
                else{
                    // [SWS_CanTp_00341]
                    if(CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, block_size, FC_WAIT, separation_time) == E_NOT_OK){
// 4   - resetowanie recievera jeżeli SendFlowControl zwróci błąd
                        CanTp_Reset_Rx_State_Variables();
                    }
                }
            }
        }
   }

   //N_Cr jest w stanie aktywnym
   if(N_Cr_timer.state == TIMER_ACTIVE){
       //N_Cr zgłasza timeout 
       if(CanTp_TimerTimeout(&N_Cr_timer) == E_NOT_OK){

// 2- reset recievera  --Zbyt duza ilość ramek FC_WAIT, 
            CanTp_Reset_Rx_State_Variables();
            // [SWS_CanTp_00223]
            PduR_CanTpRxIndication(CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);

            // [SWS_CanTp_00313] 
            //Zatrzymanie i zresetowanie licznika
            CanTp_TimerReset(&N_Cr_timer);
       }
   }

   //N_Ar jest w stanie aktywnym
   if(N_Ar_timer.state == TIMER_ACTIVE){
       //N_Ar zgłasza timeout 
       if(CanTp_TimerTimeout(&N_Ar_timer) == E_NOT_OK){

// 2- reset recievera  --Zbyt duza ilość ramek FC_WAIT, 
            CanTp_Reset_Rx_State_Variables();
            // [SWS_CanTp_00223]
            PduR_CanTpRxIndication(CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);

            // [SWS_CanTp_00313] 
            //Zatrzymanie i zresetowanie licznika
            CanTp_TimerReset(&N_Ar_timer);
       }
   }
} 

// callbacks

void CanTp_TxConfirmation ( PduIdType TxPduId, Std_ReturnType result ){


    /*

        funkcja jest wywoływana przez niższą warstwę i informuje nas o statusie operacji wysyłania
        w wypadku błędu przerwać wysyłanie lub odbierania (w zależności od tego co jest aktywne)
        w przypadku receivera funkcja powinna zatrzymać timer N_Ar, o ile nie będzie błędu

   */

   // receiver  
    if( CanTp_StateVariables.CanTp_Current_RxId == TxPduId ){
        if( (CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING ) || (CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING_SUSPENDED ) ){

            if(result == E_OK){
                CanTp_TimerReset(&N_Ar_timer);   
            }    
            else{
                // w przypadku receivera tylko wysłanie FlowControl aktywuje ten timer, oznacza to, że nie powinien się on wysypać w stanie innym niż PROCESSING
                PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
                CanTp_Reset_Rx_State_Variables();
            }
        }
        else{
            /* IGNORE */
        } 
    }

    // TRASMITER 

    /*
        W PRZYPADKU TRANSMITERA MAMY PEWNOSC ŻE RAMKA DOSZŁA (O ILE ZWRÓCIŁ OK ) I MOŻNA ŁADOWAĆ KOLEJNĄ 

    */
    if( CanTp_Tx_StateVariables.CanTp_Current_TxId == TxPduId ){
        if(result == E_OK){
            // jeżeli poprzednia ramka przeszła 
            if(CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING)
            {
               CanTp_SendNextCF();               
            }
            else{
                /* olny reset timer */
            }
        }
        else{
            /*[SWS_CanTp_00355]⌈CanTp shall abort the corrensponding session, 
            when CanTp_TxConfirmation() is called with the result E_NOT_OK.⌋()*/
            PduR_CanTpTxConfirmation(CanTp_Tx_StateVariables.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTxStateVariables();

        }
    }
    else{
        // ignore unknown ID
    }

    // UNKNOWN ID WILL BE IGNORED

}

void CanTp_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr ){

    CanPCI_Type Can_PCI;            // PCI extracted from frame
    PduInfoType Extracted_Data;     // Payload extracted from PDU
    uint8 temp_data[8];             // array for temp payload


    if( CanTp_StateVariables.CanTp_RxState == CANTP_RX_WAIT){
        
      //  CanTp_StateVariables.CanTp_Current_RxId = RxPduId; // setting of segmented frame id possible only in waiting state

        CanTp_GetPCI( PduInfoPtr, &Can_PCI );

        /* transmisja danych jeszcze nie wystartowała 
           pierwsza ramka musi być SF albo FF, w przeciwnym wypadku
            wiadomość zostanie zignorowana
        */

        if( Can_PCI.frame_type == FF ){
            CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);
        }
        else if( Can_PCI.frame_type == SF ){
            CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);           
        } 
        else if( Can_PCI.frame_type == FC ){
            /* accept flow control if Tranmitter is waiting for it
             */
            CanTp_FlowControlReception(RxPduId, &Can_PCI);
        }
        else
        {
            // in this state, CanTP expect only SF or FF, FC other frames shuld vbe ignored
            CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT; 
        } 
    }

    else if( CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING){
        /*
            in this state process only CF and FC, 
            FF and CF cause abort communication and start a new one
        */
       CanTp_GetPCI( PduInfoPtr, &Can_PCI );
       // EXPECTED
       if( Can_PCI.frame_type == CF ){
           CanTp_ConsecutiveFrameReception(RxPduId, &Can_PCI, PduInfoPtr);
       }
       else if( Can_PCI.frame_type == FC ){
           CanTp_FlowControlReception(RxPduId, &Can_PCI);
       }
       // UNEXPECTED 
        /*
            [SWS_CanTp_00057] ⌈If unexpected frames are received, the CanTp module shall
            behave according to the table below. This table specifies the N-PDU handling
            considering the current CanTp internal status (CanTp status).

        */

       else if( Can_PCI.frame_type == FF ) {            
            /*
               Table 1: Handling of N-PDU arrivals
                Terminate the current reception, report an indication, with parameter 
                Result set to E_NOT_OK, to the upper layer, 
                and process the FF N-PDU as the start of a new reception
            */
            PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
            CanTp_Reset_Rx_State_Variables();
            CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);

       }
       else if( Can_PCI.frame_type == SF ) {            
            /*
                Terminate the current reception, report an indication, 
                with parameter Result set to E_NOT_OK, to the upper layer, 
                and process the SF N-PDU as the start of a new reception
            */
            PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
            CanTp_Reset_Rx_State_Variables();
            CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);

       }
       else{
           /*
                UNKNOWN FRAME SHOULD BE IGNORED
           */

       }
    }

    /* uwaga na razie nie jestem pewny czy ten stan ma sens, mozliwe ze zostanie skasowany
        jest to stan w ktorym CanTp czeka aż zwolni się bufor 
        zasadniczo nie powinny przychodzić wtedy żadne ramki z wyjątkie flow control 
        nadejscie jakiejkolwiek innej ramki traktowane jest jako blad, ale uwaga 
        nie jest jasne jak zachowac sie na wypadek bledu (niestety)
    */

    else if( CanTp_StateVariables.CanTp_RxState == CANTP_RX_PROCESSING_SUSPENDED){    

        CanTp_GetPCI( PduInfoPtr, &Can_PCI );

        if( Can_PCI.frame_type == FC ){
            CanTp_FlowControlReception(RxPduId, &Can_PCI);
       }

        else if( Can_PCI.frame_type == FF ) {            
            /*
               Table 1: Handling of N-PDU arrivals
                Terminate the current reception, report an indication, with parameter 
                Result set to E_NOT_OK, to the upper layer, 
                and process the FF N-PDU as the start of a new reception
            */
            PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
            CanTp_Reset_Rx_State_Variables();
            CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);

       }
       else if( Can_PCI.frame_type == SF ) {            
            /*
                Terminate the current reception, report an indication, 
                with parameter Result set to E_NOT_OK, to the upper layer, 
                and process the SF N-PDU as the start of a new reception
            */
            PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
            CanTp_Reset_Rx_State_Variables();
            CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);

       }

       else {
           // uwaga na razie niejasne 
           PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
           CanTp_Reset_Rx_State_Variables();
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
                    *(CanPdu_Info->SduDataPtr + 1) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 2) = *(Can_payload + 1);
                    *(CanPdu_Info->SduDataPtr + 3) = *(Can_payload + 2);
                    *(CanPdu_Info->SduDataPtr + 4) = *(Can_payload + 3);
                    *(CanPdu_Info->SduDataPtr + 5) = *(Can_payload + 4);
                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload + 5);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 6);
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

static Std_ReturnType CanTp_SendFlowControl( PduIdType ID, uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime ){

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

        po wysłaniu ramki należy uruchomić timer N_Ar

        Jeżeli którakolwiek z wyżej wymienionych funkcji zwróci E_NOT_OK, należy przerwać działanie funkcji i też zwrócić E_NOT_OK
        Jeżeli wszystko przejdzie bez bledow, należy zwrocick E_OK

    */

    Std_ReturnType ret = E_OK;
    PduInfoType PduInfoPtr;
    CanPCI_Type CanPCI;
    uint8 payloadx[8];
    uint8 *MetaDataPtr;
    uint8 SduDataPtr[8];
    CanPCI.frame_type = FC;
    CanPCI.FS = FC_Status;
    CanPCI.BS = BlockSize;
    CanPCI.ST = SeparationTime;
    PduInfoPtr.SduLength = 0;
    PduInfoPtr.MetaDataPtr = MetaDataPtr;
    PduInfoPtr.SduDataPtr = SduDataPtr;

   if(( FC_Status == FC_OVFLW )||
      ( FC_Status == FC_WAIT  )||
      ( FC_Status == FC_CTS   ))
    {
       CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payloadx );
        ret = CanIf_Transmit(ID, &PduInfoPtr);
        if( ret == E_NOT_OK ){
            CanTp_Reset_Rx_State_Variables();
            PduR_CanTpRxIndication(ID, E_NOT_OK);
        }
        else{
            CanTp_TimerStart(&N_Ar_timer);
            if(FC_Status == FC_CTS){
                CanTp_TimerStart(&N_Cr_timer);
            }
            else if( FC_Status == FC_WAIT ){
                CanTp_TimerStart(&N_Br_timer);
            }
        }
        
    }
    else{
       ret = E_NOT_OK; 
    }

    return ret;
}

// reset all state variables 

static void CanTp_Reset_Rx_State_Variables(){
    CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
    CanTp_StateVariables.expected_CF_SN = 0;
    CanTp_StateVariables.message_length = 0;
    CanTp_StateVariables.sended_bytes = 0;
    CanTp_StateVariables.blocks_to_next_cts = 0;
    CanTp_StateVariables.CanTp_Current_RxId = 0;

    //Resetowanie timerów:
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Cr_timer);

}

// resume CanTp

static void CanTp_Resume(){
    CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;
}

//

static uint16 CanTp_Calculate_Available_Blocks( uint16 buffer_size ){

    uint16 retval; 
    uint16 remaining_bytes = CanTp_StateVariables.message_length - CanTp_StateVariables.sended_bytes;

    if( buffer_size >= remaining_bytes){
        retval = remaining_bytes / 7;
        if( CanTp_StateVariables.message_length%7 > 0) retval++; 
    }
    else{
        retval = buffer_size / 7;
    }
    
    return retval;

} 

// funkcja ustawia ilosc blokow do wyslania nastepnej ramki CTS

static void CanTp_set_blocks_to_next_cts( uint8 blocks ){

    CanTp_StateVariables.blocks_to_next_cts = blocks;

}


static Std_ReturnType  CanTp_SendSingleFrame(PduIdType id, uint8* payload, uint32 size){
    //Create and Init PduInfo 
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;

    CanPCI_Type CanPCI = {SF, size, 0, 0, 0, 0};
    Std_ReturnType ret = E_OK;
    ret = E_OK;
    //Przygotowanie PDU
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
    
    //Sprawdzamy czy transmisja odbyła się poprawnie
    if(CanIf_Transmit(id , &PduInfo) == E_OK ){
        //Startowanie timera N_As
        CanTp_TimerStart(&N_As_timer);
    }
    else{
        /*
            [SWS_CanTp_00343]  CanTp shall terminate the current  transmission connection 
             when CanIf_Transmit() returns E_NOT_OK when transmitting an SF, FF, of CF ()
        */
        //Wywołanie z argumentem E_NOT_OK
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        //Ustawnienie E_NOT_OK jako wartość zwrotną
        ret = E_NOT_OK;
    }
    return ret;
}


static void CanTp_SendNextCF(){

    // funkcja moze byc wywolana z kilku roznych miejsc wiec powinna byc maksymalnie zautomatyzowana 
    // workflow, sprawdzamy czy ilosc wyslanych danych == rozmiar wiadomosci 
    // jeżeli tak to informujemy PduR o powodzeniu wysylania 
    // jeżeli nie to kopiujemy dane i przygotowujemy ramkę do wysyłania 
    // sprawdzamy stan powyzszych opercji 
    // jezeli wszystko ok, to wysylamy ramke 
    // jezeli nie ok no to mamy problem 
    // za kazdym razem updatujemy zmienne stanu

    BufReq_ReturnType BufReq_State;
    PduInfoType PduInfoPtr;
    PduLengthType Pdu_Len;
    Std_ReturnType ret;
    uint8 bytes_to_send;
    uint8_t payload[8];

    PduInfoPtr.SduDataPtr = payload;
    PduInfoPtr.MetaDataPtr = NULL;
    

    if( CanTp_Tx_StateVariables.sent_bytes == CanTp_Tx_StateVariables.message_legth ){
        // cala wiadomosc zostala wyslana 

       /* [SWS_CanTp_00090] ⌈When the transport transmission session is 
       successfully completed, the CanTp module shall call a notification 
       service of the upper layer, PduR_CanTpTxConfirmation(), with the result E_OK. ⌋ ( )*/

        PduR_CanTpTxConfirmation(CanTp_Tx_StateVariables.CanTp_Current_TxId, E_OK);
        CanTp_ResetTxStateVariables();
    }
    else{
        // wysylamy dalej 
        if(CanTp_Tx_StateVariables.message_legth - CanTp_Tx_StateVariables.sent_bytes < 7) bytes_to_send = CanTp_Tx_StateVariables.message_legth - CanTp_Tx_StateVariables.sent_bytes;
        else bytes_to_send = 7;
        PduInfoPtr.SduLength = bytes_to_send;

        BufReq_State = PduR_CanTpCopyTxData(CanTp_Tx_StateVariables.CanTp_Current_TxId, &PduInfoPtr, NULL, &Pdu_Len);
        if(BufReq_State == BUFREQ_OK){
            

            ret = CanTp_SendConsecutiveFrame(CanTp_Tx_StateVariables.CanTp_Current_TxId, CanTp_Tx_StateVariables.next_SN, PduInfoPtr.SduDataPtr, bytes_to_send);

            if( ret == E_OK ){
                CanTp_Tx_StateVariables.sent_bytes = CanTp_Tx_StateVariables.sent_bytes + bytes_to_send;
                CanTp_Tx_StateVariables.blocks_to_fc--;
                CanTp_Tx_StateVariables.next_SN = (CanTp_Tx_StateVariables.next_SN + 1)%7;
                if((CanTp_Tx_StateVariables.blocks_to_fc == 0) && (CanTp_Tx_StateVariables.sent_bytes != CanTp_Tx_StateVariables.message_legth) )CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
                else CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING;        

            }
            else{
                CanTp_ResetTxStateVariables();
            }
            

        }
        else if(BufReq_State == BUFREQ_E_NOT_OK){
            
            PduR_CanTpTxConfirmation(CanTp_Tx_StateVariables.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTxStateVariables();  
            
        }
        else { // BUSY
            //Start N_Cs timer
            CanTp_TimerStart(&N_Cs_timer);
            CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_PROCESSING_SUSPENDED;
        }



    }


}


static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 SN, uint8* payload, uint32 size){
    //Create and Init PduInfo 
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;
    PduInfo.SduLength = size;
    CanPCI_Type CanPCI;// = {CF, size, SN, 0, 0, 0};

    CanPCI.frame_type = CF;
    CanPCI.SN = SN;

    Std_ReturnType ret = E_OK;
    //Przygotowanie PDU
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
    //Sprawdzamy czy transmisja odbyła się poprawnie
    if(CanIf_Transmit(id , &PduInfo) == E_OK ){
        //Startowanie timera N_As
        CanTp_TimerStart(&N_As_timer);
    }
    else{
        /*
            [SWS_CanTp_00343]  CanTp shall terminate the current  transmission connection 
             when CanIf_Transmit() returns E_NOT_OK when transmitting an SF, FF, of CF ()
        */
        //Wywołanie z argumentem E_NOT_OK
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        //Ustawnienie E_NOT_OK jako wartość zwrotną
        ret = E_NOT_OK;
    }

    return ret;
}



static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 message_lenght){
    //Create and Init PduInfo 
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;

    CanPCI_Type CanPCI = {FF, message_lenght, 0, 0, 0, 0}; 
    uint8 payload[8] = {0,0,0,0,0,0,0,0};
    Std_ReturnType ret = E_OK;

    //Przygotowanie PDU
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);

    //Sprawdzamy czy transmisja odbyła się poprawnie
    if(CanIf_Transmit(id, &PduInfo) == E_OK ){
        //Startowanie timera N_As i N_Bs
        CanTp_TimerStart(&N_As_timer);
        CanTp_TimerStart(&N_Bs_timer);
    }
    else{
        /*
            [SWS_CanTp_00343]  CanTp shall terminate the current  transmission connection 
             when CanIf_Transmit() returns E_NOT_OK when transmitting an SF, FF, of CF ()
        */
        //Wywołanie z argumentem E_NOT_OK
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        //Ustawnienie E_NOT_OK jako wartość zwrotną
        ret = E_NOT_OK;
    }

    return ret;
}
// celowo dałem void dlatego, że funkcja ktora je woła sama jest typu void 

static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *Can_PCI){
         
    PduLengthType buffer_size;      // use when calling PduR callbacks
    BufReq_ReturnType Buf_Status;   // use when calling PduR callbacks
    uint16 current_block_size;      // currently processes block size (will be global)

    Buf_Status = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI->frame_lenght, &buffer_size);
    if( Buf_Status == BUFREQ_OK ) {
                
        // calculate block size to be send by FLOW_CONTROL
        CanTp_StateVariables.message_length = Can_PCI->frame_lenght;
        CanTp_StateVariables.CanTp_Current_RxId = RxPduId;
        current_block_size = CanTp_Calculate_Available_Blocks( buffer_size ); // czy musie byc wystarczajaco duzo miejsca w buforze na FF?

        // enough space to send message, send CTS ans wait for CF
        if( current_block_size > 0){    
            CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, current_block_size, FC_CTS, DEFAULT_ST );   
            CanTp_set_blocks_to_next_cts( current_block_size );
            CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING; // After Successfull Reception of FF, wait for Consecutive Frames
              
        }
        // there is no space in buffer, wait for space until timeout
        else{
            CanTp_set_blocks_to_next_cts( current_block_size );
            CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, current_block_size, FC_WAIT, DEFAULT_ST );   
            CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED; // After Successfull Reception of FF, wait for Consecutive Frames
           
        }
        CanTp_StateVariables.expected_CF_SN = 1; 
    }
    
    else if ( Buf_Status == BUFREQ_OVFL ){
        /*
        [SWS_CanTp_00318] ⌈After the reception of a First Frame, if the function PduR_CanTpStartOfReception()returns BUFREQ_E_OVFL to the CanTp module, 
        the CanTp module shall send a Flow Control N-PDU with overflow status (FC(OVFLW)) and abort the N-SDU reception. */
        CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, current_block_size, FC_OVFLW, DEFAULT_ST );
        CanTp_Reset_Rx_State_Variables();
        CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT;
    }
    else { // BUF_NOT_OK or other problem
        CanTp_Reset_Rx_State_Variables();
    }
}

static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr){

    PduLengthType buffer_size;      // use when calling PduR callbacks
    BufReq_ReturnType Buf_Status;   // use when calling PduR callbacks
    PduInfoType Extracted_Data;

    CanTp_StateVariables.CanTp_RxState = CANTP_RX_WAIT; // do not change the state 
    Buf_Status = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI->frame_lenght, &buffer_size);

    if( (Buf_Status == BUFREQ_OK)  ){
                
        if(buffer_size >= Can_PCI->frame_lenght){

            Extracted_Data.SduLength = Can_PCI->frame_lenght;
            Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
            Buf_Status = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);
            PduR_CanTpRxIndication ( RxPduId, Buf_Status);
                    
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


static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr){

    
    PduLengthType buffer_size;      // use when calling PduR callbacks
    BufReq_ReturnType Buf_Status;   // use when calling PduR callbacks
    PduInfoType Extracted_Data;
    uint16 current_block_size;

    CanTp_TimerReset(&N_Cr_timer); // mamy ramkę CF, możemy zresetować timer 

    if(CanTp_StateVariables.CanTp_Current_RxId ==  RxPduId){ // sprawdz czy ID sie zgadza

        if(CanTp_StateVariables.expected_CF_SN == Can_PCI->SN){ // sprawdz czy numer ramki sie zgadza                      
            Extracted_Data.SduLength = Can_PCI->frame_lenght;
            Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+1);

            /*
                            [SWS_CanTp_00269] ⌈After reception of each Consecutive Frame the CanTp module shall 
                            call the PduR_CanTpCopyRxData() function with a PduInfo pointer 
                            containing data buffer and data length:
            */

            Buf_Status = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);
            if( Buf_Status == BUFREQ_OK ){
                            
                            // jak przeszlo to sprawdzmy czy poszla juz cala ramka

                            /*
                            [SWS_CanTp_00277] ⌈With regard to FF N-PDU reception, the content of the 
                            Flow Control N-PDU depends on the PduR_CanTpStartOfReception() service result.
                            */

                CanTp_StateVariables.sended_bytes += PduInfoPtr->SduLength;
                CanTp_StateVariables.blocks_to_next_cts--;

                if( CanTp_StateVariables.sended_bytes == CanTp_StateVariables.message_length  ){ // poszlo wszystko
                    /*
                    [SWS_CanTp_00084] ⌈When the transport reception session is completed 
                    (successfully or not) the CanTp module shall call the upper layer 
                    notification service PduR_CanTpRxIndication().
                    */
                    PduR_CanTpRxIndication(CanTp_StateVariables.CanTp_Current_RxId, E_OK);
                    CanTp_Reset_Rx_State_Variables();
                }     
                else{ 
                    CanTp_StateVariables.expected_CF_SN++;
                    CanTp_StateVariables.expected_CF_SN%8;
                    current_block_size = CanTp_Calculate_Available_Blocks( buffer_size);

                    if(current_block_size > 0){
                                    
                        //send CTS if last cts block were sent

                        /*
                        [SWS_CanTp_00278] ⌈It is important to note that FC N-PDU will 
                        only be sent after every block, composed of a number BS 
                        (Block Size) of consecutive frames.
                        */
                        CanTp_TimerStart(&N_Cr_timer);
                        if(CanTp_StateVariables.blocks_to_next_cts == 0 ){
                            CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, current_block_size, FC_CTS, DEFAULT_ST );
                            CanTp_set_blocks_to_next_cts( current_block_size );
                        }
                        CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING;         
                                    // to do add timer
                    }
                    else{
                        CanTp_StateVariables.CanTp_RxState = CANTP_RX_PROCESSING_SUSPENDED;
                        CanTp_SendFlowControl(CanTp_StateVariables.CanTp_Current_RxId, current_block_size, FC_WAIT, DEFAULT_ST ); 
                    }
                }

            }
            else{ 
                // problem z buforem 
                /* 
                [SWS_CanTp_00271] ⌈If the PduR_CanTpCopyRxData() returns BUFREQ_E_NOT_OK 
                after reception of a Consecutive Frame in a block the CanTp shall abort the 
                reception of N-SDU and notify the PduR module by calling the 
                PduR_CanTpRxIndication() with the result E_NOT_OK. 
                */
                PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
                CanTp_Reset_Rx_State_Variables();
            }
        }
        else{ 
        // zly numer ramki np zgubiona ramka albo gorszy problem
        /*
        [SWS_CanTp_00314] ⌈The CanTp shall check the correctness of each SN received during a segmented reception. 
        In case of wrong SN received the CanTp module shall abort reception and notify the upper layer of this failure 
        by calling the indication function PduR_CanTpRxIndication() with the result E_NOT_OK.
        */
            PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
            CanTp_Reset_Rx_State_Variables();
        }
    
    }
    else {

        // nieoczekiwane ID
        PduR_CanTpRxIndication ( CanTp_StateVariables.CanTp_Current_RxId, E_NOT_OK);
        CanTp_Reset_Rx_State_Variables();
    }
}

static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *Can_PCI){
    

    if( CanTp_Tx_StateVariables.Cantp_TxState == CANTP_TX_PROCESSING_SUSPENDED ){
        /*
        [SWS_CanTp_00057] ⌈If unexpected frames are received, the CanTp module shall
        behave according to the table below. This table specifies the N-PDU handling
        considering the current CanTp internal status (CanTp status). ⌋ (SRS_Can_01082)
        It must be understood, that the received N-PDU contains the same address
        information (N_AI) as the reception or transmission, which may be in progress at the
        time the N_PDU is received.
        */
        if(CanTp_Tx_StateVariables.CanTp_Current_TxId == RxPduId ){ // check ID
            if(Can_PCI->FS == FC_CTS){
                CanTp_Tx_StateVariables.blocks_to_fc = Can_PCI->BS; 
                CanTp_SendNextCF();
            }   
            else if( Can_PCI->FS == FC_WAIT ){
                /* only reset timer */
            }
            else if( Can_PCI->FS == FC_OVFLW){
                /*ABORT TRANSMSSION */ 

                /*[SWS_CanTp_00309] ⌈If a FC frame is received with the FS set to OVFLW the CanTp module shall 
                abort the transmit request and notify the upper layer by calling the callback 
                function PduR_CanTpTxConfirmation() with the result E_NOT_OK. ⌋ ( )*/

                PduR_CanTpTxConfirmation(CanTp_Tx_StateVariables.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTxStateVariables();
            }
            else{
                /* UNKNOWN/INVALID FS */ 
                /*[SWS_CanTp_00317] ⌈If a FC frame is received with an invalid FS the 
                CanTp module shall abort the transmission of this message and notify 
                the upper layer by calling the callback function PduR_CanTpTxConfirmation()
                 with the result E_NOT_OK. ⌋ ( )*/
                PduR_CanTpTxConfirmation(CanTp_Tx_StateVariables.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTxStateVariables();
            }
        }
        else{
            /* IGNORE FRAME WITH UNEXPECTED ID */
        }
    }
    else{   
        // IGNORE UNEXPECTED FLOW CONTROLL
    }
}


//Funkcja do resetowania zmienne stanu nadajnika
static void CanTp_ResetTxStateVariables(void){
    CanTp_Tx_StateVariables.sent_bytes = 0;
    CanTp_Tx_StateVariables.message_legth = 0;
    CanTp_Tx_StateVariables.Cantp_TxState = CANTP_TX_WAIT;
    CanTp_Tx_StateVariables.CanTp_Current_TxId = 0;
    CanTp_Tx_StateVariables.blocks_to_fc = 0;


    //Resetowanie timerów:
    CanTp_TimerReset(&N_As_timer);
    CanTp_TimerReset(&N_Bs_timer);
    CanTp_TimerReset(&N_Cs_timer);
}