(set-option :auto_config false)
(set-option :smt.mbqi false)
(set-option :smt.qi.eager_threshold 3)
(set-option :smt.qi.lazy_threshold 3)

(declare-fun f (Int) Int)

(assert (forall ((x Int)) (! 
    (< (f x) (f (+ x 2)))
    :pattern ((f x))
    :weight 3
)))

(assert (forall ((x Int)) (! 
    (< (f x) (f (+ x 1)))
    :pattern ((f x))
    :weight 1
)))

(assert (< (f 3) 0))
(assert (< 0 (f 0)))

(check-sat)