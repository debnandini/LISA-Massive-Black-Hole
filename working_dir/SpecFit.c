/**************************************************************************
 
 Copyright (c) 2019 Neil Cornish
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 ************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Utilities.h"
#include "ConstSpec.h"
#include "SpecFit.h"

#ifndef _OPENMP
#define omp ignore
#endif

#define verbose 1   // set to 0 for quiet run, 1 for verbose

static const gsl_rng_type *rngtype;
static const gsl_rng *rng;

//OSX
// clang -Xpreprocessor -fopenmp -lomp -w -o SpecFit SpecFit.c Utilities.c -lgsl  -lm

// Linux
// gcc -std=gnu99 -fopenmp -w -o SpecFit SpecFit.c Utilities.c -lgsl -lm



//int main(int argc, char *argv[])
int main_gut(char *file, int ch)
{

  int i, j, k, kk, ND, N, Ns, Nm, m, mc, dec;
  int imin, imax, acS, acL, cS, cL;
  int typ, sm;
  double *times, *data, *Hf;
  double x, y, z, dt, df, Tobs, ttrig, fny, fmn, fmx;
  double f, finc;
  double *SN, *SM, *SL, *PS, *S1, *S2;
  int Nsp, Nl;
  char command[1024];
  double spread, *deltafmax, *linew;
  double *linef, *lineh, *lineQ;
  double f2, f4, ff4, ff2, df2, df4;
  double ylinew, ylinef, ylineh, ylineQ, ydeltafmax;
  int ii, flag, Nlines;
  double xold, xnext, Abar;
  double max;
  //int Nspline, ch;
  int Nspline;

   double dfmin;  // minimum spline spacing
   double dfmax;  // maximum spline spacing
   double smooth;   // moving average
   double  tol=0.2;     // tolerance for difference in averages
   double pmul=5.0e-5;   // strength of prior on spline second derivatives
   int MM=400000;    // iterations of MCMC
    
   double alpha;
   double tuke = 1.0e5;    // Tukey window rise (s)
    
   const gsl_rng_type * T;
   gsl_rng * r;
    
   gsl_rng_env_setup();
    
   T = gsl_rng_default;
   r = gsl_rng_alloc (T);
    
  FILE *in;
  FILE *out;
    
    //if(argc!=3)
    //{
        //printf("./SpecFit filename channel\n");
        //return 1;
    //}
    
    //ch = atoi(argv[2]);
    
      //in = fopen(argv[1],"r");
      in = fopen(file,"r");
       N = -1;
       while(!feof(in))
       {
           fscanf(in,"%lf%lf%lf%lf", &x, &y, &x, &y);
           N++;
       }
       rewind(in);
       printf("Number of points = %d\n", N);
    
      fscanf(in,"%lf%lf%lf%lf", &x, &y, &y, &y);
      fscanf(in,"%lf%lf%lf%lf", &z, &y, &y, &y);
      rewind(in);
    
       dt = z-x;
       Tobs = (double)(N)*dt;
       df = 1.0/Tobs;
       fny = 1.0/(2.0*dt);  // Nyquist
    
       dfmin = df*16.0;
       dfmax = df*1024.0;
       smooth = 2.0*dfmin;
    
    printf("dt %f Tseg %f Nyquist %e  df %e\n", dt, Tobs, fny, df);

    times = (double*)malloc(sizeof(double)* (N));
    data = (double*)malloc(sizeof(double)* (N));
    Hf = (double*)malloc(sizeof(double)* (N));
    
    if(ch == 0)
    {
      for (i = 0; i < N; ++i)
       {
        fscanf(in,"%lf%lf%lf%lf", &times[i], &data[i], &x, &y);
        Hf[i] = 0.0;
       }
    }
    else if(ch == 1)
    {
        for (i = 0; i < N; ++i)
          {
            fscanf(in,"%lf%lf%lf%lf", &times[i], &x, &data[i], &y);
            Hf[i] = 0.0;
            }
    }
    else if(ch == 2)
       {
         for (i = 0; i < N; ++i)
            {
             fscanf(in,"%lf%lf%lf%lf", &times[i], &x, &y, &data[i]);
             Hf[i] = 0.0;
            }
       }
    fclose(in);

    
    Ns = N/2;
    
    S1 = (double*)malloc(sizeof(double)*(Ns));
    S2 = (double*)malloc(sizeof(double)*(Ns));
    SL = (double*)malloc(sizeof(double)*(Ns));
    SM = (double*)malloc(sizeof(double)*(Ns));
    SN = (double*)malloc(sizeof(double)*(Ns));
    PS = (double*)malloc(sizeof(double)*(Ns));
    
    printf("Data volume %f seconds, %f Hz\n", Tobs, fny);
    
    specest(data, Hf, N, Ns, dt, fny, SN, SM, PS);
    
    if(verbose == 1)
    {
    out = fopen("spec.dat","w");
    for (i = 0; i < Ns; ++i)
    {
        f = (double)(i)/Tobs;
        fprintf(out,"%.15e %.15e %.15e %.15e\n", f, SN[i], SM[i], PS[i]);
    }
    fclose(out);
    }
    
    
    // moving average
    sm = (int)(smooth*Tobs);
    x = 0.0;
    for (i = 0; i < sm; ++i) x += SM[i];
    for (i = sm; i < Ns; ++i)
    {
        S1[i-sm/2] = x/(double)(sm);
        x += SM[i] - SM[i-sm];
    }
    
    // moving average with wider window
    sm *= 2;
    x = 0.0;
    for (i = 0; i < sm; ++i) x += SM[i];
    for (i = sm; i < Ns; ++i)
    {
        S2[i-sm/2] = x/(double)(sm);
        x += SM[i] - SM[i-sm];
    }
    
    // fill initial bins
    for (i = 0; i < sm/2; ++i)
    {
        S1[i] = SM[i];
        S2[i] = 2.0*S1[i];
    }
    

     if(verbose == 1)
     {
    out = fopen("smooth.dat","w");
    for (i = sm/2; i < Ns; ++i)
    {
        f = (double)(i)/Tobs;
        fprintf(out,"%.15e %.15e %.15e\n", f, S1[i], S2[i]);
    }
    fclose(out);
     }
    
    
    
   // count the number of spline knots
    Nspline = 1;
    k = (int)(dfmin*Tobs);
    kk = (int)(dfmax*Tobs);
    j = 0;
    flag = 0;
    max = 0.0;
    for (i = 1; i < Ns; ++i)
    {
        x = fabs(S2[i]/S1[i]-1.0);
        if(x > max) max = x;
        j++;
         if(i%k == 0)
          {
              if(max > tol || j == kk)
              {
                 // printf("%f %f %f\n", (double)(i-j/2)/Tobs, (double)(j)/Tobs, max);
                  max = 0.0;
                 // printf("%d %d %d\n", j, k, kk);
                  j = 0;
                  Nspline++;
              }
          }
        
        
    }
    
     Nspline++;
    
     printf("There are %d spline knots\n", Nspline);
    
     double *ffit, *Xspline, *Yspline;
     
     ffit = (double*)malloc(sizeof(double)*(Nspline));
     Xspline = (double*)malloc(sizeof(double)*(Nspline));
     Yspline = (double*)malloc(sizeof(double)*(Nspline));
    
    Nspline = 1;
    j = 0;
    flag = 0;
    max = 0.0;
    for (i = 1; i < Ns; ++i)
    {
        x = fabs(S2[i]/S1[i]-1.0);
        if(x > max) max = x;
        j++;
         if(i%k == 0)
          {
              if(max > tol || j == kk)
              {
                  max = 0.0;
                  ffit[Nspline] = (double)(i-j/2)/Tobs;
                  Xspline[Nspline] = log(S1[i-j/2]);
                  j = 0;
                  Nspline++;
              }
          }
        
        
    }
    
    Nspline++;
    
    ffit[0] = 0.0;
    Xspline[0] = log(SM[0]);
    ffit[Nspline-1] = (double)(Ns-1)/Tobs;
    Xspline[Nspline-1] = log(SM[Ns-1]);
    
    
     for (i = 0; i < Nspline; ++i)
        {
            Yspline[i] = Xspline[i];
        }
    
    // Allocate spline
    gsl_spline   *cspline = gsl_spline_alloc(gsl_interp_cspline, Nspline);
    gsl_interp_accel *acc    = gsl_interp_accel_alloc();
    
    // compute spline
    gsl_spline_init(cspline,ffit,Xspline,Nspline);
     
    
     if(verbose == 1)
     {
    out = fopen("control.dat","w");
       for (i = 0; i < Nspline; ++i)
       {
           fprintf(out,"%e %e\n", ffit[i], exp(Xspline[i]));
       }
       fclose(out);
     }
     

      // count the number of lines
     j = 0;
     flag = 0;
     for (i = 0; i < Ns; ++i)
     {
         x = PS[i]/SM[i];
         // start of a line
         if(x > 9.0 && flag == 0)
         {
             k = 1;
             flag = 1;
             max = x;
             ii = i;
         }
         // in a line
         if(x > 9.0  && flag ==1)
         {
             k++;
             if(x > max)
             {
                 max = x;
                 ii = i;
             }
         }
         // have reached the end of a line
         if(flag == 1)
         {
             if(x < 9.0)
             {
                 flag = 0;
                 j++;
             }
         }
     }
    
     
     Nlines = j;
    


       linef = (double*)malloc(sizeof(double)*(Nlines));  // central frequency
       lineh = (double*)malloc(sizeof(double)*(Nlines));  // line height
       lineQ = (double*)malloc(sizeof(double)*(Nlines)); // line Q
       linew = (double*)malloc(sizeof(double)*(Nlines));  // line width
       deltafmax = (double*)malloc(sizeof(double)*(Nlines));  // cut-off
    
    j = -1;
    xold = 1.0;
    flag = 0;
    for (i = 0; i < Ns; ++i)
    {
        x = PS[i]/SM[i];
        // start of a line
        if(x > 9.0 && flag == 0)
        {
            k = 1;
            flag = 1;
            max = x;
            ii = i;
        }
        // in a line
        if((x > 9.0) && flag ==1)
        {
            k++;
            if(x > max)
            {
                max = x;
                ii = i;
            }
        }
        // have reached the end of a line
        if(flag == 1)
        {
            if(x < 9.0)
            {
                flag = 0;
                j++;
                linef[j] = (double)(ii)/Tobs;
                lineh[j] = (max-1.0)*SM[ii];
                Abar = 0.5*(SN[ii+1]/SM[ii+1]+SN[ii-1]/SM[ii-1]);
                //lineQ[j] = sqrt((max/Abar-1.0))*linef[j]*Tobs;
                 lineQ[j] = sqrt(max)*linef[j]*Tobs/(double)(k);
                
                  spread = (1.0e-2*lineQ[j]);
                  if(spread < 50.0) spread = 50.0;  // maximum half-width is f_resonance/50
                  deltafmax[j] = linef[j]/spread;
                  linew[j] = 8.0*deltafmax[j];
                
                //printf("%d %e %e %e %e %e\n", j, linef[j], lineh[j], linew, (double)(k)/Tobs, lineQ[j]);
               
            }
        }

    }
    
    printf("There are %d lines\n", Nlines);
    
    // initialize smooth spline spectrum
    SM[0] = exp(Xspline[0]);
    SM[Ns-1] = exp(Xspline[Nspline-1]);
    for (i = 1; i < Ns-1; ++i)
    {
        f = (double)(i)/Tobs;
        SM[i] = exp(gsl_spline_eval(cspline,f,acc));
    }

    
    // initialize line spectrum
    //out = fopen("lines.dat","w");
    for (i = 0; i < Ns; ++i)
    {
        f = (double)(i)/Tobs;
        y = 0.0;
        for (j = 0; j < Nlines; ++j)
        {
           
            x = fabs(f - linef[j]);
            
            if(x < linew[j])
            {
                z = 1.0;
                if(x > deltafmax[j]) z = exp(-(x-deltafmax[j])/deltafmax[j]);
                f2 = linef[j]*linef[j];
                ff2 = f*f;
                df2 = lineQ[j]*(1.0-ff2/f2);
                df4 = df2*df2;
                y += z*lineh[j]/(ff2/f2+df4);
               // printf("%d %d %e %e %e\n", i, j, f2, ff2, df4);
            }
        }
        SL[i] = y;
        //fprintf(out,"%e %e %e\n", f, SM[i] + SL[i], SL[i]);
        
    }
   // fclose(out);
    
    
    
    // changing a control point value impacts the spline in two segments
    // either side of the point changed.
    
    double *freqs, *lnLR, *lnpR;
    
    freqs = (double*)malloc(sizeof(double)*(Ns));
    lnLR = (double*)malloc(sizeof(double)*(Ns));
    lnpR = (double*)malloc(sizeof(double)*(Ns));
     
    
     if(verbose == 1)
     {
       out = fopen("specstart.dat","w");
       for (i = 0; i < Ns; ++i)
       {
           f = (double)(i)/Tobs;
           fprintf(out,"%e %e %e %e\n", f, SM[i], SL[i], SM[i]+SL[i]);
       }
       fclose(out);
     }
   
    for (i = 0; i < Ns; ++i)
     {
         freqs[i] = (double)(i)/Tobs;
         x = SM[i] + SL[i];
         lnLR[i] = -(log(x) + PS[i]/x);
         lnpR[i] = 0.0;
     }
    
     for (i = 1; i < Ns-1; ++i)
        {
          lnpR[i] = -fabs(gsl_spline_eval_deriv2(cspline, freqs[i], acc));
        }
    
     double *DlnLR, *DlnpR, *DS;
     
    // This holds the updated values in the region impacted by the change in the
    // spline point. Allocating these to the size of the full spectrum
     DlnLR = (double*)malloc(sizeof(double)*(Ns));
     DlnpR = (double*)malloc(sizeof(double)*(Ns));
     DS = (double*)malloc(sizeof(double)*(Ns));
    
    double logLx, logLy, logpx, logpy;
    double H;
    
      logLx = 0.0;
      logpx = 0.0;
      for (i = 0; i < Ns; ++i)
      {
          logLx += lnLR[i];
          logpx += lnpR[i];
      }
    
    acS = 0;
    acL = 0;
    cS = 1;
    cL = 1;
    
   if(verbose==1) out = fopen("schain.dat","w");
    
     for (mc = 0; mc < MM; ++mc)
     {
         
         // prune any weak lines
         if(mc == MM/4)
         {
             j = 0;
             for (k = 0; k < Nlines; ++k)
             {
                 i = (int)(linef[k]*Tobs);  // bin where line peaks
                 x = lineh[k]/SM[i];     // height of line relative to smooth
                 if(x > 5.0)  // keep this line
                 {
                     linef[j] = linef[k];
                     lineh[j] = lineh[k];
                     lineQ[j] = lineQ[k];
                     linew[j] = linew[k];
                     deltafmax[j] = deltafmax[k];
                     j++;
                 }
             }
             
             Nlines = j;
             
             // reset line spectrum
             for (i = 0; i < Ns; ++i)
             {
                 f = freqs[i];
                 y = 0.0;
                 for (j = 0; j < Nlines; ++j)
                 {
                    
                     x = fabs(f - linef[j]);
                     
                     if(x < linew[j])
                     {
                         z = 1.0;
                         if(x > deltafmax[j]) z = exp(-(x-deltafmax[j])/deltafmax[j]);
                         f2 = linef[j]*linef[j];
                         ff2 = f*f;
                         df2 = lineQ[j]*(1.0-ff2/f2);
                         df4 = df2*df2;
                         y += z*lineh[j]/(ff2/f2+df4);
                     }
                 }
                 SL[i] = y;
             }
             
             // reset likelihood
             for (i = 0; i < Ns; ++i)
             {
                 x = SM[i] + SL[i];
                 lnLR[i] = -(log(x) + PS[i]/x);
             }
             logLx = 0.0;
             for (i = 0; i < Ns; ++i) logLx += lnLR[i];
             
          }
         
         
         alpha = gsl_rng_uniform(r);
         
         if(alpha < 0.6) // update spline
         {
             
          typ = 0;
          cS++;
             
        for (i = 0; i < Nspline; ++i) Yspline[i] =  Xspline[i];
             
        // pick a knot to update
        k = (int)((double)(Nspline)*gsl_rng_uniform(r));
        Yspline[k] =  Xspline[k] + gsl_ran_gaussian(r,0.05);
        gsl_spline_init(cspline,ffit,Yspline,Nspline);
             
         /*
         if(k > 1 && k < Nspline-2)
         {
             imin = (int)(ffit[k-2]*Tobs);
             imax = (int)(ffit[k+2]*Tobs);
         }
          
         if(k <= 2)
         {
             imin = 0;
             imax = (int)(ffit[4]*Tobs);
         }
         
         if(k >= Nspline-2)
         {
             imax = Ns-1;
             imin = (int)(ffit[Nspline-5]*Tobs);
         }
        */
             
                  if(k > 4 && k < Nspline-4)
                     {
                         imin = (int)(ffit[k-4]*Tobs);
                         imax = (int)(ffit[k+4]*Tobs);
                     }
                      
                     if(k <= 4)
                     {
                         imin = 0;
                         imax = (int)(ffit[6]*Tobs);
                     }
                     
                     if(k >= Nspline-4)
                     {
                         imax = Ns-1;
                         imin = (int)(ffit[Nspline-7]*Tobs);
                     }
             
             
            // Delta the smooth spectrum and prior
             logpy = logpx;
             logLy = logLx;
            for (i = imin; i <= imax; ++i)
               {
                          y = 0.0;
                           if(i > 0 && i < Ns-1)
                           {
                            x = gsl_spline_eval(cspline,freqs[i],acc);
                            y = -fabs(gsl_spline_eval_deriv2(cspline, freqs[i], acc));
                           }
                           else
                           {
                               if(i==0) x = Yspline[0];
                               if(i==Ns-1) x = Yspline[Nspline-1];
                           }
                           DS[i-imin] = exp(x);
                           DlnpR[i-imin] = y;
                           x = DS[i-imin] + SL[i];
                           DlnLR[i-imin] = -(log(x) + PS[i]/x);
                           logLy += (DlnLR[i-imin] - lnLR[i]);
                           logpy += (DlnpR[i-imin] - lnpR[i]);
               }
         
         
             
         }
         else  // line update
         {
             
            typ = 1;
            cL++;
             
            // pick a line to update
            k = (int)((double)(Nlines)*gsl_rng_uniform(r));
             
            ylinef = linef[k] + gsl_ran_gaussian(r,df);
            ylineh = lineh[k]*(1.0+gsl_ran_gaussian(r,0.05));
            ylineQ = lineQ[k] + gsl_ran_gaussian(r,1.0);
            if(ylineQ < 10.0) ylineQ = 10.0;
           
             spread = (1.0e-2*ylineQ);
             if(spread < 50.0) spread = 50.0;  // maximum half-width is f_resonance/20
             ydeltafmax = ylinef/spread;
             ylinew = 8.0*ydeltafmax;
             
             
            // need to cover old and new line
             imin = (int)((linef[k]-linew[k])*Tobs);
             imax = (int)((linef[k]+linew[k])*Tobs);
             i = (int)((ylinef-ylinew)*Tobs);
             if(i < imin) imin = i;
             i = (int)((ylinef+ylinew)*Tobs);
             if(i > imax) imax = i;
             if(imin < 0) imin = 0;
             if(imax > Ns-1) imax = Ns-1;
             
            // new line contribution
            f2 = ylinef*ylinef;
            f4 = f2*f2;
            for (i = imin; i <= imax; ++i)
               {
                    f = freqs[i];
                    y = 0.0;
                    x = fabs(f - ylinef);
                    if(x < ylinew)
                    {
                    z = 1.0;
                    if(x > ydeltafmax) z = exp(-(x-ydeltafmax)/ydeltafmax);
                    ff2 = f*f;
                    df2 = ylineQ*(1.0-ff2/f2);
                    df4 = df2*df2;
                    y += z*ylineh/(ff2/f2+df4);
                    }
                    DS[i-imin] = y;
               }
            
             
            // have to recompute and remove current line since lines can overlap
            f2 = linef[k]*linef[k];
            for (i = imin; i <= imax; ++i)
               {
                    f = freqs[i];
                    y = 0.0;
                    x = fabs(f - linef[k]);
                    if(x < linew[k])
                    {
                    z = 1.0;
                    if(x > deltafmax[k]) z = exp(-(x-deltafmax[k])/deltafmax[k]);
                    ff2 = f*f;
                    df2 = lineQ[k]*(1.0-ff2/f2);
                    df4 = df2*df2;
                    y += z*lineh[k]/(ff2/f2+df4);
                    }
                    DS[i-imin] -= y;
               }
            
             logpy = logpx;
             logLy = logLx;
             for (i = imin; i <= imax; ++i)
             {
                 x = DS[i-imin] + SM[i] + SL[i];
                 DlnLR[i-imin] = -(log(x) + PS[i]/x);
                 logLy += (DlnLR[i-imin] - lnLR[i]);
             }

             
             
         }
         
         
         H = (logLy-logLx) + pmul*(logpy - logpx);
         alpha = log(gsl_rng_uniform(r));
         
         if(H > alpha)
         {
             logLx = logLy;
             logpx = logpy;
             
             if(typ == 0)
             {
             acS++;
             Xspline[k] = Yspline[k];
             for (i = imin; i <= imax; ++i)
                {
                    SM[i] = DS[i-imin];  // replace this segment
                    lnLR[i] = DlnLR[i-imin];
                    lnpR[i] = DlnpR[i-imin];
                }
             }
             
             if(typ == 1)
             {
             acL++;
               
                linef[k] = ylinef;
                lineh[k] = ylineh;
                lineQ[k] = ylineQ;
                linew[k] = ylinew;
                deltafmax[k] = ydeltafmax;
                
               for (i = imin; i <= imax; ++i)
                {
                    SL[i] += DS[i-imin];   // delta this segment
                    lnLR[i] = DlnLR[i-imin];
                }
                 
             }
               
         }
         
        if(verbose == 1)  if(mc%1000 == 0) printf("%d %e %e %f %f\n", mc, logLx, pmul*logpx, (double)acS/(double)(cS), (double)acL/(double)(cL));
         
        if(verbose == 1)  if(mc%100 == 0) fprintf(out, "%d %e %e %f %f\n", mc, logLx, pmul*logpx, (double)acS/(double)(cS), (double)acL/(double)(cL));
    
     }
     if(verbose == 1) fclose(out);
    
    printf("There are %d lines\n", Nlines);
     
    out = fopen("specfit.dat","w");
    for (i = 0; i < Ns; ++i)
    {
        f = freqs[i];
        fprintf(out,"%e %e %e %e\n", f, SM[i], SL[i], SM[i]+SL[i]);
    }
    fclose(out);
    
     if(verbose == 1)
     {
         // final check
         gsl_spline_init(cspline,ffit,Xspline,Nspline);
         SM[0] = exp(Xspline[0]);
         SM[Ns-1] = exp(Xspline[Nspline-1]);
         for (i = 1; i < Ns-1; ++i) SM[i] = exp(gsl_spline_eval(cspline,freqs[i],acc));
         
    out = fopen("speccheck.dat","w");
    for (i = 0; i < Ns; ++i)
    {
        f = freqs[i];
        y = 0.0;
        for (j = 0; j < Nlines; ++j)
        {
            x = fabs(f - linef[j]);
            if(x < linew[j])
            {
            z = 1.0;
            if(x > deltafmax[j]) z = exp(-(x-deltafmax[j])/deltafmax[j]);
            f2 = linef[j]*linef[j];
            ff2 = f*f;
            df2 = lineQ[j]*(1.0-ff2/f2);
            df4 = df2*df2;
            y += z*lineh[j]/(ff2/f2+df4);
            }
        }
        SL[i] = y;
        fprintf(out,"%e %e %e %e\n", f, SM[i], SL[i], SM[i]+SL[i]);
        
    }
    fclose(out);
     }
    

    free(lnLR);
    free(DlnLR);
    free(DS);
    free(lnpR);
    free(DlnpR);
    free(lineh);
    free(linef);
    free(lineQ);
    free(linew);
    free(deltafmax);
    free(SM);
    free(SN);
    free(PS);
    free(times);
    free(data);
    
    return 0;

}

