#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <libkeccak.h>
#include "secpbtc.h"

int generate_single_eth_address(unsigned char *priv_key, unsigned char *address)
{
	if (!priv_key || !address)
	{
		return -1;
	}

	unsigned char pub_key[65];

	if (generate_keypair(priv_key, (unsigned char *)&pub_key) == -1)
	{
		printf("Fail to generate pubkey\n");
		return -1;
	}

	unsigned char hash[32];

	struct libkeccak_state state;
	struct libkeccak_spec spec;

	// Set parameters for Keccak-256 (same as in Ethereum)
	spec.bitrate = 1088;
	spec.capacity = 512;
	spec.output = 256;

	if (libkeccak_state_initialise(&state, &spec) != 0)
	{
		return -1;
	}

	// Hash only 64 bytes of the public key (skip the first byte 0x04)
	if (libkeccak_update(&state, pub_key + 1, 64) != 0)
	{
		libkeccak_state_fast_destroy(&state);
		return -1;
	}

	// Get the hash
	if (libkeccak_digest(&state, NULL, 0, 0, NULL, hash) != 0)
	{
		libkeccak_state_fast_destroy(&state);
		return -1;
	}

	libkeccak_state_fast_destroy(&state);

	// Take the last 20 bytes of the hash (Ethereum address)
	memcpy(address, hash + 12, 20);

	return 0;
}

int generate_eth_wallets(
	unsigned char *priv_key,
	unsigned char *address)
{
	return generate_single_eth_address(priv_key, address);
}