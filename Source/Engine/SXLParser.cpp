#include "StdAfx.h"
#include "SXLParser.h"

#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

SXLParserManager *GetSXLParserManager(void)
{
  static SXLParserManager _p;
  return &_p;
}

SXLParser::SXLParser(const std::string &strFile, ParseFunc func)
{
  m_Func = func;
  GetSXLParserManager()->RegisterParser(strFile, this);
}

void SXLParserManager::RegisterParser(const std::string &strFile, SXLParser *pParser)
{
  m_ParserMap[GetLowerCase(strFile)] = pParser;
}

void SXLParserManager::Parse(const char *strFile, vx5sprite *spr, bool &bKeepSprite)
{
  std::string strFilename(strFile);
  MakeLowerCase(strFilename);

  ParserMap::iterator it = m_ParserMap.find(strFilename);
  if(it == m_ParserMap.end())
    return;

  Matrix44 mTransform = ConvertFromVoxlap(spr->p, spr->s, spr->h, spr->f);
  it->second->m_Func(spr, mTransform, bKeepSprite);
}
