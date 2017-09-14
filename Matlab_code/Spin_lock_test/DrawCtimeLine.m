cores = load('C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\ctime_outputFolder\cores.txt');
ctimes = load('C:\Users\y00428318\Downloads\testcase-lock-test-c-s-p\ctime_outputFolder\ctime.txt');

ctimes_new = ctimes - ctimes(1,1);
figure(1);
scatter(cores, ctimes, 'k');
figure(2);
scatter(cores, ctimes_new, 'k');

hold on;

% 1 is arm, 2 is x86 single line(cpu num: 5-46), 3 is x86 single line(cpu num:13-46)
drawLine = 3;
x=(0:0.2:70);
y_range_ARM = (0:1:600);
y_range_x86 = (0:0.5:250);

if(drawLine==1)
    size_cores = size(cores);
    x_data_front = cores(5:32);
    x_data_after = cores(33:size_cores(1));

    size_ctimes = size(ctimes_new);
    y_data_front = ctimes_new(5:32);
    y_data_after = ctimes_new(33:size_ctimes(1));

    y_front = polyfit(x_data_front,y_data_front,1);
    y_after = polyfit(x_data_after,y_data_after,1);
    
    y_front_e = y_front(1)*x + y_front(2);
    plot(x,y_front_e);
    y_after_e = y_after(1)*x + y_after(2);
    plot(x,y_after_e);
    
   % hold on;
    
    c_front = y_front(1)*2 + y_front(2);
    x_theory_front = 2/c_front * y_range_ARM;
    %y_theory_front = c_front/2 * x_1;
    c_after = y_after(1)*2 + y_after(2);
    x_theory_after = 2/c_after * y_range_ARM;
    %y_theory_after = c_after/2 * x_2;
    
    plot(2,c_front,'k*','markersize',10);
    plot(2,c_after,'k*','markersize',10);
    plot(x_theory_front,y_range_ARM,'r');
    plot(x_theory_after,y_range_ARM,'r');
    

end

if(drawLine ==2)
    size_cores = size(cores);
    x_data = cores(5:size_cores(1));
    
    size_ctimes = size(ctimes_new);
    y_data = ctimes_new(5:size_ctimes(1));
    
    y = polyfit(x_data,y_data,1);
    y_e = y(1)*x + y(2);
    plot(x,y_e);
    
    c= y(1)*2 + y(2);
    x_theory = 2/c * y_range_x86;
    plot(x_theory,y_range_x86,'r');
    plot(2,c,'k*','markersize',10);
end

if(drawLine ==3)
    size_cores = size(cores);
    x_data = cores(13:size_cores(1));
    
    size_ctimes = size(ctimes_new);
    y_data = ctimes_new(13:size_ctimes(1));
    
    y = polyfit(x_data,y_data,1);
    y_e = y(1)*x + y(2);
    plot(x,y_e);
    
    c= y(1)*2 + y(2);
    x_theory = 2/c * y_range_x86;
    plot(x_theory,y_range_x86,'r');
    plot(2,c,'k*','markersize',10);
end

