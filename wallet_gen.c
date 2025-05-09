#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/rand.h>
#include <secp256k1.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <unistd.h>
#include <libkeccak.h>

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


int generate_single_eth_address(unsigned char *priv_key, unsigned char *address)
{
	if (!priv_key || !address)
	{
		return -1;
	}

	secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
	if (!ctx)
	{
		return -1;
	}

	do
	{
		secure_random(priv_key, 32);
	} while (!secp256k1_ec_seckey_verify(ctx, priv_key));

	secp256k1_pubkey pubkey;
	if (!secp256k1_ec_pubkey_create(ctx, &pubkey, priv_key))
	{
		secp256k1_context_destroy(ctx);
		return -1;
	}

	unsigned char pub_key[65];
	size_t pubkey_len = 65;
	secp256k1_ec_pubkey_serialize(ctx, pub_key, &pubkey_len, &pubkey, SECP256K1_EC_UNCOMPRESSED);

	// Use Keccak-256
	unsigned char hash[32];

	// Ensure that the libkeccak library is linked and properly initialized
	struct libkeccak_state state;
	struct libkeccak_spec spec;

	// Set parameters for Keccak-256 (same as in Ethereum)
	spec.bitrate = 1088;
	spec.capacity = 512;
	spec.output = 256;

	if (libkeccak_state_initialise(&state, &spec) != 0)
	{
		secp256k1_context_destroy(ctx);
		return -1;
	}

	// Hash only 64 bytes of the public key (skip the first byte 0x04)
	if (libkeccak_update(&state, pub_key + 1, 64) != 0)
	{
		libkeccak_state_fast_destroy(&state);
		secp256k1_context_destroy(ctx);
		return -1;
	}

	// Get the hash
	if (libkeccak_digest(&state, NULL, 0, 0, NULL, hash) != 0)
	{
		libkeccak_state_fast_destroy(&state);
		secp256k1_context_destroy(ctx);
		return -1;
	}

	libkeccak_state_fast_destroy(&state);

	// Take the last 20 bytes of the hash (Ethereum address)
	memcpy(address, hash + 12, 20);

	secp256k1_context_destroy(ctx);
	return 0;
}

int generate_eth_wallets(
	unsigned char *priv_key,
	unsigned char *address)
{
	return generate_single_eth_address(priv_key, address);
}