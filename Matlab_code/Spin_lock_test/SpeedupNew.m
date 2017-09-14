function [ R ] = SpeedupNew(slope, delta)
%UNTITLED3 Summary of this function goes here
% Detailed explanation goes here
s(1)=(300 + delta);
c(1)=slope;
for i=1:31 
    a(i) = 1000*s(1)/i;
end

param = parcluster('local');
param.NumWorkers = 4;
parpool(param, param.NumWorkers);
for i=1:31
    parfor j=0:64
        R(i,j+1)=0;
        for k=0:j;
            R(i,j+1)=R(i,j+1)+k*Pk(a(i), s(1), c(1), k, j);
        end
        R(i,j+1)=j-R(i,j+1);
    end
    i
end
delete(gcp)
end

function [ S ] = SingleSpeedup(a, s, c, n)
%UNTITLED3 Summary of this function goes here
% Detailed explanation goes here
for i=0:n
    S(i+1)=0;
    for k=0:i;
        S(i+1)=S(i+1)+k*Pk(a, s, c, k, i);
    end
    S(i+1)=i-S(i+1);
end
end

function [ R ] = Pk(a, s, c, k, n)
%UNTITLED3 Summary of this function goes here
% Detailed explanation goes here
R=Single(a, s, c, k, n)/PAll(a, s, c, n);
end

function [ P ] = PAll(a, s, c, n)
%UNTITLED3 Summary of this function goes here
% Detailed explanation goes here
P=0;
for i=0:n
    P=P+Single(a, s, c, i, n);
end
end

function [ R ] = Single(a, s, c, k, n)
%UNTITLED3 Summary of this function goes here
% Detailed explanation goes here
R=prod(s+(1:k)*c)/((a^k)*factorial(n-k));
end
