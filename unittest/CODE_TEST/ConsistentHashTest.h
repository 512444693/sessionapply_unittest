#ifndef __CONSISTENTHASHTEST_H__
#define __CONSISTENTHASHTEST_H__
#include "framecommon/framecommon.h"
#include "consistenthash.h"
using namespace std;
using namespace MYFRAMECOMMON;
using namespace testing;

struct THostInfo {
	int m_nHostId;
	int m_nNum;
	THostInfo(){}
	THostInfo(int hostId) {
		m_nHostId = hostId;
		m_nNum = 0;
	}
};

class CHostInfoTable {
	public:
		CHostInfoTable() {
			m_list = new THostInfo[10];
			num = 0;
		}

		~CHostInfoTable() {
			if(m_list != NULL) {
				delete []m_list;
			}
		}

		void Add(int n, int hostId) {
			for(int i = 0; i < num; i++) {
				if(m_list[i].m_nHostId == hostId) {
					m_list[i].m_nNum += n;
					return;
				}
			} 	
			THostInfo tmp(hostId);
			tmp.m_nNum += n;
			m_list[num] =  tmp;
			num ++;
		}

		int Get(int hostId) {
			for(int i = 0; i < num; i++) {
				if(m_list[i].m_nHostId == hostId) {
					return m_list[i].m_nNum;
				}
			}
			return 0;
		}

		void clear() {
			num = 0;
		}

	private:
		THostInfo* m_list;
		int num;
};

struct TGcidInfo {
	string m_strGcid;
	int m_nHostId;

	TGcidInfo(string gcid, int hostId) {
		m_strGcid = gcid;
		m_nHostId = hostId;
	}

	unsigned int GetIndex() const {
		return CToolKit::shortELFhash(m_strGcid.c_str(), (unsigned)m_strGcid.length());
	}

	bool operator==(const TGcidInfo& other) const {
		return (this->m_strGcid == other.m_strGcid);
	}
};

class CConsistentHashTable: public CMyHashTable< TGcidInfo, list<TGcidInfo> > {
	public:
		CConsistentHashTable(int nHashLen) : CMyHashTable<TGcidInfo, list<TGcidInfo> >(nHashLen, 0) {}
		~CConsistentHashTable(){}

		int UpdateItem(TGcidInfo tGcidInfo){
			int nRet = 0;
			int nTmpIndex = tGcidInfo.GetIndex();
			int nRealIndex = nTmpIndex%m_nHashBucketLen;
			HASHLOCK(nRealIndex)
			HashIterator listItr = m_list[nRealIndex].begin();
			for (; listItr != m_list[nRealIndex].end(); listItr++){
				if (tGcidInfo == *listItr) {
					break;
				}
			}
			if (listItr == m_list[nRealIndex].end()) {
				m_list[nRealIndex].push_back(tGcidInfo);
				m_nNodeNum++;
			} else {
				*listItr = tGcidInfo;
				nRet = 1;
			}
			HASHUNLOCK(nRealIndex)
			return nRet;
		}

		void GetAllItems(vector<TGcidInfo> &vecFileinfos){
			for (int nRealIndex = 0; nRealIndex < m_nHashBucketLen; nRealIndex++) {
				HASHLOCK(nRealIndex)
				HashIterator listItr = m_list[nRealIndex].begin();
				for (; listItr != m_list[nRealIndex].end();) {
					vecFileinfos.push_back(*listItr);
					listItr ++;
				}
				HASHUNLOCK(nRealIndex)
			}
		}
};

class MyCConsistentHash : public CConsistentHash{

 public:
	int GetNodeNum() {
		return m_mapNode.size();
	}

	int GetVirtualNodeNum() {
		return m_mapVirtualNode.size();
	}

	map<int, int> GetNodeMap() {
		return m_mapNode;
	}
};

class ConsistentHashTest : public TestWithParam<string> {

 public:
	static void SetUpTestCase(){}

	void SetUp(){
		int nodes[] = {1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008};
		for (unsigned int i = 0; i < sizeof(nodes)/sizeof(int); i++) {
			lstNode.push_back(nodes[i]);	
		}
	}

	/*
	 *void TearDown(){
	 *        cout << PrintToString(myCHash.GetNodeMap()) << endl;
	 *}
	 */

	static CConsistentHash cHash;
	static CConsistentHashTable table;
	static CHostInfoTable distribution;
	static CHostInfoTable difference;
	static MyCConsistentHash myCHash;
	static int allGcidNum;

	list<int> lstNode;
};

#endif
