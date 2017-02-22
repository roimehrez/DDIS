function A_padd = padding(A,s)
    % padding
    if size(A,3)==3
        A_padd = cat(3, padding(A(:,:,1),s), ...
            padding(A(:,:,2),s), ...
            padding(A(:,:,3),s));
    else
        [m_A,n_A] = size(A);
        A_padd = repmat(feval(class(A), 0), size(A)+s(1:2)-1);
        A_padd(floor((s(1)-1)/2)+(1:m_A),floor((s(2)-1)/2)+(1:n_A)) = A;
    end
end
