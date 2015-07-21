#include "scramble.h"
#include "random.h"
#include "third_party/sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unit.h"

void
test_scramble()
{
	int salt[SCRAMBLE_SIZE/sizeof(int)];
	for (int i = 0; i < sizeof(salt)/sizeof(int); i++)
		salt[i] = rand();

	char *password = "lechododilikraskaloh";
	unsigned char hash2[SCRAMBLE_SIZE];

	SHA1_CTX ctx;

	SHA1Init(&ctx);
	SHA1Update(&ctx, (unsigned char *) password, strlen(password));
	SHA1Final(hash2, &ctx);

	SHA1Init(&ctx);
	SHA1Update(&ctx, hash2, SCRAMBLE_SIZE);
	SHA1Final(hash2, &ctx);

	char scramble[SCRAMBLE_SIZE];

	scramble_prepare(scramble, salt, password, strlen(password));

	printf("%d\n", scramble_check(scramble, salt, hash2));

	password = "wrongpass";
	scramble_prepare(scramble, salt, password, strlen(password));

	printf("%d\n", scramble_check(scramble, salt, hash2) != 0);

	scramble_prepare(scramble, salt, password, 0);

	printf("%d\n", scramble_check(scramble, salt, hash2) != 0);
}

void
test_password_prepare()
{
	char buf[SCRAMBLE_BASE64_SIZE * 2];
	int password[5];
	for (int i = 0; i < sizeof(password)/sizeof(int); i++)
		password[i] = rand();
	password_prepare((char *) password, sizeof(password),
			 buf, sizeof(buf));
	fail_unless(strlen(buf) == SCRAMBLE_BASE64_SIZE);
}

int main()
{
	random_init();

	test_scramble();
	test_password_prepare();

	return 0;
}
