; TestRegEx.tl
;
; Test regular expressions in TransLISP
; Copyright (c) 2011 by George Moromisato. All Rights Reserved.

(block Nil
	(setq gTotalSuccess 0)
	(setq gTotalFailure 0)
	
	(setq test (lambda (%a b)
		(if (eq (eval %a) b)
			(setq gTotalSuccess (add gTotalSuccess 1))
			(setq gTotalFailure (add gTotalFailure 1))
			)
		))
	
	; Test the ability to find literals
	(test (regex "" "") "")
	(test (regex "a" "") Nil)
	(test (regex "" "a") Nil)
	(test (regex "abc" "abc") "abc")
	(test (regex "abc" "def") Nil)
	(test (regex "abc" "ab") Nil)
	(test (regex "abc" "bc") Nil)
	(test (regex "abc" "abcd") Nil)
	
	; Test reserved characters
	(test (regex "$" "$$") "$")
	(test (regex "{" "${") "{")
	(test (regex "}" "$}") "}")
	(test (regex "+" "$+") "+")
	(test (regex "*" "$*") "*")
	(test (regex "." "$.") ".")
	(test (regex ":" "$:") ":")
	(test (regex "a" "$a") "a")
	(test (regex "!" "!") "!")
	(test (regex "\"" "\"") "\"")
	(test (regex "0" "$0") "0")
	
	; Test any character
	(test (regex "a" "{$?}") "a")
	(test (regex " " "{$?}") " ")
	(test (regex "abc" "{$?}") Nil)
	(test (regex "\n" "{$?}") "\n")
	
	; Test alpha character
	(test (regex "a" "{$A}") "a")
	(test (regex "z" "{$A}") "z")
	(test (regex "A" "{$A}") "A")
	(test (regex "Z" "{$A}") "Z")
	(test (regex "Ã" "{$A}") "Ã")
	(test (regex "0" "{$A}") Nil)
	(test (regex "!" "{$A}") Nil)
	(test (regex "\n" "{$A}") Nil)
	
	; Test numbers
	(test (regex "0" "{$0}") "0")
	(test (regex "5" "{$0}") "5")
	(test (regex " " "{$0}") Nil)
	(test (regex "a" "{$0}") Nil)
	(test (regex "+" "{$0}") Nil)
	(test (regex "-" "{$0}") Nil)
	(test (regex "0" "{$X}") "0")
	(test (regex "7" "{$X}") "7")
	(test (regex "a" "{$X}") "a")
	(test (regex "F" "{$X}") "F")
	(test (regex "g" "{$X}") Nil)
	(test (regex "Z" "{$X}") Nil)
	(test (regex "!" "{$X}") Nil)
	(test (regex "\n" "{$X}") Nil)
	
	; Alpha numerics
	(test (regex "a" "{$A0}") "a")
	(test (regex "z" "{$A0}") "z")
	(test (regex "A" "{$A0}") "A")
	(test (regex "Z" "{$A0}") "Z")
	(test (regex "Ã" "{$A0}") "Ã")
	(test (regex "!" "{$A0}") Nil)
	(test (regex "\n" "{$A0}") Nil)
	(test (regex "0" "{$A0}") "0")
	(test (regex "5" "{$A0}") "5")
	(test (regex " " "{$A0}") Nil)
	(test (regex "+" "{$A0}") Nil)
	(test (regex "-" "{$A0}") Nil)
	
	; Whitespace
	(test (regex " " "{$_}") " ")
	(test (regex "\t" "{$_}") "\t")
	(test (regex "\n" "{$_}") "\n")
	(test (regex "\r" "{$_}") "\r")
	(test (regex "a" "{$_}") Nil)
	(test (regex "!" "{$_}") Nil)
	
	; 0 or more
	(test (regex "a" "a*") "a")
	(test (regex "aaa" "a*") "aaa")
	(test (regex "aaab" "a*") Nil)
	(test (regex "" "a*") "")
	
	; 1 or more
	(test (regex "a" "a+") "a")
	(test (regex "aaa" "a+") "aaa")
	(test (regex "aaab" "a+") Nil)
	(test (regex "" "a+") Nil)
	
	; Sequences
	(test (regex "a" "b*a") "a")
	(test (regex "ba" "b*a") "ba")
	(test (regex "bbba" "b*a") "bbba")
	(test (regex "abbba" "b*a") Nil)
	(test (regex "bbb" "b*a") Nil)
	(test (regex "a" "ab*") "a")
	(test (regex "ab" "ab*") "ab")
	(test (regex "abbb" "ab*") "abbb")
	(test (regex "abbba" "ab*a") "abbba")
	(test (regex "aabbbaa" "a*b*a*") "aabbbaa")
	(test (regex "aabbbaa" "a*b*a*") "aabbbaa")
	(test (regex "baabbbaa" "a*b*a*") Nil)
	(test (regex "aabbbaab" "a*b*a*") Nil)
	
	; Groups
	(test (regex "abcd" "{ab}{cd}") "abcd")
	(test (regex "abcd" "{abcd}") "abcd")
	
	; Wildcards
	(test (regex "abcd" "{$?}*") "abcd")
	(test (regex "abcd" "a{$?}*") "abcd")
	(test (regex "abcd" "{$?}*d") "abcd")
	(test (regex "abcd" "a{$?}*d") "abcd")
	(test (regex "abcd" "a{$?}*{$?}+d") "abcd")
	(test (regex "abcd" "ab{$?}*{$?}+d") "abcd")
	(test (regex "abcd" "abc{$?}*{$?}+d") Nil)
	
	; Sub expressions
	(test (regex "abcd" "a{{$?}*}" 'subex) '(abcd bcd))
	(test (regex "abcd" "a{{$?}*}" '(subex)) '(abcd bcd))
	(test (regex "abcd" "a{{$?}*}d" '(subex)) '(abcd bc))
	(test (regex "abcd" "a{{$?}*}{d}" '(subex)) '(abcd bc d))
	
	(cat "Completed: " gTotalSuccess "  Failed: " gTotalFailure)
	)
