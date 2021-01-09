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

/*====================================================================================================================*\
    Typy lokalne
\*====================================================================================================================*/
typedef enum {
    CAN_TP_ON, 
    CAN_TP_OFF
}CanTpState_Type;

typedef enum {
    CANTP_RX_WAIT,
    CANTP_RX_PROCESSING
} CanTpRxState_Type;

typedef enum {
    CANTP_TX_WAIT,
    CANTP_TX_PROCESSING
} CanTpTxState_Type;

typedef enum {SF = 0, FF = 1, CF = 2, FC = 3} frame_type_t;

typedef struct{
    frame_type_t frame_type;
    uint32 frame_lenght;
    uint8 BS;
    uint8 FS;
    uint8 ST;
} CanFrameInfo_Type;

/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/

/*====================================================================================================================*\
    Zmienne lokalne (statyczne)
\*====================================================================================================================*/
static CanTpState_Type CanTpState; 
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
  @brief Can Initialisation 

  This function initializes the CanTp module.  

  Parameters (in) CfgPtr Pointer to the CanTp post-build configuration data.
  Return value None

  Element projektu: [P2]
*/
void CanTp_Init ( const CanTp_ConfigType* CfgPtr );

void CanTp_GetVersionInfo ( Std_VersionInfoType* versioninfo );

void CanTp_Shutdown ( void );

Std_ReturnType CanTp_Transmit ( PduIdType TxPduId, const PduInfoType* PduInfoPtr );

Std_ReturnType CanTp_CancelTransmit ( PduIdType TxPduId );

Std_ReturnType CanTp_CancelReceive ( PduIdType RxPduId );

Std_ReturnType CanTp_ChangeParameter ( PduIdType id, TPParameterType parameter, uint16 value );

Std_ReturnType CanTp_ReadParameter ( PduIdType id, TPParameterType parameter, uint16* value );

void CanTp_MainFunction ( void ); // uwaga tego chyba nie implementujemy bo dotyczy schedule managera

// callbacks

void CanTp_TxConfirmation ( PduIdType TxPduId, Std_ReturnType result );

void CanTp_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr );

static Std_ReturnType CanTp_FrameCheckType(uint8* can_data, CanFrameInfo_Type* CanFrameInfo){
    Std_ReturnType ret = E_OK;


    switch( (*can_data) >> 4){
        case 0x00:
            CanFrameInfo->frame_type = SF;
            CanFrameInfo->frame_lenght = *can_data;
        break;

        case 0x01:
            CanFrameInfo->frame_type = FF;
            if( ((*can_data) & 0x0F) | *(can_data + 1)) {
                CanFrameInfo->frame_lenght =  (*can_data) & 0x0F;
                CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | *(can_data + 1); 
            }
            else{
                CanFrameInfo->frame_lenght =  *(can_data+2);
                CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | *(can_data + 3); 
                CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | *(can_data + 4); 
                CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | *(can_data + 5); 
            }
        break;
        case 0x02:
            CanFrameInfo->frame_type = CF;
        break;

        case 0x03:
            CanFrameInfo->frame_type = FC;
            CanFrameInfo->FS = *(can_data) & 0x0F; 
            CanFrameInfo->BS = *(can_data + 1); 
            CanFrameInfo->ST = *(can_data + 2); 
        break;

        default:
            ret = E_NOT_OK;
        break;
    }

    return ret;
}