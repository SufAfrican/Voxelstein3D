#pragma once

/// SXL parser helper
class SXLParser
{
public:
  typedef void (*ParseFunc)(struct vx5sprite *spr, Matrix44 &mTransform, bool &bKeepSprite);
  SXLParser(const std::string &strFile, ParseFunc func);

public:
  ParseFunc m_Func;
};

/// SXL parser manager
class SXLParserManager
{
public:
  /// Registers a new parser for the given filename
  void RegisterParser(const std::string &strFile, SXLParser *pParser);
  /// Calls registered parser if any exists
  void Parse(const char *strFile, struct vx5sprite *spr, bool &bKeepSprite);

private:
  typedef std::map<std::string, SXLParser *> ParserMap;
  /// Registered parsers
  ParserMap m_ParserMap;
};

extern SXLParserManager *GetSXLParserManager(void);


#define SXLPARSER_COMBINE2(x,y) x##y
#define SXLPARSER_COMBINE(x,y) SXLPARSER_COMBINE2(x,y)

// Helper for creating a parser
#define SXLPARSER_Start(file) \
  namespace SXLPARSER_COMBINE(SXL, __COUNTER__) \
  { \
    static void SXL_Parse(struct vx5sprite *spr, Matrix44 &mTransform, bool &bKeepSprite); \
    static SXLParser _parser(file, SXL_Parse); \
    static void SXL_Parse(struct vx5sprite *spr, Matrix44 &mTransform, bool &bKeepSprite) \
    {

#define SXLPARSER_End() \
    } \
  }
