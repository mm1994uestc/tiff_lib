%%
% C:\Users\Administrator\Desktop\mamiao\Alg_Cpp\image
I = imread('C:/Users/Administrator/Desktop/mamiao/Alg_Cpp/image/A.tiff');
f = fopen('C:/Users/Administrator/Desktop/mamiao/Alg_Cpp/image/A1.bin','r');
[height, width, depth] = size(I);
for i = 1 : width
    for j = 1 : height
        for k = 1 : depth
            tmp = fread(f,1);
            if (I(i,j,k) - tmp) ~= 0
                i,j,k
            end
        end
    end
end
fclose(f);