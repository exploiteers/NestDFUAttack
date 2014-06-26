/*
 *
 * Proxy Server header file for TI81XX VPSS
 *
 * Copyright (C) 2009 TI
 * Author: Yihe Hu <yihehu@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
 */

#ifndef __LINUX_VPS_PROXYSERVER_H__
#define __LINUX_VPS_PROXYSERVER_H__

#ifdef __KERNEL__

#define VPS_PSRV_MAX_NO_NOTIFY       (32u)
/* < Maximum number of stream supported by Proxy Server. This define
    could not be changed arbiterly, there is dependency on the number
    of notify available with sysLink. Please refer the sysLink documentation
    for details. */
#define VPS_PSRV_MAX_NO_OF_CORES       (12u)
/*< Maximum number of cores that Proxy Server could serve. */
#define VPS_PSRV_NUMBER_OF_TASKS       (0x5u)
/**< Used to specify the number of task that process requests from a remote
     agent / client.  Designed to be 5. Notify management would require
     change if this changes */


/**
 * @brief  This enumeration identifies the types of callbacks.
 *
 *    On completion of an IO request OR on encountring an error, the proxy
 *    Server would notify the remote agents / clients via an priviously
 *    allocate notification number during FVID2 stream creation.
 *
 *    Since notification happens on ONE notification channel Remote agents
 *    / clients could use this enumeration to differentiate between an
 *    error or IO completion notifications.
 */
enum vps_psrvcallbacktype {
	VPS_FVID2_IO_CALLBACK          =   (0x01u),
	/**< Specifies that callback is for IO completion */
	VPS_FVID2_ERR_CALLBACK         =   (0x02u)
	/**< Specifies that the callback is for Error */
};


/**
 * @brief  Place holder for IO Request Callback
 *    Remote applications requires to be notified on completion of an IO
 *    request on HDVPSS sub-system. The structure defined here is an place
 *    holder that requires to be allocated by the remote agent / client,
 *    and passed to the proxy server. Proxy Server would update and
 *    update / maintain this structure.
 *
 */

struct vps_psrvcallback {
	u32                      cbtype;
	/**< [OUT] Remote agents / clients could use this to determine if the
	     current notification is for IO completion event or error event */
	u32                      syslnkntyno;
	/**< [Reserved] for remote agent, expected to be used by Proxy Server.
	     Remote agents are expeted not to alter this */
	u32                      syslnkprocid;
	/**< [Reserved] for the remote agent, expected to be
	     used by Proxy Server. Remote agents are expeted not
	     to alter this */
	u32                      callbackrtnvalue;
	/**< [Reserved] Reserved for now, to be used if return
	     values requires to be handled in the HDVPSS sub-system */
	void                     *fvid2handle;
	/**< [OUT] Handle to HDVPSS. Remote clients / agents
	     should not alter this.*/
	int (*appcallback) (void *handle, void *appdata, void *reserved);
	/**< [OUT] Callback provided by the apps running on remote processor.
	     Expected to be used by the remote agent to notify apps - left
	     to remote agent to determine the use. Proxy Server will
	     not alter this.
	*/
	void                     *appdata;
	/**< [OUT] Argument for the apps provided callback on the remote agent.
	     Proxy Server will not alter this. */
	void                     *reserved;
	/**< [Reserved] Future requirements/enhancements if any */
};


/**
 * @brief  Place holder for Error Callback
 *	Remote applications requires to be notified on encountering an errors
 *	in the stream.
 *	The structure defined here is an place holder that requires to be
 *	allocated by the remote agent/client, and passed to the proxy server
 *	Proxy Server would update and update / maintain this structure.
 */

struct vps_psrverrorcallback {
	u32                      cbtype;
	/**< [OUT] Remote agents / clients could use this to determine if the
	     current notification is for IO completion event or error event */
	u32                      syslnkntyno;
	/**< [Reserved] for remote agent expected to be used by Proxy Server.
	     Agents are expected not alter this. */
	u32                      syslnkprocid;
	/**< [Reserved] for the remote agent, expected to be
	     used by Proxy Server. Remote agents are expeted not to
	     alter this */
	u32                      callbackrtnvalue;
	/**< [Reserved] Reserved for now, to be used if return values
	    requires to be handled in the HDVPSS sub-system */
	void                     *fvid2handle;
	/**< [OUT] Handle to HDVPSS. Remote clients / agents should
	      not alter this.*/
	int (*appcallback)(void *handle, void *appdata,
			void *errdata, void *reserved);
	/**< [OUT] Error Callback provided by the apps running
	     on remote processor. Expected to be used by the remote
	     agent to notify apps - left to remote agent to determine the
	     use. Proxy Server will not alter this. */
	void                     *errlist;
	/**< [OUT] Argument for the apps provided callback
	     on the remote agent. Proxy Server will not alter this. */
	void                     *reserved;
	/**< [Reserved] Future requirements/enhancements if any */
};



/**
 * @brief  This enumeration defines the types of commands that are supported by
 *   Proxy Server. The interpretation of the command structure
 *   (by Proxy Server) is defined type of command.
 *   A simplex command would indicate that a an single command is to be
 *   executed.
 *   A composite command would indicate that there are more the 1
 *   command that requires to be executed. Typically multiple Q and DQ
 */
enum vps_psrvcommandtype {
	VPS_FVID2_CMDTYPE_SIMPLEX        =   (0x01u),
	/**< Specifies that command is a simplex command */
	VPS_FVID2_CMDTYPE_COMPOSITE      =   (0x02u),
	/**< Specifies that the command is compound command and
	     all the commands in the command structure should be addressed
	     by the proxy server */
	VPS_FVID2_CMDTYPEMAX
	/**< Rear enumeration guard */

};


/**
 * @brief  Proxy Server Commands
 *   This enumeration defines the commands that would be honored by the
 *   HDVPSS sub-system/Proxy Server. These commands is expected to be
 *   exercised on a remote system that shares memory with the HDVPSS
 *   subsytem.
 *
 *   Each of these commands requires the function arguments in a specific
 *   format, type definitions like VPS_PSrvFvid2InitParams,
 *   VPS_PSrvFvid2CreateParams, define the expected order.
 *   Application/Driver running on the remote processor is referred as
 *   an remote agent / client.
 */
enum vps_psrvcommands {
	VPS_FVID2_INIT                      =   (0x01u),
	/**< The very first commands to VPS Subsystem. Expected to be
	     called once, either remotely or locally. Advised to exercise this
	     command before any operation. An return error code (TBD) will
	     mean that FVID2 has been initialized earlier and other FVID2
	     calls will be honored by HDVPSS sub-system */
	VPS_FVID2_DE_INIT                   =  (0x02u),
	/**< The last control command, used to terminate FVID2 */
	VPS_FVID2_CREATE                    =  (0x03u),
	/**< Creates the named stream, calls FVID2 API FVID2_create. */
	VPS_FVID2_DELETE                    =  (0x04u),
	/**< Deletes the names stream, calls FVID2 API FVID2_delete */
	VPS_FVID2_CONTROL                   =  (0x05u),
	/**< To control a stream, calls FVID2 API FVID2_control */
	VPS_FVID2_QUEUE                     =  (0x06u),
	/**< To submit buffer that would be consumed by the driver, calls
	     FVID2 API FVID2_queue */
	VPS_FVID2_DEQUEUE                   =  (0x07u),
	/**< To retrieve the buffers that were submitted, calls the FVID2 API
	     FVID2_processFrames */
	VPS_FVID2_PROCESS_FRAMES            =  (0x08u),
	/**< Similar to VPS_FVID2_QUEUE, expected to be used with
	     memory-to-memory  driver. Calls the FVID2 API
	     FVID2_processFrames */
	VPS_FVID2_GET_PROCESSED_FRAMES      =  (0x09u),
	/**< Similar to VPS_FVID2_DEQUEUE, expected to be used with
	     memory-to-memory  driver. Calls the FVID2 API
	     FVID2_getProcessedFrames */
	VPS_FVID2_GET_FIRMWARE_VERSION      =  (0x0Au)
	/**< Command used to determine the current version of the firmware,
	     running on the host processor.
	*/
};


/**
 * @brief  Proxy Server Host tasks type
 *          All client commands are processed by the host in a tasks context.
 *          Host spawns 4 tasks that would process these commands, typically
 *          one for display, capture, memory to memory and graphics.
 *          Applications could configure the host to associate an FVID2 channel
 *          with any of the tasks mentioned below.
 *
 *          Note that there would a control task that would process commands
 *          FVID2_init, FVID2_deInit and FVID2_create.
 *
 *          Please refer the user guide that came with this release for details.
 *
 */
enum vps_psrvhosttasktype {
	VPS_FVID2_TASK_TYPE_LOWER_GUARD =   (0x00u),
	/**< Lower guard */
	VPS_FVID2_TASK_TYPE_1           =   (0x01u),
	/**< Specifies that the command should be processed by task 1 */
	VPS_FVID2_TASK_TYPE_2           =   (0x02u),
	/**< Specifies that the command should be processed by task 2 */
	VPS_FVID2_TASK_TYPE_3           =   (0x03u),
	/**< Specifies that the command should be processed by task 3 */
	VPS_FVID2_TASK_TYPE_4           =   (0x04u),
	/**< Specifies that the command should be processed by task 4 */
	VPS_FVID2_TASK_TYPE_UPPER_GUARD
	/**< Upper guard */
};


/**
 * @brief  Command Structure for Proxy Server
 *   A remote agent that requires to instruct the proxy sever, would
 *   require to use the structure below. An instace of this structure will
 *   reuqired to be populated and notificted to proxy server.
 *   This structure will define the type of command (one or multiple),
 *   and a pointer to actual FVID2 command (array of pointer in case of
 *   multiple commands)
 */

struct vps_psrvcommandstruct {
	u32                      cmdtype;
	/**< [IN] Specifies the command type. Only VPS_FVID2_CMDTYPE_SIMPLEX is
		 supported at this point */
	u32                       yieldafterncmds;
	/**< [IN] Used only when the cmdType is VPS_FVID2_CMDTYPE_COMPOSITE,
		 In cases where IO on a given streams is very high which could
		 potentially cause CPU starve on other IO streams. In these
		 conditions remote agents / clients could decide to YIELD the
		 CPU after executing N number of commands of this stream.
		 Expected to be used in conjunction with priority of streams
		 to achieve real time balance between streams. */
	u32                       syslnkprocid;
	/**< [Reserved] Reserved for remote agents, would be used by
		proxy server to hold the Proc Id of the remote processor in
		case of command VPS_FVID2_CREATE */
	u32                       syslnkntyno;
	/**< [Reserved] Reserved for remote agents, would be used by
		proxy server to hold the event number associated with remote
		processor in case of command VPS_FVID2_DELETE */
	u32                       reserved;
	/**< [Reserved] Reserved for future use */
	int                       returnvalue;
	/**< [OUT] Place holder return value, if Proxy Server could
		 not process thiscommand.
		e.g.
		Un-Supported command type
		second command received while executing first command
		etc... */
	u32                       noofcommands;
	/**< [IN] Applicable in case of VPS_FVID2_CMDTYPE_COMPOSITE,
		specifies the number of commands
		\warning This should be 1 always.*/
	void                      *simplexcmdarg;
	/**< [IN] Used only when the cmdType is VPS_FVID2_CMDTYPE_SIMPLEX,
		this pointer will point to a structure defined by the command.
		e.g if the command is VPS_FVID2_QUEUE
		VPS_PSrvCommandStruc.cmdType  = VPS_FVID2_CMDTYPE_SIMPLEX
		and
		((VPS_PSrvFvid2QueueParams *)simplexCmdArg)->command =
			VPS_FVID2_QUEUE */
	void                      **compositecmdargs;
	/**< [IN] Used only when the cmdType is VPS_FVID2_CMDTYPE_COMPOSITE,
		this will point to array of void pointer, number elements
		in the array will be equals to noOfCommands.
		Type of each of the pointer contained in array will match the
		command used.
		\warning Commands VPS_FVID2_CREATE and VPS_FVID2_DELETE
		 will not supported in the composite type command.
		\warning This is not implemented. */

};
/**
 * @brief  determine the status of proxy server and revision.
 *
 *         this structure defines the structures that would be used query the
 *         status of the proxy server and query the revision of the firmware.
 *         remote clients / agents allocates the memory for this structure,
 *         initializes the function parameters and pass a pointer to this struct
 */
struct vps_psrvgetstatusvercmdparams {
	u32                       command;
	/**< [in] command from the agent - in this case
	vps_fvid2_get_firmware_version or
	vps_fvid2_get_status.
	#vps_psrvcommands lists all supported commands */
	u32                       reserved;
	/**< [reserved] reserved for future use */
	int                       returnvalue;
	/**< [out] place holder for commands executions return value.
	positive values indicating the length of version. an negative
	value in case of an error */
	u32                       status;
	/**< [out] update for command VPS_FVID2_GET_STATUS.
	positive indicates proxy server is ready to receive fivd2
	commands, from the clients */
	u32                       version;
	/**< [out] update for command VPS_FVID2_GET_FIRMWARE_VERSION.
	     It's expected to be interpreated as 4 byte Hexadecimal Value.
	*/
};



/**
 * @brief  FVID2 Initialization parameters
 *   This structure defines the structures that would be used during
 *   FVID2 initialization.
 *   Remote clients / agents allocates the memory for this structure,
 *   initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2initparams {
	u32                       command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_INIT */
	u32                       reserved;
	/**< [Reserved] Reserved for future use */
	int                       returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                      *args;
	/**< [IN] First Argument required by the FVID2 API */
};



/**
 * @brief  FVID2 De Initialization parameters
 *   This structure defines the structures that would be used during FVID2
 *   de initialization.
 *   Remote clients / agents allocates the memory for this structure,
 *   initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2deinitparams {
	u32                       command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_DE_INIT */
	u32                       reserved;
	/**< [Reserved] Reserved for future use */
	int                       returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                      *args;
	/**< [IN] First Argument required by the FVID2 API */
};



/**
 * @brief  FVID2 stream creation parameters
 *   This structure defines the structures that would be used during FVID2
 *   create stream.
 *   Remote clients / agents allocates the memory for this structure,
 *   initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2createparams {
	u32                              command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_CREATE */
	u32                              reserved;
	/**< [Reserved] Reserved for future use */
	u32                              hosttaskinstance;
	/**< specifies the host task type */
	u32                              syslnkntyno;
	/**< [OUT] Proxy Server allocates a notification number,
		remote clients agents are expected to use this number
		for future transactions on this stream. Please refer
		sysLink for details IPC notifications. Note that type of
		eventId is defined as UInt16 by sysLink, its explicitly
		defined as u32. */
	void                             *fvid2handle;
	/**< [OUT] Place holder for FVID2 API return value */
	u32                              drvid;
	/**< [IN] First Argument required by  FVID2_create */
	u32                              instanceid;
	/**< [IN] Second Argument required by FVID2_create */
	void                             *createargs;
	/**< [IN] Third Argument required by FVID2_create */
	void                             *createstatusargs;
	/**< [IN] Fourth Argument required by FVID2_create */
	struct fvid2_cbparams            *cbparams;
	/**< [IN] Fivth Argument required by FVID2_create */
	struct vps_psrvcallback          *ioreqcb;
	/**< [IN] Place holder defined in the remote system which will be
		populated by the Proxy Server and passed back to remote agent
		on completion of an IO request.*/
	struct vps_psrverrorcallback     *errcb;
	/**< [IN] Place holder defined in the remote system which will be
		populated by the Proxy Server and passed back to remote
		agent on a error. Please refer FVID2 API header for details */
};



/**
 * @brief  FVID2 stream deletion parameters
 *  This structure defines the structures that would be used during FVID2
 *	 stream deletion
 *	 Remote clients / agents allocates the memory for this structure,
 *	 initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2deleteparams {
	u32                 command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_DELETE */
	u32                 reserved;
	/**< [Reserved] Reserved for future use */
	int                 returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                *fvid2handle;
	/**< [IN] Place holder for FVID2 stream handle */
	void                *deleteargs;
	/**< [IN] First Argument required by the FVID2 API */
};



/**
 * @brief  FVID2 stream control parameters
 *  This structure defines the structures that would be used to
 *  control a previously  created stream
 *  Remote clients / agents allocates the memory for this structure,
 *  initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2controlparams {
	u32                  command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_CONTROL */
	u32                  reserved;
	/**< [Reserved] Reserved for future use */
	int                  returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                 *fvid2handle;
	/**< [IN] FVID2 stream handle */
	u32                  cmd;
	/**< [IN] command refer FVID2 API */
	void                 *cmdargs;
	/**< [IN] First Argument required by the FVID2 API refer FVID2 API */
	void                 *cmdstatusargs;
	/**< [IN] Second Argument required by the FVID2 API refer FVID2 API */
};

/**
 * @brief  FVID2 queue IO requests
 *	This structure defines the structures that would be used queue an IO
 *	request.
 *	Remote clients / agents allocates the memory for this structure,
 *	initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2queueparams {
	u32                         command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_QUEUE */
	u32                         reserved;
	/**< [Reserved] Reserved for future use */
	int                         returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                        *fvid2handle;
	/**< [IN] FVID2 stream handle */
	struct fvid2_framelist      *framelist;
	/**< [IN] First Argument required by the FVID2 API refer FVID2 API */
	u32                          streamid;
	/**< [IN] Second Argument required by the FVID2 API refer FVID2 API */
};



/**
 * @brief  FVID2 dequeue IO requests
 *	This structure defines the structures that would be used dequeue
 *	completed IO request.
 *	Remote clients / agents allocates the memory for this structure,
 *	initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2dequeueparams {
	u32                           command;
	/**< [IN] Command from the agent - in this case VPS_FVID2_DEQUEUE */
	u32                           reserved;
	/**< [Reserved] Reserved for future use */
	int                           returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                          *fvid2handle;
	/**< [IN] FVID2 stream handle */
	struct fvid2_framelist        *framelist;
	/**< [IN] First Argument required by the FVID2 API refer FVID2 API */
	u32                           streamid;
	/**< [IN] Second Argument required by the FVID2 API refer FVID2 API */
	u32                           timeout;
	/**< [IN] third Argument required by the FVID2 API refer FVID2 API */
};



/**
 * @brief  FVID2 Process Frames
 *	This structure defines the structures that would be used submit
 *	frames to be processed.
 *	Remote clients / agents allocates the memory for this structure,
 *	initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2processframesparams {
	u32                         command;
	/**< [IN] Command from the agent -VPS_FVID2_PROCESS_FRAMES */
	u32                         reserved;
	/**< [Reserved] Reserved for future use */
	int                         returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                        *fvid2handle;
	/**< [IN] FVID2 stream handle */
	struct fvid2_processlist    *processlist;
	/**< [IN] First Argument required by the FVID2 API refer FVID2 API */
};



/**
 * @brief  FVID2 Retrieve Processed Frames
 *	This structure defines the structures that would be used retrieve
 *	processed frames
 *	Remote clients / agents allocates the memory for this structure,
 *	initializes the function parameters and pass a pointer to this struct
 *
 */
struct vps_psrvfvid2getprocessedframesparams {
	u32                          command;
	/**< [IN] Command from the agent - in this case
		VPS_FVID2_GET_PROCESSED_FRAMES */
	u32                          reserved;
	/**< [Reserved] Reserved for future use */
	int                          returnvalue;
	/**< [OUT] Place holder for FVID2 API return value */
	void                         *fvid2_handle;
	/**< [IN] FVID2 stream handle */
	struct fvid2_processlist     *processlist;
	/**< [OUT] First Argument required by the FVID2 API refer FVID2 API */
	u32                          timeout;
	/**< [IN] third Argument required by the FVID2 API refer FVID2 API */
};


#endif

#endif /* _VPS_PROXY_SERVER_H */
