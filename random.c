#include <stdint.h>
#include <stdio.h>
#include <string.h>

// LCG для генерации 256-битного числа (32 байта)
void lcg_256bit(unsigned char *output, uint64_t *state)
{
	// Параметры LCG (можно изменить)
	const uint64_t a = 6364136223846793005ULL; // множитель
	const uint64_t c = 1;					   // инкремент

	// Генерируем 4 раза по 64 бита (всего 256 бит)
	for (int i = 0; i < 4; i++)
	{
		// Обновляем состояние: Xₙ₊₁ = (a * Xₙ + c) mod 2⁶⁴
		*state = a * (*state) + c;

		// Записываем 64 бита (8 байт) в output
		memcpy(output + i * 8, state, 8);
	}
}

void gen_random(unsigned char *buf, uint64_t *state)
{
	lcg_256bit(buf, state);
}