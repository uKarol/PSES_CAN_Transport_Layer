#include "ComStack_Types.h"

BufReq_ReturnType PduR_CanTpCopyRxData ( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );

BufReq_ReturnType PduR_CanTpCopyTxData ( PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr );

BufReq_ReturnType PduR_CanTpStartOfReception (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );

void PduR_CanTpRxIndication ( PduIdType id, Std_ReturnType result );

void PduR_CanTpTxConfirmation ( PduIdType id, Std_ReturnType result );
