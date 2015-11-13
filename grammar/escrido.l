/* -------------------------------------------------------------------------- */

/* DEFINITIONS                                                                */

/* -------------------------------------------------------------------------- */

/* Deactivate yywrap(). Only required on scanning several files on a row. */
%option noyywrap

/* Section of code that will appears early in the output file. */
%{
  #include <string>;          // std::string;
  #include "content-unit.h"   // CContentUnit

  // Include the bison/yacc parser header.
  #include "yescrido.h"

  using namespace escrido;

  // Parsing buffer.
  namespace escrido
  {
    extern CContentUnit oParseContUnit;   // The content unit that is written to while lexing/parsing.
  }

  // Forward declaration of error output function.
  extern void yyerror(const char *s);

  // Global variables required for using C++ strings as input:
  const std::string* pLexSrc; // Pointer to a string used by Lex to read in source data.
  size_t             nLexOff; // Reading position on the source string (in characters).

  // Character position an line number counter.
  unsigned long nLexLine;         // Line number position of the scanner.

  // Replacement of function YY_INPUT(buf,result,max_size);
  int ReadLexInput( char* paBuf_o, unsigned long int* pBytesRead_o, size_t nMaxBytesToRead_i );

  // Redefine the Lex data input function YY_INPUT() and use ReadLexInput() instead.
  #undef YY_INPUT
  #define YY_INPUT(buf,result,max_size) ReadLexInput( buf, &result, max_size )
%}

/* Inclusive (s) and exclusive (x) start states (see Lex manual). */
%x SQMSTRING
%x DQMSTRING
%x MLDOC
%x SLDOC

/* -------------------------------------------------------------------------- */

/* ABBREVIATIONS                                                              */

/* -------------------------------------------------------------------------- */

lb         ((\r\n)|(\r)|(\n)|(\x0c))
wspace     [ \t]
tag        ([a-z0-9])+
identifier [a-zA-Z]([a-zA-Z0-9_])+

%%

  /* ------------------------------------------------------------------------ */

  /* RULES                                                                    */

  /* ------------------------------------------------------------------------ */

  /* Single quoted strings. */
"'"                        { BEGIN SQMSTRING; }                      /* Start of string. */
<SQMSTRING>{lb}            { nLexLine++; }                           /* Count line number. */
<SQMSTRING>"\\".           {}                                        /* Masked characters (including "\'"). */
<SQMSTRING>"'"             { BEGIN 0; }                              /* End of string. */
<SQMSTRING>.               {}                                        /* Anything else. */
<SQMSTRING><<EOF>>         { yyerror( "runaway string at eof" );     /* End of file. */
                             BEGIN 0;
                             return END_OF_FILE; }

  /* Double quoted strings. */
"\""                       { BEGIN DQMSTRING; }
<DQMSTRING>{lb}            { nLexLine++; }
<DQMSTRING>"\\".           {}
<DQMSTRING>"\""            { BEGIN 0; }
<DQMSTRING>.               {}
<DQMSTRING><<EOF>>         { yyerror( "runaway string at eof" );
                             BEGIN 0;
                             return END_OF_FILE; }

  /* Multi line content units. */
"/*#"                      { BEGIN MLDOC;                            /* Start multiline documentation unit. */
                             oParseContUnit.ResetParseState( cont_unit_type::MULTI_LINE ); }
<MLDOC>"#*/"               { BEGIN 0; }                              /* End multiline documentation unit. */
<MLDOC>{lb}                { nLexLine++;                             /* Count line number. */
                             return LB; }
<MLDOC>"@_"{tag}"_"        { yylval.szString = (char*) malloc( strlen( yytext ) );
                             strcpy( yylval.szString, &yytext[1] );
                             return TAG_PAGE_OPEN; }
<MLDOC><<EOF>>             { yyerror( "multi line documentation not closed at eof" );
                             BEGIN 0;
                             return END_OF_FILE; }
<MLDOC>" "                 { return BLANK; }
<MLDOC>"\t"                { return TAB; }
<MLDOC>"@"{tag}            { yylval.szString = (char*) malloc( strlen( yytext ) );
                             strcpy( yylval.szString, &yytext[1] );
                             return TAG; }
<MLDOC>.                   { yylval.cChar = yytext[0];
                             return TEXT_CHAR; }

  /* Single line content units. */
"//#"                      { BEGIN SLDOC;                            /* Start single line documentation unit. */
                             oParseContUnit.ResetParseState( cont_unit_type::SINGLE_LINE ); }
<SLDOC>{lb}{wspace}*"//#"  { nLexLine++;                             /* Stay in single line documentation unit. */
                             return LB; }
<SLDOC>{lb}                { nLexLine++;                             /* Count line number and end multiline unit. */
                             BEGIN 0; }
<SLDOC>"@_"{tag}"_"        { yylval.szString = (char*) malloc( strlen( yytext ) );
                             strcpy( yylval.szString, &yytext[1] );
                             return TAG_PAGE_OPEN; }
<SLDOC><<EOF>>             { BEGIN 0;
                             return END_OF_FILE; }
<SLDOC>" "                 { return BLANK; }
<SLDOC>"\t"                { return TAB; }
<SLDOC>"@"{tag}            { yylval.szString = (char*) malloc( strlen( yytext ) );
                             strcpy( yylval.szString, &yytext[1] );
                             return TAG; }
<SLDOC>.                   { yylval.cChar = yytext[0];
                             return TEXT_CHAR; }

  /* End of file. */
<<EOF>>                    { return END_OF_FILE; }

  /* Other content: dump it. */
{lb}                       { nLexLine++; }                           /* Count line number. */
.                          {}


%%

/* -------------------------------------------------------------------------- */

/* CODE: FUNCTIONS IMPLEMENTATION                                             */

/* -------------------------------------------------------------------------- */

// *****************************************************************************
/// \brief      Initializes the lexical scanner.
///
/// \param[in]  pSrc_i
///             String holding the source data that will be parsed by Lex.
// *****************************************************************************

void InitScanner( const std::string* pSrc_i )
{
  // Set the Lex reader pointer to the source string and reset the position
  // counter.
  pLexSrc = pSrc_i;
  nLexOff = 0;

  // Initialize counter for line numbers.
  nLexLine = 1;

  // Set start condition.
  BEGIN 0;
}

// -----------------------------------------------------------------------------

// *****************************************************************************
/// \brief      Replacement of the Lex function YY_INPUT(buf,result,max_size).
///
/// \details    The function YY_INPUT(buf,result,max_size) is used by Lex to
///             pull the next block of characters from the data source into its
///             cache. Replacing this function is required in order to apply
///             Lex directly to C++ strings.
///
///             The function is called by Lex internally. The argument structure
///             mustn't be changed.
///
/// \param[out] paBuf_o
///             Originally labeled "buf". Cache memory used by the Lex scanner.
/// \param[out] pBytesRead_o
///             Originally labeled "result". Returns either the number of
///             characters actually copy into the cache or YY_NULL (equals 0 on
///             Unix systems) to indicate 'END_OF_FILE'.
/// \param[in]  nMaxBytesToRead_i
///             Originally labeled "max_size". Number of characters the lexer
///             requests to be copied into the cache.
///
/// \return     0 on success.
// *****************************************************************************

int ReadLexInput( char* paBuf_o, unsigned long int* pBytesRead_o, size_t nMaxBytesToRead_i )
{
  // Use the very handy function std::string::copy() to perform the copying
  // into the cache. Use the string pointed to by pLexSrc as input
  // data source (see InitScanner()).
  *pBytesRead_o = pLexSrc->copy( paBuf_o, nMaxBytesToRead_i, nLexOff );
  nLexOff += *pBytesRead_o;

  // If no bytes were copied, set pBytesRead_o to YY_NULL.
  if( *pBytesRead_o == 0 )
    *pBytesRead_o = YY_NULL;

  return 0;
}
