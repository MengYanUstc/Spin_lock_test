expPath = 'C:\Users\y00428318\Documents\MATLAB\spin_lock_curve\';
plotMapFile = [expPath, 'ARM64 ctime fix 1.fig'];

surf(nn,ss,R);
set(gca,'xaxislocation','bottom','yaxislocation','left','ydir','reverse')

h1 = openfig(plotMapFile,'reuse');
D1=get(gca,'Children');
XData1=get(D1,'XData'); %get the x data  
YData1=get(D1,'YData'); %get the y data 

size_x = size(XData1);
x_data_front = XData1(5:32);
x_data_after = XData1(33:size_x(2));

size_y = size(YData1);
y_data_front = YData1(5:32);
y_data_after = YData1(33:size_y(2));

y_front = polyfit(x_data_front,y_data_front,1);
y_after = polyfit(x_data_after,y_data_after,1);

c_front = polyval(y_front, 2);
c_after = polyval(y_after, 2);

front_factor = 0.1;
after_factor = front_factor * c_after/c_front;

nn=(0:1:64);
ss=(0.1:0.1:3.1);
R_front=Speedup(front_factor)
%surf(nn,ss,R_front);
%set(gca,'xaxislocation','bottom','yaxislocation','left','ydir','reverse')
R_after=Speedup(after_factor)
R = [R_front(1:31,1:33),R_after(1:31,34:65)]
surf(nn,ss,R);
set(gca,'xaxislocation','bottom','yaxislocation','left','ydir','reverse')
