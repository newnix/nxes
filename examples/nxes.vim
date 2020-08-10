" Vim syntax file
" Language: rc, es, nxes
" Maintainer: Newnix
" Latest Revision: 2020-08-07
" Custom vim syntax file for nxes(1) and es(1) scripts
" should also be mostly applicable to the rc(1) shell from Plan9/9front

if exists("b:current_syntax")
	syntax clear
	finish
endif

" Keywords/reserved words
syn keyword esCond if else switch true false case
syn keyword esKwrd echo @ . builtin eval exec limit newpgrp shift umask cd time unwind-protect var vars %read fork fn local let
syn keyword esFlow while break forever for wait throw catch result return exit signal error eof

" Program fragment delimiters
syn keyword esCmds { }

" List delimiters
syn keyword esList ( )

" Special characters
syn keyword esDlim = ' " ` ^ \| < > ; >> <<< <> >< <>> !

" Notes for the future
syn keyword esTodo contained TODO FIXME XXX INFO NOTE HELP

" Strings
syn region esString oneline start='"' end='"' contained

" Comments
syn cluster esComments contains=@Spell,esTodo
syn match esComments "^#.*\|[^\$]#.*"

" Constants
syn match esConst "[^$-]\d\+"

" es/nxes hook functions
" these take the form '%[a-z]+(-[a-z]+)?'
syn match esHooks "\%\([[:alpha:]]\+\)\(-\w\)?"

" es/nxes primitives
" these take the form '\$&[a-z]+'
syn match esPrimitives "\(\$&[a-z]\+\)"

" References, given the utility of variables in these shells compared to
" others, they should absolutely stand out
syn match esRefs "\$\{1,\}\([*#\^]\)?\([[:alnum:]]\+\)\|\$\{1,\}\*"

" Settor Functions
syn match esSettor "\(set-\)\([[:alpha:]]\+\)"

" Return result operator
syn match esResult "<=\|=\ <=\|=<="

" Define default highlighting
hi def link esCond Conditional
hi def link esComments Comment
hi def link esTodo Todo
hi def link esCmds Statement
hi def link esList Special
hi def link esDlim Special
hi def link esKwrd Statement
hi def link esConst Constant
hi def link esHooks Underlined
hi def link esRefs Error
hi def link esResult WarningMsg
hi def link esPrimitives SpecialKey
hi def link esSettor Type
