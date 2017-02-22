function [c] = rectCorners(r)

c = [r(1:2),r(1:2)+r(3:4)-1];