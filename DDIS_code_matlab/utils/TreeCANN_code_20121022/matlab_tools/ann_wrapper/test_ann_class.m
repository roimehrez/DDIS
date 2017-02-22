function test_ann_class

fprintf(1,'start test...\n');
dbstop if error
for dim = 10:25:60
    for n = 2:3
        [anno pts Y] = make_ann(dim,10^n);
        test_ksearch(anno, pts, Y,'ksearch');
        test_ksearch(anno, pts, Y, 'prisearch');
        test_frsearch(anno, pts, Y);
        close(anno);
    end
end

% load test
clear anno;
fprintf(1,'strat load test\n');
for ii=1:6
    anno{ii} = ann(rand(100,10000));
end
for ii=1:10000
    ai = randint(1,1,6)+1;    
    [idx dst] = ksearch(anno{ai}, rand(100,1), randint(1,1,40)+10, 1.0);
    if mod(ii,100)==0
        anno{ai} = close(anno{ai});
        anno{ai} = ann(rand(100,10000));
    end
end
for ii = 1:6
    anno{ii} = close(anno{ii});
end
dbclear if error
fprintf('done testing \n');

%-------------------------------------------%
function [anno pts Y] = make_ann(dim, n)
pts = single(randn(dim,n)*10);
anno = ann(pts);
Y = squareform(pdist(pts'));
Y = single(Y.^2 + 100000*eye(n)); % make diagonal maximal

%-------------------------------------------%

function test_ksearch(anno, pts, Y, fcn)
% test the ksearch/prisearch according to fcn

% retrive the point itself
for ii=1:size(pts,2)
    [idx dst] = feval(fcn,anno, pts(:,ii), 1, 0);
    if idx ~= ii
        error('test_ann:test_ksearch','returned %d for pt %d',idx, ii);
    end
    if dst ~= 0
        error('test_ann:test_ksearch','returned %f dst - expecting zero', dst);
    end
end

% retrive point itself - matrix op
[idx dst] = feval(fcn, anno, pts, 1, 0);
if any(idx~=1:size(pts,2))
    error('test_ann:test_ksearch','did not find point itself');
end
if any(dst~=0)
    error('test_ann:test_ksearch','distance > 0');
end

% retrive NN excluding self
[idx dst] = feval(fcn, anno, pts, 1, 0, false);
if any(idx==1:size(pts,2))
    error('test_ann:test_ksearch','did not exclude self');
end
if any( abs(sum( (pts(:,idx)-pts).^2, 1 ) - dst) > dst.*1e-6 )
    error('test_ann:test_ksearch','wrong distacne');
end
% is it nearest?
if any( abs(min(Y,[],2)-dst(:)) > dst(:).*1e-6 )
    error('test_ann:test_ksearch','not nn');
end

% get k nieghbors
k = randint(1,1,10)+5;
for ii=1:size(pts,2)
    % including self
    [idx dst] = feval(fcn, anno, pts(:,ii), k, 0);
    if numel(idx)~=k || numel(dst)~=k
        error('test_ann:test_ksearch','did not returned k elements');
    end
    if idx(1)~=ii || dst(1) ~= 0
        error('test_ann:test_ksearch','did not find self');
    end
    sd = sort(Y(ii,:),'ascend')';
    if any( abs(dst(2:end)-sd(1:k-1)) > dst(2:end).*1e-6 ) 
        error('test_ann:test_ksearch','did not find nn');
    end
    % excluding self
    [idx dst] = feval(fcn, anno, pts(:,ii), k, 0, false);
    if numel(idx)~=k || numel(dst) ~=k
        error('test_ann:test_ksearch','did not returned k elements');
    end
    if any( abs(dst(:) - sd(1:k) ) > dst.*1e-6 )
       error('test_ann:test_ksearch','did not find nn');
    end 
end

%-------------------------------------------%
function test_frsearch(anno, pts, Y)
rad = 50;
r2 = rad.*rad;
for ii=1:size(pts,2)
    [idx dst inr] = frsearch(anno, pts(:,ii), rad, 1, 0, false);
    if any(dst>r2)
        error('test_ann:test_frseach','dst > r2');
    end
    if sum(Y(ii,:)<=r2) > inr*1.001 || sum(Y(ii,:)<=r2) < inr*.999
        error('test_ann:test_frseach','wrong inr value');
    end
end
