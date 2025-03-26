; #1614:
(|Set#IsMember| 
    (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|) (_module.__default.FVs ($LS ($LS $LZ)) |a#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ)) |b#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|))
    
    (o@@4!9 (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|) (_module.__default.FVs ($LS ($LS $LZ)) |a#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ)) |b#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) 
            (|Set#Union| (|Set#Union| 
                (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|) 
                (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|))) 
                (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)))))
; skolem var is member of union of FVs of g, a, x, y, b, x, y

; 2428:
(= (_module.__default.FVs ($LS $LZ) (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)) 
   (if (_module.Expr.Const_q (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)) 
        |Set#Empty| 
        (if (_module.Expr.Var_q (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)) 
            (|Set#UnionOne| |Set#Empty| ($Box SeqType (_module.Expr.name (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)))) 
            (|Set#Union| (|Set#Union| 
                (_module.__default.FVs $LZ (_module.Expr.cond (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|))) 
                (_module.__default.FVs $LZ (_module.Expr.thn (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)))) 
                (_module.__default.FVs $LZ (_module.Expr.els (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|)))))))
; Unfold FVs(If(a,x,y)) 

; 2382:
(= (_module.__default.FVs ($LS $LZ) (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)) 
   (if (_module.Expr.Const_q (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)) 
        |Set#Empty| 
        (if (_module.Expr.Var_q (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)) 
            (|Set#UnionOne| |Set#Empty| ($Box SeqType (_module.Expr.name (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)))) 
            (|Set#Union| (|Set#Union| 
                (_module.__default.FVs $LZ (_module.Expr.cond (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|))) 
                (_module.__default.FVs $LZ (_module.Expr.thn (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)))) 
                (_module.__default.FVs $LZ (_module.Expr.els (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)))))))
; Unfold FVs(If(b,x,y))

; 1611:
(= (|Set#IsMember| 
        (|Set#Union| (|Set#Union| 
            (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|)  
            (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|))) 
            (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|))) 
        (o@@4!9 (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (_module.__default.FVs #1206 |g#0@@2|) (_module.__default.FVs #1206 |a#0@@2|)) (_module.__default.FVs ($LS #1205)|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ)) |b#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (|Set#Union| (|Set#Union| (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|) (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|))) (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|)))))
    
    (|Set#IsMember| 
        (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| 
            (_module.__default.FVs ($LS #1205) |g#0@@2|) 
            (_module.__default.FVs ($LS #1205) |a#0@@2|)) 
            (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) 
            (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) 
            (_module.__default.FVs ($LS ($LS $LZ)) |b#0@@2|)) 
            (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) 
            (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) 
        (o@@4!9 (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (|Set#Union| (_module.__default.FVs #1206 |g#0@@2|) (_module.__default.FVs #1206 |a#0@@2|)) (_module.__default.FVs ($LS #1205)|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ)) |b#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|x#0@@2|)) (_module.__default.FVs ($LS ($LS $LZ))|y#0@@2|)) (|Set#Union| (|Set#Union| (_module.__default.FVs ($LS ($LS $LZ)) |g#0@@2|) (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |a#0@@2||x#0@@2||y#0@@2|))) (_module.__default.FVs ($LS ($LS $LZ)) (|#_module.Expr.If |b#0@@2||x#0@@2||y#0@@2|))))))
; skolem var is member of union of FVs of g, a, x, y, b, x, y iff it's member of union of FVs of g, If(a,x,y), If(b,x,y)

Polarities: #1614 (not #2428) (not #2382) #1611