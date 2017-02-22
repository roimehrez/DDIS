function [J] = rectOverlap(r1,R2)
% r1 is a rect [x1,y1,x2,y2]
% R2 is a set of rects 4xN each row is [x1,y1,x2,y2]

x11 = r1(1);
y11 = r1(2);
x12 = r1(3);
y12 = r1(4);
x21 = R2(:,1);
y21 = R2(:,2);
x22 = R2(:,3);
y22 = R2(:,4);

x_overlap = max(0, min(x12,x22) - max(x11,x21) );
y_overlap = max(0, min(y12,y22) - max(y11,y21) );
   
I = x_overlap .* y_overlap;
U = (y12-y11).*(x12-x11) + (y22-y21).*(x22-x21) - I;
J = I./U;