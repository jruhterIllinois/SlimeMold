//  Sigmoid based neural net with backpropagation learning
//  ANNet.c
//  Slime mold v2
//
//  Created by Jeremy Ruhter on 1/8/20.
//  Copyright © 2020 Jeremy Ruhter. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>             // UNIX protocol
#include <netdb.h>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "./inc/mold_bin.h"
#include "./inc/mold_defs.h"
#include "./inc/mold_structs.h"



float * synap_input_wt[NUM_SYNAP_NET];
float * synap_out_wt[NUM_HIDDEN_NODE];
float * synap_input_wt_k[NUM_SYNAP_NET];
float * synap_out_wt_k[NUM_HIDDEN_NODE];
float ** glb_in_trn_mem;
float ** glb_out_trn_mem;

void * ANNet_initialize(void * args)
{
    int i,j;
    //allocate the space for the population learning
    
    
    for(i = 0; i< NUM_SYNAP_NET; i++)
    {
        if(( synap_input_wt[i] = (float *)valloc(NUM_HIDDEN_NODE*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(synap_input_wt[i] , NUM_HIDDEN_NODE*sizeof(float));
 
        if(( synap_input_wt_k[i] = (float *)valloc(NUM_HIDDEN_NODE*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(synap_input_wt_k[i] , NUM_HIDDEN_NODE*sizeof(float));
        
    }
    
    for(i = 0; i< NUM_HIDDEN_NODE; i++)
    {
        if(( synap_out_wt[i] = (float *)valloc(NUM_NET_OUT*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(synap_out_wt[i] , NUM_NET_OUT*sizeof(float));

        if(( synap_out_wt_k[i] = (float *)valloc(NUM_NET_OUT*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(synap_out_wt_k[i] , NUM_NET_OUT*sizeof(float));
        
    }
    
    if(( glb_in_trn_mem = (float **)valloc(POPULATION_SZ*sizeof(float *))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(glb_in_trn_mem , POPULATION_SZ*sizeof(float*));
    
    
    for(i = 0; i< POPULATION_SZ; i++)
    {
        if(( glb_in_trn_mem[i] = (float *)valloc(NUM_SYNAP_NET*NUM_HIDDEN_NODE*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(glb_in_trn_mem[i] , NUM_SYNAP_NET*NUM_HIDDEN_NODE*sizeof(float));
    }
    
    if(( glb_out_trn_mem = (float **)valloc(POPULATION_SZ*sizeof(float *))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(glb_out_trn_mem , POPULATION_SZ*sizeof(float*));
    
    
    for(i = 0; i< POPULATION_SZ; i++)
    {
        if(( glb_out_trn_mem[i] = (float *)valloc(NUM_NET_OUT*NUM_HIDDEN_NODE*sizeof(float))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(glb_out_trn_mem[i] , NUM_NET_OUT*NUM_HIDDEN_NODE*sizeof(float));
    }
    
    
    for(i = 0; i< POPULATION_SZ; i++)
    {
        for(j=0; j< NUM_SYNAP_NET*NUM_HIDDEN_NODE; j++)
        {
            glb_in_trn_mem[i][j] = (float)rand()/(float)RAND_MAX;
         //   glb_in_trn_mem[i][j] = 0.0;
        }
        
        for(j= 0; j<NUM_NET_OUT*NUM_HIDDEN_NODE; j++)
        {
            glb_out_trn_mem[i][j]  = (float)rand()/(float)RAND_MAX;
         //   glb_out_trn_mem[i][j]  = 0.0;
        }
       // printf("STORED %f %f\r\n",  glb_in_trn_mem[i][0], glb_out_trn_mem[i][0] );
    }
    
    
    
    return(0);
    
}


void * ANNet_Organism_cmpt(ORGANISM_t * orgnsm_x, void * in, void * out)
{
    
    float impls[NUM_SYNAP_NET];
    uint32_t org_identity = orgnsm_x->pop_idx;
    float hidd_impls[NUM_HIDDEN_NODE];
    float hidd_output[NUM_HIDDEN_NODE];
    float out_impls[NUM_NET_OUT];
    float * output_res = (float *) out;
    
    
    int i,j;
    
    for(i = 0; i< NUM_SYNAP_NET; i++)
        impls[i] = ((float *)in)[i];
    
//    printf("Organism identity %i with input %f %f %f \r\n", org_identity, impls[0], impls[1], impls[2]);
    
    //load the global memory data
    for(i=0; i< NUM_SYNAP_NET; i++)
        for(j = 0; j< NUM_HIDDEN_NODE; j++)
            synap_input_wt[i][j] = glb_in_trn_mem[org_identity][NUM_SYNAP_NET*i + j];
    
    //load the global memory data
    for(i=0; i< NUM_HIDDEN_NODE; i++)
        for(j = 0; j< NUM_NET_OUT; j++)
            synap_out_wt[i][j] = glb_out_trn_mem[org_identity][NUM_HIDDEN_NODE*i + j];
    
    //hidden layer computation
    //v1 = w11*x1 + w21*x2 +  w31*x3;
    
    
    for(i = 0; i<NUM_HIDDEN_NODE; i++)
    {
        hidd_impls[i] = 0.0;
        for(j=0; j< NUM_SYNAP_NET;j++)
            hidd_impls[i] = hidd_impls[i] + synap_input_wt[j][i]*impls[j];
        
        hidd_output[i] = sigmoid_func(&hidd_impls[i],1.0,0.0);
    }
    
    
    
    
  // printf("****THE results " );
    //output layer computations
    for(i = 0; i<NUM_NET_OUT; i++)
    {
        out_impls[i] = 0.0;
        for(j=0; j< NUM_HIDDEN_NODE;j++)
        {
     //       printf("%f ",synap_out_wt[j][i]);
            out_impls[i] = out_impls[i] + hidd_output[j]*synap_out_wt[j][i];
        }
        
        output_res[i] = sigmoid_func(&out_impls[i],1.0,0.0);
        
      //  printf(" --- %f ",output_res[i]);
    }
   //printf("\r\n");
    
    
    return(0);
}


void * ANNet_Organism_trn(ORGANISM_t * orgnsm_x, void * in, void * out)
{
    
    float impls[NUM_SYNAP_NET];
    float res_out[NUM_NET_OUT];
    uint32_t org_identity = orgnsm_x->pop_idx;
    float nu = 10;
    float hidd_impls[NUM_HIDDEN_NODE];
    float hidd_output[NUM_HIDDEN_NODE];
    float out_impls[NUM_NET_OUT];
    float output_res[NUM_NET_OUT];
    float delta[NUM_NET_OUT];

    
    int i,j,k;
    
    
    
    for(i = 0; i< NUM_SYNAP_NET; i++)
        impls[i] = ((float *)in)[i];
    
    for(i=0; i<NUM_NET_OUT; i++)
        res_out[i] = ((float *)out)[i];

    
    
    
    //load the global memory data
    for(i=0; i< NUM_SYNAP_NET; i++)
        for(j = 0; j< NUM_HIDDEN_NODE; j++)
            synap_input_wt[i][j] = glb_in_trn_mem[org_identity][NUM_SYNAP_NET*i + j];
    
    //load the global memory data
    for(i=0; i< NUM_HIDDEN_NODE; i++)
        for(j = 0; j< NUM_NET_OUT; j++)
            synap_out_wt[i][j] = glb_out_trn_mem[org_identity][NUM_HIDDEN_NODE*i + j];
    

    
    //hidden layer computation
    //v1 = w11*x1 + w21*x2 +  w31*x3;
    
    
    for(i = 0; i<NUM_HIDDEN_NODE; i++)
    {
        hidd_impls[i] = 0.0;
        for(j=0; j< NUM_SYNAP_NET;j++)
        {
            hidd_impls[i] += synap_input_wt[j][i]*impls[j];
        }
        hidd_output[i] = sigmoid_func(&hidd_impls[i],1.0,0.0);
   //    printf("Impulse %i %f %f %f %f ",i,synap_input_wt[0][i]*impls[0],
  //                                          synap_input_wt[1][i]*impls[1],
  //                                          synap_input_wt[2][i]*impls[2],
   //                                         hidd_impls[i]);
        
  //     printf(" %f \r\n", hidd_output[i]);
    }
  //  printf("\r\n");
    
   //output layer computations
    for(i = 0; i<NUM_NET_OUT; i++)
    {
        out_impls[i] = 0.0;
        for(j=0; j< NUM_HIDDEN_NODE;j++)
            out_impls[i] += hidd_output[j]*synap_out_wt[j][i];
        
         output_res[i] = sigmoid_func(&out_impls[i],1.0,0.0);
    }
   // printf("Out %f \r\n",  out_impls[3] );
    
    //Backpropagate step
    //compute the delta from the training
    //d1 = (y1_t[training_idx] - y1)*y1*(1-y1);
    for(i = 0; i<NUM_NET_OUT; i++)
        delta[i] = ( res_out[i] -output_res[i])*output_res[i]*(1-output_res[i]);
 

            
    //update hidden layer weights
    //w11_n = w11 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1) *x1;
    //w21_n =  w21 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x2;
    //number of inputs
    
    /*
     w11_n = w11 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1) *x1;
     w12_n = w12 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2) *x1;
     w16_n = w16 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6) *x1;
     
     w21_n =  w21 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x2;
     w22_n =  w22 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2)*x2;
     w26_n =  w26 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6)*x2;
     
     w31_n =  w31 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x3;
     w32_n =  w32 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2)*x3;
     w36_n =  w36 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6)*x3;
     */
    
    float acc_res = 0.0;
    for(k=0; k<NUM_SYNAP_NET; k++)
    {
        //number of hidden
        for(j=0; j< NUM_HIDDEN_NODE; j++)
        {
            //number of outputs
            acc_res = 0;
            for(i = 0; i<NUM_NET_OUT; i++)
            {
                acc_res += delta[i]*synap_out_wt[j][i];
            }
            
            synap_input_wt_k[k][j] = synap_input_wt[k][j] +
                                     nu*acc_res*
                                     hidd_output[j]*
                                     (1 - hidd_output[j])*
                                     impls[k];
            
      //      printf("Hidden input %f \r\n",  synap_input_wt[k][j]);

        }
    }
    
    /*
    s11_n = s11 + nu * d1 * z1;
    s12_n = s12 + nu * d1 * z2;
     .
     .

    s21_n = s21 + nu * d2 * z1;
    s22_n = s22 + nu * d2 * z2;
    */

    //update the output layer weights
    for(i=0; i<NUM_NET_OUT; i++)
    {
        //number of hidden
        for(j=0; j< NUM_HIDDEN_NODE; j++)
        {
            synap_out_wt_k[j][i] = synap_out_wt[j][i] + nu*delta[i]*hidd_output[j];
     //       printf("Temp - %f %f\r\n", synap_out_wt[j][i], nu*delta[i]*hidd_output[j]);
        }
    }
    
    
    //update the global coefficients
    for(i=0; i< NUM_SYNAP_NET; i++)
        for(j = 0; j< NUM_HIDDEN_NODE; j++)
            glb_in_trn_mem[org_identity][NUM_SYNAP_NET*i + j] = synap_input_wt_k[i][j];

    //load the global memory data
    for(i=0; i< NUM_HIDDEN_NODE; i++)
        for(j = 0; j< NUM_NET_OUT; j++)
            glb_out_trn_mem[org_identity][NUM_HIDDEN_NODE*i + j] = synap_out_wt_k[i][j];
    
    return(0);
}









/* Static example */
//Right now the Neural Net is a simple example of XOR as implented in -
//"An Introduction to Optimization" by Edwin Chong and Stanislaw H ZaK


void * ANNet(void * args)
{

    float x1,x2,x3; //input signals   n - 2
    
    x1 = ((float *)args)[0];
    x2 = ((float *)args)[1];
    x3 = ((float *)args)[2];
    int i;
    
    //If you have 3 inputs and 6 neurons you will have 18 weights per layer
    //first is the input number and the second is the neuron
    float w11,w12,w13,w14,w15,w16; //Input stage
    float w21,w22,w23,w24,w25,w26; //Hidden layer 1 stage, 6 neurons per stage l = 6
    float w31,w32,w33,w34,w35,w36;
    
    //should be w to follow convention but too confusing and made s
    float s11,s12,s13,s14,s15,s16; //Output layer weights
    float s21,s22,s23,s24,s25,s26;
    float s31,s32,s33,s34,s35,s36;
    float s41,s42,s43,s44,s45,s46;
    
    float v1,v2,v3,v4,v5,v6; //input into the hidden layer
    //v1 = w11*x1 + w21*x2 + w31*x3
    //v1 goes into the sigmoid function
    
    
    float z1,z2,z3,z4,z5,z6; //output of hidden layer 1 stage
    //z1 = f(v1)
    
    float V1, V2, V3, V4;
    
    float y1,y2,y3,y4; //outputs m -4 states
    //y1 = f(z1*s11 + z2*s12 + z3*s13 + z4*s14+ z5*s15 +z6*s16)
    
    
    //Backpropagate algorithm (used only for training the network
    //The update to the weights into the hidden layer
    float w11_n,w12_n,w13_n,w14_n,w15_n,w16_n;
    float w21_n,w22_n,w23_n,w24_n,w25_n,w26_n;
    float w31_n,w32_n,w33_n,w34_n,w35_n,w36_n;
    
    float nu = 10;
    //w11_n = w11 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * f'(v1)*x1 First neuron for first input
    //w12_n = w12 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * f'(v2)*x1 Second neuron for first input
    //   .
    //   .
    //   .
    //w16_n = w16 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * f'(v6)*x1 Second neuron for first input
    //w21_n = w21 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * f'(v1)*x2 First neuron for first input
    //   .
    //   .
    //   .
    //f'(v1) = f(v)(1-f(v)) = z1*(1-z1)
    
    
    //should be w to follow convention but too confusing and made s
    float s11_n,s12_n,s13_n,s14_n,s15_n,s16_n; //Output layer weights
    float s21_n,s22_n,s23_n,s24_n,s25_n,s26_n;
    float s31_n,s32_n,s33_n,s34_n,s35_n,s36_n;
    float s41_n,s42_n,s43_n,s44_n,s45_n,s46_n;
    //s11_n = s11 + nu * d1 * z1   :: the first neuron weight to the first output
    //s12_n = s12 + nu * d1 * z2   :: the second neuron weight to the first output
    //   .
    //   .
    //   .
    //s16_n = s16 + nu * d1 * z6  :: the sixth neuron weight to the first output
    //s21_n = s21 + nu * d2 * z1  :: the first neuron weight to the second output
    
    float d1, d2, d3, d4;
    //d1 = (Yd1 - y1)*f'(s11*z1 + s12*z2 + s13*z3 + s14*z4 + s15*z5 +s16*z6)
    //d1 = (Yd1 - y1)*y1*(1-y1)
    
    //d2 = (Yd2 - y2)*f'(s21*z1 + s22*z2 + s23*z3 + s24*z4 + s25*z5 +s26*z6)
    //d3 = (Yd3 - y3)*f'(s31*z1 + s32*z2 + s33*z3 + s34*z4 + s35*z5 +s36*z6)
    //d4 = (Yd4 - y4)*f'(s41*z1 + s42*z2 + s43*z3 + s44*z4 + s45*z5 +s16*z6)
    
    
    //f'(v) = f(v)(1-f(v))
    
    float x1_t[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
    float x2_t[8] = { 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};
    float x3_t[8] = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    
    float y1_t[8] = { 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0};
    float y2_t[8] = { 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0};
    float y3_t[8] = { 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0};
    float y4_t[8] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0};
    
    w11 = 0.3;
    w21 = 0.8;
    w31 = 0.5;
    
    w12 = 0.8;
    w22 = 0.5;
    w32 = 0.2;
    
    w13 = 0.5;
    w23 = 0.2;
    w33 = 0.8;
    
    w14 = 0.2;
    w24 = 0.4;
    w34 = 0.1;
    
    w15 = 0.9;
    w25 = 0.7;
    w35 = 0.1;
    
    w16 = 0.4;
    w26 = 0.2;
    w36 = 0.8;
    
    s11 = 0.05;
    s21 = 0.45;
    s31 = 0.22;
    s41 = 0.66;
    
    s12 = 0.2;
    s22 = 0.3;
    s32 = 0.1;
    s42 = 0.5;
    
    s13 = 0.4;
    s23 = 0.6;
    s33 = 0.2;
    s43 = 0.3;
    
    s14 = 0.9;
    s24 = 0.1;
    s34 = 0.7;
    s44 = 0.8;
    
    s15 = 0.5;
    s25 = 0.9;
    s35 = 0.3;
    s45 = 0.1;
    
    s16 = 0.86;
    s26  = 0.23;
    s36 = 0.9;
    s46 = 0.2;
    
    int training_idx = 0;
    int training_cnt = 0;
    
    printf("starting neural net input is %f %f %f\r\n", x1, x2, x3);
    
    while(training_cnt < 10001)
    {
        x1 = ((float *)args)[0];
        x2 = ((float *)args)[1];
        x3 = ((float *)args)[2];
        
        if(training_cnt <10000)
        {
            for(training_idx = 0; training_idx < 8; training_idx++)
            {
                
                //forward pass
                x1 = x1_t[training_idx];
                x2 = x2_t[training_idx];
                x3 = x3_t[training_idx];
                
                v1 = w11*x1 + w21*x2 +  w31*x3;
                v2 = w12*x1 + w22*x2 +  w32*x3;
                v3 = w13*x1 + w23*x2 +  w33*x3;
                v4 = w14*x1 + w24*x2 +  w34*x3;
                v5 = w15*x1 + w25*x2 +  w35*x3;
                v6 = w16*x1 + w26*x2 +  w36*x3;
                
                z1 = sigmoid_func(&v1,1.0,0.0);
                z2 = sigmoid_func(&v2,1.0,0.0);
                z3 = sigmoid_func(&v3,1.0,0.0);
                z4 = sigmoid_func(&v4,1.0,0.0);
                z5 = sigmoid_func(&v5,1.0,0.0);
                z6 = sigmoid_func(&v6,1.0,0.0);
                
                
                V1 = s11*z1 + s12*z2 + s13*z3 + s14*z4  + s15*z5 + s16*z6;
                V2 = s21*z1 + s22*z2 + s23*z3 + s24*z4  + s25*z5 + s26*z6;
                V3 = s31*z1 + s32*z2 + s33*z3 + s34*z4  + s35*z5 + s36*z6;
                V4 = s41*z1 + s42*z2 + s43*z3 + s44*z4  + s45*z5 + s46*z6;
                
                y1 = sigmoid_func(&V1,1.0,0.0);
                y2 = sigmoid_func(&V2,1.0,0.0);
                y3 = sigmoid_func(&V3,1.0,0.0);
                y4 = sigmoid_func(&V4,1.0,0.0);
                
                //backward propagate
                
                //update the activation function variables
                //d1 = (Yd1 - y1)*f'(s11*z1 + s12*z2 + s13*z3 + s14*z4 + s15*z5 +s16*z6)
                d1 = (y1_t[training_idx] - y1)*y1*(1-y1);
                d2 = (y2_t[training_idx] - y2)*y2*(1-y2);
                d3 = (y3_t[training_idx] - y3)*y3*(1-y3);
                d4 = (y4_t[training_idx] - y4)*y4*(1-y4);
        //        printf("Delta %f \r\n", d1);
                
                //update the hidden layer weights
                // w11_n = w11 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x1
                
                w11_n = w11 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1) *x1;
                w12_n = w12 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2) *x1;
                w13_n = w13 + nu * (d1*s13 + d2*s23 + d3*s33 + d4*s43) * z3*(1-z3) *x1;
                w14_n = w14 + nu * (d1*s14 + d2*s24 + d3*s34 + d4*s44) * z4*(1-z4) *x1;
                w15_n = w15 + nu * (d1*s15 + d2*s25 + d3*s35 + d4*s45) * z5*(1-z5) *x1;
                w16_n = w16 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6) *x1;
                
                w21_n =  w21 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x2;
                w22_n =  w22 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2)*x2;
                w23_n =  w23 + nu * (d1*s13 + d2*s23 + d3*s33 + d4*s43) * z3*(1-z3)*x2;
                w24_n =  w24 + nu * (d1*s14 + d2*s24 + d3*s34 + d4*s44) * z4*(1-z4)*x2;
                w25_n =  w25 + nu * (d1*s15 + d2*s25 + d3*s35 + d4*s45) * z5*(1-z5)*x2;
                w26_n =  w26 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6)*x2;
                
                w31_n =  w31 + nu * (d1*s11 + d2*s21 + d3*s31 + d4*s41) * z1*(1-z1)*x3;
                w32_n =  w32 + nu * (d1*s12 + d2*s22 + d3*s32 + d4*s42) * z2*(1-z2)*x3;
                w33_n =  w33 + nu * (d1*s13 + d2*s23 + d3*s33 + d4*s43) * z3*(1-z3)*x3;
                w34_n =  w34 + nu * (d1*s14 + d2*s24 + d3*s34 + d4*s44) * z4*(1-z4)*x3;
                w35_n =  w35 + nu * (d1*s15 + d2*s25 + d3*s35 + d4*s45) * z5*(1-z5)*x3;
                w36_n =  w36 + nu * (d1*s16 + d2*s26 + d3*s36 + d4*s46) * z6*(1-z6)*x3;
                
                
                
                //output laye weights
                s11_n = s11 + nu * d1 * z1;
                s12_n = s12 + nu * d1 * z2;
                s13_n = s13 + nu * d1 * z3;
                s14_n = s14 + nu * d1 * z4;
                s15_n = s15 + nu * d1 * z5;
                s16_n = s16 + nu * d1 * z6;
                
                s21_n = s21 + nu * d2 * z1;
                s22_n = s22 + nu * d2 * z2;
                s23_n = s23 + nu * d2 * z3;
                s24_n = s24 + nu * d2 * z4;
                s25_n = s25 + nu * d2 * z5;
                s26_n = s26 + nu * d2 * z6;
                
                s31_n = s31 + nu * d3 * z1;
                s32_n = s32 + nu * d3 * z2;
                s33_n = s33 + nu * d3 * z3;
                s34_n = s34 + nu * d3 * z4;
                s35_n = s35 + nu * d3 * z5;
                s36_n = s36 + nu * d3 * z6;
                
                s41_n = s41 + nu * d4 * z1;
                s42_n = s42 + nu * d4 * z2;
                s43_n = s43 + nu * d4 * z3;
                s44_n = s44 + nu * d4 * z4;
                s45_n = s45 + nu * d4 * z5;
                s46_n = s46 + nu * d4 * z6;
                
                w11 = w11_n;
                w12 = w12_n;
                w13 = w13_n;
                w14 = w14_n;
                w15 = w15_n;
                w16 = w16_n;
                
                w21 =  w21_n;
                w22 =  w22_n;
                w23 =  w23_n;
                w24 =  w24_n;
                w25 =  w25_n;
                w26 =  w26_n;
                
                w31 =  w31_n;
                w32 =  w32_n;
                w33 =  w33_n;
                w34 =  w34_n;
                w35 =  w35_n;
                w36 =  w36_n;
                
                
                s11 = s11_n;
                s12 = s12_n;
                s13 = s13_n;
                s14 = s14_n;
                s15 = s15_n;
                s16 = s16_n;
                
                s21 = s21_n;
                s22 = s22_n;
                s23 = s23_n;
                s24 = s24_n;
                s25 = s25_n;
                s26 = s26_n;
                
                s31 = s31_n;
                s32 = s32_n;
                s33 = s33_n;
                s34 = s34_n;
                s35 = s35_n;
                s36 = s36_n;
                
                s41 = s41_n;
                s42 = s42_n;
                s43 = s43_n;
                s44 = s44_n;
                s45 = s45_n;
                s46 = s46_n;
                
  
            }
            
            
        }
        else
        {
            
            v1 = w11*x1 + w21*x2 +  w31*x3;
            v2 = w12*x1 + w22*x2 +  w32*x3;
            v3 = w13*x1 + w23*x2 +  w33*x3;
            v4 = w14*x1 + w24*x2 +  w34*x3;
            v5 = w15*x1 + w25*x2 +  w35*x3;
            v6 = w16*x1 + w26*x2 +  w36*x3;
            
            z1 = sigmoid_func(&v1,1.0,0.0);
            z2 = sigmoid_func(&v2,1.0,0.0);
            z3 = sigmoid_func(&v3,1.0,0.0);
            z4 = sigmoid_func(&v4,1.0,0.0);
            z5 = sigmoid_func(&v5,1.0,0.0);
            z6 = sigmoid_func(&v6,1.0,0.0);
            
            V1 = s11*z1 + s12*z2 + s13*z3 + s14*z4  + s15*z5 + s16*z6;
            V2 = s21*z1 + s22*z2 + s23*z3 + s24*z4  + s25*z5 + s26*z6;
            V3 = s31*z1 + s32*z2 + s33*z3 + s34*z4  + s35*z5 + s36*z6;
            V4 = s41*z1 + s42*z2 + s43*z3 + s44*z4  + s45*z5 + s46*z6;
            
            y1 = sigmoid_func(&V1,1.0,0.0);
            y2 = sigmoid_func(&V2,1.0,0.0);
            y3 = sigmoid_func(&V3,1.0,0.0);
            y4 = sigmoid_func(&V4,1.0,0.0);
            
            
     //       printf(" MODEL -- %f %f %f %f %f %f\r\n", s41, s42 , s43, s44, s45, s46);
            
        }
    //    printf("Training Idx is at %i \r\n", training_cnt);
        training_cnt++;
        
    }
  printf("*********************** Y1 %f Y2 %f Y3 %f Y4 %f \r\n", y1, y2, y3, y4);
    
    for(i = 0; i< 8; i++)
        if(x1_t[i]== x1  &&  x2_t[i]==x2  &&  x3_t[i]==x3)
                                printf("*********************** Training sequence was  Y1 %f Y2 %f Y3 %f Y4 %f \r\n", y1_t[i], y2_t[i], y3_t[i], y4_t[i]   );
    return(0);
    
}


float sigmoid_func(float * in, float beta, float offset)
{
    return( beta/(1 + exp( - in[0] -  offset) ));
}
