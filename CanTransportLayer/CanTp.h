
#ifndef CAN_TP_H
#define CAN_TP_H

/**===================================================================================================================*\
  @file CanTp.h

  @brief Can Transport Layer
  
  Implementation of Can Transport Layer

  @see Can_Tp.pdf
\*====================================================================================================================*/

/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "ComStack_Types.h"
/*====================================================================================================================*\
    Makra globalne
\*====================================================================================================================*/

/*====================================================================================================================*\
    Typy globalne
\*====================================================================================================================*/
typedef struct {


} CanTp_ConfigType;


/*====================================================================================================================*\
    Deklaracje funkcji globalnych
\*====================================================================================================================*/
void CanTp_Init ( void );

void CanTp_GetVersionInfo ( Std_VersionInfoType* versioninfo );

void CanTp_Shutdown ( void );

Std_ReturnType CanTp_Transmit ( PduIdType TxPduId, const PduInfoType* PduInfoPtr );

Std_ReturnType CanTp_CancelTransmit ( PduIdType TxPduId );

Std_ReturnType CanTp_CancelReceive ( PduIdType RxPduId );

Std_ReturnType CanTp_ChangeParameter ( PduIdType id, TPParameterType parameter, uint16 value );

Std_ReturnType CanTp_ReadParameter ( PduIdType id, TPParameterType parameter, uint16* value );

void CanTp_MainFunction ( void ); // implementujemy bo dotyczy schedule managera
// ma to być cyklicznie z konkretnym okresem wywoływane bo timer, okres do zdefiniowania 

// callbacks

void CanTp_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr );

void CanTp_TxConfirmation ( PduIdType TxPduId, Std_ReturnType result );

/*====================================================================================================================*\
    Kod globalnych funkcji inline i makr funkcyjnych
\*====================================================================================================================*/

#endif