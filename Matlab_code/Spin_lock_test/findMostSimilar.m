expPath = 'C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\outputFolder\';
theroyPath = 'C:\Users\y00428318\Documents\MATLAB\spin_lock_curve\';
diffFolder = 'C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\diffSurf\'

test_num = 5;
%testTpye = 1 is x86, 0 is ARM
testType = 1;
factor = 0.1;

if(testType == 0)
    outputFile = [diffFolder, 'ARM64,factor=', num2str(factor),'.txt'];
else
    outputFile = [diffFolder, 'x86,factor=', num2str(factor),'.txt'];
end
%outputFile = [diffFolder, 'ARM64,factor=', num2str(factor),'.txt'];
fout = fopen(outputFile, 'wt');


for i=1:8
    ctime = i*5;
    stime = ctime/factor;
    if(testType == 0)
          figFileName = [folderPath, '\ARM64\ARM64,c=', int2str(ctime), ',s=',int2str(stime),'.fig'];
    else
          figFileName = [folderPath, '\x86\x86,c=', int2str(ctime), ',s=',int2str(stime),'.fig'];
    end

    %figure(i);
    h1 = openfig(figFileName,'new');
    D1=get(gca,'Children');
    XData1=get(D1,'XData'); %get the x data  
    YData1=get(D1,'YData'); %get the y data  
    ZData1=get(D1,'ZData');
    
    if(testType == 0)
        theroyFig = [theroyPath, 'ARM64 theory_', num2str(factor), '.fig'];
    else
        theroyFig = [theroyPath, 'theory_', num2str(factor), '.fig'];
    end
    h2 = openfig(theroyFig,'new');
    D2=get(gca,'Children'); %get the handle of the line object  
    ZData2=get(D2,'ZData'); %get the x data  

    sizeOfD1 = size(ZData1);
    ZData2Rev = ZData2';
    ZData2part = ZData2Rev(2: sizeOfD1(1)+1,1:sizeOfD1(2));
    diff = ZData2part - ZData1;
    stdDiff = std2(diff);
    meanDiff = mean(mean(diff));
    fprintf(fout,'c=%d,s=%d,stdDiff=%g,meanDiff=%g\n',ctime,stime,stdDiff,meanDiff);

    s = surf(XData1,YData1,diff);
    %set(gca,'ydir','reverse','xdir','reverse');
    
    if(testType == 0)
          diffFig = [diffFolder, 'ARM64,c=', int2str(ctime), ',s=',int2str(stime)];
    else
          diffFig = [diffFolder, 'x86,c=', int2str(ctime), ',s=',int2str(stime)];
    end
    saveas(s,diffFig,'fig');
     
end

fclose(fout);