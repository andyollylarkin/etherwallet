#ifndef WALLET_GEN_H
#define WALLET_GEN_H

#include <stddef.h>

#define ETH_PRIV_KEY_SIZE 32
#define ETH_ADDRESS_SIZE 20

int generate_eth_wallets(
	unsigned char *priv_keys,
	unsigned char *address);
int generate_single_eth_address(unsigned char *priv_key, unsigned char *address);

#endif // WALLET_GEN_H