#include "precomp.h"
#include <fstream>
#include <sstream>
int main(int argc, char* argv[])
{

	if (argc < 2)
		return -1;


	std::ifstream  ff(argv[1], std::ios_base::binary);
	if (!ff.is_open())
		return -1;
	std::stringstream  sImg;
	sImg << ff.rdbuf();

	RPIMG::RPMIMG_STREAM  stream = { 0 };
	char* szBuf = new char[sImg.str().size()];
	int nSize = sImg.str().size();
	ff.seekg(std::ios::beg);
	ff.read(szBuf, nSize);

	//for (int i = 0; i < 1000000; i++)
	{
		int Width, Height;
		CRpImgRead::RpGetImgSize((const BYTE*)szBuf, sImg.str().size(), Width, Height);

			printf("width: %d, height: %d\n",  Width, Height);

	}
	delete[]szBuf;
	return 0;
}