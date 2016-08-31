#include "gtest/gtest.h"
#include "sessionmgr.h"
#include <string>
#define STRINGSIZE 1000
using namespace std;
          
TEST(SessionMgrTest, CreateAndDestroy)
{
    	CSessionMgr *t1 = CSessionMgr::GetInstance();
    	EXPECT_EQ(NULL, t1);
   	CSessionMgr::Create();
    	EXPECT_EQ(0, CSessionMgr::GetInstance()->Init());
    	CSessionMgr *t2 = CSessionMgr::GetInstance();
    	EXPECT_NE(t1, t2);
    	CSessionMgr::Create();
    	EXPECT_EQ(t2, CSessionMgr::GetInstance());

    	CSessionMgr::Destroy();
    	CSessionMgr *t3 = CSessionMgr::GetInstance();
    	EXPECT_EQ(NULL, t3);
    
    	CSessionMgr::Destroy();
    	CSessionMgr *t4 = CSessionMgr::GetInstance();
    	EXPECT_EQ(NULL, t4);    
    
    	CSessionMgr::Create();
    	CSessionMgr::GetInstance()->DeleteInstance();
    	CSessionMgr *t5 = CSessionMgr::GetInstance();
    	EXPECT_EQ(NULL, t5);
    	CSessionMgr::GetInstance()->DeleteInstance();
    	CSessionMgr *t6 = CSessionMgr::GetInstance();
    	EXPECT_EQ(NULL, t6);
}

bool existInStrs(string strs[], int strsSize, const string & str)
{
	for(int i=0; i<strsSize; i++)
	{
		if(strs[i].compare(str) == 0)
			return true;
	}
	return false;
}

void checkLength(string strs[], int strsSize)
{
        for(int i=0; i<strsSize; i++)
        {
                EXPECT_EQ(32u, strs[i].length());
        }
}

TEST(SessionMgrTest, GetSessionId)
{
	string strs[STRINGSIZE];
	string temp;
	CSessionMgr::Create();
	strs[0] = "12345678901234567890123456789012";
	for(int i=1; i < STRINGSIZE; i++)
	{
		CSessionMgr::GetInstance()->GetSessionId(temp);
		EXPECT_FALSE(existInStrs(strs, i, temp));
		strs[i] = temp;
		//cout << temp << endl;
		temp = "";
	}
	temp = "12345678901234567890123456789012";
	EXPECT_TRUE(existInStrs(strs, STRINGSIZE, temp));
	checkLength(strs, STRINGSIZE);
}
