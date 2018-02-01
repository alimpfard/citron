(begin
  (defun strf-def (lambda (x y) (cons x y)))
  (defun srefactor-lisp-symbol-to-skip (list
                                         (strf-def (' defun) 2)
                                         (strf-def (' cons) 2)
                                         (strf-def (' equal) 2)
                                         (strf-def (' member) 2)
                                         (strf-def (' eq?) 2)
                                         (strf-def (' eq) 2)
                                         (strf-def (' set) 2)
                                         (strf-def (' <) 2)
                                         (strf-def (' >) 2)
                                         (strf-def (' <=) 2)
                                         (strf-def (' >=) 2)
                                         (strf-def (' =) 2)
                                         (strf-def (' /=) 2)
                                         ))
  (defun defvar-lst (lambda lst) (if (empty-list? lst) (# True) (begin
                                                                  (set (car lst) (list))
                                                                  (defvar-lst (cdr lst))
                                                                  )))
  (defvar-lst ('
               ( token
                 token-type
                 token-str
                 ignore-num
                 tok-start
                 next-token
                 next-token-start
                 next-token-end
                 next-token-type
                 next-token-str
                 tok-end
                 cur-buf
                 first-token
                 first-token-name
                 second-token
                 lexemes
                 comment-token
                 comment-content
                 token-real-line
                 next-token-real-line
                 comment-real-line-start
                 comment-real-line-end
                 comment-token-start
                 comment-token-end
                 format-type
                 recursive-p
                 orig-format-type
                 )))
  (defun srefactor--
