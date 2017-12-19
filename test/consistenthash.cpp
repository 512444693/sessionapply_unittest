/*
 * filename: consistenthash.cpp
 * author: wangshouyan
 * date: 2017 May 9th
 */
#include "consistenthash.h"
#include "framecommon/toolkit.h"
#include "framecommon/framecommon.h"
#include "openssl/md5.h"
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;
using namespace MYFRAMECOMMON;

namespace MYFRAMECOMMON{

unsigned int shortELFhashWrapper(const string& strKey)
{
	return CToolKit::shortELFhash(strKey.c_str(), strKey.length());
}

unsigned int md5Hash(const string& strKey)
{
	unsigned int nHash = 0;
	unsigned char digest[16] = {0};

	MD5_CTX md5;
	MD5_Init(&md5);
	MD5_Update(&md5, (const unsigned char *)strKey.c_str(), strKey.length());
	MD5_Final(digest, &md5);

	for (int i = 0; i < 4 ; i ++)
	{
		nHash += ((unsigned int)(digest[i * 4 + 3] & 0xFF) << 24)
			| ((unsigned int)(digest[i * 4 + 2] & 0xFF) << 16)
			| ((unsigned int)(digest[i * 4 + 1] & 0xFF) <<  8)
			| ((unsigned int)(digest[i * 4 + 0] & 0xFF));
	}
	return nHash;
}

CConsistentHash::CConsistentHash()
{
	//two kinds of hash function to select
//	m_pHashFunc = shortELFhashWrapper;//this function is very bad!
	m_pHashFunc = md5Hash;
}

int CConsistentHash::UpdateNode(const list<int>& lstNode)
{
	list<int> lstToAdd;
	list<int> lstToDel;
	m_rwLockNode.GetWriteLock();
	list<int>::const_iterator it1 = lstNode.begin();
	for (; it1 != lstNode.end(); ++it1)
	{
		if (0 == m_mapNode.count(*it1))
		{
			lstToAdd.push_back(*it1);
		}
	}

	map<int, int>::iterator it2 = m_mapNode.begin();
	for (; it2 != m_mapNode.end(); ++it2)
	{
		bool bIfFound = false;
		it1 = lstNode.begin();
		for (; it1 != lstNode.end(); ++it1)
		{
			if (it2->first == *it1)
			{
				bIfFound = true;
				break;
			}
		}
		if (!bIfFound)
		{
			lstToDel.push_back(it2->first);
		}
	}

	list<int>::iterator it3 = lstToAdd.begin();
	for (; it3 != lstToAdd.end(); ++it3)
	{
		AddNode(*it3, false);
	}

	it3 = lstToDel.begin();
	for (; it3 != lstToDel.end(); ++it3)
	{
		DelNode(*it3, false);
	}

	m_rwLockNode.ReleaseLock();
	return 0;
}

int CConsistentHash::GetNode(const string& strGcid)
{
	int nHostId = -1;
	unsigned int nHashValue = m_pHashFunc(strGcid);
	m_rwLockNode.GetReadLock();
	map<unsigned int, int>::iterator it = m_mapVirtualNode.lower_bound(nHashValue);
	if (it == m_mapVirtualNode.end())
	{
		it = m_mapVirtualNode.begin();
		if (it != m_mapVirtualNode.end())
		{
			nHostId = it->second;
		}
	}
	else
	{
		nHostId = it->second;
	}
	m_rwLockNode.ReleaseLock();
	return nHostId;
}

int CConsistentHash::AddNode(int nHostId, bool bIfNeedLock /*= true*/, int nVirtualNodeNum /*= VNODE_REPLICA_NUM*/)
{
	if (bIfNeedLock)
	{
		m_rwLockNode.GetWriteLock();
	}

	m_mapNode[nHostId] = nVirtualNodeNum;

	for (int nIndex = 0; nIndex < nVirtualNodeNum; ++nIndex)
	{
		unsigned int nHashValue = m_pHashFunc(GenHashKey(nHostId, nIndex));
		m_mapVirtualNode[nHashValue] = nHostId;
	}

	if (bIfNeedLock)
	{
		m_rwLockNode.ReleaseLock();
	}
	return 0;
}

int CConsistentHash::DelNode(int nHostId, bool bIfNeedLock /*= true*/)
{
	int nRet = 0;
	if (bIfNeedLock)
	{
		m_rwLockNode.GetWriteLock();
	}

	map<int, int>::iterator it = m_mapNode.find(nHostId);
	if (it == m_mapNode.end())
	{
		nRet = -1;
	}
	else
	{
		for (int nIndex = 0; nIndex < it->second; ++nIndex)
		{
			unsigned int nHashValue = m_pHashFunc(GenHashKey(nHostId, nIndex));
			m_mapVirtualNode.erase(nHashValue);
		}
		m_mapNode.erase(it);
	}

	if (bIfNeedLock)
	{
		m_rwLockNode.ReleaseLock();
	}
	return nRet;
}

string CConsistentHash::GenHashKey(int nHostId, int nIndex)
{
	stringstream ss;
	ss << "BFP2p2" << nHostId % 100 << "bf!@#P2P#" << nIndex;
	return ss.str();
}

}

int main(int argc, char* argv[]){
	if (argc != 3)
	{
		printf("Usage: %s node_list_file gcid\n", argv[0]);
		return -1;
	}

	string fileName = argv[1];
	list<int> lstNode;
	string tmp;
	ifstream infile;
	infile.open(fileName.c_str());
	while(getline(infile, tmp)){
		//cout << tmp << endl;
		lstNode.push_back(atoi(tmp.c_str()));
	}
	infile.close();

	string gcid = argv[2];
	if((int)gcid.length() != 40){
		printf("gcid length is not valid\n");
		return -1;
	}
	gcid = CToolKit::HexToBi(gcid.c_str(), gcid.length());


	CConsistentHash cHash;
	cHash.UpdateNode(lstNode);
	printf("%d\n", cHash.GetNode(gcid));

	return 0;
}
