#ifndef  AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_
#define AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_
//��ȡINI,֧��ANSI��UTF8��ʽ����(����ļ�������UTF8,��ȡ�õ�ֵ��Unicodeֵ)
#pragma warning(disable:4786)
#include <string>
#include <vector>
#include <map>
using namespace std;

//////////////////////////////////////////////////////////////////////
// ʹ��˵����
// �� [KEY] �к�������(NAME=TEXT)��ɡ�[KEY]�е�([)���붥��
// ֧�� /* �� */ ��ע���У������������ŵ����ж���ע��
// ÿһ�е���ĩ���Դ� // ע�ͷ�
// KEY��NAME��TEXT��ÿ�����ᱻ������β�ո�
// ����:ini�ļ�:
// [combobox]
// Ani=public_comboxbg
// ����combobox��KEY,Ani��NAME,public_comboxbg��TEXT

// #define _USE_STRING2ID

class CIniFile  
{
public:
	CIniFile();

	CIniFile(const char* inipath);

	virtual ~CIniFile();

	void SetPath(const char* newpath);
	const char* GetPath();

	//����INI
	virtual bool ReadFile();

	//����INI(����ReadFile())
	virtual  bool ReadFile(const char* newpath);

	//���û���ҵ�������-1
	int FindKey(const char* keyname);

	//��ȡĳһ��ŵ�[KEY]����
	const char* GetKeyName(int nKeyIndex/*[KEY]�����*/);

	const char* GetValueName(int nKeyIndex, int nValueIndex);
	const char* GetValue(int  nKeyIndex, const char* sValueName);
	const char* GetValue(int  nKeyIndex, int  nValueIndex);

	bool CopySession(CIniFile& refValue, int nKeyIndex);
	bool CopySession(CIniFile& refValue, const char* keyname);
	//	const char* GetValueName(int nKeyIndex,int nValueIndex);

	//����ini�ļ�
	void WriteFile(); 

	void Reset();

	//��ȡ[KEY]������(m_sections�Ĵ�С)
	int GetKeyAmount();						

	// ��(=)������������-1��ʾ��KEY�Ĵ���
	int GetLineAmount(const char* keyname);		

	// ��(=)������������-1��ʾ��KEY�Ĵ���
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

	//����[]
	bool SetValueLine(const char* key, const char* valueline, bool create = 1);

	//	bool DeleteValue(const char* keyname, const char* valuename);
	//	bool DeleteKey(const char* keyname);

	std::string error;

protected:
	std::string m_strPath;

	struct Value 
	{
		std::string name;//��ߵ�ֵ,NAME
		std::string value;
		std::string line;//һ��
	};

	typedef std::vector< Value >		ValueVector;
	struct Section //���� NAME��TEXT
	{
		std::string name;
		ValueVector vv;//�����е�����
	};

	typedef std::vector< Section >		SectionVector;
	SectionVector m_sections;//�������е�[KEY]������(ÿ��[KEY]�ﱣ���������� NAME��TEXT)

	typedef std::map< std::string, int > Section2Index;
	Section2Index m_section2Index;//����KEY��map

	ValueVector::iterator FindValueVector(ValueVector& vv, const char* valuename);
};

//ȥ�����ҵ�\n\r\t �ַ���
extern void TrimLeftRight(std::string& s);
#endif // !defined(AFX_INIFILE2_H__D6BE0D97_13A8_11D4_A5D2_002078B03530__INCLUDED_)
