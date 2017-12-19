#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <iostream>
#include <fstream>
#include "ConsistentHashTest.h"

CConsistentHash ConsistentHashTest::cHash;
CConsistentHashTable ConsistentHashTest::table(100001);
CHostInfoTable ConsistentHashTest::distribution;
CHostInfoTable ConsistentHashTest::difference;
MyCConsistentHash ConsistentHashTest::myCHash;
int ConsistentHashTest::allGcidNum = 0;

typedef void (*assert_hostid_func)(int originHostid, int nowHostid, int expect_hostid);

void printResult(int allGcidNum, list<int> lstNode, CHostInfoTable& distribution, CHostInfoTable& difference) {
	int size = lstNode.size();
	printf("node, 占比/误差/分布, 资源增减\n");
	int average = allGcidNum / size;

	int distriAll = 0;
	double distriPropAll = 0.0;
	double  errorAll = 0.0;
	for (list<int>::iterator itr = lstNode.begin(); itr != lstNode.end(); itr++) {
		int distri = distribution.Get(*itr);
		distriAll += distri;

		double distriProp = distri * 100.0/allGcidNum;
		distriPropAll += distriProp;

		double error = (distri-average) * 100.0/allGcidNum;
		errorAll += fabs(error);

		int diff = difference.Get(*itr);
		printf("%d, %.3f%%/%.3f%%/%d, %d\n", *itr, distriProp, error, distri, diff);
	}
	ASSERT_EQ(distriAll, allGcidNum);
	//期望总误差小于等于2%
	EXPECT_PRED_FORMAT2(::testing::DoubleLE, errorAll, 2);
	printf("总共, %.3f%%/%.3f%%/%d\n", 1.0, errorAll, distriAll);
}

int initTable(CConsistentHash& cHash, CConsistentHashTable& table, CHostInfoTable& distribution, CHostInfoTable& difference) {
	string fileName = "totalgcid";

	ifstream infile;
	infile.open(fileName.c_str());
	string tmp;
	while(getline(infile, tmp)){
		string gcid = CToolKit::HexToBi(tmp.c_str(), tmp.length());
		int hostid = cHash.GetNode(gcid);
		distribution.Add(1, hostid);
		difference.Add(1, hostid);
		TGcidInfo gcidInfo(gcid, hostid);
		table.AddItem(gcidInfo);
	}
	infile.close();

	return table.GetNodeNum();
}

void updateTable(CConsistentHash& cHash, CConsistentHashTable& table, CHostInfoTable& distribution, CHostInfoTable& difference, assert_hostid_func assert_hostid, int expect_hostid) {
	distribution.clear();
	difference.clear();
	vector<TGcidInfo> vecGcidInfos;
	table.GetAllItems(vecGcidInfos);

	for(vector<TGcidInfo>::iterator itr = vecGcidInfos.begin(); itr != vecGcidInfos.end(); itr ++) {
		string gcid = itr->m_strGcid;
		int originHostid = itr->m_nHostId;
		int nowHostid = cHash.GetNode(gcid);
		distribution.Add(1, nowHostid);
		if (originHostid != nowHostid) {
			difference.Add(-1, originHostid);
			difference.Add(1, nowHostid);
			TGcidInfo gcidInfo(gcid, nowHostid);
			table.UpdateItem(gcidInfo);
			assert_hostid(originHostid, nowHostid, expect_hostid);
		}
	}
}

void assert_nowhostid(int, int nowHostid, int expect_hostid) {
	ASSERT_EQ(nowHostid, expect_hostid);
}

void assert_originhostid(int originHostid, int, int expect_hostid) {
	ASSERT_EQ(originHostid, expect_hostid);
}

TEST_F(ConsistentHashTest, errorTest){
	int nodes[] = {1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010};
	lstNode.clear();
	lstNode.push_back(1001);

	cHash.UpdateNode(lstNode);
	allGcidNum = initTable(cHash, table, distribution, difference);
	cout << "get " << allGcidNum << " gcids from file" << endl;
	printf("===初始化===\n");
	printResult(allGcidNum, lstNode, distribution, difference);


	unsigned int i = 0;
	for(; i < sizeof(nodes)/sizeof(int); i++) {
		lstNode.push_back(nodes[i]);
		cHash.UpdateNode(lstNode);
		updateTable(cHash, table, distribution, difference, assert_nowhostid, nodes[i]);
		printf("===增加%d===\n", nodes[i]);
		printResult(allGcidNum, lstNode, distribution, difference);
	}

	for(; i >= 1; i--) {
		int tmp = lstNode.front();
		lstNode.pop_front();
		cHash.UpdateNode(lstNode);
		updateTable(cHash, table, distribution, difference, assert_originhostid, tmp);
		printf("===减去%d===\n", tmp);
		printResult(allGcidNum, lstNode, distribution, difference);
	}
}

TEST_F(ConsistentHashTest, GetTime) {
	vector<TGcidInfo> vecGcidInfos;
	table.GetAllItems(vecGcidInfos);
	cHash.UpdateNode(lstNode);

	struct timeval start, end;
	gettimeofday(&start, NULL);

	for(vector<TGcidInfo>::iterator itr = vecGcidInfos.begin(); itr != vecGcidInfos.end(); itr ++) {
		string gcid = itr->m_strGcid;
		ASSERT_NE(-1, cHash.GetNode(gcid));
	}
	gettimeofday(&end, NULL);

	int timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
	printf("Count Time : %d us\n", timeuse);
}

TEST_F(ConsistentHashTest, AddNode0) {
	lstNode.clear();
	ASSERT_EQ(0, myCHash.GetNodeNum());
	ASSERT_EQ(0 * VNODE_REPLICA_NUM, myCHash.GetVirtualNodeNum());
}

TEST_F(ConsistentHashTest, AddNode8) {
	myCHash.UpdateNode(lstNode);
	ASSERT_EQ(8, myCHash.GetNodeNum());
	ASSERT_EQ(8 * VNODE_REPLICA_NUM, myCHash.GetVirtualNodeNum());
}

/*
 *TEST_P(ConsistentHashTest, GetNode) {
 *        myCHash.UpdateNode(lstNode);
 *        myCHash.GetNode(GetParam());
 *}
 *INSTANTIATE_TEST_CASE_P(GetNodeTest, ConsistentHashTest, Values("12345678901234567890", "12345678901234567891", "12345678901234567892"));
 *
 */

TEST_F(ConsistentHashTest, UpdateNode0) {
	myCHash.UpdateNode(lstNode);
	ASSERT_EQ(8, myCHash.GetNodeNum());
	ASSERT_EQ(8 * VNODE_REPLICA_NUM, myCHash.GetVirtualNodeNum());
}

TEST_F(ConsistentHashTest, GetNodeAlwaysSame) {
	string gcid = "12345678901234567890";
	int tmp = myCHash.GetNode(gcid);
	ASSERT_EQ(tmp, myCHash.GetNode(gcid));
	ASSERT_EQ(tmp, myCHash.GetNode(gcid));
	ASSERT_EQ(tmp, myCHash.GetNode(gcid));
	gcid = "";
	cout << myCHash.GetNode(gcid) << endl;
}

TEST_F(ConsistentHashTest, Add3AndRemove2) {
	lstNode.pop_front();
	lstNode.pop_front();
	lstNode.push_back(1009);
	lstNode.push_back(1010);
	lstNode.push_back(1011);
	myCHash.UpdateNode(lstNode);
	ASSERT_EQ(9, myCHash.GetNodeNum());
	ASSERT_EQ(9 * VNODE_REPLICA_NUM, myCHash.GetVirtualNodeNum());
}

TEST_F(ConsistentHashTest, RemoveNode8) {
	lstNode.clear();
	myCHash.UpdateNode(lstNode);
	ASSERT_EQ(0, myCHash.GetNodeNum());
	ASSERT_EQ(0 * VNODE_REPLICA_NUM, myCHash.GetVirtualNodeNum());
}

TEST_F(ConsistentHashTest, GetNodeFail) {
	string tmp = "12345678901234567890";
	ASSERT_EQ(-1, myCHash.GetNode(tmp));
}
