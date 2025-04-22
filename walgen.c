#include <stdio.h>
#include "wallet_gen.h"

int main(int argc, char *argv[])
{
	unsigned char priv[32];
	unsigned char pub[20];

	if (generate_eth_wallets(&priv, &pub))
	{
		printf("Fail to generate pubkey!\n");
	};

	printf("Private key : ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", priv[i]);
	}
	printf("\n");

	printf("Address: 0x");

	for (int i = 0; i < 20; i++)
	{
		printf("%02x", pub[i]);
	}
	printf("\n");
}