/*
 * Copyright (C) 2013 NVIDIA Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DRIVERS_STAGING_NVSHM_NVSHM_RPC_UTILS_H
#define __DRIVERS_STAGING_NVSHM_NVSHM_RPC_UTILS_H

#include <linux/sunrpc/xdr.h>
#include <linux/sunrpc/msg_prot.h>
#include "nvshm_rpc.h"

#define NVSHM_RPC_IN_UINT(data) { TYPE_UINT, 0, .d.uint_data = (data) }
#define NVSHM_RPC_IN_SINT(data) { TYPE_SINT, 0, .d.sint_data = (data) }
#define NVSHM_RPC_IN_STRING(data) { TYPE_STRING, 0, .d.string_data = (data) }
#define NVSHM_RPC_IN_BLOB(data, len) { TYPE_BLOB, (len), .d.blob_data = (data) }

#define NVSHM_RPC_OUT_UINT(data) { TYPE_UINT, 0, .d.uint_data = (data) }
#define NVSHM_RPC_OUT_SINT(data) { TYPE_SINT, 0, .d.sint_data = (data) }
#define NVSHM_RPC_OUT_STRING(data) { TYPE_STRING, 0, .d.string_data = (data) }
#define NVSHM_RPC_OUT_BLOB(data, len) \
				 { TYPE_BLOB, &(len), .d.blob_data = (data) }

/** Known types of data */
enum nvshm_rpc_datumtype {
	TYPE_SINT,
	TYPE_UINT,
	TYPE_STRING,
	TYPE_BLOB,
};

/**
 * Type for an input function parameter
 *
 * @param type Type of data
 * @param length Length for blob
 * @param *data Pointer to the value
 */
struct nvshm_rpc_datum_in {
	enum nvshm_rpc_datumtype type;
	u32 length;
	union {
		s32 sint_data;
		u32 uint_data;
		const char *string_data;
		const char *blob_data;
	} d;
};

/**
 * Type for an output function parameter
 *
 * @param type Type of data
 * @param length Length for blob
 * @param *data Pointer to the value
 */
struct nvshm_rpc_datum_out {
	enum nvshm_rpc_datumtype type;
	u32 *length;
	union {
		s32 *sint_data;
		u32 *uint_data;
		const char **string_data;
		const void **blob_data;
	} d;
};

/**
 * Type for a procedure
 *
 * @param program Program ID
 * @param version Program version
 * @param procedure Procedure ID
 */
struct nvshm_rpc_procedure {
	u32 program;
	u32 version;
	u32 procedure;
};

/**
 * Type for a function
 *
 * @param version Version requested by caller
 * @param request Request message
 * @param response Response message pointer to fill in
 * @return Accept status (should be either RPC_SUCCESS or RPC_SYSTEM_ERR)
 */
typedef enum rpc_accept_stat (*nvshm_rpc_function_t)(
	u32 version,
	struct nvshm_rpc_message *request,
	struct nvshm_rpc_message **response);

/**
 * Determines the size of message buffer to allocate given function data
 *
 * NOTE: this function accounts for header data too
 *
 * @param is_response Whether this is a response, or a request
 * @param data Function parameters
 * @param number Number of function parameters
 * @return Size to allocate on success, negative on error
 */
int nvshm_rpc_utils_encode_size(
	bool is_response,
	const struct nvshm_rpc_datum_in *data,
	u32 number);

/**
 * Populate a request buffer given procedure and data
 *
 * @param procedure Unique ID of the procedure
 * @param data Function parameters
 * @param number Number of function parameters
 * @param request Message to populate
 * @return 0 on success, negative on error
 */
int nvshm_rpc_utils_encode_request(
	const struct nvshm_rpc_procedure *procedure,
	const struct nvshm_rpc_datum_in *data,
	u32 number,
	struct nvshm_rpc_message *request);

/**
 * Populate a response buffer given procedure and data
 *
 * @param status Accept status of the request
 * @param data Function parameters
 * @param number Number of function parameters
 * @param response Message to populate
 * @return 0 on success, negative on error
 */
int nvshm_rpc_utils_encode_response(
	enum rpc_accept_stat status,
	const struct nvshm_rpc_datum_in *data,
	u32 number,
	struct nvshm_rpc_message *response);

/**
 * Returns the procedure info from a request message
 *
 * NOTE: the return code is garbage if used on a response message
 *
 * @param procedure Procedure to fill in
 * @param request Message to read from
 */
void nvshm_rpc_utils_decode_procedure(
	const struct nvshm_rpc_message *request,
	struct nvshm_rpc_procedure *procedure);

/**
 * Returns the error number from a response message
 *
 * NOTE: the return code is garbage if used on a request message
 *
 * @param response Message to read from
 * @return Accept status
 */
enum rpc_accept_stat nvshm_rpc_utils_decode_status(
	const struct nvshm_rpc_message *response);

/**
 * Returns the low and high version numbers supported
 *
 * NOTE: the return code is garbage if not used on a RPC_PROG_MISMATCH message
 *
 * @param response Message to read from
 * @param version_min Minimum version supported by service for this program
 * @param version_max Maximum version supported by service for this program
 * @return 0 on success, negative on error
 */
int nvshm_rpc_utils_decode_versions(
	const struct nvshm_rpc_message *response,
	u32 *version_min,
	u32 *version_max);

/**
 * Decode a message buffer to fill up function data
 *
 * This function takes a message buffer and runs through it to fill up the
 * function parameters to be returned.
 *
 * @param message Message to read from
 * @param is_response Whether this is a response, or a request
 * @param data Function parameters to be filled in
 * @param number Number of possible function parameters
 * @return 0 on success, negative on error
 */
int nvshm_rpc_utils_decode_args(
	const struct nvshm_rpc_message *message,
	bool is_response,
	struct nvshm_rpc_datum_out *data,
	u32 number);

#endif /* #ifndef __DRIVERS_STAGING_NVSHM_NVSHM_RPC_UTILS_H */