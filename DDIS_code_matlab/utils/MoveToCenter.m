function [Iout] = MoveToCenter(I, szT, v)

szI = size(I);

r0i = 1;
c0i = 1;
r1i = szI(1)-szT(1)+1;
c1i = szI(2)-szT(2)+1;
r0o = round(szT(1)/2);
c0o = round(szT(2)/2);
r1o = round(szT(1)/2)+szI(1)-szT(1);
c1o = round(szT(2)/2)+szI(2)-szT(2);

Iout = ones(size(I));
Iout = v(Iout);
Iout(r0o:r1o,c0o:c1o) = I(r0i:r1i,c0i:c1i);