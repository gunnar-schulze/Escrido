
/* ----------------------------------------------------------------- */

/* DEFINITIONEN                                                      */

/* ----------------------------------------------------------------- */

/* Turn on verbose error output. */
%error-verbose

/* Section of code that will appears early in the output file. */
%{
  /* Additional required libraries. */
  #include <stdio.h>
  #include <iostream>                    // cin, cout, cerr, endl
  #include "escrido-doc.h"               // CDocumentation, CContentUnit

  using namespace escrido;

  // Parsing buffers.
  namespace escrido
  {
    extern CDocumentation oDocumentation;  // The documentation content object.
    extern CContentUnit   oParseContUnit;  // The content unit that is written to while lexing/parsing.
  }

  // Character position an line number counter.
  extern unsigned long nLexLine;         // Line number position of the scanner.

  /* Declaration of the lexer function. */
  int yylex(void);

  /* Declaration of the error output function. */
  void yyerror(const char *);
%}

  /* Symbols. */
%union
{
  char  cChar;
  char* szString;
  void* pPointer;    // Pointer to a CContentUnit class.
}

/*%type<pPointer> doc_page doc_page_head*/
%type<szString> TAG_PAGE_OPEN TAG

%token PREC_LO
%token END_OF_FILE
%token BLANK TAB LB
%token <cChar> TEXT_CHAR
%token TAG_PAGE_OPEN TAG
%token PREC_HI

%%

  /* ----------------------------------------------------------------- */

  /* RULES                                                             */

  /* ----------------------------------------------------------------- */

input_document:
          input_document BLANK                   { oParseContUnit.AppendBlank(); }
        | input_document TAB                     { oParseContUnit.AppendTab(); }
        | input_document LB                      { oParseContUnit.AppendLineBreak(); }
        | input_document TEXT_CHAR               { oParseContUnit.AppendChar( $2 ); }
        | input_document TAG                     { oParseContUnit.AppendTag( $2 );
                                                   free( $2 ); }
        | input_document page_head               { oParseContUnit.ResetContent(); }
        | input_document END_OF_FILE             { if( !oParseContUnit.Empty() )
                                                   {
                                                     oParseContUnit.CloseWrite();
                                                     oDocumentation.PushContentUnit( oParseContUnit );
                                                     oParseContUnit.ResetContent();
                                                   }
                                                   return 0; }
        |                                        /* Parsing start point. */
        ;

page_head:
          page_head_open LB
        ;

page_head_open:
          TAG_PAGE_OPEN                          { if( !oParseContUnit.Empty() )
                                                   {
                                                     oParseContUnit.CloseWrite();
                                                     oDocumentation.PushContentUnit( oParseContUnit );
                                                   }
                                                   oDocumentation.NewDocPage( $1 );
                                                   free( $1 ); }
        | page_head_open BLANK                   { oDocumentation.Back()->AppendHeadlineChar( ' ' ); }
        | page_head_open TAB                     { oDocumentation.Back()->AppendHeadlineChar( ' ' ); }
        | page_head_open TEXT_CHAR               { oDocumentation.Back()->AppendHeadlineChar( $2 ); }
        ;

%%

/* ----------------------------------------------------------------- */

/* SUBROUTINES                                                       */

/* ----------------------------------------------------------------- */

void yyerror(const char *s) {
  printf( "parser error in line %u: %s\n", nLexLine, s );
}
