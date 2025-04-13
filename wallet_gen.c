#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/rand.h>
#include <secp256k1.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <unistd.h>
#include <libkeccak.h>

typedef struct
{
	size_t start;
	size_t end;
	unsigned char *priv_keys;
	unsigned char *addresses;
} ThreadData;

static void secure_random(unsigned char *buf, size_t len)
{
	if (RAND_bytes(buf, len) != 1)
	{
		fprintf(stderr, "RAND_bytes failed\n");
		exit(1);
	}
}

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

static void *generate_batch(void *arg)
{
	ThreadData *data = (ThreadData *)arg;
	secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
	unsigned char priv_key[32], pub_key[65], address[20];
	secp256k1_pubkey pubkey;

	for (size_t i = data->start; i < data->end; ++i)
	{
		do
		{
			secure_random(priv_key, 32);
		} while (!secp256k1_ec_seckey_verify(ctx, priv_key));

		if (!secp256k1_ec_pubkey_create(ctx, &pubkey, priv_key))
		{
			fprintf(stderr, "Failed to create public key\n");
			exit(1);
		}

		size_t pubkey_len = 65;
		secp256k1_ec_pubkey_serialize(ctx, pub_key, &pubkey_len, &pubkey, SECP256K1_EC_UNCOMPRESSED);

		EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(mdctx, EVP_sha3_256(), NULL);
		EVP_DigestUpdate(mdctx, pub_key + 1, 64);
		unsigned char hash[32];
		EVP_DigestFinal_ex(mdctx, hash, NULL);
		EVP_MD_CTX_free(mdctx);

		memcpy(address, hash + 12, 20);
		memcpy(&data->priv_keys[i * 32], priv_key, 32);
		memcpy(&data->addresses[i * 20], address, 20);
	}

	secp256k1_context_destroy(ctx);
	return NULL;
}

int generate_eth_wallets(size_t num_keys,
						 unsigned char *priv_keys,
						 unsigned char *addresses,
						 unsigned int num_threads)
{
	if (!priv_keys || !addresses)
	{
		return -1;
	}

	if (num_threads == 0)
	{
		num_threads = sysconf(_SC_NPROCESSORS_ONLN);
		if (num_threads < 1)
			num_threads = 1;
	}

	pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
	if (!threads)
		return -1;

	ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));
	if (!thread_data)
	{
		free(threads);
		return -1;
	}

	size_t batch_size = num_keys / num_threads;
	for (unsigned int i = 0; i < num_threads; ++i)
	{
		thread_data[i].start = i * batch_size;
		thread_data[i].end = (i == num_threads - 1) ? num_keys : thread_data[i].start + batch_size;
		thread_data[i].priv_keys = priv_keys;
		thread_data[i].addresses = addresses;

		if (pthread_create(&threads[i], NULL, generate_batch, &thread_data[i]))
		{
			for (unsigned int j = 0; j < i; ++j)
			{
				pthread_join(threads[j], NULL);
			}
			free(threads);
			free(thread_data);
			return -1;
		}
	}

	for (unsigned int i = 0; i < num_threads; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	free(threads);
	free(thread_data);
	return 0;
}