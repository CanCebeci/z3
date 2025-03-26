datatype Expr =
  | Const(value: bool)
  | Var(name: string)
  | If(cond: Expr, thn: Expr, els: Expr)

function FVs(e: Expr): set<string> {
  match e
  case Const(_) => {}
  case Var(name) => {name}
  case If(cond, thn, els) => FVs(cond) + FVs(thn) + FVs(els)
}

function NestedIf(g: Expr, a: Expr, b: Expr, x: Expr, y: Expr): Expr {
  If(If(g, a, b), x, y)
}

function UnnestedIf(g: Expr, a: Expr, b: Expr, x: Expr, y: Expr): Expr {
  If(g, If(a, x, y), If(b, x, y))
}

lemma FVsIf(g: Expr, a: Expr, b: Expr)
  ensures
    FVs(If(g, a, b)) == FVs(g) + FVs(a) + FVs(b)
{
}

lemma FVsIfIf(g: Expr, a: Expr, b: Expr, x: Expr, y: Expr)
  ensures
    FVs(UnnestedIf(g, a, b, x, y)) ==
    FVs(g) + FVs(a) + FVs(b) + FVs(x) + FVs(y)
{
  // WHY does this take so much time?
  
  // calc {
  //   FVs(NestedIf(g, a, b, x, y));
  // ==
  //   FVs(If(If(g, a, b), x, y));
  // ==  { FVsIf(If(g, a, b), x, y); }
  //   FVs(If(g, a, b)) + FVs(x) + FVs(y);
  // ==  { FVsIf(g, a, b); }
  //   FVs(g) + FVs(a) + FVs(b) + FVs(x) + FVs(y);
  // }

  calc {
    FVs(UnnestedIf(g, a, b, x, y));
  ==
    FVs(If(g, If(a, x, y), If(b, x, y)));
  ==  { FVsIf(g, If(a, x, y), If(b, x, y)); }
    FVs(g) + FVs(If(a, x, y)) + FVs(If(b, x, y));
  ==  { FVsIf(a, x, y); FVsIf(b, x, y); }
    FVs(g) + FVs(a) + FVs(x) + FVs(y) + FVs(b) + FVs(x) + FVs(y);
  ==
    FVs(g) + FVs(a) + FVs(b) + FVs(x) + FVs(y);
  }
}
