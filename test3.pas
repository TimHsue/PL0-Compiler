VAR x, y, z, q, r, n, f;

PROCEDURE multiply;
VAR a, b;
BEGIN
  a := x;
  b := Y;
  z := 0;
  WHILE b > 0 DO
  BEGIN
    IF ODD b THEN z := z + a;
    a := 2 * a;
    b := b / 2
  END
END;

PROCEDURE divide;
VAR w;
BEGIN
  r := x;
  q := 0;
  w := y;
  WHILE w <= r DO w := 2 * w;
  WHILE w > y DO
  BEGIN
    q := 2 * q;
    w := w / 2;
    IF w <= r THEN
    BEGIN
      r := r - w;
      q := q + 1
    END
  END
END;

PROCEDURE gcd;
VAR f, g;
BEGIN
  f := x;
  g := y;
  WHILE f # g DO
  BEGIN
    IF f < g THEN g := g - f;
    IF g < f THEN f := f - g
  END;
  z := f
END;

PROCEDURE fact;
BEGIN
  IF n > 1 THEN
  BEGIN
    f := n * f;
    n := n - 1;
    CALL fact
  END
END;

BEGIN
  read(x);
  read(y);
  CALL multiply;
  write(z);

  read(x);
  read(y);
  CALL divide;
  write(q);
  write(r);

  read(x);
  read(y);
  CALL gcd;
  write(z);

  read(n);
  f := 1;
  CALL fact;
  write(f);
END.