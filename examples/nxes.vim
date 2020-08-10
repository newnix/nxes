" Vim syntax file
" Language: rc, es, nxes
" Maintainer: Newnix
" Latest Revision: 2020-08-07
" Custom vim syntax file for nxes(1) and es(1) scripts
" should also be mostly applicable to the rc(1) shell from Plan9/9front

syntax clear
"if exists("b:current_syntax")
"	finish
"endif

" Keywords/reserved words
syn keyword esCond if else switch true false case
syn keyword esKwrd echo builtin eval exec limit newpgrp shift umask cd time unwind-protect var vars fork fn local let
syn keyword esFlow while break forever for wait throw catch result return exit signal error eof

" Program fragment delimiters
syn match esCmds "{\|}"

" List delimiters
syn match esList "(\|)"

" Special characters
syn keyword esDlim = ' ` ^ \| < > ; >> <<< <> >< <>> !

" Comments
syn cluster esComments contains=esTodo
syn match esComments "^#\+.*\|[^\$]#\+.*"

" Notes for the future
syn keyword esTodo contained TODO FIXME XXX INFO NOTE HELP

" Strings
syn region esString oneline start='"' end='"' contained

" Constants, including absolute paths
syn match esConst "[^$-]\d\+\|\(/[[:alnum:]]\+\)\+"

" es/nxes hook functions
" these take the form '%[a-z]+(-[a-z]+)?'
syn match esHooks "%\([[:alpha:]]\)\+"

" es/nxes primitives
" these take the form '\$&[a-z]+'
syn match esPrimitives "\$&[[:alpha:]]\+"

" References, given the utility of variables in these shells compared to
" others, they should absolutely stand out
syn match esRefs "\$\{1,\}[*#\^]\{0,1\}[[:alnum:]]\+\|\$\{1,\}\*"

" Settor Functions
syn match esSettor "\(set-\)\([[:alpha:]]\+\)"

" Return result operator
syn match esResult "<=\|=\ <=\|=<=\|="

" Define default highlighting
hi def link esCond Conditional
hi def link esFlow Question
hi def link esComments Comment
hi def link esTodo Todo
hi def link esCmds NonText
hi def link esList Operator
hi def link esDlim Question
hi def link esKwrd Statement
hi def link esConst Constant
hi def link esHooks Underlined
hi def link esRefs Underlined
hi def link esResult WarningMsg
hi def link esPrimitives Statement
hi def link esString String
hi def link esSettor Type
