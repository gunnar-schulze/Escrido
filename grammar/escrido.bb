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
  'Ending of web document output file. (default ".html" )'
                                                 {escrido::sWDOutputPostfix = #1;}
-webdoc-file-ending  string
  'Ending of web document output file. (default ".html" )'
                                                 {escrido::sWDOutputPostfix = #1;}

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
                                                 
it
  'Show tags of type "internal"'                 {escrido::fShowInternal = true;}
-internal
  'Show tags of type "internal"'                 {escrido::fShowInternal = true;}

-debug               'Output debug information'  {escrido::fDebug = true;}
