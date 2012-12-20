#include <stdlib.h>
#include <stdio.h>

static int usage(int argc, char** argv)
{
	(void) argc;

	fprintf(stderr,
		"Usage: %s slen mode [PARAMS]\n"
		"                        rtgen [file]\n"
		"                        rtnew [dst]\n"
		"                        rtres [file]\n"
		"                        merge src1 [src2 [dst]]\n"
		"                        tests [n_tests   [src]]\n"
		"                        crack hash       [src]\n"
		,
		argv[0]
	);
}

int main(int argc, char** argv)
{
	usage(argc, argv);
	return 0;
}
