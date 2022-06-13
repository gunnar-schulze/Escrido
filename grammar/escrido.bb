// *****************************************************************************
// *                             Argument Scheme 1                             *
// *                        Possibly singular arguments                        *
// *****************************************************************************

// Optional arguments
V                    'Show version.'             {escrido::fVersion = true;}
-version             'Show version.'             {escrido::fVersion = true;}

h                    'Show help.'                {escrido::fHelp = true;}
-help                'Show help.'                {escrido::fHelp = true;}

f                    string
  'Defines the name of a configuration file and turns its usage on.'
                                                 {escrido::sConfigFile = #1;}
-file                string
  'Defines the name of a configuration file and turns its usage on.'
                                                 {escrido::sConfigFile = #1;}

t                    string
  'Template directory. (default "./template/")'   {escrido::sTemplateDir = #1;}
-template            string
  'Template directory. (default "./template/")'   {escrido::sTemplateDir = #1;}

I                    string
  'Defines the files to be included into the documentation. The file names may
   include wildcards, e.g. /usr/src/file*. Multiple values can be given in one
   string seperated by blank spaces. In this case, the string should be embraced
   in quotation marks.'
                                                 {escrido::AppendBlankSepStrings( #1, escrido::saIncludePaths );}
-include             string
  'Defines the files to be included into the documentation. The file names may
   include wildcards, e.g. /usr/src/file*. Multiple values can be given in one
   string seperated by blank spaces. In this case, the string should be embraced
   in quotation marks.'
                                                 {escrido::AppendBlankSepStrings( #1, escrido::saIncludePaths );}

ns                   string
  'Generate documentation only for this namespace. (Multiple use is possible.)'
                                                 {escrido::saNamespaces.push_back( #1 );}
-namespace           string
  'Generate documentation only for this namespace. (Multiple use is possible.)'
                                                 {escrido::saNamespaces.push_back( #1 );}

xg                   string
  'Exclude groups from output. (Multiple use is possible.)'
                                                 {escrido::saExludeGroups.push_back( #1 );}
-exlude-groups       string
  'Exclude groups from output. (Multiple use is possible.)'
                                                 {escrido::saExludeGroups.push_back( #1 );}

it                   onoff
  'Add "internal" tags into output. (default: on)'
                                                 {escrido::fInternalTags = #1;}
-internal-tags       onoff
  'Add "internal" tags into output. (default: on)'
                                                 {escrido::fInternalTags = #1;}

wd                   onoff
  'Flag defining whether a web document shall be created. If this flag is set to "on",
   Escrido will generate HTML output. (default "on")'
                                                 {escrido::fWDOutput = #1;}
-webdoc              onoff
  'Flag defining whether a web document shall be created. If this flag is set to "on",
   Escrido will generate HTML output. (default "on")'
                                                 {escrido::fWDOutput = #1;}

wdo                  string
  'Output directory for web document files. (default "./html/")'
                                                 {escrido::sWDOutputDir = #1;}
-webdoc-output-dir   string
  'Output directory for web document files. (default "./html/")'
                                                 {escrido::sWDOutputDir = #1;}

wdfe                 string
  'Ending of web document output file. (default ".html")'
                                                 {escrido::sWDOutputPostfix = #1;}
-webdoc-file-ending  string
  'Ending of web document output file. (default ".html")'
                                                 {escrido::sWDOutputPostfix = #1;}

rl                   string string
  'Replace a fixed term (e.g. "Signatures", "Return value" etc. ) given as first
   argument by the custom term given as second argument in the output documents.
   See the manual for a full list of possible arguments. (Multiple use is possible.)'
                                                 {escrido::asRelabel.emplace_back( #1, #2 );}

-relabel             string string
  'Replace a fixed term (e.g. "Signatures", "Return value" etc. ) given as first
   argument by the custom term given as second argument in the output documents.
   See the manual for a full list of possible arguments. (Multiple use is possible.)'
                                                 {escrido::asRelabel.emplace_back( #1, #2 );}

si                   onoff
  'Flag defining whether a search index list shall be generated. If this flag is set to "on",
   Escrido will generate such file. (default "off")'
                                                 {escrido::fSearchIndex = #1;}
-search-index        onoff
  'Flag defining whether a search index list shall be generated. If this flag is set to "on",
   Escrido will generate such file. (default "off")'
                                                 {escrido::fSearchIndex = #1;}

sie                  string
  'Encoding type of the search index file. Possible values are\n
- JSON: stored as JSON array
- JS: stored as JavaScript object "searchIndex". (default "JSON")'
{
  if( strcmp( #1, "JS" ) == 0 )
    escrido::fSearchIdxEncode = escrido::search_index_encoding::JS;
  else
    escrido::fSearchIdxEncode = escrido::search_index_encoding::JSON;
}

-search-index-encoding   string
  'Encoding type of the search index file. Possible values are\n
- JSON: stored as JSON array
- JS: stored as JavaScript object "searchIndex". (default "JSON")'
{
  if( strcmp( #1, "JS" ) == 0 )
    escrido::fSearchIdxEncode = escrido::search_index_encoding::JS;
  else
    escrido::fSearchIdxEncode = escrido::search_index_encoding::JSON;
}

sif                  string
  'File path of the search index list file relative to the HTML folder. (default "srchidx.json")'
                                                 {escrido::sSeachIndexFile = #1;}
-search-index-file   string
  'File path of the search index list file relative to the HTML folder. (default "srchidx.json")'
                                                 {escrido::sSeachIndexFile = #1;}

ld                   onoff
  'Flag defining whether a LaTeX document shall be created. If this flag is set to "on",
   Escrido will generate LaTeX output. (default "off")'
                                                 {escrido::fLOutput = #1;}
-latex               onoff
  'Flag defining whether a LaTeX document shall be created. If this flag is set to "on",
   Escrido will generate LaTeX output. (default "off")'
                                                 {escrido::fLOutput = #1;}

ldo                  string
  'Output directory for LaTeX document files. (default "./latex/")'
                                                 {escrido::sLOutputDir = #1;}
-latex-output-dir    string
  'Output directory for LaTeX document files. (default "./latex/")'
                                                 {escrido::sLOutputDir = #1;}

-debug               'Output debug information'  {escrido::fDebug = true;}
