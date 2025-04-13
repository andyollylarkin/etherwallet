#include <stdio.h>
#include "wallet_gen.h"
#include <stdlib.h>

int main()
{
	unsigned char priv_key[ETH_PRIV_KEY_SIZE];
	unsigned char address[ETH_ADDRESS_SIZE];
	int sc = generate_single_eth_address(priv_key, address);
	if (sc != 0)
	{
		fprintf(stderr, "Failed to generate single Ethereum address\n");
		return sc;
	}

	printf("Private Key: ");
	printf("0x");
	for (int i = 0; i < ETH_PRIV_KEY_SIZE; i++)
	{
		printf("%02x", priv_key[i]);
	}
	printf("\nAddress: ");
	printf("0x");
	for (int i = 0; i < ETH_ADDRESS_SIZE; i++)
	{
		printf("%02x", address[i]);
	}
	printf("\n");
}
