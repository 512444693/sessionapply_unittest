/*
 * filename: consistenthash.h
 * description: consistent hashing, mainly used for mapping GCID to particular VP in taishan system
 * author: wangshouyan
 * date: 2017 May 9th
 */
#ifndef __CONSISTENT_HASH_H__
#define __CONSISTENT_HASH_H__

#include <map>
#include <list>
#include <string>
#include "framecommon/threadrwlock.h"

using namespace std;

namespace MYFRAMECOMMON{

const int VNODE_REPLICA_NUM = 2048;

/* 
 * function to calculate hash value 
 * @strKey: the input string
 */
typedef unsigned int (*conhash_hashfunc)(const string& strKey);

class CConsistentHash
{
public:
	CConsistentHash();
	virtual ~CConsistentHash(){}
	//@lstNode: contains all current hostids
	int UpdateNode(const list<int>& lstNode);
	//@strGcid: 20bytes GCID in binary
	//return hostid
	int GetNode(const string& strGcid);
protected:
	//Call this two function to manage node yourself!
	int AddNode(int nHostId, bool bIfNeedLock = true, int nVirtualNodeNum = VNODE_REPLICA_NUM);
	int DelNode(int nHostId, bool bIfNeedLock = true);
private:
	string GenHashKey(int nHostId, int nIndex);
protected:
	map<int, int> m_mapNode;//hostid->VNodeNum
	map<unsigned int, int> m_mapVirtualNode;//hashvalue->hostid
	CThreadRwLock m_rwLockNode;
	conhash_hashfunc m_pHashFunc;
};

}

#endif

