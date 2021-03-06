(begin
  (defun empty-list? (lambda (x)
                       (if (list? x)
                         (null? x)
                         (# False)
                         )))
  (defun fmap (lambda (f lst)
                (if (empty-list? lst)
                  lst
                  (cons
                    (f (car lst))
                    (fmap f (cdr lst)
                          )))))
  (defun filter (lambda (f lst)
                  (if (empty-list? lst)
                    lst
                    (if (f (car lst))
                      (cons
                        (car lst)
                        (filter f (cdr lst)))
                      (filter f (cdr lst))
                      ))))
  (defun foldl (lambda (f lst acc)
                 (if (empty-list? lst)
                   acc
                   (foldl f (cdr lst) (f acc (car lst)))
                   )))
  (defun take (lambda (count lst)
                (if (>= 0 count)
                  (' ())
                  (cons
                    (car lst)
                    (take (- count 1) (cdr lst)))
                  )))
  (defun takeWhile (lambda (f lst)
                     (if (empty-list? lst)
                       (' ())
                       (if (f (car lst))
                         (cons
                           (car lst)
                           (takeWhile f (cdr lst)))
                         (' ())
                         ))))
  (defun drop (lambda (count lst)
                (if (empty-list? lst)
                  lst
                  (if (= 0 count)
                    lst
                    (drop (- count 1) (cdr lst))
                  ))))
  (defun dropWhile (lambda (f lst)
                     (if (empty-list? lst)
                       lst
                       (if (f (car lst))
                         (dropWhile f (cdr lst))
                         lst
                       ))))
  (defun member (lambda (e lst)
                  (if (empty-list? lst)
                    (# False)
                    (if (= e (car lst))
                      (# True)
                      (member e (cdr lst))))))
  (defun reverse (lambda (lst)
                   (if (empty-list? (cdr lst))
                     lst
                     (cons
                       (reverse (cdr lst))
                       (car lst)))))
  (defun last (lambda (lst)
                (if (empty-list? (cdr lst))
                  lst
                  (last (cdr lst)))))
  (defun read-all (lambda (c)
                    (if (<= c 0)
                      (' ())
                      (cons
                        (read)
                        (read-all (- c 1))))))
  (defun read-until (lambda (f)
                      (if (f (define c (read)))
                        (list)
                        (cons c (read-until f)))))
  (defun readline (lambda () (# parse applyTo: Program waitForInput)))
  )
