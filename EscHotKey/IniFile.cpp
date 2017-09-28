#include "IniFile.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cassert>

#ifndef WIN32
#include "BaseType.h"
#endif

#pragma warning(disable:4996)

/////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////
CIniFile::CIniFile()
{

}

CIniFile::CIniFile(const char* inipath)
{
    m_strPath = inipath;
}


CIniFile::~CIniFile()
{
}

void CIniFile::SetPath(const char* newpath)
{
    m_strPath = newpath;
}

const char* CIniFile::GetPath()
{
	return m_strPath.c_str();
}

void TrimLeftRight(std::string& s)
{
    const char* whitespace = "\n\r\t ";
    int begin = s.find_first_not_of(whitespace);
    if ( std::string::npos == begin )
    {
        s = "";
        return;
    }
    std::string::size_type end = s.find_last_not_of(whitespace);
    if (end != string::npos)
        s = s.substr( begin, end-begin+1 );
}

bool CIniFile::ReadFile()
{
    FILE* inifile = fopen(m_strPath.c_str(), "rt+");
    if (inifile==NULL)
    {
        error = "Unable to open ini file.";
        return 0;
    }

    std::string keyname, valuename, value, valueline;
    std::string temp;
    bool	bNote = false;
#define BUF_SIZE 2048
    char buf[BUF_SIZE]={0};
	bool bReadUtf8=true;
	while (fgets(buf, BUF_SIZE, inifile))
    {
#if 1
		if (bReadUtf8)
		{
			unsigned char szCmp[]={0xEF,0xBB,0xBF};
			if (memcmp(buf,szCmp,3)==0)
			{
				strcpy(buf,buf + 3);//UTF-8 有BOM格式
			}

			bReadUtf8=false;
		}
#endif

        std::string readinfo = buf;
        if (readinfo != "")
        {
            TrimLeftRight( readinfo );
            if ( readinfo.empty() )
                continue;

            // 2009-9-7 by xuewen
          /*  if (std::string::npos != readinfo.find("//"))
            {
                continue;
            }*/

            if (bNote)
            {
                if ( std::string::npos != readinfo.find("*/") )
                {
                    bNote = false;
                }
                continue;
            }

            if (!bNote)
            {
                if ( std::string::npos != readinfo.find("/*") )
                {
                    bNote = true;
                    continue;
                }
            }

            if (readinfo[0] == '[')// && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
            {
                int pos = readinfo.find_first_of("]");
                if ( std::string::npos == pos )
                {
                    continue;
                }

                readinfo = readinfo.substr( 1, pos-1 );
                keyname = readinfo;
                TrimLeftRight(keyname);
				//支持空Key，2013-10-16 by ZhouYan
				AddKey(keyname.c_str());
            }
            else if ( !readinfo.empty() )
            {
                int pos = readinfo.find_first_of("=");
                if ( std::string::npos == pos )
                {
					pos = readinfo.find_first_of("/");
                    if ( std::string::npos != pos )
                    {
                        valueline = readinfo.substr( 0, pos );
                    }
                    else
                        valueline = readinfo;
                    TrimLeftRight(valueline);
                    if (!valueline.empty())
                        SetValueLine(keyname.c_str(), valueline.c_str());
                }
                else
                {
                    valuename = readinfo.substr( 0, pos );
#if 1
                    std::string::size_type pos2 = readinfo.find("//", pos);
                    if (pos2 != string::npos)
                    {
                        value = readinfo.substr( pos+1, pos2-pos-1 );
                        if (value=="http:"||value=="ftp:")
                            value = readinfo.substr( pos+1 );
                    }
                    else
                    {
#endif
                        value = readinfo.substr( pos+1 );
                    }

                    TrimLeftRight(valuename);
                    TrimLeftRight(value);
                    SetValue(keyname.c_str(),valuename.c_str(),value.c_str());
                }
            }//end_of [else if( !readinfo.empty() )]
        }
    }

    fclose(inifile);
    return true;
}

bool CIniFile::ReadFile(const char* newpath)
{
    SetPath(newpath);
    return ReadFile();
}

void CIniFile::WriteFile()
{
    ofstream inifile(m_strPath.c_str());
    for (int keynum = 0; keynum <(int) m_sections.size(); keynum++)
    {
        //if (!m_sections[keynum].vv.empty()) //支持空Key，2013-10-16 by ZhouYan
        {
            inifile << '[' << m_sections[keynum].name.c_str() << ']' << endl;
			int iSize=(int)m_sections[keynum].vv.size();
            for (int valuenum = 0; valuenum <iSize ; ++valuenum)
            {
                inifile << m_sections[keynum].vv[valuenum].name.c_str()
                << "=" << m_sections[keynum].vv[valuenum].value.c_str()
                << endl;
            }
            inifile << "\r\n";
        }
    }
    inifile.close();
}

void CIniFile::Reset()
{
    m_section2Index.clear();
    m_sections.clear();
}

int CIniFile::GetKeyAmount()
{
    return m_sections.size();
}

int CIniFile::GetLineAmount(const char* keyname)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        Section& s = m_sections[iter->second];
        return s.vv.size();
    }
    else
    {
        return -1;
    }
}

int CIniFile::GetValueAmount(const char* keyname)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        Section& s = m_sections[iter->second];		
		int nValueAmount = 0;
		ValueVector::iterator vvIter = s.vv.begin();
		for (; vvIter != s.vv.end(); ++vvIter)
		{
			if (!vvIter->name.empty())			
				++nValueAmount;
		}
        return nValueAmount;
    }
    else
    {
        return -1;
    }
}


const char* CIniFile::GetLine(const char* keyname, int idx)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        Section& s = m_sections[iter->second];
        if ( (int)s.vv.size() > idx )
        {
            return s.vv[idx].line.c_str();
        }
        else
        {
            error = "Unable to locate specified idx.";
            return "";
        }
    }
    else
    {
        error = "Unable to locate specified key.";
        return "";
    }
}


const char* CIniFile::GetValue(const char* keyname, const char* valuename)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        Section& s = m_sections[iter->second];
        ValueVector::iterator iter_vv = FindValueVector( s.vv, valuename );
        if ( iter_vv != s.vv.end() )
        {
            return (*iter_vv).value.c_str();
        }
        else
        {
            error = "Unable to locate specified value.";
            return "";
        }
    }
    else
    {
        error = "Unable to locate specified key.";
        return "";
    }
}


int CIniFile::GetValueI(const char* keyname, const char* valuename)
{
    return atoi(GetValue(keyname,valuename));
}
int CIniFile::GetValueI(int keyname, const char* valuename)
{
    return atoi(GetValue(keyname,valuename));
}

double CIniFile::GetValueF(const char* keyname, const char* valuename)
{
    return atof(GetValue(keyname, valuename));
}

bool CIniFile::SetValueLine(const char* keyname, const char* valueline, bool create)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        if ( !create )
        {
            return false;
        }
        Section& s = m_sections[iter->second];
        Value v;
        v.line	= valueline;
        s.vv.push_back( v );
    }
    else
    {
        if ( !create )
        {
            return false;
        }
        Section s;
        Value v;
        v.line	= valueline;
        s.vv.push_back( v );
        s.name = keyname;
        m_sections.push_back( s );
        m_section2Index.insert( Section2Index::value_type(keyname, m_sections.size()-1) );

    }
    return  true;
}

CIniFile::ValueVector::iterator CIniFile::FindValueVector(CIniFile::ValueVector& vv, const char* valuename)
{
    for ( ValueVector::iterator iter=vv.begin(); iter!=vv.end(); ++iter )
    {
        if ( 0 == stricmp( (*iter).name.c_str(), valuename ) )
        {
            return iter;
        }
    }
    return vv.end();
}

bool CIniFile::DelValue(const char* keyname, const char* valuename)
{
	Section2Index::iterator iter = m_section2Index.find( keyname );
	if ( iter != m_section2Index.end()
		&& iter->second < (int)m_sections.size() )
	{
		Section& s = m_sections[iter->second];
		ValueVector::iterator iter_vv = FindValueVector( s.vv, valuename );
		if ( iter_vv != s.vv.end() )
		{
			s.vv.erase(iter_vv);			
			return true;
		}
	}
	return false;
}

bool CIniFile::AddKey(const char* keyname)
{
	Section2Index::iterator iter = m_section2Index.find( keyname );
	if ( iter != m_section2Index.end()
		&& iter->second < (int)m_sections.size() )
	{
		return false;
	}
	else
	{
		Section s;	
		s.name = keyname;
		m_sections.push_back( s );
		m_section2Index.insert( Section2Index::value_type(keyname, m_sections.size()-1) );
	}
	return  true;
}

bool CIniFile::DelKey(const char* keyname)
{
	Section2Index::iterator iter = m_section2Index.find( keyname );
	if ( iter != m_section2Index.end()
		&& iter->second < (int)m_sections.size() )
	{
		return false;
	}
	else
	{
		DelKey( iter->second );
		m_section2Index.erase( iter );
	}
	return  true;
}

bool CIniFile::DelKey(int idx)
{
	if ( idx >= (int)m_sections.size() )
	{
		return false;
	}
	else
	{
		SectionVector::iterator iter = m_sections.begin();
		for (int nIdx=0 ; iter != m_sections.end(); ++iter, ++nIdx )
		{
			if (nIdx == idx)
			{
				m_sections.erase( iter );
				return true;
			}
		}
	}

	return false;
}

bool CIniFile::SetValue(const char* keyname, const char* valuename, const char* value, bool create)
{
    Section2Index::iterator iter = m_section2Index.find( keyname );
    if ( iter != m_section2Index.end()
            && iter->second < (int)m_sections.size() )
    {
        Section& s = m_sections[iter->second];
        ValueVector::iterator iter_vv = FindValueVector( s.vv, valuename );
        if ( iter_vv != s.vv.end() )
        {
            (*iter_vv).value	= value;
            (*iter_vv).line		= (*iter_vv).name + "=" + (*iter_vv).value;
        }
        else
        {
            if ( !create )
            {
                return false;
            }
            Value v;
            v.name = valuename;
            v.value = value;
            v.line		= v.name + "=" + v.value;
            s.vv.push_back( v );
        }
    }
    else//属于新的[KEY]里
    {
        if (!create)
        {
            return false;
        }
        Section s;
        Value v;
        v.name = valuename;
        v.value = value;
        v.line		= v.name + "=" + v.value;
        s.vv.push_back( v );
        s.name = keyname;
        m_sections.push_back( s );
        m_section2Index.insert( Section2Index::value_type(keyname, m_sections.size()-1) );
    }
    return  true;
}

bool CIniFile::SetValueI(const char* keyname, const char* valuename, int value, bool create)
{
    //CString temp;
    //temp.Format("%d",value);
    char temp[256] ={0};
    sprintf( temp, "%d",value );
    return SetValue(keyname, valuename, temp, create);
}

bool CIniFile::SetValueF(const char* keyname, const char* valuename, double value, bool create)
{
    //CString temp;
    //temp.Format("%e",value);
    char temp[256] ={0};
    sprintf( temp, "%e",value );
    return SetValue(keyname, valuename, temp, create);
}

/*
bool CIniFile::DeleteValue(const char* keyname, const char* valuename)
{
	Section2V::iterator iter = m_section2V.find( keyname );
	if ( iter != m_section2V.end() )
	{
		Key2Value& k2v = iter->second.kv;
		Key2Value::iterator iter_k2v = k2v.find( valuename );
		if ( iter_k2v != k2v.end() )
		{
			k2v.erase( iter_k2v );
			return true;
		}
	}
	return false;
}

bool CIniFile::DeleteKey(const char* keyname)
{
	Section2V::iterator iter = m_section2V.find( keyname );
	if ( iter != m_section2V.end() )
	{
		m_section2V.erase( iter );
		return true;
	}
	return false;
}
*/

int CIniFile::FindKey(const char* keyname)
{
    if (keyname == NULL)
    {
        return -1;
    }

    for (int i = 0; i < (int)m_sections.size(); i++)
    {
        if (stricmp(m_sections[i].name.c_str(), keyname) == 0)
        {
            return i;
        }
    }
    return -1;
}
#if 0
int CIniFile::FindValue(int keynum, const char* valuename)
{
    assert(false);//先看看什么地方在调用
    return -1;
    /*
    if (keynum == -1)
    	return -1;
    int valuenum = 0;
    //while (valuenum < keys[keynum].names.GetSize() && keys[keynum].names[valuenum] != valuename)
    while (valuenum < keys[keynum].names.size() && keys[keynum].names[valuenum] != valuename)
    {
    	valuenum++;
    }
    //if (valuenum == keys[keynum].names.GetSize())
    if (valuenum == keys[keynum].names.size())
    {
    	return -1;
    }
    return valuenum;
    */
}
#endif

const char* CIniFile::GetValue(int nKeyIndex, const char* sValueName)
{
    if ( nKeyIndex < (int)m_sections.size() )
    {
        Section& s = m_sections[nKeyIndex];
        ValueVector::iterator iter_vv = FindValueVector( s.vv, sValueName );
        if ( iter_vv != s.vv.end() )
        {
            return (*iter_vv).value.c_str();
        }
    }

    return "";
}

const char* CIniFile::GetValue( int nKeyIndex, int nValueIndex )
{
    if ( nKeyIndex <(int) m_sections.size() )
    {
        Section& s = m_sections[nKeyIndex];
        if ((int)s.vv.size() > nValueIndex)
        {
            return s.vv[nValueIndex].value.c_str();
        }
    }

    return "";
}

const char* CIniFile::GetKeyName(int nKeyIndex)
{
    if ( nKeyIndex <(int) m_sections.size() )
    {
        return m_sections[nKeyIndex].name.c_str();
    }
    return "";
    /*
    return names[nKeyIndex].c_str();
    */
}
/*
const char* CIniFile::GetValueName(int nKeyIndex,int nValueIndex)
{
	return keys[nKeyIndex].names[nValueIndex].c_str();
}
*/


//////////////////////////////////////////////////////////////////////////
//helper
#if 0
void CIniFile::GetPos(const char* keyname, const char* valuename, int* x, int* y)
{
    const char* pPos = this->GetValue( keyname, valuename );
    *x = 0;
    *y = 0;
    if (!pPos)
		return ;
    sscanf( pPos, "%d,%d", x, y );
}
#endif

bool CIniFile::CopySession( CIniFile& refValue, int nKeyIndex )
{
    if ( nKeyIndex < (int)refValue.m_sections.size() )
    {
        Section& s = refValue.m_sections[nKeyIndex];
        Section2Index::iterator iter = m_section2Index.find( s.name );
        if ( iter != m_section2Index.end() && iter->second < (int)m_sections.size())
        {
            m_sections[iter->second] = s;
        }
        else
        {
            m_sections.push_back( s );
            m_section2Index.insert( Section2Index::value_type(s.name, m_sections.size()-1) );
        }

        return true;
    }
    return false;
}

bool CIniFile::CopySession( CIniFile& refValue, const char* keyname )
{
    Section2Index::iterator iterSec = refValue.m_section2Index.find( keyname );
    if ( iterSec != refValue.m_section2Index.end()
            && iterSec->second < (int)refValue.m_sections.size() )
    {
        Section& s = refValue.m_sections[iterSec->second];
        Section2Index::iterator iter = m_section2Index.find( s.name );
        if ( iter != m_section2Index.end() && iter->second < (int)m_sections.size())
        {
            m_sections[iter->second] = s;
        }
        else
        {
            m_sections.push_back( s );
            m_section2Index.insert( Section2Index::value_type(s.name, m_sections.size()-1) );
        }

        return true;
    }
    return false;
}

const char* CIniFile::GetValueName( int nKeyIndex, int nValueIndex )
{
    if ( nKeyIndex <(int) m_sections.size() )
    {
        Section& s = m_sections[nKeyIndex];
        if ((int)s.vv.size() > nValueIndex)
        {
            return s.vv[nValueIndex].name.c_str();
        }
    }

    return "";
}