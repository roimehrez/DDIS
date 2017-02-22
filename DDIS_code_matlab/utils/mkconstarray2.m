function out = mkconstarray2(class, value, size)
%MKCONSTARRAY creates a constant array of a specified numeric class.
%   A = MKCONSTARRAY(CLASS, VALUE, SIZE) creates a constant array 
%   of value VALUE and of size SIZE.

%   Copyright 1993-2013 The MathWorks, Inc.  

out = repmat(feval(class, value), size);

