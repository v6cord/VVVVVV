#ifndef BINARYBLOB_H
#define BINARYBLOB_H

/* Laaaazyyyyyyy -flibit */
// #define VVV_COMPILEMUSIC

struct resourceheader
{
	char name[48];
	int start = 0;
	int size = 0;
	bool valid = false;
};

class binaryBlob
{
public:
	binaryBlob();

#ifdef VVV_COMPILEMUSIC
	void AddFileToBinaryBlob(const char* _path);

	void writeBinaryBlob(const char* _name);
#endif

	bool unPackBinary(const char* _name);

	int getIndex(const char* _name);

	int getSize(int _index);

	char* getAddress(int _index);

private:
	int numberofHeaders = 0;
	resourceheader m_headers[128];
	char* m_memblocks[128];
};


#endif /* BINARYBLOB_H */
