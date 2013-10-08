#include<nynn_mm_common.h>
#include<nynn_mm_local.h>

using namespace std;
using namespace nynn::mm::common;
using namespace nynn::mm::local;
int main()
{
	subchunk_t<1<<9,1<<11,1<<11>::init("subchunck0");
}
