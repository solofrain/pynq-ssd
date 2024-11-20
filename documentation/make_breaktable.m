# COnstants for 10kohm thermistor from Omega web site
A=1.032e-3;
B=2.387e-4;
C=1.580e-7;

#Arrays for intermediate results
R=ones(50,1);
V=ones(50,1);
ADU=ones(50,1);
t=ones(50,1);

#Output file
f=fopen("bptTherm10kdegC_2.dbd","w");

#Name of breaktable
fprintf(f,"breaktable(Therm10kdegC) {\n");

#Calculate R values & resulting ADC values from -70 C to 130 C in 4 deg. steps
for ci=0:50;
   i=ci*4;
   T=(i-70)+273.15; 
   t(ci+1)=i-70;
   alpha=((A-(1/T))/C);
   beta=sqrt((((B/(3*C))^3)+(alpha^2)/4));
   R(ci+1)=e^((beta-(alpha/2))^(1/3)-((beta+(alpha/2))^(1/3)));
# 3V reference, 12-bit ADC
   V(ci+1)=R(ci+1)/(R(ci+1)+1e5)*3.0;
   ADU(ci+1)=int32(V(ci+1)/3*4096);

# Print table
   fprintf(f,"   %g  %g\n",ADU(ci+1),t(ci+1));
   printf("   %g  %g\n",ADU(ci+1),t(ci+1));
endfor;

#Closing bracket
fprintf(f,"}\n");
fclose(f);

