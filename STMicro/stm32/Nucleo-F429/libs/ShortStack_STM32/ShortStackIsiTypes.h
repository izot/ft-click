/*
 * Filename: ShortStackIsiTypes.h
 *
 * Description: This file contains type definitions related to
 * Interoperable Self-Installation (ISI).
 *
 * Copyright (c) Echelon Corporation 2008-2015.  All rights reserved.
 *
 * License:
 * Use of the source code contained in this file is subject to the terms
 * of the Echelon Example Software License Agreement which is available at
 * www.echelon.com/license/examplesoftware/.
 */

#ifndef SHORTSTACK_ISI_TYPES_H
#define SHORTSTACK_ISI_TYPES_H

/*
 * *****************************************************************************
 * TITLE: SHORTSTACK ISI TYPES
 * *****************************************************************************
 *
 * Definitions of the enumerations and data types required by the Interoperable
 * Self-Installation (ISI) portion of the ShortStack LonTalk Compact API.
 */

/*
 * Note this file is best included through ShortStackDev.h, which is generated by the
 * LonTalk Interface Developer.
 */
#ifndef _LON_PLATFORM_H
#   error You must include LonPlatform.h first (prefer including ShortStackDev.h)
#endif  /* _LON_PLATFORM_H */

#ifndef DEFINED_SHORTSTACKDEV_H
#   error You must include ShortStackDev.h first
#endif  /*  DEFINED_SHORTSTACKDEV_H */

#if	LON_ISI_ENABLED

/*
 * Pull in platform specific pragmas, definitions, and so on.
 * For example, set packing directives to align objects on byte boundary.
 */
#ifdef  INCLUDE_LON_BEGIN_END
#   include "LonBegin.h"
#endif  /* INCLUDE_LON_BEGIN_END */

#include "ShortStackTypes.h"

/*
 * Basic ISI API macros
 */
#define ISI_DEFAULT_GROUP   128u
#define ISI_NO_ASSEMBLY     255u
#define ISI_NO_INDEX        255u

/*
 *  Enumeration: IsiEvent
 *  Enumeration for different event types.
 *
 *  This enumeration represents the possible events that are passed to the
 *  host when the ISI engine does a callback.
 */
typedef LON_ENUM_BEGIN(IsiEvent)
{
    IsiNormal           = 0,
    IsiRun              = 1,
    IsiPending          = 2,
    IsiApproved         = 3,
    IsiImplemented      = 4,
    IsiCancelled        = 5,
    IsiDeleted          = 6,
    IsiWarm             = 7,
    IsiPendingHost      = 8,
    IsiApprovedHost     = 9,
    IsiAborted          = 10,
    IsiRetry            = 11,
    IsiWink             = 12,
    IsiRegistered       = 13
}
LON_ENUM_END(IsiEvent);

/*
 *  Enumeration: IsiType
 *  Enumeration for different engine types.
 *
 *  This enumeration specifies the ISI protocol to be implemented by the ISI
 *  engine.
 */
typedef LON_ENUM_BEGIN(IsiType)
{
    IsiTypeS,                       /* Use for ISI-S and ISI-S/C */
    IsiTypeDa,                      /* Use for Isi-DA and Isi-DA/C */
    IsiTypeDas                      /* Use for ISI-DAS and ISI-DAS/C */
}
LON_ENUM_END(IsiType);

/*
 *  Enumeration: IsiStartFlags
 *  Enumeration for option flags used while starting the ISI engine.
 *
 *  This enumeration represents option flags for the <IsiStart> function used to
 *  start the ISI engine. Use a combination of these flags with the <IsiStart> function.
 */
typedef LON_ENUM_BEGIN(IsiStartFlags)
{
    IsiFlagNone                 = 0,    /* Does nothing */
    IsiFlagExtended             = 1,    /* Enables use of extended DRUM and enrollment messages */
    IsiFlagHeartbeat            = 2,    /* Enables ISI NV heartbeats */
    IsiFlagApplicationPeriodic  = 4,    /* Enables IsiApplicationPeriodic() */
    IsiFlagSupplyDiagnostics    = 8     /* Enables UpdateDiagnostics callback */
}
LON_ENUM_END(IsiStartFlags);

/*
 *  Enumeration: IsiDirection
 *  Enumeration for the direction of a network variable in a connection.
 *
 *  This enumeration represents the direction of the network variable
 *  on offer in a CSMO.
 */
typedef LON_ENUM_BEGIN(IsiDirection)
{
    IsiDirectionOutput          = 0,
    IsiDirectionInput           = 1,
    IsiDirectionAny             = 2,
    IsiDirectionVarious         = 3
}
LON_ENUM_END(IsiDirection);

/*
 *  Enumeration: IsiScope
 *  Enumeration for scope of a connection message.
 *
 *  This enumeration represents the different values that can be used in
 *  the scope field of a CSMO message. This value represents the scope of the
 *  resource file containing the functional profile and network variable type
 *  definitions specified by the Profile and NvType fields.
 */
typedef LON_ENUM_BEGIN(IsiScope)
{
    IsiScopeStandard            = 0,
    IsiScopeManufacturer        = 3
}
LON_ENUM_END(IsiScope);

/*
 *  Typedef: IsiConnectionId
 *  Structure representing the unique connection ID.
 *
 *  This structure contains the unique connection ID for a connection.
 */
typedef LON_STRUCT_BEGIN(IsiConnectionId)
{
    /* A unique identifier for the connection host, based on the host�s unique ID. */
    LonByte         UniqueId[LON_UNIQUE_ID_LENGTH - 1];
    /* Connection host-allocated serial number.*/
    LonWord         SerialNumber;
}
LON_STRUCT_END(IsiConnectionId);

/*
 *  Typedef: IsiConnectionHeader
 *  Structure representing the connection header.
 *
 *  Following the ISI Message Header, all connection-related messages start with
 *  this structure.
 */
typedef LON_STRUCT_BEGIN(IsiConnectionHeader)
{
    /* See <IsiConnectionId>. */
    IsiConnectionId     ConnectionId;
    /* Selector value 0 � 0x2FFF. The most significant 2 bits must be cleared
       and are reserved for future extension. */
    LonWord             Selector;
}
LON_STRUCT_END(IsiConnectionHeader);

/*
 *  Typedef: IsiConnection
 *  Structure representing a row in the connection table.
 *
 *  This structure is used to represent a row in the connection table that
 *  is returned by <IsiGetConnection> and is used in <IsiSetConnection> to set a
 *  row in the table.
 */

/*
 * Use the ISI_CONN_STATE_* macros to access the State field in IsiConnection.Attributes1
 */
#define ISI_CONN_STATE_MASK     0xC0
#define ISI_CONN_STATE_SHIFT    6
#define ISI_CONN_STATE_FIELD    Attributes1

/*
* Use the ISI_CONN_EXTEND_* macros to access the Extend field in IsiConnection.Attributes1
*/
#define ISI_CONN_EXTEND_MASK    0x20
#define ISI_CONN_EXTEND_SHIFT   5
#define ISI_CONN_EXTEND_FIELD   Attributes1

/*
 * Use the ISI_CONN_CSME_* macros to access the State field in IsiConnection.Attributes1
 */
#define ISI_CONN_CSME_MASK      0x10
#define ISI_CONN_CSME_SHIFT     4
#define ISI_CONN_CSME_FIELD Attributes1

/*
 * Use the ISI_CONN_WIDTH_* macros to access the State field in IsiConnection.Attributes1
 */
#define ISI_CONN_WIDTH_MASK     0x0F
#define ISI_CONN_WIDTH_SHIFT    0
#define ISI_CONN_WIDTH_FIELD    Attributes1

/*
 * Use the ISI_CONN_OFFSET_* macros to access the Auto field in IsiConnection.Attributes2
 */
#define ISI_CONN_OFFSET_MASK    0xFC
#define ISI_CONN_OFFSET_SHIFT   2
#define ISI_CONN_OFFSET_FIELD   Attributes2

/*
 * Use the ISI_CONN_AUTO_* macros to access the Auto field in IsiConnection.Attributes2
 */
#define ISI_CONN_AUTO_MASK      0x02
#define ISI_CONN_AUTO_SHIFT     1
#define ISI_CONN_AUTO_FIELD     Attributes2

typedef LON_STRUCT_BEGIN(IsiConnection)
{
    IsiConnectionHeader Header;
    LonByte     Host;
    LonByte     Member;
    LonByte     Attributes1;    /* contains state, extend, csme, width. See ISI_CONN_STATE_*, _EXTEND_*, _CSME_* and _WIDTH_* macros */
    LonByte     Attributes2;    /* contains offset, auto. See ISI_CONN_OFFSET_* and _AUTO_* macros */
}
LON_STRUCT_END(IsiConnection);

/*
 *  Typedef: IsiCsmoData
 *  Structure representing the CSMO message.
 *
 *  This structure contains the fields of a CSMO message to be sent by the
 *  ISI engine.
 */

/*
 * Use the ISI_CSMO_DIR_* macros to access the Direction field in IsiCsmoData.Attributes1
 */
#define ISI_CSMO_DIR_MASK       0xC0
#define ISI_CSMO_DIR_SHIFT      6
#define ISI_CSMO_DIR_FIELD      Attributes1

/*
 * Use the ISI_CSMO_WIDTH_* macros to access the Width field in IsiCsmoData.Attributes1
 */
#define ISI_CSMO_WIDTH_MASK     0x3F
#define ISI_CSMO_WIDTH_SHIFT    0
#define ISI_CSMO_WIDTH_FIELD    Attributes1

/*
 * Use the ISI_CSMO_ACK_* macros to access the Acknowledged field in IsiCsmoData.Attributes2
 */
#define ISI_CSMO_ACK_MASK       0x80
#define ISI_CSMO_ACK_SHIFT      7
#define ISI_CSMO_ACK_FIELD      Attributes2

/*
 * Use the ISI_CSMO_POLL_* macros to access the Poll field in IsiCsmoData.Attributes2
 */
#define ISI_CSMO_POLL_MASK      0x40
#define ISI_CSMO_POLL_SHIFT     6
#define ISI_CSMO_POLL_FIELD     Attributes2

/*
 * Use the ISI_CSMO_SCOPE_* macros to access the Scope field in IsiCsmoData.Attributes2
 */
#define ISI_CSMO_SCOPE_MASK     0x30
#define ISI_CSMO_SCOPE_SHIFT    4
#define ISI_CSMO_SCOPE_FIELD    Attributes2

typedef LON_STRUCT_BEGIN(IsiCsmoData)
{
    /* The group (or: device category) that this connection applies to. */
    LonByte     Group;
    LonByte     Attributes1;    /* contains Direction, Width. See ISI_CSMO_DIR_* and _WIDTH_* macros */
    /* Functional profile number of the functional profile that defines the
       functional block containing this input or output, or zero if none. */
    LonWord     Profile;
    /* NV type index of the NV type for the network variable, or zero if none
       specified. The NV type index is an index into resource file that defines the
       network variable type for the network variable on offer. */
    LonByte     NvType;
    /* Variant number for the offered network variable. Variants can be defined for
       any functional profile/member number pair. */
    LonByte     Variant;
    LON_STRUCT_NESTED_BEGIN(Extended) {
        LonByte     Attributes2;    /* contains Ack, Poll, Scope. See ISI_CSMO_ACK_*, _POLL_* and _SCOPE_* macros */
        /* The first 6 bytes of the connection host�s standard program ID.
           The last two standard program ID bytes (channel type and model
           number) are not included. */
        LonByte     Application[LON_PROGRAM_ID_LENGTH - 2];
        /* NV member number within the functional block, or zero if none. */
        LonByte     Member;
    }
    LON_STRUCT_NESTED_END(Extended);
}
LON_STRUCT_END(IsiCsmoData);

/*
 * This flag determines whether a message is supposed to be responded to.
 */
#define IsiRpcUnacknowledged        ((signed) 0x80)

/*
 *  Enumeration: IsiDownlinkRpcCode
 *  Codes for downlink calls from the host to the Micro Server.
 *
 *  These are the function identifiers for downlink API calls.
 *  Numbers are assigned explicitly just to indicate the import
 *  of maintaining certain number values as these must be consistent
 *  with the Micro Server's implementation.
 */
typedef LON_ENUM_BEGIN(IsiDownlinkRpcCode)
{
    IsiRpcStop                      = 0,
    IsiRpcStart                     = 1,
    IsiRpcReturnToFactoryDefaults   = 2,
    IsiRpcAcquireDomain             = 3,
    IsiRpcStartDeviceAcquisition    = 4,
    IsiRpcOpenEnrollment            = 5,
    IsiRpcCreateEnrollment          = 6,
    IsiRpcExtendEnrollment          = 7,
    IsiRpcCancelEnrollment          = 8,
    IsiRpcLeaveEnrollment           = 9,
    IsiRpcDeleteEnrollment          = 10,
    IsiRpcInitiateAutoEnrollment    = 11,
    IsiRpcIsConnected               = 12,
    IsiRpcImplementationVersion     = 13,
    IsiRpcProtocolVersion           = 14,
    IsiRpcIsBecomingHost            = 15,
    IsiRpcIsRunning                 = 16,
    IsiRpcCancelAcquisition         = 17,
    IsiRpcFetchDevice               = 18,
    IsiRpcFetchDomain               = 19,
    IsiRpcIssueHeartbeat            = 20
}
LON_ENUM_END(IsiDownlinkRpcCode);

/*
 *  Enumeration: IsiUplinkRpcCode
 *  Codes for uplink callbacks from the Micro Server to the host.
 *
 *  These are the callback identifiers for uplink API calls.
 *  Numbers are assigned explicitly just to indicate the import
 *  of maintaining certain number values as these must be consistent
 *  with the Micro Server's implementation.
 */
typedef LON_ENUM_BEGIN(IsiUplinkRpcCode)
{
    IsiRpcCreatePeriodicMsg         = 0,
    IsiRpcUpdateUserInterface       = 1  | IsiRpcUnacknowledged,
    IsiRpcCreateCsmo                = 2,
    IsiRpcGetPrimaryGroup           = 3,
    IsiRpcGetAssembly               = 4,
    IsiRpcGetNextAssembly           = 5,
    IsiRpcGetNvIndex                = 6,
    IsiRpcGetNextNvIndex            = 7,
    IsiRpcGetPrimaryDid             = 8,
    IsiRpcGetWidth                  = 9,
    IsiRpcGetNvValue                = 10,
    IsiRpcGetConnTabSize            = 11,
    IsiRpcGetConnection             = 12,
    IsiRpcSetConnection             = 13 | IsiRpcUnacknowledged,
    IsiRpcQueryHeartbeat            = 14,
    IsiRpcGetRepeatCount            = 15,
    IsiRpcUserCommand               = 64
}
LON_ENUM_END(IsiUplinkRpcCode);

/*
 *  Typedef: IsiRpcMessage
 *  Structure defining the data format in the RPC messages between the host and the Micro Server.
 *
 *  This structure defines the format of the data in the ISI remote procedure call messages.
 */
typedef LON_UNION_BEGIN(IsiRpcMessage)
{
    LonSmipMsg  smipMsg;
    LON_STRUCT_NESTED_BEGIN(rpcMsg) {
        LonSmipHdr  Header;         /* Message header, where command can be one of IsiCmd, IsiAck, IsiNack */
        LonByte     RpcCode;        /* RPC Function Code */
        LonByte     SequenceNumber; /* A sequence number for correlation */
        LonByte     Parameters[2];  /* Two 1-byte parameters */
        LON_STRUCT_NESTED_BEGIN(RpcData) {
            LonByte Length;         /* Length of the complete RpcData structure */
            LonByte Data[31];       /* Contains data like IsiCsmoData, IsiConnection and so on. */
        }
        LON_STRUCT_NESTED_END(RpcData);
    }
    LON_STRUCT_NESTED_END(rpcMsg);
}
LON_STRUCT_END(IsiRpcMessage);

#define IsiRpcMessageLength(p)  ((p)->rpcMsg.RpcData.Length + offsetof(IsiRpcMessage, rpcMsg.RpcData) - sizeof((p)->rpcMsg.Header))

/*
 * Turn off platform specific pragmas, definitions, and so on.
 * For example, restore packing directives.
 */
#ifdef  INCLUDE_LON_BEGIN_END
#   include "LonEnd.h"
#endif  /* INCLUDE_LON_BEGIN_END */

#endif	//	LON_ISI_ENABLED
#endif /* SHORTSTACK_ISI_TYPES_H */
