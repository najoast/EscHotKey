#ifndef  AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_
#define AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_
//读取INI,支持ANSI和UTF8格式编码(如果文件编码是UTF8,则取得的值是Unicode值)
#pragma warning(disable:4786)
#include <string>
#include <vector>
#include <map>
using namespace std;

//////////////////////////////////////////////////////////////////////
// 使用说明：
// 由 [KEY] 行和内容行(NAME=TEXT)组成。[KEY]行的([)必须顶行
// 支持 /* 和 */ 的注释行，带这两个符号的整行都是注释
// 每一行的行末可以带 // 注释符
// KEY、NAME和TEXT，每个都会被剪掉首尾空格
// 例如:ini文件:
// [combobox]
// Ani=public_comboxbg
// 其中combobox是KEY,Ani是NAME,public_comboxbg是TEXT

// #define _USE_STRING2ID

class CIniFile  
{
public:
	CIniFile();

	CIniFile(const char* inipath);

	virtual ~CIniFile();

	void SetPath(const char* newpath);
	const char* GetPath();

	//解析INI
	virtual bool ReadFile();

	//解析INI(调用ReadFile())
	virtual  bool ReadFile(const char* newpath);

	//如果没有找到，返回-1
	int FindKey(const char* keyname);

	//获取某一序号的[KEY]名字
	const char* GetKeyName(int nKeyIndex/*[KEY]的序号*/);

	const char* GetValueName(int nKeyIndex, int nValueIndex);
	const char* GetValue(int  nKeyIndex, const char* sValueName);
	const char* GetValue(int  nKeyIndex, int  nValueIndex);

	bool CopySession(CIniFile& refValue, int nKeyIndex);
	bool CopySession(CIniFile& refValue, const char* keyname);
	//	const char* GetValueName(int nKeyIndex,int nValueIndex);

	//保存ini文件
	void WriteFile(); 

	void Reset();

	//获取[KEY]的数量(m_sections的大小)
	int GetKeyAmount();						

	// 无(=)的整行数量，-1表示无KEY的错误
	int GetLineAmount(const char* keyname);		

	// 有(=)的整行数量，-1表示无KEY的错误
	int GetValueAmount(const char* keyname);	

	const char* GetValue(const char* keyname, const char* valuename); 
	int GetValueI(const char* keyname, const char* valuename); 
	int GetValueI(int keyname, const char* valuename); 
	double GetValueF(const char* keyname, const char* valuename);


	const char* GetLine(const char* keyname, int idx); 

	bool SetValue(const char* key, const char* valuename/*NAME*/, const char* value/*TEXT*/, bool create = 1);
	bool SetValueI(const char* key, const char* valuename, int value, bool create = 1);
	bool SetValueF(const char* key, const char* valuename, double value, bool create = 1);

	bool DelValue(const char* key, const char* valuename);
	bool AddKey(const char* key);
	bool DelKey(const char* key);
	bool DelKey(int idx);

	//设置[]
	bool SetValueLine(const char* key, const char* valueline, bool create = 1);

	//	bool DeleteValue(const char* keyname, const char* valuename);
	//	bool DeleteKey(const char* keyname);

	std::string error;

protected:
	std::string m_strPath;

	struct Value 
	{
		std::string name;//左边的值,NAME
		std::string value;
		std::string line;//一行
	};

	typedef std::vector< Value >		ValueVector;
	struct Section //内容 NAME和TEXT
	{
		std::string name;
		ValueVector vv;//所有行的数据
	};

	typedef std::vector< Section >		SectionVector;
	SectionVector m_sections;//保存所有的[KEY]的向量(每个[KEY]里保存内容向量 NAME和TEXT)

	typedef std::map< std::string, int > Section2Index;
	Section2Index m_section2Index;//保存KEY的map

	ValueVector::iterator FindValueVector(ValueVector& vv, const char* valuename);
};

//去除左右的\n\r\t 字符串
extern void TrimLeftRight(std::string& s);
#endif // !defined(AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_)
