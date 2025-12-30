#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "../config.h"
#include <tcl.h>
#include <ntcan/ntcan.h>

#define NS_PREFIX "ntcan::"                       /* Tcl namespace prefix for command definitions */

#define STATUS_TXT_LEN 1000

#if defined(_WIN32)
#define TCL_NTCAN_HANDLE  intptr_t
#else
#define TCL_NTCAN_HANDLE  Tcl_WideInt
#endif

extern "C" {
    // extern for C++.
    int Ntcan_Init(Tcl_Interp *interp);
    int Ntcan_Unload(Tcl_Interp *interp);
}

void FormatError(Tcl_Interp *interp, char* cmd, NTCAN_RESULT error) {
    char errorTxt[STATUS_TXT_LEN];
    char statusTxt[STATUS_TXT_LEN];

    canFormatError(error, NTCAN_ERROR_FORMAT_LONG, errorTxt, sizeof(errorTxt));
    snprintf(statusTxt, sizeof(statusTxt), "NTCAN %s() failed with error: %d / %s", cmd, error, errorTxt);
    Tcl_AppendResult(interp, &statusTxt, NULL);
}

int Scan(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    int i;
    NTCAN_HANDLE handle;                      /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */
    CAN_IF_STATUS cstat;
    char statusTxt[STATUS_TXT_LEN];

    for (i = 0; i <= NTCAN_MAX_NETS; i++) {
        retvalue = canOpen(i, 0, 1, 1, 0, 0, &handle);
        if (retvalue == NTCAN_SUCCESS) {
            retvalue = canStatus(handle, &cstat);
            canClose(handle);

            if (retvalue != NTCAN_SUCCESS) {
                snprintf(statusTxt, sizeof(statusTxt), "Cannot get Status of Net-Device %02X (ret = 0x%x)\n",
                         i, (unsigned int)retvalue);
                Tcl_AppendResult(interp, &statusTxt, NULL);
            } else {
                snprintf(statusTxt, sizeof(statusTxt), "Net %3d: ID=%s\n"
                         "         Versions (hex): Dll=%1X.%1X.%02X "
                         " Drv=%1X.%1X.%02X"
                         " FW=%1X.%1X.%02X"
                         " HW=%1X.%1X.%02X\n"
                         "         Status=%08x Features=%04x\n",
                         i, cstat.boardid,
                         cstat.dll     >>12, (cstat.dll     >>8) & 0xf, cstat.dll      & 0xff,
                         cstat.driver  >>12, (cstat.driver  >>8) & 0xf, cstat.driver   & 0xff,
                         cstat.firmware>>12, (cstat.firmware>>8) & 0xf, cstat.firmware & 0xff,
                         cstat.hardware>>12, (cstat.hardware>>8) & 0xf, cstat.hardware & 0xff,
                         (unsigned long)cstat.boardstatus, cstat.features);
                Tcl_AppendResult(interp, &statusTxt, NULL);
            }
        }
    }
    return TCL_OK;
}

int Open(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    int net;                                  /* Logical net number (here: 0) */
    uint32_t mode;                            /* Mode bits for canOpen */
    int32_t txqueuesize;                      /* No Tx queue required */
    int32_t rxqueuesize;                      /* Maximum number of Rx messages */
    int32_t txtimeout;                        /* No Tx timeout required */
    int32_t rxtimeout;                        /* Rx timeout in ms */
    NTCAN_HANDLE handle;                      /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 7) {
        Tcl_WrongNumArgs(interp, 1, objv, "net mode txqueuesize rxqueuesize txtimeout rxtimeout");
        return TCL_ERROR;
    }
    Tcl_GetIntFromObj(interp, objv[1], &net);
    long _mode;
    Tcl_GetLongFromObj(interp, objv[2], &_mode);
    mode = _mode;
    Tcl_GetIntFromObj(interp, objv[3], &txqueuesize);
    Tcl_GetIntFromObj(interp, objv[4], &rxqueuesize);
    Tcl_GetIntFromObj(interp, objv[5], &txtimeout);
    Tcl_GetIntFromObj(interp, objv[6], &rxtimeout);

    retvalue = canOpen(net,
                       mode,
                       txqueuesize,
                       rxqueuesize,
                       txtimeout,
                       rxtimeout,
                       &handle);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canOpen", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewWideIntObj((TCL_NTCAN_HANDLE)handle));
        return TCL_OK;
    }
}

int Close(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canClose((NTCAN_HANDLE)handle);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canClose", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int SetBaudrate(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t baud;                            /* Configured CAN baudrate */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle baurate");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    long _baud;
    Tcl_GetLongFromObj(interp, objv[2], &_baud);
    baud = _baud;

    retvalue = canSetBaudrate((NTCAN_HANDLE)handle, baud);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canSetBaudrate", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int GetBaudrate(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t baud;                            /* Configured CAN baudrate */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canGetBaudrate((NTCAN_HANDLE)handle, &baud);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canGetBaudrate", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(baud));
        return TCL_OK;
    }
}

int SetBaudrateX(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint16_t mode;                            /* Configured CAN FD mode */
    uint16_t flags;                           /* Configured CAN FD flags */
    uint32_t arbBaud;                         /* Configured CAN FD nominal baudrate */
    uint32_t dataBaud;                        /* Configured CAN FD data baudrate */
    NTCAN_BAUDRATE_X baud;                    /* Bit rate configuration */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 6) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle mode flags nominalBaurate dataBaudrate");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    int _intData;
    Tcl_GetIntFromObj(interp, objv[2], &_intData);
    mode = _intData;
    Tcl_GetIntFromObj(interp, objv[3], &_intData);
    flags = _intData;
    long _longData;
    Tcl_GetLongFromObj(interp, objv[4], &_longData);
    arbBaud = _longData;
    Tcl_GetLongFromObj(interp, objv[5], &_longData);
    dataBaud = _longData;

    baud.mode = mode;
    baud.flags = flags;
    baud.arb.u.idx = arbBaud;
    baud.data.u.idx = dataBaud;
    retvalue = canSetBaudrateX((NTCAN_HANDLE)handle, &baud);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canSetBaudrateX", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int GetBaudrateX(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_BAUDRATE_X baud;                    /* Bit rate configuration */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canGetBaudrateX((NTCAN_HANDLE)handle, &baud);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canGetBaudrateX", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_Obj *lResult;
        lResult = Tcl_GetObjResult(interp);
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewIntObj(baud.mode));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewIntObj(baud.flags));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(baud.arb.u.idx));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(baud.data.u.idx));
        return TCL_OK;
    }
}


int IdAdd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    int32_t id;                               /* CAN-ID to add to filter */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle id");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &id);

    retvalue = canIdAdd((NTCAN_HANDLE)handle, id);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIdAdd", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int IdRegionAdd(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    int32_t idStart;                          /* First CAN-ID or Event-ID */
    int32_t idCount;                          /* Count of requested ID's */
    int32_t idCountOut;                       /* Successful selected ID's */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */
    char statusTxt[STATUS_TXT_LEN];

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle idStart idCnt");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &idStart);
    Tcl_GetIntFromObj(interp, objv[3], &idCount);
    idCountOut = idCount;

    retvalue = canIdRegionAdd((NTCAN_HANDLE)handle, idStart, &idCountOut);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIdRegionAdd", retvalue);
        return TCL_ERROR;
    } else {
        if (idCount != idCountOut) {
            snprintf(statusTxt, sizeof(statusTxt), "NTCAN canIdRegionAdd() added only %d instead of %d IDs", idCountOut, idCount);
            Tcl_AppendResult(interp, &statusTxt, NULL);
            return TCL_ERROR;
        } else {
            return TCL_OK;
        }
    }
}

int IdDelete(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    int32_t id;                               /* CAN-ID to delete from filter */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle id");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &id);

    retvalue = canIdDelete((NTCAN_HANDLE)handle, id);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIdDelete", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int IdRegionDelete(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    int32_t idStart;                          /* First CAN-ID or Event-ID */
    int32_t idCount;                          /* Count of requested ID's */
    int32_t idCountOut;                       /* Successful selected ID's */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */
    char statusTxt[STATUS_TXT_LEN];

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle idStart idCnt");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &idStart);
    Tcl_GetIntFromObj(interp, objv[3], &idCount);
    idCountOut = idCount;

    retvalue = canIdRegionDelete((NTCAN_HANDLE)handle, idStart, &idCountOut);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIdRegionDelete", retvalue);
        return TCL_ERROR;
    } else {
        if (idCount != idCountOut) {
            snprintf(statusTxt, sizeof(statusTxt), "NTCAN canIdRegionDelete() deleted only %d instead of %d IDs", idCountOut, idCount);
            Tcl_AppendResult(interp, &statusTxt, NULL);
            return TCL_ERROR;
        } else {
            return TCL_OK;
        }
    }
}

int FlushRxFifo(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_FLUSH_RX_FIFO, NULL);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int GetRxMsgCount(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  msg_cnt;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_RX_MSG_COUNT, &msg_cnt);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(msg_cnt));
        return TCL_OK;
    }
}

int GetTxMsgCount(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  msg_cnt;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_TX_MSG_COUNT, &msg_cnt);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(msg_cnt));
        return TCL_OK;
    }
}

int GetRxTimeout(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  timeout;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_RX_TIMEOUT, &timeout);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(timeout));
        return TCL_OK;
    }
}

int GetTxTimeout(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  timeout;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_TX_TIMEOUT, &timeout);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(timeout));
        return TCL_OK;
    }
}

int SetRxTimeout(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  timeout;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle timeout");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    long _timeout;
    Tcl_GetLongFromObj(interp, objv[2], &_timeout);
    timeout = _timeout;

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_SET_RX_TIMEOUT, &timeout);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int SetTxTimeout(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    uint32_t  timeout;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle timeout");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    long _timeout;
    Tcl_GetLongFromObj(interp, objv[2], &_timeout);
    timeout = _timeout;

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_SET_TX_TIMEOUT, &timeout);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int AbortRx(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_ABORT_RX, NULL);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int AbortTx(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_ABORT_TX, NULL);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int GetBusStatistic(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_BUS_STATISTIC busStatistic;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_BUS_STATISTIC, &busStatistic);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_Obj *lResult;
        lResult = Tcl_GetObjResult(interp);
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(busStatistic.ctrl_ovr));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(busStatistic.fifo_ovr));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(busStatistic.err_frames));
        Tcl_ListObjAppendElement(interp, lResult, Tcl_NewLongObj(busStatistic.aborted_frames));
        return TCL_OK;
    }
}

int GetCtrlStatus(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_CTRL_STATE ctrlState;
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canIoctl((NTCAN_HANDLE)handle, NTCAN_IOCTL_GET_CTRL_STATUS, &ctrlState);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canIoctl", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_Obj *objResult = Tcl_GetObjResult(interp);
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(ctrlState.rcv_err_counter));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(ctrlState.xmit_err_counter));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(ctrlState.status));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(ctrlState.type));
        return TCL_OK;
    }
}

int Read(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    CMSG cmsg;                                /* Buffer for can messages */
    int32_t count = 1;                        /* # of messages for canRead() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canRead((NTCAN_HANDLE)handle, &cmsg, &count, NULL);

    if (retvalue == NTCAN_RX_TIMEOUT) {
        Tcl_AppendResult(interp, "NTCAN canRead() returned timeout", NULL);
        return TCL_ERROR;
    } else if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canRead", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_Obj *objResult = Tcl_GetObjResult(interp);
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewLongObj(cmsg.id));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(cmsg.len & 0xF0));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(NTCAN_LEN_TO_DATASIZE(cmsg.len)));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewByteArrayObj(cmsg.data, NTCAN_LEN_TO_DATASIZE(cmsg.len)));
        return TCL_OK;
    }
}

int Write(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    CMSG cmsg;                                /* Buffer for can messages */
    int32_t count = 1;                        /* # of messages for canWrite() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle id mode data");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &(cmsg.id));
    int mode;
    Tcl_GetIntFromObj(interp, objv[3], &mode);

    int dataLen;
    unsigned char *tclData = Tcl_GetByteArrayFromObj(objv[4], &dataLen);
    if (dataLen > 8) {
        Tcl_AppendResult(interp, "NTCAN canWrite() data length > 8", NULL);
        return TCL_ERROR;
    }

    cmsg.len = mode | dataLen;
    for (int i = 0; i < dataLen; i++)
        {
            cmsg.data[i] = tclData[i];
        }

    retvalue = canWrite((NTCAN_HANDLE)handle, &cmsg, &count, NULL);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canWrite", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int ReadX(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    CMSG_X cmsg;                              /* Buffer for can messages */
    int32_t count = 1;                        /* # of messages for canRead() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canReadX((NTCAN_HANDLE)handle, &cmsg, &count, NULL);

    if (retvalue == NTCAN_RX_TIMEOUT) {
        Tcl_AppendResult(interp, "NTCAN canReadX() returned timeout", NULL);
        return TCL_ERROR;
    } else if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canRead", retvalue);
        return TCL_ERROR;
    } else {
        Tcl_Obj *objResult = Tcl_GetObjResult(interp);
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewLongObj(cmsg.id));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(cmsg.len & 0xF0));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewIntObj(NTCAN_LEN_TO_DATASIZE(cmsg.len)));
        Tcl_ListObjAppendElement(interp, objResult, Tcl_NewByteArrayObj(cmsg.data, NTCAN_LEN_TO_DATASIZE(cmsg.len)));
        return TCL_OK;
    }
}

int WriteX(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    CMSG_X cmsg;                              /* Buffer for can messages */
    int32_t count = 1;                        /* # of messages for canWrite() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle id mode data");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);
    Tcl_GetIntFromObj(interp, objv[2], &(cmsg.id));
    int mode;
    Tcl_GetIntFromObj(interp, objv[3], &mode);

    int dataLen;
    unsigned char *tclData = Tcl_GetByteArrayFromObj(objv[4], &dataLen);
    if (dataLen > 64) {
        Tcl_AppendResult(interp, "NTCAN canWriteX() data length > 64", NULL);
        return TCL_ERROR;
    }

    cmsg.len = mode | NTCAN_DATASIZE_TO_DLC(dataLen);
    for (int i = 0; i < dataLen; i++)
        {
            cmsg.data[i] = tclData[i];
        }

    retvalue = canWriteX((NTCAN_HANDLE)handle, &cmsg, &count, NULL);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canWriteX", retvalue);
        return TCL_ERROR;
    } else {
        return TCL_OK;
    }
}

int Status(ClientData cData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    TCL_NTCAN_HANDLE handle;                  /* CAN handle returned by canOpen() */
    NTCAN_RESULT retvalue;                    /* Return values of NTCAN API calls */
    CAN_IF_STATUS cstat;
    char statusTxt[STATUS_TXT_LEN];

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "handle");
        return TCL_ERROR;
    }
    Tcl_GetWideIntFromObj(interp, objv[1], &handle);

    retvalue = canStatus((NTCAN_HANDLE)handle, &cstat);

    if (retvalue != NTCAN_SUCCESS) {
        FormatError(interp, "canStatus", retvalue);
        return TCL_ERROR;
    } else {
        snprintf(statusTxt, sizeof(statusTxt), "ID=%s\n"
                 "Dll=%1X.%1X.%02X\n"
                 "Drv=%1X.%1X.%02X\n"
                 "FW=%1X.%1X.%02X\n"
                 "HW=%1X.%1X.%02X\n"
                 "Status=%08x\n"
                 "Features=%04x",
                 cstat.boardid,
                 cstat.dll     >>12, (cstat.dll     >>8) & 0xf, cstat.dll      & 0xff,
                 cstat.driver  >>12, (cstat.driver  >>8) & 0xf, cstat.driver   & 0xff,
                 cstat.firmware>>12, (cstat.firmware>>8) & 0xf, cstat.firmware & 0xff,
                 cstat.hardware>>12, (cstat.hardware>>8) & 0xf, cstat.hardware & 0xff,
                 (unsigned long)cstat.boardstatus,
                 cstat.features);
        Tcl_AppendResult(interp, &statusTxt, NULL);
        return TCL_OK;
    }
}

int Ntcan_Init(Tcl_Interp *interp) {
    /* initialize Tcl stubs */
    if (Tcl_InitStubs(interp, "8.6", 0) == NULL)
        return TCL_ERROR;

    // create namespace
    if (Tcl_CreateNamespace(interp, NS_PREFIX, NULL, NULL) == NULL)
        return TCL_ERROR;

    // initialize operation
    Tcl_CreateObjCommand(interp, NS_PREFIX "Scan",               (Tcl_ObjCmdProc *)Scan, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Open",               (Tcl_ObjCmdProc *)Open, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Close",              (Tcl_ObjCmdProc *)Close, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetBaudrate",        (Tcl_ObjCmdProc *)SetBaudrate, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetBaudrate",        (Tcl_ObjCmdProc *)GetBaudrate, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetBaudrateX",       (Tcl_ObjCmdProc *)SetBaudrateX, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetBaudrateX",       (Tcl_ObjCmdProc *)GetBaudrateX, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "IdAdd",              (Tcl_ObjCmdProc *)IdAdd, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "IdRegionAdd",        (Tcl_ObjCmdProc *)IdRegionAdd, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "IdDelete",           (Tcl_ObjCmdProc *)IdDelete, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "IdRegionDelete",     (Tcl_ObjCmdProc *)IdRegionDelete, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "FlushRxFifo",        (Tcl_ObjCmdProc *)FlushRxFifo, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetRxMsgCount",      (Tcl_ObjCmdProc *)GetRxMsgCount, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetTxMsgCount",      (Tcl_ObjCmdProc *)GetTxMsgCount, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetRxTimeout",       (Tcl_ObjCmdProc *)GetRxTimeout, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetTxTimeout",       (Tcl_ObjCmdProc *)GetTxTimeout, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetRxTimeout",       (Tcl_ObjCmdProc *)SetRxTimeout, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetTxTimeout",       (Tcl_ObjCmdProc *)SetTxTimeout, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "AbortRx",            (Tcl_ObjCmdProc *)AbortRx, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "AbortTx",            (Tcl_ObjCmdProc *)AbortTx, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetBusStatistic",    (Tcl_ObjCmdProc *)GetBusStatistic, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetCtrlStatus",      (Tcl_ObjCmdProc *)GetCtrlStatus, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Read",               (Tcl_ObjCmdProc *)Read, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Write",              (Tcl_ObjCmdProc *)Write, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "ReadX",              (Tcl_ObjCmdProc *)ReadX, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "WriteX",             (Tcl_ObjCmdProc *)WriteX, 0, 0);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Status",             (Tcl_ObjCmdProc *)Status, 0, 0);

    // provide package information
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK)
        return TCL_ERROR;
  
    return TCL_OK;
}

int Ntcan_Unload(Tcl_Interp *interp, int flags) {
    // destroy operation.
    return TCL_OK;
}

