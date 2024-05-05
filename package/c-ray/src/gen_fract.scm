; generate: generates the recursive sphere thingy.
; 	sz - size of sphere
; 	iter - iterations to perform
; 	pos - the position of the sphere
; 	dir - 0 for a central sphere, otherwise px/nx/py/ny/pz/nz for direction
(define generate
  (lambda (sz iter pos dir)
	(let ((scale 0.4) (max-iter 7) (px 1) (nx 2) (py 3) (ny 4) (pz 5) (nz 6))
	  (if (> iter 0)
		(let ((ofs (+ (* sz scale) sz)) (new-scale (* sz scale)))
		  (sphere sz pos)		; generate a sphere here
		  
		  (if (not (= dir nx))	; gen +X
			(generate new-scale (- iter 1) (map + pos (list ofs 0 0)) px))
		  
		  (if (not (= dir px)) ; gen -X
			(generate new-scale (- iter 1) (map + pos (list (- ofs) 0 0)) nx))
		  
		  (if (not (= dir ny)) ; gen +Y
			(generate new-scale (- iter 1) (map + pos (list 0 ofs 0)) py))
		  
		  (if (not (= dir py)) ; gen -Y
			(generate new-scale (- iter 1) (map + pos (list 0 (- ofs) 0)) ny))
		  
		  (if (not (= dir nz)) ; gen +Z
			(generate new-scale (- iter 1) (map + pos (list 0 0 ofs)) pz))
		  
		  (if (not (= dir pz)) ; gen -Z
			(generate new-scale (- iter 1) (map + pos (list 0 0 (- ofs))) nz)))))))


; sphere: outputs a sphere
;   sz - size of the sphere
;   pos - position ...
(define sphere
  (lambda (sz pos)
	(display "s\t")
	(for-each (lambda (x) (display x) (display " ")) pos)
	(display "\t")
	(display sz)
	(display "\t0.25 0.25 0.25  50.0\t0.65")
	(newline)))


; start the process...
(generate 1.0 5 `(0 0 0) 0)			; create the thing
(display "s  0 -10002.25 0  10000  0.2 0.35 0.5  80.0  0.4") (newline) ; create floor
(display "s  0  10100.00 0  10000  0.5 0.2 0.1  40.0  0.0") (newline)    ; create ceiling
(display "l	-50 68 -50") (newline)	; and a light
(display "l	40 40 150")	(newline)	; and another light
(display "c	-9 8 -17 45  0 -1 0")	; and the camera
(newline)
