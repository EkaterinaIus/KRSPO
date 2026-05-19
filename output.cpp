#include <cstdio>

int main() {
    double a = 0.0;
    double b = 0.0;
    double _t1 = 0.0;
    double _t2 = 0.0;
    double _t3 = 0.0;
    double _t4 = 0.0;

    // (0) *  5.5  2.0  _t1
    _t1 = 5.5 * 2.0;

    // (1) =  _t1  _  a
    a = _t1;

    // (2) +  a  3.0  _t2
    _t2 = a + 3.0;

    // (3) =  _t2  _  b
    b = _t2;

L4:;
    // (4) >  b  0.0  _t3
    _t3 = (b > 0.0) ? 1.0 : 0.0;

    // (5) JF  _t3  _  10
    if (_t3 == 0.0) goto L10;

    // (6) PRINT  b  _  _
    printf("%g\n", b);

    // (7) -  b  1.0  _t4
    _t4 = b - 1.0;

    // (8) =  _t4  _  b
    b = _t4;

    // (9) JMP  _  _  4
    goto L4;

L10:;
    return 0;
}
