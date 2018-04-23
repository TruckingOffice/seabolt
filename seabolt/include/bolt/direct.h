/*
 * Copyright (c) 2002-2018 "Neo Technology,"
 * Network Engine for Objects in Lund AB [http://neotechnology.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 */

#ifndef SEABOLT_CONNECT
#define SEABOLT_CONNECT

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "addressing.h"
#include "config.h"


typedef unsigned long long bolt_request_t;


/**
 *
 */
enum BoltTransport
{
    BOLT_SOCKET,
    BOLT_SECURE_SOCKET,
};

/**
 *
 */
enum BoltConnectionStatus
{
    BOLT_DISCONNECTED,          // not connected
    BOLT_CONNECTED,             // connected but not authenticated
    BOLT_READY,                 // connected and authenticated
    BOLT_FAILED,                // recoverable failure
    BOLT_DEFUNCT,               // unrecoverable failure
};

/**
 *
 */
enum BoltConnectionError
{
    BOLT_NO_ERROR,
    BOLT_UNKNOWN_ERROR,
    BOLT_UNSUPPORTED,
    BOLT_INTERRUPTED,
    BOLT_NO_VALID_ADDRESS,
    BOLT_TIMED_OUT,
    BOLT_PERMISSION_DENIED,
    BOLT_OUT_OF_FILES,
    BOLT_OUT_OF_MEMORY,
    BOLT_OUT_OF_PORTS,
    BOLT_CONNECTION_REFUSED,
    BOLT_NETWORK_UNREACHABLE,
    BOLT_TLS_ERROR,             // general catch-all for OpenSSL errors :/
    BOLT_PROTOCOL_VIOLATION,
    BOLT_END_OF_TRANSMISSION,
};

struct BoltConnectionMetrics
{
    struct timespec time_opened;
    struct timespec time_closed;
    unsigned long long bytes_sent;
    unsigned long long bytes_received;
};

/**
 * A Bolt client-server connection instance.
 *
 */
struct BoltConnection
{
    /// The agent currently responsible for using this connection
    const void * agent;

    /// Transport type for this connection
    enum BoltTransport transport;

    /// The security context (secure connections only)
    struct ssl_ctx_st* ssl_context;
    /// A secure socket wrapper (secure connections only)
    struct ssl_st* ssl;
    /// The raw socket that backs this connection
    int socket;

    /// The protocol version used for this connection
    int32_t protocol_version;
    /// State required by the protocol
    void* protocol_state;

    // These buffers contain data exactly as it is transmitted or
    // received. Therefore for Bolt v1, chunk headers are included
    // in these buffers

    /// Transmit buffer
    struct BoltBuffer* tx_buffer;
    /// Receive buffer
    struct BoltBuffer* rx_buffer;

    /// Connection metrics
    struct BoltConnectionMetrics metrics;
    /// Current status of the connection
    enum BoltConnectionStatus status;
    /// Current connection error code
    enum BoltConnectionError error;
};

enum BoltAuthScheme
{
    BOLT_AUTH_BASIC,
};

/**
 * The profile of a database user.
 */
struct BoltUserProfile
{
    enum BoltAuthScheme auth_scheme;
    const char * user;
    const char * password;
    const char * user_agent;
};


/**
 *
 *
 * @return
 */
PUBLIC struct BoltConnection * BoltConnection_create();

/**
 *
 */
PUBLIC void BoltConnection_destroy(struct BoltConnection* connection);

/**
 * Open a connection to a Bolt server.
 *
 * This function attempts to connect a BoltConnection to _address_ over
 * _transport_. The `address` should be a pointer to a `BoltAddress` struct
 * that has been successfully resolved.
 *
 * This function blocks until the connection attempt succeeds or fails.
 * On returning, the connection status will be set to either `BOLT_CONNECTED`
 * (if successful) or `BOLT_DEFUNCT` (if not). If defunct, the error code for
 * the connection will be set to one of the following:
 *
 * @verbatim embed:rst:leading-asterisk
 * ========================  ====================================================================
 * Error code                Description
 * ========================  ====================================================================
 * BOLT_UNRESOLVED_ADDRESS   The supplied address has not been resolved.
 * BOLT_CONNECTION_REFUSED   The remote server refused to accept the connection.
 * BOLT_INTERRUPTED          The connection attempt was interrupted.
 * BOLT_NETWORK_UNREACHABLE  The server address is on an unreachable network.
 * BOLT_OUT_OF_FILES         The system limit on the total number of open files has been reached.
 * BOLT_OUT_OF_MEMORY        Insufficient memory is available.
 * BOLT_OUT_OF_PORTS         No more local ports are available.
 * BOLT_PERMISSION_DENIED    The current process does not have permission to create a connection.
 * BOLT_TIMED_OUT            The connection attempt timed out.
 * BOLT_TLS_ERROR            An error occurred while attempting to secure the connection.
 * BOLT_UNKNOWN_ERROR        An error occurred for which no further detail can be determined.
 * BOLT_UNSUPPORTED          One or more connection parameters are unsupported.
 * ========================  ====================================================================
 * @endverbatim
 *
 * @param connection the connection to open
 * @param transport the type of transport over which to connect
 * @param address descriptor of the remote Bolt server address
 * @return 0 if the connection was opened successfully, -1 otherwise
 */
PUBLIC int BoltConnection_open_b(struct BoltConnection * connection, enum BoltTransport transport, struct BoltAddress * address);

/**
 * Close a connection.
 *
 * @param connection
 */
PUBLIC void BoltConnection_close_b(struct BoltConnection* connection);

/**
 * Initialise the connection and authenticate using the basic
 * authentication scheme.
 *
 * @param connection the connection to initialise
 * @param profile credentials for a database user
 * @return
 */
PUBLIC int BoltConnection_init_b(struct BoltConnection * connection, const struct BoltUserProfile * profile);

/**
 * Reset the connection to discard any outstanding results,
 * rollback the current transaction and clear any unacknowledged
 * failures.
 *
 * @param connection
 * @return
 */
PUBLIC int BoltConnection_reset_b(struct BoltConnection * connection);

/**
 * Send all queued requests.
 *
 * @param connection
 * @return the latest request ID
 */
PUBLIC int BoltConnection_send_b(struct BoltConnection * connection);

/**
 * Take an exact amount of data from the receive buffer, deferring to
 * the socket if not enough data is available.
 *
 * @param connection
 * @param buffer
 * @param size
 * @return
 */
PUBLIC int BoltConnection_receive_b(struct BoltConnection * connection, char * buffer, int size);

/**
 * Fetch the next value from the result stream for a given request.
 * This will discard the responses of earlier requests that have not
 * already been fully consumed. This function will always consume at
 * least one record from the result stream and is not able to check
 * whether the given request has already been fully consumed; doing
 * so is the responsibility of the calling application.
 *
 * After calling this function, the value returned by
 * `BoltConnection_data` should contain either record data
 * (stored in a `BOLT_LIST`) or summary metadata (in a `BOLT_SUMMARY`).
 *
 * This function will block until an appropriate value has been fetched.
 *
 * @param connection the connection to fetch from
 * @param request the request for which to fetch a response
 * @return 1 if record data is received,
 *         0 if summary metadata is received,
 *         -1 if an error occurs
 *
 */
PUBLIC int BoltConnection_fetch_b(struct BoltConnection * connection, bolt_request_t request);

/**
 * Fetch values from the result stream for a given request, up to and
 * including the next summary. This will discard any unconsumed result
 * data for this request as well as the responses of earlier requests
 * that have not already been fully consumed. This function will always
 * consume at least one record from the result stream and is not able
 * to check whether the given request has already been fully consumed;
 * doing so is the responsibility of the calling application.
 *
 * After calling this function, the value returned by
 * `BoltConnection_data` should contain the summary metadata of the
 * received result (in a `BOLT_SUMMARY`).
 *
 * @param connection the connection to fetch from
 * @param request the request for which to fetch a response
 * @return >=0 the number of records discarded from this result
 *         -1 if an error occurs
 */
PUBLIC int BoltConnection_fetch_summary_b(struct BoltConnection * connection, bolt_request_t request);

/**
 * Obtain a pointer to the last fetched data values or summary metadata.
 *
 * Every call to a receive function, such as `BoltConnection_fetch_b`,
 * will fetch one or more values from the remote data stream. Each value
 * is written into a fixed slot within the `BoltConnection` struct and it
 * is the pointer to this that is returned. Values will either be of type
 * `BOLT_SUMMARY` (for summary metadata) or `BOLT_LIST` (for a sequence
 * of record values).
 *
 * Since the storage slot is recycled for each value received, pointers
 * will become invalid after subsequent receive function calls.
 *
 * @param connection
 * @return pointer to a `BoltValue` data structure
 */
PUBLIC struct BoltValue * BoltConnection_data(struct BoltConnection * connection);

/**
 * Set the next Cypher statement template to be run on this connection
 * from a null-terminated string.
 *
 * @param connection
 * @param cypher
 * @param n_parameters
 * @return
 */
PUBLIC int BoltConnection_cypher(struct BoltConnection * connection, const char * cypher, int32_t n_parameters);

/**
 * Set the next Cypher statement template to be run on this connection
 * from an explicitly-sized string.
 *
 * @param connection
 * @param cypher
 * @param cypher_size
 * @param n_parameters
 * @return
 */
PUBLIC int BoltConnection_cypher_x(struct BoltConnection * connection, const char * cypher, size_t cypher_size, int32_t n_parameters);

PUBLIC struct BoltValue * BoltConnection_cypher_parameter(struct BoltConnection * connection, int32_t index, const char * key);

PUBLIC struct BoltValue * BoltConnection_cypher_parameter_x(struct BoltConnection * connection, int32_t index, const char * key, size_t key_size);

PUBLIC int BoltConnection_load_bookmark(struct BoltConnection * connection, const char * bookmark);

PUBLIC int BoltConnection_load_begin_request(struct BoltConnection * connection);

PUBLIC int BoltConnection_load_commit_request(struct BoltConnection * connection);

PUBLIC int BoltConnection_load_rollback_request(struct BoltConnection * connection);

PUBLIC int BoltConnection_load_run_request(struct BoltConnection * connection);

PUBLIC int BoltConnection_load_discard_request(struct BoltConnection * connection, int32_t n);

PUBLIC int BoltConnection_load_pull_request(struct BoltConnection * connection, int32_t n);

/**
 * Obtain a handle to the last request sent to the server. This handle
 * can be used to fetch response data for a particular request.
 *
 * @param connection
 * @return
 */
PUBLIC bolt_request_t BoltConnection_last_request(struct BoltConnection * connection);

PUBLIC int32_t BoltConnection_n_fields(struct BoltConnection * connection);

PUBLIC const char * BoltConnection_field_name(struct BoltConnection * connection, int32_t index);

PUBLIC int32_t BoltConnection_field_name_size(struct BoltConnection * connection, int32_t index);

PUBLIC int BoltConnection_dump_field_names(struct BoltConnection * connection, struct BoltBuffer * buffer);

PUBLIC int BoltConnection_dump_data(struct BoltConnection * connection, struct BoltBuffer * buffer);


#endif // SEABOLT_CONNECT
