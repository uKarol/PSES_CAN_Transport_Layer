#include "ComStack_Types.h"

/**
  @brief PduR_CanTpCopyRxData

This function is called to provide the received data of an I-PDU segment (N-PDU) to the upper layer. 
Each call to this function provides the next part of the I-PDU data. 
The size of the remaining buffer is written to the position indicated by bufferSizePtr.

id : Identification of the received I-PDU.
info : Provides the source buffer (SduDataPtr) and the number of bytes to be copied (SduLength). 
       An SduLength of 0 can be used to query the current amount of available buffer in the upper layer module. 
       In this case, the SduDataPtr may be a NULL_PTR.

bufferSizePtr: Available receive buffer after data has been copied.

BufReq_ReturnType
BUFREQ_OK: Data copied successfully 
BUFREQ_E_NOT_OK: Data was not copied because an error occurred.

*/
BufReq_ReturnType PduR_CanTpCopyRxData ( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );

BufReq_ReturnType PduR_CanTpCopyTxData ( PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr );

/**
 * This function is called at the start of receiving an N-SDU. 
 * The N-SDU might be fragmented into multiple N-PDUs 
 * (FF with one or more following CFs) or might consist of a 
 * single N-PDU (SF). The service shall provide the currently 
 * available maximum buffer size when invoked with TpSduLength equal to 0.
 * 
 * 
id: Identification of the I-PDU.
info: Pointer to a PduInfoType structure containing the payload data 
    (without protocol information) and payload length of the first frame 
    or single frame of a transport protocol I-PDU reception, 
    and the MetaData related to this PDU. If neither first/single frame 
    data nor MetaData are available, this parameter is set to NULL_PTR.
TpSduLength: Total length of the N-SDU to be received.
bufferSizePtr: Available receive buffer in the receiving module. 
               This parameter will be used to compute the Block Size (BS) 
               in the transport protocol module.

BufReq_ReturnType
    BUFREQ_OK: 
        Connection has been accepted. bufferSizePtr indicates 
        the available receive buffer; reception is continued. If no buffer 
        of the requested size is available, a receive buffer size of 0 shall
        be indicated by bufferSizePtr. 
    BUFREQ_E_NOT_OK: 
        Connection has been rejected;
        reception is aborted. bufferSizePtr remains unchanged. 
    BUFREQ_E_OVFL:
        No buffer of the required length can be provided; 
        reception is aborted. bufferSizePtr remains unchanged.


*/
BufReq_ReturnType PduR_CanTpStartOfReception (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );

void PduR_CanTpRxIndication ( PduIdType id, Std_ReturnType result );

void PduR_CanTpTxConfirmation ( PduIdType id, Std_ReturnType result );
