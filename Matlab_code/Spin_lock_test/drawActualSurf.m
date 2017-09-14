test_num = 5;
%testTpye = 1 is x86, 0 is ARM
testType = 0;
c_time = int32(75);
s_tests_v = [20 10 5 2 1];

if(testType == 0)
    folderPath = 'C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\outputFolder\';
else
    folderPath = 'C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\outputFolder\';
end

% cores = load('C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\cores.txt');
% cores = cores';
% rate = load('C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\rate.txt');
% rate = rate';
% speedup = load('C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\speedup.txt');
%speedup = speedup;

for i=0:test_num-1 
    corePath = [folderPath, int2str(i), 'core.txt'];
    ratePath = [folderPath, int2str(i), 'rate.txt'];
    speedupPath = [folderPath, int2str(i), 'speedup.txt'];
    
    s_time = c_time * s_tests_v(i+1);
    
    if(testType == 0)
        saveFileName = [folderPath, '\ARM64\ARM64,c=', int2str(c_time), ',s=',int2str(s_time)];
    else
        saveFileName = [folderPath, '\x86\x86,c=', int2str(c_time), ',s=',int2str(s_time)];
    end
    
    cores = load(corePath);
    cores = cores';
    rate = load(ratePath);
    rate = rate';
    speedup = load(speedupPath);
    %speedup = speedup';
    f = figure(i+1);
    
    
    s = surf(cores,rate,speedup);
    set(gca,'ydir','reverse','xdir','reverse');
    saveas(f,saveFileName,'fig');
    if(testType == 0)
        saveFileName = [folderPath, 'ARM64,c=', int2str(c_time), ',s=',int2str(s_time)];
    else
         saveFileName = [folderPath, 'x86,c=', int2str(c_time), ',s=',int2str(s_time)];
    end
    saveas(f,saveFileName,'fig');
   % set(gca,'xaxislocation','bottom','yaxislocation','right');
   
end

%surf(cores,rate,speedup);
%set(gca,'ydir','reverse');