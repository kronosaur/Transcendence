; Test program

(block (theDict lines startTime)

	(dbgOutput "Loading dictionary...")
	(setq startTime (sysTicks))
	(setq lines 0)

	; Load the pronounciation dictionary
	; Each line is an item in the list. We skip comment lines that start with two semi-colons
	(setq theDict
		(pageMap "\\\\leibowitz\\core\\booty\\datasets\\cmudict\\cmudict.0.txt" 'textLine 'excludeNil theLine
			(block (parsedLine)
				(setq parsedLine (regex theLine "{{$A}+}{$_}+{{$A}{$?}*}" 'subex))
				
				(setq lines (add lines 1))
				(if (eq (modulo lines 1000) 0) (if parsedLine (dbgOutput (item parsedLine 1))))
				
				parsedLine
				)
			)
		)
		
	; Find pronounciation for numbers and hex letters
	(dbgOutput (find theDict "ONE" 1))
	
	; Sort the dictionary by pronounciation (for easy searching)
	(setq theDict (sort theDict 2))
	
	(for i 0 100 (dbgOutput (item theDict i)))

	(dbgOutput "Time: " (divide (subtract (sysTicks) startTime) 1000) " seconds.")
	""
	)
