// VSProjParse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tinyxml\tinyxml.h"
#include <boost\filesystem.hpp>
#include <vector>
#include <string>

using namespace boost::filesystem;

typedef std::basic_string<wchar_t> wstring;

std::vector<std::string> sourceFiles;
std::vector<std::string> headerFiles;

int basePathLength;

bool IsSourceFile(path file)
{
	wstring str(file.c_str());
	if (str.find(L".c") == str.length() - 2 || str.find(L".cpp") == str.length() - 4)
	{
		return true;
	}
	return false;
}

bool IsHeaderFile(path file)
{
	wstring str(file.c_str());
	if (str.find(L".h") == str.length() - 2 || str.find(L".hpp") == str.length() - 4)
	{
		return true;
	}
	return false;
}

void FindAndRemove(std::vector<std::string>& strVec, std::string entry)
{
	for (int i = 0; i < strVec.size(); ++i)
	{
		if (!strVec[i].compare(entry))
		{
			strVec.erase(strVec.begin() + i);
			return;
		}
	}
}

void AddSource(path file)
{	
	std::string str = file.string();
	str = str.substr(basePathLength, str.length());
	sourceFiles.push_back(str);
}

void AddHeader(path file)
{	
	std::string str = file.string();
	str = str.substr(basePathLength, str.length());
	headerFiles.push_back(str);
}

void FindFilesInDir(path dir)
{
	directory_iterator di(dir);
	while (di != directory_iterator())
	{
		const path& p = di->path();
		if (is_directory(p))
			FindFilesInDir(p);
		if (IsSourceFile(p))
		{
			AddSource(p);
		}
		if (IsHeaderFile(p))
		{
			AddHeader(p);
		}
		++di;
	}
}

int _tmain(int argc, wchar_t* argv[])
{
	wstring pathStr(argv[1]);
	//printf("%s", filename);
	char* utf8Path = (char*)malloc(sizeof(char) * pathStr.length() + 1);
	wcstombs(utf8Path, pathStr.c_str(), pathStr.length() + 1);
	TiXmlDocument doc(utf8Path);
	doc.LoadFile();	
	TiXmlNode* node = doc.LastChild()->FirstChild();

	TiXmlNode* sourceNode;
	TiXmlNode* headerNode;

	path fileLocation(argv[1]);	

	path p = absolute(fileLocation.parent_path());	
	basePathLength = p.string().length() + 1;
	//path p(".");
	
	FindFilesInDir(p);

	while (node)
	{		
		if (!strcmp(node->Value(),"ItemGroup"))
		{
			if (!strcmp(node->FirstChild()->Value(), "ClInclude"))
				headerNode = node;
			if (!strcmp(node->FirstChild()->Value(), "ClCompile"))
				sourceNode = node;
		}
		node = node->NextSibling();
	}	
	TiXmlElement* elem = headerNode->FirstChildElement();
	while (elem)
	{
		FindAndRemove(headerFiles, elem->Attribute("Include"));
		elem = elem->NextSiblingElement();
	}
	elem = sourceNode->FirstChildElement();
	while (elem)
	{
		FindAndRemove(sourceFiles, elem->Attribute("Include"));
		elem = elem->NextSiblingElement();
	}

	for (int i = 0; i < headerFiles.size(); ++i)
	{
		TiXmlElement header("ClInclude");
		header.SetAttribute("Include", headerFiles[i].c_str());
		headerNode->InsertAfterChild(headerNode->LastChild(), header);
	}

	for (int i = 0; i < sourceFiles.size(); ++i)
	{
		TiXmlElement source("ClCompile");
		source.SetAttribute("Include", sourceFiles[i].c_str());
		sourceNode->InsertAfterChild(sourceNode->LastChild(), source);
	}
	doc.SaveFile();
}

