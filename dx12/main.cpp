#include <iostream>
#include "dx12.h"

using std::cout;
using std::wcout;
using std::endl;
using namespace Dx12;

int main()
{
	cout << "dx12 learning ..." << endl;
	wcout.imbue(std::locale("", LC_CTYPE)); // set to system code

	DeviceDx12::PrintAdapterOutputDisplayInfo(wcout);

	return 0;
}
