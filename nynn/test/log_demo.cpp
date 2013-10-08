#include<public.h>
int main()
{
	info("this is a log test");
	warn("report a warning");
	error("a fatal error");
	exit_on_error(4,"a fatal error");
}
