#ifndef _MD5_H
#define _MD5_H

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD5 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[4];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}
md5_context;

/**
 * \brief          MD5 context setup
 *
 * \param ctx      MD5 context to be initialized
 */
void md5_starts( md5_context *ctx );

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md5_update( md5_context *ctx, const unsigned char *input, int ilen );

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 */
void md5_finish( md5_context *ctx, unsigned char output[16] );

/**
 * \brief          Output = MD5( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 */
void md5_csum( const unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Output = MD5( file contents )
 *
 * \param path     input file name
 * \param output   MD5 checksum result
 * \return         0 if successful, or 1 if fopen failed
 */
int md5_file( const char *path, unsigned char output[16] );

/**
 * \brief          Output = HMAC-MD5( input buffer, hmac key )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD5 result
 */
void md5_hmac( unsigned char *key, int keylen,
               const unsigned char *input, int ilen,
               unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md5_self_test( void );

/**
 * \brief          Get the hex string of MD5 of the input buffer
 *
 * \param input    input string
 * \param ilen     length of the input data
 * \param output   Hex string representation of MD5 checksum
 */
void md5_hex(const void *input, size_t ilen, char output[33]);

/**
 * \brief          Get the hex string of MD5 of null terminated input
 *
 * \param input    input string
 * \param output   Hex string representation of MD5 checksum
 */
void md5_string(const char *input, char output[33]);


#ifdef __cplusplus
}
#endif

#endif /* md5.h */
