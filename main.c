#include <stdio.h>
#include <stdlib.h>

typedef signed char       int8_t;
typedef signed short      int16_t;
typedef signed int        int32_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;

int Nb = 4;
int Nr = 10;
int Nk = 4;

/*
 * S-box transformation table
 */
static uint8_t s_box[256] = {
	// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, // 0
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, // 1
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, // 2
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, // 3
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, // 4
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, // 5
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, // 6
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, // 7
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, // 8
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, // 9
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, // a
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, // b
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, // c
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, // d
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, // e
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};// f

void RotWord(uint8_t *a)
{
	uint8_t temp;
	uint8_t i;

	temp = a[0];

	for(i = 0; i < 3; i++)
	{
		a[i] = a[i + 1];
	}

	a[3] = temp;
}

void SubWord(uint8_t *a)
{
	uint8_t i;

	for (i = 0; i < 4; i++)
	{
		a[i] = s_box[16 * ((a[i] & 0xF0) >> 4) + (a[i] & 0x0F)];
	}
}

uint8_t Mult(uint8_t a, uint8_t b) 
{

	uint8_t y = 0;
	uint8_t z = 0;
	uint8_t x = 0;

	for (z = 0; z < 8; z++) {
		if (b & 1) {
			y ^= a;
		}

		x = a & 0x80;
		a <<= 1;
		if (x) a ^= 0x1b;
		b >>= 1;
	}

	return (uint8_t)y;
}



// Used to help generate the Round Contstant
uint8_t use_round[] = {0x02, 0x00, 0x00, 0x00};

void KeyExpansion(uint8_t *key, uint8_t *w)
{
	uint8_t temp[4];
	uint8_t a = 0;
	uint8_t length = Nb * (Nr + 1);
	uint8_t * calc;

	for (a = 0; a < Nk; a++)
	{
		w[4 * a + 0] = key[4 * a + 0];
		w[4 * a + 1] = key[4 * a + 1];
		w[4 * a + 2] = key[4 * a + 2];
		w[4 * a + 3] = key[4 * a + 3];
	}

	for (a = Nk; a < length; a++) 
	{
		temp[0] = w[4 * (a - 1) + 0];
		temp[1] = w[4 * (a - 1) + 1];
		temp[2] = w[4 * (a - 1) + 2];
		temp[3] = w[4 * (a - 1) + 3];

		if (a % Nk == 0) 
		{
			RotWord(temp);
			SubWord(temp);

			uint8_t math =  a / Nk;

			if (math == 1) 
			{
				use_round[0] = 0x01;
			} 
			else if (math > 1) 
			{
				use_round[0] = 0x02;
				math--;
		
				while ((math - 1) > 0) 
				{
					use_round[0] = Mult(use_round[0], 0x02);
					math--;
				}
			}

			calc = use_round;

			temp[0] = calc[0] ^ temp[0];
			temp[1] = calc[1] ^ temp[1];
			temp[2] = calc[2] ^ temp[2];
			temp[3] = calc[3] ^ temp[3];
		} 
		else if (Nk > 6 && a % Nk == 4)
		{
			SubWord(temp);
		}

		w[4 * a + 0] = w[4 * (a - Nk) + 0] ^ temp[0];
		w[4 * a + 1] = w[4 * (a - Nk) + 1] ^ temp[1];
		w[4 * a + 2] = w[4 * (a - Nk) + 2] ^ temp[2];
		w[4 * a + 3] = w[4 * (a - Nk) + 3] ^ temp[3];
	}

}

void AddRoundKey(uint8_t *state, uint8_t *w, uint8_t r)
{
	uint8_t a;
	
	for (a = 0; a < Nb; a++) 
	{
		state[Nb * 0 + a] = state[Nb * 0 + a] ^ w[4 * Nb * r + Nb * a + 0];
		state[Nb * 1 + a] = state[Nb * 1 + a] ^ w[4 * Nb * r + Nb * a + 1];
		state[Nb * 2 + a] = state[Nb * 2 + a] ^ w[4 * Nb * r + Nb * a + 2];
		state[Nb * 3 + a] = state[Nb * 3 + a] ^ w[4 * Nb * r + Nb * a + 3];	
	}
}

void SubBytes(uint8_t *state)
{
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t rows;
	uint8_t columns;

	for (a; a < 4; a++) 
	{
		for (b = 0; b < Nb; b++) 
		{
			rows = (state[Nb * a + b] & 0xF0) >> 4;
			columns = state[Nb * a + b] & 0x0F;
			state[Nb * a + b] = s_box[16 * rows + columns];
		}
	}
}

void ShiftRows(uint8_t *state)
{
	uint8_t temp;
	uint8_t a;
	uint8_t b;
	uint8_t s;

	for (a = 1; a < 4; a++) 
	{
		s = 0;
		while (s < a) 
		{
			temp = state[Nb * a];
			
			for (b = 1; b < Nb; b++) 
			{
				state[Nb * a + b - 1] = state[Nb * a + b];
			}

			state[Nb * a + Nb - 1] = temp;
			s++;
		}
	}
}


void MixColumns(uint8_t *state)
{
	uint8_t check[] = {0x02, 0x01, 0x01, 0x03};
	uint8_t a;
	uint8_t b;
	uint8_t rows[4];
	uint8_t columns[4];

	for (b = 0; b < Nb; b++)
	{
		for (a = 0; a < 4; a++)
		{
			columns[a] = state[Nb * a + b];
		}

		rows[0] = Mult(check[0], columns[0]) ^ Mult(check[3], columns[1]) ^ Mult(check[2], columns[2]) ^ Mult(check[1], columns[3]);
		rows[1] = Mult(check[1], columns[0]) ^ Mult(check[0], columns[1]) ^ Mult(check[3], columns[2]) ^ Mult(check[2], columns[3]);
		rows[2] = Mult(check[2], columns[0]) ^ Mult(check[1], columns[1]) ^ Mult(check[0], columns[2]) ^ Mult(check[3], columns[3]);
		rows[3] = Mult(check[3], columns[0]) ^ Mult(check[2], columns[1]) ^ Mult(check[1], columns[2]) ^ Mult(check[0], columns[3]);

		for (a = 0; a < 4; a++)
		{
			state[Nb * a + b] = rows[a];
		}
	}
}


void AES(uint8_t *in, uint8_t *out, uint8_t *w)
{
	uint8_t state[4 * Nb];
	uint8_t r;
	uint8_t a;
	uint8_t b;

	for (a = 0; a < 4; a++)
	{
		for (b = 0; b < Nb; b++)
		{
			state[Nb*a+b] = in[a+4*b];
		}
	}

	AddRoundKey(state, w, 0);

	for (r = 1; r < Nr; r++)
	{
		SubBytes(state);
		ShiftRows(state);
		MixColumns(state);
		AddRoundKey(state, w, r);
	}

	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, w, Nr);

	for (a = 0; a < 4; a++) 
	{
		for (b = 0; b < Nb; b++) 
		{
			out[a + 4 * b] = state[Nb * a + b];
		}
	}
}

int main()
{
	uint8_t plaintext_temp[16];
	uint8_t cipherkey_temp[16];
	int i;

	//Read in message
	printf("Input plaintext(Enter bits in amounts of two)\n");
	scanf("%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &plaintext_temp[0], &plaintext_temp[1], &plaintext_temp[2], &plaintext_temp[3], &plaintext_temp[4], &plaintext_temp[5], &plaintext_temp[6], &plaintext_temp[7], &plaintext_temp[8], &plaintext_temp[9], &plaintext_temp[10], &plaintext_temp[11], &plaintext_temp[12], &plaintext_temp[13], &plaintext_temp[14], &plaintext_temp[15]);
	printf("\n");

		uint8_t plaintext[] = 
	{
		plaintext_temp[0], plaintext_temp[1], plaintext_temp[2], plaintext_temp[3],
		plaintext_temp[4], plaintext_temp[5], plaintext_temp[6], plaintext_temp[7],
		plaintext_temp[8], plaintext_temp[9], plaintext_temp[10], plaintext_temp[11],
		plaintext_temp[12], plaintext_temp[13], plaintext_temp[14], plaintext_temp[15]
	};


	printf("Input the cipher key(Enter bits in amounts of two) \n");
	scanf("%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &cipherkey_temp[0], &cipherkey_temp[1], &cipherkey_temp[2], &cipherkey_temp[3], &cipherkey_temp[4], &cipherkey_temp[5], &cipherkey_temp[6], &cipherkey_temp[7], &cipherkey_temp[8], &cipherkey_temp[9], &cipherkey_temp[10], &cipherkey_temp[11], &cipherkey_temp[12], &cipherkey_temp[13], &cipherkey_temp[14], &cipherkey_temp[15]);

	uint8_t cipherkey[] = 
	{
		cipherkey_temp[0], cipherkey_temp[1], cipherkey_temp[2], cipherkey_temp[3],
		cipherkey_temp[4], cipherkey_temp[5], cipherkey_temp[6], cipherkey_temp[7],
		cipherkey_temp[8], cipherkey_temp[9], cipherkey_temp[10], cipherkey_temp[11],
		cipherkey_temp[12], cipherkey_temp[13], cipherkey_temp[14], cipherkey_temp[15]
	};

	printf("Your plaintext is: \n");
	printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ", plaintext_temp[0], plaintext_temp[1], plaintext_temp[2], plaintext_temp[3], plaintext_temp[4], plaintext_temp[5], plaintext_temp[6], plaintext_temp[7], plaintext_temp[8], plaintext_temp[9], plaintext_temp[10], plaintext_temp[11], plaintext_temp[12], plaintext_temp[13], plaintext_temp[14], plaintext_temp[15]);
	printf("\n");

	printf("Your cipherkey is: \n");
	printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ", cipherkey_temp[0], cipherkey_temp[1], cipherkey_temp[2], cipherkey_temp[3], cipherkey_temp[4], cipherkey_temp[5], cipherkey_temp[6], cipherkey_temp[7], cipherkey_temp[8], cipherkey_temp[9], cipherkey_temp[10], cipherkey_temp[11], cipherkey_temp[12], cipherkey_temp[13], cipherkey_temp[14], cipherkey_temp[15]);
	printf("\n");

	uint8_t out[16]; // 128

	uint8_t *w; // Key Expanded
	w = malloc(Nb * (Nr + 1) * 4);

	KeyExpansion(cipherkey, w);

	AES(plaintext, out, w);

	printf("Encrypted Message:\n");
	
	printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ", out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9], out[10], out[11], out[12], out[13], out[14], out[15]);

	printf("\n");

	return 0;

}
