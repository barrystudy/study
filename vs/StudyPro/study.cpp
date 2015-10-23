#include <iostream>
#include "OfflineSchSrvMgr.h"
#include <openssl/aes.h>
#include <openssl/rand.h> 


class CSecurityKey
{
	std::vector<char> m_Key;
public:
	explicit CSecurityKey(const std::vector<char> &keyEqual);
	CSecurityKey(char* keyEqual, size_t size);
	std::vector<char>& getKey();
};

CSecurityKey::CSecurityKey(char* keyEqual, size_t size): m_Key(size)
{
	memcpy(&m_Key.front(), keyEqual, m_Key.size () );
}
CSecurityKey::CSecurityKey(const std::vector<char> &keyEqual):m_Key(keyEqual.size())
{            
	memcpy(&m_Key.front(), &keyEqual.front(), m_Key.size () );
}

void InitializeAesKey ( const unsigned char* aKey, unsigned int KeyLength, CSecurityKey* key );

class CAes
{
private:
	struct ctr_state {
		unsigned char ivec[16];  /* ivec[0..7] is the IV, ivec[8..15] is the big-endian counter */
		unsigned int num;
		unsigned char ecount[16];
	};
	struct ctr_state state;
	unsigned char iv[AES_BLOCK_SIZE];// = {0/*'U','T','Y','[','^','F','B',']','U','T','Y','[','^','F','B',']'*/};
	void init_ctr();
public:
	CAes();
	void EncryptECB( const char* pvBufferIn, char* pvBufferOut, unsigned long dwLength, CSecurityKey* key );
	void DecryptECB( const char* pvBufferIn, char* pvBufferOut, unsigned long dwLength, CSecurityKey* key );
};
//----------------------------------------------------------------------------------------------------------
std::vector<char>& CSecurityKey::getKey() 
{
	return m_Key;
}

CAes::CAes()
{
    init_ctr();
}
//-------------------------------------------------------------------------------------------------------------
void CAes::init_ctr()
{
    /* aes_ctr128_encrypt requires 'num' and 'ecount' set to zero on the
     * first call. */
    state.num = 0;
    memset(state.ecount, 0, AES_BLOCK_SIZE);
    /* Copy IV into 'ivec' */
    //RtlCopyMemory(state->ivec, iv, AES_BLOCK_SIZE);
    memset(state.ivec, 0, AES_BLOCK_SIZE);
}
//------------------------------------------------------------------------------------------------------------
void InitializeAesKey ( const unsigned char* aKey, unsigned int KeyLength, CSecurityKey* key )
{
    AES_KEY bfKey;
    ::memset( &bfKey, 0, sizeof (bfKey) );

    if ( AES_set_encrypt_key( aKey, KeyLength, &bfKey ) < 0 )
        throw std::exception ( "Error on AES_set_encrypt_key" );

    *key = CSecurityKey ( (char*)&bfKey, sizeof(AES_KEY));
}
//------------------------------------------------------------------------------------------------------------
void CAes::EncryptECB ( const char* pvBufferIn, char* pvBufferOut, unsigned long dwLength, CSecurityKey* key )
{
    AES_ctr128_encrypt(
		(const unsigned char*)pvBufferIn,
        (unsigned char*)pvBufferOut,
        dwLength,
        (AES_KEY*)&key->getKey().front(),
        state.ivec,
        state.ecount,
        &state.num);
}
//------------------------------------------------------------------------------------------------------------
void CAes::DecryptECB ( const char* pvBufferIn, char* pvBufferOut, unsigned long dwLength, CSecurityKey* key)
{
    AES_ctr128_encrypt((const unsigned char*)pvBufferIn,
        (unsigned char*)pvBufferOut,
        dwLength,
        (AES_KEY*)&key->getKey().front(),
        state.ivec,
        state.ecount,
        &state.num);
}

#define CRYPT_TEMP_EXT L".!mp"
#define FILE_BUFFER_SIZE 1048576

int main(int argc, char *argv[])
{
	std::string pass = "1234567812345678";
	char srcbuf[1024] = "hello, nihao,shijie";
	int nn = strlen(srcbuf);
	char resbuf[1024] = {0};
	{
		CAes aesCipher;
		CSecurityKey aesExmapleKey(std::vector< char > (1));
		int aaa = pass.length();
		InitializeAesKey ( (const unsigned char*)pass.c_str(), pass.length() * 8, &aesExmapleKey );
		std::vector<char> buffer(FILE_BUFFER_SIZE);
		aesCipher.EncryptECB( srcbuf, resbuf, nn, &aesExmapleKey );
	}
	{
		CAes aesCipher;
		CSecurityKey aesExmapleKey(std::vector< char > (1));
		InitializeAesKey ( (const unsigned char*)pass.c_str(), pass.length() * 8, &aesExmapleKey );
		char buf[1024] = {0};
		aesCipher.DecryptECB(resbuf, buf, nn, &aesExmapleKey);
		std::string aa=buf;
	}
	
	system("pause");
	return 0;
}