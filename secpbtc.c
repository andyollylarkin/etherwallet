#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "secp256k1.h"
#include "secp256k1_extrakeys.h"
#include "secp256k1_recovery.h"
#include <openssl/rand.h>
#include <unistd.h>
#include <openssl/rand.h>
#include <pthread.h>

#ifdef __linux__
#include <sys/random.h>
#include <fcntl.h>
#include <errno.h>

static void secure_random(unsigned char *buf, size_t len)
{
	ssize_t ret;
	while (len > 0)
	{
		ret = getrandom(buf, len, 0);
		if (ret < 0)
		{
			if (errno == EINTR)
			{
				continue; // Retry if interrupted by a signal
			}
			perror("getrandom failed");
			exit(1);
		}
		buf += ret;
		len -= ret;
	}
}
#else

static void secure_random(unsigned char *buf, size_t len)
{
	if (RAND_bytes(buf, len) != 1)
	{
		fprintf(stderr, "RAND_bytes failed\n");
		exit(1);
	}
}
#endif

static pthread_once_t context_once = PTHREAD_ONCE_INIT;
static secp256k1_context *ctx;

void initialize_context_once()
{
	ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
}

int generate_keypair(unsigned char priv_key[32], unsigned char pub_key[65])
{
	pthread_once(&context_once, initialize_context_once);

	do
	{
		secure_random(priv_key, 32);
	} while (!secp256k1_ec_seckey_verify(ctx, priv_key));

	secp256k1_pubkey pubkey;
	size_t pubkey_len = 65;
	if (!secp256k1_ec_pubkey_create(ctx, &pubkey, priv_key))
	{
		fprintf(stderr, "Failed to create public key\n");
		return -1;
	}

	int result = secp256k1_ec_pubkey_serialize(ctx, pub_key, &pubkey_len, &pubkey, SECP256K1_EC_UNCOMPRESSED);

	return result;
}